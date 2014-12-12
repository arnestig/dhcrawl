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

#include "state.h"

State::State()
	:	filter( 0 ),
		xid( 0 )
{
	pthread_mutex_init( &mutex, NULL );
}

State::~State()
{
	pthread_mutex_destroy( &mutex );
}

unsigned int State::getFilter()
{
	int filter;
	pthread_mutex_lock( &mutex );
	filter = this->filter;
	pthread_mutex_unlock( &mutex );
	return filter;
}

void State::setFilter( unsigned int filter )
{
	pthread_mutex_lock( &mutex );
	this->filter = filter;
	pthread_mutex_unlock( &mutex );
}

uint32_t State::getXid()
{
	uint32_t xid;
	pthread_mutex_lock( &mutex );
	xid = this->xid;
	pthread_mutex_unlock( &mutex );
	return xid;
}

void State::setXid( uint32_t xid )
{
	pthread_mutex_lock( &mutex );
	this->xid = xid;
	pthread_mutex_unlock( &mutex );
}

