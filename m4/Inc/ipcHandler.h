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
* ipcHandler.h
* ============
* Generic Communication from the M4 CPU to one of the subsidiary M0s.
* Receivers are defined in terms of FreeRTOS buffers so we can just monitor a receive buffer to check when there's
* data available.

*/

#ifndef IPCHANDLER_H_
#define IPCHANDLER_H_

#include "config.h"
#include "FreeRTOS.h"
#include "queue.h"

struct buff

{
    uint32_t rp;
    uint32_t wp;
    uint32_t len;
    uint16_t flags;
    uint8_t *buffer;
};


#ifndef IAM_M4
typedef enum {SERPORT_M4, NUM_SERPORTS} serportEnumType;
#define SERPORT_MASK (1<<SERPORT_M4)
#define SERPORT_TX(x) (1<<(3*x))
#define SERPORT_RX(x) ((1<<((3*x)+1)))
#define SERPORT_OPEN(x) ((1<<((3*x)+2)))

typedef enum {  SERPORT_EV_NONE = 0,
                SERPORT_EV_CONNECT = ( 1 << 0 ),
                SERPORT_EV_CLOSE = ( 1 << 1 ),
                SERPORT_EV_DATARX = ( 1 << 2 ),
                SERPORT_EV_BREAKRX = ( 1 << 3 ),
                SERPORT_EV_CTRLCHANGE = ( 1 << 4 )
             } serportEventType;
#endif

struct ipcBuffer
{
    struct buff m40;            /* M4 to M0 buffer */
    struct buff m04;            /* M0 to M4 buffer */
};

enum ipc {IPC_APP, IPC_SUB, NUM_IPCS, IPC_M4 = 0}; /* The IPC_M4 is a dummy for the M0 -> M4 communication direction */

// ============================================================================================
void ipcFlush( enum ipc port );           ///< Flush the receive path
BOOL ipcOpenPort( enum ipc port );        ///< Open port
void ipcClosePort( enum ipc port );       ///< Close port and release resources
uint8_t ipcGetRx( enum ipc port );        ///< Get received data element
BOOL ipcDataPending( enum ipc port );     ///< Check to see if there is any data pending
BOOL ipcConnected( enum ipc port );       ///<IPC Connected Status
BOOL ipcTxRoomCheck( enum ipc port, uint32_t len );
void ipcAlertFarEnd( void );
BOOL ipcTx( enum ipc port, uint8_t *d, uint32_t len ); ///< Transmit through the port

void ipcInit( EVENT_CB( *cb_set ) );      ///< Create the communication subsystem
// ============================================================================================
#endif /* IPCHANDLER_H_ */
