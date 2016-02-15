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

#ifndef __DHCPINTERFACE_H__
#define __DHCPINTERFACE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dhcpmessage.h"
#include "filter.h"

#include <vector>
#include <iostream>
#if defined(__UNIX__)
    #include <sys/select.h>
    #include <sys/socket.h>
    typedef int SOCKET;
    #define closesocket(i) close(i)
    #define ioctlsocket(i,l,ul ioctl(i,l,ul)
#else
    #include <WinSock2.h>
    #include <time.h>
#endif // defined(__UNIX__)

class DHCPInterface
{
	public:
		DHCPInterface();
		~DHCPInterface();

		void start();
		void stop();
		void sendDiscover( std::string hardware );
        Filter* getFilter();
        void clearMessages();
        std::vector< DHCPMessage* > getMessages();
        DHCPMessage *waitForMessage();

	private:
		static void *work( void *context );

		SOCKET DHCPInterfaceSocket[ 2 ];
		std::vector< DHCPMessage* > messages;
    	struct sockaddr_in dhcp_to;
    	struct sockaddr_in name67;
    	struct sockaddr_in name68;
        bool timeToQuit;
        Filter *filter;
        sem_t threadFinished;
        sem_t waitSemaphore;
		pthread_t worker;
		pthread_mutex_t mutex;
};

#endif
