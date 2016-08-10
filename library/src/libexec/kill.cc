/*
 * kill.cc
 *
 *  Created on: 19.11.2015
 *      Author: Daniel
 */


#include <stdio.h>
#include <string.h>
#include <error.h>
#include <stdlib.h>
#include <orcos.h>

int exec_kill(int argc, char** argv) {
    if (argc != 2) {
        return (cInvalidArgument);
     }

   int id = atoi(argv[1]);
   if (id == 0) {
       return (cInvalidArgument);
   }
   if (id != getpid()) {
       int error = task_kill(id);
       if (error < 0) {
           return (error);
       } else {
           printf("[%d] Killed" LINEFEED, id);
       }
   } else {
       printf("I dont want to kill myself.. mumble" LINEFEED);
   }

   return (cOk);
}
