/*
 * hexdump.cc
 *
 *  Created on: 19.11.2015
 *      Author: Daniel
 */

#include <stdio.h>
#include <string.h>
#include <error.h>
#include <stdlib.h>
#include <orcos.h>

static void makeHexCharCompatible(char* msg, int len) {
    if (msg == 0) return;

    for (int i= 0; i < len; i++) {
        if (msg[i] < 32)  msg[i] = '.';
        if (msg[i] > 126) msg[i] = '.';
    }
}


extern char _system_path[256];
static char buffer[256];

int exec_hexdump(int argc, char** argv) {
    if (argc < 2) {
        return (cInvalidArgument );
    }

    char* filename = argv[1];

    if (filename[0] != '/') {
        /* relative path */
        getcwd(_system_path, 256);
        strcat(_system_path, "/");
        strcat(_system_path, filename);
    } else {
        strcpy(_system_path, filename);
    }

    compactPath(_system_path);

    int filehandle = open(_system_path, 0);
    if (filehandle > 0) {
        int num = read(filehandle, buffer, 256);

        char linechars[18];
        int linebytes = 0;
        int chars = 0;
        int i;

        int address = 0;
        printf("%08x ", address);
        // be sure the msg only contains telnet ascii chars
        for (i = 0; i < num-4; i+=4) {
            printf("%08x ", *((int*)&buffer[i]));
            chars+=9;
            linebytes+=4;
            address++;

            if (chars > 35) {
                // print the ascii code
                memcpy(linechars, &buffer[(i + 1) - 16], 16);
                linechars[17] = 0;
                makeHexCharCompatible(linechars, 16);
                printf("\t%s\n", linechars);
                linebytes = 0;
                chars = 0;
                printf("%08x ", address);
            }

        }

        if (i < num) {
            int pos = i;
            while (i < num) {
               printf("%02x", buffer[i]);
               linebytes++;
               i++;
            }


            for (int j = 0; j < 36 - chars; j++) {
                printf(" ");
            }

            // add last ascii chars
            memcpy(linechars, &buffer[pos], linebytes + (num-pos) - 1);
            makeHexCharCompatible(linechars,  linebytes + (num-pos) - 1);
            linechars[ linebytes + (num-pos) - 1] = 0;
            printf("\t%s\n", linechars);
        }

        return (cOk);
    } else {
        // can not open file
        return (filehandle);
    }
}
