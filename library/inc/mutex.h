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

#ifndef MUTEX_HH_
#define MUTEX_HH_

#include "orcos.h"

#ifdef __cplusplus

class Mutex {
public:
    Mutex();
    ~Mutex();

    ErrorT acquire(int blocking = 1);

    ErrorT release();

protected:
    int counter;
};

#endif


#ifdef __cplusplus
extern "C" {
#endif

void*  mutex_create();
ErrorT  mutex_destroy(void* mutex);
ErrorT mutex_acquire(void* mutex, int blocking);
ErrorT mutex_release(void* mutex);

#ifdef __cplusplus
}
#endif


#endif /* MUTEX_HH_ */
