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
#include "dhcpmessage.h"
#include "parser.h"

DHCPMessage::DHCPMessage( struct dhcp_t package )
	:	package( package ),
		messageType( 0 )
{
	// parse some information about the dhcp package
	int i = 0;
	uint8_t option = 0;
	while ( option != 255 ) {
		option = package.options[ i ];
		uint8_t length = package.options[ ++i ];

		switch( option ) {
			case 53: // DHCP Message Type
				messageType = package.options[ ++i ];
				break;
		}

		i += length;

		if ( ++i >= 308 ) {
			break;
		};
	}
}

DHCPMessage::~DHCPMessage()
{
}

uint8_t DHCPMessage::getMessageType()
{
	return messageType;
}

void DHCPMessage::printMessage()
{
	uint32_t recv_yiaddr = ntohl( package.yiaddr );
	printf( "    OP: %d\n", package.opcode );
	printf( " HTYPE: %d\n", package.htype );
	printf( "  HLEN: %d\n", package.hlen );
	printf( "  HOPS: %d\n", package.hops );
	printf( "   XID: %x\n", htonl( package.xid ) );
	printf( "  SECS: %d\n", htonl( package.secs ) );
	printf( " FLAGS: %d\n", htonl( package.flags ) );
	printf( "CIADDR: %d.%d.%d.%d\n", ( htonl( package.ciaddr ) >> 24 ) & 0xFF, ( htonl( package.ciaddr ) >> 16 ) & 0xFF, ( htonl( package.ciaddr ) >> 8 ) & 0xFF, ( htonl( package.ciaddr ) ) & 0xFF );
	printf( "YIADDR: %d.%d.%d.%d\n", ( htonl( package.yiaddr ) >> 24 ) & 0xFF, ( htonl( package.yiaddr ) >> 16 ) & 0xFF, ( htonl( package.yiaddr ) >> 8 ) & 0xFF, ( htonl( package.yiaddr ) ) & 0xFF );
	printf( "SIADDR: %d.%d.%d.%d\n", ( htonl( package.siaddr ) >> 24 ) & 0xFF, ( htonl( package.siaddr ) >> 16 ) & 0xFF, ( htonl( package.siaddr ) >> 8 ) & 0xFF, ( htonl( package.siaddr ) ) & 0xFF );
	printf( "GIADDR: %d.%d.%d.%d\n", ( htonl( package.giaddr ) >> 24 ) & 0xFF, ( htonl( package.giaddr ) >> 16 ) & 0xFF, ( htonl( package.giaddr ) >> 8 ) & 0xFF, ( htonl( package.giaddr ) ) & 0xFF );
	printf( "CHADDR: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n", package.chaddr[ 0 ], package.chaddr[ 1 ], package.chaddr[ 2 ], package.chaddr[ 3 ], package.chaddr[ 4 ], package.chaddr[ 5 ] );
	printf( " SNAME: %s\n", package.sname );
	printf( "  FILE: %s\n", package.file );
	Parser::dumpDHCPOptions( package.options );
}
