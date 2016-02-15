/**
    Copyright (C) 2014-2016 dhcrawl - Probe DHCPInterface servers to see what offers are sent

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

#ifndef __USERINTERFACE_H__
#define __USERINTERFACE_H__

#include <semaphore.h>
#include "dhcpmessage.h"

class UserInterface
{
    public:
        UserInterface( bool showdetails );
        virtual ~UserInterface();

        virtual void init() = 0;
        virtual void work( void *context ) = 0;
		void create();

		bool timeToQuit;
        bool showdetails;
        sem_t threadFinished;
        std::string filterText[ 2 ];
        std::string forgeText;
		std::vector< DHCPMessage* > messages;

    private:
        pthread_t worker;
        static void createWork( UserInterface *instance );
};

#endif
