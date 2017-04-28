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
 *
 * uart.c
 * ======
 * Generic UART implementation covering all available UART ports.
 * Receivers are defined in terms of FreeRTOS buffers so we can just monitor a receive buffer to check when there's
 * data available.

 */

#include <string.h>
#include "uartHandler.h"
#include "serport.h"
#include "config.h"
#include "FreeRTOS.h"
#include "queue.h"
// ============================================================================================
#define MAX_CLOSE_DELAY 1000  /* Max time in mS to wait for port to be flushed */
#define STANDARD_TIMEOUT 100 /* Max time in mS to wait for character space to be available */

/* Definitions for a ring buffer */
#define RB_RESET(x) x##rp=x##wp=0
#define RB_ELEMENTS(x,s) ((x##wp+s-x##rp)%s)
#define RB_FULL(x,s) (RB_ELEMENTS(x,s)==s-1)
#define RB_EMPTY(x) (x##rp==x##wp)
#define RB_PUTBYTE(x,s,y) if (!RB_FULL(x,s)) { x##b[x##wp=((x##wp+1)%s)]=(y); }
#define RB_GETBYTE(x,s) ((RB_EMPTY(x))?0:x##b[x##rp=((x##rp+1)%s)])


const struct /* Static information about a port */
{
    LPC_USART_T *Addr;
    IRQn_Type IRQ;
    uint32_t txPort;
    uint32_t rxPort;
    uint32_t txPin;
    uint32_t txPinMux;
    uint32_t rxPin;
    uint32_t rxPinMux;
} _uart[NUM_UARTS] =
{
    {
        // Port0
        .Addr = LPC_USART0,      .IRQ = USART0_IRQn,
        .txPort=GETPORT(UART0_TX), .txPin=GETPIN(UART0_TX), .txPinMux=GETFUNC(UART0_TX),
        .rxPort=GETPORT(UART0_RX), .rxPin=GETPIN(UART0_RX), .rxPinMux=GETFUNC(UART0_RX),
    },
    {
        // Port1
        .Addr = LPC_UART1, .IRQ = UART1_IRQn,
        .txPort=GETPORT(UART1_TX), .txPin=GETPIN(UART1_TX), .txPinMux=GETFUNC(UART1_TX),
        .rxPort=GETPORT(UART1_RX), .rxPin=GETPIN(UART1_RX), .rxPinMux=GETFUNC(UART1_RX),
    },
    {
        // Port2
        .Addr = LPC_USART2, .IRQ = USART2_IRQn,
        .txPort=GETPORT(UART2_TX), .txPin=GETPIN(UART2_TX), .txPinMux=GETFUNC(UART2_TX),
        .rxPort=GETPORT(UART2_RX), .rxPin=GETPIN(UART2_RX), .rxPinMux=GETFUNC(UART2_RX),
    },
    {
        // Port 3
        .Addr = LPC_USART3, .IRQ = USART3_IRQn,
        .txPort=GETPORT(UART3_TX), .txPin=GETPIN(UART3_TX), .txPinMux=GETFUNC(UART3_TX),
        .rxPort=GETPORT(UART3_RX), .rxPin=GETPIN(UART3_RX), .rxPinMux=GETFUNC(UART3_RX),
    }
};

/* We only need a single serial callback for all ports */
static EVENT_CB(*_serial_cb);

static volatile struct /* Dynamic state of a port */
{
    uint8_t rxb[RX_QUEUE_LEN];
    uint32_t rxwp,rxrp;
    uint8_t txb[TX_QUEUE_LEN];
    uint32_t txrp,txwp;

    SemaphoreHandle_t isTxSpace;

    uartStateEnum state;
} _state[NUM_UARTS];
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Internal Routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
void _genericIRQHandler(uartEnumType port)

{
    uint32_t j;
    uint32_t stat;
    BOOL wasDataPreviously;

    portBASE_TYPE xTaskWoken = pdFALSE;
    stat = Chip_UART_ReadIntIDReg(_uart[port].Addr);
    while (!(stat&1))
        {
            if (stat & UART_IIR_INTID_RDA)
                {
                    /* Data has arrived, handle it */
                    j = Chip_UART_ReadByte(_uart[port].Addr);
                    wasDataPreviously=!RB_EMPTY(_state[port].rx);

                    RB_PUTBYTE(_state[port].rx,RX_QUEUE_LEN,j);
                    if (!wasDataPreviously)
                        CALLBACK(_serial_cb,(SERPORT_EV_DATARX|(port<<8)));
                }
            else
                {
                    if (stat & UART_IIR_INTID_THRE)
                        {
                            if (RB_EMPTY(_state[port].tx))
                                Chip_UART_IntDisable(_uart[port].Addr,UART_IER_THREINT);
                            else
                                {
                                    while (((Chip_UART_ReadLineStatus(_uart[port].Addr) & UART_LSR_THRE))
                                            && (!RB_EMPTY(_state[port].tx)))
                                        {
                                            j=RB_GETBYTE(_state[port].tx,TX_QUEUE_LEN);
                                            Chip_UART_SendByte(_uart[port].Addr,j);
                                        }
                                    xSemaphoreGiveFromISR(_state[port].isTxSpace,&xTaskWoken);
                                }
                        }
                    else
                        {
                            ASSERT(FALSE);
                        }
                }

            stat = Chip_UART_ReadIntIDReg(_uart[port].Addr);
        }
    portEND_SWITCHING_ISR(xTaskWoken);
}
// ============================================================================================
void UART0_IRQHandler(void)

{
    _genericIRQHandler(UART_SERPORT0);
}
// ============================================================================================
void UART1_IRQHandler(void)

{
    _genericIRQHandler(UART_SERPORT1);
}
// ============================================================================================
void UART2_IRQHandler(void)

{
    _genericIRQHandler(UART_SERPORT2);
}
// ============================================================================================
void UART3_IRQHandler(void)

{
    _genericIRQHandler(UART_SERPORT3);
}
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Externally Available Routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
uint32_t uartTxt(uartEnumType port, uint32_t ticksToWait, uint8_t *d, uint32_t len)

/* Transmit and wait if needed */

{
    uint32_t attemptedLen=len;

    ASSERT(port<NUM_UARTS);

    if (_state[port].state != UARTOpen)
        return 0;

    NVIC_DisableIRQ(_uart[port].IRQ);
    /* If the port currently has nothing in the queue then we can queue directly to the THR */
    if (RB_EMPTY(_state[port].tx))
        while ((len) && (Chip_UART_ReadLineStatus(_uart[port].Addr) & UART_LSR_THRE))
            {
                Chip_UART_SendByte(_uart[port].Addr,*d);
                d++;
                len--;
            }

    if (len)
        {
            /* Now queue up as much as possible without delay */
            while ((len) && (!RB_FULL(_state[port].tx,TX_QUEUE_LEN)))
                {
                    RB_PUTBYTE(_state[port].tx,TX_QUEUE_LEN,*d++);
                    len--;
                }

            /* Set a flag to generate a transmit interrupt */
            Chip_UART_IntEnable(_uart[port].Addr, UART_IER_THREINT);
            NVIC_EnableIRQ(_uart[port].IRQ);

            /* Now queue up the remainder _with_ a delay */
            while ((len) && (xSemaphoreTake(_state[port].isTxSpace,ticksToWait)))
                {
                    while ((len) && ((!RB_FULL(_state[port].tx,TX_QUEUE_LEN))))
                        {
                            RB_PUTBYTE(_state[port].tx,TX_QUEUE_LEN,*d++);
                            len--;
                        }
                }
        }
    else
        NVIC_EnableIRQ(_uart[port].IRQ);

    return (attemptedLen-len);
}
// ============================================================================================
BOOL uartForceTx(uartEnumType port, uint8_t *d, uint8_t len)

{
    ASSERT(port<NUM_UARTS);

    if (_state[port].state != UARTOpen)
        return FALSE;

    NVIC_DisableIRQ(_uart[port].IRQ);

    /* Unceremoniously queue directly to the THR */
    while (len)
        {
            /* Spin waiting for space */
            while (!(Chip_UART_ReadLineStatus(_uart[port].Addr) & UART_LSR_THRE));

            Chip_UART_SendByte(_uart[port].Addr,*d);

            if (*d=='\n')
                {
                    while (!(Chip_UART_ReadLineStatus(_uart[port].Addr) & UART_LSR_THRE));
                    Chip_UART_SendByte(_uart[port].Addr,'\r');
                }
            d++;
            len--;
        }
    NVIC_EnableIRQ(_uart[port].IRQ);
    return TRUE;
}
// ============================================================================================
uint32_t uartTx(uartEnumType port, uint8_t *d, uint8_t len)

/* Transmit without having to worry about the timeout - that's done for you */

{
    return (uartTxt(port, MILLIS_TO_TICKS(STANDARD_TIMEOUT), d, len));
}
// ============================================================================================
uint32_t uartMultiput(uartEnumType port, uint32_t ticksToWait, uint8_t c, uint32_t len)

/* Send repeats of specific character through the UART, waiting if needed for ticksToWait */
/* to give it all time to leave */

{
    uint32_t attemptedLen=len;

    ASSERT(port<NUM_UARTS);

    if (_state[port].state != UARTOpen)
        return 0;

    NVIC_DisableIRQ(_uart[port].IRQ);
    /* If the port currently has nothing in the queue then we can queue directly to the THR */
    if (RB_EMPTY(_state[port].tx))
        while ((len) && (Chip_UART_ReadLineStatus(_uart[port].Addr) & UART_LSR_THRE))
            {
                Chip_UART_SendByte(_uart[port].Addr,c);
                len--;
            }

    if (len)
        {
            /* Now queue up as much as possible without delay */
            while ((len) && (!RB_FULL(_state[port].tx,TX_QUEUE_LEN)))
                {
                    RB_PUTBYTE(_state[port].tx,TX_QUEUE_LEN,c);
                    len--;
                }

            /* Set a flag to generate a transmit interrupt */
            Chip_UART_IntEnable(_uart[port].Addr, UART_IER_THREINT);
            NVIC_EnableIRQ(_uart[port].IRQ);
            /* Now queue up the remainder _with_ a delay */
            while ((len) && (xSemaphoreTake(_state[port].isTxSpace,TICKS_TO_MILLIS(STANDARD_TIMEOUT))))
                {
                    while ((len) && (!RB_FULL(_state[port].tx,TX_QUEUE_LEN)))
                        {
                            RB_PUTBYTE(_state[port].tx,TX_QUEUE_LEN,c);
                            len--;
                        }
                }
        }
    else
        NVIC_EnableIRQ(_uart[port].IRQ);
    return (attemptedLen-len);
}
// ============================================================================================
BOOL uartOpenPort(uartEnumType port, uint32_t baudrate, uint32_t bitConfig)

/* Open specified port */

{
    ASSERT(port < NUM_UARTS);

    NVIC_DisableIRQ(_uart[port].IRQ);
    Chip_SCU_PinMuxSet(_uart[port].txPort,_uart[port].txPin,_uart[port].txPinMux);
    Chip_SCU_PinMuxSet(_uart[port].rxPort,_uart[port].rxPin,_uart[port].rxPinMux);
    Chip_UART_Init(_uart[port].Addr);
    Chip_UART_SetBaud(_uart[port].Addr,baudrate);
    Chip_UART_ConfigData(_uart[port].Addr, bitConfig);
    Chip_UART_SetupFIFOS(_uart[port].Addr, (UART_FCR_FIFO_EN | UART_FCR_RX_RS |
                                            UART_FCR_TX_RS | UART_FCR_TRG_LEV0));

    /* Enable USART */
    _state[port].state = UARTOpen;

    Chip_UART_IntEnable(_uart[port].Addr,UART_IER_RBRINT);
    NVIC_SetPriority(_uart[port].IRQ, UART_INTPRIORITY);
    NVIC_EnableIRQ(_uart[port].IRQ);
    Chip_UART_TXEnable(_uart[port].Addr);
    return TRUE;
}
// ============================================================================================
uint8_t uartGetRx(uartEnumType port)

/* Get rx data for this port */

{
    ASSERT(port < NUM_UARTS);
    return RB_GETBYTE(_state[port].rx,RX_QUEUE_LEN);
}
// ============================================================================================
BOOL uartConnected(uartEnumType port)

/* Get connected status for this port */

{
    ASSERT(port < NUM_UARTS);
    return _state[port].state==UARTOpen;
}
// ============================================================================================
BOOL uartDataPending(uartEnumType port)

{
    return !(RB_EMPTY(_state[port].rx));
}
// ============================================================================================
void uartClosePort(uartEnumType port)

/* Close specified port */

{
    ASSERT(port < NUM_UARTS);

    /* Deal with special case of UART1 - we don't use these facilities, but just in case we ever do... */
    if (_uart[port].Addr!=LPC_UART1)
        Chip_UART_IntDisable(_uart[port].Addr,UART_IER_BITMASK);
    else
        Chip_UART_IntDisable(_uart[port].Addr,UART2_IER_BITMASK);

    Chip_UART_DeInit(_uart[port].Addr);
    NVIC_DisableIRQ(_uart[port].IRQ);
    _state[port].state = UARTClosed;
}
// ============================================================================================
void uartFlushKey(uartEnumType port)

/* Flush all data from rx queue */

{
    RB_RESET(_state[port].rx);
}
// ============================================================================================
BOOL uartFlush(uartEnumType port)

{
    ASSERT(port < NUM_UARTS);
    int32_t delayRemaining=MILLIS_TO_TICKS(MAX_CLOSE_DELAY);

    while ((delayRemaining>0) && (!RB_EMPTY(_state[port].tx)))
        {
            delayRemaining-=MILLIS_TO_TICKS(10);
            vTaskDelay(MILLIS_TO_TICKS(10));
        }

    uartFlushKey(port);
    return (delayRemaining);
}
// ============================================================================================
void uartInit(EVENT_CB(*cb_set))

/* Loop through all ports and set them up/clear then */

{
    _serial_cb=cb_set;

    for (uartEnumType u = UART_SERPORT0; u<NUM_UARTS; u++)
        {
            _state[u].isTxSpace=xSemaphoreCreateBinary();
            uartClosePort(u);
        }
}
// ============================================================================================
