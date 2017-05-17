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
 *
 */


#ifndef _SERPORT_H_
#define _SERPORT_H_

#include "config.h"
#include "uartHandler.h"
#include "FreeRTOS.h"
#include "event_groups.h"

#define PRINTF_MAXLEN  (120)

/* Time we wait to perform a transmission over serial (i.e. waiting for space ... used for slow links) */
#define SERIAL_TX_WAIT MILLIS_TO_TICKS(40)

#define SERPORT_DTRON (1<<0)
#define SERPORT_RTSON (1<<1)

typedef enum {  SERPORT_EV_NONE=0,
                SERPORT_EV_CONNECT=(1<<0),
                SERPORT_EV_CLOSE=(1<<1),
                SERPORT_EV_DATARX=(1<<2),
                SERPORT_EV_BREAKRX=(1<<3),
                SERPORT_EV_CTRLCHANGE=(1<<4)
             } serportEventType;

#define SERPORT_EVENT_MASK (SERPORT_EV_CONNECT|SERPORT_EV_CLOSE|SERPORT_EV_DATARX|SERPORT_EV_BREAKRX|SERPORT_EV_CTRLCHANGE)

typedef enum {SERPORT_UART0, SERPORT_UART1, SERPORT_UART2, SERPORT_UART3, SERPORT_M0APP, SERPORT_M0SUB, SERPORT_USB, NUM_SERPORTS} serportEnumType;
#define SERPORT_MASK ((1<<SERPORT_UART0)|(1<<SERPORT_UART1)|(1<<SERPORT_UART2)|(1<<SERPORT_UART3)|(1<<SERPORT_M0APP)|(1<<SERPORT_M0SUB)|(1<<SERPORT_USB))
#define SERPORT_PORTNAMES "s0","s1","s2","s3","0a", "0s", "sU"


#define SERPORT_TX(x) (1<<(3*x))
#define SERPORT_RX(x) ((1<<((3*x)+1)))
#define SERPORT_OPEN(x) ((1<<((3*x)+2)))

/* Defines to classify ports */
#define SERPORT_ISUART(x) ((x>=SERPORT_UART0) && (x<SERPORT_M0APP))
#define SERPORT_ISUSB(x)  (x==SERPORT_USB)
#define SERPORT_ISIPC(x)  ((x==SERPORT_M0APP) || (x==SERPORT_M0SUB))
#define SERPORT_GETIPC(x)  (x-SERPORT_M0APP)
#define SERPORT_ISVALID(x) (x<NUM_SERPORTS)

typedef uint32_t serportStatsetType;

// ============================================================================================
uint32_t serportTxt(serportEnumType serport, uint32_t ticksToWait, uint8_t *d, uint32_t len);
uint32_t serportPutChar(serportEnumType serport, uint8_t t);
uint32_t serportMultiput(serportEnumType serport, uint32_t ticksToWait, uint8_t c, uint32_t len);
uint32_t serportTx(serportEnumType serport, uint8_t *d, uint32_t len);
uint32_t serportPrintf(serportEnumType serport, char *fmt, ...);
BOOL serportOpenPort(serportEnumType serport, uint32_t baudrate, uint32_t bitConfig);
BOOL serportIsOpen(serportEnumType serport);
EventBits_t serportWaitforEvent(serportEnumType serport, EventBits_t ev, uint32_t timeout);
BOOL serportGets(serportEnumType serport, char *store, uint32_t len);
void serportClosePort(serportEnumType serport);
BOOL serportFlush(serportEnumType serport);
BOOL serportFlushKey(serportEnumType serport);
serportStatsetType serportgetStatset(void);

uint8_t serportGetRx(serportEnumType serport);
BOOL serportDataPending(serportEnumType serport);
uint32_t serportGetEvent(serportEnumType serport);

void serportInit(void);
// ============================================================================================
#endif
