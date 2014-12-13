/**
    Copyright (C) 2014 dhcrawl - Probe DHCPMessage servers to see what offers are sent

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

#ifndef __DHCPMESSAGE_H__
#define __DHCPMESSAGE_H__

#include <vector>
#include <iostream>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <semaphore.h>
#include "dhcpoptions.h"

struct dhcp_t
{
	uint8_t opcode;
	uint8_t htype;
	uint8_t hlen;
	uint8_t hops;
	uint32_t xid;
	uint16_t secs;
	uint16_t flags;
	uint32_t ciaddr;
	uint32_t yiaddr;
	uint32_t siaddr;
	uint32_t giaddr;
	uint8_t chaddr[ 16 ];
	char sname[ 64 ];
	char file[ 128 ];
	uint32_t magic;
	uint8_t options[ 308 ];
};

class DHCPMessage
{
	public:
		DHCPMessage( struct dhcp_t copyPackage );
		~DHCPMessage();

		void printMessage();
		uint8_t getMessageType();
		uint32_t getXid();

	private:
		std::vector< std::pair< int, std::string > > options;
		struct dhcp_t package;
		uint8_t messageType;
		
};

#endif
