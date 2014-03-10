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


#define LINEFEED "\r\n"

const char* welcome_msg = "220 FTP Server ready.\r\n";

// the current command being received
static char command[100];

// current directory we are in
static char current_dir[100];

// last directory
static char last_dir[100];

static char return_msg[520];

static char dir_msg[1024];

static char temp_msg[200];


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


char dataBuffer[2048];

int ReceiveFile(int controlsock, int datasock, sockaddr *dataremote, char* file) {

	if (connect(datasock,dataremote) != 0) {
		puts("Error Connecting..\r\n");
		sprintf(return_msg,"425 Cannot open data connection\r\n");
		sendto(controlsock,return_msg,strlen(return_msg),0);
		return -1;
	}

	puts("Data Connection established..\r\n");


	char* dir = extractFilePath(file);
	printf("Dir: '%s' , File: '%s'\r\n",dir,file);

	if (dir == 0) {
		sprintf(return_msg,"451 Internal File Error\r\n");
		sendto(controlsock,return_msg,strlen(return_msg),0);
		return -1;
	}

	int res = fcreate(file,dir);
	if (res < 0) {
		sprintf(return_msg,"451 Error creating file\r\n");
		sendto(controlsock,return_msg,strlen(return_msg),0);
		return -1;
	}

	// file created .. read data from datasock

	volatile int msglen = 0;
	int timeout = 300;
	char* dataptr = dataBuffer;
	msglen = recv(datasock,dataptr,2048,MSG_PEEK);
	// while not disconnected try reading data
	while (msglen != -1) {

		if (msglen == 0) {
			sleep(10);
			// nothing received
			timeout--;
			if (timeout == 0) {
				// error in connection
				sprintf(return_msg,"426 Timeout receiving data\r\n");
				sendto(controlsock,return_msg,strlen(return_msg),0);
				fclose(res);
				puts("Timeout receiving file..\r\n");
				return -1;
			}
		} else {
			// write to file
		//	long long t1 = getTime();
			fwrite(dataptr,msglen,1,res);
		//	long long t2 = getTime();
		//	printf("Time %d\r",(int) (t1));
		//	printf("%d\r",(int) (t2-t1));
			timeout = 300;
		}


		msglen = recv(datasock,dataptr,2048,MSG_PEEK);
	}

	puts("File completely received..\r\n");

	fclose(res);
	return 0;

}


int sendFile(int controlsock, int datasock, sockaddr *dataremote, int handle) {

	if (connect(datasock,dataremote) != 0) {
		puts("Error Connecting..\r\n");
		sprintf(return_msg,"425 Cannot open data connection\r\n");
		sendto(controlsock,return_msg,strlen(return_msg),0);
		return -1;
	}

	puts("Data Connection established..\r\n");

	// no read file and send over data connection
	int num = fread(return_msg,512,1,handle);
	if (num < 0) num = 0; // check error


	while (num == 512) {
		int timeout = 20;
		// try sending. if failed sleep
		// failing may happen if no more free memory is available
		// inside the TCP/IP stack to hold the packet until acked
		while (sendto(datasock,return_msg,num,0) != 0) {
			sleep(100);
			timeout--;
			if (timeout == 0) {
				sprintf(return_msg,"426 Timeout sending data\r\n");
				sendto(controlsock,return_msg,strlen(return_msg),0);
				return -1;
			}
		}

		num = fread(return_msg,512,1,handle);
	}

	// send last bytes if any
	if (num > 0)
		sendto(datasock,return_msg,num,0);

	return 0;

}


void sendDirectoryContents(int controlsock, int datasock, sockaddr *dataremote) {

	if (connect(datasock,dataremote) != 0) {
		puts("Error Connecting..\r\n");
		sprintf(return_msg,"425 Cannot open data connection\r\n");
		sendto(controlsock,return_msg,strlen(return_msg),0);
		return;
	}

	puts("Data Connection established..\r\n");

	if (mydirhandle == 0) {
		puts("Internal Error: No valid Directory handle..\r\n");
		return;
	}

		// TODO: while loop
		int ret = fread(dir_content,4000,1,mydirhandle);
		if (ret > 0) {

			// interpret data
			if (dir_content[0] == cDirTypeORCOS) {
				// display directory content

				unint2 pos = 1;
				// for each directory send a message
				char* retmsg = dir_msg;
				memset(retmsg,0,10);

				while ((pos < ret) && (pos < 4000)) {
					// parse the directory contents
					unint1 namelen = (unint1) dir_content[pos];
					if (namelen > 100) return;

					int dirtype = dir_content[pos+2+namelen];

					unint4 filesize = (dir_content[pos+4+namelen] << 24) |
									  (dir_content[pos+5+namelen] << 16) |
									  (dir_content[pos+6+namelen] << 8) |
									  (dir_content[pos+7+namelen] << 0);

					if (dirtype == 1) {
						sprintf(line, "%s   1 %-10s %-10s %10u Jan  1  1980 %s\r\n",
									  "d-rw-rw-rw-", "User", "User",filesize, &dir_content[pos+1]);
					} else
						sprintf(line, "%s   1 %-10s %-10s %10u Jan  1  1980 %s\r\n",
									  "-rw-rw-rw-", "User", "User",filesize, &dir_content[pos+1]);

					// concat the line
					retmsg = strcat(retmsg,line);

					if (strlen(dir_msg) > 800) {
						//puts(dir_msg);
						sendto(datasock,dir_msg,strlen(dir_msg),0);
						retmsg = dir_msg;
						memset(retmsg,0,10);
					}
					pos += namelen + 12;
				}

				//puts(dir_msg);
				sendto(datasock,dir_msg,strlen(dir_msg),0);
			}
			return;

		} else {
			sprintf(return_msg,"450 Error reading directory\r\n");
			sendto(controlsock,return_msg,strlen(return_msg),0);
		}


}



int htonl(int n)
{
	return ((n & 0xff) << 24) |
	((n & 0xff00) << 8) |
	((n & 0xff0000UL) >> 8) |
	((n & 0xff000000UL) >> 24);
}

char recvCommand[100];

extern "C" int task_main()
{
	int i = 0;


	int mysock = socket(IPV4,SOCK_STREAM,TCP);

	// bind our socket to some address
	sockaddr* addr = (sockaddr*) malloc(sizeof(sockaddr));

	addr->port_data 	= 	21; 			       	 //< the ftp port
	addr->sa_data 		= 	IP4ADDR(192,168,1,100);
	//memcpy(addr->name_data,"FTPServer\0",13);      //< register using this service name

	bind(mysock,addr);

	// prepare data local sock address structure
	sockaddr* dataaddr  =  (sockaddr*) malloc(sizeof(sockaddr));
	dataaddr->port_data = 	0; 			       	   //< let tcp stack choose a port for us
	dataaddr->sa_data   = 	IP4ADDR(192,168,1,100);

	// our handle to the data socket which is created on demand
	int datasock;
	// the current msg received
	char* msgptr;

 	puts("FTP-Server bound and waiting for clients.\r");

	while(1)
	{
		// wait for new connection
		int newsock = listen(mysock);


		puts("New FTP connection!\r");
		sendto(newsock,welcome_msg,strlen(welcome_msg),0);

		// reset data sockaddr
		dataremote.sa_data = 0;

		// reset directory to root
		current_dir[0] = '/';
		current_dir[1] = '\0';

		if (mydirhandle != 0) fclose(mydirhandle);
		mydirhandle = fopen(current_dir,0);

		while (1) {
			msgptr = recvCommand;
			int msglen = recv(newsock,msgptr,100,MSG_WAIT);

			if (msglen == -1) {
				// disconnected
				if (mydirhandle != 0) fclose(mydirhandle);
				mydirhandle = 0;
				puts("FTP Client disconnected..");
				break;
			}

		 // be sure the string is null terminated
		 msgptr[msglen] = 0;
		 //int line = strpos(msgptr,"\n\r");
		// puts(msgptr);

		 if (strpos("USER",msgptr) == 0) {
			 /***********************************************
			  * 	USER LOGIN
			  ***********************************************/

			 //no login required
			 sendto(newsock,"200 \r\n",6,0);

		 } else if (strpos("PWD",msgptr) == 0) {
			 /***********************************************
			  * 	GET CURRENT WORKING DIRECTORY
			  ***********************************************/

			 return_msg[0] = '2';
			 return_msg[1] = '0';
			 return_msg[2] = '0';
			 return_msg[3] = ' ';

			 strcpy(&return_msg[4],current_dir);
			 strcat(return_msg,"\r\n");
			 sendto(newsock,return_msg,strlen(return_msg),0);

		 } else if (strpos("TYPE A",msgptr) == 0) {
			 /***********************************************
			  * 	PREPARE FOR ASCII TRANSFER
			  ***********************************************/

			 // going to transfer directory content a.s.o
			 sendto(newsock,"200 \r\n",6,0);

		 } else if (strpos("TYPE I",msgptr) == 0) {
			 /***********************************************
			  * 	PREPARE FOR BINARY TRANSFER
			  ***********************************************/

			 // we are going to transfer binary data
			 sendto(newsock,"200 \r\n",6,0);

		 }  else if (strpos("noop",msgptr) == 0) {
			 /***********************************************
			  * 	NOOP
			  ***********************************************/

			 sendto(newsock,"200 \r\n",6,0);

		 } else if (strpos("PASV",msgptr) == 0) {
			 /***********************************************
			  * 	REQUEST PASSIVE MODE
			  ***********************************************/

			 sendto(newsock,"500 \r\n",6,0);

		 } else if (strpos("DELE ",msgptr) == 0) {
			 /***********************************************
			  * 	DELETE FILE
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

			  printf("Delete file '%s'\r\n",file);

			  char* dir = extractFilePath(file);


			  int status = fremove(file,dir);

			  if (status == 0) {
				// tell control connection that data has been received
				char* s = "220 File deleted.\r\n";
				sendto(newsock,s,strlen(s),0);
			  } else {
				 printf("Could not delete '%s' in '%s'. Error: %d\r\n",file,dir,status);
				 char* s = "450 Error deleting file.\r\n";
				 sendto(newsock,s,strlen(s),0);
			  }

		 }  else if (strpos("STOR ",msgptr) == 0) {
			 /***********************************************
			  * 	STORE FILE
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

			 char* s = "150 Opening BINARY mode data connection\r\n";
			 sendto(newsock,s,strlen(s),0);

			 // create new socket
			 datasock = socket(IPV4,SOCK_STREAM,TCP);
			 // bind to local port
			 bind(datasock,dataaddr);

			 //puts("Sending Directory Contents..\r\n");
			 int status = ReceiveFile(newsock,datasock,&dataremote,file);

			 // stream mode must close the socket again..
			 fclose(datasock);

			 if (status == 0) {
				// tell control connection that data has been received
				s = "226 Transfer complete.\r\n";
				sendto(newsock,s,strlen(s),0);
			 }

		 } else if (strpos("RETR ",msgptr) == 0) {
			 /***********************************************
			  * 	RETRIEVE FILE
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


			 printf("Retrieve for file '%s'\r\n",file);

			 // try opening
			 int handle = fopen(file,0);

			 if (handle >= 0) {
				 // file opened .. send

				char* s = "150 Opening BINARY mode data connection\r\n";
				sendto(newsock,s,strlen(s),0);

				// create new socket
				datasock = socket(IPV4,SOCK_STREAM,TCP);
				// bind to local port
				bind(datasock,dataaddr);

				//puts("Sending Directory Contents..\r\n");
				int status = sendFile(newsock,datasock,&dataremote,handle);

				// stream mode must close the socket again..
				fclose(datasock);

				if (status == 0) {
					// tell control connection that data has been send
					s = "226 Transfer complete.\r\n";
					sendto(newsock,s,strlen(s),0);
				}

				fclose(handle);
			 } else {
				printf("Could not open '%s'. Error: %d\r\n",file,handle);
				sendto(newsock,"450 \r\n",6,0);
			 }

		 }  else if (strpos("LIST",msgptr) == 0) {
			 /***********************************************
			  * 	SEND DIRECTORY LIST
			  ***********************************************/

				if (dataremote.sa_data != 0) {
					char* s = "150 Opening ASCII mode data connection\r\n";
					sendto(newsock,s,strlen(s),0);

					// create new socket
					datasock = socket(IPV4,SOCK_STREAM,TCP);
					// bind to local port
					bind(datasock,dataaddr);

					//puts("Sending Directory Contents..\r\n");
					sendDirectoryContents(newsock,datasock,&dataremote);

					// stream mode must close the socket again..
					fclose(datasock);

					// tell control connection that data has been send
					s = "226 Transfer complete.\r\n";
					sendto(newsock,s,strlen(s),0);
				} else
					sendto(newsock,"420 \r\n",6,0);

		 } else if (strpos("PORT ",msgptr) == 0) {

			 /***********************************************
			  * 	SET REMOTE PORT
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

			 sendto(newsock,"200 \r\n",6,0);

		 } // PORT
		 else if (strpos("CWD ",msgptr) == 0) {
			 /***********************************************
			  * 	CHANGE CURRENT WORKING DIRECTORY
			  ***********************************************/

			int len = strpos("\r\n",msgptr);
			memcpy(temp_msg,msgptr,len);
			temp_msg[len] = 0;

			char* dir = &temp_msg[4];

			 if (dir[0] != '/') {
				 // relative path
				 strcpy(temp_msg,current_dir);
				 strcat(temp_msg,&msgptr[4]);
				 len = strpos("\r\n",temp_msg);
				 temp_msg[len] = 0;
				 dir  = temp_msg;
			 }

			memcpy(last_dir,current_dir,100);

			// open dir
			int newdir_len = strlen(dir);
			if (dir[newdir_len-1] == '/') newdir_len--;

			memcpy(current_dir,dir,newdir_len);
			current_dir[newdir_len] = '/';
			current_dir[newdir_len+1] = '\0';

			compactPath(current_dir);

			// try opening the new directory
			int handle = fopen(dir,0);

			if (handle >= 0) {
				// success
				//printf("opening '%s' successfull\r\n",current_dir);
				if ((mydirhandle != 0) && (mydirhandle != handle )) fclose(mydirhandle);
				mydirhandle = handle;
				sendto(newsock,"250 \r\n",6,0);
			} else {
				//printf("Could not open '%s'. Error: %d\r\n",current_dir,handle);
				memcpy(current_dir,last_dir,100);
				sendto(newsock,"450 \r\n",6,0);
			}

		 } // CWD
		 else {
			 /***********************************************
			  * 	UNKNOWN COMMAND
			  ***********************************************/

			 sendto(newsock,"500 \r\n",6,0);
		 }



		} // while connected

	} // loop forever

}
