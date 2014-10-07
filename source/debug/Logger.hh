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
class Logger;

#ifndef DEBUG_HH_
#define DEBUG_HH_

#include <logger_config.hh>

/*!
 *
 * \ingroup debug
 *
 * \brief Static framework for Logging
 *
 * Static Logging Framework. Logs could be send to serial line.
 * The logger could be configured in SCLConfig.xml.
 *
 * Possible loglevels from coarsest to finest:
 * FATAL, ERROR, WARN, INFO, DEBUG, TRACE
 *
 */
class Logger {

protected:

public:
    Logger() {
    }

    ~Logger() {
    }

    /*!
     * \brief Write msg to standard output.
     *
     * the printf is used, so %d .. could be used to as variables.
     * The prefix and level could be found in the logger_config.hh which is generated out of the SCLConfig.xml
     *
     */
    void log(Prefix prefix, Level level, const char* msg, ...);

    /* nothing to flush as everything is directly written to std out*/
    void flush() {};
};

#endif /*DEBUG_HH_*/
