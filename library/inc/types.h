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

#ifndef TYPES_H_
#define TYPES_H_

#include <archtypes.h>

typedef unsigned int size_t;

// Socket related types

typedef int1 sa_family_t;

typedef char service_name[17];

//! Structure describing the address of a socket.
typedef struct {
    unint4      sa_data;            //!< socket address
    unint2      port_data;          //!< port
    sa_family_t sa_family;          //!< The adress family of this struct
    service_name    name_data;      //!< socket name, maximum 16 chars
} sockaddr;

/*
 * Directory entry structure as returned
 * by reading a directory using fread.
 */
typedef struct {
    unint4  flags;
    unint4  filesize;
    unint4  datetime;
    unint4  resId;
    unint2  reserved;
    unint1  resType;
    unint1  namelen;
    char    name[0]; /* variable length name entry */
} Directory_Entry_t;

//! File statistics
typedef struct {
	unint4 st_size;
} stat_t;

/*!
 * \brief Service description structure which can be used to create a socket
 * in order to communicate with the service.
 *
 * This structure is returned on service discovery and contains all
 * data to create socket to communicate with that service.
 */
typedef struct {
    int domain; //!< The domain the service can be found in
    int type; //!< The socket-type the service works with (connectionless,connection-oriented)
    int transport_protocol; //!< The protocol the service uses to communicate
    sockaddr address; //!< The address of the service
} servicedescriptor;

//! The socket type. STREAM mode or DATAGRAM mode.
typedef enum {
    SOCK_STREAM, SOCK_DGRAM
} SOCK_TYPE;

//! Flags for the receive parameter
typedef enum {
    MSG_PEEK = 1, MSG_WAIT = 2, RESERVED = 4
} RECV_FLAGS;


//! Thread attribute structure for thread creation.
typedef struct {
    unint4 priority;
    unint4 phase;
    unint4 period;
    unint4 deadline;
    unint4 executionTime;
    unint4 stack_size;
} thread_attr_t;


typedef struct {
    unint2 year;
    unint1 month;
    unint1 day;
    unint1 hour;
    unint1 minute;
    unint1 second;
    unint1 week_day;
    unint1 day_of_year;
    unint1 isdaylightsaving;
} time_t;

typedef int ErrorT;
typedef unint2 EndianT;
typedef unint2 CpuClassT;
typedef unint2 CpuVersionT;

/*!
 *  Type-defines for process management
 */
typedef unint1 TaskIdT;
typedef unint1 ThreadIdT;
typedef unint4 ResourceIdT;

/*!
 * General type defines
 */
typedef unsigned char unchar;
typedef unsigned short unshort;
typedef unsigned int unint;
typedef unsigned long unlong;
typedef unint4 BitmapT;
typedef unint1 byte;
typedef void* PhysAddrT;

#endif /*TYPES_H_*/
