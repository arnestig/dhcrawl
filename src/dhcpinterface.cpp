/**
    Copyright (C) 2014 dhcrawl - Probe DHCPInterface servers to see what offers are sent

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
#include "dhcpinterface.h"
#include "resources.h"

DHCPInterface::DHCPInterface()
    :   timeToQuit( false )
{
	DHCPInterfaceSocket[ 0 ] = 0;
	DHCPInterfaceSocket[ 1 ] = 0;
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

    srand( time( NULL ) );
}

DHCPInterface::~DHCPInterface()
{
    timeToQuit = true;
    sem_wait( &threadFinished );
    //sem_destroy( &threadFinished );

	//sem_destroy( &semaphore );
	//pthread_mutex_destroy( &mutex );
}

void DHCPInterface::start()
{
    int socket_mode=1;

    DHCPInterfaceSocket[ 0 ] = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    setsockopt( DHCPInterfaceSocket[ 0 ], SOL_SOCKET, SO_REUSEADDR, &socket_mode, sizeof( socket_mode ) ); // reuse address
    setsockopt( DHCPInterfaceSocket[ 0 ], SOL_SOCKET, SO_BROADCAST, &socket_mode, sizeof( socket_mode ) ); // broadcast mode

    if ( bind( DHCPInterfaceSocket[ 0 ], (struct sockaddr *)&name67, sizeof( name67 ) ) < 0 ) {
        std::cerr << "Error during bind()" << std::endl;
        exit(1);
    }

    DHCPInterfaceSocket[ 1 ] = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    setsockopt( DHCPInterfaceSocket[ 1 ], SOL_SOCKET, SO_REUSEADDR, &socket_mode, sizeof( socket_mode ) ); // reuse address
    setsockopt( DHCPInterfaceSocket[ 1 ], SOL_SOCKET, SO_BROADCAST, &socket_mode, sizeof( socket_mode ) ); // broadcast mode

    if ( bind( DHCPInterfaceSocket[ 1 ], (struct sockaddr *)&name68, sizeof( name68 ) ) < 0 ) {
        std::cerr << "Error during bind()" << std::endl;
        exit(1);
    }

	pthread_create( &worker, NULL, work, this );
	pthread_detach( worker );

}

void DHCPInterface::stop()
{
}

void DHCPInterface::sendDiscover( std::string hardware )
{
	struct dhcp_t dhcpPackage;
	bzero( &dhcpPackage, sizeof( dhcpPackage ) );
	dhcpPackage.opcode = 1;
	dhcpPackage.htype = 1;
	dhcpPackage.hlen = 6;
	uint32_t xid = rand();
	Resources::Instance()->getState()->setXid( xid );
	dhcpPackage.xid = htonl( xid );
	dhcpPackage.magic = htonl( 0x63825363 );
	dhcpPackage.options[ 0 ] = 53;
	dhcpPackage.options[ 1 ] = 1;
	dhcpPackage.options[ 2 ] = 1;
	dhcpPackage.options[ 3 ] = 255;
    /* chaddr */
    unsigned int hw[16];
    memset(&hw,0,sizeof(hw));
    if ( sscanf( hardware.c_str(), "%x:%x:%x:%x:%x:%x", &hw[0], &hw[1], &hw[2], &hw[3], &hw[4], &hw[5] ) != 6 ) {
		std::cerr << "Invalid mac-format" << std::endl;
		exit(1);
	}
	dhcpPackage.chaddr[ 0 ] = hw[ 0 ];
	dhcpPackage.chaddr[ 1 ] = hw[ 1 ];
	dhcpPackage.chaddr[ 2 ] = hw[ 2 ];
	dhcpPackage.chaddr[ 3 ] = hw[ 3 ];
	dhcpPackage.chaddr[ 4 ] = hw[ 4 ];
	dhcpPackage.chaddr[ 5 ] = hw[ 5 ];

    if ( sendto( DHCPInterfaceSocket[ 1 ], &dhcpPackage, sizeof(dhcpPackage), 0, (struct sockaddr *)&dhcp_to, sizeof(dhcp_to) ) != sizeof(dhcpPackage) )
    {
		std::cerr << "Error during sendto()" << std::endl;
        exit(1);
    }
}

DHCPMessage* DHCPInterface::waitForMessage()
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
		DHCPMessage *dhcpMessage = messages.back();
		messages.pop_back();
		pthread_mutex_unlock( &mutex );
		return dhcpMessage;
	}
	return NULL;
}

void *DHCPInterface::work( void *context )
{
	DHCPInterface *parent = static_cast< DHCPInterface* >( context );
    struct timeval timeout;
    timeout.tv_sec=1;
    timeout.tv_usec=0;
    fd_set read;
    while ( parent->timeToQuit == false ) {
        FD_ZERO( &read );
		int max_sock = 0;
		for ( int sockid = 0; sockid < 2; sockid++ ) {
            int sockfd = parent->DHCPInterfaceSocket[ sockid ];
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
			int sockfd = parent->DHCPInterfaceSocket[ sockid ];
			if ( FD_ISSET( sockfd, &read ) ) {
				// get data from socket
				struct dhcp_t dhcpPackage;
				recv( sockfd, &dhcpPackage, sizeof( dhcpPackage ), 0 );
				DHCPMessage *dhcpMessage = new DHCPMessage( dhcpPackage );

				// check which filter is active
				bool addPackage = false;
				int DHCPInterfaceType = dhcpMessage->getMessageType();
				switch ( Resources::Instance()->getState()->getFilter() ) {
					case 1: // we look at all DHCPInterfaceDISCOVER messages
						if ( DHCPInterfaceType == 1 ) {
							addPackage = true;
						}
						break;

					case 2: // we look at all DHCPInterfaceOFFER messages
						if ( DHCPInterfaceType == 2 ) {
							addPackage = true;
						}
						break;

					case 3: // we look at all DHCPInterfaceREQUEST messages
						if ( DHCPInterfaceType == 3 ) {
							addPackage = true;
						}
						break;

					case 4: // we look at all DHCPInterfaceOFFER messages
						/** check if the xid is matching our sent out request **/
						if ( DHCPInterfaceType == 2 ) {
							if ( Resources::Instance()->getState()->getXid() == dhcpMessage->getXid() ) {
								addPackage = true;
							}
						}
						break;

					default: // default or 0, not expected or valid DHCPInterface message type
					case 0:
						break;
				}

				// if we received a message used in a current filter, we need to publish it
				addPackage = true; 
				if ( addPackage == true ) {
					pthread_mutex_lock( &parent->mutex ); // lock our data mutex
					parent->messages.push_back( dhcpMessage );
					pthread_mutex_unlock( &parent->mutex ); // unlock our data mutex
					sem_post( &parent->semaphore ); // inform main thread data is available
				}
			}
		}
    }

    sem_post( &parent->threadFinished );
    return 0;
}
