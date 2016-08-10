/*
 * ps.cc
 *
 *  Created on: 16.11.2015
 *      Author: Daniel
 */
#include <stdio.h>
#include <string.h>
#include <error.h>
#include <stdlib.h>
#include <orcos.h>

static int  taskids[100];

static const char* states[8] = {
        "NEW", // new
        "READY", // ready
        "BLOCKED", // blocked
        "TERM", // terminated
        "RES", // resource waiting
        "STOPPED", //stopped
        "SIGNAL", // signal waiting
        "DOTERM", // goint to terminate
};



const char* getStatusStr(unint4 status) {
    char* ret = "SLEEPING";
    for (int i = 0; i < 8; i++) {
        if (status & (1 << i))
            return (states[i]);
    }
    return ret;
}

bool readKernelVarStr(char* filepath, char* result, int size) {
    result[0] = 0;
    int file = open(filepath, 0);
    if (file < 0)
        return (false);

    int num = read(file, result, size - 1);
    result[num] = 0;
    close(file);
    return (true);
}

bool readKernelVar(int filehandle, void* result, int size) {
    read(filehandle, (char*) result, 4);
    return (true);
}

bool readKernelVarUInt4(char* filepath, unint4* result) {
    result[0] = 0;
    int file = open(filepath, 0);
    if (file < 0)
        return (false);

    read(file, (char*) result, 4);
    close(file);
    return (true);
}

bool readKernelVarUInt8(char* filepath, unint8* result) {
    result[0] = 0;
    int file = open(filepath, 0);
    if (file < 0)
        return (false);

    read(file, (char*) result, 8);
    close(file);
    return (true);
}


const char* getTypeStr(int resourceType);
const char* getStatusStr(unint4 status);


int exec_ps(int argc, char** argv) {
    int showThreads = 1;

    int handle = open("/sys/tasks", 0);
    if (handle < 0) {
         return (handle);
    }

     Directory_Entry_t* direntry = readdir(handle);
     int numTasks = 0;
     while (direntry) {
         taskids[numTasks] = atoi(direntry->name);
         numTasks++;
         direntry = readdir(handle);
     }

    close(handle);

    printf("PID  TID #RES STATUS \t  PRIORITY   PERIOD\t NAME"LINEFEED);

     for (int i = 0; i < numTasks; i++) {
         char path[120];
         sprintf(path,"/sys/tasks/%u",taskids[i]);
         int taskdir = open(path, 0);

         unint4 taskId;
         char name[32];
         unint4 flags;
         unint4 resources;

         if (taskdir > 0) {
             sprintf(path,"/sys/tasks/%u/myTaskId",taskids[i]);
             readKernelVarUInt4(path,&taskId);
             sprintf(path,"/sys/tasks/%u/name",taskids[i]);
             readKernelVarStr(path,name,32);
             sprintf(path,"/sys/tasks/%u/platform_flags",taskids[i]);
             readKernelVarUInt4(path,&flags);
             sprintf(path,"/sys/tasks/%u/num_resources",taskids[i]);
             readKernelVarUInt4(path,&resources);

             if (showThreads) {
                 direntry = readdir(taskdir);
                 while (direntry) {
                     if (strstr(direntry->name, "thread_") == direntry->name) {

                         char threadname[20];
                         unint8 priority = 0;
                         unint8 period   = 0;
                         unint4 threadId = 0;
                         unint4 status   = 0;

                         sprintf(path,"/sys/tasks/%u/%s/name",taskids[i],    direntry->name);
                         readKernelVarStr(path,threadname,20);

                         sprintf(path,"/sys/tasks/%u/%s/effectivePriority",  taskids[i],direntry->name);
                         readKernelVarUInt8(path,&priority);

                         sprintf(path,"/sys/tasks/%u/%s/period",taskids[i],  direntry->name);
                         readKernelVarUInt8(path,&period);

                         sprintf(path,"/sys/tasks/%u/%s/tid",taskids[i],     direntry->name);
                         readKernelVarUInt4(path,&threadId);

                         sprintf(path,"/sys/tasks/%u/%s/status",taskids[i],  direntry->name);
                         readKernelVarUInt4(path,&status);
                         const char* statusStr = getStatusStr(status);

                         printf("%2u %5u %2u %8s\t%10u %8u\t {%s} %s"LINEFEED ,
                                taskId,
                                threadId,
                                resources,
                                statusStr,
                                (unint4)priority,
                                (unint4)period,
                                threadname,
                                name);

                     }
                     direntry = readdir(taskdir);
                 }
             } else {
                 printf("%2u %2u %08x %s"LINEFEED , taskId,resources,flags,name);
             }
             close(taskdir);
         } // if taskdir
     } // for all tasks

     return (cOk);
}
