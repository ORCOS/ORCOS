/*
 * RamSharedMemoryDevice.hh
 *
 *  Created on: 27.05.2015
 *      Author: Daniel
 */

#ifndef SOURCE_FILESYSTEM_RAMSHAREDMEMORYDEVICE_HH_
#define SOURCE_FILESYSTEM_RAMSHAREDMEMORYDEVICE_HH_

#include "SharedMemResource.hh"
#include "hal/CharacterDevice.hh"
#include "Resource.hh"

/*
 * /dev/mem device implementation
 */
class RamSharedMemoryDevice: public SharedMemResource {
public:
    RamSharedMemoryDevice();

    ~RamSharedMemoryDevice();

    ErrorT mapIntoTask(Task* t, unint4& virtual_address, unint4 offset, unint4& map_size, unint4 flags);
};

#endif /* SOURCE_FILESYSTEM_RAMSHAREDMEMORYDEVICE_HH_ */
