/*
 * error.h
 *
 *  Created on: 29.01.2015
 *      Author: Daniel
 */

#ifndef LIBRARY_INC_ERROR_H_
#define LIBRARY_INC_ERROR_H_



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


// The following values with |C(s)| > 1000 are used globally (across component boundaries):


#define cOk                     (int)0


#define cError                  (int)-1000
#define cNotImplemented         (int)-1001

// Error indicating that a parameter illegally was a null pointer
#define cNullPointerProvided    (int)-1002
#define cStackOverflow          (int)-1003
#define cStackUnderflow         (int)-1004
#define cWrongAlignment         (int)-1005
// Error indicating that the resource can not be removed
#define cResourceNotRemovable   (int)-1006
// A wrong resource type has been passed to a method.
#define cWrongResourceType      (int)-1007
#define cNoData                 (int)-1008
#define cResourceAlreadyExists  (int)-1009
#define cResourceRemoved        (int)-1010
#define cInvalidResourceType    (int)-1011
#define cTransactionFailed      (int)-1012
#define cResourceNotOwned       (int)-1013
#define cResourceNotWriteable   (int)-1014
#define cResourceNotReadable    (int)-1015
#define cInvalidResource        (int)-1016
#define cCanNotAquireResource   (int)-1017
#define cFileNotFound           (int)-1018
#define cTimeout                (int)-1019
#define cInvalidConcurrentAccess (int)-1020

#define cArrayLengthOutOfBounds (int)-800
#define cWrongArrayLengthByte   (int)-801



// The following values with |C(s)| < 1000 are used inside the specific
// components. Each component has 100 possible |C(s)|.
//-----------------------------------------------------
// Filesystem               0 < |C(s)| < 100
//-----------------------------------------------------
// Warning or Status Value

#define cEOF                    (int)-5
#define cInvalidPath            (int)-6
#define cUnknownCmdOrPath       (int)-7

// Error

//-----------------------------------------------------
// Memory Management        100 <= |C(s)| < 200
//-----------------------------------------------------
// Warning or Status Value

// Error
#define cHeapMemoryExhausted    (int)-100
#define cGivenTLBEntryNotFound  (int)-101
#define cNoValidChunkAddress    (int)-102
#define cDeviceMemoryExhausted  (int)-103
#define cMemMappingError           (int)-104

//-----------------------------------------------------
// Synchronization          200 <= |C(s)| < 300
//-----------------------------------------------------
// Warning or Status Value

// Error

//-----------------------------------------------------
// Process Management       300 <= |C(s)| < 400
//-----------------------------------------------------
// Warning or Status Value

#define cInvalidCBHeader        (int)-300
#define cTaskCRCFailed          (int)-301
#define cThreadNotFound         (int)-302
// Error

//-----------------------------------------------------
// Communication            400 <= |C(s)| < 500
//-----------------------------------------------------
// Warning or Status Value
#define cNotConnected                       (int)-400
#define cTransportProtocolNotAvailable      (int)-401
#define cAddressProtocolNotAvailable        (int)-402
#define cSocketAlreadyBoundError            (int)-403
#define cTCPEnqueueFailed                   (int)-404
#define cPBufNoMoreMemory                   (int)-405
#define cSocketAlreadyListened              (int)-406
#define cInvalidSocketType                  (int)-407
#define cErrorConnecting                    (int)-408


// Error

//-----------------------------------------------------
// Database                 500 <= |C(s)| < 600
//-----------------------------------------------------
// Warning or Status Value

// Error
#define cDatabaseOverflow           (int)-500
#define cElementNotInDatabase       (int)-501
#define cIndexOutOfBounds           (int)-502

//-----------------------------------------------------
// HAL                      600 <= |C(s)| < 700
//-----------------------------------------------------
// Warning or Status Value
#define cInvalidArgument            (int)-600
// Error

//-----------------------------------------------------
// Architecture             700 <= |C(s)| < 800
//-----------------------------------------------------
// Warning or Status Value

#define cBlockDeviceReadError       (int)-700
#define cBlockDeviceWriteError      (int)-701
#define cBlockDeviceTooManyBlocks   (int)-702


// Warnings or Status Values


// The status of a method is ok if the returned error code equals 0
#define isOk(errcode)           !(errcode)

// Error codes less than cOk are always true errors
#define isError(errcode)        ((errcode) < cOk)

// In some cases error codes greater than cOk can be used as uncritical return
// values.  The next macro checks if the error code is not an error.
#define maybeOk(errcode)        ((errcode) >= cOk)

#ifdef __cplusplus
extern "C" {
#endif

char* strerror(int errornum);


#ifdef __cplusplus
}
#endif



#endif /* LIBRARY_INC_ERROR_H_ */
