/*
 * main.h
 *
 *  Created on: 26.10.2009
 *      Author: dbaldin
 */

#ifndef MAIN_H_
#define MAIN_H_

extern int mysock  __attribute__ ((aligned (8)));;

struct decoder_attr {
	sockaddr sender;
	void* msgptr;
};

extern decoder_attr args[10] __attribute__ ((aligned (8)));


extern "C" int main(int i,char** b);

#endif /* MAIN_H_ */
