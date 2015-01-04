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

#define END_OF_LIST       ((int)0xffffffff)
#define NOT_ON_OF_LIST    ((int)0xfffffffe)

#include "LockedQueue.hh"
#include "assemblerFunctions.hh"

LockedQueue::~LockedQueue() {

}

LockedQueue::LockedQueue(void* lqBaseAddress, int descAddress) {
    lqControls = (LockedQueueControl*) lqBaseAddress;
    descriptors = (Descriptor*) descAddress;
}

void LockedQueue::init(int nodeNr) {
    LockedQueueControl* lqControl = &lqControls[nodeNr];
    lqControl->lock = 0;
    lqControl->owner = nodeNr;
    lqControl->front = END_OF_LIST;
    lqControl->rear = END_OF_LIST;
}

void LockedQueue::lock(LockedQueueControl* lqControl) {
    //int* lockPtr = (int*) &lqControl->lock;
    //int lockValue = 1;

    /*asm volatile (
     "retry:                     ;"
     "swapa [%0] 1, %1\n;"
     "tst %1;"
     "bne retry;"
     "nop;"
     :
     :"r" (lockPtr), "r" (lockValue)
     :
     );*/
}

void LockedQueue::unlock(LockedQueueControl* lqControl) {
    lqControl->lock = 0;
}

void LockedQueue::add(int nodeNr, Descriptor* desc) {
    int index;
    LockedQueueControl* lqControl = &lqControls[nodeNr];
    desc->next = END_OF_LIST;
    desc->queue = lqControl->owner;
    index = desc->index;
    bool intEnabled = false;
    GET_INTERRUPT_ENABLE_BIT(intEnabled);
    _disableInterrupts();

    lock(lqControl);
    if (lqControl->front != END_OF_LIST) {
        descriptors[lqControl->rear].next = index;
    } else {
        lqControl->front = index;
    }

    lqControl->rear = index;
    unlock(lqControl);
    if (intEnabled) {
        _enableInterrupts();
    }

}

void LockedQueue::initDescriptors(int maxDescriptors, char* data) {
    for (int i = 0; i < maxDescriptors; i++) {
        descriptors[i].index = i;
        descriptors[i].data = (int) data;
        add(0, &descriptors[i]);
        data += MSG_BUFFER_SIZE;
    }
}

Descriptor* LockedQueue::getDescriptor(int nodeNr) {
    Descriptor* tmpDesc;
    int tmpFront;

    LockedQueueControl* lqControl = &lqControls[nodeNr];
    tmpDesc = 0;
    lock(lqControl);

    tmpFront = lqControl->front;
    if (tmpFront != END_OF_LIST) {
        tmpDesc = &descriptors[tmpFront];
        lqControl->front = tmpDesc->next;
        if (tmpDesc->next == END_OF_LIST) {
            lqControl->rear = END_OF_LIST;
        }
        tmpDesc->next = NOT_ON_OF_LIST;
    }

    unlock(lqControl);
    return (tmpDesc);
}

