/*
 * testandset.c
 *
 *  Created on: 23.10.2013
 *      Author: dbaldin
 */


int testandset(void* address, int testvalue, int setvalue) {

	int result;

	// multi processor scaleable test and set for ARM >= v6
	asm (
		"mov 	   r1, %3;"        // load the set value
		"LDREX 	   r0, [%1];" 	   // load the address value
		"CMP       r0, %2;"        // compare with test value
		"ITT       EQ;"  		   // if then then
		"STREXEQ   r0, r1, [%1];"  // try to store the set value and get success to r0
		"CMPEQ     r0,#0;"		   // did it succeed?
		"ITE       EQ;"   		   // if then else
		"MOVEQ     %0,#1;"		   // return value == 1 on success
		"MOVNE     %0, #0;"        // return value == 0 on failure
		: "=&r" (result)
		: "r" (address) , "r" (testvalue) , "r" (setvalue)
		: "r0", "r1"
		);

	return (result);
}
