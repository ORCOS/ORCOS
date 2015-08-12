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


#include "orcos_types.h"
#include "defines.h"
#include "orcos.h"
#include "string.h"
#include "malloc.h"
#include <signal.h>

// must always be linked into the user application
void thread_exit(int exitCode) {
    syscall(cThread_ExitSysCallId, exitCode);
    __builtin_unreachable();
}

unint8 getCycles() {
    unint8 time __attribute__((aligned(8)));
    syscall(cGetCyclesSyscallId, &time);
    return (time);
}

unint8 getTime() {
    unint8 time __attribute__((aligned(8)));
    syscall(cGetTimeSyscallId, &time);
    return (time);
}

unint4 getDateTime() {
    return (syscall(cGetDateTimeSyscallId));
}


sighandler_t signal(int signum, sighandler_t handler) {
    // install signal handler
    return 0;
}

/*
 * Extracts the next argument from the null terminated string at *str.
 * Updates *str to point to the string following the extracted argument.
 *
 * Parses strings as. Argument1 Argument2 blub => 3 arguments
 *                or "Argument is this" blub  => 2 arguments
 */
char* extractNextArg(char** str) {
    if (str == 0)
        return (0);

    if (*str == 0)
        return (0);

    if (**str == 0)
        return (0);


    /* strip leading white spaces */
    while (**str == ' ') {
        **str = 0;
        (*str)++;
    }

    char* argstart = *str;
    char* next;

    /* extract a "arg blub" type argument */
    if (**str == '"') {
        **str = 0;
        (*str)++;
        argstart = *str;
        next = strstr(*str, "\"");
        if (!next) {
            return (0);
        }
        next[0] = 0;
        *str    = next+1;
        return (argstart);
    }

    /* get ending character position of argument
     * typically a whitespace if another argument follows */
    next = strstr(*str, " ");
    if (!next) {
        /* no more argument following.. */
        int len = strlen(*str);
        /* reached end of string? */
        if (len == 0) {
            return (0);
        } else {
          (*str)[len] = 0;
          *str = &((*str)[len]);
          return (argstart);
        }
    }

    // something is following
    next[0] = 0;
    *str = next +1;
    return (argstart);

}


int parseArgs(char* str, char*** argv) {
    int arg_count = 0;
    char* curstr = str;

    if (str == 0 || argv == 0)
        return (0);

    char* arg = extractNextArg(&curstr);
    while (arg != 0) {
        arg_count++;
        arg = extractNextArg(&curstr);
    }

    *argv = (char**) malloc(sizeof(char*) * (arg_count+1));
    for (int i = 0; i < arg_count+1; i++)
        (*argv)[i] = 0;

    char* pos = str;
    for (int i = 0; i < arg_count; i++) {
        while (*pos == 0) pos++;
        (*argv)[i] = pos;
        while (*pos != 0) pos++;
    }

    return (arg_count);
}


// reduces the path by "." and ".." statements
void compactPath(char* path) {
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

        bool nextisparent = 0;
        if ((next_token != 0) && ((strcmp(next_token,"..") == 0))) nextisparent = 1;

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


extern int main (int argc, char **argv);

/* init section containing compiler generated initialization code for variables etc */
extern void* __init_start;
extern void* __init_end;

extern void* __fini_start;
extern void* __fini_end;

extern void* __sbss_start;
extern void* __sbss_end;

/*****************************************************************************
 * Method: void (*init_handler)(void)
 *******************************************************************************/
typedef  void (*init_handler)(void);
// entry point
void task_main(char* args) {
    /* initialize sbss region */
    size_t* sbss_start = (size_t*)(&__sbss_start);
    size_t* sbss_end   = (size_t*)(&__sbss_end);

    while (sbss_start < sbss_end) {
        *sbss_start = 0;
        sbss_start++;
    }

    size_t* init_start = (size_t*)(&__init_start);
    size_t* init_end   = (size_t*)(&__init_end);
    while (init_start < init_end) {
       /* execute the init code */
       init_handler initFunction = (init_handler)(*init_start);
       initFunction();
       init_start++;
    }

    char** argv = 0;
    int argc = parseArgs(args, &argv);
    int ret  = main(argc, argv);

    size_t* fini_start = (size_t*)(&__fini_start);
    size_t* fini_end   = (size_t*)(&__fini_end);
    while (fini_start < fini_end) {
      /* execute the init code */
      init_handler initFunction = (init_handler)(*fini_start);
      initFunction();
      fini_start++;
    }

    thread_exit(ret);
    __builtin_unreachable();
}
