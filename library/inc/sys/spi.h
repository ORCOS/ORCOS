/*
 * spi.h
 *
 *  Created on: 24.02.2014
 *      Author: dbaldin
 */

#ifndef SPI_H_
#define SPI_H_

/* SPI ioctrl commands */
#define SPI_CONFIGURE_CHANNEL1 1
#define SPI_CONFIGURE_CHANNEL2 2
#define SPI_CONFIGURE_CHANNEL3 3
#define SPI_CONFIGURE_CHANNEL4 4

/* SPI_CONFIGURE_CHANNELX flags */

/* Set clock active level to low */
#define SPI_CLOCK_ACTIVE_LOW  (1 << 1)
/* Set data latch to even */
#define SPI_DATA_LATCH_EVEN (1)

/* SPI channel clock settings*/
#define SPI_CLOCK_DIVIDER_1  (0 << 2)
#define SPI_CLOCK_DIVIDER_2  (1 << 2)
#define SPI_CLOCK_DIVIDER_4  (2 << 2)
#define SPI_CLOCK_DIVIDER_8  (3 << 2)
#define SPI_CLOCK_DIVIDER_16 (4 << 2)
#define SPI_CLOCK_DIVIDER_32 (5 << 2)
#define SPI_CLOCK_DIVIDER_64 (6 << 2)
#define SPI_CLOCK_DIVIDER_128 (7 << 2)
#define SPI_CLOCK_DIVIDER_256 (8 << 2)
#define SPI_CLOCK_DIVIDER_512 (9 << 2)
#define SPI_CLOCK_DIVIDER_1024 (10 << 2)

/* Set active level to low */
#define SPI_CHANNEL_SELECT_ACTIVE_LOW ( 1 << 6)

/* Channel word length */
#define SPI_WORD_LEN_4BIT  (3 << 7)
#define SPI_WORD_LEN_5BIT  (4 << 7)
#define SPI_WORD_LEN_6BIT  (5 << 7)
#define SPI_WORD_LEN_7BIT  (6 << 7)
#define SPI_WORD_LEN_8BIT  (7 << 7)
#define SPI_WORD_LEN_9BIT  (8 << 7)
#define SPI_WORD_LEN_10BIT  (9 << 7)
#define SPI_WORD_LEN_11BIT  (10 << 7)
#define SPI_WORD_LEN_12BIT  (11 << 7)
#define SPI_WORD_LEN_13BIT  (12 << 7)
#define SPI_WORD_LEN_14BIT  (13 << 7)
#define SPI_WORD_LEN_15BIT  (14 << 7)
#define SPI_WORD_LEN_16BIT  (15 << 7)
#define SPI_WORD_LEN_17BIT  (16 << 7)
#define SPI_WORD_LEN_18BIT  (17 << 7)
#define SPI_WORD_LEN_19BIT  (18 << 7)
#define SPI_WORD_LEN_20BIT  (19 << 7)
#define SPI_WORD_LEN_32BIT  (0x1f << 7)

/* Channel Transsmission receive settings */

/* Transmit & receive on this channel */
#define SPI_CHANNEL_TXRX    (0 << 12)
/* Receive only on this channel */
#define SPI_CHANNEL_RXONLY  (1 << 12)
/* Transmit only on this channel */
#define SPI_CHANNEL_TXONLY  (2 << 12)

/* Set start bit polarity to one */
#define SPI_START_BIT_POLARITY1 (1 << 24)


#endif /* SPI_H_ */
