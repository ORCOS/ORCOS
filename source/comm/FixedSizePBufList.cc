/*
 * FixedSizePBufList.cc
 *
 *  Created on: 15.02.2014
 *      Author: dbaldin
 */

#include "FixedSizePBufList.h"
#include "kernel/Kernel.hh"
#include <memtools.hh>

#include "inc/stringtools.hh"
#include "inc/defines.h"

#include "lwip/pbuf.h"

extern Kernel* theOS;

FixedSizePBufList::FixedSizePBufList(int size) {

	pbuf_list = (pbuf**) theOS->getMemoryManager()->alloc(size * sizeof(pbuf*));
	addr_list = (sockaddr*) theOS->getMemoryManager()->alloc(size * sizeof(sockaddr));

	head_id = 0;
	last_id = 0;

	for (int i = 0; i < size;i++) {
		pbuf_list[i] = 0;
	}

	this->size = size;
}

FixedSizePBufList::~FixedSizePBufList() {
	theOS->getMemoryManager()->free(pbuf_list);
}


int FixedSizePBufList::addPbuf(pbuf* p, sockaddr* from) {

	size_t last = last_id;

	last++;
	if (last >= size) last = 0;

	// all entries used?
	if (last == head_id)
		return (cError);

	pbuf_ref(p);
	pbuf_list[last_id] = p;
	memcpy(&addr_list[last_id],from,sizeof(sockaddr));
	last_id = last;

	return (cOk);
}

/*
 * Tries to get the data of the first pbuf.
 * The data is copied into the memory area pointed by
 * data if it fits in there. the Pbuf is freed afterwards.
 * If the data area is too small an error is returned and the pbuf
 * stays inside the list.
 */
int FixedSizePBufList::getFirst(char* data, size_t len, sockaddr* from,  pbuf* &pb) {
	if (!hasData()) return (cError);

	pbuf* p = pbuf_list[head_id];
	if (p->len > len) {
		LOG( COMM, WARN,(COMM, WARN,"FixedSizePBufList::getFirst(): packet too long to receive: %d > %d %d",p->len,len));
		return (cError);
	}

	memcpy(data,p->payload,p->len);
	if (from != 0)
		memcpy(from,&addr_list[head_id],sizeof(sockaddr));

	pb = p;

	pbuf_list[head_id] = 0;

	head_id++;
	if (head_id >= size) head_id = 0;


	return (p->len);
}

bool FixedSizePBufList::hasData(){
	return (pbuf_list[head_id] != 0);
}
