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

#include "./types.h"
#include "defines.h"

extern "C" int 		syscall (int syscallnumber, ...);

extern "C" int      testandset(void* address, int testvalue, int setvalue);

/**************************************
 *  Memory related system calls
 **************************************/

/*!
 * \brief The new operator for user level
 */
void* operator 		new( size_t s );

/*!
 * \brief Memory allocation method.
 */
extern "C" void*	malloc(size_t s);

/*!
 * \brief Memory deallocation method
 */
extern "C" void 	free(void *s);

/*!
 * \brief The delete operator for user level
 */
void operator 		delete( void* ptr );



/**************************************
 *  Task related system calls
 **************************************/

/*!
 * \brief Start a new task given by its file path.
 *
 * \param path 		The path to the task file to be executed
 * \param arguments A null terminated string of arguments
 *
 * \return 			Error Number < 0 or task id > 0
 */
extern "C" int 		task_run(char* path, char* arguments);


/*!
 * \brief Stops and removes a task from the system.
 *
 * \param taskid 	The ID of the task to be removed
 *
 * \return 			Error Number < 0 or 0 (on success)
 */
extern "C" int 		task_kill(int taskid);

/*!
 * \brief Stop the specified task. It could be resumed later on.
 *
 * \param taskid the id of the task that should be stopped.
 *
 * \return 			Error Number
 */
extern "C" int 		task_stop(int taskid);

/*!
 * \brief resumes the specified task. Resumes execution of a task that is currently not running.
 *
 *
 * \param taskid the id of the task that should be resumed.
 *
 * \return 			Error Number
 */
extern "C" int 		task_resume(int taskid);

/**************************************
 *  Thread related system calls
 **************************************/


/*!
 * \brief wait method.  Blocks the current calling thread until any other thread inside the current process terminates.
 *
 *  The method blocks the current thread until any other thread of the current process terminates. If no other thread exists
 *  that might terminate this can lead to a endlessly blocked thread. However, another thread from a different process
 *  may unblock the stuck thread if it issues the correct signal_signal syscall.
 *  e.g.: signal_signal(ProcessID_To_Unblock() << 16) | (SIG_CHILD_TERMINATED).
 *  This is possible as orcos uses signals for the wait operation.
 *
 * \return			The exit value of the terminated thread.
 */
extern "C" int 		wait();

/*!
 * \brief wait method.  Blocks the current calling thread until the process terminates.
 *
 *  The method blocks the current thread until the specified process terminates.  If no such process exists
 *  this can lead to a endlessly blocked thread. However, another thread from a different process
 *  may unblock the stuck thread if it issues the correct signal_signal syscall.
 *  e.g.: signal_signal(pid).
 *  This is possible as orcos uses signals for the wait operation.
 *
 * \return			The exit value of the terminated process.
 */
extern "C" int		waitpid(unint1 pid);

/*!
 *  \brief Sleep method. The calling thread will be blocked for at least 'ms' milliseconds.
 *
 * Real-Time threads are guaranteed to wake up after the exact (except latency) number of milli-
 * seconds. If they have the highest priority they will be executed directly.
 *
 * \param ms	The amount of milliseconds to sleep
 */
extern "C" void 	sleep(int ms);

/*!
 *  \brief The calling thread will be blocked for at least 'us' micro-seconds.
 *
 * Real-Time threads are guaranteed to wake up after the exact (except latency) number of micro-
 * seconds. If they have the highest priority they will be executed directly.
 *
 * \param us	The amount of microseconds to sleep
 */
extern "C"void 		usleep(int us);

/*!
 * \brief Create a new thread.
 *
 * \param threadid 		Return value which will hold the thread id on success
 * \param attr	 		Pointer to the thread attributes structure holding information for thread creation
 * \param start_routine	Pointer to the executeable code of the thread
 * \param arg			Argument passed to the thread on execution
 *
 * \return				Error Number
 */
extern "C" int 		thread_create(int* threadid,thread_attr_t* attr, void *(*start_routine)(void*), void* arg);

/*!
 * \brief Start the execution of the thread with id 'threadid'.
 *
 * \param threadid the id of the thread to run. If the threadid is <= 0 all newly created threads of the current task will be started simultaneously!
 *        This is important if you want multiple realtime threads to arrive at the same time so that the schedule is not phase shifted!
 *
 * \return 			Error Number
 */
extern "C" int 		thread_run(int threadid);

/*!
 * \brief Returns the id of the currently running thread
 */
extern "C" int 		thread_self();

/*!
 * \brief Voluntarily yield the cpu to some other thread (if any)
 */
extern "C" void 	thread_yield();

/*!
 * \brief Terminates the currently executing thread
 *
 * If a Memory Manager is used that does not support deallocation of memory the TCB and stack will be kept for further thread creation calls.
 */
extern "C" void 	thread_exit(int exitCode = cOk);



/**************************************
 *  Signal related system calls
 **************************************/

extern "C" int 			signal_wait( void* sig, bool memAddrAsSig = false );

extern "C" void 		signal_signal( void* sig, int value, bool memAddrAsSig = false );



/**************************************
 *  Character device related system calls
 **************************************/

/*
 * COMMENT:
 * Handling character device syscalls is handled quite similiar to the posix
 * standard http://www.opengroup.org/onlinepubs/009695399/ although the implemenation
 * is not 100% posix standard.
 */

extern "C" int 		fcreate(const char* filename, const char* path);

/*!
 * \brief  The fopen() function shall open the resource whose pathname is the string pointed to by filename, and associates a stream with it.
 *
 * \param filename The resource to be opened given by its path (e.g "/dev/testdev")
 * \param blocking If the resource is owned by another task and this is set to 1 the OS will block the current thread until the resource is available
 *
 * \return 			The file descriptor on success. Error Code otherwise.
 */
extern "C" int 		fopen(const char* filename, int blocking = 1);

/*!
 * \brief Close the stream associated with this file handle. May fail if this resource was not aquired
 *
 * \param fileid The file descriptor as returned by fopen() of the resource/file to be closed.
 *
 * \return			cOk on success. Error Code  otherwise.
 */
extern "C" int 		fclose(int fd);

/*!
 * \brief The fread() function shall read into the array pointed to by ptr up to nitems elements whose size is specified by size in bytes, from the stream pointed to by stream.
 *
 * \param  ptr 		The array into which the data shall be written
 * \param  size 	The amount of data to be read
 * \param  nitems 	The amount of items that shall be read (each item has the size 'size')
 * \param  stream 	The id of the stream which shall be read
 *
 * \return 			The amount of bytes read
 */
extern "C" size_t 	fread(void *ptr, size_t size, size_t nitems, int stream);

/*!
 * \brief The fputc() function shall write the byte specified by c (converted to an unsigned char) to the output stream pointed to by stream, at the position indicated by the associated file-position indicator for the stream (if defined), and shall advance the indicator appropriately. If the file cannot support positioning requests, or if the stream was opened with append mode, the byte shall be appended to the output stream.
 *
 * \param c 		The byte/character to be written
 * \param fd 		The file descriptor to be written to
 *
 * \return			Error Number
 */
extern "C" int 		fputc(short c, int fd);

/*!
 * \brief Get a byte from a stream
 *
 * \param fd		The file descriptor to be read from
 *
 * \return			The byte read
 */
extern "C" int 		fgetc(int fd);

/*!
 * \brief The fwrite() function shall write, from the array pointed to by ptr, up to nitems elements whose size is specified by size, to the stream pointed to by stream.
 *
 * \param ptr		Pointer to the data that shall be written
 * \param size		The size of each data element to be written
 * \param nitems	The amount of data elements to be written
 * \param fd		The file descriptor of the stream
 *
 * \return			Amount of data written on success
 */
extern "C" size_t 	fwrite(const void *ptr, size_t size, size_t nitems, int fd);


/*!
 * \brief The fstat method returns the file statistics of a resource given by its handle fd.
 *
 * \param fd		The file descriptor as returned by fopen()
 * \param stat		Pointer to a stat_t structure that will be filled with the file statistics.
 * \return			cOk on success. Error Code otherwise
 */
extern "C" int 		fstat(int fd, stat_t* stat);


/*!
 * \brief Tries to remove a resource given by its absolute path.
 *
 * The method fails (return value < 0) if the resource can not be found,
 * the resource is not removable or it can not be removed due to other reasons.
 * If the resource is locked by any other thread the operation will fail.
 *
 * \param filename	The name of the file to be removed inside the given path
 * \param path		The path the file to be removed is located in
 * \return 			cOk on success. Error Code otherwise
 *
 */
extern "C" int 		fremove(const char* filename, const char* path);


/*!
 * \brief Request an I/O Control operation on the device opened with handle 'fd'.
 *
 * \param fd		The opened devices handle.
 * \param request	The I/O Control request operation.
 * \param args		Argument to the I/O control operation.
 * 					Depends on operation. may be just an integer
 * 					or a pointer to a structure
 *
 */
extern "C" int		ioctl(int fd, int request, void* args);

/*!
 * \brief The fwrite() function writes the zero terminted string pointed to by prt to the stream referenced by stream. A maximum of max characters are written, which default
 * is 256
 *
 * \param ptr 		Pointer to the string that shall be written
 * \param stream	The id of the stream
 * \param max		Maximum length of the string to be written (optional)
 *
 * \return 			Amount of bytes written
 */
extern "C" size_t 	fwriteString(const void *ptr, int stream, size_t max = 256);


/*!
 * \brief Prints the string (without formatting) to standard out.
 */
extern "C" size_t 	printToStdOut(const void* ptr,size_t max = 256);


/*!
 * \brief The map_memory() function asks the kernel to map the desired physical address space to the logical address space specified
 *
 * \param log_start 	Logical address space
 * \param phy_start		Physical address space
 * \param size 			Size of the address space
 * \param protection	Protection mode of the address space
 *
 * \return 				cOk on success, Error Code  otherwise
 */
extern "C" int 		map_logmemory( const char* log_start, const char* phy_start, size_t size, int protection);


/*!
 * \brief Maps a shared memory device into the address space of the calling task.
 *
 * Returns an error code: 0 (cOk) on success.
 *
 * \param mapped_address 	return param: contains the virtual address the shared memory are is mapped to
 * \param mapped_size		return param: contains the size of the shared memory area
 *
 */
extern "C" int 		shm_map(const char* file,unint4* mapped_address, unint4* mapped_size);

/*!
 * \brief Returns the current time since startup in platform clock ticks.
 */
extern "C" unint8 	getTime();

/**************************************
 * Socket related system calls (IPC)
 **************************************/
/*!
 * \brief Create a new socket with adress family of type domain, socket type, and using the specified protocol.
 *
 * \param domain 		The Domain of the socket to be created. Domains can be e.g IPV4,IPV6 or any other kind of implemented address family
 * \param type			The type of a socket may be connectionless or connection oriented
 * \param protocol		The protocol used for communication e.g TCP or any other protocol implemented in the os
 *
 * Successfully created sockets can be destroyed using the flose() syscall. For connection oriented sockets (SOCK_STREAM)
 * this will also close the connection. After calling fclose() on a socket file descriptor the socket can not be used
 * afterwards anymore. Thus, closing a connection implicitly destroys the socket.
 *
 * \return				Id of the created socket on success. Error Code otherwise
 */
extern "C" int 		socket(int domain, int type, int protocol);


/*!
 * \brief Connect to a given destination on the provided socket.
 *
 * \param socket 		The connection oriented socket to connect on
 * \param toaddress		The connection sock address
 *
 *  This call tries to connect to the given destination address using the socket specified. This call is only
 *  valid for connection oriented sockets (SOCK_STREAM) and will block the calling thread until a timeout occurs
 *  or the connection has been established. The timeout is not configurable and depends on the IP stack configuration.
 *
 * \return				cOk on success. Error Code otherwise.
 */
extern "C" int 		connect(int socket, const sockaddr *toaddress);

/*!
 * \brief Listen for connection on a socket.
 *
 * \param socket 	The connection oriented socket to listen on
 *
 *  Listen to the specified socket. This call is only valid for connection oriented sockets (SOCK_STREAM).
 *  This call is a blocking call and returns upon successful connection establishment.
 *
 * \return			The file descriptor of the socket on success. Error Code otherwise.
 */
extern "C" int 		listen(int socket);

/*!
 * \brief Bind a socket to the address given by the parameter address.
 *
 * This call binds a socket to the sockaddr specified as parameter. This will on the one hand register the
 * socket for packet reception on the used transportprotocol and on the other hand set listening parameters in the
 * addressprotocol.
 *
 * \param socket	The file descriptor of the socket that shall be bound
 * \param sockaddr	Pointer to the sockaddr structure containing the address the socket shall be bound to
 *
 * \return			cOk on success. Error Code otherwise.
 */
extern "C" int 		bind(int socket, const sockaddr* address);

/*!
 * \brief Blocking send method for unconnected sockets.
 *
 * Sends a message to another socket which address is given by sockaddr structure pointed to by dest_addr.
 * If the dest_addr is unknown the thread will be blocked until the destination is found using some ARP or a timeout occured.
 * If this method is used in a connection oriented socket the message will be send to the connected socket.
 *
 * \param socket	The id of the socket the data shall be used for sending
 * \param data		Pointer to the data to be send
 * \param length	The amount of data the be send in the buffer
 * \param dest_addr Pointer to the sockaddr structure containing the destination socket address or null if the connection is type STREAM
 *
 * \return			Length of sent data on success. Error Code otherwise: -1 on timeout
 */
extern "C" int4 	sendto(int socket, const void* data, size_t length, const sockaddr* dest_addr);

/*!
 * \brief Receive method on socket.
 *
 * Receives a message from the specified socket. If no message is available the first thread trying to receive on this socket will be blocked. All
 * other threads will return directly without returning a message. The blocked thread will be unblocked if a message is received by the socket and the pointer
 * to the message is returned by the parameter 'msgptr'.
 *
 * \param socket	The socket we want to receive data from
 * \param msgptr 	The Address of the pointer to the message as return value
 * \param flags		flags: MSG_PEEK, MSG_WAIT
 *
 * \return			The size/length of the received message in bytes. -1 on connection disconnect
 */
extern "C" size_t 	recv(int socket,char* data,int len, int flags);


/*!
 * \brief Receive method on socket which also returns the senders address.
 *
 * Receives a message from the specified socket. If no message is available the first thread trying to receive on this socket will be blocked. All
 * other threads will return directly without returning a message. The blocked thread will be unblocked if a message is received by the socket and the pointer
 * to the message is returned by the parameter 'msgptr'.
 *
 * \param socket    The socket we want to receive data from
 * \param msgptr    The Address of the pointer to the message as return value
 * \param flags     flags: MSG_PEEK, MSG_WAIT
 * \param sender    The sock_addr structure in memory that will be filled with the senders address
 *
 * \return          The size/length of the received message in bytes. -1 on connection disconnect
 */
extern "C" size_t 	recvfrom(int socket,char* data, int len,int flags, sockaddr* sender);

/*!
 * \brief Trys to find the service specified by a name in the network. Calling thread will be blocked.
 *
 * \param name		The name of the service to be found
 * \param descr		Pointer to service_descriptor structure that can be filled on success
 * \param n			The amount of services to find (maximum)
 *
 * \return			The amount of services found
 */
extern "C" int 		getservicebyname(char* name, const servicedescriptor* descr, unint1 n);



#endif /*ORCOS_HH_*/
