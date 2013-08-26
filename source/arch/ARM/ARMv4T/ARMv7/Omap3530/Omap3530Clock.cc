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

#include "Omap3530Clock.hh"
#include "inc/memio.h"
#include "OMAP3530.h"

Omap3530Clock::Omap3530Clock( const char* name ) :
    Clock( name ) {


}

Omap3530Clock::~Omap3530Clock() {
}


void Omap3530Clock::reset() {
}

unint8 Omap3530Clock::getTimeSinceStartup() {

	  // update the current Time
	  unint4 time_32khz = INW(0x48320010);

	  return ((unint8) time_32khz);

}
