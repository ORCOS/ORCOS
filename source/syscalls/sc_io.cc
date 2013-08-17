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
 *				I/O CONTROL Syscall
 *******************************************************************/
int ioctl(int4 int_sp) {

	 int file_id;
	 int request;
	 void* args;
	 Resource* res;

	 SYSCALLGETPARAMS3(int_sp,(void*) file_id, request, args);

	 LOG(SYSCALLS,DEBUG,(SYSCALLS,DEBUG,"Syscall: ioctl(%d,%d,%x)",file_id, request, args));

	 res = pCurrentRunningTask->getOwnedResourceById( file_id );
	 if ( res != 0 ) {
		 if (res->getType() == cStreamDevice || res->getType() == cCommDevice || res->getType() == cGenericDevice || res->getType() == cBlockDevice) {
			 return ((GenericDeviceDriver*) res)->ioctl(request,args);
		 } else return cInvalidResource;
	 }
	 else return cInvalidResource;
}

/*******************************************************************
 *				FPUTC Syscall
 *******************************************************************/
#ifdef HAS_SyscallManager_fputcSyscallCfd
int fputcSyscall( int4 int_sp ) {
    short c;
    int stream;
    int retval;

    SYSCALLGETPARAMS2(int_sp, c, stream);

    LOG(SYSCALLS,DEBUG,(SYSCALLS,DEBUG,"Syscall: fputc(%d,%d)",c,stream));

    register Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById( stream );
    if ( res != 0 ) {
        // resource valid and owned
        // check if resource is a characterdevice
        if ( ( res->getType() == cStreamDevice ) || ( res->getType() == cCommDevice ) ) {
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

    return retval;
}
#endif


/*******************************************************************
 *				FGETC Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_fgetcSyscallCfd
int fgetcSyscall( int4 int_sp ) {
    char c;
    int stream;
    int retval;

    SYSCALLGETPARAMS1(int_sp,(void*) stream);

    LOG(SYSCALLS,DEBUG,(SYSCALLS,DEBUG,"Syscall: fgetc(%d,%d)",c,stream));

    register Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById( stream );
    if ( res != 0 ) {
        // resource valid and owned
        // check if resource is a characterdevice
        if ( ( res->getType() == cStreamDevice ) || ( res->getType() == cCommDevice ) ) {
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

    return retval;
}
#endif


/*******************************************************************
 *				FCREATE Syscall
 *******************************************************************/



#ifdef HAS_SyscallManager_fcreateSyscallCfd
int fcreateSyscall( int4 int_sp ) {
    char* filename;
    char* path;
    int retval;
    Resource* res;

    SYSCALLGETPARAMS2(int_sp,(void*) filename, (void*) path);
    LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: fcreate(%s,%s)",filename,path));
    res = theOS->getFileManager()->getDirectory( path );

    if ( res != 0 ) {
    	Directory* dir = (Directory*) res;
    	res = new Resource(cFile,true,filename);
    	dir->add(res);
    	return cOk;
    } else {
    	LOG(SYSCALLS,ERROR,(SYSCALLS,ERROR,"Syscall: fcreate(%s,%s) FAILED",filename,path));
    	return cInvalidResource;
    }
}
#endif


/*******************************************************************
 *				FGOPEN Syscall
 *******************************************************************/


#ifdef HAS_SyscallManager_fopenSyscallCfd
int fopenSyscall( int4 int_sp ) {
    char* filename;
    int retval;
    Resource* res;
    int blocking;

    SYSCALLGETPARAMS2(int_sp,(void*) filename,(void*) blocking);

    LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: fopen(%s)",filename));

    res = theOS->getFileManager()->getResource( filename );
    if ( res != 0 ) {
        retval = pCurrentRunningTask->aquireResource( res, pCurrentRunningThread, blocking );
    }
    else {
        retval = cInvalidResource;

        LOG(SYSCALLS,ERROR,(SYSCALLS,ERROR,"Syscall: fopen(%s) FAILED",filename));
    }
    return retval;
}
#endif



/*******************************************************************
 *				FCLOSE Syscall
 *******************************************************************/


#ifdef HAS_SyscallManager_fcloseSyscallCfd
int fcloseSyscall( int4 int_sp ) {

    int file_id;
    int retval;
    Resource* res;

    SYSCALLGETPARAMS1(int_sp,(void*) file_id);

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
        theOS->getCPUDispatcher()->dispatch( theOS->getClock()->getTimeSinceStartup() - lastCycleStamp );
#endif
        return retval;
    }
    else
        return cResourceNotOwned;
}
#endif


/*******************************************************************
 *				FWRITE Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_fwriteSyscallCfd
int fwriteSyscall( int4 int_sp ) {
    const char *write_ptr;
    unint4 write_size;
    unint4 write_nitems;
    unint4 write_stream;
    int retval;

    SYSCALLGETPARAMS4(int_sp,(void*) write_ptr,(void*) write_size, (void*) write_nitems,(void*) write_stream);

    LOG(SYSCALLS,DEBUG,(SYSCALLS,DEBUG,"Syscall: fwrite(...,%d,%d,%d)",write_size,write_nitems,write_stream));

    Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById( write_stream );
    if ( res != 0 ) {

        LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: fwrite valid"));

        // resource valid and owned
        // check if resource is a characterdevice
        if ( ( res->getType() == cStreamDevice ) || ( res->getType() == cCommDevice ) ) {
            char* pointer = (char*) write_ptr;
            for ( unint4 i = 0; i < write_nitems; i++ ) {
            	ErrorT result = ( (CharacterDeviceDriver*) res )->writeBytes( pointer, write_size );
            	// check if writing failed
            	if (isError(result)) return result;
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

    return retval;

}
#endif



/*******************************************************************
 *				FREAD Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_freadSyscallCfd
int freadSyscall( int4 int_sp ) {
    const char *read_ptr;
    unint4 read_size;
    unint4 read_nitems;
    unint4 read_stream;
    int volatile retval;

    SYSCALLGETPARAMS4(int_sp,(void*) read_ptr, (void*) read_size,(void*) read_nitems,(void*) read_stream);

    LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: fread(...,%d,%d,%d)",read_size,read_nitems,read_stream));

    Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById( read_stream );
    if ( res != 0 ) {

        LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: fread valid"));

        // resource valid and owned
        // check if resource is a characterdevice
        if ( ( res->getType() == cStreamDevice ) || ( res->getType() == cCommDevice ) || ( res->getType() == cDirectory ) || ( res->getType() == cFile )  ) {
            char* pointer = (char*) read_ptr;
            for ( unint4 i = 0; i < read_nitems; i++ ) {

                ErrorT result = ( (CharacterDeviceDriver*) res )->readBytes( pointer, read_size );
                // check if reading failed
                if (isError(result)) return result;

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
    return retval;
}
#endif

