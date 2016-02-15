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

#ifndef __NCURSES_GUI__H_
#define __NCURSES_GUI__H_

#define Y_OFFSET_SEARCH 0
#define Y_OFFSET_HELP 1
#define Y_OFFSET_COMMANDS 2

#define K_CTRL_D 4
#define K_CTRL_K 11
#define K_CTRL_T 20
#define K_ESC 27
#define K_ENTER 10
#define K_BACKSPACE 127
#define K_F5 269
#define K_F6 270

#include <string>
#include <vector>
#include <ncursesw/curses.h>
#include "userinterface.h"
#include "dhcpmessage.h"

class NCursesGUI : public UserInterface
{
    public:
        NCursesGUI();
        ~NCursesGUI();

		void init();
        void resizeScreen();
        void work( void *context );

    private:
		void handleInput( int c );
        void getNewMessages();
        bool shouldRedraw();
        void queueRedraw();
		void draw();
        void drawDetails();
        void drawFilter();
        void drawForge();
        void shutdownScreen();
        void setupScreen();

		int selectedPosition;
		unsigned int filterCursPos;
		unsigned int messageOffset;
        bool wantResize;
        bool forceDraw;
        bool showDetails;
        bool showFilter;
        bool showForge;
		unsigned int lastDrawMessageCount;

		DHCPMessage *curMessage;
		WINDOW *helpWindow;
		WINDOW *forgeWindow;
		WINDOW *titleWindow;
		WINDOW *messageWindow;
		WINDOW *detailsWindow;
		WINDOW *filterWindow;
};

#endif

