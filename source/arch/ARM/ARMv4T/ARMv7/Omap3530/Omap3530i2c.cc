/*
 * Omap3530i2c.cc
 *
 *  Created on: 20.08.2012
 *    Copyright &  Author: dbaldin
 */

#include "Omap3530i2c.hh"
#include "kernel/Kernel.hh"
#include "inc/memio.h"
#include "inc/types.hh"
#include "inc/error.hh"

extern Kernel* theOS;

/*****************************************************************************
 * Method: Omap3530i2c::wait_for_bus()
 *
 * @description
 *  Waits for the I2C Bus to be not busy any more
 *******************************************************************************/
void Omap3530i2c::wait_for_bus() {
    volatile int timeout = 10;
    unint2 stat;

    OUTW(I2C_STAT, 0xFFFF); /* clear current interruts...*/

    while ((stat = INW(I2C_STAT) & I2C_STAT_BB) && timeout--) {
        OUTW(I2C_STAT, stat);
        kwait(2);
    }

    if (timeout <= 0) {
        LOG(ARCH, ERROR, "Omap3530i2c::timed out in wait_for_bb: I2C_STAT=%x", INW(I2C_STAT));
    }
    OUTW(I2C_STAT, 0xFFFF); /* clear delayed stuff*/
}

/*****************************************************************************
 * Method: Omap3530i2c::wait_for_pin
 *
 * @description
 *
 *******************************************************************************/
unint2 Omap3530i2c::wait_for_pin() {
    unint2 status;
    volatile int timeout = 10;

    do {
        kwait(10);
        status = (unint2) INW(I2C_STAT);
    } while (!(status & (I2C_STAT_ROVR | I2C_STAT_XUDF | I2C_STAT_XRDY |
    I2C_STAT_RRDY | I2C_STAT_ARDY | I2C_STAT_NACK |
    I2C_STAT_AL)) && timeout--);

    if (timeout <= 0) {
        LOG(ARCH, ERROR, "Omap3530i2c::timed out in wait_for_pin: I2C_STAT=%x", INW(I2C_STAT));
        OUTW(I2C_STAT, 0xFFFF);
    }

    return (status);
}

/*****************************************************************************
 * Method: Omap3530i2c::flush_fifo()
 *
 * @description
 *
 *******************************************************************************/
void Omap3530i2c::flush_fifo() {
    unint2 stat;

    /* note: if you try and read data when its not there or ready
     * you get a bus error
     */
    while (1) {
        stat = (unint2) INW(I2C_STAT);
        if (stat == I2C_STAT_RRDY) {
            INW(I2C_DATA);
            OUTW(I2C_STAT, I2C_STAT_RRDY);
            kwait(1);
        } else {
            return;
        }
    }
}

/*****************************************************************************
 * Method: Omap3530i2c::i2c_init(unint4 speed)
 *
 * @description
 *
 *******************************************************************************/
void Omap3530i2c::i2c_init(unint4 speed) {
    unint2 scl;

    unint4 clocks = INW(0x48004A00);
    //printf("Clocks set: %x\r",clocks);
    OUTW(0x48004A00, clocks | 1 << 15 | 1 << 16 | 1 << 17);

    OUTH(I2C_SYSC, 0x2);  // issue reset, for ES2 after soft reset
    kwait(1);

    OUTH(I2C_SYSC, I2C_SYSC_CLOCKACTIVITY_ALL | I2C_SYSC_IDLEMODE_NO | I2C_SYSC_ENAWAKEUP);

    // enable all wakeups!
    OUTH(I2C_WE, 0xffff);

    if (INW(I2C_CON) & I2C_CON_EN) {
        OUTH(I2C_CON, 0);
        kwait(5);
    }

    // Setup the speed of the I2C Bus
    /* get 12Mhz I2C module clock */
    // set prescaler to 7 as we get a 96 MHZ input clock
    unint1 psc = 7;
    OUTH(I2C_PSC, psc);

    /* Program desired operating rate */
    unint4 fclk_rate = 96000;
    fclk_rate /= (psc + 1);
    if (psc > 2)
        psc = 2;

    speed = speed / 1000; /* 100 or 400 kHz */
    scl = (unint2) (((fclk_rate / (speed * 2)) - 7) + psc); /* use 7 when PSC = 0 */

    OUTH(I2C_SCLL, scl);        // low level signal length
    OUTH(I2C_SCLH, scl);        // high level signal length

    // set our own address if there are multiple master
    OUTH(I2C_OA, I2C_OWN_ADDRESS);

#define RTRSH 4
#define XTRSH 4
    // setup and clear FIFO
    unint2 buf = (RTRSH - 1) << 8 | 1 << 6 | (XTRSH - 1) | 1 << 14;
    OUTH(I2C_BUF, buf);

    //enable I2C module again
    OUTH(I2C_CON, I2C_CON_EN);

    /* have to enable interupts or OMAP i2c module doesn't work */
    OUTH(I2C_IE, I2C_IE_XRDY_IE | I2C_IE_RRDY_IE | I2C_IE_ARDY_IE | I2C_IE_NACK_IE | I2C_IE_AL_IE);
    kwait(1);
    flush_fifo();

    OUTH(I2C_STAT, 0xFFFF);
    // be sure no system test is running
    OUTH(I2C_SYSTEST, 0);
    OUTH(I2C_CNT, 0);
}

/*****************************************************************************
 * Method: Omap3530i2c::i2c_read_byte(unint1 devaddr,
 *                                    unint1 regoffset,
 *                                    unint1 *values,
 *                                    unint1 length)
 *
 * @description
 *  Tries to read length bytes out of register 'regoffset' from the device with
 *  devaddr connected to the I2C bus.
 *******************************************************************************/
ErrorT Omap3530i2c::i2c_read_byte(unint1 devaddr, unint1 regoffset, char *values, unint1 length) {
    int i2c_error = cOk;
    unint2 status;

    /* wait until bus not busy */
    wait_for_bus();

    /* one byte only */
    OUTH(I2C_CNT, 1);
    /* set 7-bit slave address */
    OUTH(I2C_SA, devaddr);

    // setup and clear FIFO
    unint2 clearcmd = (1 - 1) << 8 | I2C_BUF_RXFIFO_CLR | I2C_BUF_TXFIFO_CLR | (1 - 1);
    OUTH(I2C_BUF, clearcmd);
    /* no stop bit needed here */
    OUTH(I2C_CON, I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_TRX);

    status = wait_for_pin();

    if (status & I2C_STAT_XRDY) {
        /* Important: have to use byte access */
        OUTH(I2C_DATA, regoffset);
        // clear the XRDY bit
        OUTH(I2C_STAT, I2C_STAT_XRDY);

        kwait(2);    // wait 2 ms .. for ack
        if (status & I2C_STAT_NACK) {
            i2c_error = cError;
            LOG(ARCH, ERROR, "i2c_read_byte() Data was not acknowledged");
        }
    } else {
        i2c_error = cError;
        LOG(ARCH, ERROR, "i2c_read_byte() XRDY not set..");
    }

    // start read mode

    if (i2c_error == cOk) {
        //printf("Ready to read data!\r");

        OUTH(I2C_CNT, length);
        /* set 7-bit slave address */
        OUTH(I2C_SA, devaddr);

        // setup and clear FIFO
        //    unint4 buf = (length - 1) << 8 | I2C_BUF_RXFIFO_CLR | I2C_BUF_TXFIFO_CLR | (length - 1);

        // for receiving somehow rx and tx threshold 1 only works
        unint2 buf = (1 - 1) << 8 | I2C_BUF_RXFIFO_CLR | I2C_BUF_TXFIFO_CLR | (1 - 1);
        OUTH(I2C_BUF, buf);
        /* stop bit needed here */
        OUTH(I2C_CON, I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_STP);

        status = wait_for_pin();
        //printf("status=%x\r",status & I2C_STAT_RRDY );
        if (status & I2C_STAT_RRDY) {
            for (int i = 0; i < length; i++) {
                unint2 data_in = INH(I2C_DATA);
                values[i] = (unint1) data_in;
            }

            OUTH(I2C_STAT, I2C_STAT_RRDY);
        } else {
            LOG(ARCH, ERROR, "OMAP3530i2C::readBytes() Not enough data received...");
            i2c_error = cError;
        }

        if (i2c_error == cOk) {
            OUTH(I2C_CON, I2C_CON_EN);
            // clear the transaction
            while (INH(I2C_STAT) || (INH(I2C_CON) & I2C_CON_MST)) {
                flush_fifo();
                kwait(1);
                OUTH(I2C_STAT, 0xFFFF);
            }
        }
    }

    flush_fifo();
    OUTH(I2C_STAT, 0xFFFF);
    OUTH(I2C_CNT, 0);
    return (i2c_error);
}

Omap3530i2c::Omap3530i2c(T_Omap3530i2c_Init *init) :
        CharacterDevice(true, init->Name) {
    this->address = init->Address;
    i2c_init(OMAP_I2C_STANDARD);
}

Omap3530i2c::~Omap3530i2c() {
    // disable i2c
    OUTW(I2C_CON, 0);
    // disable all interrupts
    OUTW(I2C_IE, 0);
}

/*****************************************************************************
 * Method: Omap3530i2c::readBytes(char* bytes, unint4& length)
 *
 * @description
 *  Tries to read length bytes out of register  defined by bytes[1] from the device with
 *  addr bytes[0] connected to the I2C bus into the bytes array.
 *******************************************************************************/
ErrorT Omap3530i2c::readBytes(char* bytes, unint4& length) {
    //              8 bit            8 bit
    // * bytes = | slave address |  register offset |

    unint1 slave_address = bytes[0];
    unint1 offset = bytes[1];
    if (length > 32)
        return (cInvalidArgument );

    LOG(ARCH, DEBUG, "Omap3530i2c::readBytes from client %d, offset %d, length %d", slave_address, offset, length);

    if (isError(i2c_read_byte(slave_address, offset, bytes, (unint1) length))) {
        LOG(ARCH, WARN, "Omap3530i2c::readBytes I/O Error.. resetting I2C");
        i2c_init(OMAP_I2C_STANDARD);
        length = 0;
        return (cError );
    }

    return (cOk );
}

/*****************************************************************************
 * Method: Omap3530i2c::i2c_write_bytes(unint1 devaddr, const char* value, unint1 length)
 *
 * @description
 *  Tries to write length bytes to the device with addr devaddr
 *******************************************************************************/
ErrorT Omap3530i2c::i2c_write_bytes(unint1 devaddr, const char* value, unint1 length) {
    int i2c_error = cOk;
    unint2 status;

    /* wait until bus not busy */
    wait_for_bus();

    OUTH(I2C_CNT, length);
    /* set slave address */
    OUTH(I2C_SA, devaddr);

    // setup and clear FIFO
    unint2 buf = (unint2) ((length - 1) << 8 | I2C_BUF_RXFIFO_CLR | I2C_BUF_TXFIFO_CLR | (length - 1));
    OUTH(I2C_BUF, buf);

    /* stop bit needed here */
    OUTH(I2C_CON, I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_TRX | I2C_CON_STP);

    /* wait until state change */
    status = wait_for_pin();

    if (status & I2C_STAT_XRDY) {
        // FIFO has enough space

        int i = 0;
        while (i < length) {
            unint2 val = value[i];
            //printf("writing %d\r",val);
            OUTH(I2C_DATA, val);
            i++;
        }

        OUTH(I2C_STAT, I2C_STAT_XRDY);

        /* must have enough delay to allow BB bit to go low */
        kwait(2);

        if (INW(I2C_STAT) & I2C_STAT_NACK) {
            LOG(ARCH, DEBUG, "Omap3530i2c::writeBytes() NACK received.. slave not answering.");
            i2c_error = cError;
        }
    } else {
        LOG(ARCH, ERROR, "Omap3530i2c::writeBytes() unable to transmit. No XRDY flag set.");
        i2c_error = cError;
    }

    if (i2c_error == cOk) {
        int eout = 200;

        OUTH(I2C_CON, I2C_CON_EN);
        while ((status = INH(I2C_STAT)) || (INH(I2C_CON) & I2C_CON_MST)) {
            kwait(1);

            OUTH(I2C_STAT, 0xFFFF);
            if (--eout == 0)
                break;
        }
    }

    flush_fifo();
    OUTH(I2C_STAT, 0xFFFF);
    OUTH(I2C_CNT, 0);
    return (i2c_error);
}

/*****************************************************************************
 * Method: Omap3530i2c::writeBytes(const char* bytes, unint4 length)
 *
 * @description
 *  Tries to write length bytes, starting at &bytes[1], to the device
 *  with addr bytes[0]
 *******************************************************************************/
ErrorT Omap3530i2c::writeBytes(const char* bytes, unint4 length) {
    // e.g. bytes = | slaveaddress | command | data .. data .. data

    if (length < 3) {
        LOG(ARCH, WARN, "Omap3530i2c::writeBytes invalid data length. must be > 2");
        return (cInvalidArgument );
    }

    if (length > 32) {
        return (cInvalidArgument );
    }

    if (isError(i2c_write_bytes(bytes[0], &bytes[1], (unint1) (length - 1)))) {
        // error occurred .. reset i2c
        LOG(ARCH, WARN, "Omap3530i2c::writeBytes I/O Error.. resetting I2C");
        i2c_init(OMAP_I2C_STANDARD);
        return (cError );
    }

    return (cOk );
}
