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
 * Generic I/O module
 * ==================
 *
 * This module is responsible for the setup and control of generic I/O stuff like LED output.
 *
 */

#include "config.h"
#include "gio.h"

#include "leds.h"
#include "dist.h"
#include "motor.h"
#include "lms.h"
#include "mainloop.h"

#define DEBOUNCE_TIME           (MILLIS_TO_TICKS(50))   /* 50mS debounce timer for userbutton */

#define HB_FLASH_INTERVAL       (MILLIS_TO_TICKS(750))  /* Interval for HB flash */
#define HB_LENGTH               (MILLIS_TO_TICKS(200))  /* Length of HB flash */
#define SYNC_FLASH_LENGTH       (MILLIS_TO_TICKS(500))  /* Length of flash in ticks */
#define LED_DEFAULT_PWM         32                      /* Default brightness level */

/* Calculations for battery level, based on board population and configuration */
#define ADC_R2                  (10)
#define ADC_R1                  (56)
#define ADC_VREF_mV             (3300)
#define ADC_RESOLUTION_BITS     (10)
#define ADC_TO_mV(x)            (((x)*ADC_VREF_mV)/(1<<ADC_RESOLUTION_BITS))
#define ADC_TO_BAT_mV(x)        (ADC_TO_mV(x) * (ADC_R1+ADC_R2))/(ADC_R2)


const uint32_t debugLed[NUM_DEBUG_LEDS] =
{ DEBUG_LED0}; /* Convenience array for debug led pins */

const struct
{
    uint32_t r;
    uint32_t g;
    uint32_t b;
} _rgb_led[NUM_RGB_LED] = {{ RGB_LED_R, RGB_LED_G, RGB_LED_B }};



static struct gioStruct
{
    int32_t hbFlash;            /* Counter for heartbeat LED */

    uint32_t connected;         /* Has a host connected to us since reset? */
    uint32_t chaseLed;          /* Which LED we're currently working with */

    int16_t temp;               /* Current system temperature */
    uint16_t batLevel;          /* Data from the battery */

    TimerHandle_t debounce;     /* Debounce timer for userbutton */
    BOOL buttonState;           /* Current state of the userbutton */
    BOOL buttonStatePending;    /* Is there a debounce in progress for the Button State? */
} _g;

// ============================================================================================
// ============================================================================================
// ============================================================================================
// Internal Routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
void _checkButton( void )

/* Since the input button shares its function with a LED we need to poll it to see what it's
 * current state is.  To do that we temporarily turn the pin into an input. Read it and then
 * turn it back into an output again. This happens so fast that flicker won't be seen.
 *
 * If you put the button somewhere else then this switching can be removed!
 *
 */
{
    BOOL newState;

    /* Make the pin an input */
    Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, GETGPIOPORT(USERBUTTON),GETGPIOPIN(USERBUTTON));

    /* Need a small delay here to let the voltage build up (due to capacitive effects) */
    vTaskDelay(1);

    /* Now grab the state */
    newState=!Chip_GPIO_GetPinState(LPC_GPIO_PORT, GETGPIOPORT(USERBUTTON),GETGPIOPIN(USERBUTTON));

    /* ... and turn it back into an output */
    Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, GETGPIOPORT(USERBUTTON),GETGPIOPIN(USERBUTTON));

    if (_g.buttonState!=newState)
    {
        if (_g.buttonStatePending)
        {
            /* We were in the debounce period - report the new state */
            _g.buttonState=newState;
            _g.buttonStatePending=FALSE;
            MLUpdateAvailable(EVENT_USERBUTTON);
        }
        else
        {
            /* We didn't know about this state change - debounce it */
            _g.buttonStatePending=TRUE;
        }
    }
    else
    {
        _g.buttonStatePending=FALSE;
    }
}
// ============================================================================================
static portTASK_FUNCTION( _gioThread, pvParameters )

/* The task loop for flashing the LED */

{
    static ADC_CLOCK_SETUP_T ADCSetup;

    /* Setup the ADC for battery voltage */
    Chip_ADC_Init(BATTERY_ADC_PORT, &ADCSetup);
    Chip_ADC_EnableChannel(BATTERY_ADC_PORT, BATTERY_ADC_CHANNEL, ENABLE);
    Chip_ADC_SetBurstCmd(BATTERY_ADC_PORT, DISABLE);

    while (1)
        {
            /* Read the battery level while in the background */
            Chip_ADC_SetStartMode(BATTERY_ADC_PORT, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
            vTaskDelay(DEBOUNCE_TIME); /* Wait for a bit.... */
            _checkButton();

            /* Read ADC value */
           // ASSERT(Chip_ADC_ReadStatus(BATTERY_ADC_PORT, BATTERY_ADC_CHANNEL, ADC_DR_DONE_STAT)==SET);
            Chip_ADC_ReadValue(BATTERY_ADC_PORT, BATTERY_ADC_CHANNEL, &_g.batLevel);

            if (++_g.connected >= (CONNECTED_TICKS / SYNC_FLASH_LENGTH))
                {
                    _g.chaseLed = !_g.chaseLed;
                    LEDclearAll();
                    if (_g.chaseLed % 2)
                        {
                            for (uint8_t i = 0; i < NUM_OF_RGB_LEDS; i++)
                                {
                                    LEDsetColour(i, BLUE(LED_DEFAULT_PWM));
                                }
                        }
                    else
                        {
                            for (uint32_t i = 0; i < NUM_OF_RGB_LEDS; i++)
                                {
                                    LEDsetColour(i, GREEN(LED_DEFAULT_PWM));
                                }
                        }
                    LEDDoPrint();
                }

            _g.hbFlash+=DEBOUNCE_TIME;
            if (_g.hbFlash >= HB_FLASH_INTERVAL)
            {
                if (_g.hbFlash>=(HB_FLASH_INTERVAL+HB_LENGTH))
                    {
                        GIOdebugLedClear(HB_LED);
                        _g.hbFlash=0;
                    }
                else
                {
                    GIOdebugLedSet(HB_LED);
                }
            }
        }
}
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Externally available Routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
uint16_t GIOBattery(void)

{
    return (ADC_TO_BAT_mV(_g.batLevel));
}
// ============================================================================================
void GIORGBLedSetColour(enum RGB_LED_ENUM l, uint32_t c)

/* Set specified colour on output LED */

{
    Chip_GPIO_WritePortBit(LPC_GPIO_PORT, GETGPIOPORT(_rgb_led[l].r),
                           GETGPIOPIN(_rgb_led[l].r), !(c & 1 << 0));  // low active
    Chip_GPIO_WritePortBit(LPC_GPIO_PORT, GETGPIOPORT(_rgb_led[l].g),
                           GETGPIOPIN(_rgb_led[l].g), !(c & 1 << 1));  // low active
    Chip_GPIO_WritePortBit(LPC_GPIO_PORT, GETGPIOPORT(_rgb_led[l].b),
                           GETGPIOPIN(_rgb_led[l].b), !(c & 1 << 2));  // low active
}
// ============================================================================================
uint32_t GIOFlags(void)

{
    return ( (ConfigNomadic()?FLAG_NOMADIC:0) );
}
// ============================================================================================
uint32_t GIOTemp(void)

{
    return _g.temp;
}
// ============================================================================================
void GIOSetConnected(BOOL newConnectedVal)

{
    if ((_g.connected >= (CONNECTED_TICKS / SYNC_FLASH_LENGTH))
            && (newConnectedVal))
        {
            /* Have connected, so switch off LEDs */
            LEDclearAll();
        }

    if (newConnectedVal)
        _g.connected = 0;
    else
        _g.connected = (CONNECTED_TICKS / SYNC_FLASH_LENGTH);
}
// ============================================================================================
void GIOdebugLedSet(enum DBG_LED_ENUM led)
{
    Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, GETGPIOPORT(debugLed[led]),GETGPIOPIN(debugLed[led]));
}
// ============================================================================================
void GIOdebugLedClear(enum DBG_LED_ENUM led)
{
    Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, GETGPIOPORT(debugLed[led]),GETGPIOPIN(debugLed[led]));
}
// ============================================================================================
void GIOdebugLedToggle(enum DBG_LED_ENUM led)
{
    Chip_GPIO_SetPinToggle(LPC_GPIO_PORT, GETGPIOPORT(debugLed[led]),GETGPIOPIN(debugLed[led]));
}
// ============================================================================================
void GIOSmoke(BOOL isSmoking)

{
    // Set smoke condition (note inversion due to drive circuit)
    Chip_GPIO_SetPinState(LPC_GPIO_PORT, GETGPIOPORT(SMOKE), GETGPIOPIN(SMOKE),!isSmoking);
}
// ============================================================================================
BOOL GIOUserButtonState(void)

{
    return _g.buttonState;
}
// ============================================================================================
void GIOSetup(void)

/* Setup GIO system */

{
    /* Setup IR LED outputs */
    for (enum RGB_LED_ENUM i = 0; i < NUM_RGB_LED; i++)
        {
            Chip_SCU_PinMuxSet(GETPORT(_rgb_led[i].r), GETPIN(_rgb_led[i].r),GETFUNC(_rgb_led[i].r));
            Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, GETGPIOPORT(_rgb_led[i].r),GETGPIOPIN(_rgb_led[i].r));
            Chip_SCU_PinMuxSet(GETPORT(_rgb_led[i].g), GETPIN(_rgb_led[i].g),GETFUNC(_rgb_led[i].g));
            Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, GETGPIOPORT(_rgb_led[i].g),GETGPIOPIN(_rgb_led[i].g));
            Chip_SCU_PinMuxSet(GETPORT(_rgb_led[i].b), GETPIN(_rgb_led[i].b),GETFUNC(_rgb_led[i].b));
            Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, GETGPIOPORT(_rgb_led[i].b),GETGPIOPIN(_rgb_led[i].b));
            GIORGBLedSetColour(i, 0);
        }

    /* initialize debug leds */
    for (enum DBG_LED_ENUM j = 0; j < NUM_DEBUG_LEDS; j++)
        {
            /* Setup muxing for debug LEDs */
            Chip_SCU_PinMuxSet(GETPORT(debugLed[j]), GETPIN(debugLed[j]),GETFUNC(debugLed[j]));
            Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, GETGPIOPORT(debugLed[j]),GETGPIOPIN(debugLed[j]));
            GIOdebugLedClear(j);
        }

    /* We put smoke generator in here too... */
    Chip_SCU_PinMuxSet(GETPORT(SMOKE), GETPIN(SMOKE), GETFUNC(SMOKE));
    Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, GETGPIOPORT(SMOKE),GETGPIOPIN(SMOKE));

    _g.connected = (CONNECTED_TICKS / SYNC_FLASH_LENGTH);
    xTaskCreate(_gioThread, "GIO", 296, NULL,(tskIDLE_PRIORITY + 1UL), (xTaskHandle *) NULL);
}
// ============================================================================================
