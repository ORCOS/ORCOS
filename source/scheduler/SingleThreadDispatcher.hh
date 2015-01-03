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

#ifndef SINGLETHREADDISPATCHER_H_
#define SINGLETHREADDISPATCHER_H_

#include "db/LinkedList.hh"
#include "SCLConfig.hh"
#include  Kernel_Scheduler_hh

/*!
 * \brief Class managing the execution of a single thread.
 *
 */
class SingleThreadDispatcher {
public:
    SingleThreadDispatcher();

    ~SingleThreadDispatcher();

    /*****************************************************************************
     * Method: dispatch()
     *
     * @description
     *  Starts the single thread.
     *******************************************************************************/
    void dispatch();

    /*****************************************************************************
     * Method: getScheduler()
     *
     * @description
     *  Dummy method as this is not supported by the SingleThreadDispatcher
     *******************************************************************************/
    Kernel_SchedulerCfdCl* getScheduler() {
        return 0;
    }

    /*****************************************************************************
     * Method: sleep(Thread* thread)
     *
     * @description
     *  Dummy method as this is not supported by the SingleThreadDispatcher
     *******************************************************************************/
    void sleep(Thread* thread) {
    }

    /*****************************************************************************
     * Method: block(Thread* thread)
     *
     * @description
     *  Dummy method as this is not supported by the SingleThreadDispatcher
     *******************************************************************************/
    void block(Thread* thread) {
    }

    /*****************************************************************************
     * Method: unblock(Thread* thread)
     *
     * @description
     *  Dummy method as this is not supported by the SingleThreadDispatcher
     *******************************************************************************/
    void unblock(Thread* thread) {
    }

    /*****************************************************************************
     * Method: terminate_thread(Thread* thread)
     *
     * @description
     *  Dummy method as this is not supported by the SingleThreadDispatcher
     *******************************************************************************/
    void terminate_thread(Thread* thread) {
    }
};

#endif /*SingleCPUDispatcher_H_*/
