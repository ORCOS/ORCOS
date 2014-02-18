/*
 * FixedSizePBufList.h
 *
 *  Created on: 15.02.2014
 *      Author: dbaldin
 */

#ifndef FIXEDSIZEPBUFLIST_H_
#define FIXEDSIZEPBUFLIST_H_

#include "inc/types.hh"
#include "lwip/pbuf.h"

class FixedSizePBufList {

	pbuf** pbuf_list;

	sockaddr* addr_list;

	size_t head_id;

	size_t last_id;

	size_t size;
public:
	FixedSizePBufList(int size);

	int addPbuf(pbuf* p, sockaddr* from);

	/*
	 * Tries to get the data of the first pbuf.
	 * The data is copied into the memory area pointed by
	 * data if it fits in there. the Pbuf is freed afterwards.
	 * If the data area is too small an error is returned and the pbuf
	 * stays inside the list.
	 *
	 * Returns: the number of bytes received
	 * 			On error: errorcode < 0
	 *
	 */
	int getFirst(char* data, size_t len, sockaddr* from,  pbuf* &pb);

	bool hasData();

	virtual ~FixedSizePBufList();
};

#endif /* FIXEDSIZEPBUFLIST_H_ */
