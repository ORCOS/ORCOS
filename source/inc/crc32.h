/*
 * crc32.h
 *
 *  Created on: 27.01.2013
 *      Copyright & Author: Daniel Baldin
 */

#ifndef SOURCE_INC_CRC32_H_
#define SOURCE_INC_CRC32_H_

#ifdef __cplusplus
extern "C" {
#endif

    /*****************************************************************************
     * Method: crc32(unsigned char *buf, size_t len)
     *
     * @description
     *  Calculates the CRC32 checksum over the region pointed
     *  by buf and size len.
     * @params
     *  buf         Region to crc
     *  len         length of the region
     * @returns
     *  unint4      CRC32 value
     *******************************************************************************/
    unint4 crc32(unsigned char *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif  // SOURCE_INC_CRC32_H_
