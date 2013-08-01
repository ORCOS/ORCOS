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

#ifndef TRAIN_HH_
#define TRAIN_HH_

#include <orcos.hh>
#include "TrainControlUnit.hh"

/*!
 * @brief Class used for controlling a Train inside the Railway System.
 *
 * This class can be used as an abstraction of a Train inside the Railway System.
 * It covers all functionalities to control a train by its address.
 *
 * This class does not update the Position of a train. The corresponding position determination
 * algorithm needs to be implemented by the user program which then can update the stored position inside this object.
 */
class Train {
private:
	/*!
	 * @brief My Train address [1..3]
	 */
	unint1 address;

	/*!
	 * @brief The currently set speed of the train
	 */
	unint1 speed;

	/*!
	 * @brief The currently set direction of travel
	 */
	unint1 direction;

	/*!
	 * @brief Reference to the TrainControlUnit for communication
	 */
	TrainControlUnit* tcu;

	/*!
	 *@ brief  The block the train is currently standing on, which needs to be updated by the user program!
	 */
	unint1 position;

public:
	/*!
	 * @brief Constructor
	 *
	 * @params
	 * 		tcu		: Pointer to the TrainControlUnit for communication with the Railway System
	 * 		address : The address of this Train
	 */
	Train(TrainControlUnit* tcu, unint1 address);

	/*!
	 * @brief Returns the Train address
	 * @returns unsigned int : address of this Train
	 */
	unint1 getAddress() { return address; };

	/*!
	 * @brief Sets the Speed of the Train [0..30]
	 */
	void setSpeed(unint1 speed);


	/*!
	 * @brief Returns the currently set speed of the Train
	 * @returns unsigned int : currently set speed [0..30]
	 */
	unint1 getSpeed();

	/*!
	 * @brief Set the direction of travel
	 * @params
	 * 		direction : 	1: forward (default), 0: backwards
	 */
	void setDirection(unint1 direction);

	/*!
	 * @brief Returns the currently set direction of travel.
	 */
	unint1 getDirection();

	/*!
	 * @brief Stops the train.
	 */
	void stop();

	void enableFunction(unint4 function);

	void disableFunction(unint4 function);

	/*!
	 *@brief Returns the stored position inside this object.
	 *
	 * The position update needs to be done by the user program and forwarded to this object by
	 * the Train::setPosition method.
	 *
	 * @returns unsigned int : Block Number {BLOCK1,BLOCK2,..,BLOCK13,NOBLOCK}
	 */
	unint1 getPosition();

	/*!
	 * @brief Sets the current position of this Train
	 *
	 * Sets the position stored by this object. Needs to be a number out of
	 * {BLOCK1,BLOCK2,..,BLOCK13,NOBLOCK}. The user has to call this method whenever
	 * the position of a train changed.
	 */
	void setPosition(unint1 new_position);

};



#endif
