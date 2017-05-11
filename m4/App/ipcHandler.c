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
 * ipcHandler.c
 * ============
 * Generic Communication from the M4 CPU to one of the subsidiary M0s.
 * Receivers are defined in terms of FreeRTOS buffers so we can just monitor a receive buffer to check when there's
 * data available.

 */

#include <string.h>
#include "ipcHandler.h"
#include "serport.h"
#include "config.h"
#include "FreeRTOS.h"
#include "queue.h"

// ============================================================================================
#define MAX_CLOSE_DELAY 1000  /* Max time in mS to wait for port to be flushed */
#define STANDARD_TIMEOUT 100 /* Max time in mS to wait for character space to be available */

/* We only need a single serial callback for all ports */
static EVENT_CB(*_serial_cb);

#ifdef IAM_M4
static volatile struct /* Dynamic state of a port */
{
    struct ipcBuffer *buff;
} _state[NUM_IPCS];
#else
/* The M0s can only see the M4 */

static volatile struct /* Dynamic state of a port */
{
    struct ipcBuffer *buff;
} _state;
#endif

// ============================================================================================
// ============================================================================================
// ============================================================================================
// Internal Routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
void M0CORE_IRQHandler(void)

/* Interrupt happens either because the last data has been consumed by the recipient, or there
 * is data for us. We cannot tell which, so need to look at the buffers to check.
 */
{
    portBASE_TYPE xTaskWoken = pdFALSE;

#ifdef IAM_M4
    /* Reception check first */
    if (_state[IPC_APP].buff->m04.rp!=_state[IPC_APP].buff->m04.wp)
    {
        CALLBACK(_serial_cb,(SERPORT_EV_DATARX|(SERPORT_M0APP<<8)));
    }

    if (_state[IPC_SUB].buff->m04.rp!=_state[IPC_SUB].buff->m04.wp)
    {
        CALLBACK(_serial_cb,(SERPORT_EV_DATARX|(SERPORT_M0SUB<<8)));
    }

    /* ....and now the transmission side */
    if (_state[IPC_APP].buff->m40.rp!=_state[IPC_APP].buff->m40.wp)
    {
        xSemaphoreGiveFromISR(_state[IPC_APP].buff->TxEmpty,&xTaskWoken);
    }

    if (_state[IPC_SUB].buff->m40.rp==_state[IPC_SUB].buff->m40.wp)
    {
        xSemaphoreGiveFromISR(_state[IPC_APP].buff->TxEmpty,&xTaskWoken);
    }
#else
    /* Reception check */
    if (_state.buff->m40.rp!=_state.buff->m40.wp)
    {
        CALLBACK(_serial_cb,(SERPORT_EV_DATARX|(SERPORT_M4<<8)));
    }

    /* ... and transmission */
    if (_state.buff->m04.rp==_state.buff->m04.wp)
    {
        xSemaphoreGiveFromISR(_state.buff->TxEmpty,&xTaskWoken);
    }
#endif

    portEND_SWITCHING_ISR(xTaskWoken);
}
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Externally Available Routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
uint32_t ipcTxt(enum ipc port, uint32_t ticksToWait, uint8_t *d, uint32_t len)

/* Transmit and wait if needed */

{
    uint32_t attemptedLen=len;
    struct buff *b;

    ASSERT(port<NUM_IPCS);

#ifdef IAM_M4
    b=&(_state[port].buff->m40);
#else
    ASSERT(port==IPC_M4);
    b=&(_state.buff->m04);
#endif

    /* If the port currently has space in the queue then we can queue directly */

    while (len)
    {
        uint32_t n=((b->wp)+1)%b->len; /* This is the next space */
        if (n==b->rp)
        {
            /* We are full, stop queuing */
            break;
        }

        /* There is space - write this and move along */
        b->buffer[b->wp]=*d++;
        len--;
        b->wp=n;
    }

    /* Well, certainly something has been sent, so trigger the other core */
    __DSB();
    __SEV();

    if (len)
    {
        /* Now queue up the remainder _with_ a delay */
#ifdef IAM_M4
        while ((len) && (xSemaphoreTake(_state[port].buff->TxEmpty,ticksToWait)))
#else
         while ((len) && (xSemaphoreTake(_state.buff->TxEmpty,ticksToWait)))
#endif
        {
            while (len)
            {
                uint32_t n=((b->wp)+1)%b->len; /* This is the next space */
                if (n==b->rp)
                {
                    /* We are full, stop queuing */
                    break;
                }

                /* There is space - write this and move along */
                b->buffer[b->wp]=*d++;
                len--;
                b->wp=n;
            }
        }
    }
    return (attemptedLen-len);
}
// ============================================================================================
uint32_t ipcTx(enum ipc port, uint8_t *d, uint8_t len)

/* Transmit without having to worry about the timeout - that's done for you */

{
    return (ipcTxt(port, MILLIS_TO_TICKS(STANDARD_TIMEOUT), d, len));
}
// ============================================================================================
BOOL ipcOpenPort(enum ipc port)

/* Open specified port */

{
    ASSERT(port < NUM_IPCS);

    return TRUE;
}
// ============================================================================================
uint8_t ipcGetRx(enum ipc port)

/* Get rx data for this port */

{
    ASSERT(port < NUM_IPCS);

    if (!uartDataPending(port))
    {
        return  0;
    }

#ifdef IAM_M4
    return _state[port].buff->m04.buffer[(_state[port].buff->m04.rp=((_state[port].buff->m04.rp)+1)%_state[port].buff->m04.len)];
#else
    ASSERT(port==IPC_M4);
    return _state.buff->m40.buffer[(_state.buff->m40.rp=((_state.buff->m40.rp)+1)%_state.buff->m40.len)];
#endif
    return 0;
}
// ============================================================================================
BOOL ipcConnected(enum ipc  port)

/* Get connected status for this port */

{
    ASSERT(port < NUM_IPCS);
    return TRUE;
}
// ============================================================================================
BOOL ipcDataPending(enum ipc port)

/* Indicate if there is RX path data waiting */

{
#ifdef IAM_M4
    return (_state[port].buff->m04.rp!=_state[port].buff->m04.wp);
#else
    ASSERT(port==IPC_M4);
    return (_state.buff->m40.rp!=_state.buff->m40.wp);
#endif

    return 0;
}
// ============================================================================================
void ipcClosePort(enum ipc port)

/* Close specified port */

{
    ASSERT(port < NUM_IPCS);
}
// ============================================================================================
void ipcFlush(enum ipc port)

/* Flush our receive buffer */

{
    ASSERT(port < NUM_IPCS);

#ifdef IAM_M4
    _state[port].buff->m40.rp=_state[port].buff->m40.wp;
#else
    ASSERT(port==IPC_M4);
    _state.buff->m04.rp=_state.buff->m04.wp;
#endif
}
// ============================================================================================
void ipcInit(EVENT_CB(*cb_set))

/* Loop through all ipc interfaces and set them up/clears them */

{
    _serial_cb=cb_set;

#ifdef IAM_M4
    /* Set the locations where these buffers will be held */
    _state[IPC_APP].buff = (struct ipcBuffer *)ADDR_IPCAPP_BUFFER;
    bzero(_state[IPC_APP].buff,sizeof(struct ipcBuffer));
    _state[IPC_APP].buff->m40.buffer=(uint8_t *)(ADDR_IPCAPP_BUFFER+sizeof(struct ipcBuffer));
    _state[IPC_APP].buff->m04.buffer=(uint8_t *)(ADDR_IPCAPP_BUFFER+IPC_QUEUE_LEN+sizeof(struct ipcBuffer));
    _state[IPC_APP].buff->m40.len=IPC_QUEUE_LEN;
    _state[IPC_APP].buff->m04.len=IPC_QUEUE_LEN;

    _state[IPC_SUB].buff = (struct ipcBuffer *)ADDR_IPCSUB_BUFFER;
    bzero(_state[IPC_SUB].buff,sizeof(struct ipcBuffer));
    _state[IPC_SUB].buff->m40.buffer=(uint8_t *)(ADDR_IPCSUB_BUFFER+sizeof(struct ipcBuffer));
    _state[IPC_SUB].buff->m04.buffer=(uint8_t *)(ADDR_IPCSUB_BUFFER+IPC_QUEUE_LEN+sizeof(struct ipcBuffer));
    _state[IPC_SUB].buff->m40.len=IPC_QUEUE_LEN;
    _state[IPC_SUB].buff->m04.len=IPC_QUEUE_LEN;
#else
    /* Set the locations where these buffers will be held */
    _state.buff = (struct ipcBuffer *)ADDR_IPCAPP_BUFFER;
    bzero(_state.buff,sizeof(struct ipcBuffer));
    _state.buff->m40.buffer=(uint8_t *)(ADDR_IPCAPP_BUFFER+sizeof(struct ipcBuffer));
    _state.buff->m04.buffer=(uint8_t *)(ADDR_IPCAPP_BUFFER+IPC_QUEUE_LEN+sizeof(struct ipcBuffer));
    _state.buff->m40.len=IPC_QUEUE_LEN;
    _state.buff->m04.len=IPC_QUEUE_LEN;
#endif
}
// ============================================================================================
