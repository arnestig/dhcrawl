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


#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>

#include "textinterface.h"
#include "resources.h"

#if defined(__UNIX__)
    #include <sys/ioctl.h>
    #include <sys/types.h>
#else

#endif

TextGUI::TextGUI( bool showdetails ) : UserInterface( showdetails )
{
}

TextGUI::~TextGUI()
{
    std::cout << "~TextGUI" << std::endl;
}

void TextGUI::init()
{

}

void TextGUI::printDetails( DHCPMessage *message )
{
    // print details window if we have asked for it
    struct dhcp_t curPackage = message->getPackage();
    printf( "OP: %d   HTYPE: %d   HLEN: %d   HOPS: %d\n", curPackage.opcode, curPackage.htype, curPackage.hlen, curPackage.hops );
    printf( "XID: %x  SECS: %d    FLAGS: %d\n", message->getXid(), curPackage.secs, curPackage.flags );
    printf( "CIADDR: %s\n", message->getCiaddr().c_str() );
    printf( "YIADDR: %s\n", message->getYiaddr().c_str() );
    printf( "SIADDR: %s\n", message->getSiaddr().c_str() );
    printf( "GIADDR: %s\n", message->getGiaddr().c_str() );
    printf( "CHADDR: %s\n", message->getMACAddress().c_str() );
    printf( " SNAME: %s.\n", curPackage.sname );
    printf( "  FILE: %s.\n", curPackage.file );
    std::map< int, std::string > curOptions = message->getOptions();
    for( std::map< int, std::string >::iterator it = curOptions.begin(); it != curOptions.end(); ++it ) {
        printf( "%3d-%-15s: %s\n", (*it).first,DHCPOptions::getOptionName( (*it).first ).c_str(), (*it).second.c_str() );
        if ( (*it).first == 55 ) { // print parameter request list if we have it
            std::vector< std::string > curParamList = message->getParameterRequestList();
            for( std::vector< std::string >::iterator pit = curParamList.begin(); pit != curParamList.end(); ++pit ) {
                printf( "fixthisprint-%s\n", (*pit).c_str() );
            }
        }
    }
    printf( "\n" );
}

void TextGUI::printMessage( DHCPMessage *message )
{
    // draw titles

    // print the line containing mac, xid, type, server id, client offered ip
    printf( "%-20s%-11.8x%-15s%-18s%-18s\n", message->getMACAddress().c_str(), message->getXid(), DHCPOptions::getMessageTypeName( message->getMessageType() ).c_str(), message->getServerIdentifier().c_str(), message->getOfferedIP().c_str()  );

    // draw detail window if user requested it
    if ( showdetails == true ) {
        printDetails( message );
    }
}

void TextGUI::work( void *context )
{
    TextGUI *parent = static_cast< TextGUI* >( context );

    // print title
    printf( "%-20s%-11s%-15s%-18s%-18s\n", "MAC", "xid", "Type", "Server ID", "Requested / Offered IP" );
    while ( parent->timeToQuit == false ) {
        // get messages from DHCP Interface
        DHCPMessage *message = Resources::Instance()->getDHCPInterface()->waitForMessage();
        if ( message != NULL ) {
            // print message
            parent->printMessage( message );
        }
    }

    sem_post( &parent->threadFinished );
}
