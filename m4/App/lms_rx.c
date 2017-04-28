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
 * LMS Receiver for vehicle side module
 * ====================================
 *
 * Handle received messages over serial link. These routines override the WEAK definitions in the
 * protocol file and provide the actual functionality.
 *
 */

#include "leds.h"
#include "config.h"
#include "serport.h"
#include "lms.h"
#include "lmsmsg.h"
#include "motor.h"

// ============================================================================================
// ============================================================================================
// ============================================================================================
// Downlink Messages from HLB to Vehicle
// ============================================================================================
// ============================================================================================
// ============================================================================================
BOOL LmsRxSetSpeed(int32_t speed)

{
    return motorSpeed(speed);
}
// ============================================================================================
BOOL LmsRxSetServo(uint8_t servo, uint32_t value)

{
    (void)servo; /* The protocol supports multiple servos */
    return motorServoSet(value);
}
// ============================================================================================
BOOL LmsRxSetLed(uint8_t led, uint8_t red, uint8_t green, uint8_t blue)

{
    return LEDsetColour(led,RED(red)|GREEN(green)|BLUE(blue));
}
// ============================================================================================
BOOL LmsRxCmdLed(uint8_t command)

{
    switch (command)
        {
            case CMD_LED_NONE:
                return TRUE;

            case CMD_LED_PRINT:
                return LEDPrint();

            case CMD_LED_CLEAR:
                return LEDclearAll();

            default:
                return FALSE;
        }
}
// ============================================================================================
BOOL LmsRxCmdFlashUpdate(void)

/* Received a validated command to go into flash upgrade mode */

{
    /* Not currently implemented in this configuration */
    return FALSE;
}
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Uplink messages - Vehicle to HLB
// Note that these are dummy routines here - they're implemented on the HLB but are provided
// here in case anyone ever wants to use this protocol in C.
// ============================================================================================
// ============================================================================================
// ============================================================================================
BOOL LmsRxWheelTicks(uint8_t wheelId, uint32_t tickCount)

{
    return FALSE;
}
// ============================================================================================
BOOL LmsRxRevTicks(int32_t tickCount)

{

    return FALSE;
}
// ============================================================================================
BOOL LmsRx9DData(int16_t *pAccXyz, int16_t *pGyrXyz, int16_t *pMagXyz)

{
    return FALSE;
}
// ============================================================================================
BOOL LmsRxCarStatus(BOOL emergencyStopFront,BOOL emergencyStopBack,uint32_t servoPsn,
                    int32_t motorSpeed)

{
    return FALSE;
}
// ============================================================================================
BOOL LmsRxUsRangeSensor(uint8_t sensorId, uint32_t distanceToObjectMm)

{
    return FALSE;
}
// ============================================================================================
BOOL LmsRxBatteryStatus(uint16_t temperature, uint16_t stateOfChargePercentage,
                        uint16_t estimatedTimeToEmpty, uint16_t voltage)

{
    return FALSE;
}
// ============================================================================================
BOOL LmsrxWheelTicks(uint8_t wheelId, uint32_t tickCount)

{
    return FALSE;
}
// ============================================================================================
BOOL LmsRxUserbutton(BOOL isSet)

{
    return FALSE;
}
// ============================================================================================
BOOL LmsRxDistance(uint16_t distance, uint16_t ToObject, uint16_t x, uint16_t y, uint16_t z)

{
    return FALSE;
}
// ============================================================================================

