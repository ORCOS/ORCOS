/*
 * date.cc
 *
 *  Created on: 08.06.2016
 *      Author: Daniel
 */

#include <stdio.h>
#include <string.h>
#include <error.h>
#include <stdlib.h>
#include <orcos.h>

int exec_date(int argc, char** argv)
{
    time_t dateTime = (time_t) getDateTime();
    printf("%s", ctime(&dateTime));
    return (0);
}
