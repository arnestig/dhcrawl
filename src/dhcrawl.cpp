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

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "dhcp.h"


void handle_signal( int signal )
{
	if ( signal == SIGINT ) {
		// cleanup
		exit( 0 );
	};
}

int main( int argc, char *argv[] )
{
	signal( SIGINT, handle_signal );

	// check that we're running as superuser privileges for binding on DHCP port
	if ( geteuid() != 0 ) {
		std::cerr << "This application needs to run with super-user privileges." << std::endl;
		exit(1);
	}
	DHCP *dhcpServer = new DHCP();
	dhcpServer->start();
	dhcpServer->inform( "00:23:14:8f:46:d4" );
	dhcpServer->inform( "00:23:14:8f:46:d4" );
	while( 1 ) {
		dhcpServer->waitForData();
	}
	dhcpServer->stop();
	delete dhcpServer;

    return 0;
}

