/*
 * LI100MessageQueue.cc
 *
 *  Created on: 12.07.2011
 *      Author: digitrain
 */


#include "LI100MessageQueue.hh"

void LI100MessageQueue::trigger(unint4 milliseconds) {
	// parse waiting queue
	LI100MessageQueueEntry *cur_entry = first;

	while(cur_entry != 0) {
		cur_entry->delay -= milliseconds;
		cur_entry = cur_entry->next;
	}

}

int LI100MessageQueue::addMessage(LI100Message *msg, unint4 delay){
	LI100MessageQueueEntry *newEntry =  (LI100MessageQueueEntry*) malloc(sizeof(LI100MessageQueueEntry));
	if (newEntry == 0) {
			printf("addMessage:: malloc: out of memory!");
			while (true) {};
		}

	newEntry->delay = delay;
	newEntry->msg_ptr = msg;
	newEntry->next = 0;
	newEntry->prev = 0;

	// add to waiting queue
	if (first == 0) {
		first = newEntry;
		last = first;
	} else
	{
		newEntry->prev = last;
		last->next = newEntry;
		last = newEntry;
	}

	return 1;
}


void LI100MessageQueue::removeEntry(LI100MessageQueueEntry *entry){

	/*LI100MessageQueueEntry *entry = first;

	while (entry != 0 && entry->delay > 0) {
		entry = entry->next;
	}*/

	if (entry != 0) {
		if (entry->prev != 0) {
			entry->prev->next = entry->next;
		}

		if (entry->next != 0) {
			entry->next->prev = entry->prev;
		}

		if (entry == first) first = entry->next;
		if (entry == last) last = entry->prev;

		free(entry->msg_ptr);
		free(entry);

	}

}

