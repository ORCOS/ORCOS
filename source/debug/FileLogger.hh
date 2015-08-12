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

/*
 * This class provides kernel logging capabilities to file.
 *
 * Primary file location  : /mnt/ROOT/Kernel.log
 * If ROOT has not been mounted:
 * Secondary file location: /mnt/ramdisk/Kernel.log
 */
class FileLogger {
private:
    File*   logFile;

    /* current initialization status.
     * output file may not be available until initialized. */
    bool    initialized;

    /*****************************************************************************
     * Method: init()
     *
     * @description
     *  Tries to initialize the file logger.
     *******************************************************************************/
    int    init();

public:
    FileLogger();

    ~FileLogger() {
    }

    /*****************************************************************************
     * Method: log(Prefix prefix, Level level, const char* msg, ...)
     *
     * @description
     *  The prefix and level could be found in the logger_config.hh
     *  which is generated out of the SCLConfig.xml
     *******************************************************************************/
    void log(Prefix prefix, Level level, const char* msg, ...);


    /*!
     * Flushes the log content to the file
     */
    /*****************************************************************************
     * Method: flush()
     *
     * @description
     *  Flushes the log content to the file
     *******************************************************************************/
    void flush();
};

#endif /*FILELOGGER_HH_*/
