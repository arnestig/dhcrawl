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

#include "resources.h"

Resources* Resources::instance = NULL;

Resources::Resources()
    :   dhcpInterface( NULL ),
		ncursesGUI( NULL ),
		textGUI( NULL )
{
    dhcpInterface = new DHCPInterface();
}

Resources::~Resources()
{
    if ( ncursesGUI != NULL ) {
        delete ncursesGUI;
    }

    if ( textGUI != NULL ) {
        delete textGUI;
    }

	delete dhcpInterface;
}

void Resources::DestroyInstance()
{
    delete instance;
    instance = NULL;
}

Resources* Resources::Instance()
{
    if ( instance == NULL ) {
        instance = new Resources();
    }
    return instance;
}

DHCPInterface* Resources::getDHCPInterface() const
{
    return dhcpInterface;
}

NCursesGUI* Resources::getNCursesGUI()
{
	if ( ncursesGUI == NULL ) {
        ncursesGUI = new NCursesGUI();
    }
    return ncursesGUI;
}

TextGUI* Resources::getTextGUI( bool showDetails )
{
	if ( textGUI == NULL ) {
        textGUI = new TextGUI( showDetails );
    }
    return textGUI;
}

