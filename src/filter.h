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

#ifndef __FILTER__H_
#define __FILTER__H_

#include <string>
#include <stdint.h>

#include "formatter.h"

namespace FilterType {
    enum FilterType {
        INVALID_FILTER = 0,
        IP_FILTER = 1,
        MAC_FILTER = 2,
        XID_FILTER = 3
    };
    
    inline std::string getFilterTypeName( FilterType::FilterType filterType ) {
        switch( filterType ) {
            case IP_FILTER:
                return "IP";
                break;
            case MAC_FILTER:
                return "MAC";
                break;
            case XID_FILTER:
                return "xid";
                break;
            default:
                return "None";
                break;
        }
    };
}

class Filter
{
    public:
        Filter();
        ~Filter();

        void setFilter( std::string from, std::string to );
        void setXid( uint32_t xid );
        bool matchFilter( std::string MACString, std::string IPString, uint32_t currentXid );
        void getFilterText( std::string &type, std::string &range );
        void resetFilter();

    private:
        void validateFilterType();
        FilterType::FilterType filterType;
		pthread_mutex_t mutex;
        std::string filter[ 2 ];
        uint32_t IPFilterValue[ 2 ];
        uint64_t MACFilterValue[ 2 ];
		uint32_t xid;
};

#endif

