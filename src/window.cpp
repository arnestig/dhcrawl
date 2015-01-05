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
		filterCursPos( 0 ),
		messageOffset( 0 ),
        timeToQuit( false ),
        forceDraw( true ),
        showDetails( false ),
        showFilter( false ),
        showForge( false ),
        lastDrawMessageCount( 0 ),
        curMessage( NULL )
{
	initscr();
	noecho();
	start_color();
	sem_init( &threadFinished, 0, 0 );	
    filterText[ 0 ] = "";
    filterText[ 1 ] = "";
}

Window::~Window()
{
    timeToQuit = true;
    sem_wait( &threadFinished );
    sem_destroy( &threadFinished );

	delwin( helpWindow );
	delwin( forgeWindow );
	delwin( messageWindow );
	delwin( detailsWindow );
	delwin( titleWindow );
	delwin( filterWindow );
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
    wtimeout( messageWindow, 10 ); // set wgetch timeout to 10ms

	// help window
	helpWindow = newwin( 3, x - 2, y - 3, 1 );

	// details window
	detailsWindow = newwin( y-4, x / 2 - 1, 2, x / 2 );

	// filter window
	filterWindow = newwin( 10, 65, 5, 5 );

	// forge window
	forgeWindow = newwin( 6, 50, y/2-3, x/2-25 );

	pthread_create( &worker, NULL, work, this );
	pthread_detach( worker );
}

void Window::queueRedraw()
{
    forceDraw = true;
}

bool Window::shouldRedraw()
{
    return lastDrawMessageCount != messages.size();
}

void Window::getNewMessages()
{
    unsigned int oldMessagesSize = messages.size();
    
    // get messages from DHCP interface
    messages = Resources::Instance()->getDHCPInterface()->getMessages();

    // check if we should increase our selected position in messages window
    if ( oldMessagesSize <= messages.size() ) {
        selectedPosition += messages.size() - oldMessagesSize;
    } else {
        selectedPosition = 0;
    }

    // assign curMessage to first message if it's NULL
    if ( curMessage == NULL && messages.empty() == false ) { 
        curMessage = messages.back();
    }

}

void Window::handleInput( int c )
{
    DHCPInterface *dhcpInterface = Resources::Instance()->getDHCPInterface();
    if ( showFilter == true ) { // filter window is active
        switch ( c ) {
            case KEY_DOWN:
                if ( filterCursPos < 1 ) {
                    filterCursPos++;
                }
                break;
            case KEY_UP:
                if ( filterCursPos > 0 ) {
                    filterCursPos--;
                }
                break;
            case KEY_ENTER:
            case K_ENTER:
            case K_F5:
                dhcpInterface->getFilter()->setFilter( filterText[ 0 ], filterText[ 1 ] );
                showFilter = !showFilter;
                break;
            case KEY_BACKSPACE:
            case K_BACKSPACE:
                if ( filterCursPos < 2 ) {
                    if ( filterText[ filterCursPos ].length() > 0 ) {
                        filterText[ filterCursPos ].erase( filterText[ filterCursPos ].end() - 1 );
                    }
                }
                break;
            default:
                if ( c > 31 && c < 127 && filterCursPos < 2 ) {
                    filterText[ filterCursPos ].append( (char*)(&c) );
                }
                break;

        }
    } else if ( showForge == true ) { // filter window is active
        switch ( c ) {
            case KEY_ENTER:
            case K_ENTER:
            case K_F6:
                if ( Formatter::getMACValue( forgeText ) != 0 ) {
                    dhcpInterface->sendDiscover( forgeText );
                    showForge = !showForge;
                }
                break;
            case KEY_BACKSPACE:
            case K_BACKSPACE:
                if ( forgeText.length() > 0 ) {
                    forgeText.erase( forgeText.end() - 1 );
                }
                break;
            default:
                if ( c > 31 && c < 127 ) {
                    forgeText.append( (char*)(&c) );
                }
                break;

        }
    } else { // details window or default window is active
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
            case K_F5:
                showFilter = !showFilter;
                break;
            case K_F6:
                showForge = !showForge;
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
    }
}

void Window::drawDetails()
{
    // print details window if we have asked for it
    if ( curMessage != NULL ) {
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
}

void Window::drawForge()
{
    wclear( forgeWindow );
    mvwprintw( forgeWindow, 1, 5, "-- Send forged DHCP discovery --" );
    mvwprintw( forgeWindow, 3, 2, "MAC: %s", forgeText.c_str() );
    box( forgeWindow, 0, 0 );
    wnoutrefresh( forgeWindow );
}

void Window::drawFilter()
{
    wclear( filterWindow );
    Filter tempFilter;
    tempFilter.setFilter( filterText[ 0 ], filterText[ 1 ] );
    std::string type, range;
    tempFilter.getFilterText( type, range );

    mvwprintw( filterWindow, 4, 1, "Filter: %s", type.c_str() );
    mvwprintw( filterWindow, 5, 1, "REQUEST/DISCOVER/etc filter" );
    if ( filterCursPos == 0 ) {
        mvwprintw( filterWindow, 2, 1, "  To: %s", filterText[ 1 ].c_str() );
        mvwprintw( filterWindow, 1, 1, "From: %s", filterText[ 0 ].c_str() );
    } else {
        mvwprintw( filterWindow, 1, 1, "From: %s", filterText[ 0 ].c_str() );
        mvwprintw( filterWindow, 2, 1, "  To: %s", filterText[ 1 ].c_str() );
    }
    box( filterWindow, 0, 0 );
    wnoutrefresh( filterWindow );
}

void Window::draw()
{
    if ( shouldRedraw() == true || forceDraw == true ) {
        forceDraw = false;
        lastDrawMessageCount = messages.size();
        wclear( titleWindow );
        wclear( messageWindow );
        wclear( helpWindow );

        // draw help
        std::string type, range;
        Resources::Instance()->getDHCPInterface()->getFilter()->getFilterText( type, range );
        mvwprintw( helpWindow, 1, 1, "Filter: %s (%s) (F5) | Toggle details: (C-D) | Forge DHCP discovery (F7)", type.c_str(), range.c_str() );
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
        wborder( messageWindow, 0, 0, ' ', 0, ACS_VLINE, ACS_VLINE, 0, 0 );
        int windowX, windowY;
        getmaxyx( messageWindow, windowY, windowX );

        if ( messageOffset <= messages.size() ) {
            unsigned int messageIndex = 0;
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
        
        // draw detail window if user requested it
        if ( showDetails == true ) {
            drawDetails();
        } else if ( showFilter == true ) { // draw filter window if user requested it
            drawFilter();
        } else if ( showForge == true ) { // draw forge window if user requested it
            drawForge();
        }

        doupdate();
    }
}

void *Window::work( void *context )
{
    Window *parent = static_cast< Window* >( context );

    while ( parent->timeToQuit == false ) {
        /** handle input from messageWindow ?? shoult it not be all windows? **/
        int c = wgetch( parent->messageWindow );
        if ( c != ERR ) {
            parent->queueRedraw();
            parent->handleInput( c );
        }

        // get messages from DHCP Interface
        parent->getNewMessages();

        // draw window
        parent->draw();
    }

    sem_post( &parent->threadFinished );
    return 0;
}

