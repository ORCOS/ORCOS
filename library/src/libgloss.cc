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

#include "./types.h"
#include "./defines.h"


// and the defines form <errno.h> here to be not dependant on compiler include files
#define	ENOENT 2	/* No such file or directory */
#define	EBADF 9		/* Bad file number */
#define	ECHILD 10	/* No children */
#define	EAGAIN 11	/* No more processes */
#define	ENOMEM 12	/* Not enough core */
#define	EINVAL 22	/* Invalid argument */
#define	EMLINK 31	/* Too many links */
#define EBADFD 81	/* f.d. invalid for this operation */
#undef errno

extern "C" int errno;

extern "C" int syscall( int syscallnumber, ... );

char *__env[ 1 ] = { 0 };
char **environ = __env;

extern "C" int close( int file ) {
    return syscall( cFCloseSysCallId, file );
}

extern "C" int execve( char *name, char **argv, char **env ) {
    errno = ENOMEM;
    return -1;
}

extern "C" int fork() {
    errno = EAGAIN;
    return -1;
}

extern "C" void abort() {}

#include <sys/stat.h>


extern "C" int fstat( int file, struct stat *st ) {
    st->st_mode = S_IFCHR;
    return 0;
}

extern "C" int getpid() {
    int pid = 0;
   // GETPID(pid);
    return pid;
}

extern "C" int isatty( int file ) {
    return 1;
}

extern "C" int kill( int pid, int sig ) {
    errno = EINVAL;
    return ( -1 );
}

extern "C" int link( char *old, char *n_e_w ) {
    errno = EMLINK;
    return -1;
}

extern "C" int lseek( int file, int ptr, int dir ) {
    return 0;
}

extern "C" int open( const char *name, int flags, int mode ) {
    return syscall( cFOpenSysCallId, name );
}

extern "C" int read( int file, void* ptr, size_t len ) {
    switch ( file ) {
        case 0:
            // not supported atm
            return 0;
        case 1:
        case 2:
            errno = EBADF;
            return -1;
        default:
            return syscall( cFReadSysCallId, ptr, 1, len, file );
    }
}

extern "C" int stat( const char *file, struct stat *st ) {
    st->st_mode = S_IFCHR;
    return 0;
}

extern "C" int times( struct tms *buf ) {
    unint8 time;
    syscall( cGetTimeSyscallId, &time );
    return (int) time;
}

extern "C" int unlink( char *name ) {
    errno = ENOENT;
    return -1;
}

extern "C" int wait( int *status ) {
    errno = ECHILD;
    return -1;
}

extern "C" int write( int file, const void* ptr, size_t len ) {
    switch ( file ) {
        case 0:
            // Error writing to stdin!
            errno = EBADF;
            return -1;
        case 2:
            // stderr is not supported at the moment. writing to stdout instead
        case 1:
            // write to stdout
            //FIXME returns only error code
            /*return*/ syscall( cPrintToStdOut, ptr, len );
            return len;
        default:
            //FIXME returns wrong number?
            /*return*/ syscall( cFWriteSysCallId, ptr, 1, len, file );
            return len;
    }
}

extern "C" void thread_exit() __attribute__((noreturn));
extern "C" void _exit() __attribute__((noreturn));
extern "C" void _exit() {
    thread_exit();
}


#define HEAP_CHUNK_SIZE 128 * 1024

extern "C" caddr_t sbrk( int incr ) {
    extern char _end; /* Defined by the linker */
    extern char __task_heap;
    static char *heap_end;
    static int heap_chunk_size = 0;
    char *prev_heap_end = 0;
    char* ptr = 0;

    if ( &_end > &__task_heap ) {
        write( 1, "Text and heap collision\n", 24 );
        _exit();
    }

    if ( incr > 0 ) {
        if ( heap_chunk_size < incr ) {
            ptr = (char*) syscall( cNewSysCallId, HEAP_CHUNK_SIZE );

            if ( ptr <= 0 ) {
                write( 1, "Heap is full\n", 13 );
                return (caddr_t) 0;
            }

            heap_chunk_size += HEAP_CHUNK_SIZE;
        }
        else {
            heap_chunk_size -= incr;
        }
    }

    if ( heap_end == 0 ) {
        heap_end = ( ptr > 0 ) ? ptr : 0; //&__task_heap;
    }

    prev_heap_end = heap_end;
    heap_end += incr;

    return (caddr_t) prev_heap_end;
}
//char* stack_ptr;
//extern"C" caddr_t sbrk(int incr){
//    extern char _end;       /* Defined by the linker */
//    static char *heap_end;
//    char *prev_heap_end;
//
//    if (heap_end == 0) {
//        heap_end = &_end;
//    }
//    prev_heap_end = heap_end;
//
//
//    if (heap_end + incr > stack_ptr)
//    {
//        write (1, "Heap and stack collision\n", 25);
//    //abort ();
//    }
//
//    heap_end += incr;
//    return (caddr_t) prev_heap_end;
//}

