/*
 * KernelVariableFile.hh
 *
 *  Created on: 23.09.2014
 *      Author: Daniel
 */

#ifndef KERNELVARIABLEFILE_HH_
#define KERNELVARIABLEFILE_HH_

#include "SCLConfig.hh"

#if SYSFS_SUPPORT

#include <filesystem/File.hh>
#include <filesystem/Directory.hh>
#include <inc/types.h>

#define READ  1
#define WRITE 2
#define RW (READ | WRITE)
#define RO READ

class KernelVariable: public File {

    void* address;

    unint2 mode;

public:
    KernelVariable(char* name, int size, void* addr, int mode, SysFs_t type);

    virtual ~KernelVariable();

    ErrorT readBytes(char *bytes, unint4 &length);

    ErrorT writeBytes(const char *bytes, unint4 length);

    static void exportVariable(Directory* parent, char* name, void* address, int size, int mode, SysFs_t type);

    static void init();

    static Directory* getEntry(char* name, bool create);

};


#define EXPORT_VARIABLE_BY_NAME(parent, name, type, var, mode) KernelVariable::exportVariable(parent,name, &(var),sizeof((var)),mode, type);
#define EXPORT_VARIABLE(parent, type, var, mode) KernelVariable::exportVariable(parent, #var,&(var), sizeof(var), mode, type);
#else
#define EXPORT_VARIABLE_BY_NAME(parent, name, type, var, mode)
#define EXPORT_VARIABLE(parent, type, var, mode)
#endif


#endif /* KERNELVARIABLEFILE_HH_ */
