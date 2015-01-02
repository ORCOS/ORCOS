/*
 * OmapIOMux.hh
 *
 *  Created on: 21.12.2014
 *      Copyright & Author: Daniel Baldin
 */

#ifndef SOURCE_ARCH_ARM_ARMV4T_ARMV7_OMAP3530_OMAPIOMUX_HH_
#define SOURCE_ARCH_ARM_ARMV4T_ARMV7_OMAP3530_OMAPIOMUX_HH_

#include <hal/CharacterDevice.hh>

/*
 * IO Muxing support for Omap Devices Chips.
 */
class OmapIOMux: public CharacterDevice {
public:
    OmapIOMux();
    ~OmapIOMux();

    /*****************************************************************************
      * Method: readBytes(char *bytes, unint4 &length)
      *
      * @description
      *  Reads the mux state of an IO port.
      * @params
      *
      * @returns
      *  int         Error Code
      *******************************************************************************/
     ErrorT readBytes(char *bytes, unint4 &length);

     /*****************************************************************************
      * Method: writeBytes(const char *bytes, unint4 length)
      *
      * @description
      *  Writes the mux state of an IO port.
      * @params
      *
      * @returns
      *  int         Error Code
      *******************************************************************************/
     ErrorT writeBytes(const char *bytes, unint4 length);
};

#endif /* SOURCE_ARCH_ARM_ARMV4T_ARMV7_OMAP3530_OMAPIOMUX_HH_ */
