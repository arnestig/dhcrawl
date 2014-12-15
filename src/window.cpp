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
	:	selectedPosition( 0 ),
        timeToQuit( false ),
        lastDrawMessageCount( 0 )
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
	refresh();
	endwin();
}

void Window::init()
{
	int y,x;
	getmaxyx( stdscr, y, x );

	// command window
	messageWindow = newwin( y-4, x - 2, 1, 1 );
    keypad( messageWindow, true ); // allow keypad to be used, like arrow up, down left right
    wtimeout( messageWindow, 100 ); // set wgetch timeout to 100ms

	// help window
	helpWindow = newwin( 3, x - 2, y - 3 , 1 );

	pthread_create( &worker, NULL, work, this );
	pthread_detach( worker );

}

void Window::addDHCPMessage( DHCPMessage *message )
{
	messages.push_back( message );
}

void Window::showMessage()
{
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
    DHCPInterface *dhcpInterface = Resources::Instance()->getDHCPInterface();
	switch ( c ) {
		case KEY_DOWN:
			if ( selectedPosition < messages.size() - 1 ) {
				selectedPosition++;
				if ( messages.size() > selectedPosition ) {
					curMessage = messages.at( selectedPosition );
				} else {
					curMessage = NULL;
				}
			}
		break;
		case KEY_ENTER:
		case K_ENTER:
			showMessage();
		break;
        case K_CTRL_T:
            dhcpInterface->sendDiscover( "00:23:14:8f:46:d4" );
        break;
		case KEY_UP:
			if ( selectedPosition > 0 ) {
				selectedPosition--;
				if ( messages.size() > selectedPosition ) {
					curMessage = messages.at( selectedPosition );
				} else {
					curMessage = NULL;
				}
			}
		break;
		default:
			// stop displaying that message information box?
		break;
	}
}

void Window::draw()
{
    if ( shouldRedraw() == true ) {
        lastDrawMessageCount = messages.size();
        wclear( helpWindow );
        wclear( messageWindow );
        // draw help
        mvwprintw( helpWindow, 1, 1, "Mode: %d (F5) | Filter: (F6) | Forge DHCP discovery (F7)", Resources::Instance()->getState()->getFilter() );

        // draw titles
        wattron( messageWindow, A_BOLD );
        mvwprintw( messageWindow, 1, 1, "%-20s%-11s%-15s%-18s%-18s", "MAC", "xid", "Type", "Server ID", "Offered IP" );
        wattroff( messageWindow, A_BOLD );

        // draw messages
        init_pair(1,COLOR_BLACK, COLOR_YELLOW);
        unsigned int messageIndex = 0;
        for( std::vector< DHCPMessage* >::iterator it = messages.begin(); it != messages.end(); ++it ) {
            // draw background if this is our selected message
            if ( messageIndex == selectedPosition ) {
                wattron( messageWindow, COLOR_PAIR(1) );
            } 

            // print the line containing mac, xid, type, server id, client offered ip
            mvwprintw( messageWindow, 2 + messageIndex++, 1, "%-20s%-11.8x%-15s%-18s%-18s",(*it)->getMACAddress().c_str(), (*it)->getXid(), DHCPOptions::getMessageTypeName( (*it)->getMessageType() ).c_str(), (*it)->getServerIdentifier().c_str(), (*it)->getYiaddr().c_str()  );
            wattroff( messageWindow, COLOR_PAIR(1) );
        }

        box( messageWindow, 0, 0 );
        box( helpWindow, 0, 0 );
        wnoutrefresh( messageWindow );
        wnoutrefresh( helpWindow );
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

