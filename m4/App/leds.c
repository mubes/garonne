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
 * PCA9634 Led Handler
 * ==================
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
#include "i2chandler.h"
#include "mainloop.h"


#define PWM_REG_OFFSET                      2
#define AUTO_INCR_NONE                      0
#define AUTO_INCR_ALL                       (4<<5)
#define AUTO_INCR_BRIGHTNESS_ONLY           (5<<5)
#define AUTO_INCR_GLOBAL_CONTROL_ONLY       (6<<5)
#define AUTO_INCR_IND_GLOBAL_CONTROL_ONLY   (7<<5)


/* reg mapping for LED PWM Controllers */
#define LED_REG_ADDR_MODE1       0x00   // read/write Mode register 1
#define LED_REG_ADDR_MODE2       0x01   // read/write Mode register 2
#define LED_REG_ADDR_PWM0        0x02   // read/write brightness control LED0
#define LED_REG_ADDR_PWM1        0x03   // read/write brightness control LED1
#define LED_REG_ADDR_PWM2        0x04   // read/write brightness control LED2
#define LED_REG_ADDR_PWM3        0x05   // read/write brightness control LED3
#define LED_REG_ADDR_PWM4        0x06   // read/write brightness control LED4
#define LED_REG_ADDR_PWM5        0x07   // read/write brightness control LED5
#define LED_REG_ADDR_PWM6        0x08   // read/write brightness control LED6
#define LED_REG_ADDR_PWM7        0x09   // read/write brightness control LED7
#define LED_REG_ADDR_GRPPWM      0x0A   // read/write group duty cycle control
#define LED_REG_ADDR_GRPFREQ     0x0B   // read/write group frequency
#define LED_REG_ADDR_LEDOUT0     0x0C   // read/write LED output state 0
#define LED_REG_ADDR_LEDOUT1     0x0D   // read/write LED output state 1
#define LED_REG_ADDR_SUBADR1     0x0E   // read/write I2C-bus subaddress 1
#define LED_REG_ADDR_SUBADR2     0x0F   // read/write I2C-bus subaddress 2
#define LED_REG_ADDR_SUBADR3     0x10   // read/write I2C-bus subaddress 3
#define LED_REG_ADDR_ALLCALLADR  0x11   // read/write LED All Call I2C-bus address

#define LED_RESET_ADDRESS        0x03   // Address to write to reset LEDs

/* MODE1 reg */
#define LED_MODE1_AI2           (1<<7)
#define LED_MODE1_AI1           (1<<6)
#define LED_MODE1_AI0           (1<<5)
#define LED_MODE1_SLEEP         (1<<4)
#define LED_MODE1_SUB1          (1<<3)
#define LED_MODE1_SUB2          (1<<2)
#define LED_MODE1_SUB3          (1<<1)
#define LED_MODE1_ALLCALL       (1<<0)
#define LED_MODE1_ACTIVE        0

/* LEDOUT0/1 reg */
#define LED_LEDOUT0_ENABLE_0    (3<<0)
#define LED_LEDOUT0_ENABLE_1    (3<<2)
#define LED_LEDOUT0_ENABLE_2    (3<<4)
#define LED_LEDOUT0_ENABLE_3    (3<<6)
#define LED_LEDOUT1_ENABLE_4    (3<<0)
#define LED_LEDOUT1_ENABLE_5    (3<<2)
#define LED_LEDOUT1_ENABLE_6    (3<<4)
#define LED_LEDOUT1_ENABLE_7    (3<<6)

/* Convenience array for accessing the ledconfig */
const uint32_t _ledcfg[NUM_OF_RGB_LEDS] = {RGBLED0, RGBLED1, RGBLED2, RGBLED3, RGBLED4, RGBLED5, RGBLED6, RGBLED7};
const uint8_t updateOrder[NUM_OF_RGB_LEDS] = {0,2,4,6,1,3,5,7};

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
void LEDDoPrint(void)

{
    /* This construct runs through the LEDs evens then odds ... gives the LED drivers more time to recover */
    for (uint32_t led=0; led<=NUM_OF_RGB_LEDS; led=(led==NUM_OF_RGB_LEDS-2)?1:led+2)
        {
            if (LED_WAS_UPDATED(_l.state[led]))
                {
                    LED_UPDATED(_l.state[led],FALSE);
                    /* Set specific RGB LED to specific colour using burst write (faster)
                     * NOTE: can only be used when r,g and b are on contagious channels on the
                     * driver  (R is used as starting channel)
                     */
                    i2cTransfer(LED_GET_DRIVER_ID(_ledcfg[led]),
                                (uint8_t[])
                    {
                        (AUTO_INCR_BRIGHTNESS_ONLY | (GET_R(_ledcfg[led]) + PWM_REG_OFFSET)),
                        GET_R(_l.state[led]),
                        GET_G(_l.state[led]),
                        GET_B(_l.state[led]),
                    }, 4, NULL, 0);
                }
        }
}
// ============================================================================================
BOOL LEDsetColour(uint32_t led, uint32_t colour)

{
    if (led>NUM_OF_RGB_LEDS)
        return FALSE;
    _l.state[led]=colour;
    LED_UPDATED(_l.state[led],TRUE);
    return TRUE;
}
// ============================================================================================
BOOL LEDPrint(void)

{
    MLi2cUpdateAvailable(ML_PRINT_LEDS);
    return TRUE;
}
// ============================================================================================
BOOL LEDclearAll(void)

{
    /* all leds off */
    for (uint8_t i=0; i < NUM_OF_RGB_LEDS; i++)
        {
            LEDsetColour(i, LED_OFF);
        }
    LEDPrint();
    return TRUE;
}
// ============================================================================================
void LEDInit(void)

/* Initialize LED handler */

{
    if(!i2cAvailable())
        {
            return;
        }

    /* Perform LED reset */
    i2cTransfer(LED_RESET_ADDRESS, (uint8_t[])
    {
        0xA5,0x5A
    }, 2, NULL, 0);

    for (uint8_t led=0; led<NUM_OF_RGB_LEDS; led++)
        {
            i2cTransfer(LED_GET_DRIVER_ID(_ledcfg[led]), (uint8_t[])
            {
                LED_REG_ADDR_MODE1 ,   LED_MODE1_ACTIVE
            },2, NULL, 0);  // wake up driver
            i2cTransfer(LED_GET_DRIVER_ID(_ledcfg[led]), (uint8_t[])
            {
                LED_REG_ADDR_LEDOUT0 , LED_LEDOUT0_ENABLE_0 | LED_LEDOUT0_ENABLE_1 | LED_LEDOUT0_ENABLE_2 |
                LED_LEDOUT0_ENABLE_3
            },2, NULL, 0);  // enable outputs
            i2cTransfer(LED_GET_DRIVER_ID(_ledcfg[led]), (uint8_t[])
            {
                LED_REG_ADDR_LEDOUT1,  LED_LEDOUT1_ENABLE_4 | LED_LEDOUT1_ENABLE_5 | LED_LEDOUT1_ENABLE_6 |
                LED_LEDOUT1_ENABLE_7
            },2, NULL, 0);  // enable outputs
        };

    LEDclearAll();
    LEDPrint();
}
// ============================================================================================
