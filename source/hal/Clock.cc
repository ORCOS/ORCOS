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

#include <filesystem/SysFs.hh>
#include "Clock.hh"
#include "kernel/Kernel.hh"
#include "inc/endian.h"
#include "inc/const.hh"
#include "assemblerFunctions.hh"
#include "inet.h"

extern Kernel* theOS;
extern Kernel_ThreadCfdCl* pCurrentRunningThread;
extern Task* pCurrentRunningTask;

Clock::Clock(unint4 frequency) {
    synchDateTime    = 0;
    synchLocalCycles = 0;
    this->frequency  = frequency;

#if SYSFS_SUPPORT
    Directory* sysFsDir = theOS->getFileManager()->getDirectory("/sys");
    SYSFS_ADD_RO_UINT_NAMED(sysFsDir, "clockfreq", this->frequency);
#endif
}

Clock::~Clock() {
}

/*****************************************************************************
 * Method: Clock::getDateTime()
 *
 * @description
 * Returns the current Date Time as an 4 byte unsigned integer
 *        counting as seconds from January 1. 1970 if NTP SYNC was
 *        successfull. Seconds since startup otherwise.
 *
 * @returns
 *  int         The time in seconds since January 1. 1970
 *******************************************************************************/
unint4 Clock::getDateTime() {
    return (synchDateTime + ((getClockCycles() - synchLocalCycles) / CLOCK_RATE) );
}


extern "C" unint4 sys_now() {
    return (theOS->getClock()->getDateTime());
}

typedef struct {
    unint1 mode;
    unint1 stratum;
    unint1 poll;
    unint1 precision;
    unint4 root_delay;
    unint4 root_dispersion;
    unint4 reference_identifier;
    unint8 reference_timestamp;
    unint8 originate_timestamp;
    unint8 receive_timestamp;
    unint4 transmit_ts_seconds;
    unint4 transmit_ts_fraction;
} ntppacket_t;

ntppacket_t ntpquery;

/***********************************
 * Set of predefined NTP Servers
 * Queried in order of appearance
 ***********************************/
#if ENABLE_NETWORKING
unint4 ntpserver[3] = {
        IP4ADDR(5, 83, 190, 253),   /* pool.ntp.org */
        IP4ADDR(96 , 226, 242, 9),  /* nist.time.nosc.us  96.226.242.9    Carrollton, Texas  */
        IP4ADDR(206, 246, 122, 250) /* time.nist.gov   global address for all servers  Multiple locations  */
};
#endif


/*****************************************************************************
 * Method: Clock::callbackFunc(void* param)
 *
 * @description
 *  Callback scheduled after system startup to
 *  perform NTP time synchronization if enabled.
 *******************************************************************************/
void Clock::callbackFunc(void* param) {
#if ENABLE_NETWORKING
    char rxbuf[256];
    LOG(KERNEL, INFO, "Updating DateTime using NTP");
    /* We need networking for this*/
    sockaddr addr;
    Socket* ntpSocket   = new Socket(cIPV4, SOCK_DGRAM, cUDP);

    addr.port_data      = 123;
    addr.sa_data        = 0;
    ntpSocket->bind(&addr);

    sockaddr ntpsrvaddr;
    ntpsrvaddr.port_data = 123;

    memset(&ntpquery, 0, sizeof(ntppacket_t));
    ntpquery.mode       = 0x1b;

    int tries   = 0;
    int len     = 0;
    while (len < static_cast<int>(sizeof(ntppacket_t)) && tries < 3) {
        ntpsrvaddr.sa_data   = ntpserver[tries];
        tries++;
        ntpSocket->sendto(&ntpquery, sizeof(ntppacket_t), &ntpsrvaddr);

        /* wait for answer, timeout after 4 seconds */
        len = ntpSocket->recvfrom(pCurrentRunningThread, rxbuf, 256, MSG_WAIT, 0, 4000);
    }

    if (len <= 0) {
        /* timed out */
        LOG(KERNEL, WARN, "NTP SYNC failed. Receive Error %d", len);
    } else {
        ntppacket_t* ntpp   = reinterpret_cast<ntppacket_t*>(rxbuf);
        unint4 seconds      = (unint4) be32tocpu(ntpp->transmit_ts_seconds);

        LOG(KERNEL, INFO, "NTP SYNC successful (%u tries). Seconds: %u", tries, seconds);
        this->synchDateTime     = seconds;
        this->synchLocalCycles  = getClockCycles();
    }

    delete ntpSocket;
#endif
}
