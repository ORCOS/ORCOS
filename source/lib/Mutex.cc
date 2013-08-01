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

#include "Mutex.hh"

#include "orcos.hh"

Mutex::Mutex() {
	counter = 1;
}

Mutex::~Mutex() {
}

ErrorT Mutex::acquire(int blocking) {
    //TODO use test-and-set or similar

    while ( counter == 0 ) {
    	//printf("Mutex locked!\n");

    	if (!blocking)
    		return cError;

        signal_wait( (void*) this, true );
    }

   counter = 0;

   return cOk;
}

ErrorT Mutex::release() {

    counter = 1;

    signal_signal( (void*) this, true );

    return cOk;
}
