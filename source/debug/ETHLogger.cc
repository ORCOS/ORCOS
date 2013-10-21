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

#include "ETHLogger.hh"
#include <stdarg.h>
#include <sprintf.hh>
#include "kernel/Kernel.hh"
#include "lib/defines.h"
#include "stringtools.hh"
#include "memtools.hh"

extern Kernel* theOS;
extern Kernel_ThreadCfdCl*		pCurrentRunningThread;
extern Task*        	pCurrentRunningTask;

//FATAL=0,ERROR=1,WARN=2,INFO=3,DEBUG=4,TRACE=5
static const char* levelStrings[ 6 ] = { "FATAL: ", "ERROR: ", "WARN:  ", "INFO:  ", "DEBUG: ", "TRACE: " };


char outbuffer[150];

#ifndef ETHLOGGER_DEST_IP
#define ETHLOGGER_DEST_IP IP4ADDR(192,168,1,6)
#endif

#ifndef ETHLOGGER_DEST_MAC
//#define ETHLOGGER_DEST_MAC {0x00,0x02,0xb3,0x27,0x2b,0x9b}
#define ETHLOGGER_DEST_MAC {0x00,0x02,0xb3,0x27,0x2b,0x9b}
#endif


ETHLogger::ETHLogger() {

	register ProtocolPool* protopool = theOS->getProtocolPool();

	//! The addressprotocol used
    aproto =  protopool->getAddressProtocolbyId( cIPv4AddressProtocol );

    //! The transportprotocol used
    tproto = protopool->getTransportProtocolbyId( cUDP );


    char mac[6] = ETHLOGGER_DEST_MAC;

    aproto->addNeighborInfo( ETHLOGGER_DEST_IP,(CommDeviceDriver*) theOS->getFileManager()->getResource("dev/comm/eth0"),(char*) &mac,6);

	// create socket for eth_logger
    // first get some buffer the socket can work on (storing messages and so on)
	char* mysocketbuffer = (char*) pCurrentRunningTask->getMemManager()->alloc( 800, true );

    // create new Socket
    mysock = new Socket(0x800,SOCK_STREAM,6,mysocketbuffer,800);
    pCurrentRunningTask->aquiredResources.addTail( (DatabaseItem*) mysock );

	// bind our socket to some address
	sockaddr* addr = (sockaddr*) pCurrentRunningTask->getMemManager()->alloc( (sizeof(sockaddr)), true );
	addr->port_data = 	80; 			        //< the port
	addr->sa_data = 	IP4ADDR(127,0,0,1);		//< our address
	mysock->bind(addr);

}
;


void ETHLogger::log( Prefix prefix, Level level, const char* msg, ... ) {
    if ( (int) level > (int) prefix ) {
        return;
    }

    va_list arglist;
    va_start(arglist, msg);

    int id = 0;
    if (pCurrentRunningThread != 0) id = pCurrentRunningThread->getId();

    char* out = (char*) &outbuffer;

    sprintf((char*) &outbuffer,"%s Thread: %d ",levelStrings[level],id );
    int i = strlen(outbuffer);


    out = (char*) ((unint4) &outbuffer) + i;
    print(&out,msg,arglist);
    i = strlen(outbuffer);
    outbuffer[i] = '\n';

#ifdef  ETHLOGGER_DEST_PORT
    sockaddr addr;
    addr.port_data = ETHLOGGER_DEST_PORT;
    //addr.sa_data = 	 IP4_ADDR(224,0,0,251);
    addr.sa_data = 	 ETHLOGGER_DEST_IP;
    addr.name_data[0] = '\0';

    packet_layer payload_layer;
    payload_layer.bytes = (char*) outbuffer;
    payload_layer.size = i+1;
    payload_layer.next = 0;
    payload_layer.total_size = i+1;

    if (tproto != 0 && aproto != 0)
		this->tproto->send( &payload_layer, this->aproto, this->mysock );

#endif
}
