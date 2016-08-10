/*
 * ls.cc
 *
 *  Created on: 15.11.2015
 *      Author: Daniel
 */

#include <stdio.h>
#include <string.h>
#include <error.h>
#include <stdlib.h>
#include <orcos.h>

/*** Escape Sequenzen: **********/
#define ESC_RED         "\033[31m"
#define ESC_GREEN       "\033[32m"
#define ESC_YELLOW      "\033[33m"
#define ESC_BLUE        "\033[34m"
#define ESC_PURPLE      "\033[35m"
#define ESC_CYAN        "\033[36m"
#define ESC_GRAY        "\033[37m"
#define ESC_WHITE       "\033[0m"


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

char typestr[12];

static const char* getTypeStr(int resourceType) {
    char* ret = typestr;
    ret[0] = 0;
    for (int i = 0; i < 11; i++) {
        if (resourceType & (1 << i))
            strcat(ret, types[i]);
    }
    return (ret);
}


int exec_ls(int argc, char** argv) {
    bool humanReadable = 0;
    bool details       = 0;
    int  handle        = 0;

    for (int i = 1; i < argc; i++) {
        char* arg = argv[i];
        if (arg[0] == '-') {
            /* parameter */
            if (strstr(arg, "h") != 0)
                humanReadable = 1;
            if (strstr(arg, "l") != 0)
                details = 1;
        } else {
            /* must be a path! */
            char* path = argv[i];
            compactPath(path);
            /* try opening the target dir */
            handle = open(path, 0);
            if (handle < 0) {
                return (handle);
            }
        }
    }

    if (handle == 0) {
        handle = open(".", 0);
        if (handle < 0) {
            return (handle);
        }
    }

    lseek(handle, 0, SEEK_SET);

    Directory_Entry_t* direntry = readdir(handle);
    int num = 0;
    while (direntry) {

        /* shorten name */
        if (direntry->namelen > 30 && !details) {
            direntry->name[28] = '~';
            direntry->name[29] = 0;
        }

        const char* typestr = getTypeStr(direntry->resType);
        char datestr[30];
        time_t filetime = (time_t) direntry->datetime;
        sprintf(datestr,"%s", ctime(&filetime));
        datestr[strlen(datestr)-1] = 0;

        char* prefix = "";
        char* suffix = "";

        if (direntry->resType == 1) {
            prefix = ESC_CYAN;
            suffix = ESC_WHITE;
        }

        char fileSizeStr[15];
        if (!humanReadable)
            sprintf(fileSizeStr,"%u",direntry->filesize);
        else {
            if (direntry->filesize > 1024*1024) {
                int mb = direntry->filesize / (1024*1024);
                int residue = direntry->filesize - mb * 1024*1024;
                residue = (residue * 100) / (1024*1024);
                sprintf(fileSizeStr,"%u,%uM",mb,residue);
            }
            else if (direntry->filesize > 1024) {
               int kb = direntry->filesize / (1024);
               int residue = direntry->filesize - kb * 1024;
               residue = (residue * 100) / (1024);
               sprintf(fileSizeStr,"%u,%uK",kb,residue);
            } else
              sprintf(fileSizeStr,"%u",direntry->filesize);
        }

        if (details) {
            // print details
            printf("%-5u %08x %-5s %7s %s %s%s%s" LINEFEED
                   , direntry->resId, direntry->flags , typestr, fileSizeStr, datestr, prefix,direntry->name,suffix);
        } else {
            // no details
            printf("%s%-25s%s\t", prefix, direntry->name,suffix);
            num++;
            if (num >= 3)  {
                num = 0;
                printf(LINEFEED);
            }
        }

        /* read/send next entry  */
        direntry = readdir(handle);
    }

    printf(LINEFEED);

    close(handle);
    return (cOk);
}
