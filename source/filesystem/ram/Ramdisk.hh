/*
 * Ramdisk.hh
 *
 *  Created on: 06.11.2015
 *      Author: Daniel
 */

#ifndef SOURCE_FILESYSTEM_RAM_RAMDISK_HH_
#define SOURCE_FILESYSTEM_RAM_RAMDISK_HH_

/*
 *
 */
class Ramdisk: public BlockDeviceDriver {
public:
    Ramdisk();
    virtual ~Ramdisk();
};

#endif /* SOURCE_FILESYSTEM_RAM_RAMDISK_HH_ */
