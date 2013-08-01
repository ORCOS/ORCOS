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

#ifndef PPC405INTERRUPTHANDLER_HH_
#define PPC405INTERRUPTHANDLER_HH_

#include "powerpc.h"


/*!
 * \brief PPC405InterruptHandler, implementation of HAL InterruptHandler
 *
 * this is the hardware specific implementation of the InterruptHandler HAL class
 * for the RAPTOR board with an interrupt controller module inside the fpga.
 *
 */
class PPC405InterruptHandler {

public:

    PPC405InterruptHandler();
    ~PPC405InterruptHandler();

};

#endif /*PPC405INTERRUPTHANDLER_HH_*/
