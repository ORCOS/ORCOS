/*
 * MigrationManager.hh
 *
 *  Created on: 16.06.2009
 *      Author: dbaldin
 */

#ifndef MIGRATIONMANAGER_HH_
#define MIGRATIONMANAGER_HH_

#include "comm/Socket.hh"
#include "process/Thread.hh"


#define MM_TYPE_REQUEST     1
#define MM_TYPE_REPLY       2

#define MM_REPLY_OK                     1
#define MM_REPLY_TOO_MANY_MIGRATIONS    2
#define MM_REPLY_NOT_ENOUGH_MEMORY      4
#define MM_REPLY_NO_VIRTUAL_MEMORY		8

#define MM_TYPE_TASKMETA    4 // meta information packet, contains the task cb
#define MM_TYPE_PAYLOAD     8
#define MM_TYPE_FINAL       16


struct MM_Header {
    unint2 id;
    unint2 type;
};

struct MM_Reply {
    MM_Header  header;
    unint1 reply_value;
};

struct MM_Request {
    MM_Header  header;
    unint2 task_size;
    // .. some other parameters
};

struct MM_TaskMeta {
    MM_Header  header;
    // serialized task following!
};


struct MM_Payload {
    MM_Header  header;
    unint4 startaddr;
    unint2 len;
};

// we will use maximum packets sizes of 1024 bytes payload
#define MM_PAYLOAD_SIZE 1024

struct MM_Mem_Block {
    unint4 logical_start_addr;
    unint2 size; // maxium MM_PAYLOAD_SIZE
};


struct MM_Migration {
    unint2 free;        // is this migration slot used?
    unint2 id;          // the id of this migration process
    sockaddr  target;   // the target of the migration process
    Task*  task;        // the task to be migrated

    MM_Mem_Block blocks[64]; // the memory blocks to be transferred
    unint8 pl_tx;       // a 64 bit bitmap which indicates which parts of the task payload have been received/send

};

class MigrationManager: public CallableObject{
private:
    Socket* listenSocket;
    char* mysocketbuffer;

    unint2 id;

    MM_Migration currentOutgoingTask;

    MM_Migration currentIncomingTask;

private:

    int getFirstUsedBitPos(unint8 bitmap)
    {
        for (int i = 0; i <= 63; i++)
        {
            unint8 a = ((unint8) 1) << (63 - i);
            if ((bitmap & a) == a ) return i;
        }

        return -1;
    }

public:
    MigrationManager();
    ~MigrationManager();

    void handleMigrationReply(MM_Reply* reply,sockaddr* sender);

    void handleMigrationRequest(MM_Request* request,sockaddr* sender);

    void handleMigrationTaskMeta(MM_TaskMeta*, unint2 len,sockaddr* sender);

    void handleMigrationPayload(MM_Payload*,unint2 len,sockaddr* sender);

    ErrorT migrateTask(Task* task,sockaddr* destination);

    void callbackFunc( void* param );
};

#endif /* MIGRATIONMANAGER_HH_ */
