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
        timeToQuit( false )
{
	initscr();
	noecho();
	start_color();
	sem_init( &threadFinished, 0, 0 );	
}

Window::~Window()
{
    timeToQuit = true;
    sem_wait( &threadFinished );
    sem_destroy( &threadFinished );
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

void Window::handleInput( int c )
{
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
    wclear( helpWindow );
    wclear( messageWindow );
    // draw help
    mvwprintw( helpWindow, 1, 1, "Mode: %d (F5) | Filter: (F6) | Forge DHCP discovery (F7)", Resources::Instance()->getState()->getFilter() );

    // draw commands
    init_pair(1,COLOR_BLACK, COLOR_YELLOW);
    unsigned int messageIndex = 0;
    for( std::vector< DHCPMessage* >::iterator it = messages.begin(); it != messages.end(); ++it ) {
        // draw background if this is our selected command
        if ( messageIndex == selectedPosition ) {
            wattron( messageWindow, COLOR_PAIR(1) );
        } 

        mvwprintw( messageWindow, 1 + messageIndex++, 1, "%s %d %s",(*it)->getMACAddress().c_str(), (*it)->getXid(), DHCPOptions::messageTypeName[ (*it)->getMessageType() ] );
        wattroff( messageWindow, COLOR_PAIR(1) );
    }

    box( messageWindow, 0, 0 );
    box( helpWindow, 0, 0 );
    wnoutrefresh( messageWindow );
    wnoutrefresh( helpWindow );
    doupdate();
}

void *Window::work( void *context )
{
    Window *parent = static_cast< Window* >( context );

    while ( parent->timeToQuit == false ) {
        int c = wgetch( parent->messageWindow );
        parent->handleInput( c );
    }

    sem_post( &parent->threadFinished );
}

