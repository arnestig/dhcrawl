/**
    Copyright (C) 2014-2016 dhcrawl - Probe DHCPInterface servers to see what offers are sent

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
#include "formatter.h"

DHCPMessage::DHCPMessage( struct dhcp_t copyPackage )
	:	package( copyPackage ),
		messageType( 0 )
{
	// fix all network to host ordering here first so we don't have to worry about it
	package.xid = ntohl( package.xid );
	package.secs = ntohs( package.secs );
	package.flags = ntohs( package.flags );
	package.ciaddr = ntohl( package.ciaddr );
	package.yiaddr = ntohl( package.yiaddr );
	package.siaddr = ntohl( package.siaddr );
	package.giaddr = ntohl( package.giaddr );

	// parse some information about the dhcp package
	int i = 0;
	uint8_t option = 0;
	while ( i < 308 ) {
		option = package.options[ i ];
		if ( option >= 255 ) {
			break;
		}
		uint8_t length = package.options[ ++i ];
		switch ( option ) {
			case 53: // DHCP message type
				messageType = package.options[ i + 1 ];
				options[ option ] = DHCPOptions::getMessageTypeName( package.options[ i + 1 ] );
				break;
            case 54: // Server identifier
                serverIdentifier = ( package.options[ i + 1 ] << 24 ) + ( package.options[ i + 2 ] << 16 ) + ( package.options[ i + 3 ] << 8 ) + package.options[ i + 4 ];
                options[ option ] = getServerIdentifier();
                break;
            case 55: // Parameter request list
                for ( uint8_t curParReqId = 0; curParReqId < length; curParReqId++ ) {
                    parameterRequestList.push_back( DHCPOptions::getOptionName( package.options[ i + curParReqId ] ) );
                }
                break;
			case 1:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 16:
			case 28:
			case 32:
			case 41:
			case 42:
			case 44:
			case 45:
			case 48:
			case 49:
			case 50:
			case 65:
			case 68:
			case 69:
			case 70:
			case 71:
			case 72:
			case 73:
			case 74:
			case 75:
			case 76:
			case 85:
				options[ option ] = Formatter::getIPv4Address( &package.options[ i + 1 ], length );
				break;
			case 12:
			case 14:
			case 15:
			case 17:
			case 18:
			case 40:
			case 56:
			case 60:
			case 62:
			case 64:
			case 66:
			case 67:
			case 86:
			case 87:
				options[ option ] = Formatter::getString( &package.options[ i + 1 ], length );
				break;
			case 23:
			case 37:
				options[ option ] = Formatter::get8BitString( &package.options[ i + 1 ] );
				break;
			case 13:
			case 22:
			case 26:
			case 57:
				options[ option ] = Formatter::get16BitString( &package.options[ i + 1 ] );
				break;
			case 2:
			case 24:
			case 35:
			case 38:
			case 51:
			case 58:
			case 59:
				options[ option ] = Formatter::get32BitString( &package.options[ i + 1 ] );
				break;
		}

		i += length + 1;
	}
}

DHCPMessage::~DHCPMessage()
{
}

uint8_t DHCPMessage::getMessageType()
{
	return messageType;
}

uint32_t DHCPMessage::getXid()
{
	return package.xid;
}

dhcp_t DHCPMessage::getPackage()
{
    return package;
}

std::map< int, std::string > DHCPMessage::getOptions()
{
    return options;
}

std::vector< std::string > DHCPMessage::getParameterRequestList()
{
	return parameterRequestList;
}

std::string DHCPMessage::getOfferedIP()
{
	if ( package.yiaddr > 0 ) {
		return getYiaddr();
	} else if ( package.ciaddr > 0 ) {
		return getCiaddr();
	} else if ( options.count( 50 ) ) {
		return options[ 50 ];
	} else {
		return "0.0.0.0";
	}
}

std::string DHCPMessage::getYiaddr()
{
    char buf[ 16 ];
    sprintf( buf, "%d.%d.%d.%d", ( package.yiaddr >> 24 ) & 0xFF, ( package.yiaddr >> 16 ) & 0xFF, ( package.yiaddr >> 8 ) & 0xFF, ( package.yiaddr ) & 0xFF );
    return buf;
}

std::string DHCPMessage::getSiaddr()
{
    char buf[ 16 ];
    sprintf( buf, "%d.%d.%d.%d", ( package.siaddr >> 24 ) & 0xFF, ( package.siaddr >> 16 ) & 0xFF, ( package.siaddr >> 8 ) & 0xFF, ( package.siaddr ) & 0xFF );
    return buf;
}

std::string DHCPMessage::getGiaddr()
{
    char buf[ 16 ];
    sprintf( buf, "%d.%d.%d.%d", ( package.giaddr >> 24 ) & 0xFF, ( package.giaddr >> 16 ) & 0xFF, ( package.giaddr >> 8 ) & 0xFF, ( package.giaddr ) & 0xFF );
    return buf;
}

std::string DHCPMessage::getCiaddr()
{
    char buf[ 16 ];
    sprintf( buf, "%d.%d.%d.%d", ( package.ciaddr >> 24 ) & 0xFF, ( package.ciaddr >> 16 ) & 0xFF, ( package.ciaddr >> 8 ) & 0xFF, ( package.ciaddr ) & 0xFF );
    return buf;
}

std::string DHCPMessage::getServerIdentifier()
{
    char buf[ 16 ];
    sprintf( buf, "%d.%d.%d.%d", ( serverIdentifier >> 24 ) & 0xFF, ( serverIdentifier >> 16 ) & 0xFF, ( serverIdentifier >> 8 ) & 0xFF, ( serverIdentifier ) & 0xFF );
    return buf;
}

std::string DHCPMessage::getMACAddress()
{
    char buf[ 18 ];
    sprintf( buf, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", package.chaddr[ 0 ], package.chaddr[ 1 ], package.chaddr[ 2 ], package.chaddr[ 3 ], package.chaddr[ 4 ], package.chaddr[ 5 ] );
    return buf;
}

void DHCPMessage::printMessage()
{
	printf( "    OP: %d\n", package.opcode );
	printf( " HTYPE: %d\n", package.htype );
	printf( "  HLEN: %d\n", package.hlen );
	printf( "  HOPS: %d\n", package.hops );
	printf( "   XID: %x\n", package.xid );
	printf( "  SECS: %d\n", package.secs );
	printf( " FLAGS: %d\n", package.flags );
	printf( "CIADDR: %d.%d.%d.%d\n", ( package.ciaddr >> 24 ) & 0xFF, ( package.ciaddr >> 16 ) & 0xFF, ( package.ciaddr >> 8 ) & 0xFF, ( package.ciaddr ) & 0xFF );
	printf( "YIADDR: %d.%d.%d.%d\n", ( package.yiaddr >> 24 ) & 0xFF, ( package.yiaddr >> 16 ) & 0xFF, ( package.yiaddr >> 8 ) & 0xFF, ( package.yiaddr ) & 0xFF );
	printf( "SIADDR: %d.%d.%d.%d\n", ( package.siaddr >> 24 ) & 0xFF, ( package.siaddr >> 16 ) & 0xFF, ( package.siaddr >> 8 ) & 0xFF, ( package.siaddr ) & 0xFF );
	printf( "GIADDR: %d.%d.%d.%d\n", ( package.giaddr >> 24 ) & 0xFF, ( package.giaddr >> 16 ) & 0xFF, ( package.giaddr >> 8 ) & 0xFF, ( package.giaddr ) & 0xFF );
	printf( "CHADDR: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n", package.chaddr[ 0 ], package.chaddr[ 1 ], package.chaddr[ 2 ], package.chaddr[ 3 ], package.chaddr[ 4 ], package.chaddr[ 5 ] );
	printf( " SNAME: %s\n", package.sname );
	printf( "  FILE: %s\n", package.file );
	printf( "OPTIONS\n" );
	for( std::map< int, std::string >::iterator it = options.begin(); it != options.end(); ++it ) {
		printf( "%20s (%3d): %s\n", DHCPOptions::getOptionName( (*it).first ).c_str(), (*it).first, (*it).second.c_str() );
	}
}
