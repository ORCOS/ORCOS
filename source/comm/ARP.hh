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

#ifndef ARP_HH_
#define ARP_HH_

#include "AddressProtocol.hh"
#include "db/ArrayDatabase.hh"
#include "hal/CallableObject.hh"
#include "process/WorkerThread.hh"

#define MAX_ARP_REQUESTS 4

/*!
 * \brief The ARPPacketHeader. See RFC 826.
 */
typedef struct {
    int2 ar_hrd; //!< hardware address space
    int2 ar_pro; //!< protocol address space (proto id)
    int1 ar_hln; //!< length of hardware addresses
    int1 ar_pln; //!< length of protocol addresses
    int2 ar_op; //!< operation code (REQUEST | REPLY)
} ARPPacketHeader;


typedef struct {
    char* dest_addr;        //!< the address to lookup! somewhere in calling thread heap or stack
    int2 count;             //!< count how many times this request has been send
    int2 maxcount;          //!< maximum amount of retries before we fail!
    int1 dest_addr_len;     //!< the address length
    int1 free;              //!< is this a free slot
    WorkerThread* pWThread; //!< the workerthread that is assigned to this request
    int* retval;            //!< The return value for this request! on calling thread stack
    ThreadCfdCl* thread;    //!< the thread that needs this address to be looked up
    AddressProtocol* proto; //!< the send parameters for the thread that wants this address to be lookeup
} ARPRequest;

/*!
 * \brief Address Resolution Protocol
 *
 * This protocol is a simple implemenation of the ARP.
 * See RFC 826 for packet format (http://tools.ietf.org/html/rfc826)
 *
 */
class ARP: public AddressProtocol, CallableObject {
private:
    //! Table storeing the pending address resolution requests by threads and address

    //! Array storing the pending ARP requests
    ARPRequest pending_requests[ MAX_ARP_REQUESTS ];

    //! Sends the arp packet over all known communication devices
    void sendARPRequest( char* dest_addr, int addr_len, AddressProtocol* proto);

public:
    ARP( Directory* commdevsdir );
    ~ARP();

    /*!
     * \brief Send method which issues a address resolution request.
     *
     * Since the ARP is a resolution protocol no data is carried by the protocol. Thus
     * msgstart and msglength are ignored by this protocol. The parameter
     * dest_addr will be used as the address which needs mac-address lookup.
     *
     * The RequestParams struct is needed to complete the send request of the thread that
     * wants this address to be looked up.
     */
    ErrorT lookup( char* dest_addr, int addr_len, AddressProtocol* proto);

    //! Receive method called by the worker thread
    ErrorT recv( char* packetstart, int packetlength, CommDeviceDriver* fromDevice );

    //! Callback method which is used for periodic lookup processes
    void callbackFunc( void* param );
};

#endif /*ARP_HH_*/
