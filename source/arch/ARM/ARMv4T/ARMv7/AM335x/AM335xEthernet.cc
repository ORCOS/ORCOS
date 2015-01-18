/*
 * AM335xEthernet.cc
 *
 *  Created on: 30.12.2014
 *    Copyright & Author: DanielBa
 */

#include "AM335xEthernet.hh"
#include "lwip/pbuf.h"
#include "netif/etharp.h"
#include "kernel/Kernel.hh"
#include "synchro/Mutex.hh"

extern Kernel* theOS;
extern Mutex* comStackMutex;

#define SOP                 (1 << 31)
#define EOP                 (1 << 30)
#define OWNER_MAC           (1 << 29) /* packet is owned by the port */
#define EOQ                 (1 << 28)
#define TERDOWN_COMPLETE    (1 << 27)
#define CRC_PASS            (1 << 26)
#define PORTEN2             (1 << 20)
#define PORTEN1             (1 << 16)
#define PACKET_LEN          (1 << 0)

#ifndef ETH_IP4NETMASK
#define ETH_IP4NETMASK  255,255,255,0
#endif
#ifndef ETH_IP4ADDR
#define ETH_IP4ADDR     192,168,1,100
#endif
#ifndef ETH_IP4ADDR_GW
#define ETH_IP4ADDR_GW  192,168,1,1
#endif

#define ALE_UNICAST 0x0

typedef struct cppi_tx_descriptor_t {
    volatile cppi_tx_descriptor_t* next_descr;
    volatile char* buffer_pointer;
    volatile unint4 buffer_opt;
    volatile unint4 control;
    /* private data following */
    //pbuf* p_buf; /* pointer to the pbuf holding the data. to be freed on completion */
} cppi_tx_descriptor_t;

typedef struct cppi_rx_descriptor_t {
    volatile cppi_rx_descriptor_t* next_descr;
    volatile char* buffer_pointer;
    volatile unint4 buffer_opt;
    volatile unint4 control;
} cppi_rx_descriptor_t;

/* some receive area to be used by the dma to store new packets*/
static char rx_buffers[10][1520] ATTR_CACHE_INHIBIT;
static char tx_buffers[10][1520] ATTR_CACHE_INHIBIT;

/* descriptors to be used by dma for all ports */
static cppi_tx_descriptor_t* txqueue;
static cppi_rx_descriptor_t* rxqueue;

/* the mac address used for this device */
char mac[6] = { 0x00, 0x01, 0x01, 0x01, 0x01, 0x01 };
char broadcastmac[6] ={ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

extern "C" err_t ethernet_input(struct pbuf *p, struct netif *netif);

static err_t low_level_output(struct netif *netif, struct pbuf *p) {
    LOG(COMM, TRACE, "AM335xEthernet::sendPacket(). Send packet %x len: %u", p, p->tot_len);
    AM335xEthernet* ethdev = (AM335xEthernet*) netif->state;
    ethdev->mutex->acquire();

    bool send = false;

    while (!send) {
        /* get a free descriptor and send the packet */
        for (int i = 0; i < 10; i++) {
            if (!(txqueue[i].control & OWNER_MAC)) {
                /* this one is free */

                memset(tx_buffers[i], 0, 64);
                struct pbuf *curp = p;
                int pos = 0;
                while (curp != 0) {
                    memcpy(&tx_buffers[i][pos], curp->payload, curp->len);
                    pos = (unint2) (pos + curp->len);
                    curp = curp->next;
                }

                int len = p->tot_len;
                if (len < 60) {
                    len = 60;
                }

                txqueue[i].buffer_pointer = (char*) tx_buffers[i];
                txqueue[i].buffer_opt = len;
                txqueue[i].control = SOP | EOP | OWNER_MAC | len;
                send = true;

                LOG(COMM, TRACE, "AM335xEthernet::sendPacket(). Sending %x at queue pos %u", p, i);
                break;
            }
        }
    }

    /* restart queue processing */
    ethdev->cpsw_stateram_regs->tx_hdp[0] = (unint4) &txqueue[0];
    ethdev->mutex->release();

    return (ERR_OK);
}

static err_t ethernetif_init(struct netif *netif) {

#if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    netif->hostname = "orcos";
#endif /* LWIP_NETIF_HOSTNAME */

    netif->name[0] = 'E';
    netif->name[1] = '0';

#if LWIP_ARP
    netif->ip4_output = etharp_output;
    netif->ip6_output = etharp_output_ipv6;
#else
#warning "AM335xEthernet device driver non operational without ARP support."
#endif
    netif->input      = ethernet_input;
    netif->linkoutput = low_level_output;
    netif->hwaddr_len = 6;

    /* set mac address of netif */
    for (int i = 0; i < netif->hwaddr_len; i++) {
        netif->hwaddr[i] = mac[i];
    }

    /* maximum transfer unit */
    netif->mtu = 1500;

    /* device capabilities */
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
    return (cOk );
}

void AM335xEthernet::ale_addAddress(int type, char* address, int port) {
    // type 00 = unicast
    static int entry = 1;

    int word0 = (address[2] << 24) | (address[3] << 16) | (address[4] << 8) | (address[5] << 0);
    int word1 = (address[0] << 8) | (address[1]) | (1 << 28) | (type << 30);
    int word2 = (port << 2);

    OUTW(((unint4 ) cpsw_ale_regs) + 0x34, word2);
    OUTW(((unint4 ) cpsw_ale_regs) + 0x38, word1);
    OUTW(((unint4 ) cpsw_ale_regs) + 0x3c, word0);

    /* write entry*/
    cpsw_ale_regs->tblctl = entry | (1 << 31);
    entry++;
}

AM335xEthernet::AM335xEthernet(T_AM335xEthernet_Init * init) :
        CommDeviceDriver(init->Name) {
    /*
     *  Step1 : Board initialization code must already have set the MII / GMII / RGMII mode of
     *          the ETH subsystem using the Control Module!
     *  Step2 : Pin muxing must be done before as well as this is board specific
     *  Step3 : Clocks must be provided as well as this is SOC specific
     *
     */

    mutex = new Mutex();

    /* set pointers to hw regs using the init struct */
    cpsw_wr_regs        = (cpsw_wr_regs_t*) init->cpsw_wr;
    cpsw_ss_regs        = (cpsw_ss_regs_t*) init->cpsw_ss;
    cpsw_sl_regs        = (cpsw_sl_regs_t*) init->cpsw_sl1;
    cpsw_p0_regs        = (cpsw_port_regs_t*) init->cpsw_port;
    cpsw_stateram_regs  = (cpsw_stateram_regs_t*) init->cpsw_stateram;
    cpsw_ale_regs       = (cpsw_ale_regs_t*) init->cpsw_ale;
    cpsw_dma_regs       = (cpsw_cpdma_regs_t*) init->cpsw_cpdma;
    cpsw_stats_regs     = (cpsw_stats_regs_t*) init->cpsw_stats;

    txqueue = (cppi_tx_descriptor_t*) (((unint4) init->cppi_ram));
    rxqueue = (cppi_rx_descriptor_t*) (&txqueue[11]);

    // Step4: Apply soft reset.
    cpsw_dma_regs->cpdma_soft_reset = 1;
    cpsw_ss_regs->soft_reset = 1;
    cpsw_sl_regs->soft_reset = 1;
    cpsw_wr_regs->soft_reset = 1;

    int version = cpsw_sl_regs->idver;
    LOG(ARCH, INFO, "AM335xEthernet: Silverlight [1] Version: %d.%d.%d, IDENT: %d",
        (version >> 8) && 0x7,
        (version >> 0) && 0xff,
        (version >> 11) && 0x1f,
        (version >> 16));

    version = cpsw_dma_regs->tx_idver;
    LOG(ARCH, INFO, "AM335xEthernet: CPDMA TX Version: %d.%d, IDENT: %d", (version >> 8) && 0xff, (version >> 0) && 0xff, (version >> 16));

    version = cpsw_dma_regs->rx_idver;
    LOG(ARCH, INFO, "AM335xEthernet: CPDMA RX Version: %d.%d, IDENT: %d", (version >> 8) && 0xff, (version >> 0) && 0xff, (version >> 16));

    version = cpsw_ss_regs->idver;
    LOG(ARCH, INFO, "AM335xEthernet: Statistics IP Version: %d.%d, IDENT: %d, RTL.v: %d",
        (version >> 8) && 0x7,
        (version >> 0) && 0xff,
        (version >> 16),
        (version >> 11) && 0x1f);

    version = cpsw_ss_regs->idver;
    LOG(ARCH, INFO, "AM335xEthernet: CPSW_WR (Interrupt Control IP) Version: %d.%d, RTL.v: %d",
        (version >> 8) && 0x7,
        (version >> 0) && 0x1f,
        (version >> 11) && 0x1f);

    /* wait for reset to be done.. timout 5 ms*/
    if (TIMEOUT_WAIT(cpsw_ss_regs->soft_reset | cpsw_sl_regs->soft_reset | cpsw_wr_regs->soft_reset | cpsw_dma_regs->cpdma_soft_reset, 5000)) {
        LOG(ARCH, INFO, "AM335xEthernet() Error waiting for reset. AM335xEthernet deactivated.");
        return;
    }

    /* Step5: initialize HDPs and CPs to NULL. */
    for (int i = 0; i < 8; i++) {
        cpsw_stateram_regs->tx_hdp[i] = 0;
        cpsw_stateram_regs->rx_hdp[i] = 0;
        cpsw_stateram_regs->tx_cp[i] = 0;
        cpsw_stateram_regs->rx_cp[i] = 0;
    }
    memset(txqueue, 0, sizeof(txqueue));
    memset(rxqueue, 0, sizeof(rxqueue));

    /* initialize rx queue */
    for (int i = 0; i < 10; i++) {
        rxqueue[i].next_descr = &rxqueue[i + 1];
        rxqueue[i].buffer_pointer = rx_buffers[i];
        rxqueue[i].buffer_opt = 1520;
        /* give ownership of this buffer to the hw */
        rxqueue[i].control = OWNER_MAC;
    }

    /* last descriptor points to first one again */
    rxqueue[9].next_descr = 0;

    /* initialize tx queue */
    for (int i = 0; i < 10; i++) {
        txqueue[i].next_descr = &txqueue[i + 1];
        txqueue[i].next_descr = 0;
        txqueue[i].buffer_pointer = 0;
        txqueue[i].buffer_opt = 0;
        /*nothin to transmit yet so do not let the hw own this descriptor */
        txqueue[i].control = 0;
    }
    /* last descriptor ends here */
    txqueue[0].next_descr = 0;
    txqueue[0].control    = EOP;

    /* Step6: Configure Interrupts*/
    /* allow all interrupts */
    cpsw_dma_regs->rx_intmask_clear = 0xffff;
    //cpsw_dma_regs->tx_intmask_clear = 0xffff;
    /* route all interrupts to core 0 */
    cpsw_wr_regs->c0_rx_en = 0xf;
    //cpsw_wr_regs->c0_tx_en = 0xf;
    cpsw_wr_regs->c0_rx_thresh_en = 0xf;

    /* register at interrupt manager */
    intc_irq = init->INTC_IRQ;
    theOS->getInterruptManager()->registerIRQ(intc_irq, this, 5000, 0);

    /* unmask the irq to let it pass through */
    theOS->getBoard()->getInterruptController()->setIRQPriority(intc_irq, init->INTC_Priority);
    theOS->getBoard()->getInterruptController()->unmaskIRQ(intc_irq);

    /* configure control registers */
    cpsw_wr_regs->control      = (1 << 0) | (1 << 2); /* no idle no standby mode */
    cpsw_ss_regs->stat_port_en = 7; /* enable statistics on all ports */
    cpsw_sl_regs->maccontrol   = 1 | (1 << 5); /* enable full duplex mode*/

    /* enable ale */
    cpsw_ale_regs->control = (1 << 31);
    /* clear ale table */
    cpsw_ale_regs->control |= (1 << 30);
    /* bypass mode (everything to port 0) */
    cpsw_ale_regs->control |=  (1 << 4);
    /* Unknown unicast addresses to port 0 */
    cpsw_ale_regs->control |= (1 << 8);

    /* forward on port 0 */
    OUTW(((unint4 ) cpsw_ale_regs) + 0x40, 0x3);

    /* forward on port 1 */
    OUTW(((unint4 ) cpsw_ale_regs) + 0x44, 0x3);

    /* set mac address of port 1*/
    OUTW(((unint4 )cpsw_p0_regs) + 0x120, 0x01010101);
    OUTW(((unint4 )cpsw_p0_regs) + 0x124, 0x0101);

    //ale_addAddress((cpsw_ale_regs_t*) cpsw_ale_regs, ALE_UNICAST, mac, 0);
    //ale_addAddress((cpsw_ale_regs_t*) cpsw_ale_regs, 0, broadcastmac, 1 << 0);

    //PORT 1 + 2 are external. PORT 0 is the cpsw_dma
    struct ip4_addr tgwAddr;
    struct ip4_addr eth_nm;
    struct ip4_addr tIpAddr;

    int addr[4] = { ETH_IP4NETMASK };
    IP4_ADDR(&eth_nm, addr[0], addr[1], addr[2], addr[3]);
    int addr2[4] = { ETH_IP4ADDR };
    IP4_ADDR(&tIpAddr, addr2[0], addr2[1], addr2[2], addr2[3]);
    int addr3[4] = { ETH_IP4ADDR_GW };
    IP4_ADDR(&tgwAddr, addr3[0], addr3[1], addr3[2], addr3[3]);

    /* save driver in netif as state */
    netif_add(&st_netif, &tIpAddr, &eth_nm, &tgwAddr, (void*) this, &ethernetif_init, 0);
    netif_set_default(&st_netif);

    LOG(ARCH, INFO, "AM335xEthernet: RX descriptor queue at %x", (unint4 ) &rxqueue[0]);
    LOG(ARCH, INFO, "AM335xEthernet: TX descriptor queue at %x", (unint4 ) &txqueue[0]);

    cpsw_dma_regs->rx0_freebuffer = 10;
    /* enable tx and rx dma */
    cpsw_dma_regs->tx_control = 1;
    cpsw_dma_regs->rx_control = 1;

    /* set rx descriptor in hw register to the first one */
    cpsw_stateram_regs->rx_hdp[0] = (unint4) &rxqueue[0];

    netif_set_up(&st_netif);
}

AM335xEthernet::~AM335xEthernet() {
}

ErrorT AM335xEthernet::disableIRQ() {
    theOS->getBoard()->getInterruptController()->maskIRQ(intc_irq);
    return (cOk );
}

ErrorT AM335xEthernet::enableIRQ() {
    theOS->getBoard()->getInterruptController()->unmaskIRQ(intc_irq);
    return (cOk );
}

ErrorT AM335xEthernet::clearIRQ() {
    theOS->getBoard()->getInterruptController()->maskIRQ(intc_irq);
    /* no real clear here as handleIRQ must be called for that */
    return (cOk );
}

ErrorT AM335xEthernet::handleIRQ() {

    /* scan rx queue and process the data by passing it to the
     * network stack. afterwards ack packet*/

    unint4 cpvalue = cpsw_stateram_regs->rx_cp[0];
    LOG(COMM, TRACE, "AM335xEthernet::handleIRQ(). Handling RX packet IRQ, cp at %x", cpvalue);

    for (int i = 0; i < 10; i++) {
        if (!(rxqueue[i].control & OWNER_MAC)) {
            /* queue is owned by host.. must be some data in it */
            int packet_len = rxqueue[i].control & 0x7ff;
            LOG(COMM, DEBUG, "AM335xEthernet::handleIRQ(). Handling RX len: %u", packet_len);
            /* copy data from dma buffer to internal packet buffer.
             * Takes time but allows HW to receive new packets inside this buffer. */
            comStackMutex->acquire();
            struct pbuf* ptBuf = pbuf_alloc(PBUF_RAW, (unint2) (packet_len + 10), PBUF_RAM);
            if (ptBuf != 0) {
                memcpy(ptBuf->payload, (void*) (rxqueue[i].buffer_pointer), packet_len);
                ethernet_input(ptBuf, &st_netif);
                pbuf_free(ptBuf);
            }
            comStackMutex->release();

            rxqueue[i].buffer_opt = 1520;
            rxqueue[i].control = OWNER_MAC;
            cpsw_dma_regs->rx0_freebuffer = 1;
            LOG(COMM, TRACE, "AM335xEthernet::handleIRQ(). ACK cp at %x", &rxqueue[i]);
            cpsw_stateram_regs->rx_cp[0] = (unint4) &rxqueue[i];
        }
    }
    /* ACK IRQ. */
    cpsw_stateram_regs->rx_cp[0] = cpvalue;

    OUTW(((unint4 )cpsw_dma_regs) + 0x94, 0x1);
    cpsw_stateram_regs->rx_hdp[0] = (unint4) &rxqueue[0];

    this->interruptPending = false;
    return (cOk );
}

