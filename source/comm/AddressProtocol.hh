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

#ifndef ADDRESSPROTOCOL_HH_
#define ADDRESSPROTOCOL_HH_

#include "inc/types.hh"
#include "inc/const.hh"
#include "hal/CommDeviceDriver.hh"
#include "filesystem/Directory.hh"
#include "inc/types.hh"

class Socket;
class TransportProtocol;

/*!
 * \brief The AddressProtocol Base Class.
 * \ingroup comm
 *
 * Base class of all addressprotocols. Stores its id and a reference to the directory containing
 * all communication devices. All methods are needed for a valid addressprotocol and must be overwritten.
 *
 */
class AddressProtocol {
protected:
    //! The Id of this protocol used in mac frames
    int2 id;
    //! Pointer to directory containing all communication devices
    Directory* commdevsdir;

public:
    AddressProtocol(int2 i_protocol_id, Directory* p_commdevsdir) {
        this->id = i_protocol_id;
        this->commdevsdir = p_commdevsdir;
    }
    ;

    virtual ~AddressProtocol() {
    }
    ;

    //! Returns the Id of this protocol
    int2 getId() {
        return (id);
    }
    ;

    /*!
     * \brief Send method which needs to be implemented by the protocol
     *
     * \param buffer		Pointer to the beginning of a buffer the protocol can work on.
     * \param buffersize	The size/length of the buffer in bytes.
     * \param msgstart		Pointer to the start of the message (inside the buffer) that wants to be send.
     * \param msglength		Lengh of the message to be send.
     * \param dest_addr		Pointer to the destination the message shall be delivered to. Length is not needed since it is known by the protocol.
     * \param fromProto		The Transportprotocol (Layer above) the message is coming from.
     */
    virtual ErrorT send(packet_layer* upperload, unint4 dest_addr, TransportProtocol* fromProto) {
        return (cNotImplemented );
    }

    /*!
     *  \brief Receive method which needs to be implemented by the protocol
     *
     * \param packetstart	Pointer to the start of the packet the protocol received.
     * \param packetlength	Length of the packet.
     * \param fromDevice	The communication device the packet was received on.
     */
    virtual ErrorT recv(char* packetstart, int packetlength, CommDeviceDriver* fromDevice) {
        return (cNotImplemented );
    }

    /*!
     * \brief Bind method which implements specific binding behaviour of protocols.
     *
     * Binding may e.g. setup the protocol to filter some packets addressed to the socket.
     */
    virtual ErrorT bind(sockaddr* addr, Socket* sock) {
        return (cNotImplemented );
    }

    /*!
     * \brief Unbind method which implements specific binding behaviour of protocols.
     *
     * Binding may e.g. setup the protocol to filter some packets addressed to the socket.
     */
    virtual ErrorT unbind(sockaddr* addr, Socket* sock) {
        return (cNotImplemented );
    }

    /*!
     * \brief Adds another addresse to a device in the domain of this protocol.
     *
     * \param dev		The device which shall get another addresse.
     * \param addr		The addresse this host shall be identified by on the given device.
     */
    virtual ErrorT addLocalDeviceAddress(CommDeviceDriver* dev, unint4 addr) {
        return (cNotImplemented );
    }

    /*!
     * \brief Returns a the first addresse in this domain for the given device.
     */
    virtual
    void getLocalDeviceAddress(CommDeviceDriver* dev, unint4 &addr) {

    }

};

#endif /*ADDRESSPROTOCOL_HH_*/
