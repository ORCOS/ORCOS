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

#include "FileLogger.hh"
#include <stdarg.h>
#include <sprintf.hh>
#include <putc.hh>
#include "kernel/Kernel.hh"

extern Kernel* theOS;
extern Kernel_ThreadCfdCl* pCurrentRunningThread;

/* FATAL=0,ERROR=1,WARN=2,INFO=3,DEBUG=4,TRACE=5 */
static const char* levelStrings[6] = { "FATAL", "ERROR", "WARN ", "INFO ", "DEBUG", "TRACE" };

/*** Escape Sequenzen: **********/
#define ESC_RED         "\033[31m"
#define ESC_GREEN       "\033[32m"
#define ESC_YELLOW      "\033[33m"
#define ESC_BLUE        "\033[34m"
#define ESC_PURPLE      "\033[35m"
#define ESC_CYAN        "\033[36m"
#define ESC_GRAY        "\033[37m"
#define ESC_WHITE       "\033[0m"

static char buffer[2048];
static unsigned int bufferpos;

FileLogger::FileLogger() :
        logFile(0)   {

    bufferpos = 0;
    initialized = false;
}


void FileLogger::init() {

    if (initialized)
        return;

    initialized = true; /* set first to avoid endless recursion in init.*/

    /* try permanent root storage! */
    Directory* rootDir = theOS->getFileManager()->getDirectory("mnt/TEST");
    if (rootDir != 0)
    {
        File* logfile = (File*) rootDir->get("Kernel.log",strlen("Kernel.log"));
        if (logfile) {
            rootDir->remove(logfile);
        }
        logFile = rootDir->createFile("Kernel.log", 0);
        return;
    }

#if 0
    /* try ramdisk  */
    Directory* ramdisk = theOS->getFileManager()->getDirectory("mnt/ramdisk");
    if (ramdisk != 0)
    {
        logFile = ramdisk->createFile("Kernel.log", 0);
        return;
    }
#endif

    /* mount points not available .. try later*/
    initialized = false;

}

void FileLogger::flush() {

    if (bufferpos > 0) {
        int intstatus;
       // GET_INTERRUPT_ENABLE_BIT(intstatus);
        //_disableInterrupts();

        /* flush now */
       if (logFile) {
           /* write to file */
           logFile->writeBytes(buffer,bufferpos);
           bufferpos = 0;
       }
       else {
           /* print to std out*/
           buffer[bufferpos] = 0;
           bufferpos = 0;
           puts(buffer);
       }
/*       if (intstatus)
           _enableInterrupts();*/

    }
}

static void fileout(char** str, char c) {
    File* logfile = (File*) str;

    if (bufferpos >= 2046)
    {
        int intstatus;
        GET_INTERRUPT_ENABLE_BIT(intstatus);
        _disableInterrupts();
        /* flush now */
        if (logfile) {
            /* write to file */
            logfile->writeBytes(buffer,bufferpos);
            bufferpos = 0;
        }
        else {
            /* print to std out*/
            buffer[bufferpos] = 0;
            bufferpos = 0;
            puts(buffer);
        }
        if (intstatus)
            _enableInterrupts();
    }

    buffer[bufferpos++] = c;
}

void FileLogger::log(Prefix prefix, Level level, const char* msg, ...) {
    if ((int) level > (int) prefix)
    {
        return;
    }

    if (!initialized)
        init();

#if LOG_PRINT_TIME
    unint4 time = 0;
    if (theOS != 0 && theOS->getClock() != 0)
    time =(unint4) (theOS->getClock()->getTimeSinceStartup() MICROSECONDS);

    fprintf(&fileout, (char**) logFile,"[%08u]",time);
#endif

    if (pCurrentRunningThread != 0)
        fprintf(&fileout, (char**) logFile, "[%03d][%s] ", pCurrentRunningThread->getId(), levelStrings[level]);
    else
        fprintf(&fileout, (char**) logFile, "[KER][%s] ", levelStrings[level]);

    va_list arglist;
    va_start(arglist, msg);
    print(&fileout, (char**) logFile, msg, arglist);
    print(&fileout, (char**) logFile, LINEFEED,arglist);

   // fileout((char**) logFile,'\r');
}

