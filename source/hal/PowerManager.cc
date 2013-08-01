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

#include "PowerManager.hh"
#include <kernel/Kernel.hh>

extern "C"Kernel* theOS;

PowerManager::PowerManager() {
}

PowerManager::~PowerManager() {
}

ErrorT PowerManager::enterIdleThread() {
#ifdef HAS_Board_ProcessorCfd

#ifdef HAS_Board_WatchdogCfd
    // Disable Watchdog
    theOS->getBoard()->getWatchdog()->disable();
#endif

    // Send Processor to wait state, until interrupt
    return theOS->getBoard()->getProcessor()->idle();
#else
#error It makes no sense to configure a board without a processor.
#endif

}
