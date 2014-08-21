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

#ifndef CALLABLEOBJECT_HH_
#define CALLABLEOBJECT_HH_

/*!
 * \brief CallableObject Class, abstract interface for callback methods, part of the HAL.
 *
 *  The purpose of this class is to provide an interface for callback methods.
 *  This is necessary e.g. for methods that want to be called
 *  after a certain period of time. Therefore classes that possess
 *  such timed methods need to be derived from this class. A timed
 *  method call can then be requested using the WorkerTask which will
 *  activate a workerthread that can be scheduled to call the method
 *  after a least that period of time.
 */
class CallableObject {
public:
    // Destructor
    virtual ~CallableObject() { }

    // The callback method that needs to be overwritten
    virtual void callbackFunc(void* param)  = 0;
};

#endif /*CALLABLEOBJECT_HH_*/
