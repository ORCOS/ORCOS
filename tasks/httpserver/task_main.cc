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


#define IPV4 0x800
#define TCP 0x6


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




char* ErrorMsg = "HTTP/1.1 500 Server Error\r\nContent-Length: 23\r\n\r\nNot supported by Server";
char* NotFoundMsg = "HTTP/1.1 404 Not Found\r\nContent-Length: 14\r\n\r\nFile not found";


/*
char* fileRetMsg = "HTTP/1.0 200 OK\r\n\
Date: Fri, 13 Jan 2006 15:12:48 GMT\r\n\
Last-Modified: Tue, 10 Jan 2006 11:18:20 GMT\r\n\
Content-Language: de\r\n\
Content-Type: text/html; charset=utf-8\r\n\r\n";*/


char* fileRetMsg = "HTTP/1.1 200 OK\r\nContent-Length: ";



void* thread_entry(void* arg) {
    int connected;
    connected = 1;

    int newsock = (int) arg;

    /* using the stack to receive ... */
    char rcvbuf[1024];
    char return_msg[1024];
    char filename[256];

    while (connected) {
        char* msgptr = rcvbuf;
        int msglen = recv(newsock,msgptr,1024,MSG_WAIT);

        if (msglen == -1) {
            // disconnected
            puts("HTTP Client disconnected..\n");
            close(newsock);
            return 0;
        } else if (msglen < 0) {
            // error close connection
            puts("Connection Error .. closing..\n");
            close(newsock);
            return 0;
        }


        msgptr[msglen-1] = 0;
        //puts(msgptr);


     if (strpos("GET ",msgptr) == 0) {
         /***********************************************
          *     HTTP GET
          ***********************************************/
         msgptr += 5;
         int pos = strpos(" ",msgptr);

         if ( pos  >= 0 && pos < 256) {

             msgptr[pos] = 0;
             if (pos == 0)
                 msgptr = "index.html";

             printf("GET for file: %s\n",msgptr);
             sprintf(filename,"/mnt/ramdisk/%s",msgptr);

             int file = open(filename, false);
             if (file < 0) {
                 // not found
                 puts("File not found.\n");
                 sendto(newsock,NotFoundMsg,strlen(NotFoundMsg),0);

             } else {
                 stat_t stat;
                 fstat(file,&stat);

                 printf("File Size: %d\n",stat.st_size);

                 // send file
                 sprintf(return_msg,"%s%d\r\n\r\n",fileRetMsg,stat.st_size);
                 sendto(newsock,return_msg,strlen(return_msg),0);

                 // no read file and send over data connection
                  int num = read(file, return_msg, 1024);
                  if (num < 0) num = 0; // check error

                  bool abort = false;

                  while (num == 1024 && ! abort) {
                      int timeout = 20;
                      // try sending. if failed sleep
                      // failing may happen if no more free memory is available
                      // inside the TCP/IP stack to hold the packet until acked
                      while (sendto(newsock, return_msg, num, 0) != 0 && timeout) {
                          /* sleep 2 ms*/
                          usleep(2000);
                          timeout--;
                          if (timeout == 0) {
                              abort = true;
                          }
                      }

                      num = read(file, return_msg, 1024);
                  }

                  // send last bytes if any
                  if (num > 0 && !abort) {
                      int timeout = 20;
                      while (sendto(newsock,return_msg,num,0) != 0 && timeout) {
                           /* sleep 2 ms*/
                           usleep(2000);
                           timeout--;
                           if (timeout == 0) {
                               abort = true;
                           }
                       }
                  }

                  if (!abort)
                      puts("File send\n");
                  else
                      puts("File sending aborted\n");

                  close(file);
             }


         }  else {

             puts("Invalid GET pos\n");
             sendto(newsock,NotFoundMsg,strlen(NotFoundMsg),0);
         }

     } else {
         // send error
        puts("Unknown request\n");
        sendto(newsock,ErrorMsg,strlen(ErrorMsg),0);
     }

     //puts("Closing connection\r");
     usleep(1000000);
     close(newsock);
     connected = 0;

    } // while connected
}



extern "C" int task_main()
{
    int mysock = socket(IPV4, SOCK_STREAM, TCP);

    // bind our socket to some address
    sockaddr* addr = (sockaddr*) malloc(sizeof(sockaddr));

    addr->port_data     =     80;                         //< the http port
    addr->sa_data       =     0;
    //memcpy(addr->name_data,"FTPServer\0",13);      //< register using this service name

    int error = bind(mysock,addr);
    if (error < 0) {
        puts("Could not bind to port 80. Exiting.\n");
        return (1);
    }

    // prepare data local sock address structure
    sockaddr* dataaddr  =  (sockaddr*) malloc(sizeof(sockaddr));
    dataaddr->port_data =     0;                           //< let tcp stack choose a port for us
    dataaddr->sa_data   =     0;

    thread_name(0,"httpd");
    puts("HTTP-Server bound and waiting for clients.\n");

    while(1)
    {
        // wait for new connection
        int newsock = listen(mysock);

        if (newsock > 0) {
            puts("New HTTP connection!\n");

            thread_attr_t attr;
            memset(&attr,0,sizeof(thread_attr_t));

            attr.stack_size = 4096;
            attr.priority = 1 + newsock;

            ThreadIdT threadid;
            if (thread_create(&threadid, &attr, thread_entry, (void*) newsock) == 0) {
                thread_name(threadid,"httpd thread");
                thread_run(threadid);
            }
            else {
                close(newsock);
            }
        }

    } // loop forever

}
