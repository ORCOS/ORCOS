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

#include "ShmDriver.hh"
#include <assembler.h>
#include "kernel/Kernel.hh"
#include "assemblerFunctions.hh"
#include <inc/assembler.h>
#include "comm/AddressProtocol.hh"
#include <inc/memtools.hh>

//length of shared memory area

#define MAX_DESC 14

extern Kernel* theOS;
unint4* buffer_start = 0;
int buffer_pos = -1;
int buffer_len = 0;

ShmDriver::~ShmDriver() {
}

ShmDriver::ShmDriver(const char* name, int4 a) :
        CommDeviceDriver(name) {
    // base address of shared memory area
    baseAddr = SHM_START;

    // CPU count starts with 0,
    // node count starts with 1
    GET_CPU_INDEX(LocalNodeNr);
    LocalNodeNr++;

    // base address of locked queue controls
    int lqAddress = a + (MAX_NODES + 1) * sizeof(NodeInfo);

    // base address of descriptors
    descAddress = lqAddress + (MAX_NODES + 1) * sizeof(LockedQueueControl);

    lockedQueue = new LockedQueue((void*) lqAddress, descAddress);

    init();
}

void ShmDriver::init() {
    bool isMaster = false;

    if (LocalNodeNr == 1) {
        isMaster = true;
    }

    nodeStatuses = (NodeInfo*) baseAddr;
    localNodeStatus = &nodeStatuses[LocalNodeNr];

    int i = 0;

    LOG(KERNEL, INFO, (KERNEL, INFO, "ShmDriver::init() Waiting for Nodes to initialize Memory area"));

    if (isMaster) {

        //clear shared memory area
        volatile int* addr = (volatile int*) baseAddr;

        for (i = 1; i <= 0x80000; i++) {
            *addr = 0;
            addr++;
        }

        /*
         * Initialize the locked queues controls for the free descriptor pool
         * and the receive queues and set node statuses to pending
         */
        for (i = 0; i <= MAX_NODES; i++) {
            lockedQueue->init(i);
            (&nodeStatuses[i])->status = initPending;
        }

        /*
         *  Initialize all descriptors.
         */
        buffer_start = (unint4*) (descAddress + (sizeof(Descriptor) * MAX_DESC));
        LOG(KERNEL, INFO, (KERNEL, INFO, "ShmDriver::init() Buffer Start: %x:", buffer_start));
        lockedQueue->initDescriptors(MAX_DESC, (char*) buffer_start);

        localNodeStatus->irqAddress = IRQ_FORCE_REG + (4 * (LocalNodeNr - 1));
        localNodeStatus->irqValue = 1 << SHM_IRQ;

        localNodeStatus->status = initComplete;

        /*
         *  Loop until all nodes have completed initialization.
         */

        short allInitialized;
        do {
            allInitialized = 1;

            for (int i = 1; i <= MAX_NODES; i++)
                if ((&nodeStatuses[i])->status != initComplete) {
                    allInitialized = 0;
                }

        } while (allInitialized == 0);

        LOG(KERNEL, INFO, (KERNEL, INFO, "ShmDriver::init() All nodes (%d) finished.", MAX_NODES));

        /*
         *  Tell the other nodes the system is up.
         */
        for (i = 1; i <= MAX_NODES; i++)
            (&nodeStatuses[i])->status = nodeActive;
    } else {

        localNodeStatus->status = initPending;

        do {

            if (localNodeStatus->status == initPending) {

                /*
                 *  Initialize this node's interrupt information in the
                 *  shared area so other nodes can interrupt us.
                 */

                localNodeStatus->irqAddress = IRQ_FORCE_REG + (4 * (LocalNodeNr - 1));
                localNodeStatus->irqValue = 1 << SHM_IRQ;

                localNodeStatus->status = initComplete;
            }
        } while (localNodeStatus->status != nodeActive);
    }

}

int ShmDriver::getIRQ() {
    return SHM_IRQ;
}

/*---------------------------------------------------------------------------*/
ErrorT ShmDriver::enableIRQ()
/*---------------------------------------------------------------------------*/
{
    return cOk ;
}

/*---------------------------------------------------------------------------*/
ErrorT ShmDriver::disableIRQ()
/*---------------------------------------------------------------------------*/
{
    return cOk ;
}

/*---------------------------------------------------------------------------*/
ErrorT ShmDriver::writeBytes(const char* bytes, int4 length)
/*---------------------------------------------------------------------------*/
{
    return cOk ;
}

void ShmDriver::sendByte(byte Data)
/*---------------------------------------------------------------------------*/
{

}

/*---------------------------------------------------------------------------*/
bool ShmDriver::isTransmitBufferFull()
/*---------------------------------------------------------------------------*/
{
    return false;
}

bool ShmDriver::isReceiveBufferFull() {
    return false;
}

/*---------------------------------------------------------------------------*/
bool ShmDriver::hasPendingData()
/*---------------------------------------------------------------------------*/
{
    return false;
}

/*---------------------------------------------------------------------------*/
byte ShmDriver::recvByte()
/*---------------------------------------------------------------------------*/
{
    return false;
}

ErrorT ShmDriver::send(packet_layer* packet, char* dest_addr, int addr_len, int2 fromProtocol_ID) {
    ASSERT(packet != 0);
    if (packet->total_size + 2 > MSG_BUFFER_SIZE) {
        LOG(KERNEL, WARN, (KERNEL, WARN, "ShmDriver::send(): Packet size (%d) exceeds message buffer size (%d). Packet discarded.", packet->total_size+2, MSG_BUFFER_SIZE));
        return cError ;
    }

    // get a free descriptor
    Descriptor* desc = lockedQueue->getDescriptor(0);
    if (desc == 0) {
        LOG(KERNEL, WARN, (KERNEL, WARN, "No free descriptors available. Discarding message."));
        return 0;
    }

    // get the length of all layers
    int len = packet->total_size + 2;

    // copy protocol id to the begin of the packet
    memcpy((char*) desc->data, &fromProtocol_ID, 2);
    int pos = 2;

    do {
        memcpy((char*) desc->data + pos, packet->bytes, packet->size);
        pos += packet->size;
        packet = packet->next;

    } while (packet != 0);

    LOG(KERNEL, DEBUG, (KERNEL, DEBUG, "ShmDriver::send() packet length: %d destination: 0x%x descr:%x", len, *((int2* ) dest_addr), desc));

    // cast as int2 since this is what we specified as mac address size!
    int2 destNode = ((int2*) dest_addr)[0];

    if (destNode > MAX_NODES)
        return cError ;

    desc->length = len;

    // add the descriptor to the queue of the destination node
    lockedQueue->add(destNode, desc);
    volatile NodeInfo* destNodeStatus = &nodeStatuses[destNode];

    // cause interrupt on destination node
    OUTW(destNodeStatus->irqAddress, destNodeStatus->irqValue);

    return cOk ;
}

void ShmDriver::recv() {
    Descriptor * desc = 0;

    do {
        desc = lockedQueue->getDescriptor(LocalNodeNr);
        LOG(KERNEL, DEBUG, (KERNEL, DEBUG, "ShmDriver::recv(): packet received. descr:%x", desc));
        if (desc) {
            // pass to protocol
            LOG(KERNEL, DEBUG, (KERNEL, DEBUG, "ShmDriver::recv(): packet Length: %d", desc->length));
            if (desc->length > MSG_BUFFER_SIZE) {
                LOG(KERNEL, WARN, (KERNEL, WARN, "ShmDriver::recv(): Packet size (%d) exceeds message buffer size (%d). Packet discarded.", desc->length, MSG_BUFFER_SIZE));
                return;
            }
            memcpy((void*) bytes, (void*) desc->data, desc->length);
            int2 protocolNum = ((int2*) ((int) bytes))[0];
            //add descriptor to free descriptor queue
            lockedQueue->add(0, desc);

            AddressProtocol* aproto = theOS->getProtocolPool()->getAddressProtocolbyId(protocolNum);

            if (aproto != 0) {
                aproto->recv(((char*) bytes) + 2, desc->length - 2, this);
            }
        }
    } while (desc != 0);

    // clear interrupt
    //unint4 irqreg = IRQ_FORCE_REG + (4* (LocalNodeNr-1));

    //OUTW(irqreg,-1 );

    interruptPending = false;
}
ErrorT ShmDriver::broadcast(packet_layer* packet, int2 fromProtocol_ID) {
    for (int2 i = 1; i <= MAX_NODES; i++) {
        if (i != LocalNodeNr) {
            send(packet, (char*) &i, 2, fromProtocol_ID);
        }
    }
    return cOk ;

}

ErrorT ShmDriver::multicast(packet_layer* packet, int2 fromProtocol_ID, unint4 dest_addr) {
    return broadcast(packet, fromProtocol_ID);
}

const char* ShmDriver::getMacAddr() {
    return (char*) &LocalNodeNr;
}

int1 ShmDriver::getMacAddrSize() {
    return 2;
}

/*---------------------------------------------------------------------------*/
ErrorT ShmDriver::readByte(char* byteptr)
/*---------------------------------------------------------------------------*/
{
    return cOk ;
}

/*---------------------------------------------------------------------------*/
ErrorT ShmDriver::readBytes(char* bytes, int4 &length)
/*---------------------------------------------------------------------------*/
{
    return cOk ;
}

ErrorT ShmDriver::writeByte(char byte)
/*---------------------------------------------------------------------------*/
{
    return cOk ;
}
