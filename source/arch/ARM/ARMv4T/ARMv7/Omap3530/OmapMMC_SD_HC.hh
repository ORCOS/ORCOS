/*
 * MMC.hh
 *
 *  Created on: 15.09.2013
 *    Copyright &  Author: dbaldin
 */

#ifndef MMC_HH_
#define MMC_HH_

#include "hal/BlockDeviceDriver.hh"


typedef enum {
    Unknown,
    SD1_0,
    SD2_0,
    MMC,
    SDIO
} T_CardType;


class OmapMMC_SD_HC: public BlockDeviceDriver {
private:
    unint4 baseAddress;

    // relative card address of this sd/mmc card
    unint2 rca;

    unint1 isHighCapacity;

    T_CardType card_type;

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
