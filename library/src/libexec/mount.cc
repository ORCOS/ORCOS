/*
 * mount.cc
 *
 *  Created on: 15.11.2015
 *      Author: Daniel
 */

#include <stdio.h>
#include <string.h>
#include <error.h>
#include <stdlib.h>
#include <orcos.h>

int exec_mount(int argc, char** argv) {
    if (argc > 5 || argc < 3) {
        puts("Usage: mount (-t <type>) <src_path> <dst_path>");
        return (cError);
    }
    /* auto type */
    int type = 0;
    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], "-t") == 0) {
            if (strcmp(argv[i + 1], "overlay") == 0) {
                type = cMountType_Overlay;
            }
        }
    }

    char* srcpath = argv[argc - 2];
    char* dstpath = argv[argc - 1];

    printf("Mounting '%s' to '%s' (type: %d)" LINEFEED, srcpath, dstpath, type);

    int error = mount(srcpath, dstpath, type);
    return (error);
}
