/*
 * cat.cc
 *
 *  Created on: 17.11.2015
 *      Author: Daniel
 */

#include <stdio.h>
#include <string.h>
#include <error.h>
#include <stdlib.h>
#include <orcos.h>

bool readKernelVar(int filehandle, void* result, int size);

extern char _system_path[256];
static char buffer[256];


static void makeTelnetCharCompatible(char* msg, int len) {
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

int exec_cat(int argc, char** argv) {
    char* filename = argv[1];

    if (filename[0] != '/') {
        /* relative path */
        getcwd(_system_path, 256);
        strcat(_system_path,"/");
        strcat(_system_path, filename);
    } else {
        strcpy(_system_path, filename);
    }

    compactPath(_system_path);

    int filehandle = open(_system_path, 0);
    if (filehandle > 0) {
        struct stat filetype;
        filetype.st_type = 10;
        fstat(filehandle, &filetype);
        if ((filetype.st_type & cTYPE_KVAR) && filetype.st_flags != 2) {
            /* Kernel Variable */
            /* SYSFS_SIGNED_INTEGER    = 0,
               SYSFS_UNSIGNED_INTEGER  = 1,
               SYSFS_STRING = 2 */

                char* format = "%d (0x%x)\n";
                if (filetype.st_flags == 1) {
                    format = "%u (0x%x)\n";
                }

                switch (filetype.st_size) {
                case 1:
                    char cvalue;
                    readKernelVar(filehandle, &cvalue, filetype.st_size);
                    printf(format, cvalue, cvalue);
                    return (cOk);
                case 2:
                    short svalue;
                    readKernelVar(filehandle, &svalue, filetype.st_size);
                    printf(format, svalue, svalue);
                    return (cOk);
                case 4:
                    int value;
                    readKernelVar(filehandle, &value, filetype.st_size);
                    printf( format, value, value);
                    return (cOk);
                }
        } else {
            int ret = cOk;
            /* normal file.. just print content */
            int num = read(filehandle, buffer, 256);
            if (num < 0 && num != cEOF) {
                ret = num;
                num = 0;  // check error
            }

            while (num > 0) {
                // be sure the msg only contains telnet ascii chars
                makeTelnetCharCompatible(buffer, num);
                printf("%s", buffer);
                /* in case we encounter a device which has no EOF (like gpios)
                 * stop reading earlier */
                if (num < 256) break;

                num = read(filehandle, buffer, 256);
                if (num < 0 && num != cEOF) {
                    ret = num;
                    num = 0;  // check error
                }
            }
            printf("\n");
            close(filehandle);
            return (ret);
        }
    } else {
        // can not open file
        return (filehandle);
    }

    return (cOk);
}

