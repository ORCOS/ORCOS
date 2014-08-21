/*
 * TWL4030.cc
 *
 *  Created on: 07.09.2013
 *      Author: dbaldin
 */

#include "TWL4030.hh"
#include "kernel/Kernel.hh"

extern void kwait(int milliseconds);
extern Kernel* theOS;

ErrorT TWL4030::i2c_read_u8(unint1 i2c_num, unint1 *val, unint1 reg) {

    if (i2c_dev == 0)
    {
        return (cError );
    }

    char i2c_val[8];
    i2c_val[0] = i2c_num;  // register group 4b
    i2c_val[1] = reg;
    unint4 length = 1;
    ErrorT err = i2c_dev->readBytes(i2c_val, length);

    *val = i2c_val[0];
    return (err);
}

ErrorT TWL4030::i2c_write_u8(unint1 i2c_num, unint1 val, unint1 reg) {

    if (i2c_dev == 0)
    {
        return (cError );
    }

    char i2c_val[8];
    i2c_val[0] = i2c_num;  // register group 4b
    i2c_val[1] = reg;	// VAUX2_DEDICATED register
    i2c_val[2] = val;
    unint4 length = 3;
    ErrorT err = i2c_dev->writeBytes(i2c_val, length);

    return (err);
}

void TWL4030::power_reset_init(void) {
    unint1 val = 0;
    if (i2c_read_u8(TWL4030_CHIP_PM_MASTER, &val, TWL4030_PM_MASTER_P1_SW_EVENTS) != cOk)
    {
        LOG(ARCH, WARN, "TWL4030() failed to write the power register. Could not initialize hardware reset");
    }
    else
    {
        val |= TWL4030_PM_MASTER_SW_EVENTS_STOPON_PWRON;
        if (i2c_write_u8(TWL4030_CHIP_PM_MASTER, val, TWL4030_PM_MASTER_P1_SW_EVENTS) != cOk)
        {
            LOG(ARCH, WARN,"TWL4030() failed to write the power register. Could not initialize hardware reset");
        }
    }
}

void TWL4030::pmrecv_vsel_cfg(unint1 vsel_reg, unint1 vsel_val, unint1 dev_grp, unint1 dev_grp_sel) {
    int ret;

    /* Select the Voltage */
    ret = i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, vsel_val, vsel_reg);
    if (ret != cOk)
    {
        LOG(ARCH, WARN, "TWL4030()Could not write vsel to reg %02x (%d)",vsel_reg, ret);
        return;
    }

    /* Select the Device Group (enable the supply if dev_grp_sel != 0) */
    ret = i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, dev_grp_sel, dev_grp);
    if (ret != cOk)
        LOG(ARCH, WARN, "TWL4030()Could not write grp_sel to reg %02x (%d)", dev_grp, ret);
}

/*static inline int gpio_twl4030_write(unint1 address, unint1 data)
 {
 return twl4030_i2c_write_u8(TWL4030_MODULE_GPIO, data, address);
 }*/

int TWL4030::usb_write(unint1 address, unint1 data) {
    int ret;

    ret = i2c_write_u8(TWL4030_CHIP_USB, data, address);
    if (ret != 0)
        LOG(ARCH, WARN, "TWL4030()USB_Write[0x%x] Error %d", address, ret);

    return (ret);
}

int TWL4030::usb_read(unint1 address) {
    unint1 data;
    int ret;

    ret = i2c_read_u8(TWL4030_CHIP_USB, &data, address);
    if (ret == 0)
        ret = data;
    else
    LOG(ARCH, WARN, "TWL4030()USB_Read[0x%x] Error %d", address, ret);

    return (ret);
}

void TWL4030::usb_ldo_init(void) {
    /* Enable writing to power configuration registers */
    i2c_write_u8(TWL4030_CHIP_PM_MASTER, 0xC0,
    TWL4030_PM_MASTER_PROTECT_KEY);
    i2c_write_u8(TWL4030_CHIP_PM_MASTER, 0x0C,
    TWL4030_PM_MASTER_PROTECT_KEY);

    // already done by boot rom code
    /*twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER, 1 << 1 | 1 << 3,
     TWL4030_PM_MASTER_CFG_BOOT);*/

    /* put VUSB3V1 LDO in active state */
    i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, 0x00,
    TWL4030_PM_RECEIVER_VUSB_DEDICATED2);

    /* input to VUSB3V1 LDO is from VBAT, not VBUS */
    i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, 0x14,
    TWL4030_PM_RECEIVER_VUSB_DEDICATED1);

    /* turn on 3.1V regulator */
    i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, 0x20,
    TWL4030_PM_RECEIVER_VUSB3V1_DEV_GRP);
    i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, 0x00,
    TWL4030_PM_RECEIVER_VUSB3V1_TYPE);

    /* turn on 1.5V regulator */
    i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, 0x20,
    TWL4030_PM_RECEIVER_VUSB1V5_DEV_GRP);
    i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, 0x00,
    TWL4030_PM_RECEIVER_VUSB1V5_TYPE);

    /* turn on 1.8V regulator */
    i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, 0x20,
    TWL4030_PM_RECEIVER_VUSB1V8_DEV_GRP);
    i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, 0x00,
    TWL4030_PM_RECEIVER_VUSB1V8_TYPE);

    /* disable access to power configuration registers */
    i2c_write_u8(TWL4030_CHIP_PM_MASTER, 0x00,
    TWL4030_PM_MASTER_PROTECT_KEY);
}

void TWL4030::phy_power(void) {
    unint1 pwr, clk;

    /* Power the PHY, TODO check for error.. */
    pwr = (unint1) usb_read(TWL4030_USB_PHY_PWR_CTRL);
    pwr = (unint1) (pwr & ~PHYPWD);
    usb_write(TWL4030_USB_PHY_PWR_CTRL, pwr);
    /* Enable clocks */
    clk = (unint1) usb_read(TWL4030_USB_PHY_CLK_CTRL);
    clk |= CLOCKGATING_EN | CLK32K_EN;
    usb_write(TWL4030_USB_PHY_CLK_CTRL, clk);
}

/*
 * Initiaze the ULPI interface
 * ULPI : Universal Transceiver Macrocell Low Pin Interface
 * An interface between the USB link controller like musb and the
 * the PHY or transceiver that drives the actual bus.
 */
int TWL4030::usb_ulpi_init(void) {
    long timeout = 1000 * 1000; /* 1 sec */
    ;
    unint1 clk, sts, pwr;

    /* twl4030 ldo init */
    usb_ldo_init();

    /* Enable the twl4030 phy */
    phy_power();

    /* Enable DPLL to access PHY registers over I2C */
    clk = (unint1) usb_read(TWL4030_USB_PHY_CLK_CTRL);
    clk |= REQ_PHY_DPLL_CLK;
    usb_write(TWL4030_USB_PHY_CLK_CTRL, clk);

    /* Check if the PHY DPLL is locked */
    sts = (unint1) usb_read(TWL4030_USB_PHY_CLK_CTRL_STS);
    while (!(sts & PHY_DPLL_CLK) && 0 < timeout)
    {
        kwait(1);
        sts = (unint1) usb_read(TWL4030_USB_PHY_CLK_CTRL_STS);
        timeout -= 10;
    }

    /* Final check */
    sts = (unint1) usb_read(TWL4030_USB_PHY_CLK_CTRL_STS);
    if (!(sts & PHY_DPLL_CLK))
    {
        LOG(ARCH, WARN,"TWL4030(): Error:TWL4030:USB Timeout setting PHY DPLL clock");
        return (-1);
    }

    /*
     * There are two circuit blocks attached to the PHY,
     * Carkit and USB OTG.  Disable Carkit and enable USB OTG
     */
    usb_write(TWL4030_USB_IFC_CTRL_CLR, CARKITMODE);
    pwr = (unint1) usb_read(TWL4030_USB_POWER_CTRL);
    pwr |= OTG_ENAB;
    usb_write(TWL4030_USB_POWER_CTRL_SET, pwr);

    /* Clear the opmode bits to ensure normal encode */
    usb_write(TWL4030_USB_FUNC_CTRL_CLR, OPMODE_MASK);

    /* Clear the xcvrselect bits to enable the high speed transeiver */
    usb_write(TWL4030_USB_FUNC_CTRL_CLR, XCVRSELECT_MASK);

    /* Let ULPI control the DPLL clock */
    clk = (unint1) usb_read(TWL4030_USB_PHY_CLK_CTRL);
    clk = (unint1) (clk & ~REQ_PHY_DPLL_CLK);
    usb_write(TWL4030_USB_PHY_CLK_CTRL, clk);

    return (0);
}

/**
 * Activates the general power supply for VAUX3, VPLL2, VDAC
 */
void TWL4030::power_init(void) {
    /* set VAUX3 to 2.8V */
    pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX3_DEDICATED,
    TWL4030_PM_RECEIVER_VAUX3_VSEL_28,
    TWL4030_PM_RECEIVER_VAUX3_DEV_GRP,
    TWL4030_PM_RECEIVER_DEV_GRP_P1);

    /* set VPLL2 to 1.8V */
    pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VPLL2_DEDICATED,
    TWL4030_PM_RECEIVER_VPLL2_VSEL_18,
    TWL4030_PM_RECEIVER_VPLL2_DEV_GRP,
    TWL4030_PM_RECEIVER_DEV_GRP_ALL);

    /* set VDAC to 1.8V */
    pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VDAC_DEDICATED,
    TWL4030_PM_RECEIVER_VDAC_VSEL_18,
    TWL4030_PM_RECEIVER_VDAC_DEV_GRP,
    TWL4030_PM_RECEIVER_DEV_GRP_P1);

}

void TWL4030::power_mmc_init(void) {
    /* Set VMMC1 to 3.15 Volts */
    pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VMMC1_DEDICATED,
    TWL4030_PM_RECEIVER_VMMC1_VSEL_32,
    TWL4030_PM_RECEIVER_VMMC1_DEV_GRP,
    TWL4030_PM_RECEIVER_DEV_GRP_P1);
}

TWL4030::TWL4030(T_TWL4030_Init *init) :  GenericDeviceDriver(false, "twl4030") {

    i2c_dev = (Omap3530i2c*) theOS->getFileManager()->getResource(init->I2CDeviceName);
    if (i2c_dev == 0)
    {
        LOG(ARCH, ERROR, "TWL4030(): I2C device '%s' not found. Not initializing TWL4030.",init->I2CDeviceName);
        return;
    }

    power_init();
    power_reset_init();

}

TWL4030::~TWL4030() {

}

