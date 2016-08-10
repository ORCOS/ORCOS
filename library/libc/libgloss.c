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

#include <errno.h>
#include "defines.h"
#include "orcos.h"
#include "orcos_types.h"
#include <sys/types.h>
#include <sys/times.h>
//#include <sys/stat.h>


// and the defines form <errno.h> here to be not dependant on compiler include files
#define    ENOENT 2    /* No such file or directory */
#define    EBADF 9      /* Bad file number */
#define    ECHILD 10    /* No children */
#define    EAGAIN 11    /* No more processes */
#define    ENOMEM 12    /* Not enough core */
#define    EINVAL 22    /* Invalid argument */
#define    EMLINK 31    /* Too many links */
#define    EBADFD 81    /* f.d. invalid for this operation */


int _close( int file ) {
    int err = close(file);
    if (err < 0) {
        errno = err;
        return -1;
    }
    return 0;
}

int _execve( char *name, char **argv, char **env ) {
    errno = ENOMEM;
    return -1;
}

int _fork() {
    errno = EAGAIN;
    return -1;
}

//extern "C" void _abort() {}



int _fstat( int file, struct stat *st ) {
    int err = fstat(file, st);
    if (err < 0) {
        return -1;
    }
    return 0;
}

int _getpid() {
   return getpid();
}

int _isatty( int file ) {
    return 0;
}

int _kill( int pid, int sig ) {
    errno = EINVAL;
    return ( -1 );
}

int _link( char *old, char *n_e_w ) {
    errno = EMLINK;
    return -1;
}

clock_t _times(struct tms *ptms) {
    clock_t ret;
    ret = getCycles();
    ptms->tms_utime = ret;
    ptms->tms_stime = ret;
    ptms->tms_cstime = ret;
    ptms->tms_cutime = ret;
    return ret;
}

int _lseek( int file, int ptr, int dir ) {
   int err = lseek(file, ptr, dir);
   if (err < 0) {
       errno = err;
       return -1;
   }
   return err;
}

int _open( const char *name, int flags, int mode ) {
   int err = open(name, 1);
   if (err < 0) {
       errno = err;
       return -1;
   }
   return err;
}

int _read( int file, void* ptr, size_t len ) {
    //printf("_read: fd: %d, ptr: %x, len: %u\n", file, ptr, len);
    switch ( file ) {
        case 0:
            // not supported atm
            return 0;
        case 1:
        case 2:
            errno = EBADF;
            return -1;
        default: {
            int err = read(file, (char*) ptr, len);
            if (err < 0) {
                errno = err;
                return -1;
            }
            return err;
        }
    }
}

int _stat( const char *file, struct stat *st ) {
    int fd = open(file, 0);
    if ( fd < 0) {
        errno = fd;
        return -1;
    }
    int err = fstat(fd, st);
    close(fd);
    if (err < 0) {
        return -1;
    }
    return 0;
}



int _unlink( char *name ) {
    int err = remove(name);
    if (err < 0) {
       errno = err;
       return -1;
    }
    return err;
}

int _wait( int *status ) {
    errno = ECHILD;
    return -1;
}


static char stdout_buf[512];
static int  stdout_index = 0;

int writeStdOut(char* msg, int len) {
    //printToStdOut("writeStdOut:'", 13);
    //printToStdOut(msg, len);
    //printToStdOut("'\n", 2);
    int inlen = len;
    while (len > 0) {
        if (stdout_index + len < 256) {
            memcpy(&stdout_buf[stdout_index], msg, len);
            stdout_index += len;
            len = 0;
        } else {
            int remlen = 255 - stdout_index;
            memcpy(&stdout_buf[stdout_index], msg, remlen);
            stdout_buf[stdout_index+remlen+1] = '\0';
            printToStdOut(stdout_buf, 256);
            msg += remlen;
            len -= remlen;
            stdout_index = 0;
        }
    }

    if (msg[inlen-1] == '\n') {
        if (stdout_index > 0) {
            stdout_buf[stdout_index] = '\0';
            printToStdOut(stdout_buf, stdout_index);
            stdout_index = 0;
        }
    }

    return (0);
}



int _write( int file, const void* ptr, size_t len ) {
    //printf("_write: fd: %d, ptr: %x, len: %u\n", file, ptr, len);
    int err;
    switch ( file ) {
        case 0:
            // Error writing to stdin!
            errno = EBADF;
            return (-1);
        case 2:
            // stderr is not supported at the moment. writing to stdout instead
        case 1:
            // write to stdout
            err = writeStdOut((char*) ptr, len);
            if (err < 0) {
                return -1;
            }
            return len;
        default:
            err = write(file, ptr, len);
            if (err < 0) {
                errno = err;
                return -1;
            }
            return err;
    }
}

void _exit() __attribute__((noreturn));

void _exit() {
    thread_exit(0);
}


extern void* _taskHeapEnd;
extern void* _taskHeapStart;

#define HEAP_START ((caddr_t )&_taskHeapStart + 256)

/* current highest virtual address of the task */
static caddr_t current_brk     = HEAP_START;
static caddr_t lastmapped_addr = (caddr_t) &_taskHeapEnd;

int shm_map(const char* file,unint4* mapped_address, unint4* mapped_size, unint4 flags, unint4 offset);

caddr_t _sbrk( int incr ) {
  if (incr == 0) {
      return (current_brk);
  }

  caddr_t ret = current_brk;

  if (incr > 0) {
      if (current_brk + incr < lastmapped_addr) {
          current_brk+= incr;
      } else {
          unint4 addr = ((unint4)(lastmapped_addr + 0x100000)) & (~0xfffff);
          unint4 size = 0x100000;
          int result = shm_map("/mem/mem", &addr, &size, cAllocate, 0);
          if (result > 0) {
              lastmapped_addr += 0x100000;
              current_brk+= incr;
          } else {
              return ((caddr_t)-1);
          }
      }

  } else {
      if (current_brk + incr > HEAP_START) {
          current_brk += incr;
      } else {
          current_brk = HEAP_START;
      }
  }

  return (ret);
}
