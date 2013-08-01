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

#ifndef CAB_HH_
#define CAB_HH_

#include "inc/types.hh"
#include "inc/const.hh"
#include "process/Task.hh"

/*!
 *  define 10 buffers inside the cab
 *  this means the cab needs
 */
#define MAX_BUF 4

/*!
 * \brief A Cyclic Asynchronous Buffer fopr Message Storage
 * \ingroup comm
 *
 * This class represents a Cyclic Asynchronous Buffer for Message Storage
 *
 * The cab contains several buffers with the following layout
 *
 * bytes |  2  |  2    | n  |
 *       |inuse|datalen|data|
 *
 * Basically the cab stores data cyclic into the buffers. If the last time data was stored in buffer i
 * the next time new data will be stored in buffer i+1 % MAX_BUF. It may happen that the data in buffer i+1 % MAX_BUF
 * has not been read by any thread yet. If so the data is lost and replaced by the new data that arrived.
 *
 * By using the cab the programmer must be sure that it is not critical to miss some data and that the order of the messages
 * may ne be preserved if the buffer is overrun (written fast than read).
 */
class CAB {
private:
    // Pointer to the start of our cab containing the buffers
    char* bufferstart;

#ifdef HAS_MemoryManager_HatLayerCfd
    char* bufferstart_physical;
#endif

    // Length of the message buffer
    int length;

    // actual buffer
    int2 actb;

    // a free buffer
    int2 free;

    // the dimension of each buffer
    int2 dim_buf;

    // The task this cab belongs to (needed for vm)
    Task* ownerTask;

public:
    CAB( char* bufferstart, int length, Task* ownerTask );
    ~CAB();

    // store a new message in the buffer
    // if successfull returns the buffer the message was placed in
    // otherwise returns an error code < 0
    ErrorT store( char* pmsg, int msglen );

    // get the most recent message from the buffer
    int2 get( char** addressof_ret_ptrtomsg, int2 &buffer );

    bool hasData() {
        if ( actb != -1 )
            return true;
        else
            return false;
    }
};

#endif /*CAB_HH_*/
