/*
 * SMSC95xxUSBDeviceDriver.cc
 *
 *  Created on: 02.05.2013
 *      Author: dbaldin
 */

#include "SMSC95xxUSBDeviceDriver.hh"
#include "mii.hh"
#include "memtools.hh"
#include "inet.h"
#include "kernel/Kernel.hh"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "netif/etharp.h"
#include "lwip/stats.h"
#include "inc/endian.h"

extern Kernel* theOS;


extern "C" err_t ethernet_input(struct pbuf *p, struct netif *netif);



// Default Mac Address...
static char default_macaddr[6]  __attribute__((aligned(4))) = {0x1,0x1,0x1,0x1,0x1,0x1};


/* SMSC LAN95xx based USB 2.0 Ethernet Devices */

/* Tx command words */
#define TX_CMD_A_FIRST_SEG_		0x00002000
#define TX_CMD_A_LAST_SEG_		0x00001000

/* Rx status word */
#define RX_STS_FL_			0x3FFF0000	/* Frame Length */
#define RX_STS_ES_			0x00008000	/* Error Summary */

/* SCSRs */
#define ID_REV				0x00

#define TX_CFG				0x10
#define TX_CFG_ON_			0x00000004

#define HW_CFG				0x14
#define HW_CFG_BIR_			0x00001000
#define HW_CFG_RXDOFF_			0x00000600
#define HW_CFG_MEF_			0x00000020
#define HW_CFG_BCE_			0x00000002
#define HW_CFG_LRST_			0x00000008

#define PM_CTRL				0x20
#define PM_CTL_PHY_RST_			0x00000010

#define AFC_CFG				0x2C

/*
 * Hi watermark = 15.5Kb (~10 mtu pkts)
 * low watermark = 3k (~2 mtu pkts)
 * backpressure duration = ~ 350us
 * Apply FC on any frame.
 */
#define AFC_CFG_DEFAULT			0x00F830A1

#define E2P_CMD				0x30
#define E2P_CMD_BUSY_			0x80000000
#define E2P_CMD_READ_			0x00000000
#define E2P_CMD_TIMEOUT_		0x00000400
#define E2P_CMD_LOADED_			0x00000200
#define E2P_CMD_ADDR_			0x000001FF

#define E2P_DATA			0x34

#define BURST_CAP			0x38

#define INT_EP_CTL			0x68
#define INT_EP_CTL_PHY_INT_		0x00008000


#define INT_STS                         (0x08)
#define INT_STS_TX_STOP_                (0x00020000)
#define INT_STS_RX_STOP_                (0x00010000)
#define INT_STS_PHY_INT_                (0x00008000)
#define INT_STS_TXE_                    (0x00004000)
#define INT_STS_TDFU_                   (0x00002000)
#define INT_STS_TDFO_                   (0x00001000)
#define INT_STS_RXDF_                   (0x00000800)
#define INT_STS_GPIOS_                  (0x000007FF)
#define INT_STS_CLEAR_ALL_              (0xFFFFFFFF)


#define BULK_IN_DLY			0x6C

/* MAC CSRs */
#define MAC_CR				0x100
#define MAC_CR_MCPAS_			0x00080000
#define MAC_CR_PRMS_			0x00040000
#define MAC_CR_HPFILT_			0x00002000
#define MAC_CR_TXEN_			0x00000008
#define MAC_CR_RXEN_			0x00000004

#define ADDRH				0x104

#define ADDRL				0x108

#define MII_ADDR			0x114
#define MII_WRITE_			0x02
#define MII_BUSY_			0x01
#define MII_READ_			0x00 /* ~of MII Write bit */

#define MII_DATA			0x118

#define FLOW				0x11C

#define VLAN1				0x120

#define COE_CR				0x130
#define Tx_COE_EN_			0x00010000
#define Rx_COE_EN_			0x00000001

/* Vendor-specific PHY Definitions */
#define PHY_INT_SRC			29

#define PHY_INT_MASK			30
#define PHY_INT_MASK_ANEG_COMP_		((unint2)0x0040)
#define PHY_INT_MASK_LINK_DOWN_		((unint2)0x0010)
#define PHY_INT_MASK_DEFAULT_		(PHY_INT_MASK_ANEG_COMP_ | \
					 PHY_INT_MASK_LINK_DOWN_)

/* USB Vendor Requests */
#define USB_VENDOR_REQUEST_WRITE_REGISTER	0xA0
#define USB_VENDOR_REQUEST_READ_REGISTER	0xA1

/* Some extra defines */
#define HS_USB_PKT_SIZE			512
#define FS_USB_PKT_SIZE			64
#define DEFAULT_HS_BURST_CAP_SIZE	(16 * 1024 + 5 * HS_USB_PKT_SIZE)
#define DEFAULT_FS_BURST_CAP_SIZE	(6 * 1024 + 33 * FS_USB_PKT_SIZE)
#define DEFAULT_BULK_IN_DELAY		0x00002000
#define MAX_SINGLE_PACKET_SIZE		2048
#define EEPROM_MAC_OFFSET		0x01
#define SMSC95XX_INTERNAL_PHY_ID	1
#define ETH_P_8021Q	0x8100          /* 802.1Q VLAN Extended Header  */

/* local defines */
#define SMSC95XX_BASE_NAME "sms"
#define USB_CTRL_SET_TIMEOUT 5000
#define USB_CTRL_GET_TIMEOUT 5000
#define USB_BULK_SEND_TIMEOUT 5000
#define USB_BULK_RECV_TIMEOUT 5000

#define AX_RX_URB_SIZE 2048
#define PHY_CONNECT_TIMEOUT 5000

#define TURBO_MODE


/* driver private */
struct smsc95xx_private {
	size_t 	rx_urb_size;  /* maximum USB URB size */
	unint4	mac_cr;  /* MAC control register value */
	int 	have_hwaddr;  /* 1 if we have a hardware MAC address */
};


unint4 tmpbuf[40] __attribute__((aligned(4)));

/*
 * Smsc95xx infrastructure commands
 */
static int smsc95xx_write_reg(USBDevice *dev, unint2 index, unint4 data)
{

	//data = ___swab32(data);
	tmpbuf[0] = data;

	index = cputobe16(index);

	unint1 msg[8] = {USB_TYPE_VENDOR | USB_RECIP_DEVICE, USB_VENDOR_REQUEST_WRITE_REGISTER,0x0,0x0, (unint1) ((index & 0xff00) >> 8),(unint1) (index & 0xff), sizeof(data),0x0};

	int4 error = dev->controller->sendUSBControlMsg(dev,0,(unint1*)&msg,USB_DIR_OUT,sizeof(data),(unint1*)&tmpbuf);

	if (error != 4) {
		printf("smsc95xx_write_reg failed: index=%d, data=%d, len=%d\r\n", index, data, sizeof(data));
		return -1;
	}
	return cOk;

}


static int smsc95xx_read_reg(USBDevice *dev, unint2 index, unint4 *data)
{

	index = cputobe16(index);

	unint1 msg[8] = {0x80 | USB_TYPE_VENDOR | USB_RECIP_DEVICE, USB_VENDOR_REQUEST_READ_REGISTER,0x0,0x0,(unint1) ((index & 0xff00) >> 8),(unint1) (index & 0xff),sizeof(data),0x0};

	int4 error = dev->controller->sendUSBControlMsg(dev,0,(unint1*)&msg,USB_DIR_IN,sizeof(data),(unint1*)&tmpbuf);

	if (error != 4) {
		printf("smsc95xx_read_reg failed: index=%d, data=%d, len=%d, error=%d", index, *data, sizeof(data),error);
		return -1;
	}

	*data = tmpbuf[0];
	//*data = tole32(*data);
	return cOk;

}

/* Loop until the read is completed with timeout */
static int smsc95xx_phy_wait_not_busy(USBDevice *dev)
{
	volatile int4 timeout = 1000;
	unint4 val;

	do {
		timeout--;
		smsc95xx_read_reg(dev, MII_ADDR, &val);
		if (!(val & MII_BUSY_))
			return 0;
	} while (timeout > 0);

	return -1;
}

static int smsc95xx_mdio_read(USBDevice *dev, int phy_id, int idx)
{
	unint4 val, addr;

	/* confirm MII not busy */
	if (smsc95xx_phy_wait_not_busy(dev)) {
		printf("MII is busy in smsc95xx_mdio_read\n");
		return -1;
	}

	/* set the address, index & direction (read from PHY) */
	addr = (phy_id << 11) | (idx << 6) | MII_READ_;
	smsc95xx_write_reg(dev, MII_ADDR, addr);

	if (smsc95xx_phy_wait_not_busy(dev)) {
		printf("Timed out reading MII reg %02X\n", idx);
		return -1;
	}

	smsc95xx_read_reg(dev, MII_DATA, &val);

	return (unint2)(val & 0xFFFF);
}

static void smsc95xx_mdio_write(USBDevice *dev, int phy_id, int idx,
				int regval)
{
	unint4 val, addr;

	/* confirm MII not busy */
	if (smsc95xx_phy_wait_not_busy(dev)) {
		printf("MII is busy in smsc95xx_mdio_write\n");
		return;
	}

	val = regval;
	smsc95xx_write_reg(dev, MII_DATA, val);

	/* set the address, index & direction (write to PHY) */
	addr = (phy_id << 11) | (idx << 6) | MII_WRITE_;
	smsc95xx_write_reg(dev, MII_ADDR, addr);

	if (smsc95xx_phy_wait_not_busy(dev))
		printf("Timed out writing MII reg %02X\n", idx);
}

static int smsc95xx_eeprom_confirm_not_busy(USBDevice *dev)
{
	volatile int4 timeout = 10000;
	unint4 val;

	do {
		timeout--;
		smsc95xx_read_reg(dev, E2P_CMD, &val);
		if (!(val & E2P_CMD_BUSY_))
			return 0;

	} while (timeout > 0);

	printf("EEPROM is busy\n");
	return -1;
}

static int smsc95xx_wait_eeprom(USBDevice *dev)
{
	volatile int timeout = 10000;
	unint4 val;

	do {
		timeout--;
		smsc95xx_read_reg(dev, E2P_CMD, &val);
		if (!(val & E2P_CMD_BUSY_) || (val & E2P_CMD_TIMEOUT_))
			break;

	} while (timeout > 0);

	if (val & (E2P_CMD_TIMEOUT_ | E2P_CMD_BUSY_)) {
		printf("EEPROM read operation timeout\n");
		return -1;
	}
	return 0;
}

static int smsc95xx_read_eeprom(USBDevice *dev, unint4 offset, unint4 length,
				unint1 *data)
{
	unint4 val,i;
	int ret;

	ret = smsc95xx_eeprom_confirm_not_busy(dev);
	if (ret)
		return ret;

	printf("EEPROM not busy..\r");

	for (i = 0; i < length; i++) {
		val = E2P_CMD_BUSY_ | E2P_CMD_READ_ | (offset & E2P_CMD_ADDR_);
		smsc95xx_write_reg(dev, E2P_CMD, val);

		ret = smsc95xx_wait_eeprom(dev);
		if (ret < 0)
			return ret;

		smsc95xx_read_reg(dev, E2P_DATA, &val);
		memdump((unint4) &val,1);
		data[i] = val & 0xFF;
		offset++;
	}
	return 0;
}

/*
 * mii_nway_restart - restart NWay (autonegotiation) for this interface
 *
 * Returns 0 on success, negative on error.
 */
static int mii_nway_restart(USBDevice *dev)
{
	int bmcr;
	int r = -1;

	/* if autoneg is off, it's an error */
	bmcr = smsc95xx_mdio_read(dev,SMSC95XX_INTERNAL_PHY_ID, MII_BMCR);

	if (bmcr & BMCR_ANENABLE) {
		bmcr |= BMCR_ANRESTART;
		smsc95xx_mdio_write(dev, SMSC95XX_INTERNAL_PHY_ID, MII_BMCR, bmcr);
		r = 0;
	}
	return r;
}

static int smsc95xx_phy_initialize(USBDevice *dev)
{
	smsc95xx_mdio_write(dev, SMSC95XX_INTERNAL_PHY_ID, MII_BMCR, BMCR_RESET);
	smsc95xx_mdio_write(dev, SMSC95XX_INTERNAL_PHY_ID, MII_ADVERTISE,
		ADVERTISE_ALL | ADVERTISE_CSMA | ADVERTISE_PAUSE_CAP |
		ADVERTISE_PAUSE_ASYM);

	/* read to clear */
	smsc95xx_mdio_read(dev, SMSC95XX_INTERNAL_PHY_ID, PHY_INT_SRC);

	smsc95xx_mdio_write(dev, SMSC95XX_INTERNAL_PHY_ID, PHY_INT_MASK,
		PHY_INT_MASK_DEFAULT_);
	mii_nway_restart(dev);

	//printf("phy initialised succesfully\r");
	return 0;
}

static int smsc95xx_init_mac_address(char* ethaddr, USBDevice *dev)
{

	/* try reading mac address from EEPROM */
	if (smsc95xx_read_eeprom(dev, EEPROM_MAC_OFFSET, 6,(unint1*) ethaddr) == 0) {
		return 0;
		//if (is_valid_ether_addr(eth->enetaddr)) {
			/* eeprom values are valid so use them */
			//printf("MAC address read from EEPROM\n");
			//return 0;
		//}
	}

	/*
	 * No eeprom, or eeprom values are invalid. Generating a random MAC
	 * address is not safe. Just return an error.
	 */
	return -1;
}


static inline unint2 __get_unaligned_le16(unint1 *p)
{
	return p[0] | p[1] << 8; // if we need to convert
	//return p[0] << 8 | p[1];
}

static inline unint4 __get_unaligned_le32(unint1 *p)
{
	return p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24; // if we need to convert
	//return p[3] | p[2] << 8 | p[1] << 16 | p[0] << 24;
}

static int smsc95xx_write_hwaddr(USBDevice* dev, char* ethaddr)
{
	unint4 addr_lo = __get_unaligned_le32((unint1*)&ethaddr[0]);
	unint4 addr_hi = __get_unaligned_le16((unint1*)&ethaddr[4]);
	int ret;

	/* set hardware address */
	//printf("** %s()\r", __func__);
	ret = smsc95xx_write_reg(dev, ADDRL, addr_lo);
	if (ret < 0)
		return ret;

	ret = smsc95xx_write_reg(dev, ADDRH, addr_hi);
	if (ret < 0)
		return ret;

	//printf("MAC %pM\r", ethaddr);

	unint4 addr_smsc_lo;
	ret = smsc95xx_read_reg(dev, ADDRL, &addr_smsc_lo);
	//printf("MAC Set: %x",addr_smsc_lo);

	ret = smsc95xx_read_reg(dev, ADDRH, &addr_smsc_lo);
	//printf("%x\r",addr_smsc_lo);


	return 0;
}

/* Enable or disable Tx & Rx checksum offload engines */
static int smsc95xx_set_csums(USBDevice *dev,
		int use_tx_csum, int use_rx_csum)
{
	unint4 read_buf;
	int ret = smsc95xx_read_reg(dev, COE_CR, &read_buf);
	if (ret < 0)
		return ret;

	if (use_tx_csum)
		read_buf |= Tx_COE_EN_;
	else
		read_buf &= ~Tx_COE_EN_;

	if (use_rx_csum)
		read_buf |= Rx_COE_EN_;
	else
		read_buf &= ~Rx_COE_EN_;

	ret = smsc95xx_write_reg(dev, COE_CR, read_buf);
	if (ret < 0)
		return ret;

	//printf("COE_CR = 0x%08x\r", read_buf);
	return 0;
}

static void smsc95xx_set_multicast(USBDevice* dev)
{
	struct smsc95xx_private *priv = (smsc95xx_private*) dev->dev_priv;

	/* No multicast in u-boot */
	priv->mac_cr &= ~(MAC_CR_PRMS_ | MAC_CR_MCPAS_ | MAC_CR_HPFILT_);
}

/* starts the TX path */
static void smsc95xx_start_tx_path(USBDevice *dev)
{
	struct smsc95xx_private *priv = (smsc95xx_private*) dev->dev_priv;
	unint4 reg_val;

	/* Enable Tx at MAC */
	priv->mac_cr |= MAC_CR_TXEN_;

	smsc95xx_write_reg(dev, MAC_CR, priv->mac_cr);

	/* Enable Tx at SCSRs */
	reg_val = TX_CFG_ON_;
	smsc95xx_write_reg(dev, TX_CFG, reg_val);
}

/* Starts the Receive path */
static void smsc95xx_start_rx_path(USBDevice *dev)
{
	struct smsc95xx_private *priv = (smsc95xx_private*) dev->dev_priv;

	priv->mac_cr |= MAC_CR_RXEN_;
	smsc95xx_write_reg(dev, MAC_CR, priv->mac_cr);
}


// Simple delay function
void udelay(volatile int i) {

	while (i > 0) { i--; };

}


/*
 * Try to initialize the hardware
 */
ErrorT SMSC95xxUSBDeviceDriver::init()
{
	unint4 write_buf;
	unint4 read_buf;
	unint4 burst_cap;
	volatile int link_detected;
	int ret;
	volatile int timeout;

	struct smsc95xx_private *priv = (struct smsc95xx_private *)dev->dev_priv;

	write_buf = HW_CFG_LRST_;
	ret = smsc95xx_write_reg(dev, HW_CFG, write_buf);
	if (ret < 0)
		return ret;

	timeout = 0;
	do {
		ret = smsc95xx_read_reg(dev, HW_CFG, &read_buf);
		if (ret < 0)
			return ret;

		timeout++;
	} while ((read_buf & HW_CFG_LRST_) && (timeout < 1000000));

	if (timeout >= 100) {
		LOG(ARCH,ERROR,(ARCH,ERROR,"SMSC95xxUSBDeviceDriver: timeout waiting for completion of Lite Reset"));
		return -1;
	}
	LOG(ARCH,TRACE,(ARCH,TRACE,"SMSC95xxUSBDeviceDriver: SMSC95xx reset successfully.."));

	write_buf = PM_CTL_PHY_RST_;
	ret = smsc95xx_write_reg(dev, PM_CTRL, write_buf);
	if (ret < 0)
		return ret;

	timeout = 0;
	do {
		ret = smsc95xx_read_reg(dev, PM_CTRL, &read_buf);
		if (ret < 0)
			return ret;

		timeout++;
	} while ((read_buf & PM_CTL_PHY_RST_) && (timeout < 10000000));
	if (timeout >= 100) {
		LOG(ARCH,ERROR,(ARCH,ERROR,"SMSC95xxUSBDeviceDriver: timeout waiting for PHY Reset"));
		return -1;
	}

	LOG(ARCH,TRACE,(ARCH,TRACE,"SMSC95xxUSBDeviceDriver: SMSC95xx PHY reset successfully.."));

	// get preprogrammed mac address
	// beagleboardxm does not have a hard coded mac address
	/*if (! (smsc95xx_init_mac_address((char*) &ethaddr, dev) == 0)) {
		printf("Error: SMSC95xx: No MAC address set\r");
		return -1;
	}

	printf("SMSC95xx Got mac address from eeprom..\r");
	memdump((unint4) &ethaddr,2);*/

	if (smsc95xx_write_hwaddr(dev,(char*) &default_macaddr) < 0)
		return -1;

	ret = smsc95xx_read_reg(dev, HW_CFG, &read_buf);
	if (ret < 0)
		return ret;
	LOG(ARCH,TRACE,(ARCH,TRACE,"SMSC95xxUSBDeviceDriver: Read Value from HW_CFG : 0x%08x", read_buf));

	read_buf |= HW_CFG_BIR_;
	ret = smsc95xx_write_reg(dev, HW_CFG, read_buf);
	if (ret < 0)
		return ret;

	ret = smsc95xx_read_reg(dev, HW_CFG, &read_buf);
	if (ret < 0)
		return ret;
	LOG(ARCH,TRACE,(ARCH,TRACE,"SMSC95xxUSBDeviceDriver: Read Value from HW_CFG after writing HW_CFG_BIR_: 0x%08x", read_buf));

#ifdef TURBO_MODE
//	if (dev->pusb_dev->speed == USB_SPEED_HIGH) {
		burst_cap = DEFAULT_HS_BURST_CAP_SIZE / HS_USB_PKT_SIZE;
		priv->rx_urb_size = DEFAULT_HS_BURST_CAP_SIZE;
/*	} else {
		burst_cap = DEFAULT_FS_BURST_CAP_SIZE / FS_USB_PKT_SIZE;
		priv->rx_urb_size = DEFAULT_FS_BURST_CAP_SIZE;
	}*/
#else
	burst_cap = 0;
	priv->rx_urb_size = MAX_SINGLE_PACKET_SIZE;
#endif
	LOG(ARCH,TRACE,(ARCH,TRACE,"SMSC95xxUSBDeviceDriver: rx_urb_size=%d", (unint4)priv->rx_urb_size));

	ret = smsc95xx_write_reg(dev, BURST_CAP, burst_cap);
	if (ret < 0)
		return ret;

	ret = smsc95xx_read_reg(dev, BURST_CAP, &read_buf);
	if (ret < 0)
		return ret;
	LOG(ARCH,TRACE,(ARCH,TRACE,"SMSC95xxUSBDeviceDriver: Read Value from BURST_CAP after writing: 0x%08x", read_buf));

	read_buf = DEFAULT_BULK_IN_DELAY;
	ret = smsc95xx_write_reg(dev, BULK_IN_DLY, read_buf);
	if (ret < 0)
		return ret;

	ret = smsc95xx_read_reg(dev, BULK_IN_DLY, &read_buf);
	if (ret < 0)
		return ret;
	LOG(ARCH,TRACE,(ARCH,TRACE,"SMSC95xxUSBDeviceDriver: Read Value from BULK_IN_DLY after writing: 0x%08x", read_buf));

	ret = smsc95xx_read_reg(dev, HW_CFG, &read_buf);
	if (ret < 0)
		return ret;
	LOG(ARCH,TRACE,(ARCH,TRACE,"SMSC95xxUSBDeviceDriver: Read Value from HW_CFG: 0x%08x", read_buf));

#ifdef TURBO_MODE
	read_buf |= (HW_CFG_MEF_ | HW_CFG_BCE_);
#endif
	read_buf &= ~HW_CFG_RXDOFF_;

#define NET_IP_ALIGN 0
	read_buf |= NET_IP_ALIGN << 9;

	ret = smsc95xx_write_reg(dev, HW_CFG, read_buf);
	if (ret < 0)
		return ret;

	ret = smsc95xx_read_reg(dev, HW_CFG, &read_buf);
	if (ret < 0)
		return ret;
	LOG(ARCH,TRACE,(ARCH,TRACE,"SMSC95xxUSBDeviceDriver: Read Value from HW_CFG after writing: 0x%08x", read_buf));

	write_buf = 0xFFFFFFFF;
	ret = smsc95xx_write_reg(dev, INT_STS, write_buf);
	if (ret < 0)
		return ret;

	ret = smsc95xx_read_reg(dev, ID_REV, &read_buf);
	if (ret < 0)
		return ret;
	LOG(ARCH,TRACE,(ARCH,TRACE,"SMSC95xxUSBDeviceDriver: ID_REV = 0x%08x", read_buf));

	/* Init Tx */
	write_buf = 0;
	ret = smsc95xx_write_reg(dev, FLOW, write_buf);
	if (ret < 0)
		return ret;

	read_buf = AFC_CFG_DEFAULT;
	ret = smsc95xx_write_reg(dev, AFC_CFG, read_buf);
	if (ret < 0)
		return ret;

	ret = smsc95xx_read_reg(dev, MAC_CR, &priv->mac_cr);
	if (ret < 0)
		return ret;

	/* Init Rx. Set Vlan */
	write_buf = (unint4)ETH_P_8021Q;
	ret = smsc95xx_write_reg(dev, VLAN1, write_buf);
	if (ret < 0)
		return ret;

	/* Disable checksum offload engines */
	ret = smsc95xx_set_csums(dev, 1, 0);
	if (ret < 0) {
		LOG(ARCH,ERROR,(ARCH,ERROR,"SMSC95xxUSBDeviceDriver: Failed to set csum offload: %d", ret));
		return ret;
	}
	smsc95xx_set_multicast(dev);

	if (smsc95xx_phy_initialize(dev) < 0)
		return -1;
	ret = smsc95xx_read_reg(dev, INT_EP_CTL, &read_buf);
	if (ret < 0)
		return ret;

	/* enable PHY interrupts */
	read_buf |= INT_EP_CTL_PHY_INT_;

	ret = smsc95xx_write_reg(dev, INT_EP_CTL, read_buf);
	if (ret < 0)
		return ret;

	smsc95xx_start_tx_path(dev);
	smsc95xx_start_rx_path(dev);

	LOG(ARCH,INFO,(ARCH,INFO,"SMSC95xxUSBDeviceDriver: Waiting for Ethernet connection..."));
	timeout = 1000;

	do {
		link_detected = smsc95xx_mdio_read(dev, SMSC95XX_INTERNAL_PHY_ID, MII_BMSR);

		udelay(10000);
		timeout--;
	}
	//while ((link_detected == 0) && (timeout > 0));
	while (((link_detected & BMSR_LSTATUS) != BMSR_LSTATUS) && (timeout > 0));

	if (link_detected) {
		if (timeout != 0)
			LOG(ARCH,INFO,(ARCH,INFO,"SMSC95xxUSBDeviceDriver: Connection established.."));
			return cOk;
	} else {
		LOG(ARCH,WARN,(ARCH,WARN,"SMSC95xxUSBDeviceDriver: Error: Cable not connected?."));
		return cOk;
	}

	return 0;
}

#define PKTSIZE			1518

/*! The network interface for this eem module inside lwip */
// TODO: not multi interface capable.. store this inside the class
struct netif tEMAC0Netif;

ErrorT SMSC95xxUSBDeviceDriver::recv(unint4 recv_len)
{

	unint4 packet_len;

	LOG(ARCH,TRACE,(ARCH,TRACE,"SMSC95xxUSBDeviceDriver: Packet received.. USB-Len %x",recv_len));

	unint4 recvd_bytes = 0;

	while (recvd_bytes < recv_len) {

		memcpy(&packet_len, &dev->endpoints[this->bulkin_ep].recv_buffer[recvd_bytes], sizeof(packet_len));

		// cpu to le
		//packet_len = tole32(packet_len);

		if (packet_len & RX_STS_ES_) {
			LOG(ARCH,WARN,(ARCH,WARN,"SMSC95xxUSBDeviceDriver: RX packet header Error: %x",packet_len));
			return cError;
		}
		// extract packet_len
		packet_len = ((packet_len & RX_STS_FL_) >> 16);

		LOG(ARCH,TRACE,(ARCH,TRACE,"SMSC95xxUSBDeviceDriver: Packet received.. Packet-Len %x",packet_len));

		if (packet_len > 1500) {
			LOG(ARCH,WARN,(ARCH,WARN,"SMSC95xxUSBDeviceDriver: Length Validation failed: %d",packet_len));
			return (cError);
		}

		// deliver packet to upper layer
		struct pbuf* ptBuf = pbuf_alloc(PBUF_RAW, packet_len+10, PBUF_RAM);
		if (ptBuf != 0) {

			memcpy(ptBuf->payload, &dev->endpoints[bulkin_ep].recv_buffer[recvd_bytes+4], packet_len );

			// check if packet is complete
			if (packet_len < recv_len)
				ethernet_input(ptBuf,&tEMAC0Netif);
			else {
				LOG(ARCH,WARN,(ARCH,WARN,"SMSC95xxUSBDeviceDriver: Packet lost due to split transaction of USB packet.. (Len > max_packet_size)"));
			}

			pbuf_free(ptBuf);

		} else {
			LOG(ARCH,ERROR,(ARCH,ERROR,"SMSC95xxUSBDeviceDriver: no memory for pbuf!"));
		}

		recvd_bytes += packet_len + 4;
	}

	return cOk;
}


SMSC95xxUSBDeviceDriver::SMSC95xxUSBDeviceDriver(USBDevice* dev)
:  USBDeviceDriver(), CommDeviceDriver("eth0")
{
	this->dev = dev;
	this->bulkin_ep = 0;
	this->bulkout_ep = 0;
	this->int_ep = 0;
	LOG(ARCH,INFO,(ARCH,INFO,"SMSC95xxUSBDeviceDriver: new Device attached.."));


}

static char data[1500];

/*******************************************************************/
//unint4 crc32(unint4 crc, unint1 byte)
/*******************************************************************/
/*{
int i;
  crc = crc ^ byte;

  for(i=7; i>=0; i--)
    crc=(crc>>1)^(0xedb88320ul & (-(crc&1)));

  return(crc);
}*/

static err_t smsc95xx_low_level_output(struct netif *netif, struct pbuf *p) {
	LOG(ARCH,TRACE,(ARCH,TRACE,"SMSC95xxUSBDeviceDriver: sending packet.."));

	memset(data,0,64);

	if (p->tot_len > 1500) {
		//pbuf_free(p);
		LOG(ARCH,WARN,(ARCH,WARN,"SMSC95xxUSBDeviceDriver: not sending packet. Len > 1500."));
		return ERR_MEM;
	}

	int pos = 0;

	struct pbuf *curp = p;

	unint4 tx_cmd_a;
	unint4 tx_cmd_b;

	// TODO: CPU to le
	tx_cmd_a = ((unint4)p->tot_len) | TX_CMD_A_FIRST_SEG_ | TX_CMD_A_LAST_SEG_;
	tx_cmd_b = ((unint4)p->tot_len);
	//tx_cmd_a = tole32(tx_cmd_a);
	//tx_cmd_b = tole32(tx_cmd_b);

	/* prepend cmd_a and cmd_b */
	memcpy(&data[0], &tx_cmd_a, sizeof(tx_cmd_a));
	memcpy(&data[0] + sizeof(tx_cmd_a), &tx_cmd_b, sizeof(tx_cmd_b));

	pos = sizeof(tx_cmd_a) + sizeof(tx_cmd_b);

	while (curp != 0) {
		memcpy(&data[pos],curp->payload,curp->len);

		pos  += curp->len;
		curp = curp->next;
	}

	SMSC95xxUSBDeviceDriver *driver =  (SMSC95xxUSBDeviceDriver*) netif->state;

	if (driver != 0) {
		driver->dev->controller->USBBulkMsg(driver->dev,driver->bulkout_ep,USB_DIR_OUT,pos,(unint1*) data);
	}
	return ERR_OK;
}

err_t
dummy_output(struct netif *netif, struct pbuf *q, struct ip4_addr *ipaddr) {
	return ERR_OK;
}

err_t smsc9x_ethernetif_init(struct netif *netif) {

#if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

    /*
     * Initialize the snmp variables and counters inside the struct netif.
     * The last argument should be replaced with your link speed, in units
     * of bits per second.
     */
    //NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, ???);

    netif->name[0] = 'U';
    netif->name[1] = '0';
    /* We directly use etharp_output() here to save a function call.
     * You can instead declare your own function an call etharp_output()
     * from it if you have to do some checks before sending (e.g. if link
     * is available...) */
#if LWIP_ARP
    netif->ip4_output = etharp_output;
#else
    netif->ip4_output = dummy_output;
#endif
    netif->input = ethernet_input;
    netif->linkoutput = smsc95xx_low_level_output;

    netif->hwaddr_len = 6;

    for (int i = 0; i < netif->hwaddr_len; i++)
       {
           netif->hwaddr[i] = default_macaddr[i];
       }

       /* maximum transfer unit */
       netif->mtu = 1500; //MAX_FRAME_SIZE;

       /* device capabilities */
       /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
       netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP
               | NETIF_FLAG_LINK_UP;

       return cOk;
}




ErrorT SMSC95xxUSBDeviceDriver::initialize() {
	// try to initialize the device

	// first check the endpoint information

	bool bulkinep, bulkoutep, intep = false;

	for (int i = 1; i < 5; i++) {
		if (dev->endpoints[i].type == Bulk) {
			// active it

			if (dev->endpoints[i].direction == Out) {
				bulkout_ep =  i; //dev->endpoints[i].address;
				bulkoutep = true;
			} else {
				bulkin_ep =  i; //dev->endpoints[i].address;
				bulkinep = true;
				// internally handle RX transfer as interrupt requests
				dev->endpoints[bulkin_ep].type = Interrupt;
				dev->endpoints[bulkin_ep].poll_frequency = 5;
			}

			dev->activateEndpoint(i);

		} else if(dev->endpoints[i].type == Interrupt)  {
			int_ep =  i; //dev->endpoints[i].address;
			dev->endpoints[i].poll_frequency = 200;
			intep = true;
		}
	}

	if (!(bulkinep & bulkoutep & intep)) {
		LOG(ARCH,ERROR,(ARCH,ERROR,"SMSC95xxUSBDeviceDriver: probing failed.."));
		// deactivate eps again
		return cError;
	}

	dev->dev_priv = theOS->getMemoryManager()->alloc(sizeof(struct smsc95xx_private));

	if (init() < 0) {
		LOG(ARCH,ERROR,(ARCH,ERROR,"SMSC95xxUSBDeviceDriver: Initializing Ethernet device failed.."));
		return cError;
	}

	// setup periodic interrupt transfer
    dev->activateEndpoint(int_ep);

	 // set netmask for this device
	struct ip4_addr eth_nm;

	#ifndef Board_ETH_IP4NETMASK
	//define default value
	#define Board_ETH_IP4NETMASK 255,0,0,0
	#endif
	int netmask[4] = {Board_ETH_IP4NETMASK};
	IP4_ADDR(&eth_nm, netmask[0], netmask[1], netmask[2], netmask[3]);

	//#ifndef Board_ETH_IP4ADDR
	//define default value
	#define Board_ETH_IP4ADDR 192,168,1,100
	//#endif

	int ipaddr[4] = {Board_ETH_IP4ADDR};
	IP4_ADDR(&tIpAddr, ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3]);

		// save driver in netif as state
	// use ethernet interface init method in lwip_ethernetif.c
	netif_add(&tEMAC0Netif, &tIpAddr, &eth_nm, 0, 0, &smsc9x_ethernetif_init, 0);
	tEMAC0Netif.state = (void*) this;

	netif_set_default(&tEMAC0Netif);
	netif_set_up(&tEMAC0Netif);

	return cOk;
}


ErrorT SMSC95xxUSBDeviceDriver::handleInterrupt() {

	ErrorT ret = cError;

	volatile QH* qh = dev->endpoints[this->int_ep].queue_head;
	if (qh == 0) return cError;
	volatile qTD* qtd2  = (qTD*) qh->qh_curtd;


	if ( ((unint4)qtd2 > QT_NEXT_TERMINATE) && (QT_TOKEN_GET_STATUS(qh->qh_overlay.qt_token) != 0x80)) {
		unint4 interrupt_sts = *((unint4*) &dev->endpoints[this->int_ep].recv_buffer[0]);


		LOG(ARCH,DEBUG,(ARCH,DEBUG,"SMSC95xxUSBDeviceDriver: PHY Interrupt: %x",interrupt_sts));
		/* read to clear */
		smsc95xx_mdio_read(dev, SMSC95XX_INTERNAL_PHY_ID, PHY_INT_SRC);

		if ((interrupt_sts & INT_EP_CTL_PHY_INT_) != 0) {
			 /* clear interrupt status */

			LOG(ARCH,INFO,(ARCH,INFO,"SMSC95xxUSBDeviceDriver: Link Status changed.."));
			//while (1) smsc95xx_recv(dev,this->bulkin_ep);
		}

		// set qtd back to active
		qtd2->qt_token = QT_TOKEN_DT(1) | QT_TOKEN_CERR(0) | QT_TOKEN_IOC(1) | QT_TOKEN_PID(QT_TOKEN_PID_IN)
						 			 | QT_TOKEN_STATUS_ACTIVE | QT_TOKEN_TOTALBYTES(dev->endpoints[int_ep].max_packet_size);

		qh->qh_overlay.qt_next = (unint4) qtd2;

		// clear received data
		memset(&dev->endpoints[this->int_ep].recv_buffer[0],0,dev->endpoints[this->int_ep].max_packet_size);

		dev->activateEndpoint(int_ep);

		ret = cOk;
	}

	// check if this was a bulk in transfer complete irq

	qh = dev->endpoints[this->bulkin_ep].queue_head;
	qtd2  = (qTD*) qh->qh_curtd;

	if ( ((unint4)qtd2 > QT_NEXT_TERMINATE) && (QT_TOKEN_GET_STATUS(qh->qh_overlay.qt_token) != 0x80)) {
		LOG(ARCH,TRACE,(ARCH,TRACE,"SMSC95xxUSBDeviceDriver: Packet received: status %x",QT_TOKEN_GET_STATUS(qh->qh_overlay.qt_token)));


		unint4 len = dev->endpoints[bulkin_ep].max_packet_size - QT_TOKEN_GET_TOTALBYTES(qtd2->qt_token);
		// handle received data
		if (len > 0)
			recv(len);

		// clear received data
		memset(&dev->endpoints[bulkin_ep].recv_buffer[0],0,dev->endpoints[bulkin_ep].max_packet_size);
		// set qtd back to active

		dev->endpoints[bulkin_ep].status_toggle ^= 1;
		dev->activateEndpoint(bulkin_ep);

		ret = cOk;
	}

	return ret;
}


SMSC95xxUSBDeviceDriver::~SMSC95xxUSBDeviceDriver() {
	// TODO Auto-generated destructor stub
}

void SMSC95xxUSBDeviceDriver::recv() {

}


ErrorT SMSC95xxUSBDeviceDriver::lowlevel_send( char* data, int len ) {
	return cNotImplemented;
}


//checks whether the given class,product device is supported by this driver
bool SMSC95xxUSBDeviceDriverFactory::isDriverFor(USBDevice* dev) {

	if (dev->dev_descr.idVendor != 0x0424) return false;
	if (dev->dev_descr.idProduct == 0xec00 || dev->dev_descr.idProduct == 0x9500 ) return true;

	return false;

}


SMSC95xxUSBDeviceDriverFactory::SMSC95xxUSBDeviceDriverFactory(char* name)
: USBDeviceDriverFactory(name)
{

}
