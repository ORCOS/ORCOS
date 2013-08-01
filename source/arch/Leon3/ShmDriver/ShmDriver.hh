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

#ifndef SHMDRIVER_HH
#define SHMDRIVER_HH

#define IRQ_FORCE_REG	0x80000280
#define IRQ_EXT_REG		0x800002C0
#define SHM_IRQ			4

#include <hal/CommDeviceDriver.hh>
#include "comm/CAB.hh"

#include "LockedQueue.hh"
#include "shm.hh"


/*!
 *  \brief Driver for Shared Memory Communication
 *
 */
class ShmDriver : public CommDeviceDriver
{

private:

	char bytes[MSG_BUFFER_SIZE];

  volatile byte inputPending;

  int4  	baseAddr;
  int2		LocalNodeNr;
  int		descAddress;

  volatile NodeInfo* nodeStatuses;

  volatile NodeInfo* localNodeStatus;

  LockedQueue* lockedQueue;

  void		init();

  void		sendByte(byte Data);

  byte		recvByte();

  bool		isTransmitBufferFull();

  bool		isReceiveBufferFull();


protected:

public:

  //!  constructor
	ShmDriver(const char *name, int4 a);

  //!  destructor
  ~ShmDriver();

  int 			getIRQ();
  ErrorT        enableIRQ();

  ErrorT        disableIRQ();

  /// interface to meet the CharacterDeviceDriver
  ErrorT 		readByte  (char* byte);
  ErrorT 		writeByte (char byte);
  ErrorT 		readBytes (char *bytes, int4 &length);
  ErrorT 		writeBytes(const char *bytes, int4 length);



  /// interface from the CommDeviceDriver
  /// method which gets called whenver this devices throws a extern IRQ
  void 			recv ();

  /// send method which sends the bytes given to the destination addr
  /// ignores the destination addr, len since they are not needed on a 1-1 connection
  /// as serial line connections are
  ErrorT 		send  (packet_layer* packet, char* dest_addr,int addr_len, int2 fromProtocol_ID);
  ErrorT		lowlevel_send( char* data, int len ) {return cError;};

  ErrorT 		broadcast(packet_layer* packet, int2 fromProtocol_ID);

  ErrorT 		multicast( packet_layer* packet, int2 fromProtocol_ID, unint4 dest_addr );


  //! returns the mac address of this device
  const char* getMacAddr();

  //! returns the size (amount of bytes) mac addresses have on this device
  int1 getMacAddrSize();

  /// Received FIFO has vaild data. Used to distinguish between IRQs.
  bool          hasPendingData();

  ///  Timeout of -1 causes it to wait indefinitely until something is read.
  ErrorT        outputSCC(int4 Timeout, byte c);

  ErrorT        inputSCC(int4 Timeout, byte *c);

};

#endif /* ShmDriver_HH */
