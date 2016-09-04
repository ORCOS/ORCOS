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

#ifndef ORCOS_HH_
#define ORCOS_HH_

#include "error.h"
#include "orcos_types.h"
#include "defines.h"


#ifdef __cplusplus
extern "C" {
#define DEFAULT(x) =x

#else
#define DEFAULT(x)
#define bool char
#endif

// Standard linefeed for UNIX like OS
#define LINEFEED "\r\n"

 int      syscall (int syscallnumber, ...);

/*!
 * @brief Atomic test and set operation.
 *
 * Tests the value at @address on @testvalue. If the value is @testvalue the value
 * at address is set to @setvalue. The complete operation is ensured to be atomic.
 *
 * Multicore-safe.
 *
 * @return          1 on success. 0 otherwise.
 */
 int      testandset(void* address, int testvalue, int setvalue);




/**************************************
 *  Task related system calls
 **************************************/

/*!
 * @brief Start a new task given by its file path.
 *
 * @param path              The path to the task file to be executed
 * @param arguments         A null terminated string of arguments
 * @param stdout            Path to the stdout device of this task. If 0 or not found it will be ignored.
 *
 * @return                  Error Number < 0 or task id > 0
 */
 int              task_run(char* path, char* arguments, char* fstdout DEFAULT(0), char* workingDir DEFAULT(0));

/*!
 * @brief Stops and removes a task from the system.
 *
 * @param taskid            The ID of the task to be removed
 *
 * @return                  Error Number < 0 or 0 (on success)
 */
 int              task_kill(int taskid);

/*!
 * @brief Stop the specified task. It could be resumed later on.
 *
 * @param taskid            The ID of the task that should be stopped.
 *
 * @return                  Error Number
 */
 int              task_stop(int taskid);

/*!
 * @brief resumes the specified task. Resumes execution of a task that is currently not running.
 *
 * @param taskid            The ID of the task that should be resumed.
 *
 * @return                  Error Number
 */
 int              task_resume(int taskid);

/**************************************
 *  Thread related system calls
 **************************************/


/*!
 * @brief wait method.  Blocks the current calling thread until any other thread inside the current process terminates.
 *
 *  The method blocks the current thread until any other thread of the current process terminates. If no other thread exists
 *  that might terminate this can lead to a endlessly blocked thread. However, another thread from a different process
 *  may unblock the stuck thread if it issues the correct signal_signal syscall.
 *  e.g.: signal_signal(ProcessID_To_Unblock << 16) | (SIG_CHILD_TERMINATED).
 *  This is possible as orcos uses signals for the wait operation.
 *
 * @return                  The exit value of the terminated thread.
 */
 int              wait();

/*!
 * @brief wait for process method.  Blocks the current calling thread until the process terminates.
 *
 *  The method blocks the current thread until the specified process terminates.  If no such process exists
 *  this can lead to a endlessly blocked thread. However, another thread from a different process
 *  may unblock the stuck thread if it issues the correct signal_signal syscall.
 *  e.g.: signal_signal(pid << 16).
 *  This is possible as orcos uses signals for the wait operation.
 *
 * @return                 The exit value of the terminated process.
 */
 int              waitpid(int pid);


/*!
 * @brief wait for thread method.  Blocks the current calling thread until the thread terminates.
 *
 *  The method blocks the current thread until the specified thread terminates.
 *  The thread specified by 'tid' must be inside the current process. It is not
 *  supported/allowed to wait for a thread of a different process.
 *  If no such thread exists this can lead to a endlessly blocked thread.
 *  However, another thread may unblock the stuck thread if it
 *  issues the correct signal_signal syscall.
 *
 *  This is possible as ORCOS uses signals for the wait operation.
 *
 * @return                  The exit value of the terminated process.
 */
 int              waittid(int tid);

/*!
 * @brief wait for irq method.  Blocks the current calling thread until the irq is raised.
 *
 *  The method blocks the current thread until the specified irq is raised.
 *  The thread will be unblock after the irq handler executed (if any) and scheduled as normal.
 *
 * @return                  The exit value of the terminated process.
 */
 int              waitirq(unint4 irq);


/*!
 *  @brief Sleep method. The calling thread will be blocked for at least 's' seconds.
 *
 * Real-Time threads are guaranteed to wake up after the exact (except latency) number of
 * seconds. If they have the highest priority they will be executed directly.
 *
 * @param s                The number of seconds to sleep
 */
 unsigned int    sleep(unsigned int s);

/*!
 *  @brief The calling thread will be blocked for at least 'us' micro-seconds.
 *
 * Real-Time threads are guaranteed to wake up after the exact (except latency) number of micro-
 * seconds. If they have the highest priority they will be executed directly.
 *
 * @param us               The number of microseconds to sleep
 */
 int             usleep(unsigned long us);

/*!
 * @brief Returns the process ID of the current running process.
 */
 int             getpid();

/*!
 * @brief Returns the current working directory
 *
 * @param buf    Buffer to store the path
 * @param buflen size of the buffer in bytes
 * @return       Errorcode
 */
 int             getcwd(char* buf, int buflen);

 /*!
  * @brief Tries to change the current working directory to path
  * @param path
  * @return
  */
 int             chdir(const char* path);

/*!
 * @brief Create a new thread.
 *
 * @param threadid         Return value which will hold the thread id on success
 * @param attr             Pointer to the thread attributes structure holding information for thread creation
 * @param start_routine    Pointer to the executeable code of the thread
 * @param arg              Argument passed to the thread on execution
 *
 * @return                 Error Number
 */
 int             thread_create(ThreadIdT* threadid, thread_attr_t* attr, void *(*start_routine)(void*), void* arg);

/*!
 * @brief Start the execution of the thread with id 'threadid'.
 *
 * @param threadid         The id of the thread to run.
 *                         If the threadid is <= 0 all newly created threads of the current task will be started simultaneously!
 *                         This is important if you want multiple realtime threads to arrive at the same time
 *                         so that the schedule is not phase shifted!
 *
 * @return                 Error Number
 */
 int             thread_run(int threadid);


/*!
 * @brief Name the Thread given by threadId or current thread
 *
 * @param threadid         The id of the thread to be name. If the threadid is == 0 the current thread is named.
 * @param name             The new name of the thread. Maximum 30 characters will be used.
 *
 * @return                 Error Number
 */
 int             thread_name(int threadid, char* name);

/*!
 * @brief Returns the id of the currently running thread
 */
 int             thread_self();

/*!
 * @brief Voluntarily yield the cpu to some other thread (if any)
 */
 void            thread_yield();

/*!
 * @brief Returns from the the currently executing thread instance
 *
 * Periodic Threads end their currently exeucting thread instance.
 * Aperiod Threads also terminate.
 */
 void            thread_exit(int exitCode DEFAULT(cOk)) __attribute__((noreturn));

/*!
 * @brief Terminates the given thread.
 *
 * @param threadId  Thread to terminate
 * @param flag      TERM_SOFT        Soft terminate the thread after next instance
 *                  TERM_HARD        Hard terminate the thread directly
 *
 * @return ErrorCode. cOK on success
 */
 int             thread_terminate(int threadId, int flag DEFAULT(TERM_SOFT));


/**************************************
 *  Signal related system calls.
 *
 *  May be used for synchronized Message passing.
 **************************************/


/*!
 *  @brief  Causes the calling thread to wait for the signal given by parameter sig.
 *
 *  The calling thread will be blocked until the signal sig is raised by any other thread inside the system.
 *
 * @param sig              The signal to wait for
 * @param memAddrAsSig     Treat the signal sig as a memory address inside the calling address space
 *
 * @return                The signal value passed by the raising thread
 */
 int             signal_wait( void* sig, bool memAddrAsSig DEFAULT(false) );


/*!
 * @brief Raises the system wide signal given by param sig and passes param value as signal value to all waiting threads.
 *
 *  The signal_signal system call raises a system wide signal waking up all threads waiting for that signal.
 *
 * @param  sig             The signal to be raised. Must be > 0. if memAddrAsSig is true sig will be interpreted as a memory location.
 * @param  value           The value passed to all waiting threads. Effectively allows sizeof(int) bytes of message passing.
 * @param  memAddrAsSig    Treat sig as a memory address in calling threads address space.
 *
 * @comment
 *  0 is invalid for the signal number.
 */
 void            signal_signal( void* sig, int value, bool memAddrAsSig DEFAULT(false) );



/**************************************
 *  Character device related system calls
 **************************************/

/*
 * COMMENT:
 * Handling character device syscalls is handled quite similiar to the posix
 * standard http://www.opengroup.org/onlinepubs/009695399/ although the implementation
 * is not 100% posix standard.
 */

 int             create(const char* filepath, int flags DEFAULT(cTYPE_FILE));

/*!
 * @brief  The fopen() function shall open the resource whose pathname is the string pointed to by filename, and associates a stream with it.
 *
 * @param filename         The resource to be opened given by its path (e.g "/dev/testdev")
 * @param blocking         If the resource is owned by another task and this is set to 1 the OS will block
 *                         the current thread until the resource is available
 *
 * @return                 The file descriptor on success. Error Code otherwise.
 */
 int             open(const char* filename, int blocking DEFAULT(1) );

/*!
 * @brief   Close the resource associated with this file handle. May fail if this resource was not acquired.
 *          If the resource was a shared memory device all mappings created on this device are unmapped and removed!
 *          Example: shm_map on /mem/mem can be done multiple times, each returing the /mem/mem file handle.
 *          If this handle is closed ALL mappings are removed! If you want to remove only a specific mapping
 *          use shm_unmap instead!
 *
 * @param fd               The file descriptor as returned by open() of the resource/file to be closed.
 *
 * @return                 cOk on success. Error Code  otherwise.
 */
 int             close(int fd);

/*!
 * @brief The read() function shall read into the array pointed to by ptr up to size bytes, from the stream pointed to by stream.
 *
 * @param  fd              The id of the stream which shall be read
 * @param  buf             The array into which the data shall be written
 * @param  size            The amount of data to be read
 *
 * @return                 The amount of bytes read. < 0 on error
 */
 int             read(int fd, char *buf, size_t size);

/*!
 * @brief The fwrite() function shall write, from the array pointed to by ptr count bytes, to the stream pointed to by stream.
 *
 * @param fd               The file descriptor of the stream
 * @param buf              Pointer to the data that shall be written
 * @param count            The number of bytes to be written.
 *
 * @return                 Amount of data written on success, < 0 on error
 */
 int             write(int fd, const void *buf, size_t size);

/*!
 * @brief The fstat method returns the file statistics of a resource given by its handle fd.
 *
 * @param fd               The file descriptor as returned by open()
 * @param stat             Pointer to a stat_t structure that will be filled with the file statistics.
 *
 * @return                 cOk on success. Error Code otherwise
 */
 int             fstat(int fd, struct stat* buf);


/*!
 * @brief Tries to remove a resource given by its absolute path.
 *
 * The method fails (return value < 0) if the resource can not be found,
 * the resource is not removable or it can not be removed due to other reasons.
 * If the resource is locked by any other thread the operation will fail.
 *
 * @param filepath         The file path of the file to be removed
 *
 * @return                 cOk on success. Error Code otherwise
 *
 */
 int             remove(const char* filepath);


 int             link(const char* oldpath, const char* newpath);

 int             frename(int fd, char* newname);

/*!
 * @brief Tries to seek inside the file descriptor.
 *
 * @param fd               The file descriptor to seek in
 * @param offset           The offset to seek. < 0 seeks backwards > 0 seeks forward
 * @param whence           The seek behavior:
 *                          SEEK_SET: seeks to the absolute position given by offset.
 *                          SEEK_CUR: seeks from the current position.
 *                          SEEK_END: seeks from the end of the file.
 */
 int             lseek(int fd, int offset, int whence);

/*!
 * @brief Creates a virtual device inside the /dev subsystem
 *
 * Creates a fifo file at the give file path. The fifo can be used to pipe
 * e.g. stdout of a task to this device, create virtual devices
 * for user space drivers etc.
 *
 * The maximum bufferSize depends on the OS configuration.
 * May fail if the requested bufferSize exceeds the allowed maximum.
 *
 * @param filepath         Path to the fifo including filename
 * @param bufferSize       Internal buffer size of the fifo
 *
 * @return                 cOk on success. Error code otherwise.
 */
 int             mkfifo(char* filepath, int bufferSize);

/*!
 * @brief Tries to mount a device/directory etc to the location given by dst_path.
 *
 * Tries to mount a device/directory etc to the location given by dst_path.
 *
 * @param src_path         The source to mount from. Maybe a directory, device etc.
 * @param dst_path         The destination to mount to.
 * @param type             The mount type.
 *
 * @return                 cOk on success. Error code otherwise.
 */
 int             mount(char* src_path, char* dst_path, int type);

/*!
 * Sequentially reads the entries of a directory given by its file descriptor.
 * The directory needs to be acquired using fopen first.
 *
 * This method is not thread safe. Thus reading two different directories
 * at the same time will result undefined behavior.
 *
 * Repeatingly calling this method yields the next directory entry until
 * all entries are read (returns 0).
 */
 Directory_Entry_t* readdir(int fd);

/*!
 * @brief Request an I/O Control operation on the device opened with handle 'fd'.
 *
 * @param fd          The opened devices handle.
 * @param request     The I/O Control request operation.
 * @param args        Argument to the I/O control operation.
 *                    Depends on operation. may be just an integer
 *                    or a pointer to a structure
 *
 */
 int        ioctl(int fd, int request, void* args);

/*!
 * Allows IO control of tasks input output streams.
 * CMD 0 = Set STDOUT
 */
 int        taskioctl(int cmd, int taskid, char* dev);


/*!
 * @brief Prints the string (without formatting) to standard out.
 */
 size_t     printToStdOut(const void* ptr, size_t max DEFAULT(256) );

 typedef size_t (*stdoutfn_t)(char*);

 /*!
  * @brief      Allows overriding the writing to stdout by writing to the given function.
  *             Setting fn to 0 reverts the function and writes to stdout will occur
  *             as if no function was set (to terminal, file or etc).
  *
  * @param fn   The function to be called on stdout write.
  */
 void       setstdout(stdoutfn_t fn);


/*!
 * @brief Maps a shared memory device into the address space of the calling task. If the cCreate flag is set
 * the shared memory area is created if it does not exists.
 *
 * Returns an error code: 0 (cOk) on success.
 *
 * @param file              File path of the shared memory area
 * @param mapped_address    return param: contains the virtual address the shared memory area is mapped to
 * @param mapped_size       return param: contains the size of the shared memory area
 * @param flags             Mapping flags:
 *                              cCreate             Create shared memory area if it does not exists
 *                                                  Will take the value of *mapped_size as the creation size
 *                                                  On success *mapped_size will contain the actual size of the area
 *
 * @return                 The file handle on success. Error code otherwise.
 *
 *  shm_map will use the filepath provided by file if the cCreate flag as follows:
 *  If a new shared memory area is created (file could not be found or is not a shared memory area) the filepath is stripped
 *  to its filename and used for the new shared memory resource.
 *   E.g., "/mem/mySharedMem" will result in a shared memory area name "mySharedMem" on creation inside the directory "/mem" => "/mem/mySharedMem"
 *         "/path/mySharedMem" will result in a shared memory area name "mySharedMem" on creation inside the directory "/mem" => "/mem/mySharedMem"
 *
 *  This results in any shared memory area being created inside the "/mem" directory!
 *
 *  To unmap the shared memory area use the returned file handle and call fclose().
 *  e.g.:
 *
 *  int handle = shm_map("/mem/myShared",&mapped_address,&mapped_size,0);
 *  fclose(handle);
 *
 */
 int          shm_map(const char* file, unint4* mapped_address, unint4* mapped_size, unint4 flags DEFAULT(0), unint4 offset DEFAULT(0));


/*!
 * @brief Unmap a mapped area by its adress
 *
 * @param address Address of the mapped area as returned by shm_map.
 * @return ErrorCode. cOk on success
 */
 int          shm_unmap(unint4* address);

/*!
 * @brief Returns the current time since startup in platform clock ticks.
 */
 unint8       getCycles();

/*!
 * @brief Returns the current time since startup in nanoseconds (ns)
 *
 */
 unint8       getTime();

/*!
 * @brief Returns the current DateTime in seconds since 1. Jan. 1970.
 *
 */
 unint4       getDateTime();

 /*!
  * @brief Set the current DateTime in seconds since 1. Jan. 1970.
  *
  */
 int         setDateTime(SetDateTime_t* dateTime);

/**************************************
 * Socket related system calls (IPC)
 **************************************/
/*!
 * @brief Create a new socket with adress family of type domain, socket type, and using the specified protocol.
 *
 * @param domain       The Domain of the socket to be created. Domains can be e.g IPV4,IPV6 or any other kind of implemented address family
 * @param type         The type of a socket may be connectionless or connection oriented
 * @param protocol     The protocol used for communication e.g TCP or any other protocol implemented in the os
 *
 * Successfully created sockets can be destroyed using the flose() syscall. For connection oriented sockets (SOCK_STREAM)
 * this will also close the connection. After calling fclose() on a socket file descriptor the socket can not be used
 * afterwards anymore. Thus, closing a connection implicitly destroys the socket.
 *
 * @return             Id of the created socket on success. Error Code otherwise
 */
 int         socket(int domain, int type, int protocol);


/*!
 * @brief Connect to a given destination on the provided socket.
 *
 * @param socket        The connection oriented socket to connect on
 * @param toaddress     The connection sock address
 * @param timeout       Connection establishment timeout in milliseconds. Must be > 0.
 *
 *  This call tries to connect to the given destination address using the socket specified. This call is only
 *  valid for connection oriented sockets (SOCK_STREAM) and will block the calling thread until a timeout occurs
 *  or the connection has been established. The timeout is not configurable and depends on the IP stack configuration.
 *
 * @return             cOk on success. Error Code otherwise.
 */
 int         connect(int socket, const sockaddr *toaddress, int timeout DEFAULT(2000) );

/*!
 * @brief Listen for incoming connections on a socket.
 *
 * @param socket       The connection oriented socket to listen on
 * @param backlog_size The number of accepted connections that can be buffered. Valid values: 0 < x < 50
 * @param timeout_ms   Timeout in ms for the listen operation.
 *
 *  Listen to the specified socket. This call is only valid for connection oriented sockets (SOCK_STREAM).
 *  This call is a blocking call and returns upon successful connection establishment.
 *
 *  A backlog buffer is created for the first call to this function on the specified socket. The backlog
 *  buffers incoming connections until they are received by further listen calls. Each listen call
 *  returning a valid socket frees one backlog buffer.
 *
 *  Connections accepted and lying on the backlog should be received in timely manner, as otherwise the connection
 *  may be closed by the other peer due to no reaction on the connection.
 *  The best design is to have a single listen thread waiting for incoming connections by looping on the listen call.
 *  Upon receiving a new connection another thread should be used to service the connection, thus allowing the listn thread to
 *  receive further connections.
 *
 *  The timeout value can be used to force the thread to return after a given time if no connection is established. On
 *  timeout cError is returned.
 *
 * @return             The file descriptor of the socket holding the new connection on success. Error Code otherwise.
 */
 int         listen(int socket, int backlog_size, int timeout_ms DEFAULT(0));

/*!
 * @brief Bind a socket to the address given by the parameter address.
 *
 * This call binds a socket to the sockaddr specified as parameter. This will on the one hand register the
 * socket for packet reception on the used transportprotocol and on the other hand set listening parameters in the
 * addressprotocol.
 *
 * @param socket_fd    The file descriptor of the socket that shall be bound
 * @param sockaddr     Pointer to the sockaddr structure containing the address the socket shall be bound to
 *
 * @return             cOk on success. Error Code otherwise.
 */
 int         bind(int socket_fd, const sockaddr* address);

/*!
 * @brief Blocking send method for sockets.
 *
 * Sends a message to another socket which address is given by sockaddr structure pointed to by dest_addr.
 * If the dest_addr is unknown the thread will be blocked until the destination is found using some ARP or a timeout occured.
 * If this method is used in a connection oriented socket the message will be send to the connected socket.
 *
 * @param socket_fd    The file descriptor of the socket the data shall be used for sending
 * @param data         Pointer to the data to be send
 * @param length       The amount of data the be send in the buffer
 * @param dest_addr    Pointer to the sockaddr structure containing the destination socket address or null if the connection is type STREAM
 *
 * @return             Length of sent data on success. Error Code otherwise: -1 on timeout
 */
 int        sendto(int socket_fd, const void* data, size_t length, const sockaddr* dest_addr);


 int        send(int socket_fd,  const void* data, size_t length);

/*!
 * @brief Receive method on socket.
 *
 * Receives a message from the specified socket. If no message is available the first thread trying to receive on this socket will be blocked. All
 * other threads will return directly without returning a message. The blocked thread will be unblocked if a message is received by the socket and the pointer
 * to the message is returned by the parameter 'msgptr'.
 *
 * @param socket       The file descriptor of the socket we want to receive data from
 * @param msgptr       The Address of the pointer to the message as return value
 * @param flags        flags: MSG_PEEK, MSG_WAIT
 * @param timeout      Timeout in us, if 0 no timeout is used. Values valid >= 200. Everything below 200 ms will be treated as 200 ms.
 *
 * @return             The size/length of the received message in bytes. -1 on connection disconnect
 */
 int      recv(int socket_fd, char* data,int len, int flags, unint4 timeout DEFAULT(0) );


/*!
 * @brief Receive method on socket which also returns the senders address.
 *
 * Receives a message from the specified socket. If no message is available the first thread trying to receive on this socket will be blocked. All
 * other threads will return directly without returning a message. The blocked thread will be unblocked if a message is received by the socket and the pointer
 * to the message is returned by the parameter 'msgptr'.
 *
 * @param socket       The socket we want to receive data from
 * @param msgptr       The Address of the pointer to the message as return value
 * @param flags        flags: MSG_PEEK, MSG_WAIT
 * @param sender       The sock_addr structure in memory that will be filled with the senders address
 *
 * @return             The size/length of the received message in bytes. -1 on connection disconnect
 */
int      recvfrom(int socket_fd, char* data, int len,int flags, sockaddr* sender, unint4 timeout DEFAULT(0) );


/*!
 * @brief Trys to find the service specified by a name in the network. Calling thread will be blocked.
 *
 * @param name         The name of the service to be found
 * @param descr        Pointer to service_descriptor structure that can be filled on success
 * @param n            The amount of services to find (maximum)
 *
 * @return             The amount of services found
 */
int         getservicebyname(char* name, const servicedescriptor* descr, unint1 n);

/*!
 * @brief Tries to do an IPv4 host name lookup for the host given by name
 *
 * @param name         The name of the host to be found
 *
 * @return             Pointer to the resulting statically allocated hostent structure.
 */
struct hostent* gethostbyname(const char* name);


char*    inet_ntop(int af, const char *src, char *dst, size_t size);

#ifdef __cplusplus
}
#endif

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define htons(n) __builtin_bswap16(n)
#define htonl(n) __builtin_bswap32(n)
#define ntohs(n) __builtin_bswap16(n)
#define ntohl(n) __builtin_bswap32(n)
#else
#define htons(n) n
#define htonl(n) n
#define ntohs(n) n
#define ntohl(n) n
#endif

#endif /*ORCOS_HH_*/
