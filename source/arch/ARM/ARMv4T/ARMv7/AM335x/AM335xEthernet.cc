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
#include "assemblerFunctions.hh"

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
    volatile cppi_rx_descriptor_t*  next_descr;
    volatile char*                  buffer_pointer;
    volatile unint4                 buffer_opt;
    volatile unint4                 control;
} cppi_rx_descriptor_t;


/* some receive area to be used by the dma to store new packets*/
static char rx_buffers[10][1520] ATTR_CACHE_INHIBIT;
static char tx_buffers[10][1520] ATTR_CACHE_INHIBIT;

/* descriptors to be used by dma for all ports */
static cppi_tx_descriptor_t* txqueue;
static cppi_rx_descriptor_t* rxqueue;

static cppi_rx_descriptor_t* rxqueue_head;
static cppi_rx_descriptor_t* rxqueue_tail;

/* the mac address used for this device */
char mac[6] = { 0x00, 0x01, 0x01, 0x01, 0x01, 0x01 };
char broadcastmac[6] ={ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

extern "C" err_t ethernet_input(struct pbuf *p, struct netif *netif);

static err_t low_level_output(struct netif *netif, struct pbuf *p) {
    LOG(COMM, TRACE, "AM335xEthernet::sendPacket(). Send packet %x len: %u", p, p->tot_len);
    AM335xEthernet* ethdev = (AM335xEthernet*) netif->state;

    SMP_SPINLOCK_GET(lock);

    bool send = false;

    while (!send) {
        /* get a free descriptor and send the packet */
        for (int i = 0; i < 1; i++) {
            if (!(txqueue[i].control & OWNER_MAC)) {
                /* this one is free */

                //memset(tx_buffers[i], 0, 64);
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
    SMP_SPINLOCK_FREE(lock);

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

    mutex = new Mutex("AM335Ethernet");

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
    rxqueue_head = rxqueue;
    rxqueue_tail = &rxqueue[9];

    /* initialize tx queue */
    for (int i = 0; i < 1; i++) {
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
    cpsw_wr_regs->c0_rx_thresh_en = 0x0;

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
    OUTW(((unint4 )cpsw_p0_regs) + 0x124, 0x0001);

    //ale_addAddress((cpsw_ale_regs_t*) cpsw_ale_regs, ALE_UNICAST, mac, 0);
    //ale_addAddress((cpsw_ale_regs_t*) cpsw_ale_regs, 0, broadcastmac, 1 << 0);

    //PORT 1 + 2 are external. PORT 0 is the cpsw_dma
    struct ip_addr eth_nm   = IP_ADDR_INIT_IPV4(255,255,255,0);
    struct ip_addr tIpAddr  = IP_ADDR_INIT_IPV4(192,168,1,100);
    struct ip_addr tgwAddr  = IP_ADDR_INIT_IPV4(192,168,1,1);

    /* save driver in netif as state */
    netif_add(&st_netif, &tIpAddr, &eth_nm, &tgwAddr, (void*) this, &ethernetif_init, 0);
    netif_set_default(&st_netif);

    LOG(ARCH, INFO, "AM335xEthernet: RX descriptor queue at %x", (unint4 ) &rxqueue[0]);
    LOG(ARCH, INFO, "AM335xEthernet: TX descriptor queue at %x", (unint4 ) &txqueue[0]);

    rxpos = 0;
    cpsw_dma_regs->rx0_freebuffer = 10;
    /* enable tx and rx dma */
    cpsw_dma_regs->tx_control = 1;
    cpsw_dma_regs->rx_control = 1;

    /* set rx descriptor in hw register to the first one */
    cpsw_stateram_regs->rx_hdp[0] = (unint4) rxqueue_head;

    netif_set_up(&st_netif);
}

AM335xEthernet::~AM335xEthernet() {
}

ErrorT AM335xEthernet::disableIRQ() {
    theOS->getBoard()->getInterruptController()->maskIRQ(intc_irq);
    cpsw_wr_regs->c0_rx_en = 0x0;
    return (cOk );
}

ErrorT AM335xEthernet::enableIRQ() {
    theOS->getBoard()->getInterruptController()->unmaskIRQ(intc_irq);
    cpsw_wr_regs->c0_rx_en = 0xff;
    return (cOk );
}

ErrorT AM335xEthernet::clearIRQ() {
    theOS->getBoard()->getInterruptController()->maskIRQ(intc_irq);
    /* no real clear here as handleIRQ must be called for that */
    return (cOk );
}

// if we are sure a -b is never > b we can use this!
#define MODULO(a, b)  a >= b ? a-b : a

ErrorT AM335xEthernet::handleIRQ() {

    /* scan rx queue and process the data by passing it to the
     * network stack. afterwards ack packet*/

    lastIRQ = theOS->getClock()->getDateTime();
    unint4 cpvalue = cpsw_stateram_regs->rx_cp[0];
    LOG(COMM, DEBUG, "AM335xEthernet::handleIRQ(). Handling RX packet IRQ, cp at %x", cpvalue);
    int eoq = 0;

    int status = 0;
    cppi_rx_descriptor_t* descr = rxqueue_head;
    if (!descr)
        goto out;

    while (descr) {
        status = descr->control;
        if (status & OWNER_MAC) {
            // end here
            goto out;
        }

        int packet_len = status & 0x7ff;
        if (status & CRC_PASS)
            packet_len -= 4;

        rxqueue_head = (cppi_rx_descriptor_t*) descr->next_descr;
        if (status & EOQ) {
            cpsw_stateram_regs->rx_hdp[0] = (unint4) rxqueue_head;
        }

        LOG(COMM, DEBUG, "AM335xEthernet::handleIRQ(). Handling RX len: %u", packet_len);
         /* copy data from dma buffer to internal packet buffer.
          * Takes time but allows HW to receive new packets inside this buffer. */
         comStackMutex->acquire();
         struct pbuf* ptBuf = pbuf_alloc(PBUF_RAW, (unint2) (packet_len + 10), PBUF_RAM);
         if (ptBuf != 0) {
             memcpy(ptBuf->payload, (void*) (descr->buffer_pointer), packet_len);
             ethernet_input(ptBuf, &st_netif);
         }
         comStackMutex->release();

        cpsw_stateram_regs->rx_cp[0] = (unint4) descr;

        /* requeue descriptor */

        descr->next_descr = 0;
        descr->control    = OWNER_MAC | 1520 | SOP | EOP;
        descr->buffer_opt = 1520;
        rxqueue_tail->next_descr = descr;
        rxqueue_tail = descr;
        cpsw_dma_regs->rx0_freebuffer = 1;
        descr = rxqueue_head;
    }

out:

    OUTW(((unint4 )cpsw_dma_regs) + 0x94, 0x1);

    LOG(COMM, TRACE, "AM335xEthernet::handleIRQ(). exited");
    this->interruptPending = false;
    return (cOk );
}


