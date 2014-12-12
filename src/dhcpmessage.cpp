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

