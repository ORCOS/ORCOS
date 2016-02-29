/*
 * endian.h
 *
 *  Created on: 10.07.2013
 *      Copyright & Author: dbaldin
 */

#ifndef SOURCE_INC_ENDIAN_H_
#define SOURCE_INC_ENDIAN_H_

#ifdef __cplusplus
extern "C" {
#endif


    /*****************************************************************************
     * Method: cputole32(unint4 n)
     *
     * @description
     *  Converts an 32 bit integer given in cpu endiannes
     *   to little endian
     * @params
     *  n         value in host format
     * @returns
     *  int        value in little endian format
     *******************************************************************************/
    unint4 cputole32(unint4 n);


    /*****************************************************************************
     * Method: cputole16(unint4 n)
     *
     * @description
     *  Converts an 16 bit integer given in cpu endiannes
     *   to little endian
     * @params
     *  n         value in host format
     * @returns
     *  int        value in little endian format
     *******************************************************************************/
    unint2 cputole16(unint2 n);

    /*****************************************************************************
     * Method: cputobe32(unint4 n)
     *
     * @description
     *  Converts an 32 bit integer given in cpu endiannes
     *   to big endian
     * @params
     *  n         value in host format
     * @returns
     *  int        value in big endian format
     *******************************************************************************/
    unint4 cputobe32(unint4 n);

    /*****************************************************************************
     * Method: cputobe16(unint4 n)
     *
     * @description
     *  Converts an 16 bit integer given in cpu endiannes
     *   to big endian
     * @params
     *  n         value in host format
     * @returns
     *  int        value in big endian format
     *******************************************************************************/
     unint2 cputobe16(unint2 n);

#define le32tocpu cputole32
#define le16tocpu cputole16
#define be32tocpu cputobe32
#define be16tocpu cputobe16

    /*****************************************************************************
     * Method: __get_unaligned_le16(char* p)
     *
     * @description
     *   Reads a 16 bit value from an character array
     *    and converts it to a 16 bit little endian.
     *
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    static inline unint2 __get_unaligned_le16(char* p) {
#if BYTE_ORDER == LITTLE_ENDIAN
        return ((unint2) (p[0] | p[1] << 8));  // if we need to convert
#else
        return (p[0] << 8 | p[1]);
#endif
    }

    /*****************************************************************************
     * Method: __get_unaligned_le32(char* p)
     *
     * @description
     *   Reads a 32 bit value from an character array
     *    and converts it to a 32 bit little endian.
     *
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    static inline unint4 __get_unaligned_le32(char* p) {
#if BYTE_ORDER == LITTLE_ENDIAN
        return (p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24);  // if we need to convert
#else
        return (p[3] | p[2] << 8 | p[1] << 16 | p[0] << 24);
#endif
    }


    /*****************************************************************************
     * Method: le32_to_int(unsigned char *le32)
     *
     * @description
     *  Convert char[4] in little endian format to the host format integer
     * @params
     *
     * @returns
     *  int         value in host format
     *******************************************************************************/
    static inline int le32_to_int(char *le32) {
        return (__get_unaligned_le32(le32));
    }

#ifdef __cplusplus
}
#endif

#define ___swab32(x)  __builtin_bswap32(x)

/*((unint4)( \
        (((unint4)(x) & (unint4)0x000000ffUL) << 24) | \
        (((unint4)(x) & (unint4)0x0000ff00UL) <<  8) | \
        (((unint4)(x) & (unint4)0x00ff0000UL) >>  8) | \
        (((unint4)(x) & (unint4)0xff000000UL) >> 24) ))*/

#endif  // SOURCE_INC_ENDIAN_H_
