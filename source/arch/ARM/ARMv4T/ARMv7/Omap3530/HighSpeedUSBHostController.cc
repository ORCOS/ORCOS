/*
 * HighSpeedUSBHostController.cc
 *
 *  Created on: 04.02.2013
 *      Author: danielb
 */

#include "HighSpeedUSBHostController.hh"

#include "inc/memio.h"
#include "kernel/Kernel.hh"

// define a huge memory area useable by the ehci controller
static unint1 ehci_memory[8000] __attribute__((aligned(0x1000)));
extern Kernel* theOS;

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

static void udelay(int delay){
	while (delay) {delay--;};
}

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

		udelay(1000);
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

		udelay(1);
	}

	printf("ULPI: %s: reset timed out\n", __func__);

	return ULPI_ERROR;
}



#define CONFIG_OMAP_EHCI_PHY1_RESET_GPIO_PIN        19


static inline void omap_ehci_phy_reset(int on, int delay)
{
         /*
          * Refer ISSUE1:
          * Hold the PHY in RESET for enough time till
          * PHY is settled and ready
          */
         if (delay && !on)
                 udelay(delay);


         if (!on) {
        	OUTW(0x49056000 + 0x94,(1 << CONFIG_OMAP_EHCI_PHY1_RESET_GPIO_PIN) | (1 << (CONFIG_OMAP_EHCI_PHY1_RESET_GPIO_PIN-1)) | (1 << (CONFIG_OMAP_EHCI_PHY1_RESET_GPIO_PIN+1)) );  // set pin
         } else
            OUTW(0x49056000 + 0x90,(1 << CONFIG_OMAP_EHCI_PHY1_RESET_GPIO_PIN) | (1 << (CONFIG_OMAP_EHCI_PHY1_RESET_GPIO_PIN-1)) | (1 << (CONFIG_OMAP_EHCI_PHY1_RESET_GPIO_PIN+1))); // clear pin

         OUTW(0x49056000 + 0x34,INW(0x49056000 + 0x34) & ~(1 << CONFIG_OMAP_EHCI_PHY1_RESET_GPIO_PIN));


         /* Hold the PHY in RESET for enough time till DIR is high */
         /* Refer: ISSUE1 */
         if (delay && on)
                 udelay(delay);
}


/* I2C chip addresses */

/* USB */
#define TWL4030_CHIP_USB				0x48
/* AUD */
#define TWL4030_CHIP_AUDIO_VOICE			0x49
#define TWL4030_CHIP_GPIO				0x49
#define TWL4030_CHIP_INTBR				0x49
#define TWL4030_CHIP_PIH				0x49
#define TWL4030_CHIP_TEST				0x49
/* AUX */
#define TWL4030_CHIP_KEYPAD				0x4a
#define TWL4030_CHIP_MADC				0x4a
#define TWL4030_CHIP_INTERRUPTS				0x4a
#define TWL4030_CHIP_LED				0x4a
#define TWL4030_CHIP_MAIN_CHARGE			0x4a
#define TWL4030_CHIP_PRECHARGE				0x4a
#define TWL4030_CHIP_PWM0				0x4a
#define TWL4030_CHIP_PWM1				0x4a
#define TWL4030_CHIP_PWMA				0x4a
#define TWL4030_CHIP_PWMB				0x4a
/* POWER */
#define TWL4030_CHIP_BACKUP				0x4b
#define TWL4030_CHIP_INT				0x4b
#define TWL4030_CHIP_PM_MASTER				0x4b
#define TWL4030_CHIP_PM_RECEIVER			0x4b
#define TWL4030_CHIP_RTC				0x4b
#define TWL4030_CHIP_SECURED_REG			0x4b

/* Register base addresses */

/* USB */
#define TWL4030_BASEADD_USB				0x0000
/* AUD */
#define TWL4030_BASEADD_AUDIO_VOICE			0x0000
#define TWL4030_BASEADD_GPIO				0x0098
#define TWL4030_BASEADD_INTBR				0x0085
#define TWL4030_BASEADD_PIH				0x0080
#define TWL4030_BASEADD_TEST				0x004C
/* AUX */
#define TWL4030_BASEADD_INTERRUPTS			0x00B9
#define TWL4030_BASEADD_LED				0x00EE
#define TWL4030_BASEADD_MADC				0x0000
#define TWL4030_BASEADD_MAIN_CHARGE			0x0074
#define TWL4030_BASEADD_PRECHARGE			0x00AA
#define TWL4030_BASEADD_PWM0				0x00F8
#define TWL4030_BASEADD_PWM1				0x00FB
#define TWL4030_BASEADD_PWMA				0x00EF
#define TWL4030_BASEADD_PWMB				0x00F1
#define TWL4030_BASEADD_KEYPAD				0x00D2
/* POWER */
#define TWL4030_BASEADD_BACKUP				0x0014
#define TWL4030_BASEADD_INT				0x002E
#define TWL4030_BASEADD_PM_MASTER			0x0036
#define TWL4030_BASEADD_PM_RECIEVER			0x005B
#define TWL4030_BASEADD_RTC				0x001C
#define TWL4030_BASEADD_SECURED_REG			0x0000

/*
 * Power Management Master
 */
#define TWL4030_PM_MASTER_CFG_P1_TRANSITION		0x36
#define TWL4030_PM_MASTER_CFG_P2_TRANSITION		0x37
#define TWL4030_PM_MASTER_CFG_P3_TRANSITION		0x38
#define TWL4030_PM_MASTER_CFG_P123_TRANSITION		0x39
#define TWL4030_PM_MASTER_STS_BOOT			0x3A
#define TWL4030_PM_MASTER_CFG_BOOT			0x3B
#define TWL4030_PM_MASTER_SHUNDAN			0x3C
#define TWL4030_PM_MASTER_BOOT_BCI			0x3D
#define TWL4030_PM_MASTER_CFG_PWRANA1			0x3E
#define TWL4030_PM_MASTER_CFG_PWRANA2			0x3F
#define TWL4030_PM_MASTER_BGAP_TRIM			0x40
#define TWL4030_PM_MASTER_BACKUP_MISC_STS		0x41
#define TWL4030_PM_MASTER_BACKUP_MISC_CFG		0x42
#define TWL4030_PM_MASTER_BACKUP_MISC_TST		0x43
#define TWL4030_PM_MASTER_PROTECT_KEY			0x44
#define TWL4030_PM_MASTER_STS_HW_CONDITIONS		0x45
#define TWL4030_PM_MASTER_P1_SW_EVENTS			0x46
#define TWL4030_PM_MASTER_P2_SW_EVENTS			0x47
#define TWL4030_PM_MASTER_P3_SW_EVENTS			0x48
#define TWL4030_PM_MASTER_STS_P123_STATE		0x49
#define TWL4030_PM_MASTER_PB_CFG			0x4A
#define TWL4030_PM_MASTER_PB_WORD_MSB			0x4B
#define TWL4030_PM_MASTER_PB_WORD_LSB			0x4C
#define TWL4030_PM_MASTER_SEQ_ADD_W2P			0x52
#define TWL4030_PM_MASTER_SEQ_ADD_P2A			0x53
#define TWL4030_PM_MASTER_SEQ_ADD_A2W			0x54
#define TWL4030_PM_MASTER_SEQ_ADD_A2S			0x55
#define TWL4030_PM_MASTER_SEQ_ADD_S2A12			0x56
#define TWL4030_PM_MASTER_SEQ_ADD_S2A3			0x57
#define TWL4030_PM_MASTER_SEQ_ADD_WARM			0x58
#define TWL4030_PM_MASTER_MEMORY_ADDRESS		0x59
#define TWL4030_PM_MASTER_MEMORY_DATA			0x5A
#define TWL4030_PM_MASTER_SC_CONFIG			0x5B
#define TWL4030_PM_MASTER_SC_DETECT1			0x5C
#define TWL4030_PM_MASTER_SC_DETECT2			0x5D
#define TWL4030_PM_MASTER_WATCHDOG_CFG			0x5E
#define TWL4030_PM_MASTER_IT_CHECK_CFG			0x5F
#define TWL4030_PM_MASTER_VIBRATOR_CFG			0x60
#define TWL4030_PM_MASTER_DCDC_GLOBAL_CFG		0x61
#define TWL4030_PM_MASTER_VDD1_TRIM1			0x62
#define TWL4030_PM_MASTER_VDD1_TRIM2			0x63
#define TWL4030_PM_MASTER_VDD2_TRIM1			0x64
#define TWL4030_PM_MASTER_VDD2_TRIM2			0x65
#define TWL4030_PM_MASTER_VIO_TRIM1			0x66
#define TWL4030_PM_MASTER_VIO_TRIM2			0x67
#define TWL4030_PM_MASTER_MISC_CFG			0x68
#define TWL4030_PM_MASTER_LS_TST_A			0x69
#define TWL4030_PM_MASTER_LS_TST_B			0x6A
#define TWL4030_PM_MASTER_LS_TST_C			0x6B
#define TWL4030_PM_MASTER_LS_TST_D			0x6C
#define TWL4030_PM_MASTER_BB_CFG			0x6D
#define TWL4030_PM_MASTER_MISC_TST			0x6E
#define TWL4030_PM_MASTER_TRIM1				0x6F
/* P[1-3]_SW_EVENTS */
#define TWL4030_PM_MASTER_SW_EVENTS_STOPON_PWRON	(1 << 6)
#define TWL4030_PM_MASTER_SW_EVENTS_STOPON_SYSEN	(1 << 5)
#define TWL4030_PM_MASTER_SW_EVENTS_ENABLE_WARMRESET	(1 << 4)
#define TWL4030_PM_MASTER_SW_EVENTS_LVL_WAKEUP		(1 << 3)
#define TWL4030_PM_MASTER_SW_EVENTS_DEVACT		(1 << 2)
#define TWL4030_PM_MASTER_SW_EVENTS_DEVSLP		(1 << 1)
#define TWL4030_PM_MASTER_SW_EVENTS_DEVOFF		(1 << 0)

/* Power bus message definitions */

/* The TWL4030/5030 splits its power-management resources (the various
 * regulators, clock and reset lines) into 3 processor groups - P1, P2 and
 * P3. These groups can then be configured to transition between sleep, wait-on
 * and active states by sending messages to the power bus.  See Section 5.4.2
 * Power Resources of TWL4030 TRM
 */

/* Processor groups */
#define DEV_GRP_NULL		0x0
#define DEV_GRP_P1		0x1	/* P1: all OMAP devices */
#define DEV_GRP_P2		0x2	/* P2: all Modem devices */
#define DEV_GRP_P3		0x4	/* P3: all peripheral devices */

/* Resource groups */
#define RES_GRP_RES		0x0	/* Reserved */
#define RES_GRP_PP		0x1	/* Power providers */
#define RES_GRP_RC		0x2	/* Reset and control */
#define RES_GRP_PP_RC		0x3
#define RES_GRP_PR		0x4	/* Power references */
#define RES_GRP_PP_PR		0x5
#define RES_GRP_RC_PR		0x6
#define RES_GRP_ALL		0x7	/* All resource groups */

#define RES_TYPE2_R0		0x0

#define RES_TYPE_ALL		0x7

/* Resource states */
#define RES_STATE_WRST		0xF
#define RES_STATE_ACTIVE	0xE
#define RES_STATE_SLEEP		0x8
#define RES_STATE_OFF		0x0

/* Power resources */

/* Power providers */
#define RES_VAUX1               1
#define RES_VAUX2               2
#define RES_VAUX3               3
#define RES_VAUX4               4
#define RES_VMMC1               5
#define RES_VMMC2               6
#define RES_VPLL1               7
#define RES_VPLL2               8
#define RES_VSIM                9
#define RES_VDAC                10
#define RES_VINTANA1            11
#define RES_VINTANA2            12
#define RES_VINTDIG             13
#define RES_VIO                 14
#define RES_VDD1                15
#define RES_VDD2                16
#define RES_VUSB_1V5            17
#define RES_VUSB_1V8            18
#define RES_VUSB_3V1            19
#define RES_VUSBCP              20
#define RES_REGEN               21
/* Reset and control */
#define RES_NRES_PWRON          22
#define RES_CLKEN               23
#define RES_SYSEN               24
#define RES_HFCLKOUT            25
#define RES_32KCLKOUT           26
#define RES_RESET               27
/* Power Reference */
#define RES_Main_Ref            28

#define TOTAL_RESOURCES		28
/*
 * Power Bus Message Format ... these can be sent individually by Linux,
 * but are usually part of downloaded scripts that are run when various
 * power events are triggered.
 *
 *  Broadcast Message (16 Bits):
 *    DEV_GRP[15:13] MT[12]  RES_GRP[11:9]  RES_TYPE2[8:7] RES_TYPE[6:4]
 *    RES_STATE[3:0]
 *
 *  Singular Message (16 Bits):
 *    DEV_GRP[15:13] MT[12]  RES_ID[11:4]  RES_STATE[3:0]
 */

#define MSG_BROADCAST(devgrp, grp, type, type2, state) \
	((devgrp) << 13 | 1 << 12 | (grp) << 9 | (type2) << 7 \
	| (type) << 4 | (state))

#define MSG_SINGULAR(devgrp, id, state) \
	((devgrp) << 13 | 0 << 12 | (id) << 4 | (state))

#define MSG_BROADCAST_ALL(devgrp, state) \
	((devgrp) << 5 | (state))

#define MSG_BROADCAST_REF MSG_BROADCAST_ALL
#define MSG_BROADCAST_PROV MSG_BROADCAST_ALL
#define MSG_BROADCAST__CLK_RST MSG_BROADCAST_ALL

/* Power Managment Receiver */
#define TWL4030_PM_RECEIVER_SC_CONFIG			0x5B
#define TWL4030_PM_RECEIVER_SC_DETECT1			0x5C
#define TWL4030_PM_RECEIVER_SC_DETECT2			0x5D
#define TWL4030_PM_RECEIVER_WATCHDOG_CFG		0x5E
#define TWL4030_PM_RECEIVER_IT_CHECK_CFG		0x5F
#define TWL4030_PM_RECEIVER_VIBRATOR_CFG		0x5F
#define TWL4030_PM_RECEIVER_DC_TO_DC_CFG		0x61
#define TWL4030_PM_RECEIVER_VDD1_TRIM1			0x62
#define TWL4030_PM_RECEIVER_VDD1_TRIM2			0x63
#define TWL4030_PM_RECEIVER_VDD2_TRIM1			0x64
#define TWL4030_PM_RECEIVER_VDD2_TRIM2			0x65
#define TWL4030_PM_RECEIVER_VIO_TRIM1			0x66
#define TWL4030_PM_RECEIVER_VIO_TRIM2			0x67
#define TWL4030_PM_RECEIVER_MISC_CFG			0x68
#define TWL4030_PM_RECEIVER_LS_TST_A			0x69
#define TWL4030_PM_RECEIVER_LS_TST_B			0x6A
#define TWL4030_PM_RECEIVER_LS_TST_C			0x6B
#define TWL4030_PM_RECEIVER_LS_TST_D			0x6C
#define TWL4030_PM_RECEIVER_BB_CFG			0x6D
#define TWL4030_PM_RECEIVER_MISC_TST			0x6E
#define TWL4030_PM_RECEIVER_TRIM1			0x6F
#define TWL4030_PM_RECEIVER_TRIM2			0x70
#define TWL4030_PM_RECEIVER_DC_DC_TIMEOUT		0x71
#define TWL4030_PM_RECEIVER_VAUX1_DEV_GRP		0x72
#define TWL4030_PM_RECEIVER_VAUX1_TYPE			0x73
#define TWL4030_PM_RECEIVER_VAUX1_REMAP			0x74
#define TWL4030_PM_RECEIVER_VAUX1_DEDICATED		0x75
#define TWL4030_PM_RECEIVER_VAUX2_DEV_GRP		0x76
#define TWL4030_PM_RECEIVER_VAUX2_TYPE			0x77
#define TWL4030_PM_RECEIVER_VAUX2_REMAP			0x78
#define TWL4030_PM_RECEIVER_VAUX2_DEDICATED		0x79
#define TWL4030_PM_RECEIVER_VAUX3_DEV_GRP		0x7A
#define TWL4030_PM_RECEIVER_VAUX3_TYPE			0x7B
#define TWL4030_PM_RECEIVER_VAUX3_REMAP			0x7C
#define TWL4030_PM_RECEIVER_VAUX3_DEDICATED		0x7D
#define TWL4030_PM_RECEIVER_VAUX4_DEV_GRP		0x7E
#define TWL4030_PM_RECEIVER_VAUX4_TYPE			0x7F
#define TWL4030_PM_RECEIVER_VAUX4_REMAP			0x80
#define TWL4030_PM_RECEIVER_VAUX4_DEDICATED		0x81
#define TWL4030_PM_RECEIVER_VMMC1_DEV_GRP		0x82
#define TWL4030_PM_RECEIVER_VMMC1_TYPE			0x83
#define TWL4030_PM_RECEIVER_VMMC1_REMAP			0x84
#define TWL4030_PM_RECEIVER_VMMC1_DEDICATED		0x85
#define TWL4030_PM_RECEIVER_VMMC2_DEV_GRP		0x86
#define TWL4030_PM_RECEIVER_VMMC2_TYPE			0x87
#define TWL4030_PM_RECEIVER_VMMC2_REMAP			0x88
#define TWL4030_PM_RECEIVER_VMMC2_DEDICATED		0x89
#define TWL4030_PM_RECEIVER_VPLL1_DEV_GRP		0x8A
#define TWL4030_PM_RECEIVER_VPLL1_TYPE			0x8B
#define TWL4030_PM_RECEIVER_VPLL1_REMAP			0x8C
#define TWL4030_PM_RECEIVER_VPLL1_DEDICATED		0x8D
#define TWL4030_PM_RECEIVER_VPLL2_DEV_GRP		0x8E
#define TWL4030_PM_RECEIVER_VPLL2_TYPE			0x8F
#define TWL4030_PM_RECEIVER_VPLL2_REMAP			0x90
#define TWL4030_PM_RECEIVER_VPLL2_DEDICATED		0x91
#define TWL4030_PM_RECEIVER_VSIM_DEV_GRP		0x92
#define TWL4030_PM_RECEIVER_VSIM_TYPE			0x93
#define TWL4030_PM_RECEIVER_VSIM_REMAP			0x94
#define TWL4030_PM_RECEIVER_VSIM_DEDICATED		0x95
#define TWL4030_PM_RECEIVER_VDAC_DEV_GRP		0x96
#define TWL4030_PM_RECEIVER_VDAC_TYPE			0x97
#define TWL4030_PM_RECEIVER_VDAC_REMAP			0x98
#define TWL4030_PM_RECEIVER_VDAC_DEDICATED		0x99
#define TWL4030_PM_RECEIVER_VINTANA1_DEV_GRP		0x9A
#define TWL4030_PM_RECEIVER_VINTANA1_TYP		0x9B
#define TWL4030_PM_RECEIVER_VINTANA1_REMAP		0x9C
#define TWL4030_PM_RECEIVER_VINTANA1_DEDICATED		0x9D
#define TWL4030_PM_RECEIVER_VINTANA2_DEV_GRP		0x9E
#define TWL4030_PM_RECEIVER_VINTANA2_TYPE		0x9F
#define TWL4030_PM_RECEIVER_VINTANA2_REMAP		0xA0
#define TWL4030_PM_RECEIVER_VINTANA2_DEDICATED		0xA1
#define TWL4030_PM_RECEIVER_VINTDIG_DEV_GRP		0xA2
#define TWL4030_PM_RECEIVER_VINTDIG_TYPE		0xA3
#define TWL4030_PM_RECEIVER_VINTDIG_REMAP		0xA4
#define TWL4030_PM_RECEIVER_VINTDIG_DEDICATED		0xA5
#define TWL4030_PM_RECEIVER_VIO_DEV_GRP			0xA6
#define TWL4030_PM_RECEIVER_VIO_TYPE			0xA7
#define TWL4030_PM_RECEIVER_VIO_REMAP			0xA8
#define TWL4030_PM_RECEIVER_VIO_CFG			0xA9
#define TWL4030_PM_RECEIVER_VIO_MISC_CFG		0xAA
#define TWL4030_PM_RECEIVER_VIO_TEST1			0xAB
#define TWL4030_PM_RECEIVER_VIO_TEST2			0xAC
#define TWL4030_PM_RECEIVER_VIO_OSC			0xAD
#define TWL4030_PM_RECEIVER_VIO_RESERVED		0xAE
#define TWL4030_PM_RECEIVER_VIO_VSEL			0xAF
#define TWL4030_PM_RECEIVER_VDD1_DEV_GRP		0xB0
#define TWL4030_PM_RECEIVER_VDD1_TYPE			0xB1
#define TWL4030_PM_RECEIVER_VDD1_REMAP			0xB2
#define TWL4030_PM_RECEIVER_VDD1_CFG			0xB3
#define TWL4030_PM_RECEIVER_VDD1_MISC_CFG		0xB4
#define TWL4030_PM_RECEIVER_VDD1_TEST1			0xB5
#define TWL4030_PM_RECEIVER_VDD1_TEST2			0xB6
#define TWL4030_PM_RECEIVER_VDD1_OSC			0xB7
#define TWL4030_PM_RECEIVER_VDD1_RESERVED		0xB8
#define TWL4030_PM_RECEIVER_VDD1_VSEL			0xB9
#define TWL4030_PM_RECEIVER_VDD1_VMODE_CFG		0xBA
#define TWL4030_PM_RECEIVER_VDD1_VFLOOR			0xBB
#define TWL4030_PM_RECEIVER_VDD1_VROOF			0xBC
#define TWL4030_PM_RECEIVER_VDD1_STEP			0xBD
#define TWL4030_PM_RECEIVER_VDD2_DEV_GRP		0xBE
#define TWL4030_PM_RECEIVER_VDD2_TYPE			0xBF
#define TWL4030_PM_RECEIVER_VDD2_REMAP			0xC0
#define TWL4030_PM_RECEIVER_VDD2_CFG			0xC1
#define TWL4030_PM_RECEIVER_VDD2_MISC_CFG		0xC2
#define TWL4030_PM_RECEIVER_VDD2_TEST1			0xC3
#define TWL4030_PM_RECEIVER_VDD2_TEST2			0xC4
#define TWL4030_PM_RECEIVER_VDD2_OSC			0xC5
#define TWL4030_PM_RECEIVER_VDD2_RESERVED		0xC6
#define TWL4030_PM_RECEIVER_VDD2_VSEL			0xC7
#define TWL4030_PM_RECEIVER_VDD2_VMODE_CFG		0xC8
#define TWL4030_PM_RECEIVER_VDD2_VFLOOR			0xC9
#define TWL4030_PM_RECEIVER_VDD2_VROOF			0xCA
#define TWL4030_PM_RECEIVER_VDD2_STEP			0xCB
#define TWL4030_PM_RECEIVER_VUSB1V5_DEV_GRP		0xCC
#define TWL4030_PM_RECEIVER_VUSB1V5_TYPE		0xCD
#define TWL4030_PM_RECEIVER_VUSB1V5_REMAP		0xCE
#define TWL4030_PM_RECEIVER_VUSB1V8_DEV_GRP		0xCF
#define TWL4030_PM_RECEIVER_VUSB1V8_TYPE		0xD0
#define TWL4030_PM_RECEIVER_VUSB1V8_REMAP		0xD1
#define TWL4030_PM_RECEIVER_VUSB3V1_DEV_GRP		0xD2
#define TWL4030_PM_RECEIVER_VUSB3V1_TYPE		0xD3
#define TWL4030_PM_RECEIVER_VUSB3V1_REMAP		0xD4
#define TWL4030_PM_RECEIVER_VUSBCP_DEV_GRP		0xD5
#define TWL4030_PM_RECEIVER_VUSBCP_TYPE			0xD6
#define TWL4030_PM_RECEIVER_VUSBCP_REMAP		0xD7
#define TWL4030_PM_RECEIVER_VUSB_DEDICATED1		0xD8
#define TWL4030_PM_RECEIVER_VUSB_DEDICATED2		0xD9
#define TWL4030_PM_RECEIVER_REGEN_DEV_GRP		0xDA
#define TWL4030_PM_RECEIVER_REGEN_TYPE			0xDB
#define TWL4030_PM_RECEIVER_REGEN_REMAP			0xDC
#define TWL4030_PM_RECEIVER_NRESPWRON_DEV_GRP		0xDD
#define TWL4030_PM_RECEIVER_NRESPWRON_TYPE		0xDE
#define TWL4030_PM_RECEIVER_NRESPWRON_REMAP		0xDF
#define TWL4030_PM_RECEIVER_CLKEN_DEV_GRP		0xE0
#define TWL4030_PM_RECEIVER_CLKEN_TYPE			0xE1
#define TWL4030_PM_RECEIVER_CLKEN_REMAP			0xE2
#define TWL4030_PM_RECEIVER_SYSEN_DEV_GRP		0xE3
#define TWL4030_PM_RECEIVER_SYSEN_TYPE			0xE4
#define TWL4030_PM_RECEIVER_SYSEN_REMAP			0xE5
#define TWL4030_PM_RECEIVER_HFCLKOUT_DEV_GRP		0xE6
#define TWL4030_PM_RECEIVER_HFCLKOUT_TYPE		0xE7
#define TWL4030_PM_RECEIVER_HFCLKOUT_REMAP		0xE8
#define TWL4030_PM_RECEIVER_32KCLKOUT_DEV_GRP		0xE9
#define TWL4030_PM_RECEIVER_32KCLKOUT_TYPE		0xEA
#define TWL4030_PM_RECEIVER_32KCLKOUT_REMAP		0xEB
#define TWL4030_PM_RECEIVER_TRITON_RESET_DEV_GRP	0xEC
#define TWL4030_PM_RECEIVER_TRITON_RESET_TYPE		0xED
#define TWL4030_PM_RECEIVER_TRITON_RESET_REMAP		0xEE
#define TWL4030_PM_RECEIVER_MAINREF_DEV_GRP		0xEF
#define TWL4030_PM_RECEIVER_MAINREF_TYPE		0xF0
#define TWL4030_PM_RECEIVER_MAINREF_REMAP		0xF1

/* Voltage Selection in PM Receiver Module */
#define TWL4030_PM_RECEIVER_VAUX2_VSEL_18		0x05
#define TWL4030_PM_RECEIVER_VAUX3_VSEL_28		0x03
#define TWL4030_PM_RECEIVER_VPLL2_VSEL_18		0x05
#define TWL4030_PM_RECEIVER_VDAC_VSEL_18		0x03
#define TWL4030_PM_RECEIVER_VMMC1_VSEL_30		0x02
#define TWL4030_PM_RECEIVER_VMMC1_VSEL_32		0x03
#define TWL4030_PM_RECEIVER_VSIM_VSEL_18		0x03

/* Device Selection in PM Receiver Module */
#define TWL4030_PM_RECEIVER_DEV_GRP_P1			0x20
#define TWL4030_PM_RECEIVER_DEV_GRP_ALL			0xE0

/* LED */
#define TWL4030_LED_LEDEN				0xEE
#define TWL4030_LED_LEDEN_LEDAON			(1 << 0)
#define TWL4030_LED_LEDEN_LEDBON			(1 << 1)
#define TWL4030_LED_LEDEN_LEDAPWM			(1 << 4)
#define TWL4030_LED_LEDEN_LEDBPWM			(1 << 5)

/* Keypad */
#define TWL4030_KEYPAD_KEYP_CTRL_REG			0xD2
#define TWL4030_KEYPAD_KEY_DEB_REG			0xD3
#define TWL4030_KEYPAD_LONG_KEY_REG1			0xD4
#define TWL4030_KEYPAD_LK_PTV_REG			0xD5
#define TWL4030_KEYPAD_TIME_OUT_REG1			0xD6
#define TWL4030_KEYPAD_TIME_OUT_REG2			0xD7
#define TWL4030_KEYPAD_KBC_REG				0xD8
#define TWL4030_KEYPAD_KBR_REG				0xD9
#define TWL4030_KEYPAD_KEYP_SMS				0xDA
#define TWL4030_KEYPAD_FULL_CODE_7_0			0xDB
#define TWL4030_KEYPAD_FULL_CODE_15_8			0xDC
#define TWL4030_KEYPAD_FULL_CODE_23_16			0xDD
#define TWL4030_KEYPAD_FULL_CODE_31_24			0xDE
#define TWL4030_KEYPAD_FULL_CODE_39_32			0xDF
#define TWL4030_KEYPAD_FULL_CODE_47_40			0xE0
#define TWL4030_KEYPAD_FULL_CODE_55_48			0xE1
#define TWL4030_KEYPAD_FULL_CODE_63_56			0xE2
#define TWL4030_KEYPAD_KEYP_ISR1			0xE3
#define TWL4030_KEYPAD_KEYP_IMR1			0xE4
#define TWL4030_KEYPAD_KEYP_ISR2			0xE5
#define TWL4030_KEYPAD_KEYP_IMR2			0xE6
#define TWL4030_KEYPAD_KEYP_SIR				0xE7
#define TWL4030_KEYPAD_KEYP_EDR				0xE8
#define TWL4030_KEYPAD_KEYP_SIH_CTRL			0xE9

#define TWL4030_KEYPAD_CTRL_KBD_ON			(1 << 6)
#define TWL4030_KEYPAD_CTRL_RP_EN			(1 << 5)
#define TWL4030_KEYPAD_CTRL_TOLE_EN			(1 << 4)
#define TWL4030_KEYPAD_CTRL_TOE_EN			(1 << 3)
#define TWL4030_KEYPAD_CTRL_LK_EN			(1 << 2)
#define TWL4030_KEYPAD_CTRL_SOFTMODEN			(1 << 1)
#define TWL4030_KEYPAD_CTRL_SOFT_NRST			(1 << 0)

/* USB */
#define TWL4030_USB_VENDOR_ID_LO			0x00
#define TWL4030_USB_VENDOR_ID_HI			0x01
#define TWL4030_USB_PRODUCT_ID_LO			0x02
#define TWL4030_USB_PRODUCT_ID_HI			0x03
#define TWL4030_USB_FUNC_CTRL				0x04
#define TWL4030_USB_FUNC_CTRL_SET			0x05
#define TWL4030_USB_FUNC_CTRL_CLR			0x06
#define TWL4030_USB_IFC_CTRL				0x07
#define TWL4030_USB_IFC_CTRL_SET			0x08
#define TWL4030_USB_IFC_CTRL_CLR			0x09
#define TWL4030_USB_OTG_CTRL				0x0A
#define TWL4030_USB_OTG_CTRL_SET			0x0B
#define TWL4030_USB_OTG_CTRL_CLR			0x0C
#define TWL4030_USB_USB_INT_EN_RISE			0x0D
#define TWL4030_USB_USB_INT_EN_RISE_SET			0x0E
#define TWL4030_USB_USB_INT_EN_RISE_CLR			0x0F
#define TWL4030_USB_USB_INT_EN_FALL			0x10
#define TWL4030_USB_USB_INT_EN_FALL_SET			0x11
#define TWL4030_USB_USB_INT_EN_FALL_CLR			0x12
#define TWL4030_USB_USB_INT_STS				0x13
#define TWL4030_USB_USB_INT_LATCH			0x14
#define TWL4030_USB_DEBUG				0x15
#define TWL4030_USB_SCRATCH_REG				0x16
#define TWL4030_USB_SCRATCH_REG_SET			0x17
#define TWL4030_USB_SCRATCH_REG_CLR			0x18
#define TWL4030_USB_CARKIT_CTRL				0x19
#define TWL4030_USB_CARKIT_CTRL_SET			0x1A
#define TWL4030_USB_CARKIT_CTRL_CLR			0x1B
#define TWL4030_USB_CARKIT_INT_DELAY			0x1C
#define TWL4030_USB_CARKIT_INT_EN			0x1D
#define TWL4030_USB_CARKIT_INT_EN_SET			0x1E
#define TWL4030_USB_CARKIT_INT_EN_CLR			0x1F
#define TWL4030_USB_CARKIT_INT_STS			0x20
#define TWL4030_USB_CARKIT_INT_LATCH			0x21
#define TWL4030_USB_CARKIT_PLS_CTRL			0x22
#define TWL4030_USB_CARKIT_PLS_CTRL_SET			0x23
#define TWL4030_USB_CARKIT_PLS_CTRL_CLR			0x24
#define TWL4030_USB_TRANS_POS_WIDTH			0x25
#define TWL4030_USB_TRANS_NEG_WIDTH			0x26
#define TWL4030_USB_RCV_PLTY_RECOVERY			0x27
#define TWL4030_USB_MCPC_CTRL				0x30
#define TWL4030_USB_MCPC_CTRL_SET			0x31
#define TWL4030_USB_MCPC_CTRL_CLR			0x32
#define TWL4030_USB_MCPC_IO_CTRL			0x33
#define TWL4030_USB_MCPC_IO_CTRL_SET			0x34
#define TWL4030_USB_MCPC_IO_CTRL_CLR			0x35
#define TWL4030_USB_MCPC_CTRL2				0x36
#define TWL4030_USB_MCPC_CTRL2_SET			0x37
#define TWL4030_USB_MCPC_CTRL2_CLR			0x38
#define TWL4030_USB_OTHER_FUNC_CTRL			0x80
#define TWL4030_USB_OTHER_FUNC_CTRL_SET			0x81
#define TWL4030_USB_OTHER_FUNC_CTRL_CLR			0x82
#define TWL4030_USB_OTHER_IFC_CTRL			0x83
#define TWL4030_USB_OTHER_IFC_CTRL_SET			0x84
#define TWL4030_USB_OTHER_IFC_CTRL_CLR			0x85
#define TWL4030_USB_OTHER_INT_EN_RISE_SET		0x87
#define TWL4030_USB_OTHER_INT_EN_RISE_CLR		0x88
#define TWL4030_USB_OTHER_INT_EN_FALL			0x89
#define TWL4030_USB_OTHER_INT_EN_FALL_SET		0x8A
#define TWL4030_USB_OTHER_INT_EN_FALL_CLR		0x8B
#define TWL4030_USB_OTHER_INT_STS			0x8C
#define TWL4030_USB_OTHER_INT_LATCH			0x8D
#define TWL4030_USB_ID_STATUS				0x96
#define TWL4030_USB_CARKIT_SM_1_INT_EN			0x97
#define TWL4030_USB_CARKIT_SM_1_INT_EN_SET		0x98
#define TWL4030_USB_CARKIT_SM_1_INT_EN_CLR		0x99
#define TWL4030_USB_CARKIT_SM_1_INT_STS			0x9A
#define TWL4030_USB_CARKIT_SM_1_INT_LATCH		0x9B
#define TWL4030_USB_CARKIT_SM_2_INT_EN			0x9C
#define TWL4030_USB_CARKIT_SM_2_INT_EN_SET		0x9D
#define TWL4030_USB_CARKIT_SM_2_INT_EN_CLR		0x9E
#define TWL4030_USB_CARKIT_SM_2_INT_STS			0x9F
#define TWL4030_USB_CARKIT_SM_2_INT_LATCH		0xA0
#define TWL4030_USB_CARKIT_SM_CTRL			0xA1
#define TWL4030_USB_CARKIT_SM_CTRL_SET			0xA2
#define TWL4030_USB_CARKIT_SM_CTRL_CLR			0xA3
#define TWL4030_USB_CARKIT_SM_CMD			0xA4
#define TWL4030_USB_CARKIT_SM_CMD_SET			0xA5
#define TWL4030_USB_CARKIT_SM_CMD_CLR			0xA6
#define TWL4030_USB_CARKIT_SM_CMD_STS			0xA7
#define TWL4030_USB_CARKIT_SM_STATUS			0xA8
#define TWL4030_USB_CARKIT_SM_ERR_STATUS		0xAA
#define TWL4030_USB_CARKIT_SM_CTRL_STATE		0xAB
#define TWL4030_USB_POWER_CTRL				0xAC
#define TWL4030_USB_POWER_CTRL_SET			0xAD
#define TWL4030_USB_POWER_CTRL_CLR			0xAE
#define TWL4030_USB_OTHER_IFC_CTRL2			0xAF
#define TWL4030_USB_OTHER_IFC_CTRL2_SET			0xB0
#define TWL4030_USB_OTHER_IFC_CTRL2_CLR			0xB1
#define TWL4030_USB_REG_CTRL_EN				0xB2
#define TWL4030_USB_REG_CTRL_EN_SET			0xB3
#define TWL4030_USB_REG_CTRL_EN_CLR			0xB4
#define TWL4030_USB_REG_CTRL_ERROR			0xB5
#define TWL4030_USB_OTHER_FUNC_CTRL2			0xB8
#define TWL4030_USB_OTHER_FUNC_CTRL2_SET		0xB9
#define TWL4030_USB_OTHER_FUNC_CTRL2_CLR		0xBA
#define TWL4030_USB_CARKIT_ANA_CTRL			0xBB
#define TWL4030_USB_CARKIT_ANA_CTRL_SET			0xBC
#define TWL4030_USB_CARKIT_ANA_CTRL_CLR			0xBD
#define TWL4030_USB_VBUS_DEBOUNCE			0xC0
#define TWL4030_USB_ID_DEBOUNCE				0xC1
#define TWL4030_USB_TPH_DP_CON_MIN			0xC2
#define TWL4030_USB_TPH_DP_CON_MAX			0xC3
#define TWL4030_USB_TCR_DP_CON_MIN			0xC4
#define TWL4030_USB_TCR_DP_CON_MAX			0xC5
#define TWL4030_USB_TPH_DP_PD_SHORT			0xC6
#define TWL4030_USB_TPH_CMD_DLY				0xC7
#define TWL4030_USB_TPH_DET_RST				0xC8
#define TWL4030_USB_TPH_AUD_BIAS			0xC9
#define TWL4030_USB_TCR_UART_DET_MIN			0xCA
#define TWL4030_USB_TCR_UART_DET_MAX			0xCB
#define TWL4030_USB_TPH_ID_INT_PW			0xCD
#define TWL4030_USB_TACC_ID_INT_WAIT			0xCE
#define TWL4030_USB_TACC_ID_INT_PW			0xCF
#define TWL4030_USB_TPH_CMD_WAIT			0xD0
#define TWL4030_USB_TPH_ACK_WAIT			0xD1
#define TWL4030_USB_TPH_DP_DISC_DET			0xD2
#define TWL4030_USB_VBAT_TIMER				0xD3
#define TWL4030_USB_CARKIT_4W_DEBUG			0xE0
#define TWL4030_USB_CARKIT_5W_DEBUG			0xE1
#define TWL4030_USB_PHY_PWR_CTRL			0xFD
#define TWL4030_USB_PHY_CLK_CTRL			0xFE
#define TWL4030_USB_PHY_CLK_CTRL_STS			0xFF

/* GPIO */
#define TWL4030_GPIO_GPIODATAIN1			0x00
#define TWL4030_GPIO_GPIODATAIN2			0x01
#define TWL4030_GPIO_GPIODATAIN3			0x02
#define TWL4030_GPIO_GPIODATADIR1			0x03
#define TWL4030_GPIO_GPIODATADIR2			0x04
#define TWL4030_GPIO_GPIODATADIR3			0x05
#define TWL4030_GPIO_GPIODATAOUT1			0x06
#define TWL4030_GPIO_GPIODATAOUT2			0x07
#define TWL4030_GPIO_GPIODATAOUT3			0x08
#define TWL4030_GPIO_CLEARGPIODATAOUT1			0x09
#define TWL4030_GPIO_CLEARGPIODATAOUT2			0x0A
#define TWL4030_GPIO_CLEARGPIODATAOUT3			0x0B
#define TWL4030_GPIO_SETGPIODATAOUT1			0x0C
#define TWL4030_GPIO_SETGPIODATAOUT2			0x0D
#define TWL4030_GPIO_SETGPIODATAOUT3			0x0E
#define TWL4030_GPIO_GPIO_DEBEN1			0x0F
#define TWL4030_GPIO_GPIO_DEBEN2			0x10
#define TWL4030_GPIO_GPIO_DEBEN3			0x11
#define TWL4030_GPIO_GPIO_CTRL				0x12
#define TWL4030_GPIO_GPIOPUPDCTR1			0x13
#define TWL4030_GPIO_GPIOPUPDCTR2			0x14
#define TWL4030_GPIO_GPIOPUPDCTR3			0x15
#define TWL4030_GPIO_GPIOPUPDCTR4			0x16
#define TWL4030_GPIO_GPIOPUPDCTR5			0x17
#define TWL4030_GPIO_GPIO_ISR1A				0x19
#define TWL4030_GPIO_GPIO_ISR2A				0x1A
#define TWL4030_GPIO_GPIO_ISR3A				0x1B
#define TWL4030_GPIO_GPIO_IMR1A				0x1C
#define TWL4030_GPIO_GPIO_IMR2A				0x1D
#define TWL4030_GPIO_GPIO_IMR3A				0x1E
#define TWL4030_GPIO_GPIO_ISR1B				0x1F
#define TWL4030_GPIO_GPIO_ISR2B				0x20
#define TWL4030_GPIO_GPIO_ISR3B				0x21
#define TWL4030_GPIO_GPIO_IMR1B				0x22
#define TWL4030_GPIO_GPIO_IMR2B				0x23
#define TWL4030_GPIO_GPIO_IMR3B				0x24
#define TWL4030_GPIO_GPIO_EDR1				0x28
#define TWL4030_GPIO_GPIO_EDR2				0x29
#define TWL4030_GPIO_GPIO_EDR3				0x2A
#define TWL4030_GPIO_GPIO_EDR4				0x2B
#define TWL4030_GPIO_GPIO_EDR5				0x2C
#define TWL4030_GPIO_GPIO_SIH_CTRL			0x2D


ErrorT twl4030_i2c_read_u8(unint1 i2c_num, unint1 *val, unint1 reg) {
	Omap3530i2c* i2c_dev = (Omap3530i2c*) theOS->getFileManager()->getResource("dev/i2c");

	if (i2c_dev == 0) {
		LOG(ARCH,ERROR,(ARCH,ERROR,"twl4030_i2c_read_u8() ERROR: i2c needed for power control of usb hub!"));
		return cError;
	}

	char i2c_val[8];
	i2c_val[0] = i2c_num;  // register group 4b
	i2c_val[1] = reg;
	unint4 length = 1;
	ErrorT  err = i2c_dev->readBytes(i2c_val,length);

	*val = i2c_val[0];
	return err;
}

ErrorT twl4030_i2c_write_u8(unint1 i2c_num, unint1 val, unint1 reg) {
	Omap3530i2c* i2c_dev = (Omap3530i2c*) theOS->getFileManager()->getResource("dev/i2c");

	if (i2c_dev == 0) {
		LOG(ARCH,ERROR,(ARCH,ERROR,"twl4030_i2c_write_u8() ERROR: i2c needed for power control of usb hub!"));
		return cError;
	}

	char i2c_val[8];
	i2c_val[0] = i2c_num;  // register group 4b
	i2c_val[1] = reg;	// VAUX2_DEDICATED register
	i2c_val[2] = val;
	unint4 length = 3;
	ErrorT err = i2c_dev->writeBytes(i2c_val,length);
	if (err != cOk) {
		LOG(ARCH,ERROR,(ARCH,ERROR,"twl4030_i2c_write_u8() ERROR: writing to controller"));
		return cError;
	}

	return cOk;
}


void twl4030_power_reset_init(void)
 {
	 unint1 val = 0;
	 if (twl4030_i2c_read_u8(TWL4030_CHIP_PM_MASTER, &val, TWL4030_PM_MASTER_P1_SW_EVENTS) != cOk) {
			printf("Error:TWL4030: failed to read the power register\n");
			printf("Could not initialize hardware reset\n");
	 } else {
			 val |= TWL4030_PM_MASTER_SW_EVENTS_STOPON_PWRON;
			 if (twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER, val, TWL4030_PM_MASTER_P1_SW_EVENTS) != cOk) {
					 printf("Error:TWL4030: failed to write the power register\n");
					 printf("Could not initialize hardware reset\n");
			}
	 }
 }

void twl4030_pmrecv_vsel_cfg(unint1 vsel_reg, unint1 vsel_val, unint1 dev_grp, unint1 dev_grp_sel)
{
	 int ret;

	 /* Select the Voltage */
	 ret = twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, vsel_val,
							  vsel_reg);
	if (ret != cOk) {
			 printf("Could not write vsel to reg %02x (%d)\n",
				   vsel_reg, ret);
			return;
	}

	/* Select the Device Group (enable the supply if dev_grp_sel != 0) */
	 ret = twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, dev_grp_sel,
							dev_grp);
	 if (ret != cOk)
			 printf("Could not write grp_sel to reg %02x (%d)\n",
					 dev_grp, ret);
 }



/*static inline int gpio_twl4030_write(unint1 address, unint1 data)
{
	return twl4030_i2c_write_u8(TWL4030_MODULE_GPIO, data, address);
}*/





/* Defines for bits in registers */
#define OPMODE_MASK		(3 << 3)
#define XCVRSELECT_MASK		(3 << 0)
#define CARKITMODE		(1 << 2)
#define OTG_ENAB		(1 << 5)
#define PHYPWD			(1 << 0)
#define CLOCKGATING_EN		(1 << 2)
#define CLK32K_EN		(1 << 1)
#define REQ_PHY_DPLL_CLK	(1 << 0)
#define PHY_DPLL_CLK		(1 << 0)

static int twl4030_usb_write(unint1 address, unint1 data)
{
	int ret;

	ret = twl4030_i2c_write_u8(TWL4030_CHIP_USB, data, address);
	if (ret != 0)
		printf("TWL4030:USB:Write[0x%x] Error %d\n", address, ret);

	return ret;
}

static int twl4030_usb_read(unint1 address)
{
	unint1 data;
	int ret;

	ret = twl4030_i2c_read_u8(TWL4030_CHIP_USB, &data, address);
	if (ret == 0)
		ret = data;
	else
		printf("TWL4030:USB:Read[0x%x] Error %d\n", address, ret);

	return ret;
}

static void twl4030_usb_ldo_init(void)
{
	/* Enable writing to power configuration registers */
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER, 0xC0,
			     TWL4030_PM_MASTER_PROTECT_KEY);
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER, 0x0C,
			     TWL4030_PM_MASTER_PROTECT_KEY);

	// already done by boot rom code
	/*twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER, 1 << 1 | 1 << 3,
			     TWL4030_PM_MASTER_CFG_BOOT);*/

	/* put VUSB3V1 LDO in active state */
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, 0x00,
			     TWL4030_PM_RECEIVER_VUSB_DEDICATED2);

	/* input to VUSB3V1 LDO is from VBAT, not VBUS */
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, 0x14,
			     TWL4030_PM_RECEIVER_VUSB_DEDICATED1);

	/* turn on 3.1V regulator */
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, 0x20,
			     TWL4030_PM_RECEIVER_VUSB3V1_DEV_GRP);
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, 0x00,
			     TWL4030_PM_RECEIVER_VUSB3V1_TYPE);

	/* turn on 1.5V regulator */
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, 0x20,
			     TWL4030_PM_RECEIVER_VUSB1V5_DEV_GRP);
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, 0x00,
			     TWL4030_PM_RECEIVER_VUSB1V5_TYPE);

	/* turn on 1.8V regulator */
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, 0x20,
			     TWL4030_PM_RECEIVER_VUSB1V8_DEV_GRP);
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, 0x00,
			     TWL4030_PM_RECEIVER_VUSB1V8_TYPE);

	/* disable access to power configuration registers */
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER, 0x00,
			     TWL4030_PM_MASTER_PROTECT_KEY);
}

static void twl4030_phy_power(void)
{
	unint1 pwr, clk;

	/* Power the PHY */
	pwr = twl4030_usb_read(TWL4030_USB_PHY_PWR_CTRL);
	pwr &= ~PHYPWD;
	twl4030_usb_write(TWL4030_USB_PHY_PWR_CTRL, pwr);
	/* Enable clocks */
	clk = twl4030_usb_read(TWL4030_USB_PHY_CLK_CTRL);
	clk |= CLOCKGATING_EN | CLK32K_EN;
	twl4030_usb_write(TWL4030_USB_PHY_CLK_CTRL, clk);
}

/*
 * Initiaze the ULPI interface
 * ULPI : Universal Transceiver Macrocell Low Pin Interface
 * An interface between the USB link controller like musb and the
 * the PHY or transceiver that drives the actual bus.
 */
int twl4030_usb_ulpi_init(void)
{
	long timeout = 1000 * 1000; /* 1 sec */;
	unint1 clk, sts, pwr;

	/* twl4030 ldo init */
	twl4030_usb_ldo_init();

	/* Enable the twl4030 phy */
	twl4030_phy_power();

	/* Enable DPLL to access PHY registers over I2C */
	clk = twl4030_usb_read(TWL4030_USB_PHY_CLK_CTRL);
	clk |= REQ_PHY_DPLL_CLK;
	twl4030_usb_write(TWL4030_USB_PHY_CLK_CTRL, clk);

	/* Check if the PHY DPLL is locked */
	sts = twl4030_usb_read(TWL4030_USB_PHY_CLK_CTRL_STS);
	while (!(sts & PHY_DPLL_CLK) && 0 < timeout) {
		udelay(10);
		sts = twl4030_usb_read(TWL4030_USB_PHY_CLK_CTRL_STS);
		timeout -= 10;
	}

	/* Final check */
	sts = twl4030_usb_read(TWL4030_USB_PHY_CLK_CTRL_STS);
	if (!(sts & PHY_DPLL_CLK)) {
		printf("Error:TWL4030:USB Timeout setting PHY DPLL clock\n");
		return -1;
	}

	/*
	 * There are two circuit blocks attached to the PHY,
	 * Carkit and USB OTG.  Disable Carkit and enable USB OTG
	 */
	twl4030_usb_write(TWL4030_USB_IFC_CTRL_CLR, CARKITMODE);
	pwr = twl4030_usb_read(TWL4030_USB_POWER_CTRL);
	pwr |= OTG_ENAB;
	twl4030_usb_write(TWL4030_USB_POWER_CTRL_SET, pwr);

	/* Clear the opmode bits to ensure normal encode */
	twl4030_usb_write(TWL4030_USB_FUNC_CTRL_CLR, OPMODE_MASK);

	/* Clear the xcvrselect bits to enable the high speed transeiver */
	twl4030_usb_write(TWL4030_USB_FUNC_CTRL_CLR, XCVRSELECT_MASK);

	/* Let ULPI control the DPLL clock */
	clk = twl4030_usb_read(TWL4030_USB_PHY_CLK_CTRL);
	clk &= ~REQ_PHY_DPLL_CLK;
	twl4030_usb_write(TWL4030_USB_PHY_CLK_CTRL, clk);

	return 0;
}

#define TWL4030_PM_RECEIVER_VAUX1_DEV_GRP		0x72
#define TWL4030_PM_RECEIVER_VAUX1_TYPE			0x73
#define TWL4030_PM_RECEIVER_VAUX1_REMAP			0x74
#define TWL4030_PM_RECEIVER_VAUX1_DEDICATED		0x75
#define TWL4030_PM_RECEIVER_VAUX2_DEV_GRP		0x76
#define TWL4030_PM_RECEIVER_VAUX2_TYPE			0x77
#define TWL4030_PM_RECEIVER_VAUX2_REMAP			0x78
#define TWL4030_PM_RECEIVER_VAUX2_DEDICATED		0x79
#define TWL4030_PM_RECEIVER_VAUX3_DEV_GRP		0x7A
#define TWL4030_PM_RECEIVER_VAUX3_TYPE			0x7B
#define TWL4030_PM_RECEIVER_VAUX3_REMAP			0x7C
#define TWL4030_PM_RECEIVER_VAUX3_DEDICATED		0x7D
#define TWL4030_PM_RECEIVER_VAUX4_DEV_GRP		0x7E
#define TWL4030_PM_RECEIVER_VAUX4_TYPE			0x7F
#define TWL4030_PM_RECEIVER_VAUX4_REMAP			0x80
#define TWL4030_PM_RECEIVER_VAUX4_DEDICATED		0x81
#define TWL4030_PM_RECEIVER_VMMC1_DEV_GRP		0x82
#define TWL4030_PM_RECEIVER_VMMC1_TYPE			0x83
#define TWL4030_PM_RECEIVER_VMMC1_REMAP			0x84
#define TWL4030_PM_RECEIVER_VMMC1_DEDICATED		0x85
#define TWL4030_PM_RECEIVER_VMMC2_DEV_GRP		0x86
#define TWL4030_PM_RECEIVER_VMMC2_TYPE			0x87
#define TWL4030_PM_RECEIVER_VMMC2_REMAP			0x88
#define TWL4030_PM_RECEIVER_VMMC2_DEDICATED		0x89
#define TWL4030_PM_RECEIVER_VPLL1_DEV_GRP		0x8A

#define TWL4030_PM_RECEIVER_VPLL2_DEV_GRP		0x8E
#define TWL4030_PM_RECEIVER_VPLL2_TYPE			0x8F
#define TWL4030_PM_RECEIVER_VPLL2_REMAP			0x90
#define TWL4030_PM_RECEIVER_VPLL2_DEDICATED		0x91


#define TWL4030_PM_RECEIVER_VDAC_DEV_GRP		0x96
#define TWL4030_PM_RECEIVER_VDAC_TYPE			0x97
#define TWL4030_PM_RECEIVER_VDAC_REMAP			0x98
#define TWL4030_PM_RECEIVER_VDAC_DEDICATED		0x99


/* Voltage Selection in PM Receiver Module */
#define TWL4030_PM_RECEIVER_VAUX2_VSEL_18		0x05
#define TWL4030_PM_RECEIVER_VAUX3_VSEL_28		0x03
#define TWL4030_PM_RECEIVER_VPLL2_VSEL_18		0x05
#define TWL4030_PM_RECEIVER_VDAC_VSEL_18		0x03
#define TWL4030_PM_RECEIVER_VMMC1_VSEL_30		0x02
#define TWL4030_PM_RECEIVER_VMMC1_VSEL_32		0x03
#define TWL4030_PM_RECEIVER_VSIM_VSEL_18		0x03




void twl4030_power_init(void)
{
	/* set VAUX3 to 2.8V */
	twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX3_DEDICATED,
				TWL4030_PM_RECEIVER_VAUX3_VSEL_28,
				TWL4030_PM_RECEIVER_VAUX3_DEV_GRP,
				TWL4030_PM_RECEIVER_DEV_GRP_P1);

	/* set VPLL2 to 1.8V */
	twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VPLL2_DEDICATED,
				TWL4030_PM_RECEIVER_VPLL2_VSEL_18,
				TWL4030_PM_RECEIVER_VPLL2_DEV_GRP,
				TWL4030_PM_RECEIVER_DEV_GRP_ALL);

	/* set VDAC to 1.8V */
	twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VDAC_DEDICATED,
				TWL4030_PM_RECEIVER_VDAC_VSEL_18,
				TWL4030_PM_RECEIVER_VDAC_DEV_GRP,
				TWL4030_PM_RECEIVER_DEV_GRP_P1);


}

void twl4030_power_mmc_init(void)
{
	/* Set VMMC1 to 3.15 Volts */
	twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VMMC1_DEDICATED,
				TWL4030_PM_RECEIVER_VMMC1_VSEL_32,
				TWL4030_PM_RECEIVER_VMMC1_DEV_GRP,
				TWL4030_PM_RECEIVER_DEV_GRP_P1);
}

//#define TWL4030_GPIO_MAX 18


HighSpeedUSBHostController::HighSpeedUSBHostController() :
		USB_EHCI_Host_Controller(0x48064800,&ehci_memory,8000)
{
	// do some omap stuff here
	// be sure the functional clocks of the usb host subsystem are enabled!
	OUTW(CM_ICLKEN_USBHOST,0x1);
	OUTW(CM_FCLKEN_USBHOST,0x3);

	OUTW(CM_ICLKEN3_CORE,1 << 2);
	OUTW(CM_FCLKEN3_CORE,1 << 2);

	OUTW(CM_AUTOIDLE_USBHOST,0x0);
	//OUTW(CM_AUTOIDLE3_CORE,0x0);

	// be sure the twl4030 is powering the USB PHY
	twl4030_power_init();
	twl4030_power_reset_init();
	twl4030_usb_ulpi_init();
	twl4030_power_mmc_init();

	// TPS65950 has the same sw interface as the twl4030
	// VAUX2 is different for TPS65950!!
    /* Set VAUX2 to 1.8V for EHCI PHY */
    twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX2_DEDICATED,
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

	  Omap3530i2c* i2c_dev = (Omap3530i2c*) theOS->getFileManager()->getResource("dev/i2c");

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


	unint4 cm_idlest2 = INW(0x48004d24);

	if (!(cm_idlest2 & (1<<3))) {
 		printf("ERROR: 120Mhz clock not active..");
 		return;
 	}

	/* Put the PHY in RESET */
	// set phy in reset mode
	// this is done by setting the gpio pin 147 voltage to high
	// the gpio is hard wired to the phy reset pin
	omap_ehci_phy_reset(1, 1000);


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
	// todo : check for timeout!
	while (INW(UHH_HOSTCONFIG) & 1);

	host_config = INW(UHH_HOSTCONFIG);
	LOG(ARCH,INFO,(ARCH,INFO,"HighSpeedUSBHostController() UHH_HOSTCONFIG: %x",host_config));

	// get phy out of reset
	omap_ehci_phy_reset(0, 10000);

	// disable undocumented feature which interferes with usb hub suspend state
	OUTW(0x480648a0,1<<5);


#define ULPI_VID_HIGH 0x01
#define ULPI_VID_LOW 0x00

#define ULPI_FUNCTION_CONTROL 0x4
#define ULPI_FUNCTION_RESET (1 << 5)


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

#define ULPI_FUNCTION_CONTROL 0x4
#define ULPI_FUNCTION_RESET (1 << 5)

	unint1 *reg = (unint1*) ULPI_FUNCTION_CONTROL;

	ulpi_write(&ulpi_vp,reg,ULPI_FUNCTION_RESET);
	ulpi_reset_wait(&ulpi_vp);


	// call the Init method of the EHCI in baseclass to initialize the controller hardware
	this->Init();

}

HighSpeedUSBHostController::~HighSpeedUSBHostController() {
	// TODO Auto-generated destructor stub
}

