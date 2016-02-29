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

#include <orcos.h>
#include <string.h>
#include <args.h>
#include <time.h>
#include "telnet.h"
#include <stdlib.h>
#include <stdio.h>


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
static char ci           = 0;
static int history_count = 0;

// the command history
static char command_history[MAX_COMMAND_HISTORY][100];

// the current command being received
static char command[100];

// the sequence to perform a back char on the terminal
static char back_sequence[3] = { 0x8, 0x20, 0x8 };
static char down_sequence[3] = { 0x1b, 0x5b, 0x42 };

// line feed
static char return_sequence[4] = { 13, 0, 13, 0 };

// current directory we are in
static char current_dir[256];
static char recvMsg[1024];


static const char* help_msg =   "OCROS Telnet Terminal\r\nSupported commands:"LINEFEED
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
static int  cmd_pos = 0;
static int  doecho;
char        telnetcommand[10];

extern "C" char* default_stdout;
size_t sendStr(char* str);

int handleTelnetCommand(int socket, char* bytes) {
    switch (bytes[0]) {
    case 241:
        return 0;
    case 246: {
        telnetcommand[0] = 0xff;
        telnetcommand[1] = 249;
        sendto(socket, telnetcommand, 2, 0);
        return 0;
    }
    case 247: {
        if (cmd_pos > 0) {
            command[cmd_pos--] = 0;
            if (doecho)
                sendto(socket, &back_sequence, 3, 0);
        }
        return 0;
    }
    case WILL: {
        switch (bytes[1]) {
        case LINEMODE:
            telnetcommand[0] = IAC;
            telnetcommand[1] = SB;
            telnetcommand[2] = LINEMODE;
            telnetcommand[3] = MODE;
            telnetcommand[4] = 0;
            telnetcommand[5] = IAC;
            telnetcommand[6] = SE;
            sendto(socket, telnetcommand, 7, 0);
            return 1;
        default:
            /*      telnetcommand[0] = 0xff;
             telnetcommand[1] = DONT;
             telnetcommand[2] = bytes[1];
             sendto(socket,telnetcommand,3,0);*/
            return 1;
        }
    }
    case DO: {

        switch (bytes[1]) {
        case 1:  // ECHO
            telnetcommand[0] = IAC;
#if DOECHO
            telnetcommand[1] = WILL;
#else
            telnetcommand[1] = WONT;
#endif
            telnetcommand[2] = bytes[1];
            sendto(socket, telnetcommand, 3, 0);
            return 1;
        case SGA:
            telnetcommand[0] = IAC;
            telnetcommand[1] = WILL;
            telnetcommand[2] = SGA;
        default:
            return 1;
        }
    }
    case DONT: {
        if (bytes[1] == 1) {
            doecho = 0;
        }

        return 1;
    }
    case WONT: {
        return 1;
    }

    default:
        return 0;
    }

}

static char tty0buf[256];
static int newsock;

size_t sendStr(char* str) {
    int len = strlen(str);
    if (newsock) {
        sendto(newsock, str, len + 1, 0);
    }
    return (len);
}

void handleCommand(int socket, int command_length) {
    if (command_length == 0)
    {
        sendStr("\n");
    }
    else
    {
        if (strcmp("help", command) == 0) {
            memcpy(&return_msg[0], help_msg, strlen(help_msg) + 1);
            sendto(socket, &return_msg, strlen(help_msg) + 1, 0);
            return;
        }

        default_stdout = "/dev/tty0";
        setstdout(sendStr);
        ErrorT ret = system(command);
        fflush(stdout);
        setstdout(0);
    }
}


void* tty0_thread(void* arg) {
    int devid = (int) arg;
    while (1) {
        int readb = read(devid, tty0buf, 256);
        if (readb > 0 && newsock != 0) {
            send(newsock, tty0buf, readb);
        }
        usleep(10000);
    }
}





void handleMsg(char* msgptr, int msglen) {
    for (int i = 0; i < msglen; i++) {
       if (msgptr[i] > 31 && msgptr[i] < 127) {
           // normal char .. just copy
           command[cmd_pos++] = msgptr[i];
           if (doecho) {
               sendto(newsock, &msgptr[i], 1, 0);
           }
       } else
           // all other cases are special chars
           switch (msgptr[i]) {
           case 0xff: {
               // handle telnet command
               i++;
               i += handleTelnetCommand(newsock, &msgptr[i]);
               break;
           }
           case 0x7f: /* backspace key */
           case 0x8: {
               if (cmd_pos > 0) {
                   command[cmd_pos--] = 0;
                   if (doecho) {
                       sendto(newsock, &back_sequence, 3, 0);
                   }
               }
               break;
           }
           case 0x1b: {
               // esc sequence
               if ((msglen - i) > 2) {
                   // at least 2 more chars following .. thus might be a escape sequence
                   i += 2;
                   if (msgptr[i - 1] == 0x5b) {
                       switch (msgptr[i]) {
                       case 0x41: {  // oben
                           if (history_count == 0)
                               break;

                           if (ci > 0) {
                               ci--;
                           } else {
                               ci = history_count - 1;
                           }

                           int num_back = cmd_pos;

                           strcpy(command, command_history[ci]);
                           cmd_pos = strlen(command);

                           // send back sequences
                           for (int j2 = 0; j2 < num_back; j2++) {
                               memcpy(&return_msg[j2 * 3], &back_sequence, 3);
                           }
                           sendto(newsock, return_msg, num_back * 3, 0);
                           // send command from history
                           sendto(newsock, &command, strlen(command), 0);
                           break;
                       }
                       case 0x42: {  // unten
                           if (history_count == 0)
                               break;

                           ci = (ci + 1) & (MAX_COMMAND_HISTORY - 1);
                           if (ci >= history_count)
                               ci = 0;

                           int num_back = cmd_pos;

                           strcpy(command, command_history[ci]);
                           cmd_pos = strlen(command);

                           for (int j2 = 0; j2 < num_back; j2++)
                              memcpy(&return_msg[j2 * 3], &back_sequence, 3);

                           sendto(newsock, return_msg, num_back * 3, 0);

                           sendto(newsock, &command, strlen(command), 0);
                           break;
                       }
                       case 0x44:
                           break;  // nach links taste
                       case 0x43:
                           break;  // rechts
                       }
                   }
               }  // else ignore ESC
               break;
           }
           case 13: {
               i++;
               if (msglen - i > 0) {
                   if (msgptr[i] == 0 || msgptr[i] == 0xa) {
                       command[cmd_pos] = '\0';

                       if (doecho) {
                           sendto(newsock, &return_sequence, 2, 0);
                       }
                       /* send a linefeed */
                       sendMsg(newsock, "");

                       if (strlen(command) > 0) {
                           // remember last command
                           strcpy(command_history[ci], command);
                           ci = (ci + 1) & (MAX_COMMAND_HISTORY - 1);

                           if (history_count < MAX_COMMAND_HISTORY)
                               history_count++;
                       }

                       // handle command
                       handleCommand(newsock, cmd_pos);

                       getcwd(current_dir, 255);
                       int dir_len = strlen(current_dir);
                       memcpy(&return_msg[0], &current_dir[0], dir_len);
                       return_msg[dir_len] = '$';
                       return_msg[dir_len + 1] = ' ';

                       sendData(newsock, return_msg, dir_len + 2);
                       cmd_pos = 0;
                   }
                   i++;
               }

               break;
           } // case 13 (ENTER)
           default:
               ;  // do nothing
       } // switch
   } // for every character received
}



void handleConnection() {
    char* msgptr;
    while (1) {
        msgptr = recvMsg;
        int msglen = recv(newsock, msgptr, 1024, MSG_WAIT);
        if (msglen == -1) {
            // disconnected
            printf("Terminal disconnected.." LINEFEED);
            close(newsock);
            newsock = 0;
            break;
        }

        if (cmd_pos + msglen < 100) {
            handleMsg(msgptr, msglen);
        }  // if msglen < 100
    }  // while connected
}




extern "C" int main(int argc, char** argv) {
    int i = 0;
    newsock = 0;

    puts("Telnet-Server starting"LINEFEED);
    int mysock = socket(IPV4, SOCK_STREAM, TCP);

    // bind our socket to some address
    sockaddr* addr = (sockaddr*) malloc(sizeof(sockaddr));

    addr->port_data = 23;                      //< the port
    addr->sa_data   = 0;                       //< any address ...

    bind(mysock, addr);

    /* create the virtual tty device */
    int devid = mkdev("tty0", 2048);
    if (devid < 0) {
        printf("Error creating tty0: %d", devid);
        return (1);
    }

    /* set current threads name */
    thread_name(0, "telnetd");

    /* create the tty0 thread  */
    thread_attr_t attr;
    memset(&attr, 0, sizeof(thread_attr_t));
    attr.priority = 1;
    ThreadIdT threadid = -1;
    thread_create(&threadid, &attr, tty0_thread, (void*) devid);
    thread_run(threadid);
    thread_name(threadid, "tty0");

    puts("Telnet-Server bound and waiting for clients."LINEFEED);

    //setstdout(sendStr);

    while (1) {
        newsock = listen(mysock, 1);
        if (newsock < 0) {
            printf("Listen error %d: %s\n", newsock, strerror(newsock));
            continue;
        }

        cmd_pos = 0;
        doecho = DOECHO;

        if (doecho) {
            // send my telnet options
            telnetcommand[0] = 0xff;
            telnetcommand[1] = WILL;
            telnetcommand[2] = 1;  // send will echo
            sendto(newsock, telnetcommand, 3, 0);
        }

        telnetcommand[0] = IAC;
        telnetcommand[1] = WILL;
        telnetcommand[2] = 3;  //  IAC WILL SUPPRESS-GOAHEAD
        sendto(newsock, telnetcommand, 3, 0);

        telnetcommand[0] = 0xff;
        telnetcommand[1] = WONT;
        telnetcommand[2] = LINEMODE;  //
        sendto(newsock, telnetcommand, 3, 0);

        //sendMsg(newsock,"^]mode char");
        telnetcommand[0] = 0xff;
        telnetcommand[1] = DONT;
        telnetcommand[2] = 1;  // send will echo
        sendto(newsock, telnetcommand, 3, 0);

        history_count = 0;
        ci = 0;

        puts("New telnet connection!"LINEFEED);
        sendto(newsock, welcome_msg, strlen(welcome_msg), 0);

        chdir("/");
        current_dir[0] = '/';
        current_dir[1] = '\0';

        // loop until disconnected
        handleConnection();

    }  // loop forever

}
