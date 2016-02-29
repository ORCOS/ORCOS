/*
 * utils.cc
 *
 *  Created on: 29.01.2015
 *      Author: Daniel
 */
#include <orcos.h>
#include <string.h>
#include <stdio.h>
#include "telnet.h"


const char* states[8] = {
        "NEW", // new
        "READY", // ready
        "BLOCKED", // blocked
        "TERM", // terminated
        "RES", // resource waiting
        "STOPPED", //stopped
        "SIGNAL", // signal waiting
        "DOTERM", // goint to terminate
};

static const char* unknown_command = "Unknown Command"LINEFEED;

static char return_msg[520];

void sendMsg(int socket, char* msg, int error) {
    if (error != 0) {
        sprintf(return_msg, "%s%s"LINEFEED, msg, strerror(error));
    } else {
        sprintf(return_msg, "%s"LINEFEED, msg);
    }

    int end = strlen(return_msg);
    sendto(socket, return_msg, end + 1, 0);
}

void sendData(int socket, char* data, int len) {
    send(socket, data, len);
}

void sendUnknownCommand(int socket) {
    memcpy(&return_msg[0], unknown_command, strlen(unknown_command));
    sendto(socket, &return_msg, strlen(unknown_command), 0);
}

void sendStr(int socket, char* str) {
    sendto(socket, str, strlen(str) + 1, 0);
}


void makeTelnetCharCompatible(char* msg, int len) {
    if (msg == 0) return;

    for (int i= 0; i < len; i++) {
        if (msg[i] =='\r') continue;
        if (msg[i] =='\n') continue;
        if (msg[i] =='\t') continue;
        if (msg[i] =='\0') continue;
        if (msg[i] < 32)  msg[i] = '.';
        if (msg[i] > 126) msg[i] = '.';
    }
}

void makeHexCharCompatible(char* msg, int len) {
    if (msg == 0) return;

    for (int i= 0; i < len; i++) {
        if (msg[i] < 32)  msg[i] = '.';
        if (msg[i] > 126) msg[i] = '.';
    }
}


char* extractPath(char* &str) {

    while (str[0] == ' ')
        str++;

    if (str[0] == '"') {
        str = &str[1];
        char* nexttoken = strstr(str, "\"");
        if (!nexttoken)
            return (0);
        nexttoken[0] = 0;
        return (nexttoken+1);
    }

   char* nexttoken = strstr(str, " ");
   if (!nexttoken)
       return (0);
   nexttoken[0] = 0;
   return (nexttoken+1);
}
