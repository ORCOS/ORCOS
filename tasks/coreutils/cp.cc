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

char* extractNextArg(char* &str) {

	while (*str == ' ') {
		*str = 0;
		str++;
	}

	if (*str == '"') {
		*str = 0;
		str++;
		int len = strpos("\"",str);
		if (len < 0) return (0);
		str[len] = 0;
		return (&str[len+1]);
	}

	// first char != 0 & != '"'
	int len = strpos(" ",str);
	if (len < 0) {
		return (0);
	}

	// something is following
	str[len] = 0;
	return (&str[len+1]);
}


int parseArgs(char* str, char** &argv) {
	int arg_count = 0;

	char* curstr = str;

	curstr = extractNextArg(curstr);
	while (curstr != 0) {
		arg_count++;
		curstr = extractNextArg(curstr);
	}

	argv = (char**) malloc(sizeof(char*) * (arg_count+2));
	for (int i = 0; i < arg_count+2; i++)
		argv[i] = 0;

	char* pos = str;
	for (int i = 0; i < arg_count; i++) {
		while (*pos == 0) pos++;
		argv[i] = pos;
		while (*pos != 0) pos++;
	}

	return (arg_count+1);
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

// entry point
extern "C" int task_main(char* args)
{
	volatile int val = 0;

	if (args == 0) {
		puts("Usage: cp [OPTION]... SOURCE_FILE DEST_FILE\r");
		thread_exit(-1);
	}
	// in place argument detection
	char** argv;
	int argc = parseArgs(args,argv);

	printf("args %d:\r",argc);

	for (int i = 0; i < argc; i++) {
		printf("argv[%d] = 0x%x\r",i, (unint4) argv[i]);
	}


	thread_exit(cOk);
}
