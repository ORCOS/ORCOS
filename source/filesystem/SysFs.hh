/*
 * KernelVariableFile.hh
 *
 *  Created on: 23.09.2014
 *    Copyright &  Author: Daniel
 */

#ifndef KERNELVARIABLEFILE_HH_
#define KERNELVARIABLEFILE_HH_

#include "SCLConfig.hh"

#if SYSFS_SUPPORT

#include <filesystem/File.hh>
#include <filesystem/Directory.hh>
#include <inc/orcos_types.h>

#define READ  1
#define WRITE 2
#define RW (READ | WRITE)
#define RO READ

/* the syscall handler signature */
typedef ErrorT (*sysfs_write_handler_t)(const void* data, int size);


class KernelVariable: public File {
    void* address;

    /* handler to the callback method if data is written to this variable */
    sysfs_write_handler_t onWriteHandler;

    unint2 mode;


public:
    KernelVariable(char* name, int size, void* addr, int mode, SysFs_t type, sysfs_write_handler_t onWriteHandler);

    virtual ~KernelVariable();

    /*****************************************************************************
     * Method: readBytes(char *bytes, unint4 &length)
     *
     * @description
     *
     *******************************************************************************/
    ErrorT readBytes(char *bytes, unint4 &length);

    /*****************************************************************************
     * Method: writeBytes(const char *bytes, unint4 length)
     *
     * @description
     *
     *******************************************************************************/
    ErrorT writeBytes(const char *bytes, unint4 length);

    /*****************************************************************************
     * Method: exportVariable(Directory* parent, char* name, void* address, int size, int mode, SysFs_t type)
     *
     * @description
     *
     *******************************************************************************/
    static void exportVariable(Directory* parent, char* name, void* address, int size, int mode, SysFs_t type, sysfs_write_handler_t writeCallback = 0);

    /*****************************************************************************
     * Method: init()
     *
     * @description
     *
     *******************************************************************************/
    static void init();


    /*****************************************************************************
     * Method: getEntry(char* name, bool create)
     *
     * @description
     *
     *******************************************************************************/
    static Directory* getEntry(char* name, bool create);
};

#define SYSFS_ADD_RO_UINT(parent, var)                       KernelVariable::exportVariable(parent, #var, &(var), sizeof(var), RO, SYSFS_UNSIGNED_INTEGER)
#define SYSFS_ADD_RO_UINT_NAMED(parent, name, var)           KernelVariable::exportVariable(parent, name, &(var), sizeof(var), RO, SYSFS_UNSIGNED_INTEGER)
#define SYSFS_ADD_RW_UINT(parent, var, callback)             KernelVariable::exportVariable(parent, #var, &(var), sizeof(var), RO, SYSFS_UNSIGNED_INTEGER, callback)
#define SYSFS_ADD_RW_UINT_NAMED(parent, name, var, callback) KernelVariable::exportVariable(parent, name, &(var), sizeof(var), RO, SYSFS_UNSIGNED_INTEGER, callback)

#define SYSFS_ADD_RO_INT(parent, var)                        KernelVariable::exportVariable(parent, #var, &(var), sizeof(var), RO, SYSFS_SIGNED_INTEGER)
#define SYSFS_ADD_RO_INT_NAMED(parent, name, var)            KernelVariable::exportVariable(parent, name, &(var), sizeof(var), RO, SYSFS_SIGNED_INTEGER)
#define SYSFS_ADD_RW_INT(parent, var, callback)              KernelVariable::exportVariable(parent, #var, &(var), sizeof(var), RO, SYSFS_SIGNED_INTEGER, callback)
#define SYSFS_ADD_RW_INT_NAMED(parent, name, var, callback)  KernelVariable::exportVariable(parent, name, &(var), sizeof(var), RO, SYSFS_SIGNED_INTEGER, callback)

#define SYSFS_ADD_RO_STRING(parent, var)                       KernelVariable::exportVariable(parent, #var, &(var), sizeof(var), RO, SYSFS_STRING)
#define SYSFS_ADD_RO_STRING_NAMED(parent, name, var)           KernelVariable::exportVariable(parent, name, &(var), sizeof(var), RO, SYSFS_STRING)
#define SYSFS_ADD_RW_STRING(parent, var, callback)             KernelVariable::exportVariable(parent, #var, &(var), sizeof(var), RO, SYSFS_STRING, callback)
#define SYSFS_ADD_RW_STRING_NAMED(parent, name, var, callback) KernelVariable::exportVariable(parent, name, &(var), sizeof(var), RO, SYSFS_STRING, callback)

#else

#define SYSFS_ADD_RO_UINT(parent, var)
#define SYSFS_ADD_RO_UINT_NAMED(parent, name, var)
#define SYSFS_ADD_RW_UINT(parent, var, callback)
#define SYSFS_ADD_RW_UINT_NAMED(parent, name, var, callback)

#define SYSFS_ADD_RO_INT(parent, var)
#define SYSFS_ADD_RO_INT_NAMED(parent, name, var)
#define SYSFS_ADD_RW_INT(parent, var, callback)
#define SYSFS_ADD_RW_INT_NAMED(parent, name, var, callback)

#define SYSFS_ADD_RO_STRING(parent, var)
#define SYSFS_ADD_RO_STRING_NAMED(parent, name, var)
#define SYSFS_ADD_RW_STRING(parent, var, callback)
#define SYSFS_ADD_RW_STRING_NAMED(parent, name, var, callback)

#endif


#endif /* KERNELVARIABLEFILE_HH_ */
