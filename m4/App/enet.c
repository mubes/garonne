/*
*                                       ++++++++++++++++++
*                                  +++++++++++++++++++++++++++++
*                              +++++++                      +++++++++
*                          +++++++                               +++++++++++++
*         ++++++++++++++++++++                                         ++++++++++
*    +++++++++++++++++++++                                                     +++
*   +++++                                                                       +++
*  +++         ######### ######### ########  #########  #########   +++++++      ++
*  +++  +++++ ####  #### ######## ####  #### ##### #### #### ####  +++  ++++    +++
*  +++   ++++ ###     ## ###      ###    ### ###    ### ###    ### ++++++++   +++
*   ++++ ++++ ########## ###      ########## ###    ### ###    ### ++++    +++++
*    +++++++   ###### ## ###       ########  ###     ## ##     ###  ++++++++++
*
* Copyright 2017 Technolution BV  opensource@technolution.eu
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
* associated documentation files (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or substantial
* portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
* LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
* OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "config.h"
#include "lwip/init.h"
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/memp.h"
#include "lwip/tcpip.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"

#include "arch/lpc18xx_43xx_emac.h"
#include "arch/lpc_arch.h"
#include "arch/sys_arch.h"
#include "lpc_phy.h" /* For the PHY monitor support */

#ifdef INCLUDE_ETHERNET

static uint32_t _enetPins[] = {ENET_RESET, ENET_TXD0, ENET_TXD1, ENET_TXEN, ENET_RXD0, ENET_RXD1, ENET_RX_DV, ENET_MDIO, ENET_MDC, ENET_TXCLK, 0};

/* NETIF data */
static struct netif _netif;

// ============================================================================================
static void _tcpip_init_done_signal( void *arg )

/* Callback for TCPIP thread to indicate TCPIP init is done */

{
    /* Tell main thread TCP/IP init is done */
    *( s32_t * ) arg = 1;
}
// ============================================================================================
static void _setupIFTask( void *pvParameters )

/* LWIP kickoff and PHY link monitor thread */

{
    ip_addr_t ipaddr, netmask, gw;
    volatile s32_t tcpipdone = 0;
    uint32_t physts;

    uint32_t *pin = _enetPins;

    while ( *pin )
    {
        Chip_SCU_PinMuxSet( GETPORT( *pin ), GETPIN( *pin ), GETFUNC( *pin ) );
        pin++;
    }

    /* Take the MAC out of reset */
    Chip_GPIO_SetPinDIROutput( LPC_GPIO_PORT, GETGPIOPORT( ENET_RESET ), GETGPIOPIN( ENET_RESET ) );
    Chip_GPIO_SetPinState( LPC_GPIO_PORT, GETGPIOPORT( ENET_RESET ), GETGPIOPIN( ENET_RESET ), true );

#ifdef USE_RMII
    Chip_ENET_RMIIEnable( LPC_ETHERNET );
#else
#error "Not configured for MII!!"
#endif
    Chip_ENET_Reset( LPC_ETHERNET );
    vTaskDelay( 1 );

    /* Setup Ethernet clocks for MII. */
    Chip_Clock_SetBaseClock( CLK_BASE_PHY_TX, CLKIN_ENET_TX, true, false );
    Chip_Clock_SetBaseClock( CLK_BASE_PHY_RX, CLKIN_ENET_TX, true, false );
    Chip_ENET_Reset( LPC_ETHERNET );
    vTaskDelay( 1 );

    tcpip_init( _tcpip_init_done_signal, ( void * ) &tcpipdone );

    while ( !tcpipdone )
    {
        vTaskDelay( MILLIS_TO_TICKS( 10 ) );
    }

    /* Static IP assignment */
#if LWIP_DHCP
    IP4_ADDR( &gw, 0, 0, 0, 0 );
    IP4_ADDR( &ipaddr, 0, 0, 0, 0 );
    IP4_ADDR( &netmask, 0, 0, 0, 0 );
#else
    IP4_ADDR( &gw, 172, 26, 172, 254 );
    IP4_ADDR( &ipaddr, 172, 26, 172, 67 );
    IP4_ADDR( &netmask, 255, 255, 255, 0 );
#endif

    if ( !netif_add( &_netif, &ipaddr, &netmask, &gw, NULL, lpc_enetif_init,
                     tcpip_input ) )
    {
        LWIP_ASSERT( "Net interface failed to initialize\r\n", 0 );
    }

    _netif.hostname = PRODUCTNAME;
    netif_set_default( &_netif );
    netif_set_up( &_netif );

    /* Enable MAC interrupts only after LWIP is ready */
    NVIC_SetPriority( ETHERNET_IRQn, ENET_INTERRUPT_PRIORITY );
    NVIC_EnableIRQ( ETHERNET_IRQn );

#if LWIP_DHCP
    dhcp_start( &_netif );
#endif

    /* This loop monitors the PHY link and will handle cable events
       via the PHY driver. */
    while ( 1 )
    {
        /* Call the PHY status update state machine once in a while
           to keep the link status up-to-date */
        physts = lpcPHYStsPoll();

        /* Only check for connection state when the PHY status has changed */
        if ( physts & PHY_LINK_CHANGED )
        {
            if ( physts & PHY_LINK_CONNECTED )
            {

                /* Set interface speed and duplex */
                if ( physts & PHY_LINK_SPEED100 )
                {
                    Chip_ENET_SetSpeed( LPC_ETHERNET, 1 );
                }
                else
                {
                    Chip_ENET_SetSpeed( LPC_ETHERNET, 0 );
                }

                if ( physts & PHY_LINK_FULLDUPLX )
                {
                    Chip_ENET_SetDuplex( LPC_ETHERNET, true );
                }
                else
                {
                    Chip_ENET_SetDuplex( LPC_ETHERNET, false );
                }

                tcpip_callback_with_block( ( tcpip_callback_fn ) netif_set_link_up,
                                           ( void * ) &_netif, 1 );
            }
            else
            {
                tcpip_callback_with_block( ( tcpip_callback_fn ) netif_set_link_down,
                                           ( void * ) &_netif, 1 );
            }
        }

        /* Delay for link detection (250mS) */
        vTaskDelay( MILLIS_TO_TICKS( 250 ) );
    }
}
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Publicly available routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
uint32_t enetIPAddr( void )

/* Return IP Address */
  
{
    return _netif.ip_addr.addr;
}
// ============================================================================================
uint32_t enetLinkUp( void )

/* Return flag indicating link is up */
  
{
    return ( _netif.flags & NETIF_FLAG_LINK_UP ) != 0;
}
// ============================================================================================
void enetInit( void )

{
    xTaskCreate( _setupIFTask, "LinkCTL",
                 148, NULL, ( tskIDLE_PRIORITY + 1UL ),
                 ( xTaskHandle * ) NULL );
}
// ============================================================================================
#endif
