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

#include "inc/error.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;
/*
 * 0: Word
 * 2  Half Word
 * 3: Double Word
 */
static inline int decodeAccessSize(unsigned int instr){
    instr = (instr >> 19) & 3;

    if(!instr){
        return 4;
    }
    else if(instr == 3){
        return 8;
    }
    else if(instr == 2){
        return 2;
    }
    else {
        //TODO: Handle error
        return 0;
    }
}

static inline unsigned int fetchReg(unsigned int reg, unsigned int* sp_int)
{
    unsigned int *sp;

    if(reg < 16){ //register is on the trap stack
        return (!reg ? 0 : *(sp_int + (reg)));
    }
    else{ //register is on the stack of the trapped routine
        //get sp of trapped routine (= fp of trap routine)
        sp = sp_int + (14);
        sp = (unsigned int *) *sp;
    return *( sp + (reg-16) );
    }

}

static inline unsigned int fetchRegAddr(unsigned int reg, unsigned int* sp_int)
{
    unsigned int *sp;

    if(reg < 16){ //register is on the trap stack
        return (unsigned int)(sp_int + (reg));
    }
    else{ //register is on the stack of the trapped routine
        //get sp of trapped routine (= fp of trap routine)
        sp = sp_int + (14);
        sp = (unsigned int *) *sp;
    return (unsigned int)( sp + (reg-16) );
    }
}

static inline int signExtendImm13(int imm)
{
    return imm << 19 >> 19;
    }


static inline unsigned int computeEffectiveAddress(unsigned int instr, unsigned int *sp_int ){
    unsigned int rs1 = (instr >> 14) & 0x1f;
    unsigned int rs2 = instr & 0x1f;
    //unsigned int rd = (instr >> 25) & 0x1f;

    if(instr & 0x2000) { //check for immediate bit
        return (fetchReg(rs1, sp_int) + signExtendImm13(instr));
    } else {
        return (fetchReg(rs1, sp_int) + fetchReg(rs2, sp_int));
    }
}

#define doIntegerLoad(destReg, size, saddr, isSigned)      \
    asm volatile (                                        \
                /*switch size*/                            \
                   "cmp     %1, 8;"                        \
                "be         _double;"                        \
                "cmp     %1, 4;"                            \
                "be         _word;"                            \
                "ldub     [%2], %%l1;"                    \
                                                        \
  /*half word*/ "ldub    [%2], %%l1;"                    \
                  "ldub    [%2 + 1], %%l2;"                \
                  "sll    %%l1, 8, %%l1;"                    \
                  "tst    %3;"                            \
                  "be     _unsigned;"                        \
                  "add    %%l1, %%l2, %%l1;"                \
     /*signed*/    "sll    %%l1, 16, %%l1;"                \
                  "sra    %%l1, 16, %%l1;"                \
                                                          \
    "_unsigned:" "b      _end;"                            \
                 "st    %%l1, [%0];"                    \
                                                        \
    "_word:"    "ldub     [%2 + 1], %%l2;"                \
                "sll       %%l1, 24, %%l1;"                \
                "ldub   [%2 + 2], %%g7;"                \
                "sll    %%l2, 16, %%l2;"                \
                "ldub   [%2 + 3], %%g1;"                \
                "sll    %%g7, 8, %%g7;"                    \
                "or     %%l1, %%l2, %%l1;"                \
                "or     %%g7, %%g1, %%g7;"                \
                "or     %%l1, %%g7, %%l1;"                \
                "b      _end;"                            \
                "st    %%l1, [%0];"                        \
                                                        \
    "_double:"    "ldub   [%2], %%l1;"                    \
                "ldub   [%2 + 1], %%l2;"                \
                "sll    %%l1, 24, %%l1;"                \
                "ldub   [%2 + 2], %%g7;"                \
                "sll    %%l2, 16, %%l2;"                \
                "ldub   [%2 + 3], %%g1;"                \
                "sll    %%g7, 8, %%g7;"                    \
                "or     %%l1, %%l2, %%l1;"                \
                "or     %%g7, %%g1, %%g7;"                \
                "or     %%l1, %%g7, %%g7;"                \
                "ldub   [%2 + 4], %%l1;"                \
                "st     %%g7, [%0];"                    \
                "ldub     [%2 + 5], %%l2;"                \
                "sll     %%l1, 24, %%l1;"                \
                "ldub   [%2 + 6], %%g7;"                \
                "sll    %%l2, 16, %%l2;"                \
                "ldub   [%2 + 7], %%g1;"                \
                "sll    %%g7, 8, %%g7;"                    \
                "or     %%l1, %%l2, %%l1;"                \
                "or     %%g7, %%g1, %%g7;"                 \
                "or     %%l1, %%g7, %%g7;"                \
                "st     %%g7, [%0 + 4];"                \
    "_end:"        ";"                                        \
                :                                        \
                : "r" (destReg), "r" (size), "r" (saddr), "r" (isSigned)        \
                : "l1", "l2", "g7", "g1");                                        \



#define doIntegerStore(dstAddr, size, srcVal)                                      \
    asm volatile (                                                               \
            "ld     [%2], %%l1;"                                                 \
            "cmp    %1, 2;"                                                      \
            "be     _lhw;"                                                      \
            "cmp    %1, 4;"                                                      \
            "be     _lw;"                                                       \
            "srl    %%l1, 24, %%l2;"                                             \
            "srl    %%l1, 16, %%g7;"                                             \
            "stb    %%l2, [%0];"                                                 \
            "srl    %%l1, 8, %%l2;"                                              \
            "stb    %%g7, [%0 + 1];"                                             \
            "ld     [%2 + 4], %%g7;"                                             \
            "stb    %%l2, [%0 + 2];"                                             \
            "srl    %%g7, 24, %%l2;"                                             \
            "stb    %%l1, [%0 + 3];"                                             \
            "srl    %%g7, 16, %%l1;"                                             \
            "stb    %%l2, [%0 + 4];"                                             \
            "srl    %%g7, 8, %%l2;"                                              \
            "stb    %%l1, [%0 + 5];"                                             \
            "stb    %%l2, [%0 + 6];"                                             \
            "b      _endst;"                                                       \
            "stb    %%g7, [%0 + 7];"                                             \
  "_lw:"  "srl    %%l1, 16, %%g7;"                                               \
            "stb    %%l2, [%0];"                                                 \
            "srl    %%l1, 8, %%l2;"                                              \
            "stb    %%g7, [%0 + 1];"                                             \
            "stb    %%l2, [%0 + 2];"                                             \
            "b      _endst;"                                                       \
            "stb    %%l1, [%0 + 3];"                                             \
  "_lhw:"   "srl    %%l1, 8, %%l2;"                                              \
            "stb    %%l2, [%0];"                                                 \
            "stb    %%l1, [%0 + 1];"                                             \
  "_endst:" ";"                                                                     \
            :                                                                      \
            : "r" (dstAddr), "r" (size), "r" (srcVal)                            \
            : "l1", "l2", "g7", "g1"                                             \
);                                                                                 \


/*
 * Handles the mem_not_aligned trap.
 */
extern "C" void unalignedTrapHandler(unsigned int* sp_int, unsigned int instr){

    /*
     * 0: Load
     * 1: Store
     */
     int direction = (instr >> 21) & 1;

    /*
     * 0: Unsigned
     * 1: Signed
     */
     int signedness = (instr >> 22) & 1;

     int size = decodeAccessSize(instr);

     unsigned int addr = computeEffectiveAddress(instr, sp_int);

//     if (theOS !=0 && theOS->getLogger() != 0)
  //   LOG(KERNEL,INFO,(KERNEL,INFO,"--- ALIGNMENT--- addr @ 0x%x",instr));

     // invalid address (for testing)
     /*if (addr < 0x40000000){
         asm volatile (
                     "ta 1;"
                );
     }
     */

     switch(direction) {
         case 0: { /*load*/
            doIntegerLoad(fetchRegAddr(((instr>>25)&0x1f), sp_int),
                          size, (unsigned int *) addr, signedness);
            break;
         }

         case 1: { //store
             unsigned int *srcVal;
             static unsigned int zero[2] = { 0, };
                 unsigned int regNum = ((instr>>25)&0x1f);
                 if (regNum != 0) {
                     srcVal = (unsigned int*) fetchRegAddr(regNum, sp_int);
                 }else {
                     srcVal = &zero[0];
                     if (size == 8)
                         zero[1] = fetchReg(1, sp_int);
                 }
                 doIntegerStore((unsigned int *) addr, size, srcVal);
             break;
         }
         default:{
             //TODO: Handle error
             break;
         }
     }

     asm volatile (
                 "mov %0, %%i0;"
                 :
                 : "r" (sp_int)
                 :
            );
}
