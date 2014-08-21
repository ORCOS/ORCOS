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


char* ErrorMsg = "HTTP/1.1 500 Server Error\r\nContent-Length: 23\r\n\r\nNot supported by Server";
char* NotFoundMsg = "HTTP/1.1 404 Not Found\r\nContent-Length: 14\r\n\r\nFile not found";


/*
char* fileRetMsg = "HTTP/1.0 200 OK\r\n\
Date: Fri, 13 Jan 2006 15:12:48 GMT\r\n\
Last-Modified: Tue, 10 Jan 2006 11:18:20 GMT\r\n\
Content-Language: de\r\n\
Content-Type: text/html; charset=utf-8\r\n\r\n";*/


char* fileRetMsg = "HTTP/1.1 200 OK\r\n\
Content-Length: ";



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
            puts("HTTP Client disconnected..\r");
            fclose(newsock);
            return 0;
        } else if (msglen < 0) {
            // error close connection
            puts("Connection Error .. closing..\r");
            fclose(newsock);
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

             printf("GET for file: %s\r",msgptr);
             sprintf(filename,"/mnt/ramdisk/%s",msgptr);

             int file = fopen(filename,false);
             if (file < 0) {
                 // not found
                 puts("File not found.\r");
                 sendto(newsock,NotFoundMsg,strlen(NotFoundMsg),0);

             } else {
                 stat_t stat;
                 fstat(file,&stat);

                 printf("File Size: %d\r",stat.st_size);

                 // send file
                 sprintf(return_msg,"%s%d\r\n\r\n",fileRetMsg,stat.st_size);
                 sendto(newsock,return_msg,strlen(return_msg),0);

                 // no read file and send over data connection
                  int num = fread(return_msg,1024,1,file);
                  if (num < 0) num = 0; // check error

                  bool abort = false;

                  while (num == 1024 && ! abort) {
                      int timeout = 20;
                      // try sending. if failed sleep
                      // failing may happen if no more free memory is available
                      // inside the TCP/IP stack to hold the packet until acked
                      while (sendto(newsock,return_msg,num,0) != 0 && timeout) {
                          sleep(20);
                          timeout--;
                          if (timeout == 0) {
                              abort = true;
                          }
                      }

                      num = fread(return_msg,1024,1,file);
                  }

                  // send last bytes if any
                  if (num > 0 && !abort)
                      sendto(newsock,return_msg,num,0);

                  if (!abort)
                      puts("File send\r");
                  else
                      puts("File sending aborted\r");

                  fclose(file);
             }


         }  else {

             puts("Invalid GET pos\r");
             sendto(newsock,NotFoundMsg,strlen(NotFoundMsg),0);
         }

     } else {
         // send error
        puts("Unknown request\r");
        sendto(newsock,ErrorMsg,strlen(ErrorMsg),0);
     }

     //puts("Closing connection\r");
    // fclose(newsock);
     connected = 1;

    } // while connected
}



extern "C" int task_main()
{
	int i = 0;


	int mysock = socket(IPV4,SOCK_STREAM,TCP);

	// bind our socket to some address
	sockaddr* addr = (sockaddr*) malloc(sizeof(sockaddr));

	addr->port_data 	= 	80; 			       	 //< the http port
	addr->sa_data 		= 	IP4ADDR(192,168,1,100);
	//memcpy(addr->name_data,"FTPServer\0",13);      //< register using this service name

	int error = bind(mysock,addr);
	if (error < 0) {
	    puts("Could not bind to port 80. Exiting.\r");
	    return (1);
	}

	// prepare data local sock address structure
	sockaddr* dataaddr  =  (sockaddr*) malloc(sizeof(sockaddr));
	dataaddr->port_data = 	0; 			       	   //< let tcp stack choose a port for us
	dataaddr->sa_data   = 	IP4ADDR(192,168,1,100);

	// our handle to the data socket which is created on demand
	int datasock;
	// the current msg received
	char* msgptr;
 	puts("HTTP-Server bound and waiting for clients.\r");

	while(1)
	{
		// wait for new connection
		int newsock = listen(mysock);

		if (newsock > 0) {
            puts("New HTTP connection!\r");

            thread_attr_t attr;
            memset(&attr,0,sizeof(thread_attr_t));

            attr.stack_size = 4096;
            attr.priority = 1 + newsock;

            int threadid;
            if (thread_create(&threadid,&attr,thread_entry,(void*) newsock) == 0)
                thread_run(threadid);
            else
                fclose(newsock);
		}

	} // loop forever

}
