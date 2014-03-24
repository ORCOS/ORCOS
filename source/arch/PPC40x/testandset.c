/*
 * testandset.c
 *
 *  Created on: 23.10.2013
 *      Author: dbaldin
 */


int testandset(void* address, int testvalue, int setvalue) {

	int result;

	// multi processor scaleable test and set for PPC405
	asm (
			"loop:"
			"lwarx  %0,  0, %1;" /* load and reserve address */
			"cmpwi  %0, %2;"     /* compare with testvalue */
			"bne    fail;"       /* branch if not equal to testvalue */
			"stwcx. %3,  0, %1;" /* try to store non-zero */
			"li     %0,1;"  	 /* indicate success */
			"b    exit;"
			"fail:"
			"li    %0,0;"		 /* indicate fail */
			"exit:"

		: "=&r" (result)
		: "r" (address) , "r" (testvalue) , "r" (setvalue)
		:
		);

	return (result);
}
