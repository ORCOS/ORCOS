
#ifndef DEFINES_H_
#define DEFINES_H_

//-----------------------------------------------------
// Networking
//-----------------------------------------------------
#define IP4ADDR(a,b,c,d) \
        htonl((((unint4)((a) & 0xff) << 24) | \
        ((unint4)((b) & 0xff) << 16) | \
        ((unint4)((c) & 0xff) << 8) | \
         (unint4)((d) & 0xff)))


#define cSimpleTransportProtocol    150
#define cSimpleAddressProtocol      55555

#define cUDP                        17
#define cTCP                        0x6
#define cIPV4                       0x800
#define cIPv4AddressProtocol        0x800


/* Supported address families. */
#define AF_UNSPEC   0
#define AF_UNIX     1   /* Unix domain sockets      */
#define AF_INET     2   /* Internet IP Protocol     */
#define AF_AX25     3   /* Amateur Radio AX.25      */
#define AF_IPX      4   /* Novell IPX           */
#define AF_APPLETALK    5   /* Appletalk DDP        */
#define AF_NETROM   6   /* Amateur radio NetROM     */
#define AF_BRIDGE   7   /* Multiprotocol bridge     */
#define AF_AAL5     8   /* Reserved for Werner's ATM    */
#define AF_X25      9   /* Reserved for X.25 project    */
#define AF_INET6    10  /* IP version 6         */
#define AF_MAX      12  /* For now.. */

// wait for task to finish?
#define cWait                        (1 << 0)

/* Creation flag. Create resource. */
#define cCreate                     (1 << 1)

#define cTYPE_DIR                (1 << 1)
#define cTYPE_FILE               (0)
#define cTYPE_KVAR               (1 << 10)

#define cNETIF_SETIPV4     1
#define cNETIF_SETGWIPV4   2
#define cNETIF_SETNETMASK  3

#define cNETIF_SET_UP       20
#define cNETIF_SET_DOWN     21

#define cNETIF_GET_STATS    512


#define NETIF_FLAG_UP           0x01U
    /** if set, the netif has broadcast capability */
#define NETIF_FLAG_BROADCAST    0x02U
    /** if set, the netif is one end of a point-to-point connection */
#define NETIF_FLAG_POINTTOPOINT 0x04U
    /** if set, the interface is configured using DHCP */
#define NETIF_FLAG_DHCP         0x08U
    /** if set, the interface has an active link
     *  (set by the network interface driver) */
#define NETIF_FLAG_LINK_UP      0x10U
    /** if set, the netif is an device using ARP */
#define NETIF_FLAG_ETHARP       0x20U
    /** if set, the netif has IGMP capability */
#define NETIF_FLAG_IGMP         0x40U

//-----------------------------------------------------
// Error Messages
//-----------------------------------------------------

#define cOk                     (int)0
#define cError                  (int)-1000

#define cTransactionFailed      (int)-1152

#define cThreadNotFound         (int)-100
#define cResourceNotOwned       (int)-101
#define cResourceNotWriteable   (int)-102
#define cResourceNotReadable    (int)-103
#define cInvalidResource        (int)-104
#define cCanNotAquireResource   (int)-105

#define cArrayLengthOutOfBounds (int)-800
#define cWrongArrayLengthByte   (int)-801
#define cEOF                    (int)-8

/* Error constant to string conversion */
#define errstr(s)                 #s


//-----------------------------------------------------
// Filesystem
//-----------------------------------------------------

/* Absolute position inside file */
#define SEEK_SET 0
/* Offset from current position */
#define SEEK_CUR 1
/* Offset from end of file*/
#define SEEK_END 2

/* Type for overlay mounts */
#define cMountType_Overlay      1

#ifndef NULL
#define NULL 0
#endif

//-----------------------------------------------------
// Syscall IDs
//-----------------------------------------------------

// keep these syscall ids together in a rang 0 - maxsyscallnum
// this will drastically speedup the system call handling
// since the SyscallManager will use a jump table instead of lots
// of if then else statements

// Stream/File related Syscalls

#define cFOpenSysCallId                     0
#define cFCloseSysCallId                    1
#define cFReadSysCallId                     2
#define cFWriteSysCallId                    3
#define cFPutcSysCallId                     4
#define cFGetcSysCallId                     5
#define cFCreateSysCallId                   6

// Memory related Syscalls

#define cNewSysCallId                       7
#define cDeleteSysCallId                    8

// Task related Syscalls

#define cTask_StopSysCallId                 9
#define cTask_ResumeSysCallId               10

// Thread related Syscalls

#define cSleepSysCallId                     11
#define cThread_CreateSysCallId             12
#define cThread_RunSysCallId                13
#define cThread_SelfSysCallId               14
#define cThread_YieldSysCallId              15
#define cThread_ExitSysCallId               16

// Signal related Syscalls

#define cSignal_WaitSyscallId               17
#define cSignal_SignalSyscallId             18

// Socket related Syscalls

#define cSocketSyscallId                    19
#define cConnectSyscallId                   20
#define cListenSyscallId                    21
#define cBindSyscallId                      22
#define cSendtoSyscallId                    23
#define cRecvFromSyscallId                  24

#define cIOControl                          25

#define cPrintToStdOut                      26
#define cNewProtSysCallId                   27

#define cGetTimeSyscallId                   28
#define cMapMemorySyscallId                 29
#define cModuleReturnId                     30

#define cRunTaskId                          31
#define cTask_KillSysCallId                 32
#define cShmMapId                           33

#define cThread_WaitPID                     34
#define cFStatId                            35
#define cFRemoveID                          36
#define cGetPID                             37
#define cShmUnmapId                         38
#define cGetDateTimeSyscallId               39
#define cMkDevSyscallId                     40
#define cTaskioctlscallId                   41
#define cFSeekSyscallId                     42
#define cThreadTerminateSyscallId           43
#define cThreadNameSyscallId                44
#define cMountSyscallId                     45

#endif /* TASKLIB_HH_ */