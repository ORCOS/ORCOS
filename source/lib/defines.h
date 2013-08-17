

#ifndef DEFINES_H_
#define DEFINES_H_


#define cOk                     (int)0
#define cError                  (int)-1000
#define cArrayLengthOutOfBounds (int)-800
#define cWrongArrayLengthByte   (int)-801

#define IP4ADDR(a,b,c,d) \
        (((unint4)((a) & 0xff) << 24) | \
                               ((unint4)((b) & 0xff) << 16) | \
                               ((unint4)((c) & 0xff) << 8) | \
                                (unint4)((d) & 0xff))

#define cIPv4AddressProtocol        2048
#define cSimpleTransportProtocol    150
#define cSimpleAddressProtocol      55555
#define cUDP      					17

//-----------------------------------------------------
// Error Messages
//-----------------------------------------------------
#define cThreadNotFound             -100
#define cResourceNotOwned           -101
#define cResourceNotWriteable       -102
#define cResourceNotReadable        -103
#define cInvalidResource            -104
#define cCanNotAquireResource       -105

// keep these syscall ids together in a rang 0 - maxsyscallnum
// this will drastically speedup the system call handling
// since the SyscallManager will use a jump table instead of lots
// of if then else statements

// Stream/File related Syscalls

#define cFOpenSysCallId     0
#define cFCloseSysCallId    1
#define cFReadSysCallId     2
#define cFWriteSysCallId    3
#define cFPutcSysCallId     4
#define cFGetcSysCallId     5
#define cFCreateSysCallId	6

#define cDirTypeORCOS		1
#define cDirTypeFAT32		2

// Memory related Syscalls

#define cNewSysCallId       7
#define cDeleteSysCallId    8

// Task related Syscalls

#define cTask_StopSysCallId     9
#define cTask_ResumeSysCallId   10

// Thread related Syscalls

#define cSleepSysCallId         11
#define cThread_CreateSysCallId 12
#define cThread_RunSysCallId    13
#define cThread_SelfSysCallId   14
#define cThread_YieldSysCallId  15
#define cThread_ExitSysCallId   16

// Signal related Syscalls

#define cSignal_WaitSyscallId      17
#define cSignal_SignalSyscallId    18

// Socket related Syscalls

#define cSocketSyscallId        19
#define cConnectSyscallId		20
#define cListenSyscallId		21
#define cBindSyscallId          22
#define cSendtoSyscallId        23
#define cRecvFromSyscallId      24

#define cIOControl				25

#define cPrintToStdOut          26
#define cNewProtSysCallId       27

#define cGetTimeSyscallId		28
#define cMapMemorySyscallId		29
#define cModuleReturnId			30

#define cRunTaskId				31
#define cTask_KillSysCallId		32
#define cShmMapId				33

#endif /* TASKLIB_HH_ */
