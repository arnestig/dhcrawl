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

#include "resources.h"

sem_t exitSemaphore;

void cleanup()
{
	Resources::Instance()->DestroyInstance();
    sem_post( &exitSemaphore );
}

void handle_signal( int signal )
{
	if ( signal == SIGINT ) {
		cleanup();
		exit( 0 );
	};
}

int main( int argc, char *argv[] )
{
	signal( SIGINT, handle_signal );
	sem_init( &exitSemaphore, 0, 0 );	

	// check that we're running as superuser privileges for binding on DHCP port
	if ( geteuid() != 0 ) {
		std::cerr << "This application needs to run with super-user privileges." << std::endl;
		exit(1);
	}
	Resources::Instance()->getWindow()->init();
	DHCPInterface *dhcpInterface = Resources::Instance()->getDHCPInterface();
    if ( dhcpInterface->start() == false ) {
        cleanup();
        return 1;
    }

	if ( dhcpInterface->sendDiscover( "00:11:22:33:44:55" ) == false ) {
        cleanup();
        return 1;
    }

    sem_wait( &exitSemaphore );
    sem_destroy( &exitSemaphore );
    return 0;
}

