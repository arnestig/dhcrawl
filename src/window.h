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

#ifndef __WINDOW__H_
#define __WINDOW__H_

#define Y_OFFSET_SEARCH 0
#define Y_OFFSET_HELP 1
#define Y_OFFSET_COMMANDS 2

#define K_CTRL_T 20
#define K_ENTER 10
#define K_BACKSPACE 127

#include <string>
#include <vector>
#include <ncursesw/curses.h>

#include "dhcpmessage.h"

class Window
{
    public:
        Window();
        ~Window();

		void init();
		void addDHCPMessage( DHCPMessage* message );
		void draw();

    private:
		static void *work( void *context );
		void showMessage();
		void handleInput( int c );

		unsigned int selectedPosition;
        bool timeToQuit;
        sem_t threadFinished;
		pthread_t worker;
		std::vector< DHCPMessage* > messages;
		DHCPMessage *curMessage;
		WINDOW *helpWindow;
		WINDOW *messageWindow;
};

#endif

