
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
#define cNETIF_REQUEST_DHCP 50


#define NETIF_FLAG_UP           0x01U
/** If set, the netif has broadcast capability.
 * Set by the netif driver in its init function. */
#define NETIF_FLAG_BROADCAST    0x02U
/** If set, the netif is one end of a point-to-point connection.
 * Set by the netif driver in its init function. */
#define NETIF_FLAG_POINTTOPOINT 0x04U
/** If set, the interface is configured using DHCP.
 * Set by the DHCP code when starting or stopping DHCP. */
#define NETIF_FLAG_DHCP         0x08U
/** If set, the interface has an active link
 *  (set by the network interface driver).
 * Either set by the netif driver in its init function (if the link
 * is up at that time) or at a later point once the link comes up
 * (if link detection is supported by the hardware). */
#define NETIF_FLAG_LINK_UP      0x10U
/** If set, the netif is an ethernet device using ARP.
 * Set by the netif driver in its init function.
 * Used to check input packet types and use of DHCP. */
#define NETIF_FLAG_ETHARP       0x20U
/** If set, the netif is an ethernet device. It might not use
 * ARP or TCP/IP if it is used for PPPoE only.
 */
#define NETIF_FLAG_ETHERNET     0x40U
/** If set, the netif has IGMP capability.
 * Set by the netif driver in its init function. */
#define NETIF_FLAG_IGMP         0x80U

//-----------------------------------------------------
// Error Messages
//-----------------------------------------------------




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
#define cThreadWaitIRQSyscallId             46
#define cGetHostByNameSyscallId             47
#define cGetCyclesSyscallId                 48

#endif /* TASKLIB_HH_ */
