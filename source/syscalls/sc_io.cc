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
#include "assembler.h"


/*******************************************************************
 *				I/O CONTROL Syscall
 *******************************************************************/
int ioctl(int4 int_sp) {

	 ResourceIdT file_id;
	 int request;
	 void* args;
	 Resource* res;

	 SYSCALLGETPARAMS3(int_sp, file_id, request, args);
	 VALIDATE_IN_PROCESS(args);

	 LOG(SYSCALLS,DEBUG,(SYSCALLS,DEBUG,"Syscall: ioctl(%d,%d,%x)",file_id, request, args));

	 res = pCurrentRunningTask->getOwnedResourceById( file_id );
	 if ( res != 0 ) {
		 // check for correct type
		 if (res->getType() & (cStreamDevice | cCommDevice | cGenericDevice | cBlockDevice)) {
			 return (((GenericDeviceDriver*) res)->ioctl(request,args));
		 } else return (cInvalidResource);
	 }
	 else return (cInvalidResource);
}

/*******************************************************************
 *				FPUTC Syscall
 *******************************************************************/
#ifdef HAS_SyscallManager_fputcCfd
int fputcSyscall( int4 int_sp ) {
    char c;
    ResourceIdT stream;
    int retval;

    SYSCALLGETPARAMS2(int_sp, c, stream);

    LOG(SYSCALLS,DEBUG,(SYSCALLS,DEBUG,"Syscall: fputc(%d,%d)",c,stream));

    register Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById( stream );
    if ( res != 0 ) {
        // check if resource is char writeable device
        if  ( res->getType() & (cStreamDevice | cCommDevice ))  {
            // we are ok to write onto this resource
            retval = cOk;
            ( (CharacterDeviceDriver*) res )->writeByte( c );

            LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: Valid Resource %d",stream));
        }
        else
            retval = cResourceNotWriteable;

    }
    // maybe resource not owned or resource doesnt exist
    else
        retval = cResourceNotOwned;

    return (retval);
}
#endif


/*******************************************************************
 *				FGETC Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_fgetcCfd
int fgetcSyscall( int4 int_sp ) {
    char c;
    ResourceIdT stream;
    int retval;

    SYSCALLGETPARAMS1(int_sp,stream);

    LOG(SYSCALLS,DEBUG,(SYSCALLS,DEBUG,"Syscall: fgetc(%d,%d)",c,stream));

    register Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById( stream );
    if ( res != 0 ) {
        // resource valid and owned
        // check if resource is a characterdevice
    	 if  ( res->getType() & (cStreamDevice | cCommDevice ))  {
            LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: Valid Resource %d",stream));
            // we are ok to read this resource
            retval=  ((CharacterDeviceDriver*) res )->readByte( &c );
        }
        else
            retval = cResourceNotReadable;

    }
    // maybe resource not owned or resource doesnt exist
    else
        retval = cResourceNotOwned;

    return (retval);
}
#endif


/*******************************************************************
 *				FCREATE Syscall
 *******************************************************************/



#ifdef HAS_SyscallManager_fcreateCfd
int fcreateSyscall( int4 int_sp ) {
    char* filename;
    char* path;
    Resource* res;

    SYSCALLGETPARAMS2(int_sp,(void*) filename, (void*) path);

    VALIDATE_IN_PROCESS(filename);
    VALIDATE_IN_PROCESS(path);

    LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: fcreate(%s,%s)",filename,path));
    res = theOS->getFileManager()->getDirectory( path );

    if ( res != 0 ) {
    	Directory* dir = (Directory*) res;
    	res = dir->createFile(filename,0);
    	if (res != 0) {
    		res->aquire(pCurrentRunningThread,false);
    		return (res->getId());
    	}
    	return (cError);
    } else {
    	LOG(SYSCALLS,ERROR,(SYSCALLS,ERROR,"Syscall: fcreate(%s,%s) FAILED",filename,path));
    	return (cInvalidResource);
    }
}
#endif


/*******************************************************************
 *				FGOPEN Syscall
 *******************************************************************/


#ifdef HAS_SyscallManager_fopenCfd
int fopenSyscall( int4 int_sp ) {
    char* filename;
    int retval;
    Resource* res;
    int blocking;

    SYSCALLGETPARAMS2(int_sp,(void*) filename,(void*) blocking);
    VALIDATE_IN_PROCESS(filename);

    LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: fopen(%s)",filename));

    res = theOS->getFileManager()->getResource( filename );
    if ( res != 0 ) {
        retval = pCurrentRunningTask->aquireResource( res, pCurrentRunningThread, blocking );
    }
    else {
        retval = cInvalidResource;

        LOG(SYSCALLS,ERROR,(SYSCALLS,ERROR,"Syscall: fopen(%s) FAILED",filename));
    }
    return (retval);
}
#endif



/*******************************************************************
 *				FCLOSE Syscall
 *******************************************************************/


#ifdef HAS_SyscallManager_fcloseCfd
int fcloseSyscall( int4 int_sp ) {

	ResourceIdT file_id;
    int retval;
    Resource* res;

    SYSCALLGETPARAMS1(int_sp,file_id);

    LOG(SYSCALLS,DEBUG,(SYSCALLS,DEBUG,"Syscall: fclose(%d)",file_id));

    res = pCurrentRunningTask->getOwnedResourceById( file_id );
    if ( res != 0 ) {
        retval = pCurrentRunningTask->releaseResource( res, pCurrentRunningThread );

#ifdef HAS_PRIORITY

#if ENABLE_NESTED_INTERRUPTS
        // first to do is disable interrupts now
        _disableInterrupts();

        pCurrentRunningThread->executinginthandler = false;
#endif

        SET_RETURN_VALUE((void*)int_sp,(void*)retval);
        // we may have unblocked a higher priority thread so we need to reschedule now!
        theOS->getCPUDispatcher()->dispatch( (unint4) ( theOS->getClock()->getTimeSinceStartup() - lastCycleStamp) );
#endif
        return (retval);
    }
    else
        return (cResourceNotOwned);
}
#endif


/*******************************************************************
 *				FWRITE Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_fwriteCfd
int fwriteSyscall( int4 int_sp ) {
    char *write_ptr;
    unint4 write_size;
    unint4 write_nitems;
    ResourceIdT write_stream;
    int retval;

    SYSCALLGETPARAMS4(int_sp,write_ptr,write_size,write_nitems,write_stream);

    // for performance reasons we just check the write ptr..
    // range is not checked.. may thus lead to a data abort -> terminating the thread
    VALIDATE_IN_PROCESS(write_ptr);
    //VALIDATE_IN_PROCESS((unint4)write_ptr + (write_size * write_nitems ));

    LOG(SYSCALLS,DEBUG,(SYSCALLS,DEBUG,"Syscall: fwrite(...,%d,%d,%d)",write_size,write_nitems,write_stream));

    Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById( write_stream );
    if ( res != 0 ) {

        LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: fwrite valid"));

        // resource valid and owned
        // check if resource is a characterdevice
        if ( res->getType() & ( cStreamDevice | cCommDevice | cFile )) {
            char* pointer = (char*) write_ptr;
            for ( unint4 i = 0; i < write_nitems; i++ ) {
            	ErrorT result = ( (CharacterDeviceDriver*) res )->writeBytes( pointer, write_size );
            	// check if writing failed
            	if (isError(result)) return (result);
                pointer += write_size;
            }
            retval = write_nitems;
        }
        else
            retval = cResourceNotWriteable;

    }
    // maybe resource not owned or resource doesnt exist
    else {
        retval = cResourceNotOwned;
        LOG(SYSCALLS,ERROR,(SYSCALLS,ERROR,"Syscall: fwrite on device with id %d failed", write_stream));
    }

    return (retval);

}
#endif



/*******************************************************************
 *				FREAD Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_freadCfd
int freadSyscall( int4 int_sp ) {
    char *read_ptr;
    unint4 read_size;
    unint4 read_nitems;
    ResourceIdT read_stream;
    int retval;

    SYSCALLGETPARAMS4(int_sp,read_ptr, read_size, read_nitems, read_stream);

    VALIDATE_IN_PROCESS(read_ptr);

    LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: fread(...,%d,%d,%d)",read_size,read_nitems,read_stream));

    Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById( read_stream );
    if ( res != 0 ) {

        LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: fread valid"));

        // resource valid and owned
        // check if resource is a characterdevice
        if ( res->getType() & ( cStreamDevice | cCommDevice | cFile | cDirectory)) {
            char* pointer = (char*) read_ptr;
            for ( unint4 i = 0; i < read_nitems; i++ ) {

                ErrorT result = ( (CharacterDeviceDriver*) res )->readBytes( pointer, read_size );
                // check if reading failed
                if (isError(result)) return (result);

                pointer += read_size;
            }
            // set the return value (bytes read)
            retval = pointer - read_ptr;

        }
        else {
            retval = cResourceNotReadable;
            LOG(SYSCALLS,ERROR,(SYSCALLS,ERROR,"Syscall: fread failed. Not a readable device."));
        }

    }
    // maybe resource not owned or resource doesnt exist
    else {
        retval = cResourceNotOwned;
        LOG(SYSCALLS,ERROR,(SYSCALLS,ERROR,"Syscall: fread on device with id %d failed", read_stream));
    }
    return (retval);
}
#endif


int fstatSyscall( int4 int_sp ) {

	ResourceIdT file_id;
	stat_t* stat;
    Resource* res;

    SYSCALLGETPARAMS2(int_sp,file_id,stat);
    VALIDATE_IN_PROCESS(stat);

    LOG(SYSCALLS,DEBUG,(SYSCALLS,DEBUG,"Syscall: fstat(%d)",file_id));

    res = pCurrentRunningTask->getOwnedResourceById( file_id );
    if ( res != 0 ) {

    	stat->st_size = 0;
    	if (res->getType() & cFile) {
    		stat->st_size =  ((File*) res)->getFileSize();
    	}

       return (cOk);
    }

    return (cResourceNotOwned);
}

