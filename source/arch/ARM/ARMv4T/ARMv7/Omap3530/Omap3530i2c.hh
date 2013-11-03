/*
 * Omap3530i2c.hh
 *
 *  Created on: 20.08.2012
 *      Author: dbaldin
 */

#ifndef OMAP3530I2C_HH_
#define OMAP3530I2C_HH_

#include "hal/CharacterDeviceDriver.hh"
#include "assembler.h"

// i2c bus speeds
#define OMAP_I2C_FAST_MODE 		(400 kHZ)
#define OMAP_I2C_STANDARD 		(100 kHZ)

#define I2C_OWN_ADDRESS			44

#define I2C_BASE 				(this->address)

#define I2C_BASE1               0x48070000
#define I2C_BASE3               0x48060000
#define I2C_BASE2               0x48072000 /* nothing hooked up on h4 */

#define I2C_REV                 (I2C_BASE + 0x00)
#define I2C_IE                  (I2C_BASE + 0x04)
#define I2C_STAT                (I2C_BASE + 0x08)
#define I2C_IV                  (I2C_BASE + 0x0c)
#define I2C_BUF                 (I2C_BASE + 0x14)
#define I2C_CNT                 (I2C_BASE + 0x18)
#define I2C_DATA                (I2C_BASE + 0x1c)
#define I2C_SYSC                (I2C_BASE + 0x20)
#define I2C_CON                 (I2C_BASE + 0x24)
#define I2C_OA                  (I2C_BASE + 0x28)
#define I2C_SA                  (I2C_BASE + 0x2c)
#define I2C_PSC                 (I2C_BASE + 0x30)
#define I2C_SCLL                (I2C_BASE + 0x34)
#define I2C_SCLH                (I2C_BASE + 0x38)
#define I2C_SYSTEST             (I2C_BASE + 0x3c)
#define I2C_WE            		(I2C_BASE + 0x0c)

/* I2C masks */


#define I2C_SYSC_CLOCKACTIVITY_ALL (3 << 8)
#define I2C_SYSC_IDLEMODE_NO	   (1 << 3)
#define I2C_SYSC_IDLEMODE_SMART	   (2 << 3)
#define I2C_SYSC_ENAWAKEUP	   	   (1 << 2)
#define I2C_SYSC_AUTOIDLE	   	   (1 << 0)

/* I2C Interrupt Enable Register (I2C_IE): */
#define I2C_IE_GC_IE    (1 << 5)
#define I2C_IE_XRDY_IE  (1 << 4)        /* Transmit data ready interrupt enable */
#define I2C_IE_RRDY_IE  (1 << 3)        /* Receive data ready interrupt enable */
#define I2C_IE_ARDY_IE  (1 << 2)        /* Register access ready interrupt enable */
#define I2C_IE_NACK_IE  (1 << 1)        /* No acknowledgment interrupt enable */
#define I2C_IE_AL_IE    (1 << 0)        /* Arbitration lost interrupt enable */

/* I2C Status Register (I2C_STAT): */

#define I2C_STAT_SBD    (1 << 15)       /* Single byte data */
#define I2C_STAT_BB     (1 << 12)       /* Bus busy */
#define I2C_STAT_ROVR   (1 << 11)       /* Receive overrun */
#define I2C_STAT_XUDF   (1 << 10)       /* Transmit underflow */
#define I2C_STAT_AAS    (1 << 9)        /* Address as slave */
#define I2C_STAT_GC     (1 << 5)
#define I2C_STAT_XRDY   (1 << 4)        /* Transmit data ready */
#define I2C_STAT_RRDY   (1 << 3)        /* Receive data ready */
#define I2C_STAT_ARDY   (1 << 2)        /* Register access ready */
#define I2C_STAT_NACK   (1 << 1)        /* No acknowledgment interrupt enable */
#define I2C_STAT_AL     (1 << 0)        /* Arbitration lost interrupt enable */


/* I2C Interrupt Code Register (I2C_INTCODE): */

#define I2C_INTCODE_MASK        7
#define I2C_INTCODE_NONE        0
#define I2C_INTCODE_AL          1       /* Arbitration lost */
#define I2C_INTCODE_NAK         2       /* No acknowledgement/general call */
#define I2C_INTCODE_ARDY        3       /* Register access ready */
#define I2C_INTCODE_RRDY        4       /* Rcv data ready */
#define I2C_INTCODE_XRDY        5       /* Xmit data ready */

/* I2C Buffer Configuration Register (I2C_BUF): */

#define I2C_BUF_RDMA_EN         (1 << 15)       /* Receive DMA channel enable */
#define I2C_BUF_XDMA_EN         (1 << 7)        /* Transmit DMA channel enable */
#define I2C_BUF_RXFIFO_CLR      (1 << 14)        /* Transmit DMA channel enable */
#define I2C_BUF_TXFIFO_CLR      (1 << 6)        /* Transmit DMA channel enable */

/* I2C Configuration Register (I2C_CON): */

#define I2C_CON_EN      (1 << 15)       /* I2C module enable */
#define I2C_CON_BE      (1 << 14)       /* Big endian mode */
#define I2C_CON_STB     (1 << 11)       /* Start byte mode (master mode only) */
#define I2C_CON_MST     (1 << 10)       /* Master/slave mode */
#define I2C_CON_TRX     (1 << 9)        /* Transmitter/receiver mode (master mode only) */
#define I2C_CON_XA      (1 << 8)        /* Expand address */
#define I2C_CON_STP     (1 << 1)        /* Stop condition (master mode only) */
#define I2C_CON_STT     (1 << 0)        /* Start condition (master mode only) */

#define I2C_CON_SCCB    (2 << 12)        /* Start condition (master mode only) */


/* I2C System Test Register (I2C_SYSTEST): */

#define I2C_SYSTEST_ST_EN       (1 << 15)       /* System test enable */
#define I2C_SYSTEST_FREE        (1 << 14)       /* Free running mode (on breakpoint) */
#define I2C_SYSTEST_TMODE_MASK  (3 << 12)       /* Test mode select */
#define I2C_SYSTEST_TMODE_SHIFT (12)            /* Test mode select */
#define I2C_SYSTEST_SCL_I       (1 << 3)        /* SCL line sense input value */
#define I2C_SYSTEST_SCL_O       (1 << 2)        /* SCL line drive output value */
#define I2C_SYSTEST_SDA_I       (1 << 1)        /* SDA line sense input value */
#define I2C_SYSTEST_SDA_O       (1 << 0)        /* SDA line drive output value */

class Omap3530i2c: public CharacterDeviceDriver {
private:

	// MMIO address of the i2c component
	int4 	address;

	void 	wait_for_bus (void);

	unint2 	wait_for_pin (void);

	void 	flush_fifo(void);

	void 	i2c_init(unint4 speed);

	ErrorT 	i2c_read_byte (unint1 devaddr, unint1 regoffset, unint1 *values, unint1 length);

	ErrorT 	i2c_write_bytes (unint1 devaddr, const char* value, unint1 length);

public:

	Omap3530i2c( T_Omap3530i2c_Init *init );

	~Omap3530i2c();

	 /*!
	 * \brief reads a byte from the i2c Bus
	 *
	 * Reads a Byte from the I2C bus polling the device. Loops until a byte is read.
	 *
	 */
	ErrorT readByte( char* p_byte ) {
		return (cNotImplemented);
	}

	/*!
	 * \brief Not supported
	 *
	 * Not supported as I2C requires at least 2 bytes.
	 */
	ErrorT writeByte( char c_byte ) {
		return (cNotImplemented);
	}

	/*!
	 * \brief Reads a number of bytes from the device
	 *
	 * Reads multiple bytes from the I2C Bus. The number of bytes to be read is given in length.
	 * The number of bytes read will be returned in the length variable.
	 * If no bytes could be read length will be set to 0.
	 * The bytes array needs to be filled in the following format:
	 *
	 *                8 bit            8 bit
	 * bytes = | slave address |  register offset |
	 */
	ErrorT readBytes( char *bytes, unint4 &length );


	/*!
	 * \brief Writes a number of bytes to the I2C Bus
	 *
	 * The first byte passed is the I2C device receiver address.
	 * The second byte is the register offset start address
	 * of the receiver to be written
	 *                8 bit            8 bit        8 bit   8 bit
	 * bytes = | slave address |  register offset | data  | data  | ...
	 */
	ErrorT writeBytes( const char *bytes, unint4 length );

};

#endif /* OMAP3530I2C_HH_ */
