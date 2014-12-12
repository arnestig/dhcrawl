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
{
	DHCPsocket[ 0 ] = 0;
	DHCPsocket[ 1 ] = 0;
    dhcp_to.sin_family=AF_INET;
    dhcp_to.sin_addr.s_addr=INADDR_BROADCAST;
    dhcp_to.sin_port=htons(67);

    name67.sin_family = AF_INET;
    name67.sin_port = htons(67);
    name67.sin_addr.s_addr = INADDR_ANY;

    name68.sin_family = AF_INET;
    name68.sin_port = htons(68);
    name68.sin_addr.s_addr = INADDR_ANY;

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

    DHCPsocket[ 0 ] = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    setsockopt( DHCPsocket[ 0 ], SOL_SOCKET, SO_REUSEADDR, &socket_mode, sizeof( socket_mode ) ); // reuse address
    setsockopt( DHCPsocket[ 0 ], SOL_SOCKET, SO_BROADCAST, &socket_mode, sizeof( socket_mode ) ); // broadcast mode

    if ( bind( DHCPsocket[ 0 ], (struct sockaddr *)&name67, sizeof( name67 ) ) < 0 ) {
        std::cerr << "Error during bind()" << std::endl;
        exit(1);
    }

    DHCPsocket[ 1 ] = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    setsockopt( DHCPsocket[ 1 ], SOL_SOCKET, SO_REUSEADDR, &socket_mode, sizeof( socket_mode ) ); // reuse address
    setsockopt( DHCPsocket[ 1 ], SOL_SOCKET, SO_BROADCAST, &socket_mode, sizeof( socket_mode ) ); // broadcast mode

    if ( bind( DHCPsocket[ 1 ], (struct sockaddr *)&name68, sizeof( name68 ) ) < 0 ) {
        std::cerr << "Error during bind()" << std::endl;
        exit(1);
    }

	pthread_create( &worker, NULL, work, this );
	pthread_detach( worker );

}

void DHCP::stop()
{
}

void DHCP::discover( std::string hardware )
{
	struct dhcp_t dhcpMessage;
	bzero( &dhcpMessage, sizeof( dhcpMessage ) );
	dhcpMessage.opcode = 1;
	dhcpMessage.htype = 1;
	dhcpMessage.hlen = 6;
	uint32_t xid = time( NULL );
	Resources::Instance()->getState()->setXid( xid );
	dhcpMessage.xid = htonl( xid );
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

    if ( sendto( DHCPsocket[ 1 ], &dhcpMessage, sizeof(dhcpMessage), 0, (struct sockaddr *)&dhcp_to, sizeof(dhcp_to) ) != sizeof(dhcpMessage) )
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
    struct timeval timeout;
    timeout.tv_sec=1;
    timeout.tv_usec=0;
    fd_set read;
    while (1) {
        FD_ZERO( &read );
		int max_sock;
		for ( int sockid = 0; sockid < 2; sockid++ ) {
		int sockfd = parent->DHCPsocket[ sockid ];
		if ( sockfd > 0 ) {
			FD_SET( sockfd, &read );
		}

		if ( sockfd > max_sock ) {
			max_sock = sockfd;
		}

		}

        if ( select( max_sock+1, &read, 0, 0, &timeout ) < 0 )
        {
			std::cerr << "Error during select()" << std::endl;
            exit(1);
        }

		for ( int sockid = 0; sockid < 2; sockid++ ) {
			int sockfd = parent->DHCPsocket[ sockid ];
			if ( FD_ISSET( sockfd, &read ) ) {
				struct sockaddr_in fromsock;
				socklen_t fromlen=sizeof(fromsock);
				int addr;

				// get data from socket
				struct dhcp_t dhcpPackage;
				recvfrom( sockfd, &dhcpPackage, sizeof( dhcpPackage ), 0, (struct sockaddr *)&fromsock, &fromlen);
				addr=ntohl(fromsock.sin_addr.s_addr);

				// check which filter is active
				bool addPackage = false;
				int DHCPtype = Parser::getDHCPMessageType( dhcpPackage.options );
				printf("Type:%d\n", DHCPtype );
				switch ( Resources::Instance()->getState()->getFilter() ) {
					case 1: // we look at all DHCPDISCOVER messages
						if ( DHCPtype == 1 ) {
							parent->packages.push_back( dhcpPackage );
							addPackage = true;
						}
						break;

					case 2: // we look at all DHCPOFFER messages
						if ( DHCPtype == 2 ) {
							parent->packages.push_back( dhcpPackage );
							addPackage = true;
						}
						break;

					case 3: // we look at all DHCPREQUEST messages
						if ( DHCPtype == 3 ) {
							parent->packages.push_back( dhcpPackage );
							addPackage = true;
						}
						break;

					case 4: // we look at all DHCPOFFER messages
						/** check if the xid is matching our sent out request **/
						if ( DHCPtype == 2 ) {
							if ( Resources::Instance()->getState()->getXid() == ntohl( dhcpPackage.xid ) ) {
								parent->packages.push_back( dhcpPackage );
								addPackage = true;
							}
						}
						break;

					default: // default or 0, not expected or valid DHCP message type
					case 0:
						break;
				}

				// if we received a message used in a current filter, we need to publish it
				if ( addPackage == true ) {
					pthread_mutex_lock( &parent->mutex ); // lock our data mutex
					pthread_mutex_unlock( &parent->mutex ); // unlock our data mutex
					sem_post( &parent->semaphore ); // inform main thread data is available
				}
			}
		}
    }
}
