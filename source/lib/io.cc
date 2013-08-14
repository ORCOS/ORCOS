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


extern "C" int fcreate(const char* filename, const char* path)
{
	return syscall(cFCreateSysCallId,filename,path);
}

extern "C"int fopen(const char* filename, int blocking)
{
    return syscall(cFOpenSysCallId,filename, blocking);
}

extern "C"int fclose(int fileid)
{
    return syscall(cFCloseSysCallId,fileid);
}

/*!
 * The fread() function shall read into the array pointed to by ptr up to nitems elements whose size is specified by size in bytes, from the stream pointed to by stream.
 */
extern "C"size_t fread(void *ptr, size_t size, size_t nitems, int stream)
{
    return syscall(cFReadSysCallId, ptr, size, nitems, stream);
}

/*!
 * The fwrite() function shall write, from the array pointed to by ptr, up to nitems elements whose size is specified by size, to the stream pointed to by stream.
 */
extern "C"size_t fwrite(const void *ptr, size_t size, size_t nitems, int stream)
{
    return syscall(cFWriteSysCallId, ptr, size, nitems, stream);
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

extern "C" int	ioctl(int fd, int request, void* args) {

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


