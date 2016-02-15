/**
    Copyright (C) 2014-2016 dhcrawl - Probe DHCP servers to see what offers are sent

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

#include "dhcrawl.h"
#include "resources.h"
#include "version.h"

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
    std::string version = Version::getLongVersion();
    std::cout << "dhcrawl " << version << std::endl;
    std::cout << "Copyright (C) 2014-2016  Tobias Eliasson <arnestig@gmail.com" << std::endl;
    std::cout << "Licencse GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>" << std::endl;
    std::cout << "This is free software: you are free to change and redistribute it." << std::endl;
    std::cout << "There is NO WARRANTY, to the extent permitted by law." << std::endl;
    exit( 0 );
}

void printUsage()
{
    std::cout << "Usage: dhcrawl [option(s)]" << std::endl;
    std::cout << " -d  --discover=[mac]      Send a DHCP discovery packet with the supplied MAC-address" << std::endl;
    std::cout << " -f  --filter=[from[,to]]  Filter DHCP packages on MAC or IP, from and to....." << std::endl;
    std::cout << " -h  --help                Display this information" << std::endl;
    std::cout << "     --tui                 Run dhcrawl in text user interface instead of NCurses GUI" << std::endl;
    std::cout << "     --showdetails         Print detailed packages (only when using --tui)" << std::endl;
    std::cout << " -v  --version             Print dhcrawl version" << std::endl;
    exit( 0 );
}

void parseArguments( int argc, char *argv[] )
{
    int c;
    while ( 1 ) {
        static struct option long_options[] = {
        { "help",       no_argument,        NULL,           'h' },
        { "version",    no_argument,        NULL,           'v' },
        { "tui",        no_argument,        &use_tui,       1   },
        { "discover",   required_argument,  NULL,           'd' },
        { "showdetails",no_argument,        &showdetails,   1  },
        { 0,            0,                  0,              0 } };

        int option_index = 0;
        c = getopt_long( argc, argv, "d:f:hsv", long_options, &option_index );

        // are we at the end of our options? break in that case
        if ( c == -1 ) {
            break;
        }

        int comma = 0;
        std::string filterArg;
        switch ( c ) {
            case 0:
                if ( long_options[ option_index ].flag != 0 ) {
                    break;
                }
            case 'd':
                discoverMAC = optarg;
                break;
            case 'f':
                filterArg = optarg;
                comma = filterArg.find_first_of( "," );
                if ( comma != -1 ) {
                   argFilter[ 0 ] = filterArg.substr( 0, comma );
                   argFilter[ 1 ] = filterArg.substr( comma + 1 );
                } else {
                   argFilter[ 0 ] = filterArg;
                   argFilter[ 1 ] = filterArg;
                }
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

bool haveSUPrivileges()
{
    bool retval = false;
    #if defined(__linux__)
        if ( geteuid() == 0 ) {
            retval = true;
        }
    #else
        retval = true;
    #endif // defined(__linux__)
    return retval;
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
	if ( haveSUPrivileges() == false ) {
		std::cerr << "This application needs to run with super-user privileges." << std::endl;
		exit( 1 );
	}

    // start our user interface, graphical (default) or text if supplied with --tui
	Resources::Instance()->getUserInterface( use_tui, showdetails )->init();

    // start our DHCP interface
	DHCPInterface *dhcpInterface = Resources::Instance()->getDHCPInterface();
    dhcpInterface->start();

    // check if we should add a filter to the DHCP interface, supplied from the command line
    if ( argFilter[ 0 ].empty() == false && argFilter[ 1 ].empty() == false ) {
        dhcpInterface->getFilter()->setFilter( argFilter[ 0 ], argFilter[ 1 ] );
    }

    // check if forge command line was supplied, if so send a DHCP discovery with the MAC
    if ( discoverMAC.empty() == false ) {
        dhcpInterface->sendDiscover( discoverMAC );
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

