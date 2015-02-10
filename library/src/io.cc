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
#include "./orcos.hh"
#include "string.hh"
#include "sys/timer.h"

extern "C" int fcreate(const char* filepath, int flags)
{
    return syscall(cFCreateSysCallId, filepath, flags);
}

extern "C" int fopen(const char* filename, int blocking)
{
    return syscall(cFOpenSysCallId, filename, blocking);
}

extern "C" int fclose(int fileid)
{
    return syscall(cFCloseSysCallId, fileid);
}

/*!
 * The fread() function shall read into the array pointed to by ptr up to nitems elements whose size is specified by size in bytes, from the stream pointed to by stream.
 */
extern "C"size_t fread(int fd, char *ptr, size_t size)
{
    return syscall(cFReadSysCallId, fd, ptr, size);
}

/*!
 * The fwrite() function shall write, from the array pointed to by ptr, up to nitems elements whose size is specified by size, to the stream pointed to by stream.
 */
extern "C"size_t fwrite(int fd, const void *buf, size_t count)
{
    return syscall(cFWriteSysCallId, fd, buf, count);
}

/*!
 * The fwrite() function writes the zero terminted string pointed to by prt to the stream referenced by stream. A maximum of max characters are written, which default
 * is 256
 */
extern "C"size_t fwriteString(const void *ptr, int stream, size_t max)
{
    size_t nitems = 1;
    size_t size;

    for (size = 0; size<max; size++) {
        if (*((char*)ptr+size) == 0x00) {
            break;
        }
    }

    return syscall(cFWriteSysCallId, ptr, size, nitems, stream);
}

extern "C" int    ioctl(int fd, int request, void* args) {

    return syscall(cIOControl,fd,request,args);
}

extern "C" size_t printToStdOut(const void* ptr,size_t max)
{
    return syscall(cPrintToStdOut,ptr,max);
}

extern "C"int fputc(short c, int stream) {
    return syscall(cFPutcSysCallId, c, stream);
}

extern "C"int fgetc(int stream) {
    return syscall(cFGetcSysCallId,stream);
}

extern "C" int fstat(int fd, stat_t* stat) {
    return syscall(cFStatId,fd,stat);
}

extern "C" int     fremove(const char* filepath) {
    // ensure the resource has been acquired
    int error = fopen(filepath,true);
    if (error < 0) return (error);

    return (syscall(cFRemoveID,filepath));
}

extern "C" int  mkdev(char* devname, int bufferSize) {
    return (syscall(cMkDevSyscallId,devname,bufferSize));
}

extern "C" int  mount(char* src_path, char* dst_path, int type) {
    return (syscall(cMountSyscallId,src_path,dst_path,type));
}


extern "C" int  fseek(int fd, int offset, int whence) {
    return (syscall(cFSeekSyscallId,fd,offset,whence));
}

extern "C" Directory_Entry_t* readdir(int fd) {
    static char buffer[300];
    static int pos = 0;
    static int remainingBytes = 0;

    if (!remainingBytes) {
        /* unbuffered reading entry by entry
         * could be speed up by reading multiple entries at once
         * */
        int read = fread(fd,buffer,300);
        /* error occurred reading */
        if (read < 0)
            return (0);
        if (read > sizeof(Directory_Entry_t)) {
            pos = 0;
            remainingBytes = read;
        }
        else {
            /* reset directory position*/
            fseek(fd,0,SEEK_SET);
            pos = 0;
            remainingBytes = 0;
            return (0);
        }
    }

    Directory_Entry_t* ret = (Directory_Entry_t*) (buffer + pos);
    remainingBytes -= sizeof(Directory_Entry_t) + ret->namelen;
    pos += sizeof(Directory_Entry_t) + ret->namelen;
    if (remainingBytes < sizeof(Directory_Entry_t))
        remainingBytes = 0;

    return (ret);

}


char* fgets (char *s, int count, int fd) {
    char* str = s;
    int   read = 0;
    while(read < count) {
        int result = fread(fd,s,1);
        if (result == 0)
            return (0);

        if (*s == '\n')
        {
            s++;
            *s = 0;
            return str;
        }
        s++;
        read++;
    }

    return str;

}


/* Method to be called by the thread upon execution finishing. Sets the thread to blocked mode
 * and waits for the next timer tick to occur again.*/
void timer_wait() {
    signal_wait((void*)-1,0);
}

/* Configures a timer. */
int timer_configure(int fd, timer_t* timer_conf) {
    return (ioctl(fd, TIMER_IOCTL_CONFIG, timer_conf));
}

/* Resets the timer device. Removes the configuration (period, thread).*/
int timer_reset(int fd) {
    return (ioctl(fd, TIMER_IOCTL_RESET, 0));
}
