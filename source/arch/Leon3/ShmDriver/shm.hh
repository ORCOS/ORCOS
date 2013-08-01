/*
	ORCOS - an Organic Reconfigurable Operating System
	Copyright (C) 2008 University of Paderborn

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SHM_HH
#define SHM_HH

#define MSG_BUFFER_SIZE	(1560)

enum nodeStatus {
	initPending=1,
	initComplete=2,
	nodeActive=4 };

/*
 * NodeInfo holds information about the status of a node
 * and how to raise an interrupt on a node to inform it about
 * new message.
 */
typedef struct {
	nodeStatus 	status;
	int 		irqAddress;	// the address for causing an interrupt
	int 		irqValue;	// the value which causes an interrupt
} NodeInfo;


typedef struct {
	int	 	next;
	int 	queue;
	int 	index;
	int		data;
	int		length;
} Descriptor;


typedef struct {
	int lock;
	int front;
	int rear;
	int owner;
} LockedQueueControl;

#endif /* SHM_HH */
