/*
 * LI100MessageQueue.hh
 *
 *  Created on: 12.07.2011
 *      Author: digitrain
 */

#ifndef LI100MESSAGEQUEUE_HH_
#define LI100MESSAGEQUEUE_HH_

#include <orcos.hh>

/*!
 * @brief LI100 Message which is sent to  and received from the Lenz LI100 Control Module.
 */
typedef struct LI100Message{
	/*!
	 * @brief total lenght of this message in the data array.
	 */
	char length;

	/*!
	 * @brief the data (maximum length 16 bytes)
	 */
	char data[16];

	/*!
	 * @brief is the crc valid?
	 */
	char crcValid;

	/*!
	 * @brief For outgoing messages only: do we need to wait for an ACK?
	 */
	bool isBlocking;

	/*!
	 * @brief current ms counter for message send queue.
	 */
	int  ms;
} LI100Message;

typedef struct LI100MessageQueueEntry {
	LI100Message* msg_ptr;
	int4	delay;
	LI100MessageQueueEntry	*prev;
	LI100MessageQueueEntry	*next;
} LI100MessageQueueEntry;


/*!
 * @brief A Message Queue for LI100 Messages.
 *
 * In order to avoid overflows of the LI100 Lenz Controller and thus System Shutdowns we need
 * to queue message if the Controller is not ready to receive new commands yet. This can be done by this Class.
 */
class LI100MessageQueue {
public:
	LI100MessageQueueEntry *first;
	LI100MessageQueueEntry *last;

	void trigger(unint4 milliseconds);
	int addMessage(LI100Message *msg, unint4 delay);
	void removeEntry(LI100MessageQueueEntry *entry);

	LI100MessageQueue() {first = 0; last = 0;};
	~LI100MessageQueue() {};
};


#endif /* LI100MESSAGEQUEUE_HH_ */
