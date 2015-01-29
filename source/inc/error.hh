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

#ifndef _ERROR_HH
#define _ERROR_HH

#include <types.hh>
#include "SCLConfig.hh"

#if !MODULE_IN_USERSPACE
#include "sprintf.hh"
#endif

#include "logger_config.hh"

#ifdef HAS_Kernel_LoggerCfd
#ifndef LOG
#define LOG(prefix, level, ...) if ((int) level <= (int) prefix) { if (theOS != 0 && theOS->getLogger() != 0) theOS->getLogger()->log(prefix,level, __VA_ARGS__);}
#endif
#else
#ifndef LOG
#define LOG(prefix,level, ...)
#endif
#endif

/* include user space errors */
#include <inc/error.h>

//forward declaration
class Kernel_ThreadCfdCl;

/* backtrace and print the call stack from the given address and stack pointer */
extern "C" void     backtrace_addr(void* currentAddress, size_t stackPtr);

/* backtrace and print the current call stack */
extern "C" void     backtrace_current();

extern "C" void     backtrace(void** buffer, int length);

extern "C" char*    getMethodSignature(unint4 address);




// ERROR MACROS
#define ERROR(a) {printf("ERROR in line %d in file %s with message: %s",__LINE__,__FILE__,a); while(1);}

#if __DEBUG__
#define ASSERT(a) if (!(a)) {printf("ASSERTION in line %d in file %s",__LINE__,__FILE__); while (1) ;}
#else
#define ASSERT(a) if (!(a)) ;
#endif

#endif /* _ERROR_HH */
