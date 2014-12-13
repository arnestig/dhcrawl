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

#include "dhcpinterface.h"
#include "state.h"
#include <sstream>

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
}

#endif
