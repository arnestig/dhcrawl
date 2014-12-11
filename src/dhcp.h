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

#ifndef __DHCP_H__
#define __DHCP_H__

#include <vector>
#include <iostream>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <semaphore.h>

class DHCP
{
	public:
		DHCP();
		~DHCP();

		void start();
		void stop();
		void waitForData();
		void inform( std::string hardware );

		int DHCPsocket;

	private:
		void addpacket( unsigned char* pktbuf, char *buffer, int size );

		int packages;
    	struct sockaddr_in dhcp_to;
    	struct sockaddr_in name;
		sem_t semaphore;
		pthread_mutex_t mutex;
		pthread_t worker;
		int offset;
		char xid[4];
		static void *work( void *context );
};

#endif
