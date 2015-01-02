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
#include "hal/BufferDevice.hh"

/*******************************************************************
 *                I/O CONTROL Syscall
 *******************************************************************/
#ifdef HAS_SyscallManager_ioctlCfd
/*****************************************************************************
 * Method: sc_ioctl(intptr_t sp_int)
 *
 * @description
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
int sc_ioctl(intptr_t int_sp) {
    ResourceIdT file_id;
    int request;
    void* args;
    Resource* res;

    SYSCALLGETPARAMS3(int_sp, file_id, request, args);
    //VALIDATE_IN_PROCESS(args);

    LOG(SYSCALLS, DEBUG, "Syscall: ioctl(%d,%d,%x)", file_id, request, args);

    res = pCurrentRunningTask->getOwnedResourceById(file_id);
    if (res != 0) {
        /* check for correct type */
        if (res->getType() & (cStreamDevice | cCommDevice | cGenericDevice | cBlockDevice | cDirectory)) {
            /* base class of all is the CharacterDevice*/
            CharacterDevice* cdev = static_cast<CharacterDevice*>(res);
            return (cdev->ioctl(request, args));
        } else {
            return (cInvalidResource );
        }
    }

    return (cInvalidResource );
}
#endif

/*******************************************************************
 *                FPUTC Syscall
 *******************************************************************/
#ifdef HAS_SyscallManager_fputcCfd
/*****************************************************************************
 * Method: sc_fputc(intptr_t sp_int)
 *
 * @description
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
int sc_fputc(intptr_t int_sp) {
    char c;
    ResourceIdT stream;
    int retval;

    SYSCALLGETPARAMS2(int_sp, c, stream);

    LOG(SYSCALLS, DEBUG, "Syscall: fputc(%d,%d)", c, stream);

    register Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById(stream);
    if (res != 0) {
        /* check if resource is char writeable device */
        if (res->getType() & (cStreamDevice | cCommDevice)) {
            retval = cOk;
            CharacterDevice* cdev = static_cast<CharacterDevice*>(res);
            cdev->writeBytes(&c, 1);

            LOG(SYSCALLS, TRACE, "Syscall: Valid Resource %d", stream);
        } else {
            retval = cResourceNotWriteable;
        }
    } else {
        retval = cResourceNotOwned;
    }

    return (retval);
}
#endif

/*******************************************************************
 *                FGETC Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_fgetcCfd
/*****************************************************************************
 * Method: sc_fgetc(intptr_t sp_int)
 *
 * @description
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
int sc_fgetc(intptr_t int_sp) {
    char c;
    ResourceIdT stream;
    int retval;

    SYSCALLGETPARAMS1(int_sp, stream);
    LOG(SYSCALLS, DEBUG, "Syscall: fgetc(%d,%d)", c, stream);

    register Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById(stream);
    if (res != 0) {
        if (res->getType() & (cStreamDevice | cCommDevice)) {
            LOG(SYSCALLS, TRACE, "Syscall: Valid Resource %d", stream);
            unint4 len = 1;
            CharacterDevice* cdev = static_cast<CharacterDevice*>(res);
            retval = cdev->readBytes(&c, len);
        } else {
            retval = cResourceNotReadable;
        }
    } else {
        retval = cResourceNotOwned;
    }

    return (retval);
}
#endif

/*******************************************************************
 *                FCREATE Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_fcreateCfd
/*****************************************************************************
 * Method: sc_fcreate(intptr_t sp_int)
 *
 * @description
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
int sc_fcreate(intptr_t int_sp) {
    char* filepath;
    int flags;
    Resource* res;

    SYSCALLGETPARAMS2(int_sp, filepath, flags);
    VALIDATE_IN_PROCESS(filepath);
    char* filename  = basename(filepath);

    if (filepath[0] == '/') {
        /* handle absolute path */
        if (strlen(filename) == 0) {
            return (cInvalidPath);
        }

        char* lpos;

        if (filename != filepath) {
            lpos = filename -1;
            lpos[0] = 0;
        }


        LOG(SYSCALLS, DEBUG, "Syscall: fcreate(%s,%s)", filepath, filename);
        res = theOS->getFileManager()->getDirectory(filepath);

        if (filename != filepath) {
            lpos[0] = '/';
        }
    } else {
        /* file or directory creation inside current working dir */
        res = theOS->getFileManager()->getDirectory(pCurrentRunningTask->getWorkingDirectory());
    }

    if (res != 0) {
        Directory* dir = static_cast<Directory*>(res);
        /* check if resource with the same name already exists..
         * we do not allow duplicate names */
        if (dir->get(filename, strlen(filename))) {
            LOG(SYSCALLS, ERROR, "Syscall: fcreate(%s) FAILED. Name already exists.", filepath);
            return (cResourceAlreadyExists );
        }

        if (flags & cTYPE_DIR) {
            res = dir->createDirectory(filename, 0);
        } else {
            res = dir->createFile(filename, flags);
        }

        if (res != 0) {
            /* acquire this resource as we have created it */
            res->acquire(pCurrentRunningThread, false);
            return (res->getId());
        } else {
            LOG(SYSCALLS, ERROR, "Syscall: fcreate(%s) FAILED. File could not be created.", filename);
            return (cError );
        }
    } else {
        LOG(SYSCALLS, ERROR, "Syscall: fcreate(%s) FAILED. Invalid path", filename);
        return (cInvalidPath );
    }
}
#endif

/*******************************************************************
 *                FGOPEN Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_fopenCfd
/*****************************************************************************
 * Method: sc_fopen(intptr_t sp_int)
 *
 * @description
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
int sc_fopen(intptr_t int_sp) {
    char* filename;
    int retval;
    Resource* res;
    int blocking;

    SYSCALLGETPARAMS2(int_sp, filename, blocking);
    VALIDATE_IN_PROCESS(filename);

    LOG(SYSCALLS, TRACE, "Syscall: fopen(%s)", filename);

    if (filename[0] == '/') {
        res = theOS->getFileManager()->getResource(filename);
    } else {
        Directory* dir = theOS->getFileManager()->getDirectory(pCurrentRunningTask->getWorkingDirectory());
        res = dir->get(filename, strlen(filename));
    }

    if (res != 0) {
        retval = pCurrentRunningTask->acquireResource(res, pCurrentRunningThread, blocking);
    } else {
        retval = cInvalidResource;
        LOG(SYSCALLS, ERROR, "Syscall: fopen(%s) FAILED", filename);
    }
    return (retval);
}
#endif

/*******************************************************************
 *                FCLOSE Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_fcloseCfd
/*****************************************************************************
 * Method: sc_fclose(intptr_t sp_int)
 *
 * @description
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
int sc_fclose(intptr_t int_sp) {
    ResourceIdT file_id;
    Resource* res;

    SYSCALLGETPARAMS1(int_sp, file_id);
    LOG(SYSCALLS, DEBUG, "Syscall: fclose(%d)", file_id);

    res = pCurrentRunningTask->getOwnedResourceById(file_id);
    if (res != 0) {
        int retval = pCurrentRunningTask->releaseResource(res, pCurrentRunningThread);

#ifdef HAS_PRIORITY
        DISABLE_IRQS(status);
        SET_RETURN_VALUE((void*)int_sp, (void*)retval);
        /* we may have unblocked a higher priority thread so we need to reschedule now! */
        theOS->getDispatcher()->dispatch();
        __builtin_unreachable();
#endif
        return (retval);
    } else {
        return (cResourceNotOwned );
    }
}
#endif

/*******************************************************************
 *                FWRITE Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_fwriteCfd
/*****************************************************************************
 * Method: sc_fwrite(intptr_t sp_int)
 *
 * @description
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
int sc_fwrite(intptr_t int_sp) {
    char *write_ptr;
    unint4 write_size;
    unint4 write_nitems;
    ResourceIdT write_stream;
    int retval = 0;

    SYSCALLGETPARAMS4(int_sp, write_ptr, write_size, write_nitems, write_stream);

    /* for performance reasons we just check the write ptr..
     range is not checked.. may thus lead to a data abort -> terminating the thread */
    VALIDATE_IN_PROCESS(write_ptr);
    //VALIDATE_IN_PROCESS((unint4)write_ptr + (write_size * write_nitems ));

    LOG(SYSCALLS, DEBUG, "Syscall: fwrite(...,%d,%d,%d)", write_size, write_nitems, write_stream);

    Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById(write_stream);
    if (res != 0) {
        LOG(SYSCALLS, TRACE, "Syscall: fwrite valid");

        if (res->getType() & (cStreamDevice | cCommDevice | cFile)) {
            char* pointer = write_ptr;
            for (unint4 i = 0; i < write_nitems; i++) {
                CharacterDevice* cdev = static_cast<CharacterDevice*>(res);
                ErrorT result = cdev->writeBytes(pointer, write_size);
                /* check if writing failed */
                if (isError(result))
                    return (result);
                pointer += result;
            }
            retval = write_nitems;
        } else {
            retval = cResourceNotWriteable;
        }
    } else {
        retval = cResourceNotOwned;
        LOG(SYSCALLS, ERROR, "Syscall: fwrite on device with id %d failed", write_stream);
    }

    return (retval);
}
#endif

/*******************************************************************
 *                FREAD Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_freadCfd
/*****************************************************************************
 * Method: sc_fread(intptr_t sp_int)
 *
 * @description
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
int sc_fread(intptr_t int_sp) {
    char *read_ptr;
    unint4 read_size;
    unint4 read_nitems;
    ResourceIdT read_stream;
    int retval = 0;

    SYSCALLGETPARAMS4(int_sp, read_ptr, read_size, read_nitems, read_stream);
    VALIDATE_IN_PROCESS(read_ptr);
    LOG(SYSCALLS, TRACE, "Syscall: fread(...,%d,%d,%d)", read_size, read_nitems, read_stream);

    Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById(read_stream);
    if (res != 0) {
        LOG(SYSCALLS, TRACE, "Syscall: fread valid. Resource: %s", res->getName());

        if (res->getType() & (cStreamDevice | cCommDevice | cFile | cDirectory)) {
            char* pointer = read_ptr;
            for (unint4 i = 0; i < read_nitems; i++) {
                CharacterDevice* cdev = static_cast<CharacterDevice*>(res);
                ErrorT result = cdev->readBytes(pointer, read_size);
                /* check if reading failed */
                if (isError(result))
                    return (result);

                pointer += read_size;
            }
            /* set the return value (bytes read) */
            retval = pointer - read_ptr;
        } else {
            retval = cResourceNotReadable;
            LOG(SYSCALLS, ERROR, "Syscall: fread failed. Not a readable device.");
        }

    } else {
        retval = cResourceNotOwned;
        LOG(SYSCALLS, ERROR, "Syscall: fread on device with id %d failed", read_stream);
    }
    return (retval);
}
#endif

#ifdef HAS_SyscallManager_fstatCfd
/*****************************************************************************
 * Method: sc_fstat(intptr_t sp_int)
 *
 * @description
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
int sc_fstat(intptr_t int_sp) {
    ResourceIdT file_id;
    stat_t* stat;
    Resource* res;

    SYSCALLGETPARAMS2(int_sp, file_id, stat);
    VALIDATE_IN_PROCESS(stat);

    LOG(SYSCALLS, DEBUG, "Syscall: fstat(%d)", file_id);

    res = pCurrentRunningTask->getOwnedResourceById(file_id);
    if (res != 0) {
        stat->st_size = 0;
        stat->st_type = res->getType();

        if (res->getType() & cFile) {
            File* file = static_cast<File*>(res);
            stat->st_size = file->getFileSize();
            stat->st_flags = file->getFlags();
        } else if (res->getType() & cDirectory) {
            Directory* dir = static_cast<Directory*>(res);
            stat->st_size = dir->getNumEntries();
        }

        return (cOk );
    }

    return (cResourceNotOwned );
}
#endif

#ifdef HAS_SyscallManager_fremoveCfd
/*****************************************************************************
 * Method: sc_fremove(intptr_t sp_int)
 *
 * @description
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
int sc_fremove(intptr_t int_sp) {
    const char* filepath;
    Resource* res;

    SYSCALLGETPARAMS1(int_sp, filepath);
    VALIDATE_IN_PROCESS(filepath);
    char* filename  = basename(filepath);

    if (filepath[0] == '/') {
        /* handle absolute path */
         if (strlen(filename) == 0) {
             return (cInvalidPath);
         }

         char* lpos;

         if (filename != filepath) {
             lpos = filename -1;
             lpos[0] = 0;
         }

        LOG(SYSCALLS, DEBUG, "Syscall: fremove(%s, %s)", filepath, filename);
        res = theOS->getFileManager()->getDirectory(filepath);
        if (filename != filepath) {
            lpos[0] = '/';
        }
    } else {
        /* file or directory creation inside current working dir */
        res = theOS->getFileManager()->getDirectory(pCurrentRunningTask->getWorkingDirectory());
    }

    /* directory found? */
    if (res == 0) {
        return (cInvalidPath );
    }

    Directory* dir = static_cast<Directory*>(res);
    Resource* res_file = dir->get(filename, strlen(filename));
    /* resource found? */
    if (res_file == 0)
        return (cInvalidPath );

    /* if this resource is owned by the current task remove it from the task */
    /* only acquired resources may be removed to avoid references to already deleted resources! */
    res = pCurrentRunningTask->getOwnedResourceById(res_file->getId());
    if (res != 0) {
        pCurrentRunningTask->releaseResource(res, pCurrentRunningThread);
        return (dir->remove(res_file));
    } else {
        return (cError );
    }
}
#endif


/*****************************************************************************
 * Method: sc_fseek(intptr_t sp_int)
 *
 * @description
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
#ifdef HAS_SyscallManager_fseekCfd
int sc_fseek(intptr_t int_sp) {
    int fd;
    int offset;
    int whence;
    Resource* res;

    SYSCALLGETPARAMS3(int_sp, fd, offset, whence);
    res = pCurrentRunningTask->getOwnedResourceById(fd);

    if (whence < 0 || whence > 2)
        return (cInvalidArgument );

    if (res != 0) {
        if (res->getType() & (cDirectory | cFile)) {
            unint4 maxsize = 0;
            if (res->getType() & cDirectory) {
                Directory* dir = static_cast<Directory*>(res);
                maxsize = dir->getNumEntries();
            } else if (res->getType() & cFile) {
                File* file = static_cast<File*>(res);
                maxsize = file->getFileSize();
            }

            CharacterDevice* cres = static_cast<CharacterDevice*>(res);
            if (whence == SEEK_SET) {
                cres->resetPosition();
                if (offset > 0) {
                    return (cres->seek(offset));
                }
                return (cOk);
            }
            if (whence == SEEK_CUR) {
                return (cres->seek(offset));
            }
            if (whence == SEEK_END) {
                offset = maxsize - offset;
                return (cres->seek(offset));
            }

            return (cInvalidArgument );

        } else {
            return (cWrongResourceType );
        }
    }

    return (cError );
}
#endif


/*****************************************************************************
 * Method: sc_mkdev(intptr_t sp_int)
 *
 * @description
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
int sc_mkdev(intptr_t int_sp) {
    char* devname;
    size_t bufferSize;
    BufferDevice* res;
    int retval;

    SYSCALLGETPARAMS2(int_sp, devname, bufferSize);
    VALIDATE_IN_PROCESS(devname);

    if (strlen(devname) == 0) {
        return (cInvalidArgument );
    }

    Directory* dir = theOS->getFileManager()->getDirectory("/dev/");
    if (dir->get(devname, strlen(devname)) != 0)
        return (cResourceAlreadyExists );

    LOG(SYSCALLS, DEBUG, "Syscall: mkdev(%s, %u)", devname, bufferSize);
    res = new BufferDevice(devname, bufferSize);
    if (!res->isValid()) {
        delete res;
        return (cError );
    }

    retval = pCurrentRunningTask->acquireResource(res, pCurrentRunningThread, false);
    return (retval);
}

/*****************************************************************************
 * Method: sc_mount(intptr_t int_sp)
 *
 * @description
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
#ifdef HAS_SyscallManager_mountCfd
int sc_mount(intptr_t int_sp) {
    char* srcpath;
    char* dstpath;
    int   type;
    SYSCALLGETPARAMS3(int_sp, srcpath, dstpath, type);
    VALIDATE_IN_PROCESS(srcpath);
    VALIDATE_IN_PROCESS(dstpath);

    if (type == cMountType_Overlay) {
        Directory* srcDir = theOS->getFileManager()->getDirectory(srcpath);
        if (srcDir == 0) {
            return (cInvalidPath);
        }

        return (theOS->getFileManager()->overlayDirectory(srcDir, dstpath));
    }

    return (cInvalidArgument);
}
#endif
