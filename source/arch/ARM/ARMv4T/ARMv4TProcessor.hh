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

#ifndef ARMv4TFXPROCESSOR_HH_
#define ARMv4TFXPROCESSOR_HH_

#include "inc/types.hh"
#include "inc/const.hh"

class ARMv4TProcessor {
public:
    ARMv4TProcessor();
    ~ARMv4TProcessor();

    /*****************************************************************************
     * Method: idle()
     *
     * @description
     * TODO: Puts the processor to idle mode
     *
     * enables the wait state of the arm processor. it will only be left again,
     * when an interrupt occurs.
     *******************************************************************************/
    ErrorT idle();
};

#endif /*ARMv4TPROCESSOR_HH_*/
