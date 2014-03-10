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

#include "EmulatedClock.hh"
#include "inc/memio.h"


EmulatedClock::EmulatedClock(  ) :
    Clock( "cl0") {

high_precision_time = 0;
}

EmulatedClock::~EmulatedClock() {
}


void EmulatedClock::reset() {
}

TimeT EmulatedClock::getTimeSinceStartup() {

	  return ((TimeT) high_precision_time);
}