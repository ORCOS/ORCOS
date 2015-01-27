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


/*
 * This is a very simple telnet server implementation runnable under ORCOS.
 * This is probably not the nicest code i have ever written ;), however it
 * it tested and works quite well. A future version should provide support for
 * multiple connections and should utilse classes or structs as well as seperated
 * implementation files for modularization.
 *
 */


#include <orcos.hh>
#include <string.hh>
#include <args.h>
#include <time.h>

/*** Escape Sequenzen: **********/
#define ESC_RED         "\033[31m"
#define ESC_GREEN       "\033[32m"
#define ESC_YELLOW      "\033[33m"
#define ESC_BLUE        "\033[34m"
#define ESC_PURPLE      "\033[35m"
#define ESC_CYAN        "\033[36m"
#define ESC_GRAY        "\033[37m"
#define ESC_WHITE       "\033[0m"

#define IPV4 0x800
#define TCP 0x6

#define DOECHO 1


#define LINEFEED "\r\n"
#define orcos_string "         __           __          __             __           __ "LINEFEED"\
        /\\ \\         /\\ \\        /\\ \\           /\\ \\         / /\\ "LINEFEED"\
       /  \\ \\       /  \\ \\      /  \\ \\         /  \\ \\       / /  \\ "LINEFEED"\
      / /\\ \\ \\     / /\\ \\ \\    / /\\ \\ \\       / /\\ \\ \\     / / /\\ \\__ "LINEFEED"\
     / / /\\ \\ \\   / / /\\ \\_\\  / / /\\ \\ \\     / / /\\ \\ \\   / / /\\ \\___\\ "LINEFEED"\
    / / /  \\ \\_\\ / / /_/ / / / / /  \\ \\_\\   / / /  \\ \\_\\  \\ \\ \\ \\/___/ "LINEFEED"\
   / / /   / / // / /__\\/ / / / /    \\/_/  / / /   / / /   \\ \\ \\ "LINEFEED"\
  / / /   / / // / /_____/ / / /          / / /   / / /_    \\ \\ \\ "LINEFEED"\
 / / /___/ / // / /\\ \\ \\  / / /________  / / /___/ / //_/\\__/ / / "LINEFEED"\
/ / /____\\/ // / /  \\ \\ \\/ / /_________\\/ / /____\\/ / \\ \\/___/ / "LINEFEED"\
\\/_________/ \\/_/    \\_\\/\\/____________/\\/_________/   \\_____\\/ "LINEFEED"\
"LINEFEED"Welcome.. Terminal running.."LINEFEED"/$ "


const char* welcome_msg = orcos_string;

#define MAX_COMMAND_HISTORY 16

// the current index inside the command history
static char ci = 0;

static int history_count = 0;

// the command history
static char command_history[MAX_COMMAND_HISTORY][100];

// the current command being received
static char command[100];

// the sequence to perform a back char on the terminal
static char back_sequence[3] = {0x8,0x20,0x8};
static char down_sequence[3] = {0x1b,0x5b,0x42};

// line feed
static char return_sequence[4] = {13,0,13,0};

// current directory we are in
static char current_dir[100];

// last directory
static char last_dir[100];

static const char* unknown_command = "Unknown Command"LINEFEED;

static const char* dir_error = "Could not open Directory.."LINEFEED;

static const char* help_msg = "OCROS Telnet Terminal\r\nSupported commands:"LINEFEED
                        "help      - Shows this message"LINEFEED
                        "cd        - Change Directory"LINEFEED
                        "ls        - List Directory"LINEFEED
                        "ps        - List Processes"LINEFEED
                        "df        - List Heap and Filesystem Usage"LINEFEED
                        "mkdir     - Create Directory"LINEFEED
                        "rm        - Remove File/Directory"LINEFEED
                        "cat       - Display Contents of file (in ASCII)"LINEFEED
                        "hexdump   - Dumps a file as hex and ASCII"LINEFEED
                        "touch     - Creates a new file"LINEFEED
                        "ifconfig  - Network interface configuration"LINEFEED
                        "run       - Starts a task from file"LINEFEED
                        "kill      - Kills a task by ID"LINEFEED;

static char return_msg[520];

static char temp_msg[200];

static int cmd_pos = 0;

const char* types[11] = {
        "d ", // directory
        "s ", // streamdevice
        "c ", // commdevice
        "g ", // genericdevice
        "f ", // file
        "S ", //socket
        "u ", // usb
        "b ", // blockdevice
        "p ", // partition
        "sm ", // shared mem
        "kv " //kernel variable
};

const char* states[8] = {
        "NEW", // new
        "READY", // ready
        "BLOCKED", // blocked
        "TERM", // terminated
        "RES", // resource waiting
        "STOPPED", //stopped
        "SIGNAL", // signal waiting
        "DOTERM", // goint to terminate
};




static int mydirhandle = 0;


static char dir_content[4096];

char typestr[12];

const char* getTypeStr(int resourceType) {
    char* ret = typestr;
    ret[0] = 0;
    for (int i = 0; i < 11; i++) {
        if (resourceType & (1 << i))
            strcat(ret,types[i]);
    }
    return ret;
}

const char* getStatusStr(unint4 status) {
    char* ret = "SLEEPING";
    for (int i = 0; i < 8; i++) {
        if (status & (1 << i))
            return (states[i]);
    }
    return ret;
}


char* extractPath(char* &str) {

    while (str[0] == ' ') str++;

    if (str[0] == '"') {
        str = &str[1];
        int len = strpos("\"",str);
        if (len < 0) return 0;
        str[len] = 0;
        return &str[len+1];
    }

    int len = strpos(" ",str);
    if (len < 0) {
        len = strlen(str);
        return 0;
    }

    // something is following
    str[len] = 0;
    return &str[len+1];
}


void sendMsg(int socket, char* msg, int error = 0) {
    if (error != 0) {
        sprintf(return_msg,"%s. Error: %d"LINEFEED,msg,error);
    } else {
        sprintf(return_msg,"%s"LINEFEED,msg);
    }

    int end = strlen(return_msg);
    int timeout = 100;
    while (sendto(socket,return_msg,end+1,0)  < 0 && timeout > 0) {
         usleep(1000);
         timeout--;

    }
}

void sendData(int socket, char* data, int len) {
   int timeout = 100;
   while (sendto(socket,data,len,0)  < 0 && timeout > 0) {
        usleep(1000);
        timeout--;

   }
}

void sendUnknownCommand(int socket) {
    memcpy(&return_msg[0],unknown_command,strlen(unknown_command));
    sendto(socket,&return_msg,strlen(unknown_command),0);
}

void sendStr(int socket,char* str) {
   int timeout = 100;
   while (sendto(socket,str,strlen(str)+1,0) < 0 && timeout > 0) {
       usleep(1000);
       timeout--;
   }
}

bool readKernelVarStr(char* filepath,char* result, int size) {
    result[0] = 0;
    int file = fopen(filepath,0);
    if (file < 0)
        return (false);

    int num = fread(result,size-1,1,file);
    result[num] = 0;
    fclose(file);
    return (true);
}

bool readKernelVarInt(int filehandle, void* result, int size) {
    int num = fread(result,4,1,filehandle);
    return (true);
}

bool readKernelVarUInt4(char* filepath,unint4* result) {
    result[0] = 0;
    int file = fopen(filepath,0);
    if (file < 0)
        return (false);

    int num = fread(result,4,1,file);
    fclose(file);
    return (true);
}

bool readKernelVarUInt8(char* filepath,unint8* result) {
    result[0] = 0;
    int file = fopen(filepath,0);
    if (file < 0)
        return (false);

    int num = fread(result,8,1,file);
    fclose(file);
    return (true);
}

static int doecho;

char telnetcommand[10];

#define SB         250 // subnegotation begin
#define SE         240 // subnegotation end
#define WILL    0xfb
#define WONT    0xfc
#define DO        0xfd
#define DONT    0xfe
#define IAC     0xff

#define SGA 3

#define LINEMODE 34
#define MODE  1
#define FORWARDMASK 2

int handleTelnetCommand(int socket, char* bytes) {


    switch (bytes[0]) {
    case 241 : return 0;
    case 246 : {
        telnetcommand[0] = 0xff;
        telnetcommand[1] = 249;
        sendto(socket,telnetcommand,2,0);
        return 0;
    }
    case 247 : {
        if (cmd_pos > 0) {
            command[cmd_pos--] = 0;
            if (doecho) sendto(socket,&back_sequence,3,0);
        }
        return 0;
    }
    case WILL : {
        switch (bytes[1]) {
        case LINEMODE:
            telnetcommand[0] = IAC;
            telnetcommand[1] = SB;
            telnetcommand[2] = LINEMODE;
            telnetcommand[3] = MODE;
            telnetcommand[4] = 0;
            telnetcommand[5] = IAC;
            telnetcommand[6] = SE;
            sendto(socket,telnetcommand,7,0);
            return 1;
        default:
      /*      telnetcommand[0] = 0xff;
            telnetcommand[1] = DONT;
            telnetcommand[2] = bytes[1];
            sendto(socket,telnetcommand,3,0);*/
            return 1;
        }
    }
    case DO : {

        switch (bytes[1]) {
        case 1:// ECHO
            telnetcommand[0] = IAC;
            #if DOECHO
                telnetcommand[1] = WILL;
            #else
                telnetcommand[1] = WONT;
            #endif
            telnetcommand[2] = bytes[1];
            sendto(socket,telnetcommand,3,0);
            return 1;
        case SGA:
            telnetcommand[0] = IAC;
            telnetcommand[1] = WILL;
            telnetcommand[2] = SGA;
        default:
            return 1;
        }
    }
    case DONT : {
        if (bytes[1] == 1) {
            doecho = 0;
        }

        return 1;
    }
    case WONT : {
        return 1;
    }

    default : return 0;


    }

}

void makeTelnetCharCompatible(char* msg, int len) {
    if (msg == 0) return;

    for (int i= 0; i < len; i++) {
        if (msg[i] =='\r') continue;
        if (msg[i] =='\n') continue;
        if (msg[i] =='\t') continue;
        if (msg[i] =='\0') continue;
        if (msg[i] < 32)  msg[i] = '.';
        if (msg[i] > 126) msg[i] = '.';
    }
}

void makeHexCharCompatible(char* msg, int len) {
    if (msg == 0) return;

    for (int i= 0; i < len; i++) {
        if (msg[i] < 32)  msg[i] = '.';
        if (msg[i] > 126) msg[i] = '.';
    }
}




int taskids[100];

void printMemUsage(int socket, int showBlockCount) {

    unint4 UsedMem;
    unint4 TotalMem;
    readKernelVarUInt4("/sys/mem/used",&UsedMem);
    readKernelVarUInt4("/sys/mem/total",&TotalMem);

    sprintf(dir_content,"Tasks [Heap]         \t    Total\t     Used\t     Free\tUsage"LINEFEED);
    sendStr(socket,dir_content);

    sprintf(dir_content, ESC_CYAN" %-20s"ESC_WHITE"\t %8u\t %8u\t %8u\t%4u%%"LINEFEED
                     , "[kernel]",TotalMem,UsedMem,TotalMem-UsedMem,(UsedMem*100)/TotalMem);
    sendStr(socket,dir_content);

    int handle = fopen("/sys/tasks",0);
    if (handle) {
         Directory_Entry_t* direntry = readdir(handle);
         int numTasks = 0;
         while (direntry) {
             taskids[numTasks] = atoi(direntry->name);
             numTasks++;
             direntry = readdir(handle);
         }
         if (handle != mydirhandle)
             fclose(handle);

         for (int i = 0; i < numTasks; i++) {
             if (taskids[i] == 0) continue;
             char path[120];
             sprintf(path,"/sys/tasks/%u",taskids[i]);
             int taskdir = fopen(path,0);

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
                fclose(taskdir);

         }
    } // sys/tasks

    sprintf(dir_content,"\nDisks                \t    Total\t     Used\t     Free\tUsage"LINEFEED);
    sendStr(socket,dir_content);

    handle = fopen("/sys/fs",0);
    if (handle) {
         Directory_Entry_t* direntry = readdir(handle);
         int numTasks = 0;
         while (direntry) {
             char diskname[60];
             strcpy(diskname,direntry->name);
             char path[120];
             sprintf(path,"/sys/fs/%s",diskname);
             int taskdir = fopen(path,0);
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
                               fclose(taskdir);
             }

             direntry = readdir(handle);
         }
         if (handle != mydirhandle)
             fclose(handle);

    } // sys/fs
}


void printTasks(int socket, int showThreads) {

    int handle = fopen("/sys/tasks",0);
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
        fclose(handle);

    for (int i = 0; i < numTasks; i++) {
        char path[120];
        sprintf(path,"/sys/tasks/%u",taskids[i]);
        int taskdir = fopen(path,0);

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
                    if (strpos("thread_",direntry->name) == 0) {

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
                fclose(taskdir);

        } // if taskdir
    } // for all tasks

}

void printDirectory(int socket, int handle, int details, int humanReadable) {
    Directory_Entry_t* direntry = readdir(handle);
    int num = 0;
    while (direntry) {

        /* shorten name */
        if (direntry->namelen > 30) {
            direntry->name[28] = '~';
            direntry->name[29] = 0;
        }

        const char* typestr = getTypeStr(direntry->resType);
        char datestr[20];
        time_t time;
        time2date(&time,direntry->datetime);
        sprintf(datestr,"%u-%02u-%02u %02u:%02u",time.year,time.month,time.day, time.hour, time.minute);

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
            sprintf(dir_content, "%-5u %08x %-5s%7s %s %s%s%s"LINEFEED
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


void handleCommand(int socket, int command_length) {

    // do a command str compare
    if (strcmp("help",command) == 0) {

        memcpy(&return_msg[0],help_msg,strlen(help_msg)+1);
        sendto(socket,&return_msg,strlen(help_msg)+1,0);

        return;
    }
    if (strpos("cd ",command) == 0) {

        char** argv;
        int args = parseArgs(command,argv);
        if (args != 2)  {
            sendMsg(socket,"Usage: cd <path>");
            return;
        }

        char* filename = argv[1];
        char path[100];

        if (filename[0] !='/') {
            /* relative path */
            strcpy(path,current_dir);
            strcat(path,"/");
            strcat(path,filename);
        } else {
            strcpy(path,filename);
        }

        compactPath(path);

        memcpy(last_dir,current_dir,100);
        memcpy(current_dir,path,100);
        strcat(current_dir,"/");

        //printf("newdir: %s\r\n",current_dir);

        int handle = fopen(current_dir,0);
        if (handle >= 0) {

            /* check type */
            stat_t filetype;
            filetype.st_type = 10;
            if (fstat(handle,&filetype) != cOk || (filetype.st_type & STAT_TYPE_DIRECTORY) == 0) {
                fclose(handle);
                memcpy(current_dir,last_dir,100);
                return (sendMsg(socket,"Could not open directory"));
            }

            // success
            if ((mydirhandle != 0) && (mydirhandle != handle )) fclose(mydirhandle);
            mydirhandle = handle;
        } else {
            memcpy(current_dir,last_dir,100);
            return (sendMsg(socket,"Could not open directory"));
        }


        return;
    }

    if (strpos("mount", command) == 0) {
        char** argv;
          /* TODO: delete argv as it is malloced by parseArgs! */
          int args = parseArgs(command,argv);
          if (args > 5 || args < 3)  {
                sendMsg(socket,"Usage: mount (-t <type>) <src_path> <dst_path>");
                return;
          }
          /* auto type */
          int type = 0;
          for (int i = 1; i < args-1; i++) {
              if (strcmp(argv[i],"-t") == 0) {
                  if (strcmp(argv[i+1],"overlay") == 0) {
                      type = cMountType_Overlay;
                  }
              }
          }

          char* srcpath = argv[args-2];
          char* dstpath = argv[args-1];

          sprintf(return_msg,"Mounting '%s' to '%s' (type: %d)" LINEFEED,srcpath,dstpath,type);
          sendMsg(socket,return_msg);

          int error = mount(srcpath,dstpath,type);
          if (error != 0) {
              sprintf(return_msg,"Error: %d" LINEFEED,error);
              sendMsg(socket,return_msg);
          }
          /* reopen current dir as it might be overlayed */
          fclose(mydirhandle);
          mydirhandle = fopen(current_dir);

          return;
    }

    if (strpos("ls",command) == 0) {
        char** argv; /* must be deleted */
        int args = parseArgs(command,argv);
        int handle = mydirhandle;
        int humanReadable = 0;
        int details       = 0;

        for (int i = 1; i < args; i++) {
            char* arg = argv[i];
            if (arg[0] == '-') {
                /* parameter */
                if (strpos("h",arg) >= 0)
                  humanReadable = 1;
                if (strpos("l",arg) >= 0)
                  details = 1;
            } else {
                /* must be a path! */
               char* path = argv[1];
               compactPath(path);
               /* try opening the target dir */
               handle = fopen(path,0);
               if (handle < 0) {
                   return (sendMsg(socket,"Could not open directory",handle));
               }
            }
        }

        delete argv;
        printDirectory(socket,handle,details,humanReadable);

        if (args == 2 && handle != mydirhandle) {
            fclose(handle);
        }

        return;
    }

    if (strpos("/",command) == 0 || strpos("./",command) == 0) {
        int arg = strpos(" ",command);
        char* filename = command;
        char* arguments = extractPath(filename);

        if (filename[0] != '/') {
            // prepend current directory
            strcpy(temp_msg,current_dir);
            strcat(temp_msg,filename);
            filename = temp_msg;
        }
        compactPath(filename);

        int taskid = task_run(filename,arguments);
        if (taskid > 0) {
            /* task started  */
            //sprintf(return_msg,"TID: %d"LINEFEED,taskid);
            /* set stdout of new task to us! */
            taskioctl(0,taskid,"/dev/tty0");
        } else {
            sprintf(return_msg,"Error: %d"LINEFEED,taskid);
        }

        sendto(socket,return_msg,strlen(return_msg)+1,0);
        return;

    }

    if (strpos("kill",command) == 0) {
        int arg = strpos(" ",command);
        if (arg > 0) {
            char* taskid = &command[arg+1];
            // todo test on errors
            int id = atoi(taskid);
            if (id != getpid()) {
                int error = task_kill(id);
                // no argument given
                sprintf(return_msg,"Killing Task Error: %d"LINEFEED,error);
                sendto(socket,return_msg,strlen(return_msg)+1,0);
            } else {
                sprintf(return_msg,"I dont want to kill myself.."LINEFEED);
                sendto(socket,return_msg,strlen(return_msg)+1,0);
            }

            return;
        }

    }

    if (strpos("hexdump",command) == 0) {

      char** argv;
      int args = parseArgs(command,argv);
      if (args != 2)  {
          sendMsg(socket,"Usage: hexdump <filename>");
          return;
      }

      char* filename = argv[1];
      char path[100];

      if (filename[0] !='/') {
          /* relative path */
          strcpy(path,current_dir);
          strcat(&path[strlen(current_dir)],filename);
      } else {
          strcpy(path,filename);
      }

      compactPath(path);

        int filehandle = fopen(path,0);
        if (filehandle > 0) {
            int num = fread(temp_msg,200,1,filehandle);
            int end = num;
            int pos = 0;

            char linechars[9];

            int linebytes = 0;
            int i;
            // be sure the msg only contains telnet ascii chars
            for (i = 0; i < num; i++) {
                sprintf(&return_msg[pos]," %02x",temp_msg[i]);
                pos += 3;
                linebytes++;

                if (linebytes > 7) {
                    // print the ascii code
                    memcpy(linechars,&temp_msg[(i+1)-8],8);
                    linechars[8] = 0;
                    makeHexCharCompatible(linechars,8);
                    sprintf(&return_msg[pos],"\t%s"LINEFEED,linechars);
                    pos +=11;
                    linebytes = 0;
                }

            }

            for (int j = 0; j < 8-linebytes; j++) {
                sprintf(&return_msg[pos],"   ");
                pos += 3;
            }

            // add last ascii chars
            memcpy(linechars,&temp_msg[i-linebytes],linebytes);
            linechars[linebytes] = 0;
            makeHexCharCompatible(linechars,linebytes);
            sprintf(&return_msg[pos],"\t%s"LINEFEED,linechars);
            pos += linebytes+3;

            return_msg[pos] = '\r';
            return_msg[pos+1] = '\n';
            return_msg[pos+2] = '\0';

            sendto(socket,return_msg,pos+3,0);

            if (filehandle != mydirhandle)
                fclose(filehandle);
            return;
        } else {
            // can not open file
            sendMsg(socket,"Opening file failed. Error %d",filehandle);
            return;
        }
    }

    if (strpos("cat",command) == 0) {
        char** argv;
        int args = parseArgs(command,argv);
        if (args != 2)  {
            sendMsg(socket,"Usage: cat <filename>");
            return;
        }

        char* filename = argv[1];
        char path[100];

        if (filename[0] !='/') {
            /* relative path */
            strcpy(path,current_dir);
            strcat(&path[strlen(current_dir)],filename);
        } else {
            strcpy(path,filename);
        }

        compactPath(path);

        int filehandle = fopen(path,0);
        if (filehandle > 0) {
            stat_t filetype;
            filetype.st_type = 10;
            fstat(filehandle,&filetype);
            if (filetype.st_type & cTYPE_KVAR && filetype.st_type != 2) {
                /* Kernel Variable */
                /*SYSFS_SIGNED_INTEGER    = 0,
                SYSFS_UNSIGNED_INTEGER  = 1,
                SYSFS_STRING = 2 */
                if (filetype.st_flags < 2) {
                    char* format = "d (0x%x)\n";
                    if (filetype.st_flags == 1)
                        format = "%u (0x%x)\n";

                    switch (filetype.st_size) {
                    case 1:
                        char cvalue;
                        readKernelVarInt(filehandle,&cvalue,filetype.st_size);
                        sprintf(return_msg,format,cvalue,cvalue);
                        sendStr(socket,return_msg);
                        return;
                    case 2:
                        short svalue;
                        readKernelVarInt(filehandle,&svalue,filetype.st_size);
                        sprintf(return_msg,format,svalue,svalue);
                        sendStr(socket,return_msg);
                        return;
                    case 4:
                        int value;
                        readKernelVarInt(filehandle,&value,filetype.st_size);
                        sprintf(return_msg,format,value,value);
                        sendStr(socket,return_msg);
                        return;
                    }
                }

            } else {
                /* normal file.. just print content */
                int num = fread(return_msg,512,1,filehandle);
                if (num < 0) {
                    sendMsg(socket,"Error reading: %d",num);
                    num = 0; // check error
                }

                while (num == 512) {
                    // be sure the msg only contains telnet ascii chars
                    makeTelnetCharCompatible(return_msg,num);

                    //printf(return_msg);
                    sendData(socket,return_msg,num);

                    num = fread(return_msg,512,1,filehandle);
                    if (num < 0) {
                      sendMsg(socket,"Error reading: %d",num);
                      num = 0; // check error
                    }
                }

                // last packet
                makeTelnetCharCompatible(return_msg,num);
                return_msg[num] = '\r';
                return_msg[num+1] = '\n';
                return_msg[num+2] = '\0';

                //printf(return_msg);
                sendData(socket,return_msg,num+3);

                if (filehandle != mydirhandle)
                    fclose(filehandle);
                return;
            }
        } else {
            // can not open file
            sprintf(return_msg,"Opening file failed. Error %d",filehandle);
            int end = strlen(return_msg);
            return_msg[end] = '\r';
            return_msg[end+1] = '\n';
            return_msg[end+2] = '\0';
            sendData(socket,return_msg,end+3);
            return;
        }


    }


    if (strpos("ps",command) == 0) {
        printTasks(socket,1);
        return;
    }

    if (strpos("df",command) == 0) {
        int blocks = 0;

        /* display blocks instead of Bytes*/
        if (strpos("-b",command) > 0) {
            blocks = 1;
        }

        printMemUsage(socket, blocks);
        return;
    }

    if (strpos("mkdir",command) == 0) {
          char** argv;
          int args = parseArgs(command,argv);
          if (args != 2)  {
              sendMsg(socket,"Usage: mkdir <name>");
              return;
          }

          char* filename = argv[1];
          char path[100];

          if (filename[0] !='/') {
              /* relative path */
              strcpy(path,current_dir);
              strcat(&path[strlen(current_dir)],filename);
          } else {
              strcpy(path,filename);
          }

          printf("path: %s\r\n",path);

          compactPath(path);

          printf("path: %s\r\n",path);

           int res = fcreate(path,cTYPE_DIR);
           if (res < 0)
               sendMsg(socket,"Error creating directory",res);

           return;

       }


    if (strpos("ifconfig",command) == 0) {
        char** argv;
        int args = parseArgs(command,argv);
        if (args > 1) {
            char* interface = argv[1];

        } else  {
            /* display stats */
            int handle = fopen("/dev/comm",0);
            if (handle) {
                  Directory_Entry_t* direntry = readdir(handle);

                  char devpath[60];
                  sprintf(devpath,"/dev/comm/%s",direntry->name);
                  int devicehandle = fopen(devpath);
                  netif_stat_t netifstats;
                  ioctl(devicehandle,cNETIF_GET_STATS,&netifstats);

                  int len = sprintf(dir_content, "%s\tHWaddr: %02x:%02x:%02x:%02x:%02x:%02x\n",
                                    direntry->name,netifstats.hwaddr[0],netifstats.hwaddr[2],
                                    netifstats.hwaddr[2],netifstats.hwaddr[3],netifstats.hwaddr[4],
                                    netifstats.hwaddr[5]);
                  len += sprintf(dir_content+len, "   \tinet addr:%u.%u.%u.%u  Bcast:%u.%u.%u.255  Mask:%u.%u.%u.%u\n",
                                 (netifstats.ipv4addr >> 24) & 0xff,
                                 (netifstats.ipv4addr >> 16) & 0xff,
                                 (netifstats.ipv4addr >> 8) & 0xff,
                                 (netifstats.ipv4addr) & 0xff,
                                 (netifstats.ipv4addr >> 24) & 0xff,
                                 (netifstats.ipv4addr >> 16) & 0xff,
                                 (netifstats.ipv4addr >> 8) & 0xff,
                                 (netifstats.ipv4netmask >> 24) & 0xff,
                                 (netifstats.ipv4netmask >> 16) & 0xff,
                                 (netifstats.ipv4netmask >> 8) & 0xff,
                                 (netifstats.ipv4netmask) & 0xff
                                 );

                  inet_ntop(AF_INET6,netifstats.ipv6addr,devpath,60);
                  len += sprintf(dir_content+len, "   \tinet6 addr:%s\n",devpath);
                  len += sprintf(dir_content+len, "   \t");
                  if (netifstats.flags & NETIF_FLAG_UP)
                      len += sprintf(dir_content+len, "UP ");
                  else
                      len += sprintf(dir_content+len, "DOWN ");

                  if (netifstats.flags & NETIF_FLAG_BROADCAST)
                      len += sprintf(dir_content+len, "BROADCAST ");

                  if (netifstats.flags & NETIF_FLAG_POINTTOPOINT)
                      len += sprintf(dir_content+len, "PPP ");

                  if (netifstats.flags & NETIF_FLAG_DHCP)
                      len += sprintf(dir_content+len, "DHCP ");

                  len += sprintf(dir_content+len, "RUNNING  MTU:%u  Metric:1\n",netifstats.mtu);
                  len += sprintf(dir_content+len, "   \tRX packets:%u  TX packets:%u  errors:%u\n",
                                 netifstats.rxpackets,netifstats.txpackets,netifstats.errors);
                  len += sprintf(dir_content+len, "   \tRX bytes:%u (%u KiB) TX bytes:%u (%u KiB)\n\n",
                                 netifstats.rxbytes,netifstats.rxbytes/1024, netifstats.txbytes, netifstats.txbytes/1024);

                  sendStr(socket,dir_content);

                  fclose(devicehandle);
            }

            if (handle != mydirhandle) {
                fclose(handle);
            }

        }

        delete argv;
        return;
    }


    if (strpos("touch",command) == 0) {
       char** argv;
       int args = parseArgs(command,argv);
       if (args != 2)  {
           sendMsg(socket,"Usage: touch <filename>");
           return;
       }

       char* filename = argv[1];
       char path[100];

       if (filename[0] !='/') {
           /* relative path */
           strcpy(path,current_dir);
           strcat(&path[strlen(current_dir)],filename);
       } else {
           strcpy(path,filename);
       }

       compactPath(path);

        int res = fcreate(path);
        if (res < 0)
            sendMsg(socket,"Error creating file..");

        return;

    }

    if (strpos("rm",command) == 0) {
      char** argv;
      int args = parseArgs(command,argv);
      if (args != 2)  {
          sendMsg(socket,"Usage: rm <filename>");
          return;
      }

      char* filename = argv[1];
      char path[100];

      if (filename[0] !='/') {
          /* relative path */
          strcpy(path,current_dir);
          strcat(&path[strlen(current_dir)],filename);
      } else {
          strcpy(path,filename);
      }

      compactPath(path);

      int res = fremove(path);
      if (res < 0)
        sendMsg(socket,"Error removing file");
      return;

    }


    sendUnknownCommand(socket);
}


static char tty0buf[256];
static int newsock;

void* tty0_thread(void* arg) {
    int devid = (int) arg;
    while (1) {
        int read = fread(tty0buf, 256, 1, devid);
        if (read > 0 && newsock != 0) {
            sendData(newsock,tty0buf,read);
        }
        sleep(10);
    }
}

char recvMsg[1024];

extern "C" int task_main()
{
    int i = 0;
    newsock = 0;
    int mysock = socket(IPV4,SOCK_STREAM,TCP);

    // bind our socket to some address
    sockaddr* addr = (sockaddr*) malloc(sizeof(sockaddr));

    addr->port_data =     23;                      //< the port
    addr->sa_data   =     0;                       //< any address ...
    memcpy(addr->name_data,"TelnetServer\0",13);   //< register using this service name

    bind(mysock,addr);

    puts("Telnet-Server bound and waiting for clients."LINEFEED);

    /* create the virtual tty device */
    int devid = mkdev("tty0",2048);
    if (devid < 0) {
        printf("Error creating tty0: %d",devid);
        return 1;
    }

    /* set current threads name */
    thread_name(0,"telnetd");

    /* create the tty0 thread  */
    thread_attr_t attr;
    memset(&attr,0,sizeof(thread_attr_t));
    attr.priority = 1;
    int threadid = -1;
    thread_create(&threadid,&attr,tty0_thread,(void*) devid);
    thread_run(threadid);
    thread_name(threadid,"tty0");



    while(1)
    {
        newsock = listen(mysock);
        char* msgptr;
        cmd_pos = 0;
        doecho = DOECHO;
        mydirhandle = 0;

        if (doecho) {
            // send my telnet options
            telnetcommand[0] = 0xff;
            telnetcommand[1] = WILL;
            telnetcommand[2] = 1; // send will echo
            sendto(newsock,telnetcommand,3,0);
        }


        telnetcommand[0] = IAC;
        telnetcommand[1] = WILL;
        telnetcommand[2] = 3; //  IAC WILL SUPPRESS-GOAHEAD
        sendto(newsock,telnetcommand,3,0);

        telnetcommand[0] = 0xff;
        telnetcommand[1] = WONT;
        telnetcommand[2] = LINEMODE; //
        sendto(newsock,telnetcommand,3,0);

        //sendMsg(newsock,"^]mode char");
        telnetcommand[0] = 0xff;
        telnetcommand[1] = DONT;
        telnetcommand[2] = 1; // send will echo
        sendto(newsock,telnetcommand,3,0);


        history_count = 0;
        ci = 0;

        puts("New connection!"LINEFEED);
        sendto(newsock,welcome_msg,strlen(welcome_msg),0);

        current_dir[0] = '/';
        current_dir[1] = '\0';
        mydirhandle = fopen(current_dir,0);

        while (1) {
            msgptr = recvMsg;
            int msglen = recv(newsock,msgptr,1024,MSG_WAIT);
            if (msglen < 0) {
                // disconnected
                printf("Terminal disconnected..");
                fclose(newsock);
                newsock = 0;
                break;
            }

            /*puts("rx:");
            for (int i = 0; i< msglen; i++) {
                printf("%x ",msgptr[i]);
            }
            puts("\r\n");*/

            if (cmd_pos + msglen < 100) {

                for (int i = 0; i < msglen; i++ ){
                    if (msgptr[i] > 31 && msgptr[i] < 127) {
                        // normal char .. just copy
                        command[cmd_pos++] = msgptr[i];
                        if (doecho) sendto(newsock,&msgptr[i],1,0);
                    } else

                    // all other cases are special chars
                    switch (msgptr[i])  {
                        case 0xff : {
                            // handle telnet command
                            i++;
                            i += handleTelnetCommand(newsock,&msgptr[i]);
                            break;
                        }
                        /* backspace key */
                        case 0x7f:
                        case 0x8 : {
                            if (cmd_pos > 0) {
                                command[cmd_pos--] = 0;
                                if (doecho) sendto(newsock,&back_sequence,3,0);
                            }
                            break;
                        }
                        case 0x1b: {
                            // esc sequence
                            if ((msglen - i) > 2) {
                                // at least 2 more chars following .. thus might be a escape sequence
                                i+=2;
                                if (msgptr[i-1] == 0x5b) {
                                    switch (msgptr[i]) {
                                        case 0x41: { // oben
                                            if (history_count == 0) break;

                                            if (ci > 0) ci--;
                                            else ci = history_count-1;

                                            int num_back = cmd_pos;

                                            strcpy(command,command_history[ci]);
                                            cmd_pos=strlen(command);

                                            /* put cursor back down */
                                            //sendto(newsock,down_sequence,3,0);
                                            if (1) {
                                                for (int j2 = 0; j2 < num_back; j2++)
                                                    memcpy(&return_msg[j2*3],&back_sequence,3);

                                                sendto(newsock,return_msg,num_back*3,0);
                                            }
                                            if (1) sendto(newsock,&command,strlen(command),0);
                                            break;
                                        }
                                        case 0x42: {// unten
                                            if (history_count == 0) break;

                                            ci = (ci+1) & (MAX_COMMAND_HISTORY -1);
                                            if (ci >= history_count) ci = 0;

                                            int num_back = cmd_pos;

                                            strcpy(command,command_history[ci]);
                                            cmd_pos=strlen(command);

                                            if (1) {
                                                for (int j2 = 0; j2 < num_back; j2++)
                                                    memcpy(&return_msg[j2*3],&back_sequence,3);

                                                sendto(newsock,return_msg,num_back*3,0);
                                            }
                                            if (1) sendto(newsock,&command,strlen(command),0);
                                            break;
                                        }
                                        case 0x44: break; // nach links taste
                                        case 0x43: break; // rechts
                                    }
                                }

                            } // else ignore ESC
                            break;
                        }
                        case 13: {
                            i++;
                            if (msglen - i > 0) {
                                if (msgptr[i] == 0 || msgptr[i] == 0xa) {
                                    command[cmd_pos] = '\0';

                                    if (doecho) sendto(newsock,&return_sequence,2,0);
                                    /* send a linefeed */
                                    sendMsg(newsock,"");

                                    if (strlen(command) > 0) {
                                        // remember last command
                                        strcpy(command_history[ci],command);
                                        ci = (ci+1) & (MAX_COMMAND_HISTORY -1);

                                        if (history_count < MAX_COMMAND_HISTORY)
                                            history_count++;
                                    }

                                    // handle command
                                    handleCommand(newsock,cmd_pos);

                                    int dir_len = strlen(current_dir);
                                    memcpy(&return_msg[0],&current_dir[0],dir_len);
                                    return_msg[dir_len] = '$';
                                    return_msg[dir_len+1] = ' ';

                                    sendData(newsock,return_msg,dir_len+2);
                                    cmd_pos = 0;
                                }
                                i++;
                            }

                            break;
                        }
                        default: ;// do nothing
                    }

                }


            } // if msglen < 100



        } // while connected

    } // loop forever

}
