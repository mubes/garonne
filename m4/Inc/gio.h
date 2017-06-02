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
 * This module is responsible for the setup and control of generic I/O stuff like the ID
 * bit getting and LED output.
 *
 */

#ifndef _GIO_H_
#define _GIO_H_

#include "config.h"

enum RGB_LED_ENUM {RGB0_LED, NUM_RGB_LED };
enum DBG_LED_ENUM {DBG0_LED, NUM_DEBUG_LEDS };

#define NO_BATT   (0)                   /* Indicator that there is no battery present */

// ============================================================================================
void GIOTaskRun( void );
void GIORGBLedSetColour( enum RGB_LED_ENUM l, uint32_t c );
uint16_t GIOBattery( void );
void GIOSetConnected( BOOL newConnectedVal );
void GIOdebugLedSet( enum DBG_LED_ENUM led );
void GIOdebugLedClear( enum DBG_LED_ENUM led );
void GIOdebugLedToggle( enum DBG_LED_ENUM led );
void GIOSmoke( BOOL isSmoking );
BOOL GIOUserButtonState( void );
uint32_t GIOFlags( void );
uint32_t GIOTemp( void );

void GIOSetup( void );
// ============================================================================================
#endif /* _GIO_H_ */
