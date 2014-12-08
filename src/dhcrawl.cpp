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

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <iostream>
#include <netdb.h>

#define MAXLEN 2048

int offset;
int dhcp_socket;
char xid[4];

void addpacket( unsigned char* pktbuf, char *buffer, int size )
{
    memcpy( pktbuf+offset, buffer, size );
    offset += size;
}

void readDHCPResponse()
{
    unsigned char buffer[MAXLEN];
    struct sockaddr_in fromsock;
    socklen_t fromlen=sizeof(fromsock);
    int addr;

    recvfrom( dhcp_socket, buffer, MAXLEN, 0, (struct sockaddr *)&fromsock, &fromlen);
    addr=ntohl(fromsock.sin_addr.s_addr);

    /** check if the xid is matching our sent out request **/
    char received_xid[4];
    memcpy( received_xid, &buffer[4], 4 );
    if ( memcmp( xid, &buffer[4], 4 ) == 0 ) {
		printf( "%d.%d.%d.%d offered %d.%d.%d.%d\n",
			( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
			( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF,
			buffer[16], buffer[17], buffer[18], buffer[19] );
    }
}

void dhcp_discover( std::string hardware)
{
    char buffer[MAXLEN];
    unsigned char pktbuf[MAXLEN];
    unsigned int hw[16];
    struct sockaddr_in dhcp_to;

    dhcp_to.sin_family=AF_INET;
    dhcp_to.sin_addr.s_addr=INADDR_BROADCAST;
    dhcp_to.sin_port=htons(67);

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

    if ( sendto(dhcp_socket,pktbuf,offset,0,(struct sockaddr *)&dhcp_to,sizeof(dhcp_to)) != offset )
    {
		std::cerr << "Error during sendto()" << std::endl;
        exit(1);
    }
}

void dhcp_setup()
{
    int flag;
    struct sockaddr_in name;
    int socket_mode=1;

    name.sin_family = AF_INET;
    name.sin_port = htons(68);
    name.sin_addr.s_addr = INADDR_ANY;

    dhcp_socket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    setsockopt( dhcp_socket, SOL_SOCKET, SO_REUSEADDR, &socket_mode, sizeof( socket_mode ) ); // reuse address
    setsockopt( dhcp_socket, SOL_SOCKET, SO_BROADCAST, &socket_mode, sizeof( socket_mode ) ); // broadcast mode

    if ( bind( dhcp_socket, (struct sockaddr *)&name, sizeof( name ) ) < 0 ) {
        std::cerr << "Error during bind()" << std::endl;
        exit(1);
    }
}

int main( int argc, char *argv[] )
{
	// check that we're running as superuser privileges for binding on DHCP port
	if ( geteuid() != 0 ) {
		std::cerr << "This application needs to run with super-user privileges." << std::endl;
		exit(1);
	}

    dhcp_setup();
    dhcp_discover( "40:f4:07:dd:f8:41" );

    struct timeval timeout;
    timeout.tv_sec=1;
    timeout.tv_usec=0;
    fd_set read;
    while (1) {
        FD_ZERO(&read);
        FD_SET(dhcp_socket,&read);

        if ( select( dhcp_socket+1, &read, NULL, NULL, &timeout ) < 0 )
        {
			std::cerr << "Error during select()" << std::endl;
            exit(1);
        }

        if ( FD_ISSET( dhcp_socket, &read ) ) {
            readDHCPResponse();
        }
    }

    return 0;
}

