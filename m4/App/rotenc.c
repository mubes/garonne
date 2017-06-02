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
* Rotary Encoder module
* =====================
*
* This module monitors the rotary encoder to see what movement is occuring.
*
* Based on a brilliant little article from http://www.mkesc.co.uk/ise.pdf.
*
*/

// ============================================================================================

#include "config.h"
#include "rotenc.h"
#include "ui.h"
#include "lms.h"

/* increment of angle for the 16 possible bit codes */
const int32_t _moveTable[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

volatile struct
{
    uint32_t ab;
    int32_t psn;
} _p;
// ============================================================================================
static inline void _check( void )

{
    _p.ab = ( _p.ab << 2 )
            | ( Chip_GPIO_GetPinState( LPC_GPIO_PORT, GETGPIOPORT( ROT_A ), GETGPIOPIN( ROT_A ) ) << 1 )
            | ( Chip_GPIO_GetPinState( LPC_GPIO_PORT, GETGPIOPORT( ROT_B ), GETGPIOPIN( ROT_B ) ) );

    _p.psn += _moveTable[( _p.ab & 0xf )];
}
// ============================================================================================
void ROT_A_HANDLER( void )

{
    _check();
    Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( ROT_A_CHANNEL ) );
}
// ============================================================================================
void ROT_B_HANDLER( void )

{
    _check();
    Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( ROT_B_CHANNEL ) );
}
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Publicly available routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
int32_t RotencGetPsn( void )

{
    return _p.psn;
}
// ============================================================================================
void RotencCheckOutput( void )

{
    LmsSendRevTicks( _p.psn );
}
// ============================================================================================
void RotencInit( void )

{
    Chip_SCU_PinMuxSet( GETPORT( ROT_A ), GETPIN( ROT_A ), GETFUNC( ROT_A ) );
    Chip_GPIO_SetPinDIRInput( LPC_GPIO_PORT, GETGPIOPORT( ROT_A ), GETGPIOPIN( ROT_A ) );
    Chip_SCU_GPIOIntPinSel( ROT_A_CHANNEL, GETGPIOPORT( ROT_A ), GETGPIOPIN( ROT_A ) );
    Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( ROT_A_CHANNEL ) );
    Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH( ROT_A_CHANNEL ) );
    Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH( ROT_A_CHANNEL ) );
    Chip_PININT_EnableIntHigh( LPC_GPIO_PIN_INT, PININTCH( ROT_A_CHANNEL ) );
    NVIC_ClearPendingIRQ( ROT_A_IRQn );
    NVIC_SetPriority( ROT_A_IRQn, ROT_ENC_INT_PRIORITY );
    NVIC_EnableIRQ( ROT_A_IRQn );


    Chip_SCU_PinMuxSet( GETPORT( ROT_B ), GETPIN( ROT_B ), GETFUNC( ROT_B ) );
    Chip_GPIO_SetPinDIRInput( LPC_GPIO_PORT, GETGPIOPORT( ROT_B ), GETGPIOPIN( ROT_B ) );
    Chip_SCU_GPIOIntPinSel( ROT_B_CHANNEL, GETGPIOPORT( ROT_B ), GETGPIOPIN( ROT_B ) );
    Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( ROT_B_CHANNEL ) );
    Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH( ROT_B_CHANNEL ) );
    Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH( ROT_B_CHANNEL ) );
    Chip_PININT_EnableIntHigh( LPC_GPIO_PIN_INT, PININTCH( ROT_B_CHANNEL ) );
    NVIC_ClearPendingIRQ( ROT_B_IRQn );
    NVIC_SetPriority( ROT_B_IRQn, ROT_ENC_INT_PRIORITY );
    NVIC_EnableIRQ( ROT_B_IRQn );
}
// ============================================================================================
