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

#include <defines.h>
#include <orcos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

static char* ntpserver[2] = { "pool.ntp.org", "time.nist.gov" };
static int   updateInteval = 60 * 30; // 30 min
static int   gmtOffset = 0;

#define be32tocpu cputobe32

unint4 cputobe32(unint4 n) {
    return (__builtin_bswap32(n));
}

int ntp_update(unint4& dateTime, unint4 ntpServerIpv4)
{
    int ret = 1;
    int ntpSocket = socket(cIPV4, SOCK_DGRAM, cUDP);
    char rxbuf[256];

    // bind our socket to some address

    sockaddr addr;

    addr.port_data = 124;
    addr.sa_data   = 0;

    int error = bind(ntpSocket, &addr);
    if (error < 0) {
       printf("Could not bind socket: %s\n", strerror(error));
       return (1);
    }
    printf("> NTP REQUEST: ");

    ntppacket_t ntpquery;
    memset(&ntpquery, 0, sizeof(ntppacket_t));
    ntpquery.mode       = 0x1b;

    int tries   = 0;
    int len     = 0;
    while (len < static_cast<int>(sizeof(ntppacket_t)) && tries < 3)
    {
          sockaddr ntpsrvaddr;
          ntpsrvaddr.port_data = 123;
          ntpsrvaddr.sa_data   = ntpServerIpv4;
          tries++;
          int err = sendto(ntpSocket, &ntpquery, sizeof(ntppacket_t), &ntpsrvaddr);
          if (err < 0) {
              printf("Error sending NTP Query: %s\n", strerror(err));
          }

          /* wait for answer, timeout after 4 seconds */
          len = recv(ntpSocket, rxbuf, 256, MSG_WAIT, 4000);
          if (len < 0) {
              printf("Error receiving NTP answer: %s\n", strerror(len));
          }
    }

    if (len <= 0) {
           /* timed out */
           printf("Failed\n");
    } else {
           ntppacket_t* ntpp   = reinterpret_cast<ntppacket_t*>(rxbuf);
           unint4 seconds      = (unint4) be32tocpu(ntpp->transmit_ts_seconds);

           printf("SUCCESS (%u tries).\n", tries);
           printf("Seconds: %u UNIX Time: %u\n", seconds, seconds- 2208988800UL);
           SetDateTime_t newDate;
           newDate.datetime  = seconds- 2208988800UL;
           newDate.gmtoffset = gmtOffset;
           ret = 0;
           int error = setDateTime(&newDate);
           if (error < 0) {
               printf("Error setting datetime: %s\n", strerror(error));
           }
   }
   close(ntpSocket);
   return ret;

}


int lookup(char* hostname, unint4& ipv4Addr)
{
    printf("Lookup for %s: ", hostname);
    struct hostent* host = gethostbyname(hostname);

    if (host->h_length > 0) {
       printf("%s\n", host->h_addr_list[0]);
       char b[4];
       sscanf(host->h_addr_list[0], "%d.%d.%d.%d", &b[0], &b[1], &b[2], &b[3]);
       ipv4Addr = IP4ADDR(b[0], b[1], b[2], b[3]);
       return 0;
    }

    printf("Timed out..\n");
    return -1;
}


extern "C" int main(int argc, char** argv) {
    puts("NTP Client v1.0\n");
    printf("Update Interval: %u seconds\n", updateInteval);
    if (argc > 1)
    {
        gmtOffset = atoi(argv[1]);
    }
    printf("gmtOffset: %d\n", gmtOffset);

    while (1)
    {
        for (int i = 0; i < 2; i++)
        {
            unint4 ipAddr;
            if (lookup(ntpserver[i], ipAddr) == 0)
            {
                unint4 dateTime;
                if (ntp_update(dateTime, ipAddr) == 0)
                {
                    break;
                }
            }
        }
        sleep(updateInteval);
    }

}
