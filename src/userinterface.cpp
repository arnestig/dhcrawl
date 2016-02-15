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

#include "userinterface.h"

UserInterface::UserInterface( bool showdetails )
    :   showdetails( showdetails )
{
}

UserInterface::~UserInterface()
{
    timeToQuit = true;
    sem_wait( &threadFinished );
    sem_destroy( &threadFinished );
}

void UserInterface::create()
{
    sem_init( &threadFinished, 0, 0 );
    filterText[ 0 ] = "";
    filterText[ 1 ] = "";
	pthread_create( &worker, NULL, (void*(*)(void*))UserInterface::createWork, this );
	pthread_detach( worker );
}

void UserInterface::createWork( UserInterface *instance )
{
    instance->init();
    instance->work( instance );
}
