/*
 * args.h
 *
 *  Created on: 20.09.2014
 *      Author: Daniel
 */

#ifndef ARGS_H_
#define ARGS_H_


extern "C"  char* extractNextArg(char* &str);
extern "C"  int parseArgs(char* str, char** &argv);
extern "C"  void compactPath(char* path);

#endif /* ARGS_H_ */
