/*
 * VirtualCharacterDevice.hh
 *
 *  Created on: 15.08.2014
 *      Author: Daniel
 */

#ifndef VIRTUALCHARACTERDEVICE_HH_
#define VIRTUALCHARACTERDEVICE_HH_

#include "hal/CharacterDevice.hh"

/*
 * The virtual character device is a class that offers a user space task
 * to create a character device inside the device subsystem to e.g.
 * create pipes between tasks for stdout.The device offers a small buffer to
 * write / read from in a stream fashion.
 *
 * E.g. Task 1 creates the device and polls the buffer using read().
 * It spawns another task and pipes the stdout into the device. The new task
 * will then write its data into this device using writes to stdout.
 *
 * The buffer is a standard ring buffer.
 */
class BufferDevice : public CharacterDevice {
private:
    char*       buffer;

    size_t      buffersize;

    /* bytes used starting from the readPointer */
    size_t      usedBytes;

    int         readPointer;

public:
    BufferDevice(char* name, int bufferSize);

    bool isValid() {return (buffer != 0); };

    virtual ~BufferDevice();

    ErrorT readBytes(char *bytes, unint4 &length);

    ErrorT writeBytes(const char *bytes, unint4 length);
};

#endif /* VIRTUALCHARACTERDEVICE_HH_ */
