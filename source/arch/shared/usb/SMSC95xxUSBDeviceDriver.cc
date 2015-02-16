/*
 * SMSC95xxUSBDeviceDriver.cc
 *
 *  Created on: 02.05.2013
 *      Copyright & Author: dbaldin
 */

/***************************************
 *             INCLUDES
 ***************************************/
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
#include "synchro/Mutex.hh"

extern Kernel* theOS;
extern "C" err_t ethernet_input(struct pbuf *p, struct netif *netif);

#if !ENABLE_NETWORKING
#error "SMSC95xx requires Networking to be enabled. Either remove the SMSC95xx from this configuration or enable networking"
#endif

/***************************************
 *             DEFINES
 ***************************************/

/* SMSC LAN95xx based USB 2.0 Ethernet Devices */
/* Tx command words */
#define TX_CMD_A_FIRST_SEG_     0x00002000
#define TX_CMD_A_LAST_SEG_      0x00001000

/* Rx status word */
#define RX_STS_FL_              0x3FFF0000  /* Frame Length */
#define RX_STS_ES_              0x00008000  /* Error Summary */

/* SCSRs */
#define ID_REV                  0x00

#define TX_CFG                  0x10
#define TX_CFG_ON_              0x00000004

#define HW_CFG                  0x14
#define HW_CFG_BIR_             0x00001000
#define HW_CFG_RXDOFF_          0x00000600
#define HW_CFG_MEF_             0x00000020
#define HW_CFG_BCE_             0x00000002
#define HW_CFG_LRST_            0x00000008

#define PM_CTRL                 0x20
#define PM_CTL_PHY_RST_         0x00000010

#define AFC_CFG                 0x2C

/*
 * Hi watermark = 15.5Kb (~10 mtu pkts)
 * low watermark = 3k (~2 mtu pkts)
 * backpressure duration = ~ 350us
 * Apply FC on any frame.
 */
#define AFC_CFG_DEFAULT         0x00F830A1

#define E2P_CMD                 0x30
#define E2P_CMD_BUSY_           0x80000000
#define E2P_CMD_READ_           0x00000000
#define E2P_CMD_TIMEOUT_        0x00000400
#define E2P_CMD_LOADED_         0x00000200
#define E2P_CMD_ADDR_           0x000001FF

#define E2P_DATA                0x34

#define BURST_CAP               0x38

#define INT_EP_CTL              0x68
#define INT_EP_CTL_PHY_INT_     0x00008000

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

#define BULK_IN_DLY         0x6C

/* MAC CSRs */
#define MAC_CR              0x100
#define MAC_CR_MCPAS_       0x00080000
#define MAC_CR_PRMS_        0x00040000
#define MAC_CR_HPFILT_      0x00002000
#define MAC_CR_TXEN_        0x00000008
#define MAC_CR_RXEN_        0x00000004

#define ADDRH               0x104
#define ADDRL               0x108

#define MII_ADDR            0x114
#define MII_WRITE_          0x02
#define MII_BUSY_           0x01
#define MII_READ_           0x00 /* ~of MII Write bit */
#define MII_DATA            0x118

#define FLOW                0x11C
#define VLAN1               0x120

#define COE_CR              0x130
#define Tx_COE_EN_          0x00010000
#define Rx_COE_EN_          0x00000001

/* Vendor-specific PHY Definitions */
#define PHY_INT_SRC                  29
#define PHY_INT_MASK                 30
#define PHY_INT_MASK_ANEG_COMP_      ((unint2)0x0040)
#define PHY_INT_MASK_LINK_DOWN_      ((unint2)0x0010)
#define PHY_INT_MASK_DEFAULT_        (PHY_INT_MASK_ANEG_COMP_ | PHY_INT_MASK_LINK_DOWN_)

/* USB Vendor Requests */
#define USB_VENDOR_REQUEST_WRITE_REGISTER    0xA0
#define USB_VENDOR_REQUEST_READ_REGISTER    0xA1

/* Some extra defines */
//#define HS_USB_PKT_SIZE            512
#define HS_USB_PKT_SIZE              512
#define FS_USB_PKT_SIZE              64
#define DEFAULT_HS_BURST_CAP_SIZE    (16 * 1024 + 5 * HS_USB_PKT_SIZE)
#define DEFAULT_FS_BURST_CAP_SIZE    (6 * 1024 + 33 * FS_USB_PKT_SIZE)
#define DEFAULT_BULK_IN_DELAY        0x00002000
#define MAX_SINGLE_PACKET_SIZE       2048
#define EEPROM_MAC_OFFSET            0x01
#define SMSC95XX_INTERNAL_PHY_ID     1
#define ETH_P_8021Q    0x8100          /* 802.1Q VLAN Extended Header  */

/* local defines */
#define SMSC95XX_BASE_NAME "sms"
#define USB_CTRL_SET_TIMEOUT        5000
#define USB_CTRL_GET_TIMEOUT        5000
#define USB_BULK_SEND_TIMEOUT       5000
#define USB_BULK_RECV_TIMEOUT       5000

#define AX_RX_URB_SIZE              2048
#define PHY_CONNECT_TIMEOUT         5000

/* Activate Turbo Mode */
#define TURBO_MODE

#define PKTSIZE         1518

/* driver private */
struct smsc95xx_private {
    size_t rx_urb_size; /* maximum USB URB size */
    unint4 mac_cr; /* MAC control register value */
    int have_hwaddr; /* 1 if we have a hardware MAC address */
};

/***************************************
 *             VARIABLES
 ***************************************/

/* Default Mac Address */
static char default_macaddr[6] = { 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0 };

/* usb transfer memory area. placed in cache inhibit memory region to allow ehci hc
 * to see the correct data  */
static char tmp_data[2048] ATTR_CACHE_INHIBIT;

int4 tmpbuf[2] __attribute__((aligned(4))) ATTR_CACHE_INHIBIT;

/* control msg variable */
static char msg[8] ATTR_CACHE_INHIBIT;

unint4 dummyFrame[2] = { (TX_CMD_A_FIRST_SEG_ | TX_CMD_A_LAST_SEG_), 0x0 };

extern void* comStackMutex;

/***************************************
 *             IMPLEMENTATION
 ***************************************/

/*****************************************************************************
 * Method: smsc95xx_write_reg(USBDevice *dev, unint2 index, unint4 data)
 *
 * @description
 *  Write SMSC95xx register
 *
 * @params
 *    dev       The SMSC95xx USB device to write to
 *    index     register index to write
 *    data      the data to write
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
static int smsc95xx_write_reg(USBDevice *dev, unint2 index, unint4 data) {
    tmpbuf[0] = data;
    index = cputobe16(index);

    char msg2[8] = { USB_TYPE_VENDOR | USB_RECIP_DEVICE, USB_VENDOR_REQUEST_WRITE_REGISTER, 0x0, 0x0, (unint1) ((index & 0xff00) >> 8),
            (unint1) (index & 0xff), sizeof(data), 0x0 };

    memcpy(msg, msg2, 8);

    int4 error = dev->controller->sendUSBControlMsg(dev,                                  /* device to send control msg to*/
                                                    0,                                    /* endpoint number */
                                                    msg,                                  /* control msg*/
                                                    USB_DIR_OUT,                          /* transfer direction */
                                                    sizeof(data),                         /* length of data */
                                                    reinterpret_cast<char*>(&tmpbuf));    /* data  */

    /* check if all bytes are written */
    if (error != 4) {
        LOG(ARCH, DEBUG, "smsc95xx_write_reg failed: index=%d, data=%d, len=%d", index, data, sizeof(data));
        return (-1);
    }

    return (cOk);
}

/*****************************************************************************
 * Method: smsc95xx_read_reg(USBDevice *dev, unint2 index, unint4 *data)
 *
 * @description
 *  Read register from SMSC95xx
 *
 * @params
 *    dev       The SMSC95xx USB device to read from
 *    index     Register index to read
 *
 * @returns
 *  int         Error Code
 *  data        The Register contents
 *******************************************************************************/
static int smsc95xx_read_reg(USBDevice *dev, unint2 index, unint4 *data) {
    index = cputobe16(index);

    char msg2[8] = { 0x80 | USB_TYPE_VENDOR | USB_RECIP_DEVICE, USB_VENDOR_REQUEST_READ_REGISTER, 0x0, 0x0,
                    (unint1) ((index & 0xff00) >> 8), (unint1) (index & 0xff), sizeof(data), 0x0 };
    memcpy(msg, msg2, 8);

    int4 error = dev->controller->sendUSBControlMsg(dev,                                 /* device to send control msg to*/
                                                    0,                                   /* endpoint number */
                                                    msg,                                 /* control msg*/
                                                    USB_DIR_IN,                          /* transfer direction */
                                                    sizeof(data),                        /* length of data */
                                                    reinterpret_cast<char*>(&tmpbuf));   /* data  */

    if (error != 4) {
        LOG(ARCH, DEBUG, "smsc95xx_read_reg failed: index=%d, data=%d, len=%d, error=%d", index, *data, sizeof(data), error);
        return (cError );
    }

    *data = tmpbuf[0];
    return (cOk );
}

/*****************************************************************************
 * Method: smsc95xx_phy_wait_not_busy(USBDevice *dev)
 *
 * @description
 *  Waits until the PHY is ready with timeout
 *
 * @params
 *  dev         Device to read from
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
static int smsc95xx_phy_wait_not_busy(USBDevice *dev) {
    volatile int4 timeout = 1000;
    unint4 val;

    /*  Loop until the read is completed with timeout */
    do {
        timeout--;
        smsc95xx_read_reg(dev, MII_ADDR, &val);
        if (!(val & MII_BUSY_))
            return (0);
    } while (timeout > 0);

    return (-1);
}

/*****************************************************************************
 * Method: smsc95xx_mdio_read(USBDevice *dev, int phy_id, int idx)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
static int smsc95xx_mdio_read(USBDevice *dev, int phy_id, int idx) {
    unint4 val, addr;

    /* confirm MII not busy */
    if (smsc95xx_phy_wait_not_busy(dev)) {
        LOG(ARCH, DEBUG, "MII is busy in smsc95xx_mdio_read");
        return (-1);
    }

    /* set the address, index & direction (read from PHY) */
    addr = (phy_id << 11) | (idx << 6) | MII_READ_;
    smsc95xx_write_reg(dev, MII_ADDR, addr);

    if (smsc95xx_phy_wait_not_busy(dev)) {
        LOG(ARCH, DEBUG, "Timed out reading MII reg %02X", idx);
        return (-1);
    }

    smsc95xx_read_reg(dev, MII_DATA, &val);

    return ((unint2) (val & 0xFFFF));
}

/*****************************************************************************
 * Method: smsc95xx_mdio_write(USBDevice *dev, int phy_id, int idx, int regval)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
static void smsc95xx_mdio_write(USBDevice *dev, int phy_id, int idx, int regval) {
    unint4 val, addr;

    /* confirm MII not busy */
    if (smsc95xx_phy_wait_not_busy(dev)) {
        LOG(ARCH, DEBUG, "MII is busy in smsc95xx_mdio_write");
        return;
    }

    val = regval;
    smsc95xx_write_reg(dev, MII_DATA, val);

    /* set the address, index & direction (write to PHY) */
    addr = (phy_id << 11) | (idx << 6) | MII_WRITE_;
    smsc95xx_write_reg(dev, MII_ADDR, addr);

    if (smsc95xx_phy_wait_not_busy(dev))
        LOG(ARCH, DEBUG, "Timed out writing MII reg %02X", idx);
}

#if 0
/*****************************************************************************
 * Method: smsc95xx_eeprom_confirm_not_busy(USBDevice *dev)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
static int smsc95xx_eeprom_confirm_not_busy(USBDevice *dev) {
    volatile int4 timeout = 10000;
    unint4 val;

    do {
        timeout--;
        smsc95xx_read_reg(dev, E2P_CMD, &val);
        if (!(val & E2P_CMD_BUSY_))
        return 0;
    } while (timeout > 0);

    LOG(ARCH, DEBUG, "EEPROM is busy"));
    return -1;
}

/*****************************************************************************
 * Method: smsc95xx_wait_eeprom(USBDevice *dev)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
static int smsc95xx_wait_eeprom(USBDevice *dev) {
    volatile int timeout = 10000;
    unint4 val;

    do {
        timeout--;
        smsc95xx_read_reg(dev, E2P_CMD, &val);
        if (!(val & E2P_CMD_BUSY_) || (val & E2P_CMD_TIMEOUT_))
        break;
    } while (timeout > 0);

    if (val & (E2P_CMD_TIMEOUT_ | E2P_CMD_BUSY_)) {
        LOG(ARCH, DEBUG, "EEPROM read operation timeout");
        return -1;
    }
    return 0;
}

/*****************************************************************************
 * Method: smsc95xx_read_eeprom(USBDevice *dev,
 *                              unint4 offset,
 *                              unint4 length,
 *                              unint1 *data)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
static int smsc95xx_read_eeprom(USBDevice *dev, unint4 offset, unint4 length, unint1 *data) {
    unint4 val, i;
    int ret;

    ret = smsc95xx_eeprom_confirm_not_busy(dev);
    if (ret)
    return ret;

    for (i = 0; i < length; i++) {
        val = E2P_CMD_BUSY_ | E2P_CMD_READ_ | (offset & E2P_CMD_ADDR_);
        smsc95xx_write_reg(dev, E2P_CMD, val);

        ret = smsc95xx_wait_eeprom(dev);
        if (ret < 0)
        return ret;

        smsc95xx_read_reg(dev, E2P_DATA, &val);
        memdump((unint4) &val, 1);
        data[i] = val & 0xFF;
        offset++;
    }
    return 0;
}
#endif


/*****************************************************************************
 * Method: mii_nway_restart(USBDevice *dev)
 *
 * @description
 *  restart NWay (autonegotiation) for this interface
 *
 * @params
 *  dev         The SMSC95xx usb device
 *
 * @returns
 *  int         Error Code:  0 on success, negative on error.
 *******************************************************************************/
static int mii_nway_restart(USBDevice *dev) {
    int bmcr;
    int r = -1;

    /* if autoneg is off, it's an error */
    bmcr = smsc95xx_mdio_read(dev, SMSC95XX_INTERNAL_PHY_ID, MII_BMCR);

    if (bmcr & BMCR_ANENABLE) {
        bmcr |= BMCR_ANRESTART;
        smsc95xx_mdio_write(dev, SMSC95XX_INTERNAL_PHY_ID, MII_BMCR, bmcr);
        r = 0;
    }
    return (r);
}

/*****************************************************************************
 * Method: smsc95xx_phy_initialize(USBDevice *dev)
 *
 * @description
 *  Initializes the SMSC95xx PHY
 *
 * @params
 * dev         The SMSC95xx usb device
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
static int smsc95xx_phy_initialize(USBDevice *dev) {
    smsc95xx_mdio_write(dev, SMSC95XX_INTERNAL_PHY_ID, MII_BMCR, BMCR_RESET);
    smsc95xx_mdio_write(dev, SMSC95XX_INTERNAL_PHY_ID, MII_ADVERTISE,
    ADVERTISE_ALL | ADVERTISE_CSMA | ADVERTISE_PAUSE_CAP |
    ADVERTISE_PAUSE_ASYM);

    /* read to clear */
    smsc95xx_mdio_read(dev, SMSC95XX_INTERNAL_PHY_ID, PHY_INT_SRC);

    smsc95xx_mdio_write(dev, SMSC95XX_INTERNAL_PHY_ID, PHY_INT_MASK,
    PHY_INT_MASK_DEFAULT_);
    mii_nway_restart(dev);

    return (0);
}

#if 0

/*****************************************************************************
 * Method: smsc95xx_init_mac_address(char* ethaddr, USBDevice *dev)
 *
 * @description
 *   read mac address out of eeprom
 *
 * @params
 *  dev         The SMSC95xx USB Device
 *
 * @returns
 *  int         Error Code. 0 on success if mac address was read. -1 on error
 *  ethaddr     The MAC Address from eeprom if available.
 *******************************************************************************/
static int smsc95xx_init_mac_address(char* ethaddr, USBDevice *dev) {
    /* try reading mac address from EEPROM */
    if (smsc95xx_read_eeprom(dev, EEPROM_MAC_OFFSET, 6, reinterpret_cast<unint1*>(ethaddr)) == 0) {
        return 0;
    }

    /*
     * No eeprom, or eeprom values are invalid. Generating a random MAC
     * address is not safe. Just return an error.
     */
    return (-1);
}
#endif

/*****************************************************************************
 * Method: smsc95xx_write_hwaddr(USBDevice* dev, char* ethaddr)
 *
 * @description
 *  Sets the MAC address of the SMSC95xx Device
 *
 * @params
 *   dev         The SMSC95xx USB Device
 *   ethaddr     MAC address to use
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
static int smsc95xx_write_hwaddr(USBDevice* dev, char* ethaddr) {
    /* TODO this seams buggy.. */
    unint4 addr_lo = __get_unaligned_le32(&ethaddr[0]);
    unint4 addr_hi = __get_unaligned_le16(&ethaddr[4]);
    int ret;

    /* set hardware address */
    ret = smsc95xx_write_reg(dev, ADDRL, addr_lo);
    if (ret < 0) {
        LOG(ARCH, ERROR, "smsc95xx_write_hwaddr() error setting mac address\n");
        return (ret);
    }

    ret = smsc95xx_write_reg(dev, ADDRH, addr_hi);
    if (ret < 0) {
        LOG(ARCH, ERROR, "smsc95xx_write_hwaddr() error setting mac address\n");
        return (ret);
    }

    return (0);
}

/*****************************************************************************
 * Method: smsc95xx_set_csums(USBDevice *dev, int use_tx_csum, int use_rx_csum)
 *
 * @description
 *  Enable or disable TX & RX checksum offload engines
 *
 * @params
 *   dev         The SMSC95xx USB Device
 * @returns
 *  int         Error Code
 *******************************************************************************/
static int smsc95xx_set_csums(USBDevice *dev, int use_tx_csum, int use_rx_csum) {
    unint4 read_buf;
    int ret = smsc95xx_read_reg(dev, COE_CR, &read_buf);
    if (ret < 0)
        return (ret);

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
        return (ret);

    return (0);
}

/*****************************************************************************
 * Method: smsc95xx_set_multicast(USBDevice* dev)
 *
 * @description
 *  Enables multicast support
 *
 * @params
 *   dev        The SMSC95xx USB Device
 * @returns
 *  int         Error Code
 *******************************************************************************/
static void smsc95xx_set_multicast(USBDevice* dev) {
    struct smsc95xx_private *priv = reinterpret_cast<smsc95xx_private*>(dev->dev_priv);
    priv->mac_cr &= ~(MAC_CR_PRMS_ | MAC_CR_MCPAS_ | MAC_CR_HPFILT_);
}


/*****************************************************************************
 * Method: smsc95xx_start_tx_path(USBDevice *dev)
 *
 * @description
 *  Starts the TX path. Enables packet transmission
 *
 * @params
 *   dev        The SMSC95xx USB Device
 * @returns
 *  int         Error Code
 *******************************************************************************/
static void smsc95xx_start_tx_path(USBDevice *dev) {
    struct smsc95xx_private *priv = reinterpret_cast<smsc95xx_private*>(dev->dev_priv);
    unint4 reg_val;

    /* Enable Tx at MAC */
    priv->mac_cr |= MAC_CR_TXEN_;

    smsc95xx_write_reg(dev, MAC_CR, priv->mac_cr);

    /* Enable Tx at SCSRs */
    reg_val = TX_CFG_ON_;
    smsc95xx_write_reg(dev, TX_CFG, reg_val);
}


/*****************************************************************************
 * Method: smsc95xx_start_rx_path(USBDevice *dev)
 *
 * @description
 *  Starts the RX path. Enables packet reception
 *
 * @params
 *   dev        The SMSC95xx USB Device
 * @returns
 *  int         Error Code
 *******************************************************************************/
static void smsc95xx_start_rx_path(USBDevice *dev) {
    struct smsc95xx_private *priv =  reinterpret_cast<smsc95xx_private*>(dev->dev_priv);

    priv->mac_cr |= MAC_CR_RXEN_;
    smsc95xx_write_reg(dev, MAC_CR, priv->mac_cr);
}


/*****************************************************************************
 * Method: SMSC95xxUSBDeviceDriver::init()
 *
 * @description
 *  Try to initialize the hardware. Initializes all PHY, MII etc. components
 *  using USB control transfers to the SMSC95xx usb device.
 *
 * @params
 *
 * @returns
 *  int         Error Code. cOk == 0 on success.
 *******************************************************************************/
ErrorT SMSC95xxUSBDeviceDriver::init() {
    unint4 write_buf;
    unint4 read_buf;
    unint4 burst_cap;
    int ret;
    volatile int timeout;

    struct smsc95xx_private *priv = (struct smsc95xx_private *) dev->dev_priv;

    write_buf = HW_CFG_LRST_;
    ret = smsc95xx_write_reg(dev, HW_CFG, write_buf);
    if (ret < 0)
        return (ret);

    timeout = 0;
    do {
        ret = smsc95xx_read_reg(dev, HW_CFG, &read_buf);
        if (ret < 0)
            return (ret);

        timeout++;
    } while ((read_buf & HW_CFG_LRST_) && (timeout < 1000000));

    if (timeout >= 100) {
        LOG(ARCH, ERROR, "SMSC95xxUSBDeviceDriver: timeout waiting for completion of Lite Reset");
        return (-1);
    }
    LOG(ARCH, TRACE, "SMSC95xxUSBDeviceDriver: SMSC95xx reset successfully..");

    write_buf = PM_CTL_PHY_RST_;
    ret = smsc95xx_write_reg(dev, PM_CTRL, write_buf);
    if (ret < 0)
        return (ret);

    timeout = 0;
    do {
        ret = smsc95xx_read_reg(dev, PM_CTRL, &read_buf);
        if (ret < 0)
            return (ret);

        timeout++;
    } while ((read_buf & PM_CTL_PHY_RST_) && (timeout < 10000000));
    if (timeout >= 100) {
        LOG(ARCH, ERROR, "SMSC95xxUSBDeviceDriver: timeout waiting for PHY Reset");
        return (-1);
    }

    LOG(ARCH, TRACE, "SMSC95xxUSBDeviceDriver: SMSC95xx PHY reset successfully..");

    // get preprogrammed mac address
    // beagleboardxm does not have a hard coded mac address
    /*if (! (smsc95xx_init_mac_address((char*) &ethaddr, dev) == 0)) {
     printf("Error: SMSC95xx: No MAC address set\r");
     return -1;
     }

     printf("SMSC95xx Got mac address from eeprom..\r");
     memdump((unint4) &ethaddr,2);*/

    if (smsc95xx_write_hwaddr(dev, reinterpret_cast<char*>(&default_macaddr)) < 0)
        return (-1);

    ret = smsc95xx_read_reg(dev, HW_CFG, &read_buf);
    if (ret < 0)
        return (ret);
    LOG(ARCH, TRACE, "SMSC95xxUSBDeviceDriver: Read Value from HW_CFG : 0x%08x", read_buf);

    read_buf |= HW_CFG_BIR_;
    ret = smsc95xx_write_reg(dev, HW_CFG, read_buf);
    if (ret < 0)
        return (ret);

    ret = smsc95xx_read_reg(dev, HW_CFG, &read_buf);
    if (ret < 0)
        return (ret);
    LOG(ARCH, TRACE, "SMSC95xxUSBDeviceDriver: Read Value from HW_CFG after writing HW_CFG_BIR_: 0x%08x", read_buf);

#ifdef TURBO_MODE
//    if (dev->pusb_dev->speed == USB_SPEED_HIGH) {
    burst_cap = DEFAULT_HS_BURST_CAP_SIZE / HS_USB_PKT_SIZE;
    priv->rx_urb_size = DEFAULT_HS_BURST_CAP_SIZE;
    /*    } else {
     burst_cap = DEFAULT_FS_BURST_CAP_SIZE / FS_USB_PKT_SIZE;
     priv->rx_urb_size = DEFAULT_FS_BURST_CAP_SIZE;
     }*/
#else
    burst_cap = 0;
    priv->rx_urb_size = MAX_SINGLE_PACKET_SIZE;
#endif
    LOG(ARCH, TRACE, "SMSC95xxUSBDeviceDriver: rx_urb_size=%d", (unint4) priv->rx_urb_size);

    ret = smsc95xx_write_reg(dev, BURST_CAP, burst_cap);
    if (ret < 0)
        return (ret);

    ret = smsc95xx_read_reg(dev, BURST_CAP, &read_buf);
    if (ret < 0)
        return (ret);
    LOG(ARCH, TRACE, "SMSC95xxUSBDeviceDriver: Read Value from BURST_CAP after writing: 0x%08x", read_buf);

    read_buf = DEFAULT_BULK_IN_DELAY;
    //read_buf = 0x1000;
    ret = smsc95xx_write_reg(dev, BULK_IN_DLY, read_buf);
    if (ret < 0)
        return (ret);

    ret = smsc95xx_read_reg(dev, BULK_IN_DLY, &read_buf);
    if (ret < 0)
        return (ret);
    LOG(ARCH, TRACE, "SMSC95xxUSBDeviceDriver: Read Value from BULK_IN_DLY after writing: 0x%08x", read_buf);

    ret = smsc95xx_read_reg(dev, HW_CFG, &read_buf);
    if (ret < 0)
        return (ret);
    LOG(ARCH, TRACE, "SMSC95xxUSBDeviceDriver: Read Value from HW_CFG: 0x%08x", read_buf);

#ifdef TURBO_MODE
    read_buf |= (HW_CFG_MEF_ | HW_CFG_BCE_);
#endif
    read_buf &= ~HW_CFG_RXDOFF_;

#define NET_IP_ALIGN 0
    read_buf |= NET_IP_ALIGN << 9;

    ret = smsc95xx_write_reg(dev, HW_CFG, read_buf);
    if (ret < 0)
        return (ret);

    ret = smsc95xx_read_reg(dev, HW_CFG, &read_buf);
    if (ret < 0)
        return (ret);
    LOG(ARCH, TRACE, "SMSC95xxUSBDeviceDriver: Read Value from HW_CFG after writing: 0x%08x", read_buf);

    write_buf = 0xFFFFFFFF;
    ret = smsc95xx_write_reg(dev, INT_STS, write_buf);
    if (ret < 0)
        return (ret);

    ret = smsc95xx_read_reg(dev, ID_REV, &read_buf);
    if (ret < 0)
        return (ret);
    LOG(ARCH, TRACE, "SMSC95xxUSBDeviceDriver: ID_REV = 0x%08x", read_buf);

    /* Init Tx */
    write_buf = 0;
    ret = smsc95xx_write_reg(dev, FLOW, write_buf);
    if (ret < 0)
        return (ret);

    read_buf = AFC_CFG_DEFAULT;
    ret = smsc95xx_write_reg(dev, AFC_CFG, read_buf);
    if (ret < 0)
        return (ret);

    ret = smsc95xx_read_reg(dev, MAC_CR, &priv->mac_cr);
    if (ret < 0)
        return (ret);

    /* Init Rx. Set Vlan */
    write_buf = (unint4) ETH_P_8021Q;
    ret = smsc95xx_write_reg(dev, VLAN1, write_buf);
    if (ret < 0)
        return (ret);

    /* Enable checksum offload engines */
    ret = smsc95xx_set_csums(dev, 1, 1);
    if (ret < 0) {
        LOG(ARCH, ERROR, "SMSC95xxUSBDeviceDriver: Failed to set csum offload: %d", ret);
        return (ret);
    }
    smsc95xx_set_multicast(dev);

    if (smsc95xx_phy_initialize(dev) < 0)
        return (-1);
    ret = smsc95xx_read_reg(dev, INT_EP_CTL, &read_buf);
    if (ret < 0)
        return (ret);

    /* enable PHY interrupts */
    read_buf |= INT_EP_CTL_PHY_INT_;

    ret = smsc95xx_write_reg(dev, INT_EP_CTL, read_buf);
    if (ret < 0)
        return (ret);

    smsc95xx_start_tx_path(dev);
    smsc95xx_start_rx_path(dev);

    return (0);
}

/*****************************************************************************
 * Method: SMSC95xxUSBDeviceDriver::recv(unint4 recv_len)
 *
 * @description
 *  Receives the packet transfered by the IRQ usb transfer and
 *  checks the packet on completeness. If the packet is complete
 *  it is forwarded up the network stack.
 *
 * @params
 *  recv_len    The length received by the IRQ usb transfer.
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT SMSC95xxUSBDeviceDriver::recv(unint4 recv_len) {
    int4 packet_len;
    LOG(ARCH, DEBUG, "SMSC95xxUSBDeviceDriver: RX IRQ. len: %d", recv_len);

    unint4 recvd_bytes = 0;
    char* rxbuffer = reinterpret_cast<char*>(&dev->endpoints[4].recv_buffer[0]);
    if (recv_len <= 0)
        return (-1);

    while (recvd_bytes < recv_len) {
        if (remaining_len > 0) {
            char* p = reinterpret_cast<char*>(last_pbuf->payload);

            if (recv_len >= remaining_len) {
                LOG(ARCH, DEBUG, "SMSC95xxUSBDeviceDriver: rx split packet: %d", remaining_len);

                recvd_bytes += remaining_len;
                recvd_bytes = (recvd_bytes + 3) & (~3);

                memcpy(&p[cur_len], rxbuffer, remaining_len);
                remaining_len = 0;
                cur_len = 0;

                acquireMutex(comStackMutex);
                ethernet_input(last_pbuf, &st_netif);
                pbuf_free(last_pbuf);
                releaseMutex(comStackMutex);
            } else {
                LOG(ARCH, DEBUG, "SMSC95xxUSBDeviceDriver: rx split packet: %d", recv_len);

                /* more packets */
                memcpy(&p[cur_len], rxbuffer, recv_len);
                remaining_len -= recv_len;
                cur_len += recv_len;

                recvd_bytes += recv_len;
                recvd_bytes = (recvd_bytes + 3) & (~3);
            }
        } else {
            memcpy(&packet_len, &rxbuffer[recvd_bytes], sizeof(packet_len));

            /* cpu to le */
            packet_len = cputole32(packet_len);

            if (packet_len & RX_STS_ES_) {
                LOG(ARCH, ERROR, "SMSC95xxUSBDeviceDriver: RX packet header Error: %x", packet_len);
                return (cError );
            }
            /* extract packet_len */
            packet_len = ((packet_len & RX_STS_FL_) >> 16);
            LOG(ARCH, DEBUG, "SMSC95xxUSBDeviceDriver: Packet received.. Packet-Len %d", packet_len);

            if ((packet_len > 1500) || (packet_len == 0)) {
                LOG(ARCH, ERROR, "SMSC95xxUSBDeviceDriver: Length Validation failed: %d", packet_len);
                return (cError );
            }

            /* deliver packet to upper layer */
            acquireMutex(comStackMutex);
            struct pbuf* ptBuf = pbuf_alloc(PBUF_RAW, (unint2) (packet_len + 10), PBUF_RAM);
            if (ptBuf != 0) {
                int r_len = (recv_len - (recvd_bytes + 4));
                /* check if packet is complete */
                if (packet_len <= r_len) {
                    memcpy(ptBuf->payload, &rxbuffer[recvd_bytes + 4], packet_len);
                    ethernet_input(ptBuf, &st_netif);
                    pbuf_free(ptBuf);
                } else {
                    memcpy(ptBuf->payload, &rxbuffer[recvd_bytes + 4], r_len);
                    last_pbuf = ptBuf;
                    cur_len = r_len;
                    remaining_len = packet_len - cur_len;

                    dev->endpoints[bulkin_ep].data_toggle = dev->endpoints[4].data_toggle;

                    /*recv_len = dev->controller->USBBulkMsg(dev,bulkin_ep,USB_DIR_IN,512,tmp_data);
                     if ((int) recv_len >= remaining_len) {
                     dev->endpoints[4].data_toggle = dev->endpoints[bulkin_ep].data_toggle;

                     char* p = (char*) last_pbuf->payload;
                     memcpy(&p[cur_len],tmp_data, remaining_len );
                     recvd_bytes = 0;
                     packet_len = remaining_len - 4;
                     remaining_len = 0;
                     cur_len = 0;
                     rxbuffer = (char*) tmp_data;
                     ethernet_input(last_pbuf,&tEMAC0Netif);
                     pbuf_free(last_pbuf);
                     } else if (recv_len > 0) {
                     dev->endpoints[4].data_toggle = dev->endpoints[bulkin_ep].data_toggle;
                     // more packets
                     char* p = (char*) last_pbuf->payload;
                     memcpy(&p[cur_len],tmp_data,recv_len);
                     remaining_len -= recv_len;
                     cur_len += recv_len;

                     recvd_bytes += recv_len;
                     recvd_bytes = (recvd_bytes + 3) & (~3);
                     }*/
                }
            } else {
                LOG(ARCH, ERROR, "SMSC95xxUSBDeviceDriver: no memory for pbuf!");
            }
            releaseMutex(comStackMutex);

            recvd_bytes += packet_len + 4;
            recvd_bytes = (recvd_bytes + 3) & (~3);
        }
    }

    return (cOk );
}

SMSC95xxUSBDeviceDriver::SMSC95xxUSBDeviceDriver(USBDevice* p_dev) :
        USBDeviceDriver(),
        CommDeviceDriver("eth0") {
    this->dev = p_dev;
    this->bulkin_ep = 0;
    this->bulkout_ep = 0;
    this->int_ep = 0;
    this->last_pbuf = 0;
    this->remaining_len = 0;
    this->cur_len = 0;
    this->mutex = 0;
    LOG(ARCH, INFO, "SMSC95xxUSBDeviceDriver: new Device attached..");
    /* PR 1*/
    kwait(10);

    link_up = false;
}

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

/*****************************************************************************
 * Method: smsc95xx_low_level_output(struct netif *netif, struct pbuf *p)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  err_t         Error Code. 0 on success
 *******************************************************************************/
static err_t smsc95xx_low_level_output(struct netif *netif, struct pbuf *p) {
    LOG(ARCH, DEBUG, "SMSC95xxUSBDeviceDriver: sending packet of length: %d", p->tot_len);

    if (p->tot_len > 1500) {
        LOG(ARCH, WARN, "SMSC95xxUSBDeviceDriver: not sending packet. Len > 1500.");
        return (ERR_MEM);
    }

    if (p == 0) {
        return (ERR_ARG);
    }

    SMSC95xxUSBDeviceDriver *driver = reinterpret_cast<SMSC95xxUSBDeviceDriver*>(netif->state);
    driver->mutex->acquire();

    unint2 pos = 0;
    struct pbuf *curp = p;
    unint2 len = p->tot_len;

    unint4 tx_cmd_a;
    unint4 tx_cmd_b;

    tx_cmd_a = cputole32(((unint4) len) | TX_CMD_A_FIRST_SEG_ | TX_CMD_A_LAST_SEG_);
    tx_cmd_b = cputole32((unint4) len);

    pos = sizeof(tx_cmd_a) + sizeof(tx_cmd_b);
    char* data = tmp_data;

    while (curp != 0) {
        memcpy(&tmp_data[pos], curp->payload, curp->len);
        pos = (unint2) (pos + curp->len);
        curp = curp->next;
    }

    /* prepend cmd_a and cmd_b */
    memcpy(&data[0], &tx_cmd_a, sizeof(tx_cmd_a));
    memcpy(&data[0] + sizeof(tx_cmd_a), &tx_cmd_b, sizeof(tx_cmd_b));

    if (driver != 0) {
        LOG(ARCH, DEBUG, "SMSC95xxUSBDeviceDriver: sending packet len %d", len);

        /* Send dummy frame to ensure completion of previous frame ...
         * maybe this is not needed at all. needs more testing:
         * without this the smsc stalls with too much data send .. ? */
        int error = driver->dev->controller->USBBulkMsg(driver->dev, driver->bulkout_ep, USB_DIR_OUT, 8, reinterpret_cast<char*>(dummyFrame));
        error |= driver->dev->controller->USBBulkMsg(driver->dev, driver->bulkout_ep, USB_DIR_OUT, (unint2) (len + 8), data);
    }

    driver->mutex->release();
    return (ERR_OK);
}

/*****************************************************************************
 * Method: smsc9x_ethernetif_init(struct netif *netif)
 *
 * @description
 *  Callback method for the network stack in netif initialization.
 *
 * @params
 *  netif       The netif structure assigned to initialize
 *
 * @returns
 *  err_t         Error Code. 0 on success
 *******************************************************************************/
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
#warning "SMSC95xx USB device Driver non operational without ARP support."
#endif
    netif->input        = ethernet_input;
    netif->linkoutput   = smsc95xx_low_level_output;
    netif->hwaddr_len   = 6;

    for (int i = 0; i < netif->hwaddr_len; i++) {
        netif->hwaddr[i] = default_macaddr[i];
    }

    /* maximum transfer unit */
    //netif->mtu = HS_USB_PKT_SIZE - 100; //MAX_FRAME_SIZE;
    netif->mtu          = 1400;
    netif->rxbytes      = 0;
    netif->txbytes      = 0;
    netif->rxpackets    = 0;
    netif->txpackets    = 0;
    netif->txerrors     = 0;

    /* device capabilities */
    /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
    return (cOk );
}

/*****************************************************************************
 * Method: SMSC95xxUSBDeviceDriver::initialize()
 *
 * @description
 *  Tries to initialize the SMSC95xx device. Probes the endpoint.
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT SMSC95xxUSBDeviceDriver::initialize() {
   /* try to initialize the device
    * first check the endpoint information */

    bool bulkinep, bulkoutep, intep = false;

    for (unint1 i = 1; i < 5; i++) {
        if (dev->endpoints[i].type == Bulk) {
            /* active it */
            if (dev->endpoints[i].direction == Out) {
                LOG(ARCH, DEBUG, "SMSC95xxUSBDeviceDriver: BulkOut EP: %d", i);
                bulkout_ep = i;
                bulkoutep = true;
                dev->activateEndpoint(i);
            } else {
                bulkin_ep = i;
                bulkinep = true;
                dev->activateEndpoint(i);
                LOG(ARCH, DEBUG, "SMSC95xxUSBDeviceDriver: BulkIn EP: %d", i);
            }

        } else if (dev->endpoints[i].type == Interrupt) {
            LOG(ARCH, DEBUG, "SMSC95xxUSBDeviceDriver: IRQ EP: %d", i);
            int_ep = i;
            dev->endpoints[i].poll_frequency = 200;
            intep = true;
        }
    }

    if (!(bulkinep & bulkoutep & intep)) {
        LOG(ARCH, ERROR, "SMSC95xxUSBDeviceDriver: probing failed..");
        /* deactivate endpoints again */
        return (cError );
    }

    /* internally handle RX transfer as interrupt requests
     * Therefore we clone the endpoint information to simulate
     * another endpoint identically to the bulk endpoint.
     * We then change the type to interrupt, remove the queue head pointer
     * and set the receive size to the maximum transferrable with an
     * interrupt request.
     * Last we activate the cloned endpoint to start the interrupt transfer
     * to the bulk endpoint.
     **/
    memcpy(&dev->endpoints[4], &dev->endpoints[bulkin_ep], sizeof(DeviceEndpoint));
    dev->endpoints[4].queue_head                = 0;
    dev->endpoints[4].interrupt_receive_size    = 512*3;
    dev->endpoints[4].type                      = Interrupt;
    dev->endpoints[4].poll_frequency            = 2;
    dev->activateEndpoint(4);

    /* get our private structure  */
    dev->dev_priv = theOS->getMemoryManager()->alloc(sizeof(struct smsc95xx_private));

    /* Try initializing the device */
    if (init() < 0) {
        LOG(ARCH, ERROR, "SMSC95xxUSBDeviceDriver: Initializing Ethernet device failed..");
        return (cError );
    }

    mutex = new Mutex("SMSC95xx");

    /* setup periodic interrupt transfer that
     * will get e.g. port status changes  */
    dev->activateEndpoint(int_ep);

    /* set netmask for this device */
    struct ip4_addr eth_nm;

#ifndef Board_ETH_IP4NETMASK
#define Board_ETH_IP4NETMASK 255, 255, 255, 0
#endif

    int netmask[4] = { Board_ETH_IP4NETMASK };
    IP4_ADDR(&eth_nm, netmask[0], netmask[1], netmask[2], netmask[3]);

#ifndef Board_ETH_IP4ADDR
#define Board_ETH_IP4ADDR 192, 168, 1, 100
#endif

    int ipaddr[4] = { Board_ETH_IP4ADDR };

    struct ip4_addr tgwAddr;
    IP4_ADDR(&tgwAddr, 192, 168, 1, 1);
    IP4_ADDR(&tIpAddr, ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3]);

    /* save driver in netif as state
     * use ethernet interface init method in lwip_ethernetif.c */
    netif_add(&st_netif, &tIpAddr, &eth_nm, &tgwAddr, this, &smsc9x_ethernetif_init, 0);
    netif_set_default(&st_netif);
    netif_set_down(&st_netif);

    return (cOk );
}


/*****************************************************************************
 * Method: SMSC95xxUSBDeviceDriver::handleInterrupt()
 *
 * @description
 *  SMSC95xx Interrupt Handler called by the USB EHCI CH Implementation
 *  if an irq request succeeds. This handler handles standard irq
 *  received from the interrupt endpoint of the smsc95 as well as
 *  interrupts due to successfull packet receptions done by the
 *  virtual interrupt created above.
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT SMSC95xxUSBDeviceDriver::handleInterrupt() {
    ErrorT ret = cError;

    volatile QH* qh = dev->endpoints[this->int_ep].queue_head;
    if (qh == 0)
        return (cError );
    volatile qTD* qtd2 = reinterpret_cast<qTD*>(qh->qh_curtd);

    if (((unint4) qtd2 > QT_NEXT_TERMINATE) && (QT_TOKEN_GET_STATUS(qh->qh_overlay.qt_token) != 0x80)) {
        LOG(ARCH, DEBUG, "SMSC95xxUSBDeviceDriver: PHY Interrupt");

        if (dev->endpoints[this->int_ep].recv_buffer == 0)
            return (cError );

        unint4 interrupt_sts = *(reinterpret_cast<unint4*>(dev->endpoints[this->int_ep].recv_buffer));

        LOG(ARCH, DEBUG, "SMSC95xxUSBDeviceDriver: PHY Interrupt: %x", interrupt_sts);
        /* read to clear */
        smsc95xx_mdio_read(dev, SMSC95XX_INTERNAL_PHY_ID, PHY_INT_SRC);

        if ((interrupt_sts & INT_EP_CTL_PHY_INT_) != 0) {
            /* clear interrupt status */
            LOG(ARCH, DEBUG, "SMSC95xxUSBDeviceDriver: Link Activated..");
            link_up = true;
            netif_set_default(&st_netif);
            netif_set_up(&st_netif);
        }

        /* clear received data */
        memset(&dev->endpoints[this->int_ep].recv_buffer[0], 0, dev->endpoints[this->int_ep].interrupt_receive_size);
        /* reactivate interrupt transfer */
        dev->activateEndpoint(int_ep);
        ret = cOk;
    }

    /* check if this was a bulk in transfer complete irq */
    qh = dev->endpoints[4].queue_head;
    qtd2 = reinterpret_cast<qTD*>(qh->qh_curtd);

    if (((unint4) qtd2 > QT_NEXT_TERMINATE) && (QT_TOKEN_GET_STATUS(qh->qh_overlay.qt_token) != 0x80)) {
        LOG(ARCH, DEBUG, "SMSC95xxUSBDeviceDriver: Packet received: status %x", QT_TOKEN_GET_STATUS(qh->qh_overlay.qt_token));

        int len = dev->endpoints[4].interrupt_receive_size - QT_TOKEN_GET_TOTALBYTES(qtd2->qt_token);

        /* handle received data */
        if (len > 0)
            recv(len);

        /* clear received data */
        memset(&dev->endpoints[4].recv_buffer[0], 0, dev->endpoints[4].interrupt_receive_size);
        /* set qtd back to active */

        /* dev->activateEndpoint(bulkin_ep); */
        dev->activateEndpoint(4);

        ret = cOk;
    }

    return (ret);
}

SMSC95xxUSBDeviceDriver::~SMSC95xxUSBDeviceDriver() {
    dev->deactivate();
    netif_remove(&st_netif);
}


/*****************************************************************************
 * Method: SMSC95xxUSBDeviceDriverFactory::isDriverFor(USBDevice* dev)
 *
 * @description
 *  checks whether the given class,product device is supported by this driver
 *
 * @params
 *  dev         The usb device to be checked.
 *
 * @returns
 *  bool         true if this driver factory provides a driver for this device.
 *******************************************************************************/
bool SMSC95xxUSBDeviceDriverFactory::isDriverFor(USBDevice* dev) {
    if (dev->dev_descr.idVendor != 0x0424)
        return (false);
    if (dev->dev_descr.idProduct == 0xec00 || dev->dev_descr.idProduct == 0x9500)
        return (true);

    return (false);
}


SMSC95xxUSBDeviceDriverFactory::SMSC95xxUSBDeviceDriverFactory(char* p_name) :
        USBDeviceDriverFactory(p_name) {
}
