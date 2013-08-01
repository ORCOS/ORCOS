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
#ifndef NEWLIB_HELPER_HH_
#define NEWLIB_HELPER_HH_

#include "inc/types.hh"
/*!
 * \brief   NEWLIB Helper Functions.
 *
 *  This is the complete set of system definitions (primarily subroutines) required;
 *  the examples shown implement the minimal functionality required to allow NEWLIB libc to link.
 */

#ifdef __cplusplus
extern "C" {
#endif

void abort(void);
/*!
 *  \brief Close a file.
 *  \param id is the file handle to close
 */
int close( int );

/*!
 *  \brief Status of a file.
 *  \param id is the file handle to close
 *  \param stat is a struct to save the information
 */
int fstat( int, struct stat * );

/*!
 *  \brief Get the Process-ID
 */
int getpid();

/*!
 *  \brief Query whether output stream is a terminal.
 *
 *  For consistency with the other minimal implementations, which only
 *  support output to stdout, this minimal implementation is suggested:
 *
 *  \param id is the terminal
 *  \param stat is a struct to save the information
 */
int isatty( int );

/*!
 *  \brief Send a signal.
 *
 *  \param id is the process to kill
 *  \param signal is the signal to invoke
 */
int kill( int, int );

/*!
 *  \brief Set position in a file.
 *
 *  \param file is the file handle for seek
 *  \param ptr is the file pointer
 *  \param dir is the direction
 */
int lseek( int, int, int );

/*!
 *  \brief Print to StdOut
 *
 *  \param string is the char* to print
 */
//void print( char* string );

/*!
 *  \brief Wait for a child process.
 */
void wait();

/*!
 *  \brief Invoke a NOP.
 */
void noop();

/*!
 *  \brief Send a signal to unblock an blocked task.
 *
 *  \param id is the task to signal
 */
void signal( unsigned int taskNr );

#ifdef __cplusplus
}
#endif

/*!
 *  \brief NEW-Operator for Memorymanager
 *  \param s is the size_t struct for memory allocation
 */
void* operator new( size_t s );

/*!
 *  \brief NEW-Operator for Memorymanager
 *  \param s is the size_t struct for memory allocation
 *  \param aligned is the boolean value, which have to be true is the memory allocation should be happen aligned
 */
void* operator new( size_t s, bool aligned );

/*!
 *  \brief DELETE-Operator for Memorymanager
 *  \param ptr is the pointer to free
 */
void operator delete( void* ptr );

#endif /*NEWLIB_HELPER_HH_*/
