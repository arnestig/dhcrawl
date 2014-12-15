/**
    Copyright (C) 2014 dhcrawl - Probe DHCPMessage servers to see what offers are sent

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

#ifndef __DHCPOPTIONS_H__
#define __DHCPOPTIONS_H__

namespace DHCPOptions {
	enum MessageType {
		DHCPDISCOVER = 1,
		DHCPOFFER = 2,
		DHCPREQUEST = 3,
		DHCPDECLINE = 4,
		DHCPACK = 5,
		DHCPNAK = 6,
		DHCPRELEASE = 7,
		DHCPINFORM = 8
	};
	
    inline std::string getMessageTypeName( int messageType ) {
        switch( messageType ) {
            case 0:
            default:
                return "";
                break;
            case 1:
                return "DHCPDISCOVER";
                break;
            case 2:
                return "DHCPOFFER";
                break;
            case 3:
                return "DHCPREQUEST";
                break;
            case 4:
                return "DHCPDECLINE";
                break;
            case 5:
                return "DHCPACK";
                break;
            case 6:
                return "DHCPNAK";
                break;
            case 7:
                return "DHCPRELEASE";
                break;
            case 8:
                return "DHCPINFORM";
                break;
        }
    };

    inline std::string getOptionName( int option ) {
        switch ( option ) {
            default:
                return "";
                break;
            case 1:
                return "Subnet Mask";
                break;
            case 2:
                return "Time Offset";
                break;
            case 3:
                return "Router";
                break;
            case 4:
                return "Time Server";
                break;
            case 5:
                return "Name Server";
                break;
            case 6:
                return "Domain Server";
                break;
            case 7:
                return "Log Server";
                break;
            case 8:
                return "Quotes Server";
                break;
            case 9:
                return "LPR Server";
                break;
            case 10:
                return "Impress Server";
                break;
            case 11:
                return "RLP Server";
                break;
            case 12:
                return "Hostname";
                break;
            case 13:
                return "Boot File Size";
                break;
            case 14:
                return "Merit Dump File";
                break;
            case 15:
                return "Domain Name";
                break;
            case 16:
                return "Swap Server";
                break;
            case 17:
                return "Root Path";
                break;
            case 18:
                return "Extension File";
                break;
            case 19:
                return "Forward On/Off";
                break;
            case 20:
                return "SrcRte On/Off";
                break;
            case 21:
                return "Policy Filter";
                break;
            case 22:
                return "Max DG Assembly";
                break;
            case 23:
                return "Default IP TTL";
                break;
            case 24:
                return "MTU Timeout";
                break;
            case 25:
                return "MTU Plateau";
                break;
            case 26:
                return "MTU Interface";
                break;
            case 27:
                return "MTU Subnet";
                break;
            case 28:
                return "Broadcast Address";
                break;
            case 29:
                return "Mask Discovery";
                break;
            case 30:
                return "Mask Supplier";
                break;
            case 31:
                return "Router Discovery";
                break;
            case 32:
                return "Router Request";
                break;
            case 33:
                return "Static Route";
                break;
            case 34:
                return "Trailers";
                break;
            case 35:
                return "ARP Timeout";
                break;
            case 36:
                return "Ethernet";
                break;
            case 37:
                return "Default TCP TTL";
                break;
            case 38:
                return "Keepalive Time";
                break;
            case 39:
                return "Keepalive Data";
                break;
            case 40:
                return "NIS Domain";
                break;
            case 41:
                return "NIS Servers";
                break;
            case 42:
                return "NTP Servers";
                break;
            case 43:
                return "Vendor Specific";
                break;
            case 44:
                return "NETBIOS Name Srv";
                break;
            case 45:
                return "NETBIOS Dist Srv";
                break;
            case 46:
                return "NETBIOS Node Type";
                break;
            case 47:
                return "NETBIOS Scope";
                break;
            case 48:
                return "X Window Font";
                break;
            case 49:
                return "X Window Manager";
                break;
            case 50:
                return "Address Request";
                break;
            case 51:
                return "Address Time";
                break;
            case 52:
                return "Overload";
                break;
            case 53:
                return "DHCP Msg Type";
                break;
            case 54:
                return "DHCP Server Id";
                break;
            case 55:
                return "Parameter List";
                break;
            case 56:
                return "DHCP Message";
                break;
            case 57:
                return "DHCP Max Msg Size";
                break;
            case 58:
                return "Renewal Time";
                break;
            case 59:
                return "Rebinding Time";
                break;
            case 60:
                return "Class Id";
                break;
            case 61:
                return "Client Id";
                break;
            case 62:
                return "NetWare/IP Domain";
                break;
            case 63:
                return "NetWare/IP Option";
                break;
            case 64:
                return "NIS-Domain-Name";
                break;
            case 65:
                return "NIS-Server-Addr";
                break;
            case 66:
                return "Server-Name";
                break;
            case 67:
                return "Bootfile-Name";
                break;
            case 68:
                return "Home-Agent-Addrs";
                break;
            case 69:
                return "SMTP-Server";
                break;
            case 70:
                return "POP3-Server";
                break;
            case 71:
                return "NNTP-Server";
                break;
            case 72:
                return "WWW-Server";
                break;
            case 73:
                return "Finger-Server";
                break;
            case 74:
                return "IRC-Server";
                break;
            case 75:
                return "StreetTalk-Server";
                break;
            case 76:
                return "STDA-Server";
                break;
            case 77:
                return "User-Class";
                break;
            case 78:
                return "Directory Agent";
                break;
            case 79:
                return "Service Scope";
                break;
            case 80:
                return "Rapid Commit";
                break;
            case 81:
                return "Client FQDN";
                break;
            case 82:
                return "Relay Agent Information";
                break;
            case 83:
                return "iSNS";
                break;
            case 85:
                return "NDS Servers";
                break;
            case 86:
                return "NDS Tree Name";
                break;
            case 87:
                return "NDS Context";
                break;
            case 88:
                return "BCMCS Controller Domain Name list";
                break;
            case 89:
                return "BCMCS Controller IPv4 address option";
                break;
            case 90:
                return "Authentication";
                break;
            case 91:
                return "client-last-transaction-time option";
                break;
            case 92:
                return "associated-ip option";
                break;
            case 93:
                return "Client System";
                break;
            case 94:
                return "Client NDI";
                break;
            case 95:
                return "LDAP";
                break;
            case 97:
                return "UUID/GUID";
                break;
            case 98:
                return "User-Auth";
                break;
            case 99:
                return "GEOCONF_CIVIC";
                break;
            case 100:
                return "PCode";
                break;
            case 101:
                return "TCode";
                break;
            case 112:
                return "Netinfo Address";
                break;
            case 113:
                return "Netinfo Tag";
                break;
            case 114:
                return "URL";
                break;
            case 116:
                return "Auto-Config";
                break;
            case 117:
                return "Name Service Search";
                break;
            case 118:
                return "Subnet Selection Option";
                break;
            case 119:
                return "Domain Search";
                break;
            case 120:
                return "SIP Servers DHCP Option";
                break;
            case 121:
                return "Classless Static Route Option";
                break;
            case 122:
                return "CCC";
                break;
            case 123:
                return "GeoConf Option";
                break;
            case 124:
                return "V-I Vendor Class";
                break;
            case 125:
                return "V-I Vendor-Specific Information";
                break;
            case 136:
                return "OPTION_PANA_AGENT";
                break;
            case 137:
                return "OPTION_V4_LOST";
                break;
            case 138:
                return "OPTION_CAPWAP_AC_V4";
                break;
            case 139:
                return "OPTION-IPv4_Address-MoS";
                break;
            case 140:
                return "OPTION-IPv4_FQDN-MoS";
                break;
            case 141:
                return "SIP UA Configuration Service Domains";
                break;
            case 142:
                return "OPTION-IPv4_Address-ANDSF";
                break;
            case 144:
                return "GeoLoc";
                break;
            case 145:
                return "FORCERENEW_NONCE_CAPABLE";
                break;
            case 146:
                return "RDNSS Selection";
                break;
            case 150:
                return "TFTP server address";
                break;
            case 151:
                return "status-code";
                break;
            case 152:
                return "base-time";
                break;
            case 153:
                return "start-time-of-state";
                break;
            case 154:
                return "query-start-time";
                break;
            case 155:
                return "query-end-time";
                break;
            case 156:
                return "dhcp-state";
                break;
            case 157:
                return "data-source";
                break;
            case 158:
                return "OPTION_V4_PCP_SERVER";
                break;
            case 175:
                return "Etherboot";
                break;
            case 176:
                return "IP Telephone";
                break;
            case 177:
                return "Etherboot";
                break;
            case 208:
                return "PXELINUX Magic";
                break;
            case 209:
                return "Configuration File";
                break;
            case 210:
                return "Path Prefix";
                break;
            case 211:
                return "Reboot Time";
                break;
            case 212:
                return "OPTION_6RD";
                break;
            case 213:
                return "OPTION_V4_ACCESS_DOMAIN";
                break;
            case 220:
                return "Subnet Allocation Option";
                break;
            case 221:
                return "VSS";
                break;
        }
    };
}

#endif
