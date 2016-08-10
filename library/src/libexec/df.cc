/*
 * df.cc
 *
 *  Created on: 17.11.2015
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

int  taskids[100];

bool readKernelVarUInt8(char* filepath, unint8* result);
bool readKernelVarUInt4(char* filepath, unint4* result);
bool readKernelVarStr(char* filepath, char* result, int size);

int exec_df(int argc, char** argv) {
    unint4 UsedMem;
    unint4 TotalMem;
    int showBlockCount = 0;

    readKernelVarUInt4("/sys/mem/used",&UsedMem);
    readKernelVarUInt4("/sys/mem/total",&TotalMem);

    printf("Tasks [Heap]         \t    Total\t     Used\t     Free\tUsage"LINEFEED);


    printf(ESC_CYAN" %-20s"ESC_WHITE"\t %8u\t %8u\t %8u\t%4u%%"LINEFEED
                     , "[kernel]",TotalMem,UsedMem,TotalMem-UsedMem,(UsedMem*100)/TotalMem);


    int handle = open("/sys/tasks",0);
    if (handle) {
         Directory_Entry_t* direntry = readdir(handle);
         int numTasks = 0;
         while (direntry) {
             taskids[numTasks] = atoi(direntry->name);
             numTasks++;
             direntry = readdir(handle);
         }
         close(handle);

         for (int i = 0; i < numTasks; i++) {
             if (taskids[i] == 0) continue;
             char path[120];
             sprintf(path,"/sys/tasks/%u",taskids[i]);
             int taskdir = open(path,0);

             if (taskdir > 0) {
                 char name[32];
                 sprintf(path,"/sys/tasks/%u/usedmem",taskids[i]);
                 readKernelVarUInt4(path, &UsedMem);
                 sprintf(path,"/sys/tasks/%u/totalmem",taskids[i]);
                 readKernelVarUInt4(path, &TotalMem);
                 sprintf(path,"/sys/tasks/%u/name",taskids[i]);
                 readKernelVarStr(path, name, 32);
                 printf( ESC_CYAN" %-20s"ESC_WHITE"\t %8u\t %8u\t %8u\t%4u%%"LINEFEED
                         ,name, TotalMem,UsedMem,TotalMem-UsedMem,(UsedMem*100)/TotalMem);
             }
             close(taskdir);
         }
    } // sys/tasks

    printf(LINEFEED "Disks                \t    Total\t     Used\t     Free\tUsage"LINEFEED);

    handle = open("/sys/fs",0);
    if (handle) {
         Directory_Entry_t* direntry = readdir(handle);
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
                     printf(ESC_CYAN" %-20s"ESC_WHITE"\t%8u\t%8u\t%8u\t%4u%%"LINEFEED
                                    , diskname,numBlocks,numBlocks-freeBlocks,freeBlocks,((numBlocks-freeBlocks)*100)/numBlocks);
                 } else {
                     printf(ESC_CYAN" %-20s"ESC_WHITE"\t%8uK\t%8uK\t%8uK\t%4u%%"LINEFEED
                                        , diskname,
                                        (numBlocks * blockSize) / 1024,
                                        ((numBlocks-freeBlocks) * blockSize) / 1024,
                                        (freeBlocks * blockSize) / 1024,
                                        ((numBlocks-freeBlocks)*100)/numBlocks);

                 }
                 close(taskdir);
             }
             direntry = readdir(handle);
         }
         close(handle);
    } // sys/fs

    return (cOk);
}
