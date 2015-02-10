/*
 * utils.cc
 *
 *  Created on: 29.01.2015
 *      Author: Daniel
 */
#include <orcos.hh>
#include <string.hh>
#include "telnet.h"

const char* types[11] = {
        "d ", // directory
        "s ", // streamdevice
        "c ", // commdevice
        "g ", // genericdevice
        "f ", // file
        "S ", //socket
        "u ", // usb
        "b ", // blockdevice
        "p ", // partition
        "sm ", // shared mem
        "kv " //kernel variable
};

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
        sprintf(return_msg, "%s. Error: %s"LINEFEED, msg, strerror(error));
    } else {
        sprintf(return_msg, "%s"LINEFEED, msg);
    }

    int end = strlen(return_msg);
    int timeout = 100;
    while (sendto(socket, return_msg, end + 1, 0) < 0 && timeout > 0) {
        usleep(1000);
        timeout--;

    }
}

void sendData(int socket, char* data, int len) {
    int timeout = 100;
    while (sendto(socket, data, len, 0) < 0 && timeout > 0) {
        usleep(1000);
        timeout--;

    }
}

void sendUnknownCommand(int socket) {
    memcpy(&return_msg[0], unknown_command, strlen(unknown_command));
    sendto(socket, &return_msg, strlen(unknown_command), 0);
}

void sendStr(int socket, char* str) {
    int timeout = 100;
    while (sendto(socket, str, strlen(str) + 1, 0) < 0 && timeout > 0) {
        usleep(1000);
        timeout--;
    }
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


char typestr[12];

const char* getTypeStr(int resourceType) {
    char* ret = typestr;
    ret[0] = 0;
    for (int i = 0; i < 11; i++) {
        if (resourceType & (1 << i))
            strcat(ret,types[i]);
    }
    return ret;
}

const char* getStatusStr(unint4 status) {
    char* ret = "SLEEPING";
    for (int i = 0; i < 8; i++) {
        if (status & (1 << i))
            return (states[i]);
    }
    return ret;
}

bool readKernelVarStr(char* filepath, char* result, int size) {
    result[0] = 0;
    int file = fopen(filepath, 0);
    if (file < 0)
        return (false);

    int num = fread(file, result, size - 1);
    result[num] = 0;
    fclose(file);
    return (true);
}

bool readKernelVarInt(int filehandle, void* result, int size) {
    int num = fread(filehandle, (char*) result, 4);
    return (true);
}

bool readKernelVarUInt4(char* filepath, unint4* result) {
    result[0] = 0;
    int file = fopen(filepath, 0);
    if (file < 0)
        return (false);

    int num = fread(file, (char*) result, 4);
    fclose(file);
    return (true);
}

bool readKernelVarUInt8(char* filepath, unint8* result) {
    result[0] = 0;
    int file = fopen(filepath, 0);
    if (file < 0)
        return (false);

    int num = fread(file, (char*) result, 8);
    fclose(file);
    return (true);
}

char* extractPath(char* &str) {

    while (str[0] == ' ')
        str++;

    if (str[0] == '"') {
        str = &str[1];
        int len = strpos("\"", str);
        if (len < 0)
            return 0;
        str[len] = 0;
        return &str[len + 1];
    }

    int len = strpos(" ", str);
    if (len < 0) {
        len = strlen(str);
        return 0;
    }

    // something is following
    str[len] = 0;
    return &str[len + 1];
}
