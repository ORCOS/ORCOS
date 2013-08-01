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

#ifndef Leon3CLOCK_HH_
#define Leon3CLOCK_HH_

#include <hal/Clock.hh>

/*! \brief Leon3 Clock, Implementation of HAL Clock
 */
class Leon3Clock : public Clock
{
public:
	Leon3Clock(const char* name);
	~Leon3Clock();
	
	/*!
	 * \brief return the time since the system startup in us (microseconds)
	 */
	unint8 getTimeSinceStartup();
	
	/*!
	 * \brief fills a given TimeStruct with the time since system startup
	 * 
	 * the given TimeStruct is populated with the time since system startup
	 * the time is broken down to the various data types. so if for example
	 * exactly 1 year has passed only the year field of the struct will be 1
	 * and all other fields will contain 0.
	 */
	bool getTimeSinceStartup(TimeStruct* ts);
	
	/*!
	 * \brief Resets the time base registers.
	 */
	void reset();
	
	/*
	 * updates total time on timer overflow
	 */
	void updateTotalTime();
	
private:
	
	/*
	 * time in microseconds
	*/
	unint8 totalTime;
	unsigned int baseAddr;
};

#endif /*Leon3CLOCK_HH_*/
