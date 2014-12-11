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
	//memset( dhcpMessage.chaddr, 0, 16 );
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

void DHCP::waitForData()
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
		packages = 0;
		pthread_mutex_unlock( &mutex );
	}
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

			pthread_mutex_lock( &parent->mutex ); // lock our data mutex
			parent->packages++;
			pthread_mutex_unlock( &parent->mutex ); // unlock our data mutex
			sem_post( &parent->semaphore ); // inform main thread data is available

			/** check if the xid is matching our sent out request **/
			//if ( xid == ntohl( dhcpPackage.xid ) ) {
				uint32_t recv_yiaddr = ntohl( dhcpPackage.yiaddr );
				printf( "    OP: %d\n", dhcpPackage.opcode );
				printf( " HTYPE: %d\n", dhcpPackage.htype );
				printf( "  HLEN: %d\n", dhcpPackage.hlen );
				printf( "  HOPS: %d\n", dhcpPackage.hops );
				printf( "   XID: %x\n", htonl( dhcpPackage.xid ) );
				printf( "  SECS: %d\n", htonl( dhcpPackage.secs ) );
				printf( " FLAGS: %d\n", htonl( dhcpPackage.flags ) );
				printf( "CIADDR: %d.%d.%d.%d\n", ( htonl( dhcpPackage.ciaddr ) >> 24 ) & 0xFF, ( htonl( dhcpPackage.ciaddr ) >> 16 ) & 0xFF, ( htonl( dhcpPackage.ciaddr ) >> 8 ) & 0xFF, ( htonl( dhcpPackage.ciaddr ) ) & 0xFF );
				printf( "YIADDR: %d.%d.%d.%d\n", ( htonl( dhcpPackage.yiaddr ) >> 24 ) & 0xFF, ( htonl( dhcpPackage.yiaddr ) >> 16 ) & 0xFF, ( htonl( dhcpPackage.yiaddr ) >> 8 ) & 0xFF, ( htonl( dhcpPackage.yiaddr ) ) & 0xFF );
				printf( "SIADDR: %d.%d.%d.%d\n", ( htonl( dhcpPackage.siaddr ) >> 24 ) & 0xFF, ( htonl( dhcpPackage.siaddr ) >> 16 ) & 0xFF, ( htonl( dhcpPackage.siaddr ) >> 8 ) & 0xFF, ( htonl( dhcpPackage.siaddr ) ) & 0xFF );
				printf( "GIADDR: %d.%d.%d.%d\n", ( htonl( dhcpPackage.giaddr ) >> 24 ) & 0xFF, ( htonl( dhcpPackage.giaddr ) >> 16 ) & 0xFF, ( htonl( dhcpPackage.giaddr ) >> 8 ) & 0xFF, ( htonl( dhcpPackage.giaddr ) ) & 0xFF );
				printf( "CHADDR: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n", dhcpPackage.chaddr[ 0 ], dhcpPackage.chaddr[ 1 ], dhcpPackage.chaddr[ 2 ], dhcpPackage.chaddr[ 3 ], dhcpPackage.chaddr[ 4 ], dhcpPackage.chaddr[ 5 ] );
				printf( " SNAME: %s\n", dhcpPackage.sname );
				printf( "  FILE: %s\n", dhcpPackage.file );
				int i = 0;
				uint8_t option = 0;
				while ( option != 255 ) {
					option = dhcpPackage.options[ i ];
					uint8_t length = dhcpPackage.options[ ++i ];
					printf( "Option: %d ( %d ): ", option, length );
					for ( uint8_t x = 0; x < length; x++ ) {
						printf( "%d", dhcpPackage.options[ ++i ] );
					}
					printf( "\n" );

					if ( ++i >= 308 ) {
						break;
					};
				}
				printf( "%d.%d.%d.%d offered %d.%d.%d.%d\n",
						( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
						( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF,
						( recv_yiaddr >> 24 ) & 0xFF, ( recv_yiaddr >> 16 ) & 0xFF,
						( recv_yiaddr >>  8 ) & 0xFF, ( recv_yiaddr       ) & 0xFF );
			//}
        }
    }
}
