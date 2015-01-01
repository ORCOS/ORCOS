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

#ifndef CHARACTERDEVICEDRIVER_H_
#define CHARACTERDEVICEDRIVER_H_

#include "filesystem/Resource.hh"
#include "GenericDeviceDriver.hh"

/*!
 * \ingroup devicedrivers
 * \brief    CharacterDeviceDriver
 *             provides an interface for various character devices
 *             which can be used for example inside the kernel for outputting debug info
 */

class CharacterDevice: public GenericDeviceDriver {
protected:
    unint4 position;

public:
    CharacterDevice(bool sync_res, const char* name);

    /*!
     * Constructor for drivers inheriting a character device driver class
     */
    CharacterDevice(ResourceType rt, bool sync_res, const char* name);

    /*!
     * \brief Constructor for unregistered device drivers!
     *
     * Handles can not be retrieved by the filesystem
     */
    CharacterDevice() :  GenericDeviceDriver(cStreamDevice, false, 0) {
        position = 0;
    }


    /*****************************************************************************
     * Method: getPosition()
     *
     * @description
     *  Returns the current position of this characterdevice
     *******************************************************************************/
    unint4 getPosition() const { return (position); }


    virtual ~CharacterDevice();

    /*****************************************************************************
     * Method: readBytes(char *bytes, unint4 &length)
     *
     * @description
     * Reads a number of bytes from the device.
     *
     * Abstract function to read the bytes located given with the pointer *bytes
     * to the device. the number of bytes to be read is specified by the 2nd
     * parameter length, which is given by reference.
     * the function should alter the length parameter according to the number
     * of bytes which could be read.
     * if no data could be read, length should be set to 0 and an error value
     * is returned by the function.
      *******************************************************************************/
    virtual ErrorT readBytes(char *bytes, unint4 &length) {
        return (cNotImplemented);
    }

    /*****************************************************************************
     * Method: writeBytes(const char *bytes, unint4 length)
     *
     * @description
     * writes a number of bytes to the device
     *
     * abstract function to write a number of bytes to the device.
     * if the device could not accept as much bytes as specified by length
     * an error value is returned.
      *******************************************************************************/
    virtual ErrorT writeBytes(const char *bytes, unint4 length) {
        return (cNotImplemented );
    }

    /*****************************************************************************
     * Method: seek(int4 seek_value)
     *
     * @description
     * Seeks the position of the character device by the value 'seek_value'
     *
     * On some devices this might have an effect, as moving inside a file or block device.
     * Some other devices as e.g. serial devices may ignore the position.
     *
     * Block devices will interpret the seek_value as sectors NOT bytes!
      *******************************************************************************/
    virtual ErrorT seek(int4 seek_value) {
        /* avoid negative positions */
        if (this->position + seek_value < 0)
            this->position = 0;
        else
            this->position += seek_value;
        return (cOk );
    }


    /*****************************************************************************
     * Method: resetPosition()
     *
     * @description
     *  Resets the position of this character device.
      *******************************************************************************/
    virtual ErrorT resetPosition() {
        this->position = 0;
        return (cOk );
    }
};

#endif /*CHARACTERDEVICEDRIVER_H_*/
