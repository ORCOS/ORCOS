/*
 * ifconfig.cc
 *
 *  Created on: 17.11.2015
 *      Author: Daniel
 */


#include <stdio.h>
#include <string.h>
#include <error.h>
#include <stdlib.h>
#include <orcos.h>

#define LINEFEED "\n"

/*** Escape Sequenzen: **********/
#define ESC_RED         "\033[31m"
#define ESC_GREEN       "\033[32m"
#define ESC_YELLOW      "\033[33m"
#define ESC_BLUE        "\033[34m"
#define ESC_PURPLE      "\033[35m"
#define ESC_CYAN        "\033[36m"
#define ESC_GRAY        "\033[37m"
#define ESC_WHITE       "\033[0m"

int exec_ifconfig(int argc, char** argv) {
    if (argc == 1) {
        /* display stats */
        int handle = open("/dev/comm", 0);
        if (handle) {
            Directory_Entry_t* direntry = readdir(handle);

            char devpath[60];
            sprintf(devpath, "/dev/comm/%s", direntry->name);
            int devicehandle = open(devpath);
            netif_stat_t netifstats;
            ioctl(devicehandle, cNETIF_GET_STATS, &netifstats);

            printf("%s\tHWaddr: %02x:%02x:%02x:%02x:%02x:%02x\n", direntry->name, netifstats.hwaddr[0], netifstats.hwaddr[2], netifstats.hwaddr[2], netifstats.hwaddr[3], netifstats.hwaddr[4], netifstats.hwaddr[5]);
            printf("   \tinet addr:%u.%u.%u.%u  Bcast:%u.%u.%u.255  Mask:%u.%u.%u.%u\n", (netifstats.ipv4addr >> 24) & 0xff, (netifstats.ipv4addr
                            >> 16) & 0xff, (netifstats.ipv4addr >> 8) & 0xff, (netifstats.ipv4addr) & 0xff, (netifstats.ipv4addr >> 24) & 0xff, (netifstats.ipv4addr
                            >> 16) & 0xff, (netifstats.ipv4addr >> 8) & 0xff, (netifstats.ipv4netmask >> 24) & 0xff, (netifstats.ipv4netmask >> 16) & 0xff, (netifstats.ipv4netmask
                            >> 8) & 0xff, (netifstats.ipv4netmask) & 0xff);

            inet_ntop(AF_INET6, netifstats.ipv6addr, devpath, 60);
            printf("   \tinet6 addr:%s\n", devpath);
            printf("   \t");
            if (netifstats.flags & NETIF_FLAG_UP)
                printf("UP ");
            else
                printf("DOWN ");

            if (netifstats.flags & NETIF_FLAG_BROADCAST)
                printf("BROADCAST ");

            if (netifstats.flags & NETIF_FLAG_POINTTOPOINT)
                printf("PPP ");

            if (netifstats.flags & NETIF_FLAG_DHCP)
                printf("DHCP ");

            printf("RUNNING  MTU:%u  Metric:1\n", netifstats.mtu);
            printf("   \tRX packets:%u  TX packets:%u  errors:%u\n", netifstats.rxpackets, netifstats.txpackets, netifstats.errors);
            printf("   \tRX bytes:%u (%u KiB) TX bytes:%u (%u KiB)\n\n",
                   netifstats.rxbytes,
                   netifstats.rxbytes / 1024,
                   netifstats.txbytes,
                   netifstats.txbytes / 1024);

            close(devicehandle);
        }

       close(handle);
       return (cOk);
    } else {
        /* configure some network device*/
        return (cInvalidArgument);
    }
}
