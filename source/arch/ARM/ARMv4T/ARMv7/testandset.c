/*
 * testandset.c
 *
 *  Created on: 23.10.2013
 *    Copyright & Author: dbaldin
 */

/*****************************************************************************
 * Method: testandset(void* address, int testvalue, int setvalue)
 *
 * @description
 * Tries to set 'setvalue' at address 'address'. Before setting the value
 * it tests the address on testvalue. If the address contains testvalue
 * or setting setvalue fails (due to concurrent access) the function returns 0.
 * On exclusively and successfully setting setvalue the function returns 1.
 * This function is SMP safe.
 *******************************************************************************/
int testandset(void* address, int testvalue, int setvalue) {
    int result;

    // multi processor scaleable test and set for ARM >= v6
    __asm__ volatile (
            "LDREX     r0, [%1];"       // load the address value
            "CMP       r0, %2;"         // compare with test value
            "ITT       EQ;"             // if then then (2 conditional instr following)
            "STREXEQ   r0, %3, [%1];"   // try to store the set value and get success to r0
            "CMPEQ     r0, #0;"         // did it succeed?
            "ITE       EQ;"             // if then else
            "MOVEQ     %0, #1;"         // return value == 1 on success
            "MOVNE     %0, #0;"         // return value == 0 on failure
            : "=&r" (result)
            : "r" (address) , "r" (testvalue) , "r" (setvalue)
            : "r0"
    );

    return (result);
}
