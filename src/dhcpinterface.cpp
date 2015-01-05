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
#include <signal.h>
#include "dhcpinterface.h"

extern std::vector< std::string > errorLog;

DHCPInterface::DHCPInterface()
    :   timeToQuit( false ),
        filter( new Filter() )
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

	sem_init( &threadFinished, 0, 0 );	
	pthread_mutex_init( &mutex, NULL );

    srand( time( NULL ) );
}

DHCPInterface::~DHCPInterface()
{
    timeToQuit = true;
    sem_wait( &threadFinished );
    sem_destroy( &threadFinished );

	pthread_mutex_destroy( &mutex );
    delete filter;
}

void DHCPInterface::start()
{
    int socket_mode=1;

    DHCPInterfaceSocket[ 0 ] = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    setsockopt( DHCPInterfaceSocket[ 0 ], SOL_SOCKET, SO_REUSEADDR, &socket_mode, sizeof( socket_mode ) ); // reuse address
    setsockopt( DHCPInterfaceSocket[ 0 ], SOL_SOCKET, SO_BROADCAST, &socket_mode, sizeof( socket_mode ) ); // broadcast mode

    if ( bind( DHCPInterfaceSocket[ 0 ], (struct sockaddr *)&name67, sizeof( name67 ) ) < 0 ) {
        errorLog.push_back( "Error during bind():" );
        raise( SIGINT );
    }

    DHCPInterfaceSocket[ 1 ] = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    setsockopt( DHCPInterfaceSocket[ 1 ], SOL_SOCKET, SO_REUSEADDR, &socket_mode, sizeof( socket_mode ) ); // reuse address
    setsockopt( DHCPInterfaceSocket[ 1 ], SOL_SOCKET, SO_BROADCAST, &socket_mode, sizeof( socket_mode ) ); // broadcast mode

    if ( bind( DHCPInterfaceSocket[ 1 ], (struct sockaddr *)&name68, sizeof( name68 ) ) < 0 ) {
        errorLog.push_back( "Error during bind():" );
        raise( SIGINT );
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
    // set our filter to the xid we just generated, we only want to see data from this xid
	filter->setXid( xid );
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
        errorLog.push_back( "Invalid mac-format:" );
        raise( SIGINT );
	}
	dhcpPackage.chaddr[ 0 ] = hw[ 0 ];
	dhcpPackage.chaddr[ 1 ] = hw[ 1 ];
	dhcpPackage.chaddr[ 2 ] = hw[ 2 ];
	dhcpPackage.chaddr[ 3 ] = hw[ 3 ];
	dhcpPackage.chaddr[ 4 ] = hw[ 4 ];
	dhcpPackage.chaddr[ 5 ] = hw[ 5 ];

    if ( sendto( DHCPInterfaceSocket[ 1 ], &dhcpPackage, sizeof(dhcpPackage), 0, (struct sockaddr *)&dhcp_to, sizeof(dhcp_to) ) != sizeof(dhcpPackage) )
    {
        errorLog.push_back( "Error during sendto():" );
        raise( SIGINT );
    }
}

Filter* DHCPInterface::getFilter()
{
    return filter;
}

std::vector< DHCPMessage* > DHCPInterface::getMessages()
{
    std::vector< DHCPMessage* > retvec;
    pthread_mutex_lock( &mutex ); // lock our data mutex
    for( std::vector< DHCPMessage* >::iterator it = messages.begin(); it != messages.end(); ++it ) {
        if ( filter->matchFilter( (*it)->getMACAddress(), (*it)->getOfferedIP(), (*it)->getXid() ) == true ) {
            retvec.push_back( (*it) );
        }
    }
    pthread_mutex_unlock( &mutex ); // unlock our data mutex
    return retvec;
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

        if ( select( max_sock+1, &read, 0, 0, &timeout ) < 0 ) {
			std::cerr << "Error during select()" << std::endl;
        }

		for ( int sockid = 0; sockid < 2; sockid++ ) {
			int sockfd = parent->DHCPInterfaceSocket[ sockid ];
			if ( FD_ISSET( sockfd, &read ) ) {
				// get data from socket
				struct dhcp_t dhcpPackage;
				recv( sockfd, &dhcpPackage, sizeof( dhcpPackage ), 0 );
				DHCPMessage *dhcpMessage = new DHCPMessage( dhcpPackage );

                pthread_mutex_lock( &parent->mutex ); // lock our data mutex
                parent->messages.insert( parent->messages.begin(), dhcpMessage );
                pthread_mutex_unlock( &parent->mutex ); // unlock our data mutex
			}
		}
    }

    sem_post( &parent->threadFinished );
    return 0;
}
