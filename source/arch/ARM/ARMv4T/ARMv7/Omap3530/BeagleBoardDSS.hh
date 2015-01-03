/*
 * BeagleBoardDSS.hh
 *
 *  Created on: 07.08.2012
 *    Copyright & Author: danielb
 */

#ifndef BEAGLEBOARDDSS_HH_
#define BEAGLEBOARDDSS_HH_

#include <types.hh>
#include <hal/CharacterDevice.hh>
#include "filesystem/SharedMemResource.hh"

// CharacterDevice Driver for support as the standard output device
// console
class BeagleBoardDSS: public CharacterDevice {
private:
    SharedMemResource* framebuffer;

public:
    explicit BeagleBoardDSS(T_BeagleBoardDSS_Init *init);

    ~BeagleBoardDSS();

    /*****************************************************************************
     * Method: init()
     *
     * @description
     *******************************************************************************/
    void init();

    /*****************************************************************************
     * Method: writeByte(char byte)
     *
     * @description
     *******************************************************************************/
    ErrorT writeByte(char byte);

    /*****************************************************************************
     * Method: writeBytes(const char *bytes, unint4 length)
     *
     * @description
     *******************************************************************************/
    ErrorT writeBytes(const char *bytes, unint4 length);
};

#endif /* BEAGLEBOARDDSS_HH_ */
