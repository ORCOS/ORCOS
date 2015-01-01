/*
 * MMC.hh
 *
 *  Created on: 15.09.2013
 *    Copyright &  Author: dbaldin
 */

#ifndef MMC_HH_
#define MMC_HH_

#include "hal/BlockDeviceDriver.hh"

// offsets to registers

#define MMCHS_SYSCONFIG 0x10
#define MMCHS_SYSSTATUS 0x14
#define MMCHS_CSRE 0x24
#define MMCHS_CON 0x2c
#define MMCHS_BLK 0x104
#define MMCHS_ARG 0x108
#define MMCHS_CMD 0x10c

#define MMCHS_RSP10 0x110
#define MMCHS_RSP32 0x114
#define MMCHS_RSP54 0x118
#define MMCHS_RSP76 0x11c
#define MMCHS_DATA 0x120

#define MMCHS_PSTATE 0x124
#define MMCHS_HCTL 0x128

#define MMCHS_SYSCTL 0x12c
#define MMCHS_STAT 0x130
#define MMCHS_IE 0x134
#define MMCHS_ISE 0x138

#define MMCHS_AC12 0x13c
#define MMCHS_CAPA 0x140
#define MMCHS_CUR_CAPA 0x148
#define MMCHS_REV 0x1fc

#define STAT_CC (1 << 0)
#define STAT_TC (1 << 1)
#define STAT_BGE (1 << 2)
#define STAT_BWR (1 << 4)
#define STAT_BRR (1 << 5)
#define STAT_CIRQ (1 << 8)
#define STAT_OBI (1 << 9)
#define STAT_ERRI (1 << 15)
#define STAT_CTO (1 << 16)

class OmapMMC_SD_HC: public BlockDeviceDriver {
private:
    unint4 baseAddress;

    // relative card address of this sd/mmc card
    unint2 rca;

    unint1 isHighCapacity;

public:
    /*****************************************************************************
     * Method: OmapMMC_SD_HC(T_OmapMMC_SD_HC_Init *init)
     *
     * @description
     *  Constructor called on board initialization
     *******************************************************************************/
    explicit OmapMMC_SD_HC(T_OmapMMC_SD_HC_Init *init);

    /*
     * Initializes the card and stars card identification
     */
    /*****************************************************************************
     * Method: init()
     *
     * @description
     *  Initializes the card and stars card identification
     *******************************************************************************/
    void init();

    ~OmapMMC_SD_HC();

    /*****************************************************************************
     * Method: sendCommand(unint4 cmd, unint4 arg)
     *
     * @description
     *  Send the given MMC/SD Card command with argument to the
     *  attached SD/MMS card.
     *******************************************************************************/
    ErrorT sendCommand(unint4 cmd, unint4 arg);

    /*****************************************************************************
     * Method: readBlock(unint4 blockNum, char* buffer, unint4 length)
     *
     * @description
     *
     *******************************************************************************/
    ErrorT readBlock(unint4 blockNum, char* buffer, unint4 length);

    /*****************************************************************************
     * Method: writeBlock(unint4 blockNum, char* buffer, unint4 length)
     *
     * @description
     *
     *******************************************************************************/
    ErrorT writeBlock(unint4 blockNum, char* buffer, unint4 length);
};

#endif /* MMC_HH_ */
