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

#include "GenericDeviceDriver.hh"

/*!
 * \ingroup devicedrivers
 * \brief	CharacterDeviceDriver
 * 			provides an interface for various character devices
 * 			which can be used for example inside the kernel for outputting debug info
 */

class CharacterDeviceDriver: public GenericDeviceDriver {

protected:
	unint4 position;

public:

    CharacterDeviceDriver( bool sync_res, const char* name );

    /*!
     * Constructor for drivers inheriting a character device driver class
     */
    CharacterDeviceDriver( ResourceType rt, bool sync_res, const char* name );

    /*!
     * \brief Constructor for unregistered device drivers!
     *
     * Handles can not be retrieved by the filesystem
     */
    CharacterDeviceDriver() : GenericDeviceDriver(0,"none") { position=0;};

    virtual ~CharacterDeviceDriver();

    /*!
     * \brief reads a byte from the device
     *
     * abstract function to read a byte from the device and writes it
     * to the supplied pointer a error is returned when no byte is available.
     */
    virtual ErrorT readByte( char* byte ) {
        return cNotImplemented;
    }

    /*!
     * \brief writes a byte to the device
     *
     * abstract function to write a byte to the device
     * if not possible an error is returned.
     */
    virtual ErrorT writeByte( char byte ) {
        return cNotImplemented;
    }

    /*!
     * \brief reads a number of bytes from the device
     *
     * abstract function to read the bytes located given with the pointer *bytes
     * to the device. the number of bytes to be read is specified by the 2nd
     * parameter length, which is given by reference.
     * the function should alter the length parameter according to the number
     * of bytes which could be read.
     * if no data could be read, length should be set to 0 and an error value
     * is returned by the function.
     */
    virtual ErrorT readBytes( char *bytes, unint4 &length ) {
        return cNotImplemented;
    }

    /*!
     * \brief writes a number of bytes to the device
     *
     * abstract function to write a number of bytes to the device.
     * if the device could not accept as much bytes as specified by length
     * an error value is returned.
     */
    virtual ErrorT writeBytes( const char *bytes, unint4 length ) {
        return cNotImplemented;
    }


    /*!
     * \brief Seeks the position of the character device by the value 'seek_value'
     *
     * On some devices this might have an effect, as moving inside a file or block device.
     * Some other devices as e.g. serial devices may ignore the position.
     *
     * Block devices will interpret the seek_value as sectors NOT bytes!
     */
    virtual ErrorT seek(int4 seek_value) { this->position += seek_value; return cOk; };


    virtual ErrorT resetPosition() { this->position = 0; return cOk; }


};

#endif /*CHARACTERDEVICEDRIVER_H_*/
