/*
 * telnet.h
 *
 *  Created on: 29.01.2015
 *      Author: Daniel
 */

#ifndef TASKS_TELNET_SERVER_TELNET_H_
#define TASKS_TELNET_SERVER_TELNET_H_

#include <orcos.hh>

/*** Escape Sequenzen: **********/
#define ESC_RED         "\033[31m"
#define ESC_GREEN       "\033[32m"
#define ESC_YELLOW      "\033[33m"
#define ESC_BLUE        "\033[34m"
#define ESC_PURPLE      "\033[35m"
#define ESC_CYAN        "\033[36m"
#define ESC_GRAY        "\033[37m"
#define ESC_WHITE       "\033[0m"

#define IPV4    0x800
#define TCP     0x6

#define DOECHO 1

#define SB          250 // subnegotation begin
#define SE          240 // subnegotation end
#define WILL        0xfb
#define WONT        0xfc
#define DO          0xfd
#define DONT        0xfe
#define IAC         0xff

#define SGA         3

#define LINEMODE    34
#define MODE        1
#define FORWARDMASK 2

#define LINEFEED "\r\n"

void sendMsg(int socket, char* msg, int error = 0) ;
void sendData(int socket, char* data, int len);
void sendUnknownCommand(int socket);
void sendStr(int socket,char* str);

void makeTelnetCharCompatible(char* msg, int len);
void makeHexCharCompatible(char* msg, int len);

bool readKernelVarStr(char* filepath,char* result, int size);
bool readKernelVarUInt4(char* filepath, unint4* result);
bool readKernelVarUInt8(char* filepath,unint8* result);
bool readKernelVarInt(int filehandle, void* result, int size);

const char* getTypeStr(int resourceType);
const char* getStatusStr(unint4 status);

char* extractPath(char* &str);

void command_df(int socket, int showBlockCount);
void command_ps(int socket, int showThreads);
void command_ls(int socket, int handle, int details, int humanReadable);

#endif /* TASKS_TELNET_SERVER_TELNET_H_ */
