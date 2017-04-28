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
 * MAIN module
 * ===========
 *
 * This module is called from the startup code and initialises the system, launches the
 * scheduler then sits back and waits.
 *
 */

#include <sys/types.h>
#include "config.h"

#include <stdio.h>

#include "mainloop.h"
#include "llb_init.h"
#include "gio.h"
// ============================================================================================
//
// Note that this code runs from Flash even if a RAM configuration is set up (set by the link
// script). This means you can't step into these routines without special configuration of the
// debug system.
//
// ============================================================================================
// ============================================================================================
// ============================================================================================
// System Setup
// ============================================================================================
// ============================================================================================
// ============================================================================================
const uint32_t OscRateIn = XTAL_FREQ;           /* Clock crystal speed */
const uint32_t ExtRateIn = 0;
const uint8_t idSeq[]= {UUID,VERSION_NUMBER,(VERSION_SEQ>>24)&0XFF,(VERSION_SEQ>>16)&0XFF,(VERSION_SEQ>>8)&0XFF,VERSION_SEQ&0XFF};
// ============================================================================================
// ============================================================================================
// ============================================================================================
// FreeRTOS Support
// ============================================================================================
// ============================================================================================
// ============================================================================================
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )

{
    /* This function will get called if a task overflows its stack */

    ( void ) pxTask;
    ( void ) pcTaskName;

    ASSERT(FALSE);
}
// ============================================================================================
void vApplicationMallocFailedHook(void)

/* FreeRTOS malloc fail hook */

{
    ASSERT(FALSE);
}
// ============================================================================================
void vApplicationIdleHook(void)

/* FreeRTOS application idle hook */

{
    /* Best to sleep here until next systick */
    __WFI();
}
// ============================================================================================
// ============================================================================================
// ============================================================================================
int main( void )

{
    Chip_CREG_SetFlashAcceleration(MAX_CLOCK_FREQ);
    Chip_SetupXtalClocking();
    LLBInitSetupMuxing();
    LLBInitSetupClocking();
    SystemCoreClockUpdate();
    fpuInit();

    /* Wakeup basic stuff we'll be needing */
    Chip_GPIO_Init(LPC_GPIO_PORT);

    /* Interrupt init */
    Chip_PININT_Init(LPC_GPIO_PIN_INT);

    /* Mainloop configuration */
    MLInit();

#ifdef INCLUDE_M0_APP
    /* All good - wake up the M0app CPU */
    Chip_RGU_TriggerReset(RGU_M0APP_RST);
    Chip_Clock_Enable(CLK_M4_M0APP);
    /* Keep in mind the M0 image must be aligned on a 4K boundary */
    Chip_CREG_SetM0AppMemMap(0x10008000);
    Chip_RGU_ClearReset(RGU_M0APP_RST);
#endif

    /* Start the scheduler */
    vTaskStartScheduler();

    /* Should certainly not reach here! */
    ASSERT(FALSE);
}
// ============================================================================================
