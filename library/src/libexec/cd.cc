/*
 * cd.cc
 *
 *  Created on: 19.11.2015
 *      Author: Daniel
 */

#include <stdio.h>
#include <string.h>
#include <error.h>
#include <stdlib.h>
#include <orcos.h>

extern char _system_path[256];

int exec_cd(int argc, char** argv) {
    if (argc > 2) {
        return (cInvalidArgument);
    }
    if (argc == 1) {
        return (cOk);
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
    return (chdir(_system_path));
}
