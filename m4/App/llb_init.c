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
 * Initialize LLB v2 board
 * ========================
 *
 * Setup initial pinmuxing and clocking
 */

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "config.h"


// ============================================================================================
// ============================================================================================
// ============================================================================================
// Externally Available Routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
void LLBInitSetupMuxing(void)

/*
 * Sets up system pin muxing.
 */
{

    Chip_SCU_ClockPinMuxSet(GETPIN(CLOCK0),GETFUNC(CLOCK0));
    Chip_SCU_ClockPinMuxSet(GETPIN(CLOCK1),GETFUNC(CLOCK1));
    Chip_SCU_ClockPinMuxSet(GETPIN(CLOCK2),GETFUNC(CLOCK2));
    Chip_SCU_ClockPinMuxSet(GETPIN(CLOCK3),GETFUNC(CLOCK3));
    /* SPIFI pin setup is done prior to setting up system clocking */
    Chip_SCU_PinMuxSet(GETPORT(SPIFI_CS),GETPIN(SPIFI_CS  ),GETFUNC(SPIFI_CS));
    Chip_SCU_PinMuxSet(GETPORT(SPIFI_SCK ),GETPIN(SPIFI_SCK ),GETFUNC(SPIFI_SCK ));
    Chip_SCU_PinMuxSet(GETPORT(SPIFI_MISO),GETPIN(SPIFI_MISO),GETFUNC(SPIFI_MISO));
    Chip_SCU_PinMuxSet(GETPORT(SPIFI_MOSI),GETPIN(SPIFI_MOSI),GETFUNC(SPIFI_MOSI));
    Chip_SCU_PinMuxSet(GETPORT(SPIFI_SIO2),GETPIN(SPIFI_SIO2),GETFUNC(SPIFI_SIO2));
    Chip_SCU_PinMuxSet(GETPORT(SPIFI_SIO3),GETPIN(SPIFI_SIO3),GETFUNC(SPIFI_SIO3));
}
// ============================================================================================
void LLBInitSetupClocking(void)

/*
 * Set up and initialize clocking.
 */

{
    /* Setup FLASH acceleration to target clock rate prior to clock switch */
    Chip_SetupCoreClock(CLOCK_INPUT, MAX_CLOCK_FREQ, true);

    /* Reset and enable 32Khz oscillator */
    LPC_CREG->CREG0 &= ~((1 << 3) | (1 << 2));
    LPC_CREG->CREG0 |= (1 << 1) | (1 << 0);

    /* Attach main PLL clock to divider C with a divider of 2 */
    Chip_Clock_SetDivider(CLK_IDIV_C, CLKIN_MAINPLL, 2);
}
// ============================================================================================
