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

#define MAX_COMMAND_HISTORY 10

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
						"cp        - Copies a file\r"
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
	// fill history
	if (history_count < MAX_COMMAND_HISTORY) {
		history_count++;
	}

	memcpy(&command_history[ci][0],&command[0],command_length);
	ci = (ci +1) % MAX_COMMAND_HISTORY;

	// do a command str compare
	if (strcmp("help",command) == 0) {

		memcpy(&return_msg[0],help_msg,strlen(help_msg)+1);
		sendto(socket,&return_msg,strlen(help_msg)+1,0);

		return;
	}
	if (strpos("cd",command) == 0) {
		// try to open directory
		if (command[2] != ' ') return sendUnknownCommand(socket);

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
		//printf("handle: %d\r\n", handle);
		if (handle >= 0) {
			// success
			//printf("opening %s successfull\r\n",current_dir);

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

		//printf("reading dir handle %d..\r",mydirhandle);
		int ret = fread(dir_content,4000,1,mydirhandle);
		if (ret > 0) {
			//printf("data len: %d\r",ret);
			// interpret data
			if (dir_content[0] == cDirTypeORCOS) {
				// display directory content

				unint2 pos = 1;
				// for each directory send a message
				char* retmsg = return_msg;
				retmsg[0] = '\r';
				retmsg[1] = 0;
				retmsg += 2;

				unint2 msglen = 2;

				while ((pos < ret) && (pos < 4000)) {
					unint1 namelen = (unint1) dir_content[pos];
					if (namelen > 100) return;
					const char* typestr = "Unknown";

					int dirtype = dir_content[pos+2+namelen];

					for (int i = 0; i < 11; i++) {
						if (dirtype & ( 1 << i)) {
							typestr = types[i];
							break;
						}
					}
					unint4 filesize = (dir_content[pos+4+namelen] << 24) |
									  (dir_content[pos+5+namelen] << 16) |
									  (dir_content[pos+6+namelen] << 8) |
									  (dir_content[pos+7+namelen] << 0);

					unint4 flags 	= (dir_content[pos+8+namelen] << 24) |
									  (dir_content[pos+9+namelen] << 16) |
									  (dir_content[pos+10+namelen] << 8) |
									  (dir_content[pos+11+namelen] << 0);


					if (dirtype == 1) {
						sprintf(retmsg,ESC_CYAN);
						msglen += sizeof(ESC_CYAN)-1;
						retmsg += sizeof(ESC_CYAN)-1;
					}

					makeTelnetCharCompatible(&dir_content[pos+1],namelen);
					sprintf(retmsg,"%-30s %4d %3d %-15s %-8d %-8x\r",&dir_content[pos+1],(unint4)dir_content[pos+3+namelen],(unint4)dir_content[pos+2+namelen],typestr,filesize,flags);
					msglen += 39 + 25 + 10;
					retmsg += 39 + 25 + 10;

					if (dirtype == 1) {
						sprintf(retmsg,ESC_WHITE);
						msglen += sizeof(ESC_WHITE)-1;
						retmsg += sizeof(ESC_WHITE)-1;
					}

					//printf(return_msg,strlen(return_msg));
					//return_msg[strlen(return_msg)+1] = '\0';

					if (msglen > 250) {
						return_msg[msglen +1] = '\0';
						sendto(socket,return_msg,msglen+1,0);
						msglen = 0;
						retmsg = return_msg;
					}
					pos += namelen + 12;
				}

				return_msg[msglen ] = '\r';
				return_msg[msglen +1] = '\0';
				sendto(socket,return_msg,msglen+2,0);

			}

			if (ret >= 4000 ) {
				sprintf(return_msg,"\rMore files..\r");
				sendto(socket,return_msg,strlen(return_msg)+1,0);
			}
			return;

		} else {

			sprintf(return_msg,"Can not open directory. Error %d\r",ret);
			sendto(socket,return_msg,strlen(return_msg)+1,0);
		}


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
			// no argument given
			sprintf(return_msg,"Status: %d\r",error);
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
			int error = task_kill(id);
			// no argument given
			sprintf(return_msg,"Killing Task Error: %d\r",error);
			sendto(socket,return_msg,strlen(return_msg)+1,0);
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

	if (strpos("cp",command) == 0) {
		int arg = strpos(" ",command);
		if (arg > 0) {
			char* filename = &command[arg+1];
			char* filename2 = extractPath(filename);
			if (filename2 == 0) {
				sendMsg(socket,"No destination given. Usage: cp source dest");
				return;
			}
			extractPath(filename2);
			int filehandle1;
			int filehandle2;

			char path1[100];

			if (filename[0] != '/') {
				strcpy(path1,current_dir);
				strcat(&path1[strlen(current_dir)],filename);
			} else {
				strcpy(path1,filename);
			}

			char path2[100];

			if (filename2[0] != '/') {
				strcpy(path2,current_dir);
				strcat(&path2[strlen(current_dir)],filename2);
			} else {
				strcpy(path2,filename2);
			}

			sprintf(return_msg,"copy '%s' -> '%s'",path1,path2);
			int end = strlen(return_msg);
			return_msg[end] = '\r';
			return_msg[end+1] = '\0';
			sendto(socket,return_msg,end+2,0);
			return;

			/*int filehandle = fopen(path,0);
			if (filehandle > 0) {
				int res;
				for (int i = 0; i < 200; i++) {
					char str[20];
					sprintf(str,"%3d: test\n",i);
					res = fwrite(str,10,1,filehandle);
				}

				sprintf(return_msg,"Result: %d",res);
				int end = strlen(return_msg);
				return_msg[end] = '\r';
				return_msg[end+1] = '\0';
				sendto(socket,return_msg,end+2,0);
				return;

			} else {
				// can not open file
				sprintf(return_msg,"Opening file failed. Error %d",filehandle);
				int end = strlen(return_msg);
				return_msg[end] = '\r';
				return_msg[end+1] = '\0';
				sendto(socket,return_msg,end+2,0);
				return;
			}*/

		} else {
			sendMsg(socket,"Usage: cp source dest");
			return;
		}

	}

	sendUnknownCommand(socket);


}


extern "C" int task_main()
{
	int i = 0;

	char* mysocketbuffer = (char*) malloc(800);
	// then create a socket

	int mysock = socket(IPV4,SOCK_STREAM,TCP,mysocketbuffer,800);

	// bind our socket to some address
	sockaddr* addr = (sockaddr*) malloc(sizeof(sockaddr));

	addr->port_data = 	23; 			       	   //< the port
	addr->sa_data = 	IP4ADDR(192,168,1,100);
	memcpy(addr->name_data,"TelnetServer\0",13);   //< register using this service name

	bind(mysock,addr);

	printf("Telnet-Server bound and waiting for clients.\r");

	while(1)
	{
		int newsock = listen(mysock);
		char* msgptr;
		cmd_pos = 0;
		doecho = 1;
		mydirhandle = 0;

		// send my telnet options
		telnetcommand[0] = 0xff;
		telnetcommand[1] = WILL;
		telnetcommand[2] = 1; // send will echo
		sendto(newsock,telnetcommand,3,0);

		printf("New connection!\r");
		sendto(newsock,welcome_msg,strlen(welcome_msg),0);

		current_dir[0] = '/';
		current_dir[1] = '\0';
		mydirhandle = fopen(current_dir,0);
		//printf("handle: %d\r\n", mydirhandle);

		while (1) {
			int msglen = recv(newsock,&msgptr,MSG_WAIT);
			if (msglen == -1) {
				// disconnected
				printf("Terminal disconnected..");
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
										case 0x41: break; // oben
										case 0x42: break; // unten
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
									//printf("Command '%s' \r\n",&command);
									if (doecho) sendto(newsock,&return_sequence,2,0);

									// handle command
									handleCommand(newsock,cmd_pos);

									int dir_len = strlen(current_dir);
									memcpy(&return_msg[0],&current_dir[0],dir_len);
									return_msg[dir_len] = '$';
									return_msg[dir_len+1] = ' ';


									//sendto(newsock,unknown_command,strlen(unknown_command),0);
									//current_dir[0] = '/';
									//current_dir[1] = '$';
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
