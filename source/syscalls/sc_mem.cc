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

 Author: dbaldin
 */

#include "syscalls.hh"
#include Kernel_Thread_hh
#include "assemblerFunctions.hh"
#include "filesystem/SharedMemResource.hh"

#ifdef HAS_Board_HatLayerCfd

/*******************************************************************
 *                SHM_MAP  Syscall
 *******************************************************************/
#ifdef HAS_SyscallManager_shm_mapCfd
/*****************************************************************************
 * Method: sc_shm_map(intptr_t sp_int)
 *
 * @description
 *  shm_map syscall handler. Allows mapping shared memory region into the
 *  calling task. TODO: allow mapping of I/O devices.
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
int sc_shm_map(intptr_t sp_int) {
    const char* file;
    unint4* mapped_address;
    unint4* mapped_size;
    unint4  flags;
    unint4  offset;

    SYSCALLGETPARAMS5(sp_int, file, mapped_address, mapped_size, flags, offset);

    /* filename must be provided from task space */
    VALIDATE_IN_PROCESS(file);

    /* Be sure return addresses are inside the task */
    VALIDATE_IN_PROCESS(mapped_address);
    VALIDATE_IN_PROCESS(mapped_size);

    if (offset & 0xfff) {
        LOG(MEM, ERROR, "sc_shm_map() offset %u is not page aligned!", offset)
        return (cInvalidArgument);
    }

    unint4 shmsize = *mapped_size;

    /* try to find the specified resource to be mapped */
    Resource* res = theOS->getFileManager()->getResourceByNameandType(file, cSharedMem);
    if (res == 0) {
        if (!(flags & cCreate)) {
            return (cFileNotFound);
        }

        /* size 0 is invalid! */
        if (shmsize == 0) {
            return (cInvalidArgument);
        }

        /* check for bogus offset */
        if (offset >= shmsize) {
            return (cInvalidArgument);
        }

        /* strip the path prefix for new shared memory creation */
        char* shmname   = basename(file);
        int filenamelen = strlen(shmname);
        if (filenamelen > 255 || filenamelen <= 0) {
            return (cInvalidArgument);
        }

        /* allocate memory for name */
        char* name = reinterpret_cast<char*>(theOS->getMemoryManager()->alloc(filenamelen + 1));
        memcpy(name, shmname, filenamelen + 1);

        /* create the shared mem resource */
        SharedMemResource* sres = new SharedMemResource(shmsize, name, pCurrentRunningTask);
        /* check on creation failure */
        if (sres->getPhysicalStartAddress() == 0) {
            /* delete resource again */
            delete sres;
            return (cMemMappingError);
        } else {
            res = sres;
        }
    }

    /* must be a shared mem resource */
    SharedMemResource* shm_res = static_cast<SharedMemResource*>(res);
    unint4 virtual_address = *mapped_address;

    /* check for bogus offset */
    if (offset >= shm_res->getSize()) {
        return (cInvalidArgument);
    }

    /* is this shared mem resource valid? */
    if (shm_res->getPhysicalStartAddress() == (unint4)-1)
        return (cInvalidResource);

    unint4 mapping_size = shmsize;

    /* map it into the address space of the calling task */
    int retval = shm_res->mapIntoTask(pCurrentRunningTask, virtual_address, offset, mapping_size, flags);
    if (retval == cOk) {
        /* set return addresses */
        *mapped_address = virtual_address;
        *mapped_size    = mapping_size;

        pCurrentRunningTask->acquireResource(shm_res, pCurrentRunningThread, false);
        return (shm_res->getId());
    }

    return (retval);
}

#endif

#else
/*****************************************************************************
 * Method: sc_shm_map(intptr_t sp_int)
 *
 * @description
 *  Dummy if no virtual memory is available
 *******************************************************************************/
int sc_shm_map(intptr_t sp_int) {
    return (cNotImplemented);
}
#endif /* check if hat layer is available */


