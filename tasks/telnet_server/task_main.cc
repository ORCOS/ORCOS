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


#define LINEFEED "\r"
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

// line feed
static char return_sequence[4] = {13,0,13,0};

// current directory we are in
static char current_dir[100];

// last directory
static char last_dir[100];

static const char* unknown_command = "Unknown Command\r";

static const char* dir_error = "Could not open Directory..\r";

static const char* help_msg = "OCROS Telnet Terminal\r\rSupported commands:\r"
						"help      - Shows this message\r"
						"cd        - Change Directory\r"
						"ls        - List Directory\r"
						"cat       - Display Contents of file (in ASCII)\r"
						"hexdump   - Dumps a file as hex and ASCII\r"
						"touch     - Creates a new file\r"
						"run       - Starts a task from file\r"
						"kill      - Kills a task by ID\r";

static char return_msg[520];

static char temp_msg[200];

static int cmd_pos = 0;

const char* types[11] = {
		"Directory",
		"StreamDevice",
		"CommDevice",
		"GenericDevice",
		"File",
		"Socket",
		"USBDriver",
		"BlockDevice",
		"Partition",
		"SharedMem"
		"AnyNoDir",
		"Any"
};




static int mydirhandle = 0;


static char dir_content[4096];


const char* getTypeStr(int resourceType) {
    char* ret = "";
    for (int i = 0; i < 11; i++) {
        if (resourceType & (1 << i))
            return (types[i]);
    }
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

void sendUnknownCommand(int socket) {
	memcpy(&return_msg[0],unknown_command,strlen(unknown_command));
	sendto(socket,&return_msg,strlen(unknown_command),0);
}


static int doecho;

char telnetcommand[10];

#define SB 		250 // subnegotation begin
#define SE 		240 // subnegotation end
#define WILL	251
#define WONT	252
#define DO		253
#define DONT	254

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
		telnetcommand[0] = 0xff;
		telnetcommand[1] = DONT;
		telnetcommand[2] = bytes[1];
		sendto(socket,telnetcommand,3,0);
		return 1;
	}
	case DO : {
		telnetcommand[0] = 0xff;

		if (bytes[1] == 1) // ECHO
			telnetcommand[1] = WILL;
		else
			telnetcommand[1] = WONT;

		telnetcommand[2] = bytes[1];
		sendto(socket,telnetcommand,3,0);
		return 1;
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


void handleCommand(int socket, int command_length) {

	// do a command str compare
	if (strcmp("help",command) == 0) {

		memcpy(&return_msg[0],help_msg,strlen(help_msg)+1);
		sendto(socket,&return_msg,strlen(help_msg)+1,0);

		return;
	}
	if (strpos("cd",command) == 0) {
		// try to open directory
		if (command[2] != ' ') return (sendUnknownCommand(socket));

		memcpy(last_dir,current_dir,100);

		char* path = &command[3];
		if (path[0] == '/') {
			current_dir[0] = '\0';
		}

		// open dir
		int curdir_len = strlen(current_dir);
		int newdir_len = strlen(path);
		if (path[newdir_len-1] == '/') newdir_len--;

		memcpy(&current_dir[curdir_len],path,newdir_len);
		current_dir[curdir_len+newdir_len] = '/';
		current_dir[curdir_len+newdir_len+1] = '\0';

		compactPath(current_dir);

		int handle = fopen(current_dir,0);
		if (handle >= 0) {
			// success
			if ((mydirhandle != 0) && (mydirhandle != handle )) fclose(mydirhandle);
			mydirhandle = handle;
		} else {
			memcpy(current_dir,last_dir,100);
			memcpy(&return_msg[0],dir_error,strlen(dir_error)+1);
			sendto(socket,&return_msg,strlen(dir_error)+1,0);
		}


		return;
	}

	if (strpos("ls",command) == 0) {
		// read directory
		if (mydirhandle == 0) return;

		int humanReadable = 0;
		if (strpos("-h",command) > 0)
		    humanReadable = 1;

		Directory_Entry_t* direntry = readdir(mydirhandle);
		while (direntry) {

		    /* shorten name */
		    if (direntry->namelen > 30) {
		        direntry->name[28] = '~';
		        direntry->name[29] = 0;
		    }

		    const char* typestr = getTypeStr(direntry->resType);
		    char* datestr = "Jan 1 1980";

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

		    sprintf(dir_content, "%s%-30s   %5d  %-13s %-8x %10s %s%s\r"
		                         ,prefix, direntry->name, direntry->resId, typestr ,direntry->flags , fileSizeStr, datestr, suffix);

		    int timeout = 100;
		    while (sendto(socket,dir_content,strlen(dir_content)+1,0) < 0 && timeout > 0) {
		        usleep(1000);
		        timeout--;
		    }
		    if (timeout == 0) return;
		    /* read/send next entry  */
		    direntry = readdir(mydirhandle);
		}

		return;
	}

	if (strpos("run",command) == 0) {
		int arg = strpos(" ",command);
		if (arg > 0) {
			char* filename = &command[arg+1];
			char* arguments = extractPath(filename);

			if (filename[0] != '/') {
				// prepend current directory
				strcpy(temp_msg,current_dir);
				strcat(temp_msg,filename);
				filename = temp_msg;
			}

			int error = task_run(filename,arguments);
			if (error > 0) {
			    // no argument given
			    sprintf(return_msg,"%d\r",error);
			    /* set stdout of new task to us! */
			    taskioctl(0,error,"/dev/tty0");
			} else {
                sprintf(return_msg,"Error: %d\r",error);
			}

			sendto(socket,return_msg,strlen(return_msg)+1,0);


			return;
		}

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
			    sprintf(return_msg,"Killing Task Error: %d\r",error);
			    sendto(socket,return_msg,strlen(return_msg)+1,0);
			} else {
			    sprintf(return_msg,"I dont want to kill myself..\r");
			    sendto(socket,return_msg,strlen(return_msg)+1,0);
			}

			return;
		}

	}

	if (strpos("hexdump",command) == 0) {
		// try reading the file and display as hex
		int arg = strpos(" ",command);
		if (arg > 0) {
			char* filename = &command[arg+1];

			char path[100];
			strcpy(path,current_dir);
			strcat(&path[strlen(current_dir)],filename);


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
						sprintf(&return_msg[pos],"\t%s\r",linechars);
						pos +=10;
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
				sprintf(&return_msg[pos],"\t%s\r",linechars);
				pos +=linebytes+2;

				return_msg[pos] = '\r';
				return_msg[pos+1] = '\0';

				//printf(return_msg);
				sendto(socket,return_msg,pos+2,0);

				fclose(filehandle);
				return;
			} else {
				// can not open file
				sprintf(return_msg,"Opening file failed. Error %d",filehandle);
				int end = strlen(return_msg);
				return_msg[end] = '\r';
				return_msg[end+1] = '\0';
				sendto(socket,return_msg,end+2,0);
				return;
			}

		} else {
			// no argument given
			return sendMsg(socket,"No argument given..");
		}

	}

	if (strpos("cat",command) == 0) {
		// try reading the file and display as ascii
		int arg = strpos(" ",command);
		if (arg > 0) {
			char* filename = &command[arg+1];

			char path[100];
			strcpy(path,current_dir);
			strcat(&path[strlen(current_dir)],filename);


			int filehandle = fopen(path,0);
			if (filehandle > 0) {

				int num = fread(return_msg,512,1,filehandle);
				if (num < 0) num = 0; // check error

				while (num == 512) {
					// be sure the msg only contains telnet ascii chars
					makeTelnetCharCompatible(return_msg,num);

					//printf(return_msg);
					sendto(socket,return_msg,num,0);

					num = fread(return_msg,512,1,filehandle);
				}

				// last packet
				makeTelnetCharCompatible(return_msg,num);
				return_msg[num] = '\r';
				return_msg[num+1] = '\0';

				//printf(return_msg);
				sendto(socket,return_msg,num+2,0);

				fclose(filehandle);
				return;
			} else {
				// can not open file
				sprintf(return_msg,"Opening file failed. Error %d",filehandle);
				int end = strlen(return_msg);
				return_msg[end] = '\r';
				return_msg[end+1] = '\0';
				sendto(socket,return_msg,end+2,0);
				return;
			}

		} else {
			// no argument given
			sendMsg(socket,"Error: No argument given..");
			return;
		}
	}


	if (strpos("touch",command) == 0) {
		int arg = strpos(" ",command);
		if (arg > 0) {
			char* filename = &command[arg+1];

			int res = fcreate(filename,current_dir);
			if (res < 0)
				sendMsg(socket,"Error creating file..");

			return;
		} else {
			// no argument given
			sendMsg(socket,"Error: No filename given..");
			return;
		}
	}


	// try running the application with the name


	sendUnknownCommand(socket);

}


static char tty0buf[256];
static int newsock;

void* tty0_thread(void* arg) {
    int devid = (int) arg;
    while (1) {
        int read = fread(tty0buf, 256, 1, devid);
        if (read > 0 && newsock != 0) {
            sendto(newsock,tty0buf,read,0);
        }
        sleep(10);
    }
}

char recvMsg[100];

extern "C" int task_main()
{
	int i = 0;
	newsock = 0;
	int mysock = socket(IPV4,SOCK_STREAM,TCP);

	// bind our socket to some address
	sockaddr* addr = (sockaddr*) malloc(sizeof(sockaddr));

	addr->port_data = 	23; 			       	   //< the port
	addr->sa_data   =   IP4ADDR(192,168,1,100);
	memcpy(addr->name_data,"TelnetServer\0",13);   //< register using this service name

	bind(mysock,addr);

	puts("Telnet-Server bound and waiting for clients.\r");

	/* create the virtual tty device */
	int devid = mkdev("tty0",2048);
	if (devid < 0) {
	    printf("Error creating tty0: %d",devid);
	    return 1;
	}

	/* create the tty0 thread  */
	thread_attr_t attr;
	memset(&attr,0,sizeof(thread_attr_t));
    attr.priority = 1;
	thread_create(0,&attr,tty0_thread,(void*) devid);
	thread_run(0);

	while(1)
	{
		newsock = listen(mysock);
		char* msgptr;
		cmd_pos = 0;
		doecho = 1;
		mydirhandle = 0;

		// send my telnet options
		telnetcommand[0] = 0xff;
		telnetcommand[1] = WILL;
		telnetcommand[2] = 1; // send will echo
		sendto(newsock,telnetcommand,3,0);

		history_count = 0;
		ci = 0;

		puts("New connection!\r");
		sendto(newsock,welcome_msg,strlen(welcome_msg),0);

		current_dir[0] = '/';
		current_dir[1] = '\0';
		mydirhandle = fopen(current_dir,0);

		while (1) {
			msgptr = recvMsg;
			int msglen = recv(newsock,msgptr,100,MSG_WAIT);
			if (msglen == -1) {
				// disconnected
				printf("Terminal disconnected..");
				fclose(newsock);
				newsock = 0;
				break;
			}

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

											if (doecho) {
												for (int j2 = 0; j2 < num_back; j2++)
													memcpy(&return_msg[j2*3],&back_sequence,3);

												sendto(newsock,return_msg,num_back*3,0);
											}
											if (doecho) sendto(newsock,&command,strlen(command),0);
											break;
										}
										case 0x42: {// unten
											if (history_count == 0) break;

											ci = (ci+1) & (MAX_COMMAND_HISTORY -1);
											if (ci >= history_count) ci = 0;

											int num_back = cmd_pos;

											strcpy(command,command_history[ci]);
											cmd_pos=strlen(command);

											if (doecho) {
												for (int j2 = 0; j2 < num_back; j2++)
													memcpy(&return_msg[j2*3],&back_sequence,3);

												sendto(newsock,return_msg,num_back*3,0);
											}
											if (doecho) sendto(newsock,&command,strlen(command),0);
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
								if (msgptr[i] == 0) {
									command[cmd_pos] = '\0';

									if (doecho) sendto(newsock,&return_sequence,2,0);

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

									sendto(newsock,&return_msg,dir_len+2,0);
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
