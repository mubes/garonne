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
 * Led String Handler
 * ==================
 *
 * Maintain LED string
 *
 */

#ifndef _LEDS_H_
#define _LEDS_H_

#include "config.h"

#define RED(x)          ((x&0xff)<<16)
#define GREEN(x)        ((x&0xff)<<8)
#define BLUE(x)         (x&0xff)
#define WHITE(x)        (RED(x) | GREEN(x) | BLUE(x))
#define LED_OFF         (0)

// ==============================================================================================
BOOL LEDsetColour(uint32_t led, uint32_t colour);       /* Set specific LED to specific colour */
BOOL LEDclearAll();                                     /* Clear all pending LEDs */
BOOL LEDPrint(void);                                    /* Request Output current state of LEDs to phy */
void LEDDoPrint(void);                                  /* Perform output current LED state to phy */
void LEDInit(void);                                     /* Initialize LED driver*/
// ============================================================================================

#endif /* _LEDS_H_ */
