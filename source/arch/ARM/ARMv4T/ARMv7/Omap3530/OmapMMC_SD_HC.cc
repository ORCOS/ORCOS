/*
 * MMC.cc
 *
 *  Created on: 15.09.2013
 *    Copyright & Author: dbaldin
 */

#include "OmapMMC_SD_HC.hh"
#include "inc/memio.h"

#include "kernel/Kernel.hh"
#include "inc/memtools.hh"
#include "dma.h"

extern Kernel* theOS;



// offsets to registers

#define MMCHS_SYSCONFIG     0x10
#define MMCHS_SYSSTATUS     0x14
#define MMCHS_CSRE          0x24
#define MMCHS_CON           0x2c
#define MMCHS_BLK           0x104
#define MMCHS_ARG           0x108
#define MMCHS_CMD           0x10c

#define MMCHS_RSP10         0x110
#define MMCHS_RSP32         0x114
#define MMCHS_RSP54         0x118
#define MMCHS_RSP76         0x11c
#define MMCHS_DATA          0x120

#define MMCHS_PSTATE        0x124
#define MMCHS_HCTL          0x128

#define MMCHS_SYSCTL        0x12c
#define MMCHS_STAT          0x130
#define MMCHS_IE            0x134
#define MMCHS_ISE           0x138

#define MMCHS_AC12          0x13c
#define MMCHS_CAPA          0x140
#define MMCHS_CUR_CAPA      0x148
#define MMCHS_REV           0x1fc

#define STAT_CC             (1 << 0)
#define STAT_TC             (1 << 1)
#define STAT_BGE            (1 << 2)
#define STAT_BWR            (1 << 4)
#define STAT_BRR            (1 << 5)
#define STAT_CIRQ           (1 << 8)
#define STAT_OBI            (1 << 9)
#define STAT_ERRI           (1 << 15)
#define STAT_CTO            (1 << 16)

#define CEN                 (1 << 2)


OmapMMC_SD_HC::OmapMMC_SD_HC(T_OmapMMC_SD_HC_Init *p_init) :
        BlockDeviceDriver(p_init->Name) {

    card_type      = Unknown;
    baseAddress    = p_init->Address;
    rca            = 0;
    isHighCapacity = false;

    int pnum     = strlen(p_init->Name);
    char* name   = new char[pnum + 2];
    memcpy(name, p_init->Name, pnum);
    name[pnum]   = '0' + BlockDeviceDriver::freeBlockDeviceIDs.getNextID();
    name[pnum+1] = 0;
    this->name   = name;

    /* reset module */
    LOG(ARCH, INFO, "MMC/SD HC() Resetting.");

    OUTW(baseAddress + MMCHS_SYSCONFIG, 0x2);
    if (TIMEOUT_WAIT(((INW(baseAddress + MMCHS_SYSSTATUS) & 0x1) == 0x0), 40000)) {
        LOG(ARCH, ERROR, "MMC/SD HC() Timeout resetting module..");
        return;
    }

    /* soft reset all */
    OUTW(baseAddress + MMCHS_SYSCTL, INW(baseAddress + MMCHS_SYSCTL) | (1 << 24));

    if (TIMEOUT_WAIT(((INW(baseAddress + MMCHS_SYSCTL) & (1 << 24)) != 0x0), 40000)) {
        LOG(ARCH, ERROR, "MMC/SD HC() Timeout on soft reset..");
        return;
    }

    LOG(ARCH, INFO, "MMC/SD HC() Reset successful.");

    /* activate 1.8v and 3v output */
    unint4 value = INW(baseAddress + MMCHS_CAPA);
    value |= 0x7000000;
    OUTW(baseAddress + MMCHS_CAPA, value);

    /* try initializing right now */
    this->init();
}

/*****************************************************************************
 * Method: OmapMMC_SD_HC::init()
 *
 * @description
 *  Initializes the MMC_SD Card device, thereby identifying SD cards
 *******************************************************************************/
void OmapMMC_SD_HC::init() {
    LOG(ARCH, DEBUG, "MMC/SD HC() Setting Defaults..");
    OUTW(baseAddress + MMCHS_CON, 0x00000000);

    int voltage_capa = (INW(baseAddress + MMCHS_CAPA) >> 24) & 0x7;
    OUTW(baseAddress + MMCHS_HCTL, (voltage_capa << 9) | (1 << 24));

    // activate bus power
    OUTW(baseAddress + MMCHS_HCTL, INW(baseAddress + MMCHS_HCTL) | (1 <<8));

    if (!(INW(baseAddress + MMCHS_HCTL) & (1 << 8))) {
        LOG(ARCH, INFO, "MMC/SD HC() Bus power on failed. No card or invalid voltage.");
        return;
    }

    // stop clock
    OUTW(baseAddress + MMCHS_SYSCTL, 0x00000000);

    // set clock frequency
    OUTW(baseAddress + MMCHS_SYSCTL, 0x0000a001 | (240 << 16));

    // wait until frequency is set
    while ((INW(baseAddress + MMCHS_SYSCTL) & (1 << 1)) == 0x0) {
    }

    /* set clock activity to always on */
    OUTW(baseAddress + MMCHS_SYSCONFIG, (3 << 8) | (1 << 3));

    // provide clock to card
    OUTW(baseAddress + MMCHS_SYSCTL, INW(baseAddress + MMCHS_SYSCTL) | (1 << 2));

    /* check for card */
    if (!(INW(baseAddress + MMCHS_PSTATE) & (1 << 16))) {
        LOG(ARCH, INFO, "MMC/SD HC() CINS is low. No card present!");
    }

    // enable all interrupts...
    // must be done as all responses from the card are otherwise ignored ..
    /*
     * IE_BADA | IE_CERR | IE_DEB | IE_DCRC | IE_DTO | IE_CIE |
     IE_CEB | IE_CCRC | IE_CTO | IE_BRR | IE_BWR | IE_TC | IE_CC
     */
    //OUTW(baseAddress + MMCHS_IE, 0x307F0033);
    OUTW(baseAddress + MMCHS_IE, 0x337F01F3);
    OUTW(baseAddress + 0x138, 0x337F01F3);

    LOG(ARCH, INFO, "MMC/SD HC() INIT. Synchronizing with card..");

    OUTW(baseAddress + MMCHS_STAT, STAT_CC);
    OUTW(baseAddress + MMCHS_CON, 0x2);
    OUTW(baseAddress + MMCHS_ARG, 0x0);
    OUTW(baseAddress + MMCHS_CMD, 0x0);

    // wait for initial clock generation to be stable
    kwait(1);
    while ((INW(baseAddress + MMCHS_STAT) & STAT_CC) == 0x0) {
    }

    OUTW(baseAddress + MMCHS_STAT, STAT_CC);
    OUTW(baseAddress + MMCHS_CMD, 0x0);
    while ((INW(baseAddress + MMCHS_STAT) & STAT_CC) == 0x0) {
    }

    // stop init
    OUTW(baseAddress + MMCHS_CON, 0x0);

    // clear all in stat
    OUTW(baseAddress + MMCHS_STAT, 0xffffffff);

    LOG(ARCH, DEBUG, "MMC/SD HC() Pre-Card Identification..");

    // set clock frequency to 400khz
    //OUTW(baseAddress + MMCHS_HCTL,0x00000b00);
    OUTW(baseAddress + MMCHS_SYSCTL, 0x00003C07);
    //OUTW(baseAddress + MMCHS_CON,0x00000001);

    // send CMD0 to set back all mmc cards to initialize mode
    sendCommand(0, 0);

    card_type = Unknown;

    // send CMD5
    if (sendCommand(0x05020000, 0) == cOk) {
        LOG(ARCH, ERROR, "MMC/SD HC() SDIO card detected. Currently unsupported.");
        //TODO follow SDIO Standard Specification to idenfity card type
        card_type = SDIO;
        return;
    }
    LOG(ARCH, INFO, "MMC/SD HC() No SDIO card. Continuing for SD and MMC.");

    /*
      1) CMD8 fails and CMD58 fails: must be MMC, thus initialize using CMD1
      2) CMD8 fails and CMD58 passes: must be Ver1.x Standard Capacity SD Memory Card
      3) CMD8 passes and CMD58 passes (CCS = 0): must be Ver2.00 or later Standard Capacity SD Memory Card
      4) CMD8 passes and CMD58 passes (CCS = 1): must be Ver2.00 or later High Capacity SD Memory Card
      5) CMD8 passes and CMD58 passes but indicates non compatible voltage range: unusable card
     */

    // send CMD8
    if (sendCommand(0x81a0000, 0x01aa) == cOk) {
      LOG(ARCH, INFO, "MMC/SD HC() SD 2.0 Card detected.");
      card_type = SD2_0;
    }
    if (sendCommand(0x371a0000, 0) == cOk) {
      /* ACMD41 with read OCR */
      if (sendCommand(0x29030000, 0) == cError) {
          LOG(ARCH, ERROR, "MMC/SD HC() Can not detect card.");
          return;
      }
    }
    // TODO: SD1_0
    if (card_type != SD2_0) {

        if (sendCommand(0x01030000, 0) == cOk) {
            LOG(ARCH, INFO, "MMC/SD HC() CMD 1 succeeded. MMC assumed.");
            card_type = MMC;

            int value = INW(baseAddress + MMCHS_RSP10);
            LOG(ARCH, TRACE, "MMC/SD HC() RSP10: 0x%x", value);

            int tries = 0;
            while ((value >> 31) == 0) {
                if (sendCommand(0x01030000, value) == cError) {
                    LOG(ARCH, ERROR, "MMC/SD HC() Error on MMC Card Init: CMD1 failed..");
                    return;
                }
                value = INW(baseAddress + MMCHS_RSP10)  | 1 << 28 | 1 << 30;
                LOG(ARCH, TRACE, "MMC/SD HC() RSP10: 0x%x", value);
                tries++;
                if (tries > 500) {
                    LOG(ARCH, ERROR, "MMC/SD HC() Error. Card stays busy.. No Connection? ");
                    return;
                }
            }

            isHighCapacity = (value >> 30) & 0x1;
            LOG(ARCH, INFO, "MMC/SD HC() High Capacity MMC : %d", isHighCapacity);

        } else {
            LOG(ARCH, ERROR, "MMC/SD HC() Unknown Card. ");
            return;
        }
    } else {

        int busy = 1;
        int arg = 0x0; //0x1 | 1 << 28 | 1 << 30;
        while (busy) {
            unint4 value = INW(baseAddress + MMCHS_RSP10);
            LOG(ARCH, TRACE, "MMC/SD HC() RSP10: 0x%x", value);
            int ready = (value >> 31) & 0x1;
            if (!ready) {
                /* keep OCR value */
                arg = value | 1 << 28 | 1 << 30;
                /* query state again by issuing CMD55 + ACMD41 again*/
                /* send CMD55 */
                if (sendCommand(0x371a0000, 0) == cError) {
                   LOG(ARCH, ERROR, "MMC/SD HC() Error on CMD 55.");
                   return;
                }
                /* send ACMD41 with busy after response */
                if (sendCommand(0x29030000, arg) == cError) {
                   LOG(ARCH, ERROR, "MMC/SD HC() Error on ACMD41. Can not detect card.");
                   return;
                }
           } else {
               isHighCapacity = (value >> 30) & 0x1;
               LOG(ARCH, INFO, "MMC/SD HC() High Capacity SD : %d", isHighCapacity);
               busy = 0;
           }
        }
    } // SD2_0 card type


    // init MMC or SD x.0 card here
    // send ALL_SEND_CID to get Card Identification number
    if (sendCommand(0x02090000, 0) == cError) {
        LOG(ARCH, WARN, "MMC/SD HC() reading CID failed.. No Card found..");
        return;
    }

    /* CID Register
     * from SD Specification:
     *     Manufacturer ID MID 8 [127:120]
     *     OEM/Application ID OID 16 [119:104]
     *     Product name PNM 40 [103:64]
     *     Product revision PRV 8 [63:56]
     *     Product serial number PSN 32 [55:24]
     *     reserved -- 4 [23:20]
     *     Manufacturing date MDT 12 [19:8]
     *     CRC7 checksum CRC 7 [7:1]
     *     not used, always 1 - 1 [0:0]
     *
     */

    unint4 value  = INW(baseAddress + MMCHS_RSP10);
    unint4 mdt    = value & 0xffff;
    unint4 value2 = INW(baseAddress + MMCHS_RSP32);
    unint4 serial = (value >> 24) | ((value2 & 0xffffff) << 8);

    LOG(ARCH, INFO, "MMC/SD HC() MDT=0x%x, Product Serial: 0x%x", mdt, serial);
    // unint4 revision = (value2 >> 8) & 0xff;
    // get product name
    unint4 value3 = INW(baseAddress + MMCHS_RSP54);
    unint4 value4 = INW(baseAddress + MMCHS_RSP76);

    char p_name[6];
    memcpy(&p_name[1], &value3, 4);
    p_name[0] = value4 & 0xff;
    p_name[5] = 0x0;

    LOG(ARCH, INFO, "MMC/SD HC() Product Name: %s", p_name);

    if (card_type != MMC) {
        // ask CARD for relative card address (RCA)
        sendCommand(0x031a0000, 0x1 << 16);
        value = INW(baseAddress + MMCHS_RSP10);
        this->rca = (unint2) (value >> 16);

        LOG(ARCH, DEBUG, "MMC/SD HC() RCA: 0x%x", rca);
    } else {
         // set CARD relative card address (RCA)
         sendCommand(0x031a0000, 0x0 << 16);
         value = INW(baseAddress + MMCHS_RSP10);
         this->rca = (unint2) (value >> 16);
         LOG(ARCH, DEBUG, "MMC/SD HC() RCA: 0x%x", rca);
    }

    // ask card for CSD
    sendCommand(0x09090000, rca << 16);
    // read CSD
    // [0:31]
    value = INW(baseAddress + MMCHS_RSP10);
    // [32:63]
    value2 = INW(baseAddress + MMCHS_RSP32);
    // [64:95]
    value3 = INW(baseAddress + MMCHS_RSP54);
    // [96:127]
    value4 = INW(baseAddress + MMCHS_RSP76);

    unint4 trans_speed = value4 & 0xff;
    unint4 block_size  = (value3 >> 16) & 0xf;
    this->sector_size  = 2 << block_size;
    LOG(ARCH, INFO, "MMC/SD HC() TRANS_SPEED: 0x%x, Block Size: %d", trans_speed, sector_size);

    // CMD 7 with RCA in upper 16 bits as argument select that card
    sendCommand(0x071a0000, rca << 16);

    // change speed to maximum supported by card
    // tran_speed = 0x32 = 25 mhz, 50 mhz otherwise

    // stop clock
    OUTW(baseAddress + MMCHS_SYSCTL, INW(baseAddress + MMCHS_SYSCTL) & ~(CEN));

    // set clock frequency
    // OUTW(baseAddress + MMCHS_SYSCTL,0x0000a001 | (4 << 16));
    OUTW(baseAddress + MMCHS_SYSCTL, 0x00000001 | (8 << 16) | (5 << 6));

    // wait until frequency is set
    while ((INW(baseAddress + MMCHS_SYSCTL) & (1 << 1)) == 0x0) {
    }

    // provide clock to card
    OUTW(baseAddress + MMCHS_SYSCTL, INW(baseAddress + MMCHS_SYSCTL) | (CEN));

    //OUTW(baseAddress + MMCHS_HCTL,INW(baseAddress + MMCHS_HCTL) | (1 <<8));

    // TODO: activate high speed mode. if card supports it is known
    // by isHighSpeed flag queried above

    if (card_type != MMC) {
        // CMD 55
        sendCommand(0x371a0000, rca << 16);

        // ACMD6
        if (sendCommand(0x061b0000, 0x2) == cOk) {
            // set to 4 bits data bus
            OUTW(baseAddress + MMCHS_HCTL, INW(baseAddress + MMCHS_HCTL) | (1 <<1));
        }
   } else {
       /* activate 8 bit data bus */
       // TODO issue CMD6 to switch bus width

       //OUTW(baseAddress + MMCHS_CON, INW(baseAddress + MMCHS_CON) | (1 << 5));
   }

    // CMD 16 .. set block len
    sendCommand(0x101a0000, 512);
    this->sector_size = 512;

    LOG(ARCH, INFO, "MMC/SD HC() Card ready..");

    theOS->getPartitionManager()->registerBlockDevice(this);
}

OmapMMC_SD_HC::~OmapMMC_SD_HC() {
}

/*****************************************************************************
 * Method: OmapMMC_SD_HC::sendCommand(unint4 cmd, unint4 arg)
 *
 * @description
 *
 *******************************************************************************/
ErrorT OmapMMC_SD_HC::sendCommand(unint4 cmd, unint4 arg) {
    unint4 value = 0;
    LOG(ARCH, DEBUG, "MMC/SD HC() Issuing CMD 0x%x, ARG: 0x%x", cmd, arg);
    /* wait until cmd line is not used any more */
    bool timeout = TIMEOUT_WAIT(INW(baseAddress + MMCHS_PSTATE) & 0x1, 30000);
    if (timeout) {
        LOG(ARCH, ERROR, "MMC/SD HC()::sendCommand Timeout waiting on cmd lines");
        return (cError );
    }

    OUTW(baseAddress + MMCHS_STAT, 0xffffffff);

    OUTW(baseAddress + MMCHS_ARG, arg);
    OUTW(baseAddress + MMCHS_CMD, cmd);

    if (TIMEOUT_WAIT(((INW(baseAddress + MMCHS_STAT) & (STAT_CC | STAT_CTO)) == 0x0), 5000)) {
        goto error;
    }

    value = INW(baseAddress + MMCHS_RSP10);
    if ((value >> 24) != 0) {
        LOG(ARCH, DEBUG, "MMC/SD HC() CMD ERROR: 0x%x", value);
    }

    if (((INW(baseAddress + MMCHS_STAT) & (STAT_CC)) != 0x0))
        return (cOk );

error:
    LOG(ARCH, DEBUG, "MMC/SD HC() Error on CMD %x, ARG %x STAT: %x", cmd, arg, INW(baseAddress + MMCHS_STAT));

    if (((INW(baseAddress + MMCHS_STAT) & (STAT_CTO)) != 0x0)) {
        /* reset command line if timed out */
        OUTW(baseAddress + MMCHS_SYSCTL, INW(baseAddress + MMCHS_SYSCTL) | (1 << 25));
        if (TIMEOUT_WAIT(((INW(baseAddress + MMCHS_SYSCTL) & (1 << 25)) != 0x0), 5000)) {
            LOG(ARCH, WARN, "MMC/SD HC() sendCommand(): Timout waiting for cmd line reset.");
        }
    }

    return (cError );
}

/*****************************************************************************
 * Method: OmapMMC_SD_HC::readBlock(unint4 blockNum, char* buffer, unint4 length)
 *
 * @description
 *
 *******************************************************************************/
ErrorT OmapMMC_SD_HC::readBlock(unint4 blockNum, char* buffer, unint4 length) {
    LOG(ARCH, DEBUG, "MMC/SD HC()::readBlock() blockNum: %d length: %d, buffer: %x", blockNum, length, buffer);
    ErrorT ret = cOk;
    bool timeout;
    unint4 mmc_stat;
    unint4* p_buf;

    this->acquire();

    timeout = TIMEOUT_WAIT((INW(baseAddress + MMCHS_PSTATE) & (1 << 1)), 300000);
    if (timeout) {
        LOG(ARCH, DEBUG, "MMC/SD HC()::readBlock, timeout waiting for data lines");
        ret = cError;
        goto out;
    }

    OUTW(baseAddress + MMCHS_BLK, 0x200 | (length << 16));

    /* if (isHighCapacity == 0)
     blockNum *= 512;*/

    length *= 512;

    // buffer must be 4 bytes aligned
    p_buf = reinterpret_cast<unint4*>(buffer);

#if 0
    DMA4_CCR(10) &= ~(1 << 7); /* disable channel */
    DMA4_CSDP(10) = 0x1C002;

    DMA4_CFN(10) = 0x1;

    DMA4_CCR(10) = 0xC401E;
    //DMA4_CCR(10) = 0x1000;

    DMA4_CEN(10) = 512 / 4;
    DMA4_CDSA(10) = (unint4) p_buf;
    DMA4_CSSA(10) = baseAddress + MMCHS_DATA;
    DMA4_CDEI(10) = 1;
    DMA4_CSEI(10) = 1;
    DMA4_CDFI(10) = 0x200;

    DMA4_CCR(10) = 0xC409E;
#endif

    OUTW(baseAddress + MMCHS_STAT, 0xffffffff);

    if (sendCommand(0x123A0036, blockNum) == cError) { /* auto cmd 12*/
        ret = cError;
        goto out;
    }

    timeout = TIMEOUT_WAIT(((INW(baseAddress + MMCHS_STAT) & (1 << 5)) == 0), 30000);

    mmc_stat = INW(baseAddress + MMCHS_STAT);
    if (mmc_stat & (1 << 15)) {
        LOG(ARCH, WARN, "MMC/SD HC()::readBlock failed: STAT: %x", mmc_stat);
        ret = cError;
        goto out;
    }

    if (timeout) {
        LOG(ARCH, WARN, "MMC/SD HC()::readBlock timeout..");
        ret = cError;
        goto out;
    }

    //while(DMA4_CCR(10) & (1<< 7));

    // check brr
    while ((length > 0)) {
        while ((INW(baseAddress + MMCHS_PSTATE) & (1 << 11)) == 0) {
        }
        *p_buf = INW(baseAddress + MMCHS_DATA);
        p_buf++;
        length -= 4;
    }

    TIMEOUT_WAIT(((INW(baseAddress + MMCHS_STAT) & (STAT_TC)) == 0), 30000);

    // CMD 12 (Stop transmission)
    //sendCommand(0x0c1a0000,0x0);

    out: this->release();

    return (ret);
}

/*****************************************************************************
 * Method: OmapMMC_SD_HC::writeBlock(unint4 blockNum, char* buffer, unint4 length)
 *
 * @description
 *
 *******************************************************************************/
ErrorT OmapMMC_SD_HC::writeBlock(unint4 blockNum, char* buffer, unint4 length) {
    LOG(ARCH, DEBUG, "MMC/SD HC()::writeBlock, blockNum: %d length: %d, buffer: %x", blockNum, length, buffer);
    ErrorT ret = cOk;
    unint4 mmc_stat;
    unint4* p_buf;

    bool timeout;
    this->acquire();

    /* wait until we are allowed to use data lines */
    timeout = TIMEOUT_WAIT((INW(baseAddress + MMCHS_PSTATE) & (1 << 1)), 300000);
    if (timeout) {
        LOG(ARCH, DEBUG, "MMC/SD HC()::writeBlock, timeout waiting for data lines");
        ret = cError;
        goto out;
    }

    OUTW(baseAddress + MMCHS_BLK, 0x200 | (length << 16));

    /*if (isHighCapacity == 0)
     blockNum *= 512;*/

    length *= 512;

    p_buf = reinterpret_cast<unint4*>(buffer);

#if 0
    DMA4_CCR(10) &= ~(1 << 7); /* disable channel */
    DMA4_CSDP(10) = 0x10182;

    DMA4_CFN(10) = 0x1;

    DMA4_CCR(10) = 0xC101D;
    //DMA4_CCR(10) = 0x1000;

    DMA4_CEN(10) = 512 / 4;
    DMA4_CSSA(10) = (unint4) p_buf;
    DMA4_CDSA(10) = baseAddress + MMCHS_DATA;
    DMA4_CDEI(10) = 1;
    DMA4_CSEI(10) = 1;
    DMA4_CDFI(10) = 0x200;

    DMA4_CCR(10) = 0xC109D;
#endif
    //DMA4_CCR(10) = 0x1080;
    //while(DMA4_CCR(10) & (1<< 7));

    /* Write multiple blocks .. AUTO CMD12, COUNT Blocks*/
    if (sendCommand(0x193a0026, blockNum) == cError) {
        ret = cError;
        goto out;
    }

    timeout = TIMEOUT_WAIT((INW(baseAddress + MMCHS_STAT) & (1 << 4)) == 0, 30000);

    // test error bit
    mmc_stat = INW(baseAddress + MMCHS_STAT);
    if (mmc_stat & (1 << 15)) {
        LOG(ARCH, WARN, "MMC/SD HC()::writeBlock failed: STAT: %x", mmc_stat);
        ret = cError;
        goto out;
    }

    if (timeout) {
        LOG(ARCH, WARN, "MMC/SD HC()::writeBlock timeout..");
        ret = cError;
        goto out;
    }

    OUTW(baseAddress + MMCHS_STAT, 0xffffffff);
    //while(DMA4_CCR(10) & (1<< 7));

    // no error.. start writing data .. first wait until we are allowed to write
    // now write everything
    while ((length > 0)) {
        while ((INW(baseAddress + MMCHS_PSTATE) & (1 << 10)) == 0) {
        }
        OUTW(baseAddress + MMCHS_DATA, *p_buf);
        p_buf++;
        length -= 4;
    }

    timeout = TIMEOUT_WAIT(((INW(baseAddress + MMCHS_STAT) & (STAT_TC)) == 0), 300000);

    // test error bit
    mmc_stat = INW(baseAddress + MMCHS_STAT);
    if (mmc_stat & (1 << 15)) {
        LOG(ARCH, WARN, "MMC/SD HC():: transfer failed: STAT: %x", mmc_stat);
        ret = cError;
        goto out;
    }

    if (timeout) {
        LOG(ARCH, WARN, "MMC/SD HC()::transfer timeout..");
        ret = cError;
        goto out;
    }

    // CMD 12 (Stop transmission)
    // sendCommand(0x0c1a0000,0x0);

    out: this->release();

    return (ret);
}
