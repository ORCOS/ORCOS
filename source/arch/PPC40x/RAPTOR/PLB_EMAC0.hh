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

#ifndef _PLB_EMAC0_HH
#define _PLB_EMAC0_HH

#include <error.hh>
#include <types.hh>
#include <hal/CommDeviceDriver.hh>
#include "../powerpc.h"

#define TX_ADDR_OFFSET          0x00000
#define STATUS_REG_TX_OFFSET    0x04000
#define MAC_LOW_ADDR_OFFSET     0x08000
#define MAC_HIGH_ADDR_OFFSET    0x0C000
#define CONTROL_REG_RX_OFFSET   0x10000
#define CONTROL_REG_TX_OFFSET   0x14000
#define RX_ADDR_OFFSET          0x18000
#define STATUS_REG_RX_OFFSET    0x1C000
#define PROMISCUOUS_EN_OFFSET   0x20000
#define BROADCAST_EN_OFFSET     0x24000
#define RECONFIGURATION_START   0x28000
#define SOFT_RESET_OFFSET       0x2C000
#define MULTICAST_EN_OFFSET     0x30000
#define NETWORK_LOAD_OFFSET     0x34000
#define RECON_START_OFFSET      0x38000

// Anordnung der Frames im Speicher
#define FRAME_BEGIN             0x0017C

#define MAX_FRAME_SIZE    1514
#define MAX_FRAME_SIZE_IN_WORDS ((MAX_FRAME_SIZE / sizeof(unsigned long)) + 1)
#define EMAC_HDR_SIZE       14      /* size of Ethernet header */
#define MAC_ADDR_SIZE        6      /* size of MAC address */

/* TODO: replace with new init code*/
static unsigned char LocalAddress[ MAC_ADDR_SIZE]     = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 };
static unsigned char BroadcastAddress[ MAC_ADDR_SIZE] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

/*!
 *  \brief PLB_EMAC0 driver class
 *
 *  This class encapsulates the PLB_EMAC0 driver implementation.
 */
class PLB_EMAC0: public CommDeviceDriver {
private:
    int4 base_addr;

    int1 tx_frame_number;

    int1 rx_frame_number;

public:
    //!  constructor
    PLB_EMAC0(T_PLB_EMAC0_Init* init);

    //!  destructor
    ~PLB_EMAC0();

    //! enables Interrupt Requests of this device
    ErrorT enableIRQ();

    //! disables Interrupt Requests of this device. Interrupts may still be pending.
    ErrorT disableIRQ();

    //! Clears all pending irqs from this device
    ErrorT clearIRQ();

    ErrorT handleIRQ();

    //! Checks whether there is some data available inside the receive fifo.
    bool hasPendingData();

    //! returns the mac address of this device
    const char* getMacAddr() {
        return (char*) &LocalAddress;
    }

    //! returns the size (amount of bytes) mac addresses have on this device
    int1 getMacAddrSize() {
        return MAC_ADDR_SIZE;
    }

    //! Returns the id of the hardware address space (ethernet, wlan ..)
    int2 getHardwareAddressSpaceId() {
        return 6;
    }

    //! returns the broadcast address for this device medium
    const char* getBroadcastAddr() {
        return (char*) &BroadcastAddress;
    }
};

#endif /* _PLB_EMAC0_HH */
