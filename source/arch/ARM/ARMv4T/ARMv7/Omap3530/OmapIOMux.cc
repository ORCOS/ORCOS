/*
 * OmapIOMux.cc
 *
 *  Created on: 21.12.2014
 *    Copyright & Author: Daniel Baldin
 */

#include <OmapIOMux.hh>
#include <inc/sys/omapmux.h>

#define writew(b, addr) (void)((*(volatile unint2 *) (addr)) = (b))
#define readw(addr) (*(volatile unint2 *) (addr))

#define OMAP34XX_CTRL_BASE 0x48002000
#define MUX_VAL(OFFSET, VALUE) writew((VALUE), OMAP34XX_CTRL_BASE + (OFFSET));

OmapIOMux::OmapIOMux() : CharacterDevice(false, "mux") {
}

OmapIOMux::~OmapIOMux() {
}

/*****************************************************************************
  * Method: OmapIOMux::readBytes(char *bytes, unint4 &length)
  *
  * @description
  *  Reads the mux state of an IO port.
  * @params
  *
  * @returns
  *  int         Error Code
  *******************************************************************************/
 ErrorT OmapIOMux::readBytes(char *bytes, unint4 &length) {
     if (length != sizeof(omap_muxval_t)) {
         return (cInvalidArgument);
     }
     omap_muxval_t* muxval = reinterpret_cast<omap_muxval_t*>(bytes);

     muxval->muxval = readw(OMAP34XX_CTRL_BASE + muxval->padconf);
     return (cOk);
 }

 /*****************************************************************************
  * Method: OmapIOMux::writeBytes(const char *bytes, unint4 length)
  *
  * @description
  *  Writes the mux state of an IO port.
  * @params
  *
  * @returns
  *  int         Error Code
  *******************************************************************************/
 ErrorT OmapIOMux::writeBytes(const char *bytes, unint4 length) {
     if (length != sizeof(omap_muxval_t)) {
         return (cInvalidArgument);
     }
     omap_muxval_t* muxval = (omap_muxval_t*)(bytes);

     MUX_VAL(muxval->padconf, muxval->muxval);
     return (cError);
 }
