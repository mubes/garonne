#ifndef __LWIPOPTS_H_
#define __LWIPOPTS_H_

/* RTOS build */
#define NO_SYS                          0

/* Use LWIP timers */
#define NO_SYS_NO_TIMERS                0

/* Need for memory protection */
#define SYS_LIGHTWEIGHT_PROT            1

/* 32-bit alignment */
#define MEM_ALIGNMENT                   4

/* pbuf buffers in pool. In zero-copy mode, these buffers are
   located in peripheral RAM. In copied mode, they are located in
   internal IRAM */
#define PBUF_POOL_SIZE                  5

/* No padding needed */
#define ETH_PAD_SIZE                    0

#define IP_SOF_BROADCAST                1
#define IP_SOF_BROADCAST_RECV           1

/* The ethernet FCS is performed in hardware. The IP, TCP, and UDP
   CRCs still need to be done in hardware. */
#define CHECKSUM_GEN_IP                 1
#define CHECKSUM_GEN_UDP                1
#define CHECKSUM_GEN_TCP                1
#define CHECKSUM_CHECK_IP               1
#define CHECKSUM_CHECK_UDP              1
#define CHECKSUM_CHECK_TCP              1
#define LWIP_CHECKSUM_ON_COPY           1

/* Use LWIP version of htonx() to allow generic functionality across
   all platforms. If you are using the Cortex Mx devices, you might
   be able to use the Cortex __rev instruction instead. */
#define LWIP_PLATFORM_BYTESWAP          0
#define MEM_SIZE                        (12 * 1024)

/* Raw interface not needed */
#define LWIP_RAW                        0

/* DHCP is ok, UDP is required with DHCP */
#define LWIP_DHCP                       1
#define LWIP_UDP                        1

/* Hostname can be used */
#define LWIP_NETIF_HOSTNAME             1

#define LWIP_BROADCAST_PING             1

/* MSS should match the hardware packet size */
#define TCP_MSS                         1460
#define TCP_SND_BUF                     (2 * TCP_MSS)

#define LWIP_SOCKET                     0
#define LWIP_NETCONN                    1
#define MEMP_NUM_SYS_TIMEOUT            300

#define LWIP_STATS                      0
#define LINK_STATS                      0
#define LWIP_STATS_DISPLAY              0

#ifdef DEBUG
    #define LWIP_DEBUG                      1
#endif

#define LWIP_DEBUG 1

/* There are more *_DEBUG options that can be selected.
   See opts.h. Make sure that LWIP_DEBUG is defined when
   building the code to use debug. */
#define TCP_DEBUG                       LWIP_DBG_OFF
#define ETHARP_DEBUG                    LWIP_DBG_OFF
#define PBUF_DEBUG                      LWIP_DBG_OFF
#define IP_DEBUG                        LWIP_DBG_OFF
#define TCPIP_DEBUG                     LWIP_DBG_OFF
#define DHCP_DEBUG                      LWIP_DBG_OFF
#define UDP_DEBUG                       LWIP_DBG_OFF

/* This define is custom for the LPC EMAC driver. Enabled it to
   get debug messages for the driver. */
#define EMAC_DEBUG                    LWIP_DBG_OFF

#define DEFAULT_THREAD_PRIO             (tskIDLE_PRIORITY + 1)
#define DEFAULT_THREAD_STACKSIZE        (150)
#define DEFAULT_ACCEPTMBOX_SIZE         4
#define DEFAULT_TCP_RECVMBOX_SIZE       4
#define DEFAULT_UDP_RECVMBOX_SIZE       4

#define IRQ_PRIO_ETHERNET               config_ETHERNET_INTERRUPT_PRIORITY

/* TCPIP thread must run at higher priority than MAC threads! */
#define TCPIP_THREAD_PRIO               (DEFAULT_THREAD_PRIO + 2)

#define TCPIP_THREAD_NAME               "TCP Thr"
#define TCPIP_THREAD_STACKSIZE          (224)

#define TCPIP_MBOX_SIZE                 4

#define MEM_LIBC_MALLOC                 0
#define MEMP_MEM_MALLOC                 0

/* Required for malloc/free */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "lpc_types.h"
#include "FreeRTOS.h"

#endif /* __LWIPOPTS_H_ */




