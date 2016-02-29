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

#include "ftpserver.hh"

#include <orcos.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define IPV4 0x800
#define TCP 0x6

#define LINEFEED "\r\n"

const char* welcome_msg = "220 ORCOS FTP Server ready.\r\n";



unint2 htons(unint2 n) {
    return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

void FTPServer::sendResponse(int socket, char* msg, ...) {
    va_list arglist;
    va_start(arglist, msg);
    vsnprintf(responseMsg, 255, msg, arglist);
    va_end(arglist);

    send(socket, responseMsg, strlen(responseMsg));
    printf("< %s", responseMsg);
}

ErrorT FTPServer::receiveFile(int datasock,int fd) {
    int   msglen    = 0;
    int   timeout   = 5;
    char* dataptr   = dataBuffer;

    msglen = recv(datasock, dataptr, 2048, MSG_WAIT, 2000);
    if (msglen < 0) {
        close(fd);
        sendResponse(socketfd, "426 Timeout receiving data\r\n");
        puts("Timeout receiving file..\n");
        return (msglen);
    }

    // while not disconnected try reading data
    while (msglen != -1) {
        if (msglen <= 0) {
            // nothing received
            timeout--;
            if (timeout == 0) {
                // error in connection
                sendResponse(socketfd, "426 Timeout receiving data\r\n");
                close(fd);
                puts("Timeout receiving file..\n");
                return (cError);
            }
        } else {
            // write to file
            int error = write(fd, dataptr, msglen);
            if (error < 0) {
                sendResponse(socketfd, "426 Error writing data\r\n");
                close(fd);
                return (error);
            }
            timeout = 10;
        }

        msglen = recv(datasock, dataptr, 2048, MSG_WAIT, 200);
    }

    puts("File completely received..\r\n");

    close(fd);
    return (cOk);
}


ErrorT FTPServer::sendFile(int datasock, int handle) {

    // now read file and send over data connection
    int num = read(handle, dataBuffer, 1400);
    if (num == cEOF) {
        return (cOk);
    }

    if (num < 0) {
        printf("Error reading file: %s\r\n", strerror(num));
        sendResponse(socketfd, "550 Error reading file: %s\r\n", strerror(num));
        return (num);
    }

    while (num > 0) {
        int err = send(datasock, dataBuffer, num);
        if (err < 0) {
            printf("Error sending data: %s\r\n", strerror(err));
            sendResponse(socketfd, "426 Error sending data: %s\r\n", strerror(err));
            return (num);
        }

        num = read(handle, dataBuffer, 1400);
    }

    close(handle);
    puts("File completely send..\r\n");
    return cOk;
}


static const char mon_name[][4] = {
   "Jan", "Feb", "Mar", "Apr", "May", "Jun",
   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
 };

ErrorT FTPServer::sendDirectoryContents(int datasock) {
    // temporary line buffer
    char line[256];
    line[255] = 0;
    char timestr[40];

    int mydirhandle = open(cwd);

    if (mydirhandle < 0) {
        puts("Internal Error opening directory...\n");
        return (mydirhandle);
    }

    Directory_Entry_t* direntry = readdir(mydirhandle);
    char* retmsg = dataBuffer;
    memset(retmsg, 0, 2048);

    while (direntry)
    {
        struct tm* pTm;
        long int datetime = direntry->datetime;
        pTm = gmtime(&datetime);
        snprintf(timestr, 40, "%3.3s %2d %02d:%02d", mon_name[pTm->tm_mon], pTm->tm_mday,  pTm->tm_hour, pTm->tm_min );

        if (direntry->resType == 1) {
            snprintf(line, 255, "d-rw-rw-rw-  1 %-8s %-8s %10u %s %s\r\n", "root", "root", 0, timestr, direntry->name);
        } else {
            snprintf(line, 255, "--rw-rw-rw-  1 %-8s %-8s %10u %s %s\r\n", "root", "root", direntry->filesize, timestr, direntry->name);
        }

        // concat the line
        retmsg = strcat(retmsg, line);

        int buflen = strlen(dataBuffer);

        if (buflen > 700) {
            sendto(datasock, dataBuffer, buflen, 0);
            retmsg = dataBuffer;
            memset(retmsg, 0, 2048);
        }

        direntry = readdir(mydirhandle);
    }

    close(mydirhandle);
    sendto(datasock, dataBuffer, strlen(dataBuffer), 0);
    return cOk;
}

int htonl(int n)
{
    return ((n & 0xff) << 24) |
    ((n & 0xff00) << 8) |
    ((n & 0xff0000UL) >> 8) |
    ((n & 0xff000000UL) >> 24);
}

char* FTPServer::getFTPPath(char* msgptr) {

   int len = strpos("\r\n", msgptr);
   if (len > 200) {
       return ("");
   }
   msgptr[len] = 0;
   tmp_path[0] = 0;

   if (msgptr[0] != '/') {
       // relative path
       strcpy(tmp_path, cwd);
       strcat(tmp_path, "/");
   }

   strcat(tmp_path, msgptr);
   return (tmp_path);
}


FTPServer::FTPServer(int socket)
{
    this->socketfd = socket;

    // reset directory to root
    cwd[0] = '/';
    cwd[1] = '\0';
    lwd[0] = '/';
    lwd[1] = '\0';
}



void FTPServer::data_thread_entry()
{
    if (!datasocket) {
        return;
    }

    /* Wait for incoming connection. 2s timeout.. */
    int newsock = listen(datasocket, 1, 2000);
    if (newsock < 0) {
       printf("Data-Connection listen error: %s\n", strerror(newsock));
       goto out;
    }

    //puts("Data Connection Established\n");
    if (newsock > 0) {
        int timeout = 100;
        // wait for operation to start
        while (!data_command && timeout) {
            usleep(10000);
            timeout--;
        }
        if (timeout == 0) {
            goto out;
        }
        //printf("Data Command: %d\n", data_command);
        // do operation
        switch (data_command) {
            case DATA_CMD_LIST:  data_result = sendDirectoryContents(newsock); break;
            case DATA_CMD_STOR:  data_result = receiveFile(newsock, data_fd); break;
            case DATA_CMD_RETR:  data_result = sendFile(newsock, data_fd); break;
        }

        close(newsock);
        data_command = 0;
    }

    //puts("Data Connection Closed\n");
out:
    ftp_mode &= ~FTP_PASV;
    close(datasocket);
    datasocket = 0;
}

void* data_ftp_entry(void* arg) {
    FTPServer* server = (FTPServer*) arg;
    server->data_thread_entry();
    return 0;
}

void FTPServer::thread_entry() {
    int connected;
    connected = 1;

    send(socketfd, welcome_msg, strlen(welcome_msg));

    // prepare data local sock address structure
    sockaddr* dataaddr  =  (sockaddr*) malloc(sizeof(sockaddr));
    dataaddr->port_data =  0;   //< let tcp stack choose a port for us
    dataaddr->sa_data   =  IP4ADDR(192,168,1,100);

    // reset data sockaddr
    dataremote.sa_data = 0;

    // our handle to the data socket which is created on demand
    int datasock;
    // the current msg received
    char* msgptr;
    char  rcvbuf[512];
    int   disconnectCounter = 0;
    char  temp_msg[256];

    while (1) {
        msgptr = rcvbuf;
        /* wait for 200 milliseconds on next packet */
        int msglen = recv(socketfd, msgptr, 512, MSG_WAIT, 200);

        if (msglen == -1) {
            // disconnected
            puts("FTP Client disconnected..\n");
            close(socketfd);
            break;
        }
        if (msglen <= 0) {
            disconnectCounter++;
            /* if we did not receive anything for 20 seconds.. we disconnect*/
            if (disconnectCounter > 200) {
                puts("Disconnecting..\n");
                close(socketfd);
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
         sendResponse(socketfd, "200 \r\n");
     } else if (strpos("PWD",msgptr) == 0) {
         /***********************************************
          *     GET CURRENT WORKING DIRECTORY
          ***********************************************/

         sendResponse(socketfd, "257 \"%s\" is current directory\r\n", cwd);
     } else if (strpos("MKD",msgptr) == 0) {
        /***********************************************
         *     MAKE DIRECTORY
         ***********************************************/
         int len = strpos("\r\n",msgptr);
         memcpy(temp_msg, msgptr, len);
         temp_msg[len] = 0;

         char* file = &temp_msg[4];
         if (file[0] != '/') {
             // relative path
             strcpy(temp_msg, cwd);
             strcat(temp_msg, "/");
             strcat(temp_msg, &msgptr[4]);
             len = strpos("\r\n",temp_msg);
             temp_msg[len] = 0;
             file  = temp_msg;
         }

         printf("Create Directory '%s'\n", file);

         int err = create(file, cTYPE_DIR);
         if (err >= 0) {
             sendResponse(socketfd, "250 \r\n");
             close(err);
         } else {
             printf("Could not create '%s'. Error: %s\n", file, strerror(err));
             sendResponse(socketfd, "450 Error creating directory: %s\r\n", strerror(err));
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
             strcpy(temp_msg, cwd);
             strcat(temp_msg, "/");
             strcat(temp_msg, &msgptr[5]);
             len = strpos("\r\n",temp_msg);
             temp_msg[len] = 0;
             file  = temp_msg;
         }
         strcpy(renameFromFile, file);

         // going to transfer directory content a.s.o
         sendResponse(socketfd, "350 \r\n");

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
             strcpy(temp_msg, cwd);
             strcat(temp_msg, "/");
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
                 sendResponse(socketfd, "450 Error renaming. Error opening %s.\r\n", renameFromFile);
             } else {
                 frename(fd, basename(renameToFile));
                 sendResponse(socketfd, "250 \r\n");
             }

         } else {
             printf("Cannot rename files. Directory names are not equal.. %s != %s\n", dirname(renameToFile), dirname(renameFromFile));
             sendResponse(socketfd, "450 Error renaming. Directories do not match.\r\n");
         }

     } else if (strpos("TYPE A",msgptr) == 0) {
         /***********************************************
          *     PREPARE FOR ASCII TRANSFER
          ***********************************************/

         // going to transfer directory content a.s.o
         sendResponse(socketfd, "200 \r\n");

     } else if (strpos("TYPE I",msgptr) == 0) {
         /***********************************************
          *     PREPARE FOR BINARY TRANSFER
          ***********************************************/

         // we are going to transfer binary data
         sendResponse(socketfd, "200 \r\n");

     }  else if (strpos("noop",msgptr) == 0) {
         /***********************************************
          *     NOOP
          ***********************************************/

         sendResponse(socketfd, "200 \r\n");
     } else if (strpos("PASV",msgptr) == 0) {
         /***********************************************
          *     REQUEST PASSIVE MODE
          ***********************************************/
         if (datasocket != 0) close(datasocket);

         datasocket = socket(IPV4, SOCK_STREAM, TCP);
         if (datasocket < 0)
         {
             sendResponse(socketfd, "500 Error entering passive mode\r\n");
             continue;
         }
         // bind our socket to some address

         datasockaddr.port_data     =     0;
         datasockaddr.sa_data       =     IP4ADDR(192, 168, 1, 100);
         bind(datasocket, &datasockaddr);

         printf("Data socket bound to port : %d\n", datasockaddr.port_data);
         int addr = htonl(datasockaddr.sa_data);
         int port = datasockaddr.port_data;

         thread_attr_t attr;
         memset(&attr, 0, sizeof(thread_attr_t));

         attr.stack_size = 4096;
         attr.priority   = 50;
         data_command    = 0;

         ThreadIdT threadid;
         if (thread_create(&threadid, &attr, data_ftp_entry, (void*) this) == 0) {
             thread_name(threadid,"ftp data");
             thread_run(threadid);

             ftp_mode |= FTP_PASV;
             sendResponse(socketfd, "227 Entering Passive Mode (%u,%u,%u,%u,%u,%u)\r\n",
                          (addr >> 24) & 0xff, (addr >> 16) & 0xff, (addr >> 8) & 0xff, (addr >> 0) & 0xff,
                          (port >> 8) & 0xff, port & 0xff);
         } else {
             sendResponse(socketfd, "500 Error entering passive mode\r\n");
         }

     } else if (strpos("RMD ",msgptr) == 0) {
         /***********************************************
          *     REMOVE DIRECTORY
          ***********************************************/
         int len = strpos("\r\n",msgptr);
         memcpy(temp_msg,msgptr,len);
         temp_msg[len] = 0;

         char* file = &temp_msg[4];
         if (file[0] != '/') {
            // relative path
            strcpy(temp_msg, cwd);
            strcat(temp_msg, "/");
            strcat(temp_msg, &msgptr[4]);
            len = strpos("\r\n",temp_msg);
            temp_msg[len] = 0;
            file  = temp_msg;
         }

         printf("Delete directory '%s'\n",file);

         int status = remove(file);
         if (status == 0) {
           // tell control connection that data has been received
           sendResponse(socketfd, "250 Directory deleted.\r\n");
         } else {
           printf("Could not delete '%s'. Error: %d\n",file,status);
           sendResponse(socketfd, "450 Error deleting directory: %s\r\n", strerror(status));
         }
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
             strcpy(temp_msg, cwd);
             strcat(temp_msg, "/");
             strcat(temp_msg, &msgptr[5]);
             len = strpos("\r\n",temp_msg);
             temp_msg[len] = 0;
             file  = temp_msg;
          }

          printf("Delete file '%s'\n",file);

          int status = remove(file);

          if (status == 0) {
             // tell control connection that data has been received
             sendResponse(socketfd, "250 File deleted.\r\n");
          } else {
             printf("Could not delete '%s'. Error: %d\n",file,status);
             sendResponse(socketfd, "450 Error deleting file: %s\r\n", strerror(status));
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
             strcpy(temp_msg, cwd);
             strcat(temp_msg, "/");
             strcat(temp_msg,&msgptr[5]);
             len = strpos("\r\n",temp_msg);
             temp_msg[len] = 0;
             file  = temp_msg;
         }

         printf("Store file '%s'\n",file);

         // be sure file is removed first
         remove(file);

         data_fd = create(file);
         if (data_fd < 0) {
             sendResponse(socketfd, "451 Error creating file: %s\r\n", strerror(data_fd));
             data_fd = 0;
             continue;
         }

         if (ftp_mode & FTP_PASV) {
             // signal data thread to start!
             data_command = DATA_CMD_STOR;
             // wait until data thread finished
             sendResponse(socketfd, "150 Ready to receive file.\r\n");
             while (data_command) {
                usleep(100000);
             }
             if (data_result == cOk) {
                 // tell control connection that data has been send
                 sendResponse(socketfd, "226 Transfer complete.\r\n");
             } /*else {
                 sendResponse(socketfd, "552 Error storing file: %s\r\n", strerror(data_result));
             }*/
         } else {
             sendResponse(socketfd, "150 Opening BINARY mode data connection\r\n");

             // create new socket
             datasock = socket(IPV4, SOCK_STREAM, TCP);
             // bind to local port
             bind(datasock, dataaddr);

             if (connect(datasock, &dataremote) != 0) {
                 puts("ReceiveFile(): Error Connecting..\n");
                 sendResponse(socketfd, "425 Cannot open data connection\r\n");
                 continue;
             }

             puts("Data Connection established..\n");

             //puts("Sending Directory Contents..\r\n");
             int status = receiveFile(datasock, data_fd);

             // stream mode must close the socket again..
             close(datasock);

             if (status == 0) {
                // tell control connection that data has been received
                sendResponse(socketfd, "226 Transfer complete.\r\n");
             } /*else {
                sendResponse(socketfd, "552 Error storing file: %s\r\n", strerror(status));
             }*/
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
             strcpy(temp_msg, cwd);
             strcat(temp_msg, "/");
             strcat(temp_msg, &msgptr[5]);
             len = strpos("\r\n",temp_msg);
             temp_msg[len] = 0;
             file  = temp_msg;
         }


         printf("Retrieve for file '%s'\n",file);

         // try opening
         int handle = open(file,0);

         if (handle >= 0) {

             if (ftp_mode & FTP_PASV) {
                 // signal data thread to start!
                 data_fd      = handle;
                 data_command = DATA_CMD_RETR;

                 // wait until data thread finished
                 sendResponse(socketfd, "150 Here comes your file in BINARY mode.\r\n");
                 while (data_command) {
                    usleep(100000);
                 }
                 if (data_result == cOk) {
                     // tell control connection that data has been send
                     sendResponse(socketfd, "226 Transfer complete.\r\n");
                 }
             } else {
                // file opened .. send
                // create new socket
                datasock = socket(IPV4,SOCK_STREAM,TCP);
                // bind to local port
                bind(datasock, dataaddr);

                sendResponse(socketfd, "150 Opening BINARY mode data connection\r\n");

                if (connect(datasock, &dataremote) != 0) {
                    puts("sendFile(): Error Connecting..\n");
                    sendResponse(socketfd, "425 Cannot open data connection\r\n");
                    continue;
                }

                puts("Data Connection established..\n");

                int status = sendFile(datasock, handle);

                // stream mode must close the socket again..
                close(datasock);

                if (status == 0) {
                    // tell control connection that data has been send
                    sendResponse(socketfd, "226 Transfer complete.\r\n");
                }

                close(handle);
             }
         } else {
             sendResponse(socketfd, "450 Could not open '%s'. Error: %d\r\n", file,handle);
         }

     }  else if (strpos("LIST",msgptr) == 0) {
         /***********************************************
          *     SEND DIRECTORY LIST
          ***********************************************/
             if (ftp_mode & FTP_PASV) {
                 // signal data thread to start!
                 data_command = DATA_CMD_LIST;
                 // wait until data thread finished
                 sendResponse(socketfd, "150 Here comes the directory listing.\r\n");
                 while (data_command) {
                     usleep(100000);
                 }
                 if (data_result == cOk) {
                     // tell control connection that data has been send
                     sendResponse(socketfd, "226 Transfer complete.\r\n");
                 } else {
                     sendResponse(socketfd, "426 Error listing: %s\r\n", strerror(data_result));
                 }
             } else {
                if (dataremote.sa_data != 0) {
                    sendResponse(socketfd, "150 Opening ASCII mode data connection\r\n");

                    // create new socket
                    datasock = socket(IPV4,SOCK_STREAM,TCP);
                    // bind to local port
                    bind(datasock, dataaddr);

                    if (connect(datasock, &dataremote) != 0) {
                        puts("Error Connecting..\r\n");
                        sendResponse(socketfd, "425 Cannot open data connection\r\n");
                        continue;
                    }

                    puts("Connected..\r\n");
                    //puts("Sending Directory Contents..\r\n");
                    sendDirectoryContents(datasock);

                    dataremote.sa_data = 0;
                    // tell control connection that data has been send
                    sendResponse(socketfd, "226 Transfer complete.\r\n");

                } else {
                    sendResponse(socketfd, "426 \r\n",6,0);
                }
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
         ftp_mode &= ~FTP_PASV;

         sendResponse(socketfd, "200 PORT command successful.\r\n");
     } // PORT
     else if (strpos("CWD ", msgptr) == 0) {
         /***********************************************
          *     CHANGE CURRENT WORKING DIRECTORY
          ***********************************************/
        char* dir = getFTPPath(&msgptr[4]);

        memcpy(lwd, cwd, 256);

        // open dir
        int newdir_len = strlen(dir);
        if (dir[newdir_len-1] == '/') newdir_len--;

        memcpy(cwd, dir, newdir_len);
        cwd[newdir_len] = '/';
        cwd[newdir_len+1] = '\0';

        compactPath(cwd);

        // try opening the new directory
        int handle = open(cwd, 0);

        if (handle >= 0) {
            // success
            close(handle);
            sendResponse(socketfd, "250 \"%s\" is current directory.\r\n", cwd);
        } else {
            sendResponse(socketfd, "450 Could not open \"%s\". Error: %s\r\n", cwd, strerror(handle));
            memcpy(cwd, lwd, 256);
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
             sendResponse(socketfd, "213 %u\r\n", filetype.st_size);
         } else {
            printf("Could not open '%s'. Error: %s\r\n", cwd, strerror(handle));
            sendResponse(socketfd, "550 Could not open '%s'. Error: %s\r\n", cwd, strerror(handle));
            memcpy(cwd, lwd, 256);
         }
     } else {
         /***********************************************
          *     UNKNOWN COMMAND
          ***********************************************/
         printf("Unknown Command: %s", msgptr);
         sendResponse(socketfd, "500 command not understood.\r\n", msgptr);
     }

    } // while connected

}



void* ftp_entry(void* arg) {
    FTPServer* server = (FTPServer*) arg;
    server->thread_entry();
    delete server;
    return 0;
}

extern "C" int main(int argc, char** argv) {
    int i;
    puts("FTP-Server starting.\n");
    int mysock = socket(IPV4, SOCK_STREAM, TCP);

    // bind our socket to some address
    sockaddr* addr = (sockaddr*) malloc(sizeof(sockaddr));

    addr->port_data     =     21;                         //< the ftp port
    addr->sa_data       =     IP4ADDR(192,168,1,100);

    bind(mysock, addr);

    thread_name(0, "ftpd");
    puts("FTP-Server bound and waiting for clients.\n");

    while(1)
    {
        // wait for new connection
        int newsock = listen(mysock, 5);
        if (newsock < 0) {
            printf("Listen error %d: %s\n", newsock, strerror(newsock));
            continue;
        }

        if (newsock > 0) {
            puts("New FTP connection!\n");

            thread_attr_t attr;
            memset(&attr, 0, sizeof(thread_attr_t));

            attr.stack_size = 4096;
            attr.priority   = 50;

            FTPServer* server = new FTPServer(newsock);
            if (server)
            {
                ThreadIdT threadid;
                if (thread_create(&threadid, &attr, ftp_entry, (void*) server) == 0) {
                    thread_name(threadid,"ftp cmd");
                    thread_run(threadid);
                } else {
                    close(newsock);
                }
            } else
            {
                close(newsock);
            }
        }
    }
}
