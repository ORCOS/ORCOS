/*
 * MigrationManager.cc
 *
 *  Created on: 16.06.2009
 *      Author: dbaldin
 */

#include "MigrationManager.hh"
#include "kernel/Kernel.hh"
#include "lib/defines.h"
#include "inc/memtools.hh"

extern Kernel* theOS;
extern ThreadCfdCl* pCurrentRunningThread;

MigrationManager::MigrationManager() {

    currentIncomingTask.free = 1;
    currentIncomingTask.id = -1;
    currentIncomingTask.task = 0;

    currentOutgoingTask.free = 1;

    this->id = 1;


#if USE_WORKERTASK
    TimedFunctionCall* jobparam = new TimedFunctionCall;
    jobparam->objectptr = this; // call this object
    jobparam->parameterptr = 0;
    jobparam->time = theOS->getClock()->getTimeSinceStartup() + 1000 ms; // call the first time in 5 seconds

    // we got a workertask inside the kernel so we can assign the periodic send  job to a workerthread
    if ( !theOS->getWorkerTask()->addJob( TimedFunctionCallJob, 0, jobparam, 0000 ) ) {
        ERROR("Could not assign a Workerthread to the MigrationManager!");
    }
#else
    // kernel compiled without workertask concept
    ERROR("MigrationManager will not work without Workerthreads!");
#endif

}

MigrationManager::~MigrationManager() {


}

void MigrationManager::handleMigrationRequest( MM_Request* request, sockaddr* sender ) {

#ifndef HAS_MemoryManager_HatLayerCfd
	 LOG(KERNEL,WARN,(KERNEL,WARN,"MigrationManager: Rejecting incoming migration. Reply: NO_VIRTUAL_MEMORY"));
	 MM_Reply reply;
	 reply.header.id = 0;
	 reply.header.type = MM_TYPE_REPLY;
	 reply.reply_value = MM_REPLY_NO_VIRTUAL_MEMORY;
	 listenSocket->sendto(&reply,sizeof(MM_Reply),sender);
#else

    if ( currentIncomingTask.free ) {
        LOG(KERNEL,INFO,(KERNEL,INFO,"MigrationManager: New incoming migration. Reply: OK"));
        // send a reply!
        currentIncomingTask.id = this->id++;
        currentIncomingTask.pl_tx = -1;
        currentIncomingTask.task = 0;
        currentIncomingTask.free = 0;
        currentIncomingTask.target = *sender;

        MM_Reply reply;
        reply.header.id = currentIncomingTask.id;
        reply.header.type = MM_TYPE_REPLY;
        reply.reply_value = MM_REPLY_OK;
        listenSocket->sendto(&reply,sizeof(MM_Reply),sender);
    }
    else
    {
        LOG(KERNEL,WARN,(KERNEL,WARN,"MigrationManager: Rejecting incoming migration. Reply: NO"));
        MM_Reply reply;
        reply.header.id = 0;
        reply.header.type = MM_TYPE_REPLY;
        reply.reply_value = MM_REPLY_TOO_MANY_MIGRATIONS;
        listenSocket->sendto(&reply,sizeof(MM_Reply),sender);
    }
#endif
}

void MigrationManager::handleMigrationReply( MM_Reply* reply, sockaddr* sender ) {
    // we must be migrating otherwise a reply must be dropped!
    if (currentOutgoingTask.free == 1) return;

    // we only accept replies from our target migration node!
    if ( currentOutgoingTask.target.sa_data != sender->sa_data )
        return;

    if ( reply->reply_value > MM_REPLY_OK && currentOutgoingTask.id == 0) {
    	// error
    	 LOG(KERNEL,INFO,(KERNEL,INFO,"MigrationManager: Reply error: %d ",reply->reply_value));
    	currentOutgoingTask.id = -1;
     	currentOutgoingTask.free = true;
     	return;
    }

    if ( reply->reply_value == MM_REPLY_OK ) {
        // we are ready to send


        // check whether this is the initial packet
        if ( currentOutgoingTask.id == 0 ) {
            // store the unique migration id
            currentOutgoingTask.id = reply->header.id;

            char buf[ 200 ];
            unint2 len = 0;
            ( (MM_Header*) &buf )->id = currentOutgoingTask.id;
            ( (MM_Header*) &buf )->type = MM_TYPE_TASKMETA;

            Task* task = currentOutgoingTask.task;

            volatile unint4 pid;
            GETPID(pid);

            SETPID(task->getId());

            if ( !task->serialize( (void*) ( ( unint4 ) & buf + sizeof(MM_Header) ), len ) )ERROR("Tasks could not be serialized!");

            SETPID(pid);

            LOG(KERNEL,INFO,(KERNEL,INFO,"MigrationManager: Sending serialized TaskCB with length %d",len));

            // send the task
            listenSocket->sendto(buf,len + sizeof(MM_Header),sender);
        }
        else
        {
            // send the next payload packet in turn
            int next_mem_block = getFirstUsedBitPos(currentOutgoingTask.pl_tx);
            if (next_mem_block != -1)
            {


                char buf[MM_PAYLOAD_SIZE + sizeof(MM_Payload)];

                // send the block
                MM_Payload* packet = (MM_Payload*) &buf;
                packet->header.id = currentOutgoingTask.id;
                packet->header.type = MM_TYPE_PAYLOAD;
                packet->len = currentOutgoingTask.blocks[next_mem_block].size;
                packet->startaddr = currentOutgoingTask.blocks[next_mem_block].logical_start_addr;

                LOG(KERNEL,INFO,(KERNEL,INFO,"MigrationManager: Sending task memory 0x%x size: %d",packet->startaddr,packet->len));


                unint4 pid;
                GETPID(pid);
                SETPID(currentOutgoingTask.task->getId());

                // copy memory content
                memcpy((void*) ((unint4) packet + sizeof(MM_Payload)),(void*) packet->startaddr,packet->len);

                SETPID(pid);
                currentOutgoingTask.pl_tx &= ~( ((unint8) 1) << (63-next_mem_block));

                next_mem_block = getFirstUsedBitPos(currentOutgoingTask.pl_tx);
                if (next_mem_block == -1)
                packet->header.type |= MM_TYPE_FINAL;

                // send the task
                listenSocket->sendto(packet,packet->len + sizeof(MM_Payload),sender);

            }	 else
            {
            	// migration done
                LOG(KERNEL,INFO,(KERNEL,INFO,"MigrationManager: Done! "));
                currentOutgoingTask.task->terminate();
            }

        }
    }

}

void MigrationManager::handleMigrationPayload( MM_Payload* payload, unint2 len, sockaddr* sender ) {
    if ( currentIncomingTask.target.sa_data != sender->sa_data )
        return;

    if ( currentIncomingTask.id == payload->header.id && currentIncomingTask.task != 0 ) {
        LOG(KERNEL,INFO,(KERNEL,INFO,"MigrationManager: Incoming Task Payload"));

        volatile unint4 pid;
        GETPID(pid);
        SETPID(currentIncomingTask.task->getId());

        LOG(KERNEL,INFO,(KERNEL,INFO,"MigrationManager: Storing task memory 0x%x size: %d",payload->startaddr,payload->len));

        // copy memory content
        memcpy((void*) payload->startaddr,(void*) ((unint4) payload + sizeof(MM_Payload)),payload->len);

        SETPID(pid);

        MM_Reply reply;
        reply.header.id = currentIncomingTask.id;
        reply.header.type = MM_TYPE_REPLY;
        reply.reply_value = MM_REPLY_OK;
        listenSocket->sendto(&reply,sizeof(MM_Reply),sender);

        // check if this was the last packet and the new task can be started
        if ((payload->header.type & MM_TYPE_FINAL) == MM_TYPE_FINAL)
        	{
				 //done migrating
				LOG(KERNEL,INFO,(KERNEL,INFO,"MigrationManager: Migration complete! Starting new Task!"));

				theOS->getTaskDatabase()->addTail(currentIncomingTask.task );
				currentIncomingTask.task->myTaskDbItem = theOS->getTaskDatabase()->getTail();
				currentIncomingTask.task->run();

				// reset for new migration
			    currentIncomingTask.free = 1;
			    currentIncomingTask.id = -1;
			    currentIncomingTask.task = 0;

			    currentOutgoingTask.free = 1;

        	}

    }

}

ErrorT MigrationManager::migrateTask( Task* task, sockaddr* destination ) {
    if ( currentOutgoingTask.free ) {

        volatile unint4 pid;
        GETPID(pid);
        SETPID(task->getId());

        // calculate text size area
        unint4 static_size = task->tasktable->task_data_end - task->tasktable->task_entry_addr;
        // now the dynamic memory region
       // unint4 data_size = task->getMemManager()->getUsedMemSize();

        currentOutgoingTask.pl_tx = 0;

        // create Mem_blocks to be transferred
        unint4 size = 0;
        unint4 addr = task->tasktable->task_entry_addr;

        int i;

        for ( i = 0; i < 64; i++ ) {
            if ( static_size - size > 512 ) {
                currentOutgoingTask.pl_tx |= ( (unint8) 1 ) << ( 63 - i );
                currentOutgoingTask.blocks[ i ].logical_start_addr = addr;
                currentOutgoingTask.blocks[ i ].size = 512;
                addr += 512;
                size += 512;
            }
            else {
                // last block
                currentOutgoingTask.pl_tx |= ( (unint8) 1 ) << ( 63 - i );
                currentOutgoingTask.blocks[ i ].logical_start_addr = addr;
                currentOutgoingTask.blocks[ i ].size = static_size - size;
                break;
            }
        }

     /*   i++;
        size = 0;
        addr = (unint4) theOS->getHatLayer()->getLogicalAddress( (void*) task->tasktable->task_heap_start );

        for ( ; i < 64; i++ ) {
            if ( data_size - size > 1024 ) {
                currentOutgoingTask.pl_tx |= ( (unint8) 1 ) << ( 63 - i );
                currentOutgoingTask.blocks[ i ].logical_start_addr = addr;
                currentOutgoingTask.blocks[ i ].size = 1024;
                addr += 1024;
                size += 1024;
            }
            else {
                // last block
                currentOutgoingTask.pl_tx |= ( (unint8) 1 ) << ( 63 - i );
                currentOutgoingTask.blocks[ i ].logical_start_addr = addr;
                currentOutgoingTask.blocks[ i ].size = data_size - size;
                break;
            }
        }*/

        currentOutgoingTask.free = 0;
        currentOutgoingTask.task = task;
        currentOutgoingTask.id = 0;
        currentOutgoingTask.target = *destination;

        MM_Request request;

        request.task_size = task->tasktable->task_heap_end - task->tasktable->task_start_addr;
        SETPID(pid);

        request.header.id = 0;
        request.header.type = MM_TYPE_REQUEST;

        LOG(KERNEL,INFO,(KERNEL,INFO,"MigrationManager: Requesting new Migration to destination 0x%x",destination->sa_data));

        return listenSocket->sendto(&request,sizeof(MM_Request),destination);
    }
    else return cError;

}

#ifndef MM_TASK_SPACE
#define MM_TASK_SPACE (0x44000000 + (256*1024))
#endif

void MigrationManager::handleMigrationTaskMeta( MM_TaskMeta* meta, unint2 len, sockaddr* sender ) {
    // only accept data from our migration partner
    if ( currentIncomingTask.target.sa_data != sender->sa_data )
        return;

    if ( currentIncomingTask.id == meta->header.id ) {
        LOG(KERNEL,INFO,(KERNEL,INFO,"MigrationManager: Incoming TaskCB"));

        // get free address space
        void* addressspace = (void*) MM_TASK_SPACE;
        // create mapping
        // we get the ID of the task we will create next
        TaskIdT tid = Task::getIdOfNextCreatedTask();

        theOS->getHatLayer()->map((void*) LOG_TASK_SPACE_START,addressspace, 0x40000 ,7,3,tid, !ICACHE_ENABLE);
        // now since the task is mapped activate its virtual memory map by setting the pid
        volatile unint4 pid;
        GETPID(pid);

        SETPID(tid);

        Task* t = Task::deserialize((void*) ((unint4)meta + sizeof(MM_Header)),(unint2) (len - sizeof(MM_Header)),addressspace);

        if (!t) ERROR("Deserialization Error!");

        LOG(KERNEL,INFO,(KERNEL,INFO,"MigrationManager: Created new TaskCB/ThreadCBs."));
        LOG(KERNEL,INFO,(KERNEL,INFO,"MigrationManager: Task %d:" ,t->getId()));
        LOG(KERNEL,INFO,(KERNEL,INFO,"MigrationManager: start at 0x%x, end at 0x%x" , t->getTaskTable()->task_start_addr, t->getTaskTable()->task_heap_end));
        LOG(KERNEL,INFO,(KERNEL,INFO,"MigrationManager: entry at 0x%x, exit at 0x%x" , t->getTaskTable()->task_entry_addr, t->getTaskTable()->task_thread_exit_addr));
        LOG(KERNEL,INFO,(KERNEL,INFO,"MigrationManager: heap at 0x%x, size %d" , t->getTaskTable()->task_heap_start, (int) t->getTaskTable()->task_heap_end - (int) t->getTaskTable()->task_heap_start));

		// create initial thread of that task
		 new ThreadCfdCl( (void*) t->tasktable->task_entry_addr, (void*) t->tasktable->task_thread_exit_addr, t, t->getMemManager(),
		            DEFAULT_USER_STACK_SIZE, (void*) ( t->tasktable + 1 ), false );


        SETPID(pid);

        currentIncomingTask.task = t;
        MM_Reply reply;
        reply.header.id = currentIncomingTask.id;
        reply.header.type = MM_TYPE_REPLY;
        reply.reply_value = MM_REPLY_OK;
        listenSocket->sendto(&reply,sizeof(MM_Reply),sender);

    }

}

void MigrationManager::callbackFunc( void* param ) {

	// initialize socket!
    mysocketbuffer = (char*) theOS->getMemManager()->alloc( 8000 );
    listenSocket = new Socket( cIPv4AddressProtocol, SOCK_DGRAM, cUDP, mysocketbuffer, 8000 );

    // bind our socket to some address
    sockaddr* addr = (sockaddr*) theOS->getMemManager()->alloc( sizeof(sockaddr) );

    addr->port_data = 1; // listen on port 1
    addr->name_data[0] = '\0';
    addr->sa_data = 0; // address is not needed

    listenSocket->bind( addr );

	char* msgptr;
    sockaddr sender;

	LOG(KERNEL, INFO, (KERNEL, INFO, "MigrationManager: Started.."));

    while ( 1 ) {
        unint2 len = listenSocket->recvfrom( (ThreadCfdCl*)pCurrentRunningThread, &msgptr, MSG_WAIT, &sender );

        MM_Header* header = (MM_Header*) msgptr;
        switch ( header->type ) {
            case MM_TYPE_REQUEST:
                handleMigrationRequest( (MM_Request*) msgptr, &sender );
                break;
            case MM_TYPE_REPLY:
                handleMigrationReply( (MM_Reply*) msgptr, &sender );
                break;
            case MM_TYPE_TASKMETA:
                handleMigrationTaskMeta( (MM_TaskMeta*) msgptr, len, &sender );
                break;
            case MM_TYPE_PAYLOAD:
                handleMigrationPayload( (MM_Payload*) msgptr, len, &sender );
                break;
            case MM_TYPE_PAYLOAD | MM_TYPE_FINAL:
                handleMigrationPayload( (MM_Payload*) msgptr, len, &sender );
                break;
            default:
            	LOG(KERNEL, WARN, (KERNEL, WARN, "MigrationManager: Invalid Packet received!"));

        };

    }
}
