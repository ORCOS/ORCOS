/*
 * exec.cc
 *
 *  Created on: 15.11.2015
 *      Author: Daniel
 */

#include <stdio.h>
#include <string.h>
#include <error.h>
#include <stdlib.h>
#include <orcos.h>

extern "C" char* default_stdout;
char* default_stdout = 0;

static char  filecwd[256];
static char  filepath[256];
static char  filestdoutpath[256];
static char  arguments[256];

int exec_task(int argc, char** argv)
{
    char* filename     = argv[0];
    char* filestdout   = default_stdout;

    getcwd(filecwd, 256);
    arguments[0] = '\0';
    filepath[0]  = '\0';
    filestdoutpath[0] = '\0';

    //printf("filename = '%s'\n", filename);
    //printf("cwd = '%s'\n", filecwd);

    char* args = arguments;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '>' && argv[i][0] != '&') {
            args += sprintf(args, "\"%s\" ", argv[i]);
        } else {
            if (argv[i][0] == '>' && i < (argc-1)) {
                filestdout = argv[i+1];

                if (filestdout[0] != '/') {
                    // prepend cwd
                    strcat(filestdoutpath, filecwd);
                    strcat(filestdoutpath, filestdout);
                    filestdout = filestdoutpath;
                }

                if (argv[i][1] != '>') {
                    // not appending...
                    remove(filestdout);
                }

                int filehandle = open(filestdout, 0);
                if (filehandle < 0) {
                    /* create the file */
                    filehandle = create(filestdout, 0);
                }
                if (filehandle < 0) {
                    return (filehandle);
                }
                close(filehandle);
            }
            break;
        }
    }

    filepath[0]  = '\0';
    if (filename[0] == '.') {
        // prepend wd
        strcat(filepath, filecwd);
        strcat(filepath, &filename[1]);
    } else {
        strcat(filepath, filename);
    }

    //printf("filepath = '%s'\n", filepath);
    //printf("arguments = '%s'\n", arguments);
    //printf("filestdout = '%s'\n", filestdout);

    int taskid = task_run(filepath, arguments, filestdout, filecwd);
    if (taskid < 0) {
       return (taskid);
    }
    if (argv[argc-1][0] != '&') {
        waitpid(taskid);
        usleep(100000);
    }

    return (taskid);
}
