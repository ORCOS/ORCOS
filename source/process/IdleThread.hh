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

#ifndef IDLETHREAD_HH_
#define IDLETHREAD_HH_

#include "hal/CallableObject.hh"

/*!
 * \brief The Idle Thread class that is executed whenever there is no thread to execute.
 *
 *
 * The run method of this class is executed whenever there is no other thread to execute. This is also the point where
 * power management can take place. Therefore this thread calls the method enterIdleThread() of the Powermanager which then
 * cares about all possible power save mechanisms.
 */
class IdleThread {
public:
    IdleThread();
    ~IdleThread();

    void run();

};

#endif /*IDLETHREAD_HH_*/
