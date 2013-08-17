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
#include ThreadCfd_hh
#include "assembler.h"


/*******************************************************************
 *				MAP MEMORY Syscall
 *******************************************************************/
int mapMemory(int4 sp_int)
{
	 const char* log_start;
	 const char* phy_start;
	 size_t size;
	 int protection;

	 SYSCALLGETPARAMS4(sp_int,log_start,phy_start,size,protection);

#ifdef HAS_MemoryManager_HatLayerCfd
     // create the vm map for the task! protection = 7 = RWX, ZoneSelect = 3
     theOS->getHatLayer()->map((void*) log_start,(void*) phy_start, size ,protection,3,pCurrentRunningTask->getId(), !ICACHE_ENABLE);
     return cOk;
#endif

     return cError;
}


/*******************************************************************
 *				SHM_MAP  Syscall
 *******************************************************************/
int shm_mapSyscall(int4 sp_int) {
	const char* file;
	unint4* mapped_address;
	unint4* mapped_size;

	SYSCALLGETPARAMS3(sp_int,file,mapped_address,mapped_size);

	// try to find the specified resource
	Resource* res = theOS->getFileManager()->getResourceByNameandType(file,cSharedMem);
	if (res == 0) return cInvalidArgument;

	// must be a shared mem resource
	SharedMemResource* shm_res = (SharedMemResource*) res;
	unint4 virtual_address;

	// is this shared mem resource valid?
	if (shm_res->getPhysicalStartAddress() == 0) return cInvalidResource;

	// mpa it into the address space of the calling task
	int retval = shm_res->mapIntoTask(pCurrentRunningTask,virtual_address);

	// set return addresses
	*mapped_address = virtual_address;
	*mapped_size = shm_res->getSize();

	return retval;

}

/*******************************************************************
 *				DELETE Syscall
 *******************************************************************/
#ifdef HAS_SyscallManager_deleteSyscallCfd
int deleteSyscall( int4 int_sp ) {
    void* addr;
    SYSCALLGETPARAMS1(int_sp,(void*) addr);
    int retval;

    LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: free(%x)",addr));
    retval = (int) pCurrentRunningTask->getMemManager()->free( addr );

    if (retval != cOk) {
    	ASSERT(0);
    }

    return retval;
}
#endif



/*******************************************************************
 *				NEW Syscall
 *******************************************************************/
#ifdef HAS_SyscallManager_newSyscallCfd
int newSyscall( int4 int_sp ) {
    size_t size;
    SYSCALLGETPARAMS1(int_sp,(void*) size);
    int retval;

    LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: Thread new called. size: %d",size));
    retval = (int) pCurrentRunningTask->getMemManager()->alloc( size, true );
    ASSERT(retval);
    LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: assigned memory at: 0x%x",retval));

    return retval;
}
#endif


/*******************************************************************
 *				MALLOCP Syscall
 *******************************************************************/
int mallocp(int4 sp_int) {
    size_t size;
    int protection_mode;
    SYSCALLGETPARAMS2(sp_int,(void*) size,protection_mode);
    int retval;

#if USE_AIS_MEM
	// call AIS MemManager
   	retval = (int) theOS->getBoard()->aisMemManager->allocp(size,protection_mode);
#else
    retval = (int) pCurrentRunningTask->getMemManager()->alloc( size, true );
#endif

    ASSERT(retval);

    return retval;
}
