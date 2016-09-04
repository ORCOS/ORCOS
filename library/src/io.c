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


#include "orcos_types.h"
#include "defines.h"
#include "orcos.h"
#include "string.h"
#include "sys/timer.h"

int create(const char* filepath, int flags)
{
    return syscall(cFCreateSysCallId, filepath, flags);
}

int open(const char* filename, int blocking)
{
    return syscall(cFOpenSysCallId, filename, blocking);
}

int close(int fileid)
{
    return syscall(cFCloseSysCallId, fileid);
}

/*!
 * The fread() function shall read into the array pointed to by ptr up to nitems elements whose size is specified by size in bytes, from the stream pointed to by stream.
 */
int read(int fd, char *ptr, size_t size)
{
    return syscall(cFReadSysCallId, fd, ptr, size);
}

/*!
 * The fwrite() function shall write, from the array pointed to by ptr, up to nitems elements whose size is specified by size, to the stream pointed to by stream.
 */
int write(int fd, const void *buf, size_t count)
{
    return syscall(cFWriteSysCallId, fd, buf, count);
}

int link(const char* oldpath, const char* newpath)
{
    return syscall(cLinkSysCallId, oldpath, newpath);
}

int ioctl(int fd, int request, void* args) {

    return syscall(cIOControl, fd, request, args);
}

stdoutfn_t current_stdout = 0;

void setstdout(stdoutfn_t fn) {
    current_stdout = fn;
}

size_t printToStdOut(const void* ptr, size_t max)
{
    if (current_stdout != 0) {
        return (current_stdout((char*) ptr));
    } else {
        return syscall(cPrintToStdOut, ptr, max);
    }
}

int fstat(int fd, struct stat* stat) {
    return syscall(cFStatId, fd, stat);
}

int frename(int fd, char* newname) {
    return syscall(cRenameSyscallId, fd, newname);
}

int remove(const char* filepath) {
    // ensure the resource has been acquired
    int error = open(filepath, 1);
    if (error < 0) return (error);

    return (syscall(cFRemoveID, filepath));
}

int unlink(const char* name) {
    return (remove(name));
}

int  mkfifo(char* filepath, int bufferSize) {
    return (syscall(cMkFifoSyscallId, filepath, bufferSize));
}

int  mount(char* src_path, char* dst_path, int type) {
    return (syscall(cMountSyscallId,src_path,dst_path,type));
}


int  lseek(int fd, int offset, int whence) {
    return (syscall(cFSeekSyscallId, fd, offset, whence));
}

int   getcwd(char* buf, int buflen) {
    if (!buf) {
        return (cInvalidArgument);
    }
    if (buflen < 1) {
        return (cInvalidArgument);
    }
    return (syscall(cGetCwdSyscallId, buf, buflen));
}

int chdir(const char* path) {
    return (syscall(cChDirSyscallId, path));
}




Directory_Entry_t* readdir(int fd) {
    static char buffer[512];
    static int pos = 0;
    static int remainingBytes = 0;

    if (!remainingBytes) {
        int readb = read(fd, buffer, 512);
        /* error occurred reading */
        if (readb < 0) {
            return (0);
        }

        if (readb > (int) sizeof(Directory_Entry_t)) {
            pos = 0;
            remainingBytes = readb;
        } else {
            /* reset directory position (readb == 0)*/
            lseek(fd, 0, SEEK_SET);
            pos = 0;
            remainingBytes = 0;
            return (0);
        }
    }

    Directory_Entry_t* ret = (Directory_Entry_t*) (buffer + pos);
    remainingBytes -= sizeof(Directory_Entry_t) + ret->namelen;
    pos += sizeof(Directory_Entry_t) + ret->namelen;

    if ((remainingBytes < (int) sizeof(Directory_Entry_t)) ||
        ((Directory_Entry_t*) (buffer + pos))->reserved == 0xffff ) {
        remainingBytes = 0;
    }

    return (ret);

}

int isatty(int file) {
    return 0;
}


/* Method to be called by the thread upon execution finishing. Sets the thread to blocked mode
 * and waits for the next timer tick to occur again.*/
void timer_wait() {
    signal_wait((void*)-1,0);
}

/* Configures a timer. */
int timer_configure(int fd, orcos_timer_t* timer_conf) {
    return (ioctl(fd, TIMER_IOCTL_CONFIG, timer_conf));
}

/* Resets the timer device. Removes the configuration (period, thread).*/
int timer_reset(int fd) {
    return (ioctl(fd, TIMER_IOCTL_RESET, 0));
}
