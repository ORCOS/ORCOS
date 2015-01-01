/*
 * VirtualCharacterDevice.cc
 *
 *  Created on: 15.08.2014
 *     Copyright & Author: Daniel
 */

#include "hal/BufferDevice.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;

#define MAX_BUFFER_SIZE 1024

BufferDevice::BufferDevice(char* name, int bufferSize) :
        CharacterDevice(false, name) {
    buffersize = bufferSize;
    if (buffersize > MAX_BUFFER_SIZE)
        buffersize = MAX_BUFFER_SIZE;

    buffer      = new char[buffersize];
    usedBytes   = 0;
    readPointer = 0;
}

BufferDevice::~BufferDevice() {
}

/*****************************************************************************
 * Method: BufferDevice::readBytes(char *bytes, unint4 &length)
 *
 * @description
 *******************************************************************************/
ErrorT BufferDevice::readBytes(char *bytes, unint4 &length) {
    if (!isValid())
        return (cError );
    if (usedBytes == 0) {
        length = 0;
        return (cOk );
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

    return (cOk );
}

/*****************************************************************************
 * Method: BufferDevice::writeBytes(const char *bytes, unint4 length)
 *
 * @description
 *******************************************************************************/
ErrorT BufferDevice::writeBytes(const char *bytes, unint4 length) {
    if (!isValid())
        return (cError );

    int writeBytes = length;
    if (writeBytes + usedBytes > buffersize)
        writeBytes = buffersize - usedBytes;

    if (writeBytes == 0)
        return (cDeviceMemoryExhausted );

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
    return (cOk );
}
