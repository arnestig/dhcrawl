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

#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>

#include "window.h"
#include "resources.h"

Window::Window()
	:	selectedPosition( -1 ),
        timeToQuit( false ),
        showDetails( false ),
        lastDrawMessageCount( 0 ),
        curMessage( NULL ),
		messageOffset( 0 )
{
	initscr();
	noecho();
	start_color();
	sem_init( &threadFinished, 0, 0 );	
	pthread_mutex_init( &mutex, NULL );
}

Window::~Window()
{
    timeToQuit = true;
    sem_wait( &threadFinished );
    sem_destroy( &threadFinished );
	//pthread_mutex_destroy( &mutex, NULL );

	delwin( helpWindow );
	delwin( messageWindow );
	delwin( detailsWindow );
	delwin( titleWindow );
	refresh();
	endwin();
}

void Window::init()
{
	int y,x;
	getmaxyx( stdscr, y, x );

	// title window
	titleWindow = newwin( 3, x - 2, 1, 1 );

	// message window
	messageWindow = newwin( y-6, x - 2, 3, 1 );
    keypad( messageWindow, true ); // allow keypad to be used, like arrow up, down left right
    wtimeout( messageWindow, 100 ); // set wgetch timeout to 100ms

	// help window
	helpWindow = newwin( 3, x - 2, y - 3 , 1 );

	// details window
	detailsWindow = newwin( y-4, x / 2 - 1, 2 , x / 2  );

	pthread_create( &worker, NULL, work, this );
	pthread_detach( worker );

}

void Window::addDHCPMessage( DHCPMessage *message )
{
    pthread_mutex_lock( &mutex );
	messages.insert( messages.begin(), message );
	selectedPosition++;
    if ( curMessage == NULL ) { // assign curMessage to first message if it's NULL
        curMessage = messages.back();
    }
    pthread_mutex_unlock( &mutex );
}

void Window::queueRedraw()
{
    pthread_mutex_lock( &mutex );
    lastDrawMessageCount = 0;
    pthread_mutex_unlock( &mutex );
}

bool Window::shouldRedraw()
{
    pthread_mutex_lock( &mutex );
    bool retval = lastDrawMessageCount != messages.size();
    pthread_mutex_unlock( &mutex );
    return retval;
}

void Window::handleInput( int c )
{
    pthread_mutex_lock( &mutex );
    DHCPInterface *dhcpInterface = Resources::Instance()->getDHCPInterface();
	switch ( c ) {
		case KEY_DOWN:
			if ( selectedPosition + messageOffset < messages.size() - 1 ) {
                int windowY, windowX;
                getmaxyx( messageWindow, windowY, windowX );
                if ( selectedPosition < windowY - 2 ) {
                    selectedPosition++;
                } else {
                    messageOffset++;
                }

				if ( messages.size() > selectedPosition ) {
					curMessage = messages.at( selectedPosition );
				} else {
					curMessage = NULL;
				}
			}
            break;
        case KEY_ENTER:
        case K_ENTER:
        case K_CTRL_D: // toggle details window
            showDetails = !showDetails;
            break;
        case K_CTRL_T:
            dhcpInterface->sendDiscover( "00:23:14:8f:46:d4" );
            break;
        case KEY_UP:
            if ( selectedPosition == 0 && messageOffset > 0 ) {
                messageOffset--;
            }

            if ( selectedPosition > 0 ) {
                selectedPosition--;
            }

            if ( messages.size() > selectedPosition ) {
                curMessage = messages.at( selectedPosition );
            } else {
                curMessage = NULL;
            }
            break;
        default:
            break;
    }
    pthread_mutex_unlock( &mutex );
}

void Window::draw()
{
    if ( shouldRedraw() == true ) {
        lastDrawMessageCount = messages.size();
        wclear( titleWindow );
        wclear( messageWindow );
        wclear( helpWindow );

        // draw help
        mvwprintw( helpWindow, 1, 1, "Mode: %d (F5) | Filter: (F6) | Toggle details: (C-D) | Forge DHCP discovery (F7)", Resources::Instance()->getState()->getFilter() );
        box( helpWindow, 0, 0 );
        wnoutrefresh( helpWindow );

        // draw titles
        wattron( titleWindow, A_BOLD );
        mvwprintw( titleWindow, 1, 1, "%-20s%-11s%-15s%-18s%-18s", "MAC", "xid", "Type", "Server ID", "Requested / Offered IP" );
        wattroff( titleWindow, A_BOLD );
        box( titleWindow, 0, 0 );
        wnoutrefresh( titleWindow );

        // draw messages
        init_pair(1,COLOR_BLACK, COLOR_YELLOW);
        unsigned int messageIndex = 0;
        wborder( messageWindow, 0, 0, ' ', 0, ACS_VLINE, ACS_VLINE, 0, 0 );
        int windowX, windowY;
        getmaxyx( messageWindow, windowY, windowX );
        if ( messageOffset <= messages.size() ) {
            for( std::vector< DHCPMessage* >::iterator it = messages.begin() + messageOffset; it != messages.end() && it != messages.begin() + messageOffset + windowY - 1; ++it ) {
                // draw background if this is our selected message
                if ( messageIndex == selectedPosition ) {
                    wattron( messageWindow, COLOR_PAIR(1) );
                } 

                // print the line containing mac, xid, type, server id, client offered ip
                mvwprintw( messageWindow, messageIndex++, 1, "%-20s%-11.8x%-15s%-18s%-18s",(*it)->getMACAddress().c_str(), (*it)->getXid(), DHCPOptions::getMessageTypeName( (*it)->getMessageType() ).c_str(), (*it)->getServerIdentifier().c_str(), (*it)->getOfferedIP().c_str()  );
                wattroff( messageWindow, COLOR_PAIR(1) );
            }
        }

        wnoutrefresh( messageWindow );

        // print details window if we have asked for it
        if ( showDetails == true && curMessage != NULL ) {
            struct dhcp_t curPackage = curMessage->getPackage();
            unsigned int detailsIndex = 0;
            wclear( detailsWindow );
            mvwprintw( detailsWindow, ++detailsIndex, 1, "OP: %d   HTYPE: %d   HLEN: %d   HOPS: %d", curPackage.opcode, curPackage.htype, curPackage.hlen, curPackage.hops );
            mvwprintw( detailsWindow, ++detailsIndex, 1, "XID: %x  SECS: %d    FLAGS: %d", curMessage->getXid(), curPackage.secs, curPackage.flags );
            mvwprintw( detailsWindow, ++detailsIndex, 1, "CIADDR: %s", curMessage->getCiaddr().c_str() );
            mvwprintw( detailsWindow, ++detailsIndex, 1, "YIADDR: %s", curMessage->getYiaddr().c_str() );
            mvwprintw( detailsWindow, ++detailsIndex, 1, "SIADDR: %s", curMessage->getSiaddr().c_str() );
            mvwprintw( detailsWindow, ++detailsIndex, 1, "GIADDR: %s", curMessage->getGiaddr().c_str() );
            mvwprintw( detailsWindow, ++detailsIndex, 1, "CHADDR: %s", curMessage->getMACAddress().c_str() );
            mvwprintw( detailsWindow, ++detailsIndex, 1, " SNAME: %s.", curPackage.sname );
            mvwprintw( detailsWindow, ++detailsIndex, 1, "  FILE: %s.", curPackage.file );
            std::map< int, std::string > curOptions = curMessage->getOptions();
            for( std::map< int, std::string >::iterator it = curOptions.begin(); it != curOptions.end(); ++it ) {
                mvwprintw( detailsWindow, ++detailsIndex, 1, "%3d-%-15s: %s", (*it).first,DHCPOptions::getOptionName( (*it).first ).c_str(), (*it).second.c_str() );
				if ( (*it).first == 55 ) { // print parameter request list if we have it
        			std::vector< std::string > curParamList = curMessage->getParameterRequestList();
            		for( std::vector< std::string >::iterator pit = curParamList.begin(); pit != curParamList.end(); ++pit ) {
						mvwprintw( detailsWindow, ++detailsIndex, 1, "fixthisprint-%s", (*pit).c_str() );
					}
					
				}
            }

            box( detailsWindow, 0, 0 );
            wnoutrefresh( detailsWindow );
        }

        doupdate();
    }
}

void *Window::work( void *context )
{
    Window *parent = static_cast< Window* >( context );

    while ( parent->timeToQuit == false ) {
        int c = wgetch( parent->messageWindow );
        if ( c != ERR ) {
            parent->queueRedraw();
            parent->handleInput( c );
        }
    }

    sem_post( &parent->threadFinished );
    return 0;
}

