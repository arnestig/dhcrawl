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

#ifndef __PARSER_H__
#define __PARSER_H__

namespace Parser {

	inline void dumpDHCPOptions( uint8_t options[] ) {
		int i = 0;
		uint8_t option = 0;
		while ( option != 255 ) {
			option = options[ i ];
			uint8_t length = options[ ++i ];
			printf( "Option: %d ( %d ): ", option, length );
			for ( uint8_t x = 0; x < length; x++ ) {
				printf( "%d", options[ ++i ] );
			}
			printf( "\n" );

			if ( ++i >= 308 ) {
				break;
			};
		}
	};

	inline uint8_t getDHCPMessageType( uint8_t options[] ) {
		int i = 0;
		uint8_t option = 0;
		while ( option != 255 ) {
			option = options[ i ];
			uint8_t length = options[ ++i ];
			i += length;

			// is option DHCP Message Type (53) ?
			if ( option == 53 ) {
				return options[ i ];
			}

			if ( ++i >= 308 ) {
				break;
			};
		}
		return 0; // No message type found
	};

	inline void printDHCPMessage( struct dhcp_t dhcpPackage ) {
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
		Parser::dumpDHCPOptions( dhcpPackage.options );
	};
}

#endif
