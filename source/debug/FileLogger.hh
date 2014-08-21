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
class FileLogger;

#ifndef FILELOGGER_HH_
#define FILELOGGER_HH_

#include <logger_config.hh>
#include "filesystem/File.hh"

class FileLogger {

private:
    File* logFile;

public:
    FileLogger();

    ~FileLogger() {
    }

    /*!
     * \brief Write msg to serial line.
     *
     * thprintf is used, so %d .. could be used to as variables.
     * The prefix and level could be found in the logger_config.hh which is generated out of the SCLConfig.xml
     *
     */
    void log(Prefix prefix, Level level, const char* msg, ...);


    /*!
     * Flushes the log content to the file
     */
    void flush();
};

#endif /*FILELOGGER_HH_*/
