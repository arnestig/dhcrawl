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

#ifndef __FORMATTER_H__
#define __FORMATTER_H__

#include <sstream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

namespace Formatter {
	inline std::string getIPv4Address( uint8_t data[], uint8_t length ) {
		std::stringstream ss;
		uint8_t amount = length / 4;
		while ( amount > 0 ) {
			char buf[ 20 ];
			sprintf( buf, "%d.%d.%d.%d", data[ 0 ], data[ 1 ], data[ 2 ], data[ 3 ] );
			ss << buf;
			amount--;
			if ( amount > 0 ) {
				ss << ", ";
			}
			data += 4;
		}
		return ss.str();
	};

	inline std::string getString( uint8_t data[], uint8_t length ) { 
		char buf[ length + 1 ];
		strncpy( buf, (char*)data, length );
		buf[ length ] = 0;
		return buf;
	};

	inline std::string get32BitString( uint8_t data[] ) {
		char buf[ 20 ];
		sprintf( buf, "%d", ( data[ 0 ] << 24 ) + ( data[ 1 ] << 16 ) + ( data[ 2 ] << 8 ) + data[ 3 ] );
		return buf;
	};

	inline std::string get16BitString( uint8_t data[] ) {
		char buf[ 20 ];
		sprintf( buf, "%d", ( data[ 0 ] << 8 ) + data[ 1 ] );
		return buf;
	};

	inline std::string get8BitString( uint8_t data[] ) { 
		char buf[ 20 ];
		sprintf( buf, "%d", data[ 0 ] );
		return buf;
	};

    inline uint64_t getMACValue( std::string MACString ) {
        int a, b, c, d, e, f;
        uint64_t value = 0;
        if ( sscanf( MACString.c_str(), "%2x:%2x:%2x:%2x:%2x:%2x", &a, &b, &c, &d, &e, &f ) != 6 ) {
            return value;
        }

        value = uint64_t( a ) << 40;
        value |= uint64_t( b ) << 32;
        value |= uint64_t( c ) << 24;
        value |= uint64_t( d ) << 16;
        value |= uint64_t( e ) << 8;
        value |= uint64_t( f );

        return value;
    };

    inline uint32_t getIPv4Value( std::string dottedIPv4String ) {
        int a, b, c, d;
        uint32_t value = 0;
        if ( sscanf( dottedIPv4String.c_str(), "%3d.%3d.%3d.%3d", &a, &b, &c, &d ) != 4 ) {
            return 0;
        }

        value = a << 24;
        value |= b << 16;
        value |= c << 8;
        value |= d;

        return value;
    };
}

#endif
