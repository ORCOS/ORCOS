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

#include <orcos.hh>
#include <string.hh>
#include <args.h>

#define optProgress  (1 << 0)
#define optSpeed     (1 << 1)
#define optForce     (1 << 2)
#define optRecursive (1 << 3)
#define optVerbose (1 << 4)

typedef struct {
    char* charcode;
    int optionFlag;
} Options;

Options opts[] = { { "p", optProgress }, { "s", optSpeed }, { "f", optForce }, { "r", optRecursive }, { "v", optVerbose }, };

char* source;
char* target;

static int opt;

int parseOptions(char** argv, int argc) {
    int ret = 0;
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == '-') {
            /* parameter */
            for (int j = 0; j < (sizeof(opts) / sizeof(Options)); j++) {
                if (strpos(opts[j].charcode, argv[i]) >= 0) {
                    ret |= opts[j].optionFlag;
                }
            }
            continue;
        }
        if (source == 0) {
            source = argv[i];
            continue;
        }
        if (target == 0) {
            target = argv[i];
        }
    }

    return (ret);
}

void printUsage() {
    puts("\nUsage  : cp <options> source dest\n");
    puts("Options:\n");
    puts("   -p   Show progress\n");
    puts("   -s   Show speed\n");
    puts("   -f   Force. Overwrites existing files\n");
    puts("   -r   Recursive copy.\n");
    thread_exit(-1);
}

char fileSizeStr[40];
char buffer[40960];

char* getHumanreadableSize(int bytes) {
    if (bytes > 1024 * 1024) {
        int mb = bytes / (1024 * 1024);
        int residue = bytes - mb * 1024 * 1024;
        residue = (residue * 100) / (1024 * 1024);
        sprintf(fileSizeStr, "%u,%u Mb", mb, residue);
    } else if (bytes > 1024) {
        int kb = bytes / (1024);
        int residue = bytes - kb * 1024;
        residue = (residue * 100) / (1024);
        sprintf(fileSizeStr, "%u,%u Kb", kb, residue);
    } else {
        sprintf(fileSizeStr, "%u Bytes", bytes);
    }

    return (fileSizeStr);
}



int copyFile(char* source, char* target) {
   /* one file copy */
   int fd = open(source);
   /* bail out if file not found */
   if (fd < 0) {
       printf("File '%s' open error: %s\n", source, strerror(fd));
       thread_exit(fd);
   }
   /* get file size */
   stat_t stat;
   fstat(fd, &stat);
   int filesize = stat.st_size;

   int fd2 = open(target);
   if ((fd2 > 0 && !(opt & optForce)) || (fd2 == fd)) {
       printf("File '%s' already exists\n", target);
       return (2);
   }

   if (opt & optVerbose) {
       printf("Copying %s --> %s [%s]", source, target, getHumanreadableSize(filesize));
   }
   TimeT start = getTime();
   fd2 = create(target);
   if (fd2 < 0) {
       printf("File '%s' creation error: %s\n", target, strerror(fd2));
        return (fd2);
   }

   int numread = read(fd, buffer, 40960);
   while (numread) {
       write(fd2, buffer, numread);
       numread = read(fd, buffer, 40960);
   }

   close(fd2);
   close(fd);
   TimeT end = getTime();
   if (opt & optSpeed) {
       TimeT duration = end - start; /* in ns*/
       unint8 bytesperns = ((unint8)filesize) * 1000000000;
       bytesperns /= duration;
       if (opt & optVerbose) {
           printf(" [%s/s]\n", getHumanreadableSize(bytesperns));
       } else {
           printf("%s [%s/s]\n", target, getHumanreadableSize(bytesperns));
       }
   }

}


// entry point
extern "C" int task_main(char* args) {
    volatile int val = 0;

    if (args == 0) {
        printUsage();
        thread_exit(-1);
    }
    // in place argument detection
    char** argv;
    int argc = parseArgs(args, argv);
    opt = parseOptions(argv, argc);

    if (source == 0 || target == 0) {
        printUsage();
        thread_exit(-1);
    }

    if (opt & optRecursive) {
        //return copyRecursive(opt, source, target);
        return 0;
    }

    char* dir = dirname(source);
    char* base = basename(source);

    printf("\n");

    if (strpos("*", base) >= 0) {
        /* wildcard copy*/
        return (0);
    } else {
        return (copyFile(source, target));
    }

    return (0);

    thread_exit(cOk);
}


