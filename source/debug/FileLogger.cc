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
#include "assemblerFunctions.hh"

extern Kernel* theOS;
extern Kernel_ThreadCfdCl* pCurrentRunningThread;

/* FATAL=0,ERROR=1,WARN=2,INFO=3,DEBUG=4,TRACE=5 */
static const char* levelStrings[6] = { "FATAL", "ERROR", "WARN ", "INFO ", "DEBUG", "TRACE" };

#define LOG_PRINT_TIME 1

/*** Escape Sequenzen: **********/
#define ESC_RED         "\033[31m"
#define ESC_GREEN       "\033[32m"
#define ESC_YELLOW      "\033[33m"
#define ESC_BLUE        "\033[34m"
#define ESC_PURPLE      "\033[35m"
#define ESC_CYAN        "\033[36m"
#define ESC_GRAY        "\033[37m"
#define ESC_WHITE       "\033[0m"

static char buffer1[2048];
static char buffer2[2048];
static unsigned int bufferpos;
static char* bufferptr;

FileLogger::FileLogger() :
        logFile(0) {
    bufferpos = 0;
    initialized = false;
    bufferptr = buffer1;
}

/*****************************************************************************
 * Method: FileLogger::init()
 *
 * @description
 *
 *******************************************************************************/
int FileLogger::init() {
    if (initialized)
        return (1);

    initialized = true; /* set first to avoid endless recursion in init.*/

    // TODO get paths from configuration
    /* try permanent root storage! */
    Directory* rootDir = theOS->getFileManager()->getDirectory("/mnt/ROOT");
    if (rootDir != 0) {
        File* logfile = static_cast<File*>(rootDir->get("Kernel.log", strlen("Kernel.log")));
        if (logfile) {
            rootDir->remove(logfile);
        }
        logFile = rootDir->createFile("Kernel.log", 0);
        return (1);
    }


    /* try ramdisk as fallback.. */
    Directory* ramdisk = theOS->getFileManager()->getDirectory("/mnt/ramdisk");
    if (ramdisk != 0) {
        logFile = ramdisk->createFile("Kernel.log", 0);
        return (1);
    }

    /* mount points not available .. try later*/
    initialized = false;
    return (0);
}

/*****************************************************************************
 * Method: FileLogger::flush()
 *
 * @description
 *
 *******************************************************************************/
void FileLogger::flush() {
    if (bufferpos > 0) {
         DISABLE_IRQS(irqstatus);
         char* curbuf = bufferptr;
         int len = bufferpos;

         if (bufferptr == buffer1)
             bufferptr = buffer2;
         else
             bufferptr = buffer1;
         bufferpos = 0;
         RESTORE_IRQS(irqstatus);

        /* flush now */
        if (logFile) {
            /* write to file */
            logFile->writeBytes(curbuf, len);
        } else {
            /* print to std out*/
            curbuf[len] = 0;
            puts(curbuf);
        }
    }
}

/*****************************************************************************
 * Method: fileout(char** str, char c)
 *
 * @description
 *
 *******************************************************************************/
static void fileout(char** str, char c) {
    File* logfile = reinterpret_cast<File*>(str);

    if (bufferpos >= 2046) {
        DISABLE_IRQS(irqstatus);
        char* curbuf = bufferptr;
        int len = bufferpos;

        if (bufferptr == buffer1)
            bufferptr = buffer2;
        else
            bufferptr = buffer1;
        bufferpos = 0;
        RESTORE_IRQS(irqstatus);


        /* flush now */
        if (logfile) {
            /* write to file */
            logfile->writeBytes(curbuf, len);
        } else {
            /* print to std out*/
            curbuf[len] = 0;
            puts(curbuf);
        }
    }

    bufferptr[bufferpos++] = c;
}

/*****************************************************************************
 * Method: FileLogger::log(Prefix prefix, Level level, const char* msg, ...)
 *
 * @description
 *
 *******************************************************************************/
void FileLogger::log(Prefix prefix, Level level, const char* msg, ...) {
    if (level > prefix) {
        return;
    }

    if (!initialized) {
        if (!init()) {
            return;
        }
    }

#if LOG_PRINT_TIME
    unint4 time = 0;
    if (theOS != 0 && theOS->getClock() != 0)
    time =(unint4) (theOS->getClock()->getTimeSinceStartup() MICROSECONDS);

    unint4 seconds = time / (1000000);
    time = time - (seconds * 1000000);

    fprintf(&fileout, reinterpret_cast<char**>(logFile), "[%05u.%08u]", seconds, time * 100);
#endif

    if (pCurrentRunningThread != 0)
        fprintf(&fileout, reinterpret_cast<char**>(logFile), "[%03d][%s] ", pCurrentRunningThread->getId(), levelStrings[level]);
    else
        fprintf(&fileout, reinterpret_cast<char**>(logFile), "[KER][%s] ", levelStrings[level]);

    va_list arglist;
    va_start(arglist, msg);
    print(&fileout, reinterpret_cast<char**>(logFile), msg, arglist);
    print(&fileout, reinterpret_cast<char**>(logFile), LINEFEED, arglist);
    va_end(arglist);
}

