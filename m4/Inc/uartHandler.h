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
 * UART handler module
 * ===================
 *
 * This module handles the reception and transmission through the UART.
 */

#ifndef UARTHANDLER_H_
#define UARTHANDLER_H_

#include "config.h"
#include "FreeRTOS.h"
#include "queue.h"

#define TX_QUEUE_LEN  (128)
#define RX_QUEUE_LEN  (256)

typedef enum {UARTClosed, UARTOpen} uartStateEnum;
typedef enum {UART_SERPORT0, UART_SERPORT1, UART_SERPORT2, UART_SERPORT3, NUM_UARTS} uartEnumType;

// ============================================================================================
void uartInit(EVENT_CB(*cb_set));               ///< Create the communication subsystem

BOOL uartOpenPort(uartEnumType port, uint32_t baudrate,
                  uint32_t bitConfig); ///< Open port with specified baudrate
void uartClosePort(uartEnumType port);          ///< Close port and release resources
uint8_t uartGetRx(uartEnumType port);           ///< Get received data element
BOOL uartDataPending(uartEnumType port);        ///< Check to see if there is any data pending
BOOL uartConnected(uartEnumType port);          ///<UART Connected Status
BOOL uartFlush(uartEnumType port);          ///< Flush all data from tx and rx queues
void uartFlushKey(uartEnumType port);               ///< Flush all data from rx queue
BOOL uartForceTx(uartEnumType port, uint8_t *d, uint8_t len); ///< Force data out of tx port
uint32_t uartMultiput(uartEnumType port, uint32_t ticksToWait, uint8_t c,
                      uint32_t len); ///< Send repeats of specific character
uint32_t uartTx(uartEnumType port, uint8_t *d, uint8_t len);  ///< Transmit through the port
uint32_t uartTxt(uartEnumType port, uint32_t ticksToWait, uint8_t *d,
                 uint32_t len); ///< Transmit and wait if needed
// ============================================================================================
#endif /* UARTHANDLER_H_ */
