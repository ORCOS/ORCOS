/*
 * HighSpeedUSBHostController.cc
 *
 *  Created on: 04.02.2013
 *      Author: danielb
 */

#include "HighSpeedUSBHostController.hh"

#include "inc/memio.h"
#include "kernel/Kernel.hh"

extern Kernel* theOS;

extern void kwait(int milliseconds);

#define UHH_REVISION 0x48064000
#define UHH_HOSTCONFIG 0x48064040

#define TLL_CHANNEL_CONF 0x48062040

#define CM_FCLKEN_USBHOST 0x48005400
#define CM_ICLKEN_USBHOST 0x48005410
#define CM_AUTOIDLE_USBHOST 0x48005430
#define CM_AUTOIDLE3_CORE 0x48004a38

#define CM_FCLKEN3_CORE 0x48004a08
#define CM_ICLKEN3_CORE 0x48004a18

#define SETBITS(a,UP,LOW,val) a = ((a & ~(( (1 << (UP - LOW + 1)) -1) << LOW)) | ((val & ((1 << (UP - LOW + 1)) -1)) << LOW) )


#define ADDRESS_GROUP_ID3 0x4a
#define LEDEN_PHYSICAL_ADDRESS 0xEE

#define TWL4030_PM_RECEIVER_DEV_GRP_P1                  0x20
#define TWL4030_PM_RECEIVER_DEV_GRP_ALL                 0xE0

#define LEDAON (1 | (1 << 1))
//#define LEDAPWM (1 << 5) | (1 << 6)
#define LEDAPWM ((1 << 4) | (1 << 5))

#define CONFIG_OMAP_EHCI_PYH1_RESET_GPIO 147

#define ULPI_FUNCTION_CONTROL 0x4
#define ULPI_FUNCTION_RESET (1 << 5)



int omap3_ulpi_write_register(unint1 port, unint1 reg, unint1 data) {
	unint4 reg_val = ( 2 << 22) | ((port & 0xf) << 24) | (1 << 31)  | ((reg & 0x3F) << 16) | (data & 0xff);
	OUTW(0x480648a4,reg_val);

	// wait till completion
	volatile int4 timeout = 0;
	unint4 ulpi_reg;
	do {
		ulpi_reg = INW(0x480648a4);
		timeout++;
	}
	while ((ulpi_reg & (1 << 31)) && (timeout < 10000));

	if (timeout < 10000) {
		printf("omap3_ulpi_write_register: %x finished after %d tries..\n",reg_val,timeout);
		return cOk;
	}
	else {
		//printf("Timeout writing ulpi register value %x..\n",reg_val);
		return cError;
	}
}

int omap3_ulpi_read_register(unint1 port, unint1 reg, unint1 &data) {

	unint4 reg_val = ( 0x3 << 22) | ((port & 0xf) << 24) | (1 << 31)  | ((reg & 0x3F) << 16);
	data = 0;
	OUTW(0x480648a4,reg_val);

	volatile int4 timeout = 0x0;
	unint4 ulpi_reg;
	do {
		ulpi_reg = INW(0x480648a4);
		timeout++;
	}
	while ((ulpi_reg & (1 << 31)) && (timeout < 10000));

	if (timeout < 10000) {
		data = INW(0x480648a4) & 0xff;
		printf("omap3_ulpi_read_register: %x finished after %d tries. value: %x\n",reg_val,timeout,data);
		return cOk;
	}
	else {
		data = 0;
		//printf("Timeout reading ulpi register..\r");
		return cError;
	}
}



#define CONFIG_USB_ULPI_TIMEOUT 1000	/* timeout in us */
#define ULPI_ERROR	(1 << 8) /* overflow from any register value */

#define readw(addr)		(*(volatile unint2 *) (addr))
#define readl(addr)		(*(volatile unint4 *) (addr))
#define writew(b,addr)		((*(volatile unint2 *) (addr)) = (b))
#define writel(b,addr)		((*(volatile unint4 *) (addr)) = (b))

#define OMAP_ULPI_WR_OPSEL	(3 << 21)
#define OMAP_ULPI_ACCESS	(1 << 31)

#define CONFIG_OMAP_EHCI_PHY1_RESET_GPIO_PIN        19

#define ULPI_VID_HIGH 0x01
#define ULPI_VID_LOW 0x00

#define ULPI_FUNCTION_CONTROL 0x4
#define ULPI_FUNCTION_RESET (1 << 5)



struct ulpi_viewport {
	unint4 viewport_addr;
	unint4 port_num;
};

/*
 * Wait for the ULPI Access to complete
 */
static int ulpi_wait(struct ulpi_viewport *ulpi_vp, unint4 mask)
{
	int timeout = CONFIG_USB_ULPI_TIMEOUT;

	while (--timeout) {
		if ((readl(ulpi_vp->viewport_addr) & mask))
			return 0;

		kwait(1);
	}

	return ULPI_ERROR;
}

/*
 * Wake the ULPI PHY up for communication
 *
 * returns 0 on success.
 */
static int ulpi_wakeup(struct ulpi_viewport *ulpi_vp)
{
	int err;

	if (readl(ulpi_vp->viewport_addr) & OMAP_ULPI_ACCESS)
		return 0; /* already awake */

	writel(OMAP_ULPI_ACCESS, ulpi_vp->viewport_addr);

	err = ulpi_wait(ulpi_vp, OMAP_ULPI_ACCESS);
	if (err)
		printf("ULPI wakeup timed out\n");

	return err;
}

/*
 * Issue a ULPI read/write request
 */
static int ulpi_request(struct ulpi_viewport *ulpi_vp, unint4 value)
{
	int err;

	err = ulpi_wakeup(ulpi_vp);
	if (err)
		return err;

	writel(value, ulpi_vp->viewport_addr);

	err = ulpi_wait(ulpi_vp, OMAP_ULPI_ACCESS);
	if (err)
		printf("ULPI request timed out\n");

	return err;
}

int ulpi_write(struct ulpi_viewport *ulpi_vp, unint1 *reg, unint4 value)
{
	unint4 val = ((ulpi_vp->port_num & 0xf) << 24) |
			OMAP_ULPI_WR_OPSEL | ((unint4)reg << 16) | (value & 0xff);

	return ulpi_request(ulpi_vp, val);
}

unint4 ulpi_read(struct ulpi_viewport *ulpi_vp, unint1 *reg)
{
	int err;
	unint4 val = ((ulpi_vp->port_num & 0xf) << 24) |
			 OMAP_ULPI_WR_OPSEL | ((unint4)reg << 16);

	err = ulpi_request(ulpi_vp, val);
	if (err)
		return err;

	return readl(ulpi_vp->viewport_addr) & 0xff;
}

int ulpi_init(struct ulpi_viewport *ulpi_vp)
{
	unint4 val, id = 0;
	unint1 *reg = (unint1*) 0x3;
	int i;

	/* Assemble ID from four ULPI ID registers (8 bits each). */
	for (i = 0; i < 4; i++) {
		val = ulpi_read(ulpi_vp, reg - i);
		if (val == ULPI_ERROR)
			return val;

		id = (id << 8) | val;
	}

	/* Split ID into vendor and product ID. */
	//printf("ULPI transceiver ID 0x%04x:0x%04x\n\r", id >> 16, id & 0xffff);

	return 0;
}


#define ULPI_FC_RESET			(1 << 5)

static int ulpi_reset_wait(struct ulpi_viewport *ulpi_vp)
{
	unint4 val;
	int timeout = CONFIG_USB_ULPI_TIMEOUT;

	/* Wait for the RESET bit to become zero */
	while (--timeout) {
		/*
		 * This function is generic and suppose to work
		 * with any viewport, so we cheat here and don't check
		 * for the error of ulpi_read(), if there is one, then
		 * there will be a timeout.
		 */
		unint1 *reg = (unint1*) 0x4;

		val = ulpi_read(ulpi_vp, reg);
		if (!(val & ULPI_FC_RESET))
			return 0;

		kwait(1);
	}

	printf("ULPI: %s: reset timed out\n", __func__);

	return ULPI_ERROR;
}






static inline void omap_ehci_phy_reset(int on, int delay)
{
         /*
          * Refer ISSUE1:
          * Hold the PHY in RESET for enough time till
          * PHY is settled and ready
          */
         if (delay && !on)
                 kwait(delay);


         if (!on) {
        	OUTW(0x49056000 + 0x94,(1 << CONFIG_OMAP_EHCI_PHY1_RESET_GPIO_PIN) | (1 << (CONFIG_OMAP_EHCI_PHY1_RESET_GPIO_PIN-1)) | (1 << (CONFIG_OMAP_EHCI_PHY1_RESET_GPIO_PIN+1)) );  // set pin
         } else
            OUTW(0x49056000 + 0x90,(1 << CONFIG_OMAP_EHCI_PHY1_RESET_GPIO_PIN) | (1 << (CONFIG_OMAP_EHCI_PHY1_RESET_GPIO_PIN-1)) | (1 << (CONFIG_OMAP_EHCI_PHY1_RESET_GPIO_PIN+1))); // clear pin

         OUTW(0x49056000 + 0x34,INW(0x49056000 + 0x34) & ~(1 << CONFIG_OMAP_EHCI_PHY1_RESET_GPIO_PIN));


         /* Hold the PHY in RESET for enough time till DIR is high */
         /* Refer: ISSUE1 */
         if (delay && on)
                 kwait(delay);
}



//#define TWL4030_GPIO_MAX 18


HighSpeedUSBHostController::HighSpeedUSBHostController(T_HighSpeedUSBHostController_Init *init) :
		USB_EHCI_Host_Controller(0x48064800)
{
	// do some omap stuff here
	// be sure the functional clocks of the usb host subsystem are enabled!
	OUTW(CM_ICLKEN_USBHOST,0x1);
	OUTW(CM_FCLKEN_USBHOST,0x3);

	OUTW(CM_ICLKEN3_CORE,1 << 2);
	OUTW(CM_FCLKEN3_CORE,1 << 2);

	OUTW(CM_AUTOIDLE_USBHOST,0x0);
	//OUTW(CM_AUTOIDLE3_CORE,0x0);

	// this line will cause a compile error if no TWL4030 exists!
	TWL4030* twl4030 = theOS->getBoard()->getExtPowerControl();

	// be sure the twl4030 is powering the USB PHY
	twl4030->usb_ulpi_init();
	twl4030->power_mmc_init();

	// TPS65950 has the same sw interface as the twl4030
	// VAUX2 is different for TPS65950!!
    /* Set VAUX2 to 1.8V for EHCI PHY */
	twl4030->pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX2_DEDICATED,
						  TWL4030_PM_RECEIVER_VAUX2_VSEL_18,
						  TWL4030_PM_RECEIVER_VAUX2_DEV_GRP,
						  TWL4030_PM_RECEIVER_DEV_GRP_P1);


    // USB PHY power is provided by LEDA and LEDB of the TWL4030 chip
    // active power now!

	  /* LED */
	   #define TWL4030_LED_LEDEN                               0xEE
	   #define TWL4030_LED_LEDEN_LEDAON                        (1 << 0)
	   #define TWL4030_LED_LEDEN_LEDBON                        (1 << 1)
	   #define TWL4030_LED_LEDEN_LEDAPWM                       (1 << 4)
	   #define TWL4030_LED_LEDEN_LEDBPWM                       (1 << 5)

	  Omap3530i2c* i2c_dev = (Omap3530i2c*) theOS->getFileManager()->getResource(init->I2CDeviceName);

	  	if (i2c_dev == 0) {
	  		LOG(ARCH,ERROR,(ARCH,ERROR,"HighSpeedUSBHostController() ERROR: i2c needed for power control of usb hub!"));
	  		return;
	  	}

	  	char i2c_val[8];


	  	i2c_val[0] = 0x49;
		i2c_val[1] = 0x45;
		i2c_val[2] = 0;

		 unint4 length = 3;
		 ErrorT err = i2c_dev->writeBytes(i2c_val,length);

		if (err != cOk) {
			LOG(ARCH,ERROR,(ARCH,ERROR,"HighSpeedUSBHostController() ERROR: deactivating vibra failed.."));
			return;
		}

		i2c_val[0] = ADDRESS_GROUP_ID3;
		i2c_val[1] = 0xef;
		i2c_val[2] = 1;

		length = 3;
		 err = i2c_dev->writeBytes(i2c_val,length);

		if (err != cOk) {
			LOG(ARCH,ERROR,(ARCH,ERROR,"HighSpeedUSBHostController() ERROR: setting PWMAON failed.."));
			return;
		}

		i2c_val[0] = ADDRESS_GROUP_ID3;
		i2c_val[1] = 0xf0;
		i2c_val[2] = 1;

		 length = 3;
		 err = i2c_dev->writeBytes(i2c_val,length);

		if (err != cOk) {
			LOG(ARCH,ERROR,(ARCH,ERROR,"HighSpeedUSBHostController() ERROR: setting PWMAOFF failed.."));
			return;
		}

		i2c_val[0] = ADDRESS_GROUP_ID3;
		i2c_val[1] = 0xf1;
		i2c_val[2] = 1;

		length = 3;
		 err = i2c_dev->writeBytes(i2c_val,length);

		if (err != cOk) {
			LOG(ARCH,ERROR,(ARCH,ERROR,"HighSpeedUSBHostController() ERROR: setting PWMBON failed.."));
			return;
		}

		i2c_val[0] = ADDRESS_GROUP_ID3;
		i2c_val[1] = 0xf2;
		i2c_val[2] = 1;

		 length = 3;
		 err = i2c_dev->writeBytes(i2c_val,length);

		if (err != cOk) {
			LOG(ARCH,ERROR,(ARCH,ERROR,"HighSpeedUSBHostController() ERROR: setting PWMBOFF failed.."));
			return;
		}


	  // power on the usb hub!
	  	i2c_val[0] = ADDRESS_GROUP_ID3;
	  	i2c_val[1] = TWL4030_LED_LEDEN;
	  	i2c_val[2] = TWL4030_LED_LEDEN_LEDAON | TWL4030_LED_LEDEN_LEDBON | TWL4030_LED_LEDEN_LEDAPWM | TWL4030_LED_LEDEN_LEDBPWM ;
		//i2c_val[2] = TWL4030_LED_LEDEN_LEDAON | TWL4030_LED_LEDEN_LEDAPWM ;

	  	length = 3;
	  	err = i2c_dev->writeBytes(i2c_val,length);

	  	if (err != cOk) {
	  		LOG(ARCH,ERROR,(ARCH,ERROR,"HighSpeedUSBHostController() ERROR: hub power on failed.."));
	  		return;
	  	}


	/* Put the PHY in RESET */
	// set phy in reset mode
	// this is done by setting the gpio pin 147 voltage to high
	// the gpio is hard wired to the phy reset pin
	omap_ehci_phy_reset(1, 1);


	unint1 uhh_revision = INB(UHH_REVISION);
	LOG(ARCH,INFO,(ARCH,INFO,"HighSpeedUSBHostController() UHH_REVISION: %x",uhh_revision));


	// reset UHH module
	OUTW(0x48064010,0x2 );
	// wait till reset
	while (!INW(0x48064014)) {};
	LOG(ARCH,INFO,(ARCH,INFO,"HighSpeedUSBHostController() UHH module reset successful"));

	// reset TLLs
	OUTW(0x48062010, 1 << 1 );
	// wait till reset
	while (!(INW(0x48062014) & 1)) {};
	LOG(ARCH,INFO,(ARCH,INFO,"HighSpeedUSBHostController() TLL reset successful"));

	// write tll config
	OUTW(0x48062010,0x1A );

	// write uhh sysconfig
	OUTW(0x48064010,  1 << 2 | 1 << 3 | 1 << 8 | 1 << 12);


	// set port ulpi bypassing
	unint4 host_config = 0;

	// set host config register .. enable tll bypass to use ulpi phy
	OUTW(UHH_HOSTCONFIG,0x21c);

	/* Ensure that BYPASS is set */
	volatile int timeout = 20;
	while ( (INW(UHH_HOSTCONFIG) & 1) && (timeout) ) {timeout--; kwait(1);};

	host_config = INW(UHH_HOSTCONFIG);
	LOG(ARCH,INFO,(ARCH,INFO,"HighSpeedUSBHostController() UHH_HOSTCONFIG: %x",host_config));

	// get phy out of reset
	omap_ehci_phy_reset(0, 5);

	// disable undocumented feature which interferes with usb hub suspend state
	OUTW(0x480648a0,1<<5);

	//do ULPI initialization

	struct ulpi_viewport ulpi_vp;

	ulpi_vp.viewport_addr = (unint4) 0x480648a4;
	ulpi_vp.port_num = 2;
	int ret = ulpi_wakeup(&ulpi_vp);

	if (ret == 0) {
		LOG(ARCH,INFO,(ARCH,INFO,"HighSpeedUSBHostController() ULPI PHY is awake.."));
	}
	else {
		LOG(ARCH,ERROR,(ARCH,ERROR,"HighSpeedUSBHostController() ULPI PHY not operating .. returning.."));
		return;
	}

	ulpi_init(&ulpi_vp);

	unint1 *reg = (unint1*) ULPI_FUNCTION_CONTROL;

	ulpi_write(&ulpi_vp,reg,ULPI_FUNCTION_RESET);
	ulpi_reset_wait(&ulpi_vp);


	// call the Init method of the EHCI in baseclass to initialize the controller hardware
	this->Init();

}

HighSpeedUSBHostController::~HighSpeedUSBHostController() {

}

