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
#ifdef IAM_M4
    #include "serport.h"
#endif
#include "config.h"
#include "FreeRTOS.h"
#include "queue.h"

/* For the uplink direction, set the index to be used for this instance */
#ifdef IAM_M0APP
    #define SUBINDEX IPC_APP
#endif

#ifdef IAM_M0SUB
    #define SUBINDEX IPC_SUB
#endif
// ============================================================================================
#define MAX_CLOSE_DELAY 1000  /* Max time in mS to wait for port to be flushed */
#define STANDARD_TIMEOUT 100 /* Max time in mS to wait for character space to be available */

/* We only need a single serial callback for all ports */
static EVENT_CB( *_serial_cb );

static volatile struct /* Dynamic state of a port */
{
    struct ipcBuffer *buff;
} _state[NUM_IPCS];

// ============================================================================================
// ============================================================================================
// ============================================================================================
// Internal Routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
#ifdef IAM_M4

void M0CORE_IRQHandler( void ) ALIAS( M0APP_IRQHandler );
void M0APP_IRQHandler( void )

{
    /* Clear down the interrupt */
    Chip_CREG_ClearM0AppEvent();

    /* Reception check first */
    if ( _state[IPC_APP].buff->m04.rp != _state[IPC_APP].buff->m04.wp )
    {
        CALLBACK( _serial_cb, IPC_APP );
    }
}

void M0SUB_IRQHandler( void )

{
    /* Clear down the interrupt */
    Chip_CREG_ClearM0SubEvent();

    /* Reception check first */
    if ( _state[IPC_SUB].buff->m04.rp != _state[IPC_SUB].buff->m04.wp )
    {
        CALLBACK( _serial_cb, IPC_SUB );
    }
}
#endif

#if defined(IAM_M0APP) | defined(IAM_M0SUB)
void M0_M4CORE_IRQHandler( void )

/* Interrupt happens either because the last data has been consumed by the recipient, or there
 * is data for us. We cannot tell which, so need to look at the buffers to check.
 */
{
    /* Clear down the interrupt */
    Chip_CREG_ClearM4Event();

    /* Reception check */
    if ( _state[SUBINDEX].buff->m40.rp != _state[SUBINDEX].buff->m40.wp )
    {
        CALLBACK( _serial_cb, IPC_M4 );
    }
}
#endif
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Externally Available Routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
BOOL ipcTxRoomCheck( enum ipc port, uint32_t len )

{
    struct buff *b;

    ASSERT( port < NUM_IPCS );

#ifdef IAM_M4
    b = &( _state[port].buff->m40 );
#else
    ASSERT( port == IPC_M4 );
    b = &( _state[SUBINDEX].buff->m04 );
#endif
    return ( ( len < ( b->len - ( ( b->wp + b->len - b->rp ) % b->len ) ) ) );
}
// ============================================================================================
BOOL ipcTx( enum ipc port, uint8_t *d, uint32_t len )

/* Transmit entire message or nothing at all */

{
    struct buff *b;

    ASSERT( port < NUM_IPCS );

    if ( !ipcTxRoomCheck( port, len ) )
    {
        return FALSE;
    }

#ifdef IAM_M4
    b = &( _state[port].buff->m40 );
#else
    ASSERT( port == IPC_M4 );
    b = &( _state[SUBINDEX].buff->m04 );
#endif

    while ( len )
    {
        /* Write this and move along */
        b->buffer[b->wp] = *d++;
        len--;
        b->wp = ( ( b->wp ) + 1 ) % b->len;
    }

    return TRUE;
}
// ============================================================================================
void ipcAlertFarEnd( void )

{
    __DSB();
    __SEV();
}
// ============================================================================================
BOOL ipcOpenPort( enum ipc port )

/* Open specified port */

{
    ASSERT( port < NUM_IPCS );
#ifdef IAM_M4

    if ( port == IPC_APP )
    {
        NVIC_SetPriority( M0APP_IRQn, 7 );
        NVIC_EnableIRQ( M0APP_IRQn );
    }

    if ( port == IPC_SUB )
    {
        NVIC_SetPriority( M0SUB_IRQn, 7 );
        NVIC_EnableIRQ( M0SUB_IRQn );
    }

#endif

#if defined(IAM_M0APP) | defined (IAM_M0SUB)
    NVIC_SetPriority( M4_IRQn, 7 );
    NVIC_EnableIRQ( M4_IRQn );
#endif

    return TRUE;
}
// ============================================================================================
uint8_t ipcGetRx( enum ipc port )

/* Get rx data for this port */

{
    uint8_t r = 0;
    ASSERT( port < NUM_IPCS );

    if ( !ipcDataPending( port ) )
    {
        return  0;
    }

#ifdef IAM_M4
    r = _state[port].buff->m04.buffer[_state[port].buff->m04.rp];
    _state[port].buff->m04.rp = ( ( _state[port].buff->m04.rp ) + 1 ) % _state[port].buff->m04.len;
#else
    ASSERT( port == IPC_M4 );
    r = _state[SUBINDEX].buff->m40.buffer[_state[SUBINDEX].buff->m40.rp];
    _state[SUBINDEX].buff->m40.rp = ( ( _state[SUBINDEX].buff->m40.rp ) + 1 ) % _state[SUBINDEX].buff->m40.len;
#endif

    return r;
}
// ============================================================================================
BOOL ipcConnected( enum ipc  port )

/* Get connected status for this port */

{
    ASSERT( port < NUM_IPCS );
    return TRUE;
}
// ============================================================================================
BOOL ipcDataPending( enum ipc port )

/* Indicate if there is RX path data waiting */

{
#ifdef IAM_M4
    return ( _state[port].buff->m04.rp != _state[port].buff->m04.wp );
#else
    ASSERT( port == IPC_M4 );
    return ( _state[SUBINDEX].buff->m40.rp != _state[SUBINDEX].buff->m40.wp );
#endif

    return 0;
}
// ============================================================================================
void ipcClosePort( enum ipc port )

/* Close specified port */

{
    ASSERT( port < NUM_IPCS );
}
// ============================================================================================
void ipcFlush( enum ipc port )

/* Flush our receive buffer */

{
    ASSERT( port < NUM_IPCS );

#ifdef IAM_M4
    _state[port].buff->m40.rp = _state[port].buff->m40.wp;
#else
    ASSERT( port == IPC_M4 );
    _state[SUBINDEX].buff->m04.rp = _state[SUBINDEX].buff->m04.wp;
#endif
}
// ============================================================================================
void ipcInit( EVENT_CB( *cb_set ) )

/* Loop through all ipc interfaces and set them up/clears them */

{
    _serial_cb = cb_set;
    uint8_t *allocframe = ( uint8_t * )ADDR_IPCAPP_BUFFER;

    /* Set the locations where these buffers will be held */
    _state[IPC_APP].buff = ( struct ipcBuffer * )allocframe;
    allocframe += sizeof( struct ipcBuffer );
    bzero( _state[IPC_APP].buff, sizeof( struct ipcBuffer ) );
    _state[IPC_APP].buff->m40.buffer = ( uint8_t * )allocframe;
    allocframe += IPC_QUEUE_LEN;
    _state[IPC_APP].buff->m04.buffer = ( uint8_t * )allocframe;
    allocframe += IPC_QUEUE_LEN;
    _state[IPC_APP].buff->m40.len = IPC_QUEUE_LEN;
    _state[IPC_APP].buff->m04.len = IPC_QUEUE_LEN;

    _state[IPC_SUB].buff = ( struct ipcBuffer * )allocframe;
    allocframe += sizeof( struct ipcBuffer );
    bzero( _state[IPC_SUB].buff, sizeof( struct ipcBuffer ) );
    _state[IPC_SUB].buff->m40.buffer = ( uint8_t * )allocframe;
    allocframe += IPC_QUEUE_LEN;
    _state[IPC_SUB].buff->m04.buffer = ( uint8_t * )allocframe;
    allocframe += IPC_QUEUE_LEN;
    _state[IPC_SUB].buff->m40.len = IPC_QUEUE_LEN;
    _state[IPC_SUB].buff->m04.len = IPC_QUEUE_LEN;
}
// ============================================================================================
