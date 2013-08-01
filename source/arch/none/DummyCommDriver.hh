/*
 ORCOS - an Organic Reconfigurable Operating System
 Copyright (C) 2008 University of Paderborn

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DUMMYETH_HH
#define DUMMYETH_HH

#include <error.hh>
#include <types.hh>
#include <hal/CommDeviceDriver.hh>




/*!
 *  \brief DummyCommDriver driver class
 *
 */
class DummyCommDriver: public CommDeviceDriver {

public:

    //!  constructor
	DummyCommDriver( const char *name, int4 a ) {}

    //!  destructor
    ~DummyCommDriver() {};

    void recv() {};


    ErrorT lowlevel_send( char* data, int len ) {
        return cNotImplemented;
    }


    ErrorT send( packet_layer* packet, char* dest_addr, int addr_len, int2 fromProtocol_ID ) {
        return cNotImplemented;
    }


    ErrorT broadcast( packet_layer* packet, int2 fromProtocol_ID ) {
        return cNotImplemented;
    };


    ErrorT multicast( packet_layer* packet, int2 fromProtocol_ID, unint4 dest_addr ) {
        return cNotImplemented;
    };




};

#endif
