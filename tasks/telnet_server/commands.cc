/*
 * commands.cc
 *
 *  Created on: 29.01.2015
 *      Author: Daniel
 */

#include <orcos.h>
//#include <string.hh>
#include <args.h>
#include <time.h>
#include "telnet.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static char dir_content[4096];

int         taskids[100];

/* handle to the currently held directory by the telnet server */
extern      int mydirhandle;

void command_df(int socket, int showBlockCount) {

    unint4 UsedMem;
    unint4 TotalMem;
    readKernelVarUInt4("/sys/mem/used",&UsedMem);
    readKernelVarUInt4("/sys/mem/total",&TotalMem);

    sprintf(dir_content, "Tasks [Heap]         \t    Total\t     Used\t     Free\tUsage"LINEFEED);
    sendStr(socket, dir_content);

    sprintf(dir_content, ESC_CYAN" %-20s"ESC_WHITE"\t %8u\t %8u\t %8u\t%4u%%"LINEFEED
                     , "[kernel]",TotalMem,UsedMem,TotalMem-UsedMem,(UsedMem*100)/TotalMem);
    sendStr(socket, dir_content);

    int handle = open("/sys/tasks",0);
    if (handle) {
         Directory_Entry_t* direntry = readdir(handle);
         int numTasks = 0;
         while (direntry) {
             taskids[numTasks] = atoi(direntry->name);
             numTasks++;
             direntry = readdir(handle);
         }
         if (handle != mydirhandle)
             close(handle);

         for (int i = 0; i < numTasks; i++) {
             if (taskids[i] == 0) continue;
             char path[120];
             sprintf(path,"/sys/tasks/%u",taskids[i]);
             int taskdir = open(path,0);

             if (taskdir > 0) {
                 char name[32];
                 sprintf(path,"/sys/tasks/%u/usedmem",taskids[i]);
                 readKernelVarUInt4(path,&UsedMem);
                 sprintf(path,"/sys/tasks/%u/totalmem",taskids[i]);
                 readKernelVarUInt4(path,&TotalMem);
                 sprintf(path,"/sys/tasks/%u/name",taskids[i]);
                 readKernelVarStr(path,name,32);
                 sprintf(dir_content, ESC_CYAN" %-20s"ESC_WHITE"\t %8u\t %8u\t %8u\t%4u%%"LINEFEED
                                    , name,TotalMem,UsedMem,TotalMem-UsedMem,(UsedMem*100)/TotalMem);
                 sendStr(socket,dir_content);
             }
             if (taskdir != mydirhandle)
                close(taskdir);

         }
    } // sys/tasks

    sprintf(dir_content,"\nDisks                \t    Total\t     Used\t     Free\tUsage"LINEFEED);
    sendStr(socket,dir_content);

    handle = open("/sys/fs",0);
    if (handle) {
         Directory_Entry_t* direntry = readdir(handle);
         int numTasks = 0;
         while (direntry) {
             char diskname[60];
             strcpy(diskname,direntry->name);
             char path[120];
             sprintf(path,"/sys/fs/%s",diskname);
             int taskdir = open(path,0);
             if (taskdir > 0) {
                 unint4 numBlocks;
                 unint4 blockSize;
                 unint4 freeBlocks;

                 sprintf(path,"/sys/fs/%s/numBlocks",diskname);
                 readKernelVarUInt4(path,&numBlocks);
                 sprintf(path,"/sys/fs/%s/freeBlocks",diskname);
                 readKernelVarUInt4(path,&freeBlocks);
                 sprintf(path,"/sys/fs/%s/blockSize",diskname);
                 readKernelVarUInt4(path,&blockSize);

                 if (showBlockCount) {
                     sprintf(dir_content, ESC_CYAN" %-20s"ESC_WHITE"\t%8u\t%8u\t%8u\t%4u%%"LINEFEED
                                    , diskname,numBlocks,numBlocks-freeBlocks,freeBlocks,((numBlocks-freeBlocks)*100)/numBlocks);
                 } else {
                     sprintf(dir_content, ESC_CYAN" %-20s"ESC_WHITE"\t%8uK\t%8uK\t%8uK\t%4u%%"LINEFEED
                                        , diskname,
                                        (numBlocks * blockSize) / 1024,
                                        ((numBlocks-freeBlocks) * blockSize) / 1024,
                                        (freeBlocks * blockSize) / 1024,
                                        ((numBlocks-freeBlocks)*100)/numBlocks);

                 }
                 sendStr(socket,dir_content);

                 if (taskdir != mydirhandle)
                               close(taskdir);
             }

             direntry = readdir(handle);
         }
         if (handle != mydirhandle)
             close(handle);

    } // sys/fs
}



void command_ps(int socket, int showThreads) {

    int handle = open("/sys/tasks",0);
    if (handle < 0)
        return;

    Directory_Entry_t* direntry = readdir(handle);
    int numTasks = 0;
    while (direntry) {
        taskids[numTasks] = atoi(direntry->name);
        numTasks++;
        direntry = readdir(handle);
    }

    if (handle != mydirhandle)
        close(handle);


    sprintf(dir_content, "PID  TID #RES STATUS \t  PRIORITY   PERIOD\t NAME"LINEFEED);

   sendStr(socket,dir_content);

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

                        sprintf(dir_content, "%2u %5u %2u %8s\t%10u %8u\t {%s} %s"LINEFEED
                                         , taskId,threadId,resources,statusStr,(unint4)priority,(unint4)period,threadname,name);
                        sendStr(socket,dir_content);
                    }
                    direntry = readdir(taskdir);
                }
            } else {

                sprintf(dir_content, "%2u %2u %08x %s"LINEFEED
                                   , taskId,resources,flags,name);
                sendStr(socket,dir_content);
            }

            if (taskdir != mydirhandle)
                close(taskdir);

        } // if taskdir
    } // for all tasks

}



void command_ls(int socket, int handle, int details, int humanReadable) {
    Directory_Entry_t* direntry = readdir(handle);
    int num = 0;
    while (direntry) {

        /* shorten name */
        if (direntry->namelen > 30) {
            direntry->name[28] = '~';
            direntry->name[29] = 0;
        }

        const char* typestr = getTypeStr(direntry->resType);
        char datestr[30];
        time_t time;
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
            sprintf(dir_content, "%-5u %08x %-5s %7s %s %s%s%s"LINEFEED
                               , direntry->resId, direntry->flags , typestr, fileSizeStr, datestr, prefix,direntry->name,suffix);
        } else {
            // no details
            sprintf(dir_content,"%s%-25s%s\t",prefix,direntry->name,suffix);
            num++;
            if (num >= 3)  {
                num = 0;
                strcat(dir_content,LINEFEED);
            }
        }

        sendStr(socket,dir_content);

        /* read/send next entry  */
        direntry = readdir(handle);
        if (!direntry && !details) {
            sendStr(socket,LINEFEED);
        }
    }

}
