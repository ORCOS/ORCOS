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


#include "handle_syscalls.hh"
#include Kernel_Thread_hh
#include "assemblerFunctions.hh"
#include "filesystem/SharedMemResource.hh"

#ifdef HAS_Board_HatLayerCfd
/*******************************************************************
 *				MAP MEMORY Syscall
 *******************************************************************/
#ifdef HAS_SyscallManager_mapMemoryCfd
int mapMemory(int4 sp_int)
{
	 const char* log_start;
	 const char* phy_start;
	 size_t size;
	 int protection;

	 SYSCALLGETPARAMS4(sp_int,log_start,phy_start,size,protection);

	 // avoid due to security leak
#if 0
#ifdef HAS_Board_HatLayerCfd
     // create the vm map for the task! protection = 7 = RWX, ZoneSelect = 3
     theOS->getHatLayer()->map((void*) log_start,
                               (void*) phy_start,
                               size ,
                               protection,
                               3,
                               pCurrentRunningTask->getId(),
                               !ICACHE_ENABLE);
     return cOk;
#endif

#endif

     return cError;
}
#endif

/*******************************************************************
 *				SHM_MAP  Syscall
 *******************************************************************/
#ifdef HAS_SyscallManager_shm_mapCfd
int shm_mapSyscall(int4 sp_int) {
	const char* file;
	unint4* mapped_address;
	unint4* mapped_size;
	unint4 flags;

	SYSCALLGETPARAMS4(sp_int,file,mapped_address,mapped_size,flags);

	/* filename must be provided from task space */
	VALIDATE_IN_PROCESS(file);

	/* Be sure return addresses are inside the task */
	VALIDATE_IN_PROCESS(mapped_address);
	VALIDATE_IN_PROCESS(mapped_size);

	/* try to find the specified resource */
	Resource* res = theOS->getFileManager()->getResourceByNameandType(file,cSharedMem);
	if (res == 0)
	{
	     if (!(flags & cCreate)) return (cInvalidArgument);

	     /* strip the path prefix for new shared memory creation*/
	     int filenamelen = strlen(file);
	     if (filenamelen > 256) return (cInvalidArgument);

	     /* identify index with filename start*/
	     int i;
	     for (i = filenamelen-1; i > 0; i--)
	         if (file[i] == '/') break;

	     /* allocate memory for name */
	     char* name = (char*) theOS->getMemoryManager()->alloc(filenamelen-i);
	     memset(name,0,filenamelen-i);
	     memcpy(name,&file[i+1],filenamelen-i -1);

	     // create the shared mem resource
	     SharedMemResource* sres = new SharedMemResource(*mapped_size,name,pCurrentRunningTask);
	     /* check on creation failure */
	     if (sres->getPhysicalStartAddress() == 0) {
	          /* delete resource again */
	          delete sres;
	          return (cDeviceMemoryExhausted);
	     } else
	         res = sres;
	}

	/* must be a shared mem resource */
	SharedMemResource* shm_res = (SharedMemResource*) res;
    unint4 virtual_address;

	/* is this shared mem resource valid? */
	if (shm_res->getPhysicalStartAddress() == 0) return (cInvalidResource);

	/* map it into the address space of the calling task */
	int retval = shm_res->mapIntoTask(pCurrentRunningTask,virtual_address);
	if (retval == cOk) {
        /* set return addresses */
        *mapped_address = virtual_address;
        *mapped_size = shm_res->getSize();

        pCurrentRunningTask->acquireResource(shm_res,pCurrentRunningThread,false);
        return (shm_res->getId());
	}

	return (retval);
}



#endif

#endif // check if hat layer is available

/*******************************************************************
 *				DELETE Syscall
 *******************************************************************/
#ifdef HAS_SyscallManager_deleteCfd
int deleteSyscall( int4 int_sp ) {
    void* addr;
    SYSCALLGETPARAMS1(int_sp,addr);
    int retval;

    VALIDATE_IN_PROCESS(addr);

    LOG(SYSCALLS,TRACE,"Syscall: free(%x)",addr);
    retval = (int) pCurrentRunningTask->getMemManager()->free( addr );

    if (retval != cOk) {
    	ASSERT(0);
    }

    return (retval);
}
#endif



/*******************************************************************
 *				NEW Syscall
 *******************************************************************/
#ifdef HAS_SyscallManager_newCfd
int newSyscall( int4 int_sp ) {
    size_t size;
    SYSCALLGETPARAMS1(int_sp,size);
    int retval;

    LOG(SYSCALLS,TRACE,"Syscall: Thread new called. size: %d",size);
    retval = (int) pCurrentRunningTask->getMemManager()->alloc( size, true );
    LOG(SYSCALLS,TRACE,"Syscall: assigned memory at: 0x%x",retval);

    return (retval);
}
#endif



