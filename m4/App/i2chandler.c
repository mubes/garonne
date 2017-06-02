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
* I2C handler module
* ==================
*
* Handle I2C communication
*
*/

#include "config.h"
#include "i2chandler.h"

#define I2CMAXWAIT              (20)    /* Max time in mS for a transfer before fail is assumed */

static struct
{
    BOOL                available;      /* Flag indicating that i2c is available */
    I2CM_XFER_T         xfer;           /* Transfer control buffer */
    SemaphoreHandle_t   s;              /* Signal from I2C Interrupt handler to base code */
    SemaphoreHandle_t   mutex;          /* Mutex to prevent dual simultaneous use of I2C */
} _i2c;
// ============================================================================================
void I2C0_IRQHandler( void )

{
    /* Call I2CM ISR function with the I2C device and transfer rec */
    Chip_I2CM_XferHandler( LPC_I2C0, &_i2c.xfer );

    if ( _i2c.xfer.status != I2CM_STATUS_BUSY )
    {
        BaseType_t higherPriorityTaskWoken = FALSE;
        xSemaphoreGiveFromISR( _i2c.s, &higherPriorityTaskWoken );
        portEND_SWITCHING_ISR( higherPriorityTaskWoken );
    }
}
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Public functions
// ============================================================================================
// ============================================================================================
// ============================================================================================
BOOL i2cTransfer( uint8_t slaveAddr, uint8_t *txBuff, uint32_t txLen, uint8_t *rxBuff, uint32_t rxLen )

{
    BOOL retval;

    if ( !_i2c.available )
    {
        return FALSE;
    }

    if ( !xSemaphoreTake( _i2c.mutex, MILLIS_TO_TICKS( I2CMAXWAIT ) ) )
    {
        return FALSE;
    }

    _i2c.xfer.slaveAddr = slaveAddr;
    _i2c.xfer.options = 0;
    _i2c.xfer.status = 0;
    _i2c.xfer.rxBuff = rxBuff;
    _i2c.xfer.txBuff = txBuff;
    _i2c.xfer.txSz = txLen;
    _i2c.xfer.rxSz = rxLen;

    Chip_I2CM_Xfer( LPC_I2C0, &_i2c.xfer );

    if ( !xSemaphoreTake( _i2c.s, MILLIS_TO_TICKS( I2CMAXWAIT ) ) )
    {
        // We've still got to release the mutex - better luck next time
        Chip_I2C_Init( I2C0 );
        Chip_I2CM_Init( LPC_I2C0 );
        Chip_I2C_SetClockRate( I2C0, I2C0_BITRATE );
        xSemaphoreGive( _i2c.mutex );
        return FALSE;
    }

    retval = ( _i2c.xfer.status == I2CM_STATUS_OK );
    xSemaphoreGive( _i2c.mutex );
    return retval;
}
// ============================================================================================
BOOL i2cAvailable( void )

{
    return _i2c.available;
}
// ============================================================================================
void i2cInit( void )

{
    /* Semaphore for transfer complete */
    _i2c.s = xSemaphoreCreateBinary();
    _i2c.mutex = xSemaphoreCreateMutex();

    xSemaphoreGive( _i2c.mutex );

    /* Initialize I2C */
    Chip_SCU_I2C0PinConfig( I2C0_MODE );
    Chip_I2CM_Init( LPC_I2C0 );
    Chip_I2CM_SetBusSpeed( LPC_I2C0, I2C0_BITRATE );
    Chip_I2C_SetMasterEventHandler( I2C0, Chip_I2C_EventHandler );
    NVIC_SetPriority( I2C0_IRQn, I2C0_INTPRIORITY );
    NVIC_EnableIRQ( I2C0_IRQn );

    _i2c.available = TRUE;
}
// ============================================================================================

