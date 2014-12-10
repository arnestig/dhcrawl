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
	:	offset( 0 ),
		DHCPsocket( 0 )
{
	memset( xid, 0, 4 );
    dhcp_to.sin_family=AF_INET;
    dhcp_to.sin_addr.s_addr=INADDR_BROADCAST;
    dhcp_to.sin_port=htons(67);

    name.sin_family = AF_INET;
    name.sin_port = htons(68);
    name.sin_addr.s_addr = INADDR_ANY;

	
	//pthread_mutex_init( &myMutex, NULL );
}

DHCP::~DHCP()
{
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

	pthread_mutex_init( &dhcp_read_mutex, NULL );
}

void DHCP::stop()
{
}

void DHCP::addpacket( unsigned char* pktbuf, char *buffer, int size )
{
    memcpy( pktbuf+offset, buffer, size );
    offset += size;
}

void DHCP::inform( std::string hardware )
{
    char buffer[2048];
    unsigned char pktbuf[2048];
    unsigned int hw[16];

    memset(buffer,0,sizeof(buffer));
    sprintf(buffer,"\1\1%c%c",6,0);
    addpacket(pktbuf,buffer,4);

    /* xid */
    unsigned long time_xid=time(NULL);
    memcpy(xid,&time_xid,4);
    addpacket(pktbuf,xid,4);

    /* secs,flags,ciaddr,yiaddr,siaddr,giaddr */
    memset(buffer,0,20);
    addpacket(pktbuf,buffer,20);

    /* chaddr */
    memset(&hw,0,sizeof(hw));
    if ( sscanf( hardware.c_str(), "%x:%x:%x:%x:%x:%x", &hw[0], &hw[1], &hw[2], &hw[3], &hw[4], &hw[5] ) != 6 ) {
		std::cerr << "Invalid mac-format" << std::endl;
		exit(1);
	}

    sprintf(buffer,"%c%c%c%c%c%c", hw[0],hw[1],hw[2],hw[3],hw[4],hw[5] );
    addpacket(pktbuf,buffer,16);

    /* sname, file */
    memset(buffer,0,192);
    addpacket(pktbuf,buffer,192);

    /* cookie */
    sprintf(buffer,"%c%c%c%c",99,130,83,99);
    addpacket(pktbuf,buffer,4);

    /* dhcp-type */
    sprintf(buffer,"%c%c%c",53,1,1);
    addpacket(pktbuf,buffer,3);

    /* end of options */
    sprintf(buffer,"%c",255);
    addpacket(pktbuf,buffer,1);

    if ( sendto( DHCPsocket, pktbuf, offset, 0, (struct sockaddr *)&dhcp_to, sizeof(dhcp_to) ) != offset )
    {
		std::cerr << "Error during sendto()" << std::endl;
        exit(1);
    }
}

void DHCP::waitForData()
{
	std::cout << "waitForData++" << std::endl;
	pthread_mutex_lock( &dhcp_read_mutex );
	pthread_mutex_unlock( &dhcp_read_mutex );
	std::cout << "waitForData--" << std::endl;
}

void *DHCP::work( void *context )
{
	DHCP *parent = static_cast< DHCP* >( context );
	int sockfd = parent->DHCPsocket;
    struct timeval timeout;
    timeout.tv_sec=1;
    timeout.tv_usec=0;
    fd_set read;
	size_t bufferSize = 2048;
    while (1) {
		pthread_mutex_lock( &parent->dhcp_read_mutex );
		std::cout << "mutex locked" << std::endl;
        FD_ZERO( &read );
        FD_SET( sockfd, &read );

        if ( select( sockfd+1, 0, &read, 0, &timeout ) < 0 )
        {
			std::cerr << "Error during select()" << std::endl;
            exit(1);
        }

        if ( FD_ISSET( sockfd, &read ) ) {

			unsigned char buffer[ bufferSize ];
			struct sockaddr_in fromsock;
			socklen_t fromlen=sizeof(fromsock);
			int addr;

			recvfrom( sockfd, buffer, bufferSize, 0, (struct sockaddr *)&fromsock, &fromlen);
			addr=ntohl(fromsock.sin_addr.s_addr);

			std::cout << "mutex unlocked" << std::endl;
			/** check if the xid is matching our sent out request **/
			char xid[4];
			char received_xid[4];
			memcpy( received_xid, &buffer[4], 4 );
			//if ( memcmp( xid, &buffer[4], 4 ) == 0 ) {
				printf( "%d.%d.%d.%d offered %d.%d.%d.%d\n",
						( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
						( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF,
						buffer[16], buffer[17], buffer[18], buffer[19] );
			//}
        }
		pthread_mutex_unlock( &parent->dhcp_read_mutex );
    }
}
