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

#include "ETHLI100.hh"

Switch UnknownSwitch = {255,SWITCH_UNKNOWN,0};

ETHLI100::ETHLI100(int mysock){
/*---------------------------------------------------------------------------*/
	this->mysockID = mysock;
	if (this->mysockID != cInvalidResource){
		connected = 1;
	}else{
		connected = 0;
	}

	pendingCommand = 0;

	for (int i=0; i < 14; i++) {
		switches[i].switchid = i;
		switches[i].state = SWITCH_UNKNOWN;
		switches[i].activated = 0;
	}

	for (int i=0; i < 14; i++) {
		block_occupied[i] = BLOCK_UNKNOWN;
	}

	state = ALL_OFF;
}

Switch ETHLI100::getSwitch(int address) {
		if (address >= 0 && address <= 14)
			return switches[address];
		else return UnknownSwitch;
	}

LI100Message *ETHLI100::getStateOfSwitchMessage(int address) {
	LI100Message *getSwitch = (LI100Message*) malloc(sizeof(LI100Message));
	getSwitch->length = 0x4;
	getSwitch->data[0] = 0x42;
	getSwitch->data[1] = (char) address / 4;

	int nibble = (address % 4) / 2;
	getSwitch->data[2] = 0x80 | nibble;
	getSwitch->data[3] = 0x0;

	// build CRC
	for (int i = 0; i < 3; i++){
		getSwitch->data[3] ^= getSwitch->data[i];
	}
	getSwitch->isBlocking = false;

	return getSwitch;
}


void ETHLI100::initialize() {

	LI100Message message;

	// send the ON signal
	allOn();

	// wait until the LI100 Control sends the ALL On message
	// to be sure the system is ready
	do {
		this->receiveMessageBlocking(&message);
		this->printMessage(&message);

	} while (state != ALL_ON);


	while (pendingCommand) {
			this->receiveMessageBlocking(&message);
			this->printMessage(&message);
		};

	printf("LI100: Initializing all Switches to State SWITCH_TURN\n");

	// check the state of the switches
	for (int address=0; address < 14; address++) {

		int err;

		printf("Setting switch : %d\n", address);
		err = setSwitch(address+1, (unint1) SWITCH_TURN);
		sleep(100);
		// try to receive the answer to this command
		do {
			this->receiveMessageBlocking(&message);
			this->printMessage(&message);
		} while (pendingCommand );
		// trigger next waiting message to be send
		trigger(0);
		switches[address].state = SWITCH_TURN;
	}

	// wait until all commands are accepted to avaoid overload condition
	/*do {
		this->receiveMessageBlocking(&message);
		this->printMessage(&message);
	} while (pendingCommand );*/

	printf("LI100: Switch Initialization SUCCESSFULL\r\n");
	this->printSwichtStates();

	// check the state of the block
	printf("LI100: Reading Block State...\r\n");

	for (int address=0; address < 14; address+=4) {

		int blockgroup = 64 + (address / 8);

		LI100Message getBlock = COMMAND_GET_MODULE(blockgroup);
		int nibble = ((address % 8) / 4);

		getBlock.data[2] = 0x80 | nibble;
		// build CRC
		for (int i = 0; i < 3; i++){
			getBlock.data[3] ^= getBlock.data[i];
		}
		getBlock.isBlocking = false;

		this->sendCommand( &getBlock , false);
		// Now wait until we have the state of the switches
		// TODO implement a timeout with resend if we dont get a response!
		bool initialized = false;

		do {
			this->receiveMessageBlocking(&message);
			this->printMessage(&message);

			if ((block_occupied[BC_OCCUPIED65_IDMAP[address]] != BLOCK_UNKNOWN) &
				(block_occupied[BC_OCCUPIED65_IDMAP[address+1]] != BLOCK_UNKNOWN) &
				(block_occupied[BC_OCCUPIED65_IDMAP[address+2]] != BLOCK_UNKNOWN) &
				(block_occupied[BC_OCCUPIED65_IDMAP[address+3]] != BLOCK_UNKNOWN)) initialized = true;

		} while (!initialized);

	}

	printf("LI100: Block Initialization SUCCESSFULL\r\n");
	this->printBlockStates();

}

LI100Message idle_Message;

void ETHLI100::trigger(unint4 milliseconds){
	messageQueue.trigger(milliseconds);

	this->pending_ms += milliseconds;
	// TIMEOUT
	if (this->pending_ms > 2000) {
		this->pendingCommand = 0;
		this->pending_ms = 0;
	}

	LI100MessageQueueEntry* msg = messageQueue.first;

	while (msg != 0) {

		if (msg->delay <= 0) {

			int err = this->sendCommandTrigger(msg->msg_ptr, false);
			if (err == 1) {
				messageQueue.removeEntry(msg);
				// successfully send
				return;
			} else msg = msg->next;


		} else msg = msg->next;
	}


}

LI100Message* ETHLI100::tryReceiveMessage() {
	char c;
	char crc;

	char* data;

	int num = recv(this->mysockID,&data,MSG_PEEK);

	int i = 0;
	while (i < num) {
		int len = (data[i] & 0xf) + 2;

		memcpy(&message.data[0],&data[i],len);
		message.length = len;

		crc = 0;
		for (int i2 = 0; i2 < message.length-1; i2++){
			crc = crc ^ message.data[i2];
		}

		if (crc == message.data[message.length-1]) message.crcValid = true;
		else message.crcValid = false;

		i+= len;

		//this->printMessage(&message);
		this->handleMessage(&message);
	}

}

/*---------------------------------------------------------------------------*/
void ETHLI100::receiveMessageBlocking( LI100Message* message ){

	char* data;
	int num = recv(this->mysockID,&data,MSG_WAIT);
	int crc;

	int i = 0;
	while (i < num) {
		int len = (data[i] & 0xf) + 2;

		memcpy(&message->data[0],&data[i],len);
		message->length = len;

		crc = 0;
		for (int i2 = 0; i2 < message->length-1; i2++){
			crc = crc ^ message->data[i2];
		}

		if (crc == message->data[message->length-1]) message->crcValid = true;
		else message->crcValid = false;

		i+= len;
		//this->printMessage(message);
		this->handleMessage(message);
	}

}

/*---------------------------------------------------------------------------*/
void ETHLI100::printMessage(LI100Message* message){
/*---------------------------------------------------------------------------*/
		int crc = 0;

		printf("L100Message (%d Bytes): ", message->length);
		for (int i = 0; i < message->length-1; i++){
			printf("0x%x ", message->data[i]);
			crc = crc ^ message->data[i];
		}
		printf("0x%x ", message->data[message->length-1]);

		if (crc == message->data[message->length-1]) message->crcValid = true;
		else message->crcValid = false;

		if (message->crcValid){
			printf(" [Valid]");
		} else {
			printf(" [Bad]");
		}
		printf("\r\n");

}

/*---------------------------------------------------------------------------*/
int ETHLI100::sendCommandTrigger(LI100Message* message, bool free_message){
/*---------------------------------------------------------------------------*/
	if (message->isBlocking) {
		if (pendingCommand != 0) {
			// just stop since the message is already in the message queue
			return 0;
		}

		pendingCommand = message;
		pending_ms = 0;

		sendto(mysockID,&message->data,message->length,0);
		if (free_message) free(message);
		return 1;

	}
	else {

		sendto(mysockID,&message->data,message->length,0);
		if (free_message) free(message);
		return 1;
	}
}

/*---------------------------------------------------------------------------*/
int ETHLI100::sendCommand(LI100Message* message, bool free_message){
/*---------------------------------------------------------------------------*/
	if (message->isBlocking) {
		if (pendingCommand != 0) {
			// enqueue the message
			this->messageQueue.addMessage(message,0);
			return 0;
		}

		pendingCommand = message;
		pending_ms = 0;

		sendto(mysockID,&message->data,message->length,0);
		if (free_message) free(message);
		return 1;

	}
	else {

		sendto(mysockID,&message->data,message->length,0);
		if (free_message) free(message);
		return 1;
	}
}

/*---------------------------------------------------------------------------*/
void ETHLI100::printBlockStates() {
	/*---------------------------------------------------------------------------*/
	printf(" B1  | B2  | B3  | B4  | B5  | B6  | B7  | B8  | B9  | B10 | B11 | B12 | B13\r\n");
	for (int i = 1; i < 14; i++) {
		printf(" %2d  |",block_occupied[i]);
	}
	printf("\r\n");
}

void ETHLI100::printSwichtStates() {
	/*---------------------------------------------------------------------------*/
	printf(" S0  | S1  | S2  | S3  | S4  | S5  | S6  | S7  | S8  | S9  | S10 | S11 | S12 | S13\r\n");
	for (int i = 0; i < 14; i++) {
		printf(" %2d  |",switches[i].state);
	}
	printf("\r\n");

}


void ETHLI100::handleBCResponse(char* response) {
	unint1 nibble = (response[1] & 0x10) >> 4;


	if ((response[0] >= 0) & (response[0] <= 4)) {
		// Handle the Switch State Message
		nibble = nibble << 1;
		int switchgroup = response[0] * 4;

		int state = (response[1] & 0x3);
		int switchid = switchgroup+nibble;
		if (switchid < 14) {
			if (state == 1) {
				switches[switchid].state = SWITCH_TURN;
			} else
			if (state == 2) {
				switches[switchid].state = SWITCH_STRAIGHT;
			} else {
				printf("ERROR in Switch %d: state: %d!\r\n",switchid,state);
				return;
			}
		}

		state = (response[1] & 0xc) >> 2;
		switchid = switchgroup+nibble+1;
		if (switchid < 14 ) {
			if (state == 1) {
				switches[switchid].state = SWITCH_TURN;
			} else
			if (state == 2) {
				switches[switchid].state = SWITCH_STRAIGHT;
			} else {
				printf("ERROR in Switch %d: state: %d!\r\n",switchid,state);
			}
		}
	} else
	{
		switch (response[0]) {

		case 64:
			// Block Rückmelder
			for (int i=0; i < 4; i++) {
			 int id = i + (nibble*4);
			 int value = (response[1] & (1 << i)) >> i;
			 block_occupied[BC_OCCUPIED65_IDMAP[id]] = value;
			}
			//printBlockStates();
			break;
		case 65:
			// Block Rückmelder
			for (int i=0; i < 4; i++) {
			 int id = i + (nibble*4);
			 int value = (response[1] & (1 << i)) >> i;
			 block_occupied[BC_OCCUPIED66_IDMAP[id]] = value;
			}
			//printBlockStates();
			break;
		default:
			printf("Unhandled Message from LI100\r\n");
			break;
		}
	}

}

/*---------------------------------------------------------------------------*/
void ETHLI100::handleMessage(LI100Message *message) {
/*---------------------------------------------------------------------------*/
	if (message == 0) return;
	//printMessage(message);

	if (message->crcValid == false) {
		printf("Invalid CRC! \r\n");
		printMessage(message);
		allOff();
		return;
	}

	// message header must be 0x42 for a BC "Rückmeldung"
	if ( (message->data[0] >= 0x40) && (message->data[0] <= 0x60)) {

		int num_bytes = message->data[0] - 0x40;
		int pos = 1;
		while (num_bytes > 0) {
			handleBCResponse(&message->data[pos]);
			pos +=2;
			num_bytes -= 2;
		}

	} else
	if ((message->data[0] == 0x1) & (message->data[1] == 0x4)) {
		// ACK Received
		// Command send to Central Control Unit
		pendingCommand = 0;
	} else
	if ((message->data[0] == 0x1) & (message->data[1] == 0x1)) {
		// Communication Failure! Fatal System Shutdown
		printf("Communication Failed!r\n");
		allOff();
	} else
	if ((message->data[0] == 0x61) & (message->data[1] == 0x0)) {
		//printf("Railway power OFF \r\n");
		state = ALL_OFF;
	} else
	if ((message->data[0] == 0x61) & (message->data[1] == 0x1)) {
		//printf("Railway power ON \r\n");
		state = ALL_ON;
	} else
	if ((message->data[0] == 0x81) & (message->data[1] == 0x0)) {
		printf("Railway Trains Power OFF \r\n");
	} else
	if ((message->data[0] == 0x61) & (message->data[1] == 0x81)) {

		if (pendingCommand != 0) {
			//printf("Busy! Resending Command!\r\n");
			// Now set to non-blocking to force the message to be resent
			pendingCommand->isBlocking = false;
			sendCommand(pendingCommand,false);
		}
	}	else {
		printMessage(message);
	}

}


/*---------------------------------------------------------------------------*/
int ETHLI100::allOn(){
/*---------------------------------------------------------------------------*/
	LI100Message *allOn = (LI100Message*) malloc(sizeof(LI100Message));
	allOn->length = 3;
	//(char) 0x21 ,(char) 0x81, (char) 0xA0
	allOn->data[0] = 0x21;
	allOn->data[1] = 0x81;
	allOn->data[2] = 0xA0;
	allOn->isBlocking = true;
	return sendCommand(allOn, true);
}

/*---------------------------------------------------------------------------*/
int ETHLI100::allOff(){
/*---------------------------------------------------------------------------*/
	LI100Message *allOff = (LI100Message*) malloc(sizeof(LI100Message));
	allOff->length = 3;
	//(char) 0x21 ,(char) 0x80, (char) 0xA1
	allOff->data[0] = 0x21;
	allOff->data[1] = 0x80;
	allOff->data[2] = 0xA1;
	allOff->isBlocking = false;
	return sendCommand(allOff, true);
}

/*---------------------------------------------------------------------------*/
int ETHLI100::stopAllTrains(){
/*---------------------------------------------------------------------------*/

	LI100Message *stopAllTrains = (LI100Message*) malloc(sizeof(LI100Message));
	stopAllTrains->length = 2;
	stopAllTrains->data[0] = 0x80;
	stopAllTrains->data[1] = 0x80;
	stopAllTrains->isBlocking = false;
	return sendCommand(stopAllTrains, true);
}

/*---------------------------------------------------------------------------*/
int ETHLI100::stopTrain(unint4 address){
/*---------------------------------------------------------------------------*/
	LI100Message *stopTrain = (LI100Message*) malloc(sizeof(LI100Message));
	stopTrain->length = 4;
	stopTrain->data[0] = 0x92;
	stopTrain->data[1] = 0x0;
	stopTrain->data[2] = address;
	stopTrain->data[3] = 0x92 ^ address;

	stopTrain->isBlocking = true;
	return sendCommand(stopTrain, true);

}

/*---------------------------------------------------------------------------*/
int ETHLI100::setTrainSpeed(unint4 address, unint4 speed, unint4 direction = 1){
/*---------------------------------------------------------------------------*/
	unint4 encodedSpeed = 0;
	LI100Message *setTrainSpeed = (LI100Message*) malloc(sizeof(LI100Message));
	setTrainSpeed->length = 0x6;
	setTrainSpeed->data[0] = 0xB4;
	setTrainSpeed->data[1] = (char) address;

	if (speed != 0){
		encodedSpeed = ((speed + 3) & 0x01) << 4;
		encodedSpeed |= (speed + 3) >> 1;
	}
	setTrainSpeed->data[2] = encodedSpeed | (direction << 6);
	setTrainSpeed->data[3] = 0;
	setTrainSpeed->data[4] = 0x02;
	setTrainSpeed->data[5] = 0;

	for (int i = 0; i < 5; i++){
		setTrainSpeed->data[5] ^= setTrainSpeed->data[i];
	}
	setTrainSpeed->isBlocking = true;

	return sendCommand(setTrainSpeed, true);
}

int ETHLI100::disableSwitch( unint4 address, unint4 dir ){
/*---------------------------------------------------------------------------*/

	//printf("Disabling Switch %d\r\n",address);

	LI100Message setSwitch = COMMAND_SET_SWITCH(address / 4);
	unint4 encodedDir = (address % 4) << 1;
	encodedDir = encodedDir | (dir & 0x1);
	setSwitch.data[2] = 0x80 | encodedDir;

	setSwitch.data[3] = 0;

	for (int i = 0; i < 3; i++){
		setSwitch.data[3] ^= setSwitch.data[i];
	}

	setSwitch.isBlocking = true;

	LI100Message *disableSwitch = (LI100Message*) malloc(sizeof(LI100Message));
	memcpy(disableSwitch,&setSwitch,sizeof(LI100Message));
	switches[address].activated = 0;
	return sendCommand(disableSwitch, true);

}

/*---------------------------------------------------------------------------*/
int ETHLI100::setSwitch( unint4 address, unint4 dir ){
/*---------------------------------------------------------------------------*/
	address = address-1;

	if (address > 13) return 0;

	if (switches[address].state == dir)  {
		//printf("No need to set switch %d to %d\r\n",address,dir);
		return 1;
	}

	LI100Message *setSwitch = (LI100Message*) malloc(sizeof(LI100Message));
	if (setSwitch == 0) {
		printf("malloc: out of memory!");
		while (true) {};
	}

	setSwitch->length = 0x04;
	setSwitch->data[0] = 0x52;
	setSwitch->data[1] = (char) (address / 4);

	unint4 encodedDir = (address % 4) << 1;
	encodedDir = encodedDir | (dir & 0x1);
	setSwitch->data[2] = 0x80 | encodedDir | 0x8;

	setSwitch->data[3] = 0;

	for (int i = 0; i < 3; i++){
		setSwitch->data[3] ^= setSwitch->data[i];
	}

	setSwitch->isBlocking = true;
	switches[address].state = dir;
	switches[address].activated = 1;
	return sendCommand(setSwitch, true);

}
