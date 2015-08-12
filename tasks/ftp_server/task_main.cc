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

#include <orcos.h>
#include <string.h>
#include <stdio.h>
#include <string.hh>
#include <stdlib.h>
#include <stdarg.h>

#define IPV4 0x800
#define TCP 0x6


#define LINEFEED "\r\n"

const char* welcome_msg = "220 ORCOS FTP Server ready.\r\n";

// the current command being received
static char command[100];

// current directory we are in
static char current_dir[100];

// last directory
static char last_dir[100];

static char return_msg[520];

static char dir_msg[1024];

static char temp_msg[200];

static char dataBuffer[2048];

char recvCommand[100];

char renameFromFile[256];
char renameToFile[256];


//file handle to the cwd directory
static int mydirhandle = 0;

// buffer used for reading the directory contents
static char dir_content[4096];
// temporary line buffer
char line[150];
// Address of remote data socket to send data to
sockaddr dataremote;


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

char* extractFilePath(char* &str) {

    char* ret = str;

    int len = strlen(str);
    for (int i = len-1; i > 0; i--) {
        if (str[i] == '/') {
            str[i] = 0;
            str = &str[i+1];
            return ret;
        }
    }

    return 0;
}


// reduces the path by "." and ".." statements
void compactPath(char* path) {

    char newpath[100];
    newpath[0] = '/';
    newpath[1] = '\0';


    char* token = strtok(path,"/");
    char* next_token;

    while (token != 0) {

        next_token = strtok(0,"/");

        bool nextisparent = false;
        if ((next_token != 0) && ((strcmp(next_token,"..") == 0))) nextisparent = true;

        if ((strcmp(token,".") != 0) && !nextisparent && (strcmp(token,"..") != 0 )) {
            strcat(newpath,token);
            strcat(newpath,"/");
        }

        token = next_token;
    }

    memcpy(path,newpath,strlen(newpath)+1);
}



void sendMsg(int socket, char* msg) {
    sprintf(return_msg,msg);
    int end = strlen(return_msg);
    return_msg[end] = '\r';
    return_msg[end+1] = '\0';
    sendto(socket,return_msg,end+1,0);
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

//char line[50];

void hexdump(char* dataptr,int msglen) {

    int num = 0;
    int num2 = 0;

    while( num < msglen) {
        num2 = num + 16;
        if (num2 >= msglen) num2= msglen;

        for (int i = num; i < num2; i++) {
            sprintf(&line[(i-num)*3],"%02x  ",dataptr[i]);
        }

        line[(num2-num)*3 +1] = 0;
        printf("%s\r",line);
        num += 16;
    }

}


void sendResponse(int socket, char* msg, ...) {
    va_list arglist;
    va_start(arglist, msg);
    vsprintf(return_msg, msg, arglist);
    va_end(arglist);

    sendto(socket, return_msg, strlen(return_msg), 0);
    printf("< %s", return_msg);
}

int ReceiveFile(int controlsock, int datasock, sockaddr *dataremote, char* file) {

    if (connect(datasock,dataremote) != 0) {
        puts("ReceiveFile(): Error Connecting..\r\n");
        sendResponse(controlsock, "425 Cannot open data connection\r\n");
        return -1;
    }

    puts("Data Connection established..\r\n");

    printf("File: '%s'\r\n",file);

    int res = create(file);
    if (res < 0) {
        sendResponse(controlsock, "451 Error creating file\r\n");
        return -1;
    }

    // file created .. read data from datasock
    int msglen = 0;
    int timeout = 5;
    char* dataptr = dataBuffer;
    msglen = recv(datasock,dataptr,2048,MSG_WAIT,200);
    if (msglen < 0) {
        close(res);
        puts("Timeout receiving file..\r\n");
        return (-1);
    }
    // while not disconnected try reading data
    while (msglen != -1) {
        if (msglen <= 0) {
            // nothing received
            timeout--;
            if (timeout == 0) {
                // error in connection
                sendResponse(controlsock, "426 Timeout receiving data\r\n");
                close(res);
                puts("Timeout receiving file..\r\n");
                return (-1);
            }
        } else {
            // write to file
            int error = write(res,dataptr,msglen);
            if (error < 0) {
                close(res);
                return (error);
            }
            timeout = 5;
        }

        msglen = recv(datasock,dataptr,2048,MSG_WAIT,200);
    }

    puts("File completely received..\r\n");

    close(res);
    return (0);
}


int sendFile(int controlsock, int datasock, sockaddr *dataremote, int handle) {

    if (connect(datasock,dataremote) != 0) {
        puts("sendFile(): Error Connecting..\r\n");
        sendResponse(controlsock, "425 Cannot open data connection\r\n");
        return -1;
    }

    puts("Data Connection established..\r\n");

    // now read file and send over data connection
    int num = read(handle, return_msg, 512);
    if (num == cEOF) {
        return (0);
    }
    if (num < 0) {
        printf("Error reading file: %s\r\n", strerror(num));
        sendResponse(controlsock, "550 Error reading file: %s\r\n", strerror(num));
        return (-1);
    }


    while (num == 512) {
        int timeout = 500;
        // try sending. if failed sleep
        // failing may happen if no more free memory is available
        // inside the TCP/IP stack to hold the packet until acked
        while (sendto(datasock,return_msg,num,0) != 0) {
            usleep(1000);
            timeout--;
            if (timeout == 0) {
                puts("Timeout sending data..\r\n");
                sendResponse(controlsock, "426 Timeout sending data\r\n");
                return -1;
            }
        }

        num = read(handle, return_msg, 512);
        if (num == cEOF) {
           return (0);
        }
        if (num < 0) {
            printf("Error reading file: %s\r\n", strerror(num));
            sendResponse(controlsock, "550 Error reading file: %s\r\n", strerror(num));
            return (-1);
        }
    }

    // send last bytes if any
    if (num > 0) {
        int timeout = 500;
        while (sendto(datasock, return_msg, num, 0) != 0) {
           usleep(1000);
           timeout--;
           if (timeout == 0) {
               puts("Timeout sending data..\r\n");
               sendResponse(controlsock, "426 Timeout sending data\r\n");
               return -1;
           }
       }
    }

    usleep(5000);
    puts("File completely send..\r\n");
    return 0;

}

static char timestr[40];

static const char mon_name[][4] = {
   "Jan", "Feb", "Mar", "Apr", "May", "Jun",
   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
 };

void sendDirectoryContents(int controlsock, int datasock, sockaddr *dataremote) {

    if (connect(datasock,dataremote) != 0) {
        puts("sendDirectoryContents(): Error Connecting..\r\n");
        sendResponse(controlsock, "425 Cannot open data connection\r\n");
        return;
    }

    puts("Data Connection established..\r\n");

    if (mydirhandle == 0) {
        puts("Internal Error: No valid Directory handle..\r\n");
        return;
    }


    Directory_Entry_t* direntry = readdir(mydirhandle);
    char* retmsg = dir_msg;
    memset(retmsg,0,10);

    while (direntry) {

        struct tm pTm;
        time2date(&pTm, direntry->datetime);
        sprintf(timestr, "%s %2d %02d:%02d", mon_name[pTm.tm_mon], pTm.tm_mday,  pTm.tm_hour, pTm.tm_min );

        if (direntry->resType == 1) {
            sprintf(line, "%s   1 %-10s %-10s %10u %s %s\r\n",
                          "d-rw-rw-rw-", "User", "User", 0, timestr, direntry->name);
        } else {
            //sprintf(line, "%s   1 %-10s %-10s %10u Jan  1  1980 %s\r\n",
            sprintf(line, "%s   1 %-10s %-10s %10u %s %s\r\n",
                          "-rw-rw-rw-", "User", "User", direntry->filesize, timestr, direntry->name);
        }

        // concat the line
        retmsg = strcat(retmsg,line);

        if (strlen(dir_msg) > 800) {
            //puts(dir_msg);
            sendto(datasock,dir_msg,strlen(dir_msg),0);
            retmsg = dir_msg;
            memset(retmsg,0,10);
        }

        direntry = readdir(mydirhandle);
    }


    sendto(datasock,dir_msg,strlen(dir_msg),0);
}




int htonl(int n)
{
    return ((n & 0xff) << 24) |
    ((n & 0xff00) << 8) |
    ((n & 0xff0000UL) >> 8) |
    ((n & 0xff000000UL) >> 24);
}

char* getFTPPath(char* msgptr) {
   int len = strpos("\r\n", msgptr);
   memcpy(temp_msg, msgptr, len);
   temp_msg[len] = 0;

   char* file = &temp_msg[0];
   if (file[0] != '/') {
       // relative path
       strcpy(temp_msg, current_dir);
       strcat(temp_msg, &msgptr[0]);
       len           = strpos("\r\n", temp_msg);
       temp_msg[len] = 0;
       file          = temp_msg;
   }

   return (file);
}



extern "C" int main(int argc, char** argv) __attribute__((used));

extern "C" int main(int argc, char** argv) {
    int i;
    puts("FTP-Server starting.\n");
    int mysock = socket(IPV4,SOCK_STREAM,TCP);

    // bind our socket to some address
    sockaddr* addr = (sockaddr*) malloc(sizeof(sockaddr));

    addr->port_data     =     21;                         //< the ftp port
    addr->sa_data       =     IP4ADDR(192,168,1,100);

    bind(mysock,addr);

    // prepare data local sock address structure
    sockaddr* dataaddr  =  (sockaddr*) malloc(sizeof(sockaddr));
    dataaddr->port_data =     0;                           //< let tcp stack choose a port for us
    dataaddr->sa_data   =     IP4ADDR(192,168,1,100);

    // our handle to the data socket which is created on demand
    int datasock;
    // the current msg received
    char* msgptr;

    thread_name(0, "ftpd");
    puts("FTP-Server bound and waiting for clients.\n");

    while(1)
    {
        // wait for new connection
        int newsock = listen(mysock);
        if (newsock < 0) {
            printf("Listen error %d: %s\n", newsock, strerror(newsock));
            continue;
        }

        puts("New FTP connection!\r");
        sendto(newsock,welcome_msg,strlen(welcome_msg),0);

        // reset data sockaddr
        dataremote.sa_data = 0;

        // reset directory to root
        current_dir[0] = '/';
        current_dir[1] = '\0';

        if (mydirhandle != 0) close(mydirhandle);
        mydirhandle = open(current_dir,0);
        int disconnectCounter = 0;


        while (1) {

            msgptr = recvCommand;
            /* wait for 200 milliseconds on next packet */
            int msglen = recv(newsock, msgptr, 100, MSG_WAIT, 200);

            if (msglen == -1) {
                // disconnected
                if (mydirhandle != 0) close(mydirhandle);
                mydirhandle = 0;
                puts("FTP Client disconnected..");
                close(newsock);
                break;
            }
            if (msglen <= 0) {
                disconnectCounter++;
                /* if we did not receive anything for 20 seconds.. we disconnect*/
                if (disconnectCounter > 100) {
                    if (mydirhandle != 0) {
                        close(mydirhandle);
                    }
                    mydirhandle = 0;
                    puts("Disconnecting..");
                    close(newsock);
                    break;
                }
                continue;
            }

            disconnectCounter = 0;

            // be sure the string is null terminated
            msgptr[msglen] = 0;
            printf("> %s", msgptr);

            // TODO: add mkdir

         if (strpos("USER",msgptr) == 0) {
             /***********************************************
              *     USER LOGIN
              ***********************************************/
             //no login required
             sendResponse(newsock, "200 \r\n");
         } else if (strpos("PWD",msgptr) == 0) {
             /***********************************************
              *     GET CURRENT WORKING DIRECTORY
              ***********************************************/

             sendResponse(newsock, "257 \"%s\" is current directory\r\n", current_dir);
         } else if (strpos("MKD",msgptr) == 0) {
            /***********************************************
             *     GET CURRENT WORKING DIRECTORY
             ***********************************************/
             int len = strpos("\r\n",msgptr);
             memcpy(temp_msg,msgptr,len);
             temp_msg[len] = 0;

             char* file = &temp_msg[4];
             if (file[0] != '/') {
                 // relative path
                 strcpy(temp_msg, current_dir);
                 strcat(temp_msg, &msgptr[4]);
                 len = strpos("\r\n",temp_msg);
                 temp_msg[len] = 0;
                 file  = temp_msg;
             }

             printf("Create Directory '%s'\r\n", file);

             int err = create(file, cTYPE_DIR);
             if (err >= 0) {
                 sendto(newsock,"250 \r\n",6,0);
             } else {
                 printf("Could not create '%s'. Error: %s\r\n", file, strerror(err));
                 sendResponse(newsock, "450 Error creating directory: %s\r\n", strerror(err));
             }
         } else if (strpos("RNFR",msgptr) == 0) {
             /***********************************************
              *     RENAME FROM
              ***********************************************/
             int len = strpos("\r\n",msgptr);
             memcpy(temp_msg, msgptr, len);
             temp_msg[len] = 0;

             char* file = &temp_msg[5];
             if (file[0] != '/') {
                 // relative path
                 strcpy(temp_msg, current_dir);
                 strcat(temp_msg, &msgptr[5]);
                 len = strpos("\r\n",temp_msg);
                 temp_msg[len] = 0;
                 file  = temp_msg;
             }
             strcpy(renameFromFile, file);

             // going to transfer directory content a.s.o
             sendResponse(newsock, "350 \r\n");

         } else if (strpos("RNTO", msgptr) == 0) {
             /***********************************************
              *    RENAME TO
              ***********************************************/
             int len = strpos("\r\n",msgptr);
             memcpy(temp_msg, msgptr, len);
             temp_msg[len] = 0;

             char* file = &temp_msg[5];
             if (file[0] != '/') {
                 // relative path
                 strcpy(temp_msg, current_dir);
                 strcat(temp_msg, &msgptr[5]);
                 len = strpos("\r\n",temp_msg);
                 temp_msg[len] = 0;
                 file  = temp_msg;
             }
             strcpy(renameToFile, file);

             if (strcmp(dirname(renameToFile), dirname(renameFromFile)) ==  0) {
                 // check if file exists and can be opened
                 int fd = open(renameFromFile);
                 if (fd < 0) {
                     sendResponse(newsock, "450 Error renaming. Error opening %s.\r\n", renameFromFile);
                 } else {

                     sendResponse(newsock, "250 \r\n");
                 }

             } else {
                 printf("Cannot rename files. Directory names are not equal.. %s != %s\n", dirname(renameToFile), dirname(renameFromFile));
                 sendResponse(newsock, "450 Error renaming. Directories do not match.\r\n");
             }

         } else if (strpos("TYPE A",msgptr) == 0) {
             /***********************************************
              *     PREPARE FOR ASCII TRANSFER
              ***********************************************/

             // going to transfer directory content a.s.o
             sendResponse(newsock, "200 \r\n");

         } else if (strpos("TYPE I",msgptr) == 0) {
             /***********************************************
              *     PREPARE FOR BINARY TRANSFER
              ***********************************************/

             // we are going to transfer binary data
             sendResponse(newsock, "200 \r\n");

         }  else if (strpos("noop",msgptr) == 0) {
             /***********************************************
              *     NOOP
              ***********************************************/

             sendResponse(newsock, "200 \r\n");
         } else if (strpos("PASV",msgptr) == 0) {
             /***********************************************
              *     REQUEST PASSIVE MODE
              ***********************************************/
             sendResponse(newsock, "500 PASV mode not supported.\r\n");
         } else if (strpos("DELE ",msgptr) == 0) {
             /***********************************************
              *     DELETE FILE
              ***********************************************/
              int len = strpos("\r\n",msgptr);
              memcpy(temp_msg,msgptr,len);
              temp_msg[len] = 0;

              char* file = &temp_msg[5];
              if (file[0] != '/') {
                 // relative path
                 strcpy(temp_msg, current_dir);
                 strcat(temp_msg, &msgptr[5]);
                 len = strpos("\r\n",temp_msg);
                 temp_msg[len] = 0;
                 file  = temp_msg;
              }

              printf("Delete file '%s'\r\n",file);

              int status = remove(file);

              if (status == 0) {
                 // tell control connection that data has been received
                 sendResponse(newsock, "250 File deleted.\r\n");
              } else {
                 printf("Could not delete '%s'. Error: %d\r\n",file,status);
                 sendResponse(newsock, "450 Error deleting file.\r\n");
              }

         }  else if (strpos("STOR ",msgptr) == 0) {
             /***********************************************
              *     STORE FILE
              ***********************************************/
             int len = strpos("\r\n",msgptr);
             memcpy(temp_msg,msgptr,len);
             temp_msg[len] = 0;

             char* file = &temp_msg[5];
             if (file[0] != '/') {
                 // relative path
                 strcpy(temp_msg,current_dir);
                 strcat(temp_msg,&msgptr[5]);
                 len = strpos("\r\n",temp_msg);
                 temp_msg[len] = 0;
                 file  = temp_msg;
             }

             printf("Store file '%s'\r\n",file);

             sendResponse(newsock, "150 Opening BINARY mode data connection\r\n");

             // create new socket
             datasock = socket(IPV4,SOCK_STREAM,TCP);
             // bind to local port
             bind(datasock,dataaddr);

             //puts("Sending Directory Contents..\r\n");
             int status = ReceiveFile(newsock,datasock,&dataremote,file);

             // stream mode must close the socket again..
             close(datasock);

             if (status == 0) {
                // tell control connection that data has been received
                sendResponse(newsock, "226 Transfer complete.\r\n");
             } else {
                sendResponse(newsock, "552 Error storing file.\r\n");
             }

         } else if (strpos("RETR ",msgptr) == 0) {
             /***********************************************
              *     RETRIEVE FILE
              ***********************************************/

             int len = strpos("\r\n", msgptr);
             memcpy(temp_msg,msgptr,len);
             temp_msg[len] = 0;

             char* file = &temp_msg[5];
             if (file[0] != '/') {
                 // relative path
                 strcpy(temp_msg,current_dir);
                 strcat(temp_msg,&msgptr[5]);
                 len = strpos("\r\n",temp_msg);
                 temp_msg[len] = 0;
                 file  = temp_msg;
             }


             printf("Retrieve for file '%s'\r\n",file);

             // try opening
             int handle = open(file,0);

             if (handle >= 0) {
                // file opened .. send
                // create new socket
                datasock = socket(IPV4,SOCK_STREAM,TCP);
                // bind to local port
                bind(datasock, dataaddr);

                sendResponse(newsock, "150 Opening BINARY mode data connection\r\n");

                int status = sendFile(newsock,datasock,&dataremote,handle);

                // stream mode must close the socket again..
                close(datasock);

                if (status == 0) {
                    // tell control connection that data has been send
                    sendResponse(newsock, "226 Transfer complete.\r\n");
                }

                close(handle);
             } else {
                 sendResponse(newsock, "450 Could not open '%s'. Error: %d\r\n", file,handle);
             }

         }  else if (strpos("LIST",msgptr) == 0) {
             /***********************************************
              *     SEND DIRECTORY LIST
              ***********************************************/

                if (dataremote.sa_data != 0) {
                    sendResponse(newsock, "150 Opening ASCII mode data connection\r\n");

                    // create new socket
                    datasock = socket(IPV4,SOCK_STREAM,TCP);
                    // bind to local port
                    bind(datasock, dataaddr);

                    //puts("Sending Directory Contents..\r\n");
                    sendDirectoryContents(newsock, datasock, &dataremote);

                    // tell control connection that data has been send
                    sendResponse(newsock, "226 Transfer complete.\r\n");

                    // stream mode must close the socket again..
                    close(datasock);
                } else {
                    sendResponse(newsock, "426 \r\n",6,0);
                }

         } else if (strpos("PORT ",msgptr) == 0) {

             /***********************************************
              *     SET REMOTE PORT
              ***********************************************/

             char* val = &msgptr[5];

             int len = strpos(",",val);
             val[len] = 0;
             int num = atoi(val) << 24;

             val = &val[len+1];
             len = strpos(",",val);
             val[len] = 0;
             num = num | (atoi(val) << 16);

             val = &val[len+1];
             len = strpos(",",val);
             val[len] = 0;
             num = num | (atoi(val) << 8);

             val = &val[len+1];
             len = strpos(",",val);
             val[len] = 0;
             num = num | atoi(val) ;

             dataremote.sa_data = htonl(num);

             val = &val[len+1];
             len = strpos(",",val);
             val[len] = 0;
             int port = atoi(val) << 8;

             val = &val[len+1];
             len = strpos("\r\n",val);
             val[len] = 0;
             port = port | atoi(val);

            // printf("Data Connection to: %x , port: %d\r\n", (unsigned int) num,port);

             dataremote.port_data = port;

             sendResponse(newsock, "200 PORT command successful.\r\n");
         } // PORT
         else if (strpos("CWD ", msgptr) == 0) {
             /***********************************************
              *     CHANGE CURRENT WORKING DIRECTORY
              ***********************************************/
            char* dir = getFTPPath(&msgptr[4]);

            memcpy(last_dir,current_dir,100);

            // open dir
            int newdir_len = strlen(dir);
            if (dir[newdir_len-1] == '/') newdir_len--;

            memcpy(current_dir,dir,newdir_len);
            current_dir[newdir_len] = '/';
            current_dir[newdir_len+1] = '\0';

            compactPath(current_dir);

            // try opening the new directory
            int handle = open(current_dir,0);

            if (handle >= 0) {
                // success
                if ((mydirhandle != 0) && (mydirhandle != handle )) close(mydirhandle);
                mydirhandle = handle;
                sendResponse(newsock, "250 \"%s\" is current directory.\r\n", current_dir);
            } else {
                sendResponse(newsock, "450 Could not open \"%s\". Error: %s\r\n", current_dir, strerror(handle));
                memcpy(current_dir, last_dir, 100);
            }

         } else if (strpos("SIZE ",msgptr) == 0) {
             char* file = getFTPPath(&msgptr[5]);

             // try opening the file
             int handle = open(file, 0);
             if (handle >= 0) {
                // success
                 struct stat filetype;
                 filetype.st_type = 10;
                 fstat(handle, &filetype);
                 close(handle);
                 sendResponse(newsock, "213 %u\r\n", filetype.st_size);
             } else {
                printf("Could not open '%s'. Error: %s\r\n", current_dir, strerror(handle));
                sendResponse(newsock, "550 Could not open '%s'. Error: %s\r\n", current_dir, strerror(handle));
                memcpy(current_dir, last_dir, 100);
             }
         }
         else {
             /***********************************************
              *     UNKNOWN COMMAND
              ***********************************************/
             printf("Unknown Command: %s", msgptr);
             sendResponse(newsock, "500 command not understood.\r\n", msgptr);
         }



        } // while connected

    } // loop forever

}
