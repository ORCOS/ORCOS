/*
 * BufferDevice.cc
 *
 *  Created on: 15.08.2014
 *     Copyright & Author: Daniel
 */

#include "hal/BufferDevice.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;


BufferDevice::BufferDevice(char* name, int bufferSize) :
        CharacterDevice((ResourceType) (cFifo | cStreamDevice), false, name) {

    buffersize = bufferSize;

    if (buffersize > BUFFERDEVICE_MAX_BUFFER_SIZE) {
        buffersize = BUFFERDEVICE_MAX_BUFFER_SIZE;
    }

    // For now: allocate in kernel space so every process can read this buffer
    // without mapping the space
    // TODO: change this to use the MemoryManager and map pages into the
    // the process acquiring this device on demand (just like the shared memory device)
    buffer        = new char[buffersize];
    usedBytes     = 0;
    readPointer   = 0;
    blockedThread = 0;
}

BufferDevice::~BufferDevice() {
    if (buffer) {
        delete (buffer);
        buffer = 0;
    }
    usedBytes     = 0;
    readPointer   = 0;
    blockedThread = 0;
    buffersize    = 0;
}

/*****************************************************************************
 * Method: BufferDevice::readBytes(char *bytes, unint4 &length)
 *
 * @description
 *******************************************************************************/
ErrorT BufferDevice::readBytes(char *bytes, unint4 &length) {
    if (!isValid()) {
        return (cError);
    }

    if (usedBytes == 0) {
        /* block until new data is available */
        blockedThread = pCurrentRunningThread;
        pCurrentRunningThread->block();
    }

    /* check again after unblocking ... if usedBytes is 0 again some
     * other thread must have retrieved the data first ... signal to thread
     * that this happened.. also a writeBytes of length 0 might have caused this
     * which is can be used as some form of synchronization*/
    if (usedBytes == 0) {
        length = 0;
        return (cOk);
    }

    if (length > usedBytes) {
        length = usedBytes;
    }

    /* now read length bytes and advance read pointer by
     * subtraction length */
    if (this->readPointer + length >= buffersize) {
        int copy1 = buffersize - readPointer;
        memcpy(bytes, &buffer[readPointer], copy1);
        memcpy(bytes + copy1, &buffer[0], length - copy1);
        readPointer = length - copy1;
    } else {
        memcpy(bytes, &buffer[readPointer], length);
        this->readPointer += length;
    }

    usedBytes -= length;

    return (cOk);
}

/*****************************************************************************
 * Method: BufferDevice::writeBytes(const char *bytes, unint4 length)
 *
 * @description
 *******************************************************************************/
ErrorT BufferDevice::writeBytes(const char *bytes, unint4 length) {
    if (!isValid())
        return (cError);

    int writeBytes = length;
    if (writeBytes + usedBytes > buffersize)
        writeBytes = buffersize - usedBytes;

    if (writeBytes == 0) {
        return (cDeviceMemoryExhausted);
    }

    if (this->position + writeBytes >= buffersize) {
        int copy1 = buffersize - this->position;
        memcpy(&buffer[this->position], bytes, copy1);
        memcpy(&buffer[0], bytes + copy1, writeBytes - copy1);
        this->position = writeBytes - copy1;
    } else {
        /* just copy*/
        memcpy(&buffer[this->position], bytes, writeBytes);
        this->position += writeBytes;
    }

    usedBytes += writeBytes;

    /* unblock waiting thread */
    if (blockedThread) {
        Thread* pThread = blockedThread;
        blockedThread = 0;
        /* unblock .. we may directly be preempted */
        pThread->unblock();

    }
    return (cOk );
}
