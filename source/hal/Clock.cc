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

#include "Clock.hh"
#include "kernel/Kernel.hh"
#include "inc/endian.h"

extern Kernel* theOS;
extern Kernel_ThreadCfdCl* pCurrentRunningThread;
extern Task* pCurrentRunningTask;

Clock::Clock() {
    synchDateTime    = 0;
    synchLocalCycles = 0;
}

Clock::~Clock() {
}

unint4 Clock::getDateTime() {
    return (synchDateTime + ((getClockCycles() - synchLocalCycles) / CLOCK_RATE) );
}


#define cTCP                        0x6
#define cIPV4                       0x800
#define cUDP                        17

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

#define IP4ADDR( a,b,c,d) \
                         htonl(((u32_t)((a) & 0xff) << 24) | \
                               ((u32_t)((b) & 0xff) << 16) | \
                               ((u32_t)((c) & 0xff) << 8) | \
                                (u32_t)((d) & 0xff))

char rxbuf[256];

void Clock::callbackFunc(void* param) {

#if ENABLE_NETWORKING
    LOG( KERNEL,INFO,"Updating DateTime using NTP");
    /* We need networking for this*/
    sockaddr addr;

    Socket* ntpSocket   = new Socket(cIPV4,SOCK_DGRAM,cUDP);
    addr.port_data      = 123;
    addr.sa_data        = 0;
    ntpSocket->bind(&addr);

    sockaddr ntpsrvaddr;
    ntpsrvaddr.port_data    = 123;
    ntpsrvaddr.sa_data      = IP4ADDR(128,176,0,12);

    memset(&ntpquery,0,sizeof(ntppacket_t));
    ntpquery.mode = 0x1b;

    ntpSocket->sendto(&ntpquery,sizeof(ntppacket_t),&ntpsrvaddr);

    /* wait for answer, timeout after 2 seconds */
    int len = 0;

    len = ntpSocket->recvfrom(pCurrentRunningThread,rxbuf,256,MSG_WAIT,0,4000 ms);


    if (len <= 0) {
        /* timed out */
        LOG( KERNEL,WARN,"NTP Receive Error %d",len);

    } else {
        ntppacket_t* ntpp = (ntppacket_t*) rxbuf;
        unint4 seconds = (unint4) cputobe32(ntpp->transmit_ts_seconds);

        LOG( KERNEL,INFO,"NTP Update successful. Seconds: %u",seconds);
        this->synchDateTime     = seconds;
        this->synchLocalCycles  = getClockCycles();
    }

    delete ntpSocket;

#endif

}
