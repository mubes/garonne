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
 * Serial Port Abstraction
 * =======================
 *
 * A simple serial port abstraction mechanism. Primarily intended for USB and 'conventional' serial ports,
 * but applicable to just about anything, simply by providing the appropriate underlying drivers and callbacks.
 * Based on some of Daves original code, passed to Technolution with no restriction on use.
 *
 */



#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "config.h"
#include "event_groups.h"
#include "serport.h"
#include "uartHandler.h"
#ifdef INCLUDE_USB
#include "cdc_vcom.h"
#endif
#include "terminal.h"
#include "mainloop.h"
#include "lms.h"

#define SEM_BLOCK_TIME (1000) /* Time in milliseconds to wait for transmission to become possible */

/* Stats for RX and TX, used in the GUI to show when serial activity has occurred */
static serportStatsetType _stat;

/* Tracking structure for a serial port of some form */
static struct

{
    uint32_t e;
    SemaphoreHandle_t lock;
} _state[NUM_SERPORTS];
// ============================================================================================
#ifdef INCLUDE_USB
EVENT_CB(_usb_cb)

/* This is the callback from the CDC (USB) serial port when something interesting has happened.
 * This routine may be called from interrupt level.
 */

{
    BaseType_t xHigherPriorityTaskWoken;
    _state[SERPORT_USB].e|=j&SERPORT_EVENT_MASK;

    xHigherPriorityTaskWoken=MLUpdateAvailableFromISR(EVENT_USB0);

    _stat|=SERPORT_RX(SERPORT_USB);
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}
#endif
// ============================================================================================
EVENT_CB(_serial_cb)

/* This is the callback from the Serial stack when something interesting has happened.
 * This routine may be called from interrupt level.
 */

{
    BaseType_t xHigherPriorityTaskWoken;
    _state[(j>>8)&0xFF].e|=j&SERPORT_EVENT_MASK;
    xHigherPriorityTaskWoken=MLUpdateAvailableFromISR((j>>8)&SERPORT_MASK);

    _stat|=SERPORT_RX((j>>8)&0xFF);
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}
// ============================================================================================
// ============================================================================================
// ============================================================================================
// External routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
uint32_t serportTxt(serportEnumType serport, uint32_t ticksToWait, uint8_t *d, uint32_t len)

/* Send bytes to serial port with specified timeout */

{
    uint32_t retval=0;

    if (!SERPORT_ISVALID(serport))
        return retval;

    xSemaphoreTake(_state[serport].lock,MILLIS_TO_TICKS(SEM_BLOCK_TIME));

    if (SERPORT_ISUART(serport))
        {
            _stat|=SERPORT_TX(serport);
            retval=uartTxt(serport, ticksToWait, d, len);
        }

#ifdef INCLUDE_USB
    if (SERPORT_ISUSB(serport))
        {
            _stat|=SERPORT_TX(serport);

            uint32_t timeNow=xTaskGetTickCount();
            while ((retval<len) && serportIsOpen(serport) && (timeNow-xTaskGetTickCount()<ticksToWait))
                {
                    retval+=vcom_write (&d[retval], len-retval);
                }
        }
#endif
    xSemaphoreGive(_state[serport].lock);
    return retval;
}
// ============================================================================================
uint32_t serportMultiput(serportEnumType serport, uint32_t ticksToWait, uint8_t c, uint32_t len)

/* Send single byte multiple times to port */

{
    uint32_t retval=0;
    BOOL semRet;

    if (!SERPORT_ISVALID(serport))
        return retval;

    semRet=xSemaphoreTake(_state[serport].lock,MILLIS_TO_TICKS(SEM_BLOCK_TIME));
    (void)semRet; /* Avoid unused variable in production builds */
    ASSERT(semRet);

    if (serportIsOpen(serport))
        {
            _stat|=SERPORT_TX(serport);
            if (SERPORT_ISUART(serport))
                {
                    retval=uartMultiput(serport-SERPORT_UART0, ticksToWait, c, len);
                }

#ifdef INCLUDE_USB
            if (SERPORT_ISUSB(serport))
                {
                    ASSERT(len<TERM_COLS);
                    while ((len) && serportIsOpen(serport))
                        len-=vcom_write (&c, 1);
                }
#endif
        }

    xSemaphoreGive(_state[serport].lock);
    return retval;
}
// ============================================================================================
uint32_t serportPutChar(serportEnumType serport, uint8_t t)

/* Send single character to serial port - with no timeout */

{
    return (serportTxt(serport,SERIAL_TX_WAIT,&t,1));
}
// ============================================================================================
uint32_t serportTx(serportEnumType serport, uint8_t *d, uint32_t len)

/* Send sequence of bytes to serial port with pre-defined timeout */

{
    return (serportTxt(serport, SERIAL_TX_WAIT, d, len));
}
// ============================================================================================
uint32_t serportPrintf(serportEnumType serport, char *fmt, ...)

/* Perform printf to serial port */

{
    static char op[PRINTF_MAXLEN];

    va_list va;
    va_start(va, fmt);
    vsiprintf(op, fmt, va);
    va_end(va);
    ASSERT(strlen(op)<=PRINTF_MAXLEN);
    return serportTx(serport, (uint8_t *) op, strlen(op));
}
// ============================================================================================
uint8_t serportGetRx(serportEnumType serport)

/* Get received data element */

{
    if (SERPORT_ISUART(serport))
        return uartGetRx(serport);

#ifdef INCLUDE_USB
    return vcomGetRx();
#else
    return 0;
#endif
}
// ============================================================================================
BOOL serportDataPending(serportEnumType serport)

/* Check to see if there is any data pending */

{
    if (SERPORT_ISUART(serport))
        return uartDataPending(serport);

#ifdef INCLUDE_USB
    if (SERPORT_ISUSB(serport))
        return (vcomDataPending());
#endif

    return FALSE;
}
// ============================================================================================
BOOL serportOpenPort(serportEnumType serport, uint32_t baudrate, uint32_t bitConfig)

/* Open the specified port */

{
    if (SERPORT_ISUART(serport))
        {
            if (uartOpenPort(serport, baudrate,bitConfig))
                {
                    _serial_cb(SERPORT_EV_CONNECT|(serport<<8));
                    return TRUE;
                }
            return FALSE;
        }

    /* There is nothing to do for USB serial open - it's always there */
    if (SERPORT_ISUSB(serport))
    {
        _serial_cb(SERPORT_EV_CONNECT|(serport<<8));
        return (TRUE);
    }

    return (FALSE);
}
// ============================================================================================
void serportClosePort(serportEnumType serport)

/* Close the specified port */

{
    if (SERPORT_ISUART(serport))
        {
            uartClosePort(serport);
            _serial_cb(SERPORT_EV_CLOSE|(serport<<8));
        }
}
// ============================================================================================
BOOL serportFlush(serportEnumType serport)

/* Perform flush on specified port */

{
    if (SERPORT_ISUART(serport))
        {
            uartFlush(serport);
            return TRUE;
        }

#ifdef INCLUDE_USB
    if (SERPORT_ISUSB(serport))
        {
            vcom_flush();
            return TRUE;
        }
#endif
    return FALSE;
}

// ============================================================================================
BOOL serportIsOpen(serportEnumType serport)

/* Check if specified port is open */

{
    if (SERPORT_ISUART(serport))
        {
            return uartConnected(serport);
        }

#ifdef INCLUDE_USB
    return vcom_connected();
#endif

    return FALSE;
}
// ============================================================================================
BOOL serportFlushKey(serportEnumType serport)

{
    if (SERPORT_ISUART(serport))
        {
            uartFlushKey(serport);
            return TRUE;
        }

    return FALSE;
}
// ============================================================================================
uint32_t serportGetEvent(serportEnumType serport)

{
    __disable_irq();
    uint32_t r=_state[serport].e;
    _state[serport].e=0;
    __enable_irq();

    return r;
}
// ============================================================================================
serportStatsetType serportgetStatset(void)

/* Get the current statset and reset to zero - this is used to tell which ports have been active */

{
    serportStatsetType s=_stat;
    _stat=0;
    return s;
}
// ============================================================================================
void serportInit(void)

/* Initialise the serial port subsystem */

{
    for (serportEnumType i=SERPORT_UART0; i<NUM_SERPORTS; i++)
        {
            _state[i].lock=xSemaphoreCreateMutex();
            xSemaphoreGive(_state[i].lock);
        }

    /* Set up the callbacks to deliver results to us */
    uartInit(_serial_cb);
#ifdef INCLUDE_USB
    USBInit(_usb_cb);
#endif

}
// ============================================================================================
