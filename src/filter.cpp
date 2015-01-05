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
#include <sstream>

#include "filter.h"

Filter::Filter()
{
    resetFilter();
	pthread_mutex_init( &mutex, NULL );
}

Filter::~Filter()
{
}

void Filter::resetFilter()
{
	pthread_mutex_lock( &mutex );
    filterType = FilterType::INVALID_FILTER;
    filter[ 0 ] = "";
    filter[ 1 ] = "";
    xid = 0;
    IPFilterValue[ 0 ] = 0;
    IPFilterValue[ 1 ] = 0;
    MACFilterValue[ 0 ] = 0;
    MACFilterValue[ 1 ] = 0;
	pthread_mutex_unlock( &mutex );
}

void Filter::setFilter( std::string from, std::string to )
{
	pthread_mutex_lock( &mutex );
    filter[ 0 ] = from;
    filter[ 1 ] = to;

    if ( from.empty() == true && to.empty() == false ) {
        filter[ 0 ] = to;
    }

    if ( from.empty() == false && to.empty() == true ) {
        filter[ 1 ] = from;
    }

    for( int x = 0; x < 2; x++ ) {
        MACFilterValue[ x ] = Formatter::getMACValue( filter[ x ] );
        IPFilterValue[ x ] = Formatter::getIPv4Value( filter[ x ] );
    }
    
    xid = 0;
	pthread_mutex_unlock( &mutex );
    validateFilterType();
}

void Filter::setXid( uint32_t xid )
{
	pthread_mutex_lock( &mutex );
	this->xid = xid;
    filter[ 0 ] = "";
    filter[ 1 ] = "";
	pthread_mutex_unlock( &mutex );
    validateFilterType();
}

void Filter::validateFilterType()
{
	pthread_mutex_lock( &mutex );
    if ( xid != 0 ) {
        filterType = FilterType::XID_FILTER;
    } else if ( filter[ 0 ].empty() == false  ) {
        if ( Formatter::getMACValue( filter[ 0 ] ) != 0 ) {
            filterType = FilterType::MAC_FILTER;
        } else if ( Formatter::getIPv4Value( filter[ 0 ] ) != 0 ) {
            filterType = FilterType::IP_FILTER;
        }
    } else {
        filterType = FilterType::INVALID_FILTER;
    }

	pthread_mutex_unlock( &mutex );
}

void Filter::getFilterText( std::string &type, std::string &range )
{
	pthread_mutex_lock( &mutex );
    type = FilterType::getFilterTypeName( filterType );
    if ( filterType == FilterType::XID_FILTER ) {
        char buf[ 8 ];
        sprintf( buf, "%.8x", xid );
        range = buf;
    } else if ( filterType == FilterType::MAC_FILTER || filterType == FilterType::IP_FILTER ) {
        std::stringstream ss;
        ss << "(" << filter[ 0 ] << "-" << filter[ 1 ] << ")";
        range = ss.str();
    }
	pthread_mutex_unlock( &mutex );
}

bool Filter::matchFilter( std::string MACString, std::string IPString, uint32_t currentXid )
{
    bool retval = false;
    
	pthread_mutex_lock( &mutex );
    // check if our filter is valid, if not we don't have a filter active
    if ( filterType == FilterType::INVALID_FILTER ) {
        retval = true;
    }

    // check what filter is active and make correct comparisons
    if ( filterType == FilterType::MAC_FILTER ) {
        uint64_t curMACValue = Formatter::getMACValue( MACString );
        if ( curMACValue >= MACFilterValue[ 0 ] && curMACValue <= MACFilterValue[ 1 ] ) {
            retval = true;
        }
    } else if ( filterType == FilterType::IP_FILTER ) {
        uint32_t curIPValue = Formatter::getIPv4Value( IPString );
        if ( curIPValue >= IPFilterValue[ 0 ] && curIPValue <= IPFilterValue[ 1 ] ) {
            retval = true;
        }
    } else if ( filterType == FilterType::XID_FILTER ) {
        if ( xid == currentXid ) {
            retval = true;
        }
    }

	pthread_mutex_unlock( &mutex );

    return retval;
}

