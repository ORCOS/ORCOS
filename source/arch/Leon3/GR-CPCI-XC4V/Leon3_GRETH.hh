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

#ifndef LEON3_GRETH_HH
#define LEON3_GRETH_HH

#include <hal/CommDeviceDriver.hh>


#define    GRETH_CTRL_REG_OFFSET            0x0        // control register
#define STATUS_REG_OFFSET        0x4        // status register
#define MAC_MSB_REG_OFFSET        0x8        // most significant bits of mac address
#define MAC_LSB_REG_OFFSET        0xC        // least significant bits of mac address
#define MDIO_REG_OFFSET            0x10    // MIDO register
#define TX_DESCR_REG_OFFSET        0x14    // transmit descriptor pointer register
#define RX_DESCR_REG_OFFSET        0x18    // receive descriptor pointer register
#define EDCL_REG_OFFSET            0x1C    // EDCL register

// control register fields
#define    CTRL_REG_TE                0x1        // transmit enable
#define    CTRL_REG_RE                0x2        // receive enable
#define    CTRL_REG_TI                0x4        // enable transmitter interrupts
#define    CTRL_REG_RI                0x8        // enable receiver interrupts
#define    CTRL_REG_RS                0x40    // resets the GRETH core

// receive descriptor fields
#define RX_DESCR_LENGTH            0x7FF    // length of received package
#define RX_DESCR_EN                0x800    // enable descriptor
#define RX_DESCR_WR                0x1000    // wrap descriptor pointer
#define RX_DESCR_IE                0x2000    // enable interrupt
#define RX_DESCR_AE                0x4000    // alignment error
#define RX_DESCR_FT                0x8000    // frame too long
#define RX_DESCR_CR                0x10000    // crc error dedected
#define RX_DESCR_OE                0x20000    // overrun error
#define RX_DESCR_LE                0x40000    // length error

// transmit descriptor fields
#define TX_DESCR_LENGTH            0x7FF    // length of package to transmit
#define TX_DESCR_EN                0x800    // enable descriptor
#define TX_DESCR_WR                0x1000    // wrap descriptor pointer
#define TX_DESCR_IE                0x2000    // enable interrupt
#define TX_DESCR_UE                0x4000    // underrund error
#define TX_DESCR_AL                0x8000  // Attempt Limit Error


#define MAX_FRAME_SIZE    1514
#define MAC_ADDR_SIZE        6      /* size of MAC address */

static unsigned char LocalAddress[ MAC_ADDR_SIZE ] = { 0x05, 0x04, 0x03, 0x02, 0x01, 0x00 };

/*!
 *  \brief Leon3 Ethernet driver
 *
 *  This class encapsulates the ethernet driver implementation.
 */
class Leon3_GRETH : public CommDeviceDriver {
private:
    int port;

public:
  //!  constructor
    Leon3_GRETH(const char *name, int4 a);

  //!  destructor
  ~Leon3_GRETH();

  ErrorT        enableIRQ();

  ErrorT        disableIRQ();

  int           getIRQ();

  unint2        getMTU();

  int2             getHardwareAddressSpaceId() {
      return 6;
  }

  const char*     getMacAddr() {
      return (char*) &LocalAddress;
  }

  int1     getMacAddrSize() {
      return MAC_ADDR_SIZE;
  }
};

#endif /* LEON3_GRETH_HH */
