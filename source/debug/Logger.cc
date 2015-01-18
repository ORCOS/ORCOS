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

#include "debug/Logger.hh"
#include <stdarg.h>
#include <sprintf.hh>
#include <putc.hh>
#include "kernel/Kernel.hh"

extern Kernel* theOS;
extern Kernel_ThreadCfdCl* pCurrentRunningThread;

#define LOG_PRINT_TIME 1

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

/*****************************************************************************
 * Method: _putchar(char** str, char c)
 *
 * @description
 *
 *******************************************************************************/
extern void _putchar(char** str, char c);

/*****************************************************************************
 * Method: Logger::log(Prefix prefix, Level level, const char* msg, ...)
 *
 * @description
 *
 *******************************************************************************/
void Logger::log(Prefix prefix, Level level, const char* msg, ...) {
    if (static_cast<int>(level) > static_cast<int>(prefix)) {
        return;
    }

#if LOGGER_PRINT_HIGHLIGHTS
    if (level <= ERROR) {
        printf(ESC_RED);
    } else if ( level == WARN ) {
        printf(ESC_YELLOW);
    }
#endif

#if LOG_PRINT_TIME
    unint4 time = 0;
    if (theOS != 0 && theOS->getClock() != 0)
    time = (unint4) (theOS->getClock()->getClockCycles() / (1 MICROSECONDS));

    int seconds = time / (1000000);
    time = time - (seconds * 1000000);
    printf("[%05u.%08u]", seconds, time);
#endif

    if (pCurrentRunningThread != 0) {
        printf("[%03d][%s] ", pCurrentRunningThread->getId(), levelStrings[level]);
    } else {
        printf("[KER][%s] ", levelStrings[level]);
    }

    va_list arglist;
    va_start(arglist, msg);
    print(&_putchar, 0, msg, arglist);

#if LOGGER_PRINT_HIGHLIGHTS
    if ( level <= WARN ) {
        printf(ESC_WHITE);
    }
#endif

    va_end(arglist);
    puts(LINEFEED);
}

