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
 * MAIN module for M0App
 * =====================
 *
 * The m0 app is awoken by the m4 once all initialisation of h/w etc is complete.
 * so it just gets to play with the h/w without having to configure it.
 */

#include <sys/types.h>
#include "config.h"

// ============================================================================================
//
// Note that all M0 App code is RAM based only. It is copied from the SPIFI to the 
// RAM by the M4 during boot so there's nothing for it to do.
//
// ============================================================================================
// ============================================================================================
// ============================================================================================
// System Setup
// ============================================================================================
// ============================================================================================
// ============================================================================================
const uint32_t OscRateIn = XTAL_FREQ;			/* Clock crystal speed */
const uint32_t ExtRateIn = 0;                   /* CLKIN not used */
// ============================================================================================
void SystemInit(void)

/* Set up and initialize hardware prior to call to main - called from startup code */

{

}
// ============================================================================================
// ============================================================================================
// ============================================================================================
int main( void )

{
  uint32_t spin;

  while (1)
    {

      spin=10000000; while (spin--) __asm__("NOP");
      Chip_GPIO_SetPinToggle(LPC_GPIO_PORT, GETGPIOPORT(DEBUG_LED5),GETGPIOPIN(DEBUG_LED5));
    }
}
// ============================================================================================
