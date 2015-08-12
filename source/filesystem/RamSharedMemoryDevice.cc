/*
 * RamSharedMemoryDevice.cc
 *
 *  Created on: 27.05.2015
 *      Author: Daniel
 */

#include "RamSharedMemoryDevice.hh"
#include "process/Task.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;

RamSharedMemoryDevice::RamSharedMemoryDevice() : SharedMemResource("mem", 0x0, 0xffffffff) {
    LOG(MEM, INFO, "RamSharedMemoryDevice: /dev/mem created");
}

RamSharedMemoryDevice::~RamSharedMemoryDevice() {
}

ErrorT RamSharedMemoryDevice::mapIntoTask(Task* t, unint4& virtual_address, unint4 offset, unint4& map_size, unint4 flags) {
    if (flags & cAllocate) {
        if (t == 0) {
            return (cInvalidArgument);
        }
        if (map_size == 0) {
            return (cInvalidArgument);
        }

        if (map_size > 0x010000) {
              /* bigger than 64 KB.. cannot map this using one entry .. use a 1 MB page */
            map_size = (unint4) alignCeil((char*) map_size, 0x100000);
        } else if (map_size > 0x001000) {
            /* bigger than 4 KB.. cannot map this using one entry .. use a 64 KB page */
            map_size = (unint4) alignCeil((char*) map_size, 0x010000);
        } else {
            /* use a single 4 KB page */
            map_size = 0x001000;
        }

        TaskIdT id = t->getId();
        unint4 physical_page = (unint4) theOS->getRamManager()->alloc_physical(map_size, id);
        if (physical_page != 0) {
            return (SharedMemResource::mapIntoTask(t, virtual_address, physical_page, map_size, flags));
        } else {
            return (cHeapMemoryExhausted);
        }
    } else {
        return (SharedMemResource::mapIntoTask(t, virtual_address, offset, map_size, flags));
    }
}
