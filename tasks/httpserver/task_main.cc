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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define IPV4 0x800
#define TCP  0x6

#define PRINTDEBUG 0

#if PRINTDEBUG
#define DEBUG(a, ...) printf(a, __VA_ARGS__)
#else
#define DEBUG(a, ...)
#endif

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

    int timeout = 0;

    while (connected) {
        char* msgptr = rcvbuf;
        int msglen = recv(newsock, msgptr, 1023, MSG_WAIT, 200);

        if (msglen == -1) {
            // disconnected
            DEBUG("HTTP Client disconnected..\n");
            close(newsock);
            return (0);
        }

        if (msglen <= 0) {
            // timeout
            timeout++;
            if (timeout > 50) {
                connected = 0;
            }
            continue;
        }

        timeout = 0;
        msgptr[msglen] = 0;

        if (strpos("GET ", msgptr) == 0) {
            /***********************************************
             *     HTTP GET
             ***********************************************/
            msgptr += 5;
            int pos = strpos(" ", msgptr);

            if (pos >= 0 && pos < 256) {
                msgptr[pos] = 0;
                if (pos == 0) {
                    msgptr = "index.html";
                }

                DEBUG("GET for file: %s\n",msgptr);
                sprintf(filename, "%s", msgptr);

                int file = open(filename, false);
                if (file < 0) {
                    // not found
                    printf("[HTTPD] Error. File %s not found.\n", filename);
                    sendto(newsock, NotFoundMsg, strlen(NotFoundMsg), 0);
                } else {
                    struct stat stat;
                    fstat(file, &stat);

                    DEBUG("[HTTPD] File Size: %u\n",stat.st_size);

                    // send file
                    sprintf(return_msg, "%s%u\r\n\r\n", fileRetMsg, stat.st_size);
                    sendto(newsock, return_msg, strlen(return_msg), 0);
                    bool abort = false;

                    // now read file and send over data connection
                    int num = read(file, return_msg, 1024);

                    while (num > 0) {
                        int err = send(newsock, return_msg, num);
                        if (err < 0) {
                            err = send(newsock, return_msg, num);
                            if (err < 0) {
                                printf("[HTTPD] Error sending file %s. %s\n", filename,  strerror(err));
                                break;
                            }
                        }

                        num = read(file, return_msg, 1024);
                    }

                    close(file);
                }
            } else {
                DEBUG("Invalid GET pos\n");
                sendto(newsock, NotFoundMsg, strlen(NotFoundMsg), 0);
            }

        } else {
            // send error
            printf("[HTTPD] Unknown request\n");
            sendto(newsock, ErrorMsg, strlen(ErrorMsg), 0);
        }

    }  // while connected

    close(newsock);
}

extern "C" int main(int argc, char** argv) {
    int mysock = socket(IPV4, SOCK_STREAM, TCP);

    // bind our socket to some address
    sockaddr* addr = (sockaddr*) malloc(sizeof(sockaddr));

    addr->port_data = 80;                         //< the http port
    addr->sa_data = 0;

    int error = bind(mysock, addr);
    if (error < 0) {
        puts("Could not bind to port 80. Exiting.\n");
        return (1);
    }

    // prepare data local sock address structure
    sockaddr* dataaddr = (sockaddr*) malloc(sizeof(sockaddr));
    dataaddr->port_data = 0;                           //< let tcp stack choose a port for us
    dataaddr->sa_data = 0;

    thread_name(0, "httpd");
    puts("HTTP-Server bound and waiting for clients.\n");

    while (1) {
        // wait for new connection
        int newsock = listen(mysock, 10);

        if (newsock > 0) {
            DEBUG("New HTTP connection!\n");

            thread_attr_t attr;
            memset(&attr, 0, sizeof(thread_attr_t));

            attr.stack_size = 4096;
            attr.priority   = 50;

            ThreadIdT threadid;
            if (thread_create(&threadid, &attr, thread_entry, (void*) newsock) == 0) {
                thread_name(threadid, "httpd thread");
                thread_run(threadid);
            } else {
                close(newsock);
            }
        }
    }  // loop forever

}
