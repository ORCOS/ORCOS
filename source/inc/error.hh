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
//#define LOG(arg)                       theOS->getLogger()->log arg
#define LOG(prefix,level,msg) if ((int) level <= (int) prefix)  theOS->getLogger()->log msg
#else
#define LOG(prefix,level,msg)
#endif


//forward declaration
class ThreadCfdCl;

/* All of the following codes 'x' indicate either an error code or a warning.
 This fact is distinguished by the value of x:

 x == cOk:                    ok status
 x <  cOk (negative codes):   error codes
 x >  cOk (positive codes):   warnings or ordinary return values

 As the same define name may interpreted as both (an error or a warning) each
 value should be used only once!  If in the following a symbol 's' is defined
 by a negative value C(s) then it is usually returned as an error code.
 However, some function may return -s to tell the environment that s
 represents a warning in this situation.  Conversely, if in the following a
 symbol 's' is defined by a positive value, then 's' is used as a return
 status in general.  Again, under certain circumstances some functions may //
 return -s instead to tell the environment that it is an error.

 */

/*-- Symbol name s ----------------------------|-- code C(s) --*/
#define cOk					(int)0

// The following values with |C(s)| > 1000 are used globally (across component boundaries):

// Warnings or Status Values

// Errors
// General Error without further information
#define cError				(int)-1000
// Error indicating that the method called is not implemented
#define cNotImplemented		(int)-1001
// Error indicating that a parameter illegally was a null pointer
#define cNullPointerProvided (int)-1002

#define cStackOverflow (int)-1003;

#define cStackUnderflow (int) -1004;

// The following values with |C(s)| < 1000 are used inside the specific
// components. Each component has 100 possible |C(s)|.
//-----------------------------------------------------
// Scheduling               0 < |C(s)| < 100
//-----------------------------------------------------
// Warning or Status Value

// Error

//-----------------------------------------------------
// Memory Management        100 <= |C(s)| < 200
//-----------------------------------------------------
// Warning or Status Value

// Error
#define cHeapMemoryExhausted (int)-100
#define cGivenTLBEntryNotFound (int)-101
#define cNoValidChunkAddress (int)-102

//-----------------------------------------------------
// Synchronization          200 <= |C(s)| < 300
//-----------------------------------------------------
// Warning or Status Value

// Error

//-----------------------------------------------------
// Process Management       300 <= |C(s)| < 400
//-----------------------------------------------------
// Warning or Status Value

#define cInvalidCBHeader (int)-300

// Error

//-----------------------------------------------------
// Communication            400 <= |C(s)| < 500
//-----------------------------------------------------
// Warning or Status Value
#define cNotConnected   (int)-400
#define cTransportProtocolNotAvailable (int)-401
#define cAddressProtocolNotAvailable (int)-402
#define cSocketAlreadyBoundError (int)-403
// Error

//-----------------------------------------------------
// Database                 500 <= |C(s)| < 600
//-----------------------------------------------------
// Warning or Status Value

// Error
#define cDatabaseOverflow (int)-500
#define cElementNotInDatabase (int)-501
#define cIndexOutOfBounds (int)-502

//-----------------------------------------------------
// HAL                      600 <= |C(s)| < 700
//-----------------------------------------------------
// Warning or Status Value
#define cInvalidArgument 	(int)-600
// Error

//-----------------------------------------------------
// Architecture             700 <= |C(s)| < 800
//-----------------------------------------------------
// Warning or Status Value

// Error
#define cInvalidResourceType    (int)-700

// The status of a method is ok if the returned error code equals 0
#define isOk(errcode)           !(errcode)

// Error codes less than cOk are always true errors
#define isError(errcode)        ((errcode) < cOk)

// In some cases error codes greater than cOk can be used as uncritical return
// values.  The next macro checks if the error code is not an error.
#define maybeOk(errcode)        ((errcode) >= cOk)

//methods to handle a behavior when an error occurs
//implemented in error.cc

//void printf( const char *format, ... );

//#define ERROR(a) {printf("ERROR in line %d in file %s with message: %s",__LINE__,__FILE__,a); while(1);}
#define ERROR(a) {printf("ERROR in line %d in file %s with message: %s",__LINE__,__FILE__,a); while(1);}

#if __DEBUG__
#define ASSERT(a) if (!a) {printf("ASSERTION in expression in line %d in file %s \n\r",__LINE__,__FILE__); while (1) ;}
#else
#define ASSERT(a) if (!a) ;
#endif

ErrorT handleError( ErrorT );
ErrorT handleError( ErrorT, ThreadCfdCl* );

#endif /* _ERROR_HH */
