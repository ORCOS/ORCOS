/*
 * args.h
 *
 *  Created on: 20.09.2014
 *      Author: Daniel
 */

#ifndef ARGS_H_
#define ARGS_H_

#ifdef __cplusplus
extern "C" {
#endif

char* extractNextArg(char* &str);
int parseArgs(char* str, char** &argv);
void compactPath(char* path);

#ifdef __cplusplus
}
#endif


#endif /* ARGS_H_ */
