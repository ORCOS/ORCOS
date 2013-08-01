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

#ifndef ETHLI100_HH_
#define ETHLI100_HH_

#include <orcos.hh>
#include "TrainControlUnit.hh"
#include "LI100MessageQueue.hh"


class ETHLI100: public TrainControlUnit {
private:

	/*!
	 * @brief Message to be received.
	 */
	LI100Message message;

	/*!
	 * @brief The Resource ID of our communication Socket
	 */
	int4 mysockID;

	/*!
	 * @brief Are we connected? (1|0)
	 */
	unint1 connected;

	/*!
	 * @brief The currently pending Command
	 */
	LI100Message* pendingCommand;

	/*!
	 * @brief The amount of time (ms) the pending Command is pending.
	 */
	int pending_ms;

	/*!
	 * @brief Railway System state (ALL_ON, ALL_OFF)
	 */
	int state;

	/*!
	 * The message queue which keeps the reference to all messages to be sent
	 */
	LI100MessageQueue messageQueue;

	/*!
	 * @brief Tries to send a command to the railway system.
	 *
	 * If the command is a blocking command (one that needs a response from the system)
	 * the message may be queued if there is currently a pending Command.
	 *
	 * If the message is non-blocking the message will directly be sent (Warning: no overflow control)
	 * @params
	 * 		command : Pointer to the message to be send
	 * 		free_message : shall the message be free upon successfull send operation?
	 */
	int sendCommand(LI100Message* command, bool free_message);

	/*!
	 * @brief Internal Decoder Method to handle BC Controller Responses
	 */
	void handleBCResponse(char* response);

	/*!
	 * @brief Return the getStateofSwitch Message for the LI100 System for the switch with address "address"
	 */
	LI100Message* getStateOfSwitchMessage(int address);

	int sendCommandTrigger(LI100Message* message, bool free_message);

	/*!
	 * @brief Block Occupied Map.
	 */
	unint1 block_occupied[14];

	/*!
	 * @brief The Switches of the Railway System
	 */
	Switch switches[15];


public:

	/*!
	 * @brief Constructor. Takes the SocketID of the connected TCP connection to the
	 * LI100 Ethernet Module.
	 */
	ETHLI100(int mysock);

	/*!
	 * @brief Destructor
	 */
	~ETHLI100(){};

	/*!
	 * @brief Tries to initilize the connected railyway system.
	 *
	 * This will set all Switches to the SWITCH_LEFT mode and we will try to
	 * receive all block states. Needs to be called before proper operation with the railway system!
	 */
	void initialize();

	/*!
	 * @brief Return the State of the addresses block (BLOCK_FREE,BLOCK_OCCUPIED,BLOCK_UNKNOWN).
	 * @param address 	: valid values in [1..14]
	 * @returns int : [BLOCK_FREE,BLOCK_OCCUPIED,BLOCK_UNKNOWN]
	 */
	int getBlockState(int address) {
		if (address >= 1 && address <= 14)
			return block_occupied[address];
		else return BLOCK_UNKNOWN;
	}

	/*!
	* @brief Return the Switch of address @param address.
	* @param address 	: valid values in [0..14]
	* @returns The Switch structure of the addressed switch.
	*/
	Switch getSwitch(int address);


	/*!
	 * @brief Tries to receive a message from the LI100 Ethernet Module.
	 *
	 * Blocks the calling thread until a message is received.
	 */
	void receiveMessageBlocking( LI100Message* message );

	/*!
	 * @brief Tries to receive a message from the LI100 Control System.
	 *
	 * If we received a message the message is interpreted by the ETHLI100 Class
	 * and returned to the calling thread.
	 *
	 * @returns Pointer to the received message or NULL
	 */
	LI100Message* tryReceiveMessage();

	/*!
	 * @brief Prints the Contents of a LI100 Message (for debug purpose mainly)
	 */
	void printMessage(LI100Message* message);

	/*!
	 * @brief Prints the State of all Blocks to StdOut
	 */
	void printBlockStates();

	/*!
	 * @brief Prints the States of all Switches to StdOut
	 */
	void printSwichtStates();


	/*!
	 * @brief Handles a LI100 Message and updates internal structures.
	 *
	 * This can be any kind of message by the LI100 Control System
	 * e.g. Block State, System Broadcasts ..
	 * This method is automatically called by tryReceiveMessage().
	 */
	void handleMessage(LI100Message *message);

	/*!
	 * @brief Returns true if we have a pending command we are waiting on to be accepted by the LI100 Railway System.
	 */
	bool hasPendingCommand() { return (pendingCommand != 0);};

	/*!
	 * @brief Triggers the Message Queue.
	 *
	 * The system will try to send delayed messages if there is no pending Command.
	 */
	void trigger(unint4 milliseconds);

	/*!
	 * @brief Sends the ALL_ON Message to the Railway System.
	 *
	 * @returns int : 1 if Message has been send, 0 if Message has been queued
	 */
	int allOn();

	/*!
	 * @brief Sends the ALL_OFF Message to the Railway System thus stopping all trains and operations immediately.
	 *
	 * @returns int : 1 if Message has been send, 0 if Message has been queued
	 */
	int allOff();

	/*!
	 * @brief Stops all Trains immediately
	 *
	 * @returns int : 1 if Message has been send, 0 if Message has been queued
	 */
	int stopAllTrains();

	/*!
	 * @brief Stops the Train with ID "address"
	 *
	 * @returns int : 1 if Message has been send, 0 if Message has been queued
	 */
	int stopTrain(unint4 address);

	/*!
	 * @brief Set the Speed and Direction of the Train with ID "address"
	 *
	 * @returns int : 1 if Message has been send, 0 if Message has been queued
	 */
	int setTrainSpeed(unint4 address, unint4 speed, unint4 direction);

	/*!
	 * @brief Sets the Switch state of the Switch with ID "address".
	 *
	 * @param dir	: SWITCH_TURN or SWITCH_STRAIGHT
	 *
	 * @returns int : 1 if Message has been send, 0 if Message has been queued
	 */
	int setSwitch( unint4 address, unint4 dir );

	/*!
	 * @brief Disables the Voltage of a Switch
	 *
	 * @returns int : 1 if Message has been send, 0 if Message has been queued
	 */
	int disableSwitch( unint4 address, unint4 dir );
};

#endif
