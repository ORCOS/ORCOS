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

#ifndef TRAINCONTROLUNIT_HH
#define TRAINCONTROLUNIT_HH


/*---------------------------------------------------
 * DEFINES
 ----------------------------------------------------*/

#define COMMAND_SOFTWAREVERSION \
	{ (char) 0x03 ,(char) 0x21, (char) 0x21, (char) 0x0 }
#define COMMAND_STATUS \
	{ (char) 0x03 ,(char) 0x21, (char) 0x24, (char) 0x5 }
#define COMMAND_STOP_ALL_TRAINS \
	{ (char) 0x02 ,(char) 0x80, (char) 0x80 }
#define COMMAND_ALL_ON \
	{ (char) 0x03 ,(char) 0x21 ,(char) 0x81, (char) 0xA0 }
#define COMMAND_ALL_OFF \
	{ (char) 0x03 ,(char) 0x21 ,(char) 0x80, (char) 0xA1 }
#define COMMAND_STOP_TRAIN(a) \
	{ (char) 0x03 ,(char) 0x91, (char) a, (char) (0x91 ^ a) }
#define COMMAND_SET_TRAIN_SPEED(a) \
	{ (char) 0x06 ,(char) 0xB4, (char) a }
#define COMMAND_SET_SWITCH(a) \
	{ (char) 0x04 ,(char) 0x52, (char) a }
#define COMMAND_GET_MODULE(a) \
	{ (char) 0x04 ,(char) 0x42, (char) a }


#define BLOCK1 				1
#define BLOCK2 				2
#define BLOCK3 				3
#define BLOCK4 				4
#define BLOCK5 				5
#define BLOCK6 				6
#define BLOCK7 				7
#define BLOCK8 				8
#define BLOCK9 				9
#define BLOCK10 			10
#define BLOCK11 			11
#define BLOCK12 			12
#define BLOCK13 			13
#define NOBLOCK 			-1

// SWITCH States
#define SWITCH_TURN 		0
#define SWITCH_STRAIGHT 	1
#define SWITCH_UNKNOWN 		2

// BAHN States
#define ALL_ON   			1
#define ALL_OFF  			0

// BLOCK States
#define BLOCK_FREE 			0
#define BLOCK_OCCUPIED 		1
#define BLOCK_UNKNOWN 		2

/*---------------------------------------------------
 * TYPEDEF
 ----------------------------------------------------*/

/*!
 * @brief Abstraction of the Railway Switch structure.
 *
 * This structure contains all information on a specific switch in the railway system
 */
typedef struct {
	/*!
	 * @brief The Switch address [1..14]
	 */
	int1 switchid;
	/*!
	 * @brief The state of the switch [SWITCH_TURN,SWITCH_STRAIGHT,SWITCH_UNKNOWN]
	 */
	int1 state;
	/*!
	 * @brief Switch Voltage active? [1,0]
	 */
	int1 activated;
} Switch;

/*---------------------------------------------------
 * VARIABLES
 ----------------------------------------------------*/


// Map for translation between block IDs and BLOCK LI100 Control System addresses
static unint1 BC_OCCUPIED65_IDMAP[] = {BLOCK10,BLOCK2,BLOCK3,BLOCK1,BLOCK11,BLOCK4,BLOCK12,NOBLOCK};
static unint1 BC_OCCUPIED66_IDMAP[] = {BLOCK6,BLOCK5,BLOCK7,BLOCK8,BLOCK13,BLOCK9,NOBLOCK,NOBLOCK};



class TrainControlUnit {

public:
	TrainControlUnit() {

	};

	~TrainControlUnit() {

	};

	virtual int allOn() {};
	virtual int allOff() {};
	virtual int stopAllTrains() {};
	virtual int stopTrain(unint4 address) {};
	virtual int setTrainSpeed(unint4 address, unint4 speed, unint4 direction) {};
	virtual int setSwitch( unint4 address, unint4 dir ) {} ;
};

#endif
