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
#include <getopt.h>
#include <signal.h>

#include "resources.h"

sem_t exitSemaphore;
std::vector< std::string > errorLog;
static int list_hashes;
static int use_tui;
static std::string forgeMAC;

void cleanup()
{
    sem_post( &exitSemaphore );
}

void handle_signal( int signal )
{
	if ( signal == SIGINT ) {
		cleanup();
	};
}

void printVersion()
{
    std::cout << "dhcrawl version X.Y" << std::endl;
    exit( 0 );
}

void printUsage()
{
    std::cout << "Usage: dhcrawl [option(s)]" << std::endl;
    std::cout << " -f  --forge=[mac]   Forge and send a DHCP discovery packet with the supplied MAC-address" << std::endl;
    std::cout << " -h  --help          Display this information" << std::endl;
    std::cout << "     --tui           Run dhcrawl in text user interface instead of NCurses GUI" << std::endl;
    std::cout << " -v  --version       Print dhcrawl version" << std::endl;
    exit( 0 );
}

void parseArguments( int argc, char *argv[] )
{
    int c;
    while ( 1 ) {

        static struct option long_options[] = {
        { "help",       no_argument,    NULL,           'h' },
        { "version",    no_argument,    NULL,           'v' },
        { "tui",        no_argument,    &use_tui,       1 },
        { "forge",      required_argument,   NULL,       'f' },
        { 0,        0,              0,                  0 } };

        int option_index = 0;
        c = getopt_long( argc, argv, "f:hv", long_options, &option_index );
        
        // are we at the end of our options? break in that case
        if ( c == -1 ) {
            break;
        }

        switch ( c ) {
            case 0:
                if ( long_options[ option_index ].flag != 0 ) {
                    break;
                }
            case 'f':
                forgeMAC = optarg;
                break; 
            case 'v':
                printVersion();
                break;
            case 'h':
            case '?':
                printUsage();
                break;
            default:
                abort();
        }
    }
}

int main( int argc, char *argv[] )
{
    // register SIGINT signal to our signal handler
	signal( SIGINT, handle_signal );

    // initialize our exit synchronizer semaphore
	sem_init( &exitSemaphore, 0, 0 );	

    // parse command line arguments
    parseArguments( argc, argv );

	// check that we're running as superuser privileges for binding on DHCP port
	if ( geteuid() != 0 ) {
		std::cerr << "This application needs to run with super-user privileges." << std::endl;
		exit( 1 );
	}

    // start our user interface, graphical (default) or text if supplied with --tui
	if ( use_tui == 1 ) {
        Resources::Instance()->getTextGUI();
    } else {
        Resources::Instance()->getNCursesGUI()->init();
    }

    // start our DHCP interface
	DHCPInterface *dhcpInterface = Resources::Instance()->getDHCPInterface();
    dhcpInterface->start();

    // check if forge command line was supplied, if so send a DHCP discovery with the MAC
    if ( forgeMAC.empty() == false ) {
        dhcpInterface->sendDiscover( forgeMAC );
    }

    // wait for our exit semaphore to be posted
    sem_wait( &exitSemaphore );
    sem_destroy( &exitSemaphore );

    // destroy our resources and threads
	Resources::Instance()->DestroyInstance();

    // print possible errors
    for( std::vector< std::string >::iterator it = errorLog.begin(); it != errorLog.end(); ++it ) {
        std::cerr << (*it) << std::endl;
    }

    if ( errorLog.empty() == false ) {
        exit( 1 );
    }

    return 0;
}

