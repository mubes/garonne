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
* Led Handler
* ===========
*
* Maintain LED string
*
*/

#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>

#include "leds.h"
#include "config.h"
#include "lms.h"
#include "mainloop.h"

static struct
{
    uint32_t state[NUM_OF_RGB_LEDS];
} _l;

// ============================================================================================
// ============================================================================================
// ============================================================================================
// Externally available routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
void LEDDoPrint( void )

/* Actually output the LED string */
  
{
    taskENTER_CRITICAL();

    for ( uint32_t led = 0; led < NUM_OF_RGB_LEDS; led++ )
    {
        uint32_t bits = 24;
        uint32_t colour = _l.state[led];

        while ( bits-- )
        {
            /* New values are clocked on rising edges... */
            Chip_GPIO_WritePortBit( LPC_GPIO_PORT, GETGPIOPORT( LEDSTRING_DAT ), GETGPIOPIN( LEDSTRING_DAT ), ( ( colour & ( 0x00800000 ) ) != 0 ) );
            Chip_GPIO_WritePortBit( LPC_GPIO_PORT, GETGPIOPORT( LEDSTRING_CLK ), GETGPIOPIN( LEDSTRING_CLK ), FALSE );
            colour = colour << 1;
            Chip_GPIO_WritePortBit( LPC_GPIO_PORT, GETGPIOPORT( LEDSTRING_CLK ), GETGPIOPIN( LEDSTRING_CLK ), TRUE );
        }
    }

    taskEXIT_CRITICAL();
}
// ============================================================================================
BOOL LEDsetColour( uint32_t led, uint32_t colour )

/* Set the colour of specified LED (Doesn't actually commit to h/w) */
  
{
    if ( led > NUM_OF_RGB_LEDS )
    {
        return FALSE;
    }

    _l.state[led] = colour;
    return TRUE;
}
// ============================================================================================
BOOL LEDPrint( void )

/* Schedule the print event ... it will happen later */
  
{
    MLUpdateAvailable( EVENT_PRINT_LEDS );
    return TRUE;
}
// ============================================================================================
BOOL LEDclearAll( void )

      /* all leds off */

{
    for ( uint8_t i = 0; i < NUM_OF_RGB_LEDS; i++ )
    {
        LEDsetColour( i, LED_OFF );
    }

    LEDPrint();
    return TRUE;
}
// ============================================================================================
void LEDInit( void )

/* Initialize LED handler */

{
    Chip_SCU_PinMuxSet( GETPORT( LEDSTRING_CLK ), GETPIN( LEDSTRING_CLK ), GETFUNC( LEDSTRING_CLK ) );
    Chip_GPIO_SetPinDIROutput( LPC_GPIO_PORT, GETGPIOPORT( LEDSTRING_CLK ), GETGPIOPIN( LEDSTRING_CLK ) );

    Chip_SCU_PinMuxSet( GETPORT( LEDSTRING_DAT ), GETPIN( LEDSTRING_DAT ), GETFUNC( LEDSTRING_DAT ) );
    Chip_GPIO_SetPinDIROutput( LPC_GPIO_PORT, GETGPIOPORT( LEDSTRING_DAT ), GETGPIOPIN( LEDSTRING_DAT ) );

    LEDclearAll();
    LEDPrint();
}
// ============================================================================================
