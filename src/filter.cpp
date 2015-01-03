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

#include <string>

#include "filter.h"

Filter::Filter()
{
    filter[ 0 ] = "";
    filter[ 1 ] = "";
    IPFilterValue[ 0 ] = 0;
    IPFilterValue[ 1 ] = 0;
    MACFilterValue[ 0 ] = 0;
    MACFilterValue[ 1 ] = 0;
	pthread_mutex_init( &mutex, NULL );
}

Filter::~Filter()
{
}

void Filter::setFilter( std::string from, std::string to )
{
	pthread_mutex_lock( &mutex );
    filter[ 0 ] = from;
    filter[ 1 ] = to;
    MACFilterValue[ 0 ] = Formatter::getMACValue( filter[ 0 ] );
    MACFilterValue[ 1 ] = Formatter::getMACValue( filter[ 1 ] );
	pthread_mutex_unlock( &mutex );
}

void Filter::setXid( uint32_t xid )
{
	pthread_mutex_lock( &mutex );
	this->xid = xid;
	pthread_mutex_unlock( &mutex );
}

FilterType::FilterType Filter::getFilterType()
{
    if ( 0 == 1 ) {
        return FilterType::IP_FILTER;
    } else if ( 0 == 2 ) {
        return FilterType::MAC_FILTER;
    }
    return FilterType::INVALID_FILTER;
}

void Filter::getFilterText( std::string &from, std::string &to )
{
	pthread_mutex_lock( &mutex );
    from = filter[ 0 ];
    to = filter[ 1 ];
	pthread_mutex_unlock( &mutex );
}

bool Filter::isFilterActive()
{
    if ( filter[ 0 ].empty() == true || filter[ 1 ].empty() == true ) {
        return false;
    }
    return true;
}

bool Filter::matchFilter( std::string MACString, std::string IPString )
{
    bool retval = false;
    // check if our filter is active
    if ( isFilterActive() == false ) {
        return true;
    }

	pthread_mutex_lock( &mutex );
    uint64_t curMACValue = Formatter::getMACValue( MACString );
    uint32_t curIPValue = Formatter::getIPv4Value( IPString );
    if ( curMACValue >= MACFilterValue[ 0 ] && curMACValue <= MACFilterValue[ 1 ] ) {
        retval = true;
    }
	pthread_mutex_unlock( &mutex );

    return retval;
}

