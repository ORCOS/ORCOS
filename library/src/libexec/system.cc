/*
 * libexec.c
 *
 *  Created on: 15.11.2015
 *      Author: Daniel
 */

#include <stdio.h>
#include <string.h>
#include <error.h>
#include <stdlib.h>
#include <orcos.h>

int exec_task(int argc, char** argv);
int exec_mount(int argc, char** argv);
int exec_ls(int argc, char** argv);
int exec_ps(int argc, char** argv);
int exec_df(int argc, char** argv);
int exec_cat(int argc, char** argv);
int exec_ifconfig(int argc, char** argv);
int exec_cd(int argc, char** argv);
int exec_mkdir(int argc, char** argv);
int exec_rm(int argc, char** argv);
int exec_touch(int argc, char** argv);
int exec_kill(int argc, char** argv);
int exec_hexdump(int argc, char** argv);
int exec_date(int argc, char** argv);

extern "C" int parseArgs(char* str, char*** argv);

char _system_path[256];

typedef int (*systemFn_t)(int argc, char** argv);

typedef struct systemCmd {
    char*       cmd;
    systemFn_t  func;
} systemCmd;

systemCmd commands[] = {
     {"/",          exec_task},
     {"./",         exec_task},
     {"ls",         exec_ls},
     {"ps",         exec_ps},
     {"df",         exec_df},
     {"cat",        exec_cat},
     {"ifconfig",   exec_ifconfig},
     {"cd",         exec_cd},
     {"mkdir",      exec_mkdir},
     {"rm",         exec_rm},
     {"mount",      exec_mount},
     {"touch",      exec_touch},
     {"kill",       exec_kill},
     {"hexdump",    exec_hexdump},
     {"date",       exec_date}
};


extern "C" int system(const char *cmd)
{
    if (cmd == 0)
    {
        return (cError);
    }
    //printf("system(%s)\n", cmd);

    int    ret     = cUnknownCmdOrPath;
    char*  command = trim(cmd);
    char** argv    = 0;
    int    argc    = parseArgs(command, &argv);

    if (argv[0][0] == 0)
    {
        return (cOk);
    }

    if (argv[0][0] == '/' || (argv[0][0] == '.' && argv[0][1] == '/'))
    {
        ret = exec_task(argc, argv);
    } else
    {
        for (unsigned int i = 0; i < sizeof(commands) / sizeof(systemCmd); i++)
        {
            if (strcmp(argv[0], commands[i].cmd) == 0)
            {
                ret =  commands[i].func(argc, argv);
                break;
            }
        }
    }

    /* last chance try to find argv[0] inside /bin */
    if (ret < 0)
    {
        char tmppath[128];
        snprintf(tmppath, 127, "/bin/%s", argv[0]);
        tmppath[127] = 0;
        argv[0] = tmppath;
        ret = exec_task(argc, argv);
    }

    if (ret < 0) {
        printf("%s" LINEFEED, strerror(ret));
    }

    free(command);
    free(argv);

    return (ret);
}
