/**
    Copyright (C) 2014 dhcrawl - Probe DHCP servers to see what offers are sent

    Written by Tobias Eliasson <arnestig@gmail.com>.

    This file is part of dhcrawl https://github.com/arnestig/dhcrawl

    dhcrawl is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    dhcrawl is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with dhcrawl.  If not, see <http://www.gnu.org/licenses/>.
**/

#include <pthread.h>
#include "dhcp.h"
#include "parser.h"
#include "resources.h"

DHCP::DHCP()
	:	DHCPsocket( 0 )
{
    dhcp_to.sin_family=AF_INET;
    dhcp_to.sin_addr.s_addr=INADDR_BROADCAST;
    dhcp_to.sin_port=htons(67);

    name.sin_family = AF_INET;
    name.sin_port = htons(68);
    name.sin_addr.s_addr = INADDR_ANY;

	sem_init( &semaphore, 0, 0 );	
	pthread_mutex_init( &mutex, NULL );
}

DHCP::~DHCP()
{
	sem_destroy( &semaphore );
	pthread_mutex_destroy( &mutex );
}

void DHCP::start()
{
    int socket_mode=1;

    DHCPsocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    setsockopt( DHCPsocket, SOL_SOCKET, SO_REUSEADDR, &socket_mode, sizeof( socket_mode ) ); // reuse address
    setsockopt( DHCPsocket, SOL_SOCKET, SO_BROADCAST, &socket_mode, sizeof( socket_mode ) ); // broadcast mode

    if ( bind( DHCPsocket, (struct sockaddr *)&name, sizeof( name ) ) < 0 ) {
        std::cerr << "Error during bind()" << std::endl;
        exit(1);
    }

	pthread_create( &worker, NULL, work, this );
	pthread_detach( worker );

}

void DHCP::stop()
{
}

void DHCP::inform( std::string hardware )
{
	struct dhcp_t dhcpMessage;
	bzero( &dhcpMessage, sizeof( dhcpMessage ) );
	dhcpMessage.opcode = 1;
	dhcpMessage.htype = 1;
	dhcpMessage.hlen = 6;
	dhcpMessage.xid = htonl( time( NULL ) );;
	dhcpMessage.magic = htonl( 0x63825363 );
	dhcpMessage.options[ 0 ] = 53;
	dhcpMessage.options[ 1 ] = 1;
	dhcpMessage.options[ 2 ] = 1;
	dhcpMessage.options[ 3 ] = 255;
    /* chaddr */
    unsigned int hw[16];
    memset(&hw,0,sizeof(hw));
    if ( sscanf( hardware.c_str(), "%x:%x:%x:%x:%x:%x", &hw[0], &hw[1], &hw[2], &hw[3], &hw[4], &hw[5] ) != 6 ) {
		std::cerr << "Invalid mac-format" << std::endl;
		exit(1);
	}
	dhcpMessage.chaddr[ 0 ] = hw[ 0 ];
	dhcpMessage.chaddr[ 1 ] = hw[ 1 ];
	dhcpMessage.chaddr[ 2 ] = hw[ 2 ];
	dhcpMessage.chaddr[ 3 ] = hw[ 3 ];
	dhcpMessage.chaddr[ 4 ] = hw[ 4 ];
	dhcpMessage.chaddr[ 5 ] = hw[ 5 ];

    if ( sendto( DHCPsocket, &dhcpMessage, sizeof(dhcpMessage), 0, (struct sockaddr *)&dhcp_to, sizeof(dhcp_to) ) != sizeof(dhcpMessage) )
    {
		std::cerr << "Error during sendto()" << std::endl;
        exit(1);
    }
}

bool DHCP::waitForData( struct dhcp_t &package )
{
	// setup a timeout so we can wait maximum of 40ms
	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	if ( timeout.tv_nsec + 40000000 > 999999999 ) {
		timeout.tv_sec += 1;
		timeout.tv_nsec += 40000000 - 999999999;
	} else {
		timeout.tv_nsec += 40000000;
	}

	if ( sem_timedwait( &semaphore, &timeout ) == 0 ) {
		pthread_mutex_lock( &mutex );
		package = packages.back();
		packages.pop_back();
		pthread_mutex_unlock( &mutex );
		return true;
	}
	return false;
}

void *DHCP::work( void *context )
{
	DHCP *parent = static_cast< DHCP* >( context );
	int sockfd = parent->DHCPsocket;
    struct timeval timeout;
    timeout.tv_sec=1;
    timeout.tv_usec=0;
    fd_set read;
    while (1) {
        FD_ZERO( &read );
        FD_SET( sockfd, &read );

        if ( select( sockfd+1, 0, &read, 0, &timeout ) < 0 )
        {
			std::cerr << "Error during select()" << std::endl;
            exit(1);
        }

        if ( FD_ISSET( sockfd, &read ) ) {

			struct sockaddr_in fromsock;
			socklen_t fromlen=sizeof(fromsock);
			int addr;

			// get data from socket
			struct dhcp_t dhcpPackage;
			recvfrom( sockfd, &dhcpPackage, sizeof( dhcpPackage ), 0, (struct sockaddr *)&fromsock, &fromlen);
			addr=ntohl(fromsock.sin_addr.s_addr);

			// check which filter is active
			bool packageAdded = false;
			switch ( Resources::Instance()->getState()->getFilter() ) {
				case 1: { // PROBE - so we only want to see DHCPOFFER messages
					int DHCPtype = Parser::getDHCPMessageType( dhcpPackage.options );
					/** check if the xid is matching our sent out request **/
					//if ( xid == ntohl( dhcpPackage.xid ) ) {
					if ( DHCPtype == 2 ) {
						parent->packages.push_back( dhcpPackage );
						packageAdded = true;
					}
					break;
				}
				case 2: { // MONITOR - we look at all DHCPREQUEST messages
					int DHCPtype = Parser::getDHCPMessageType( dhcpPackage.options );
					if ( DHCPtype == 3 ) {
						parent->packages.push_back( dhcpPackage );
						packageAdded = true;
					}
					break;
				}

				default: // default or 0, not expected or valid DHCP message type
				case 0:
					break;
			}

			if ( packageAdded == true ) {
				pthread_mutex_lock( &parent->mutex ); // lock our data mutex
				pthread_mutex_unlock( &parent->mutex ); // unlock our data mutex
				sem_post( &parent->semaphore ); // inform main thread data is available
			}
        }
    }
}
