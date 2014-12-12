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

#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#include "dhcpinterface.h"
#include "state.h"

class Resources
{
    public:
        static Resources* Instance();
        static void DestroyInstance();

        DHCPInterface* getDHCPInterface() const;
        State* getState() const;

    private:
        static Resources* instance;
        Resources();
        ~Resources();
        Resources( Resources const& ) {};

        DHCPInterface *dhcpInterface;
        State *state;
};

#endif
