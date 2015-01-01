/*
 * FixedSizePBufList.cc
 *
 *  Created on: 15.02.2014
 *    Copyright &  Author: dbaldin
 */

#include <comm/FixedSizePBufList.hh>
#include "kernel/Kernel.hh"
#include "inc/memtools.hh"

#include "inc/stringtools.hh"
#include "inc/defines.h"

#include "lwip/pbuf.h"

extern Kernel* theOS;
extern "C" Mutex* comStackMutex;

FixedSizePBufList::FixedSizePBufList(size_t arg_size) {
    pbuf_list = new pbuf*[arg_size]; //(pbuf**) theOS->getMemoryManager()->alloc(arg_size * sizeof(pbuf*));
    addr_list = new sockaddr[arg_size]; //(sockaddr*) theOS->getMemoryManager()->alloc(arg_size * sizeof(sockaddr));

    head_id = 0;
    last_id = 0;

    if (pbuf_list == 0 || addr_list == 0) {
        size = 0;
        LOG(COMM, ERROR, "FixedSizePBufList() no memory for pbuf list or addr list");
        return;
    }

    for (unint i = 0; i < size; i++) {
        pbuf_list[i] = 0;
    }

    this->size = arg_size;
}

FixedSizePBufList::~FixedSizePBufList() {
    comStackMutex->acquire();
    for (size_t i = 0; i < size; i++) {
        if (pbuf_list[i] != 0) {
            pbuf_free(pbuf_list[i]);
        }
    }
    comStackMutex->release();

   delete pbuf_list;
   delete addr_list;
}

/*****************************************************************************
 * Method: FixedSizePBufList::addPbuf(pbuf* p, sockaddr* from)
 *
 * @description
 *
 *******************************************************************************/
int FixedSizePBufList::addPbuf(pbuf* p, sockaddr* from) {
    size_t last = last_id;

    last++;
    if (last >= size)
        last = 0;

    // all entries used?
    if (last == head_id) {
        return (cError );
    }

    LOG(COMM, TRACE, "FixedSizePBufList::addPbuf():last_id: %d, pbuf_list : %x, fromaddr: %x", last_id, pbuf_list, from);

    pbuf_ref(p);
    pbuf_list[last_id] = p;

    if (from != 0)
        memcpy(&addr_list[last_id], from, sizeof(sockaddr));
    else
        memset(&addr_list[last_id], 0, sizeof(sockaddr));

    last_id = last;

    return (cOk );
}

/*****************************************************************************
 * Method: FixedSizePBufList::getFirst(char* data, size_t len, sockaddr* from, pbuf* &pb)
 *
 * @description
 *
 * Tries to get the data of the first pbuf.
 * The data is copied into the memory area pointed by
 * data if it fits in there. the Pbuf is freed afterwards.
 * If the data area is too small an error is returned and the pbuf
 * stays inside the list.
 *******************************************************************************/
int FixedSizePBufList::getFirst(char* data, size_t len, sockaddr* from, pbuf* &pb) {
    if (!hasData())
        return (cNoData );

    pbuf* p = pbuf_list[head_id];
    if (p->len > len) {
        LOG(COMM, WARN, "FixedSizePBufList::getFirst(): packet too long to receive: %d > %d", p->len, len);
        return (cError );
    }

    memcpy(data, p->payload, p->len);
    if (from != 0)
        memcpy(from, &addr_list[head_id], sizeof(sockaddr));

    pb = p;

    pbuf_list[head_id] = 0;

    head_id++;
    if (head_id >= size)
        head_id = 0;

    return (p->len);
}

/*****************************************************************************
 * Method: FixedSizePBufList::hasData()
 *
 * @description
 *
 *******************************************************************************/
bool FixedSizePBufList::hasData() {
    return (pbuf_list[head_id] != 0);
}
