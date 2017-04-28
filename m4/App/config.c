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
 * Config Store and Restore
 * ========================
 *
 * Available everywhere, this is the configuration management module that stores and loads configuration parameters.
 * The actual structure of the config is opaque, and known only in this module. Accessors are provided to get to/from
 * parameters.
 *
 */

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "config.h"
#include "serport.h"
#include "gio.h"

#define ID_BASE (0x40045000)

#define ITM_CONSOLE                     /* Send debug output to ITM Channel (or Terminal, otherwise) */

ConfigType ConfigStore;                 /* Current configuration (may not be the same as the saved version! */
BOOL wasDefaulted;                      /* Indication of if this load was a defaulting load */
BOOL isSaved;                           /* Indication of if the current configuration has been saved */
static uint32_t chipSerialNumber[4];    /* Storage for chip serial number */

// ============================================================================================
// Structure of the EEPROM storage - This is internal!!
// ===============================

#define EEPROM_CONFIG_OFFSET (0)    // Location of config in EEPROM
//      uint32_t version
//      ConfigType Config

// ============================================================================================
// ============================================================================================
// ============================================================================================
// Publicly available routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
void ConfigAssertDA(char *msg, char *file, uint32_t line)

/* This is a locking Assert loop (print error message and lockup tigher than a Ducks A...) */
{

    uint32_t spin=1;
    uint32_t activeLed=0;
    uint32_t nopCount=0;

    while (spin)
        {
            vTaskSuspendAll(); /* We don't want tasks running while this is going on */
            __disable_irq();
            dbgprint("%s: %s line %d" EOL,(msg==NULL)?"Assert Fail":msg,file,line);

            /* Cycle the LEDs to give a visual indication of the assert */
            /* This is a waste of battery, but at least it's only under exception conditions */
            GIOdebugLedToggle(activeLed);
            nopCount=1000000;
            while (nopCount--) __asm__("NOP");
            GIOdebugLedToggle((activeLed+1)%NUM_DEBUG_LEDS);
            activeLed=(activeLed+1)%NUM_DEBUG_LEDS;
            nopCount=1000000;
            while (nopCount--) __asm__("NOP");
        }
    __enable_irq();
    xTaskResumeAll();
}
// ============================================================================================

#ifdef ITM_CONSOLE
void dbgprint(char *fmt, ...)

{
    static char op[80];
    char *p=op;

    va_list va;
    va_start(va, fmt);
    vsiprintf(op, fmt, va);
    va_end(va);

    while(*p != '\0')
    {
        /* Filter DOS line endings */
        if ((*p=='\n' && *(p+1)=='\r') || (*p=='\r' && *(p+1)=='\n'))
        {
            ITM_SendChar('\n');
            p+=2;
        }
        else
        {
            ITM_SendChar(*p++);
        }
    }
}
#else
#ifdef DEBUG_ON_TERMINAL
void dbgprint(char *fmt, ...)

{
    static char op[PRINTF_MAXLEN];

    va_list va;
    va_start(va, fmt);
    vsiprintf(op, fmt, va);
    va_end(va);
    ASSERT(strlen(op)<=PRINTF_MAXLEN);
    uartTx(TERMINAL_PORT, (uint8_t *) op, strlen(op));
}
#else
void dbgprint(char *fmt, ...)

{
}
#endif
#endif
// ============================================================================================
void ConfigInit(void)

/* Initialise a configuration either from EEPROM or default if EEPROM isn't valid */

{
    /* First things first, identify the chip */
    /* read serial - stored in magic location ... See UM10503 Rev 1.9 Pg 42 (Table 14) */
    for (uint32_t i=0; i<4; i++)
        chipSerialNumber[i]=((uint32_t *)ID_BASE)[i];


    /* Version didn't match, so load a default */
    ConfigStore=(ConfigType)DEFAULT_CONFIG;
    wasDefaulted=TRUE;
    isSaved=FALSE;
}
// ============================================================================================
const uint32_t *ConfigGetSerialNumber(void)

/* Return serial number of the chip */

{
    return chipSerialNumber;
}
// ============================================================================================
BOOL ConfigCommit(void)

/* Save the current configuration to EEPROM */

{
    return FALSE;
}
// ============================================================================================
