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
    unint4          sa_data;            //!< socket address
    unint2          port_data;          //!< port
    sa_family_t     sa_family;          //!< The adress family of this struct
    service_name    name_data;      //!< socket name, maximum 16 chars
} sockaddr;


typedef struct {
    char* buffer;
    unint4 len;
} buffer_t;

struct hostent {
   char  *h_name;            /* official name of host */
   char **h_aliases;         /* alias list */
   int    h_addrtype;        /* host address type */
   int    h_length;          /* length of address */
   char **h_addr_list;       /* list of addresses */
};

#define STAT_TYPE_FILE      0
#define STAT_TYPE_DIRECTORY 1
#define STAT_TYPE_SYSVAR    2

typedef enum {
    SYSFS_SIGNED_INTEGER    = 0,
    SYSFS_UNSIGNED_INTEGER  = 1,
    SYSFS_STRING            = 2
} SysFs_t;

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



typedef struct {
    char    hwaddr[8];
    unint4  ipv4addr;
    unint4  gwipv4addr;
    unint4  ipv4netmask;
    unint4  flags;
    unint4  rxpackets;
    unint4  txpackets;
    unint4  rxbytes;
    unint4  txbytes;
    unint4  dropped;
    unint2  mtu;
    unint2  errors;
    unint4  irqnum;
    char    ipv6addr[16];
} netif_stat_t;

/*
 * Type of a resource. Used inside fields
 * Directory_Entry_t.resType and
 * stat_t.st_type
 */
typedef enum {
    cResT_Directory           = 1 << 0,
    cResT_cStreamDevice       = 1 << 1,
    cResT_cCommDevice         = 1 << 2,
    cResT_cGenericDevice      = 1 << 3,
    cResT_cFile               = 1 << 4,
    cResT_cSocket             = 1 << 5,
    cResT_cUSBDriver          = 1 << 6,
    cResT_cBlockDevice        = 1 << 7,
    cResT_cPartition          = 1 << 8,
    cResT_cSharedMem          = 1 << 9,
    cResT_cKernelVariable     = 1 << 10,
} ResourceType_t;

/* Soft termination flag. Terminate thread after next instance */
#define TERM_SOFT 1
/* Hard termination flag. Terminate thread now */
#define TERM_HARD 2


#define TIOCTL_SET_STDOUT 0

//! File statistics
struct stat {
    unint4 st_size;
    unint4 st_type;
    unint4 st_flags;
    unint4 st_date;
};

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
    unint8 arrivaltime;     /* absolute time to be started*/
    unint4 priority;
    unint4 period;
    unint4 deadline;
    unint4 executionTime;
    unint4 stack_size;
    unint4 reserved;        /* for alignment */
} thread_attr_t;



typedef int ErrorT;
typedef unint2 EndianT;
typedef unint2 CpuClassT;
typedef unint2 CpuVersionT;

/*!
 *  Type-defines for process management
 */
typedef unint1 TaskIdT;
typedef unint2 ThreadIdT;
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
