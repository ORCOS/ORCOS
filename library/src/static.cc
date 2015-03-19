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


#include "types.h"
#include "defines.h"
#include "orcos.hh"
#include "string.hh"

// must always be linked into the user application
extern "C" void thread_exit(int exitCode) {
    syscall(cThread_ExitSysCallId, exitCode);
}

extern "C" unint8 getCycles() {
    unint8 time __attribute__((aligned(8)));
    syscall(cGetCyclesSyscallId, &time);
    return (time);
}

extern "C" unint8 getTime() {
    unint8 time __attribute__((aligned(8)));
    syscall(cGetTimeSyscallId, &time);
    return (time);
}

extern "C" unint4 getDateTime() {
    return (syscall(cGetDateTimeSyscallId));
}

extern "C" char* extractNextArg(char* &str) {

    if (str == 0)
        return (0);

    if (*str == 0)
        return (0);

    /* strip leading white spaces */
    while (*str == ' ') {
        *str = 0;
        str++;
    }

    char* argstart = str;

    /* extract a "arg blub" type argument */
    if (*str == '"') {
        *str = 0;
        str++;
        argstart = str;
        int len = strpos("\"",str);
        if (len < 0) return (0);
        str[len] = 0;
        str += len+1;
        return (argstart);
    }

    /* get ending character position of argument
     * typically a whitespace if another argument follows */
    int len = strpos(" ",str);
    if (len < 0) {
        /* no more argument following.. */
        len = strlen(str);
        /* reached end of string? */
        if (len == 0)
            return (0);
        else {
          str[len] = 0;
          str += len;
          return (argstart);
        }
    }

    // something is following
    str[len] = 0;
    str += len +1;
    return (argstart);

}


extern "C" int parseArgs(char* str, char** &argv) {
    int arg_count = 0;
    char* curstr = str;

    if (str == 0)
        return (0);

    char* arg = extractNextArg(curstr);
    while (arg != 0) {
        arg_count++;
        arg = extractNextArg(curstr);
    }

    argv = (char**) malloc(sizeof(char*) * (arg_count+1));
    for (int i = 0; i < arg_count+1; i++)
        argv[i] = 0;

    char* pos = str;
    for (int i = 0; i < arg_count; i++) {
        while (*pos == 0) pos++;
        argv[i] = pos;
        while (*pos != 0) pos++;
    }

    return (arg_count);
}


// reduces the path by "." and ".." statements
extern "C"  void compactPath(char* path) {

    char newpath[100];

    if (path[0] == '/') {
        newpath[0] = '/';
        newpath[1] = '\0';
    } else  {
        newpath[0] = 0;
    }

    char* token = strtok(path,"/");
    char* next_token;

    while (token != 0) {

        next_token = strtok(0,"/");

        bool nextisparent = false;
        if ((next_token != 0) && ((strcmp(next_token,"..") == 0))) nextisparent = true;

        if ((strcmp(token,".") != 0) && !nextisparent && (strcmp(token,"..") != 0 )) {
            strcat(newpath,token);
            if (next_token != 0)
                strcat(newpath,"/");
        }

        token = next_token;
    }

    int pathlen = strlen(newpath);
    if (pathlen > 0 && newpath[pathlen-1] == '/') {
        newpath[pathlen-1] = 0;
    }

    memcpy(path,newpath,pathlen+1);
}
