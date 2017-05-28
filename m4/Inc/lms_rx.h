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
 * These are the receive side messages
 */

#include "config.h"
#ifndef _LMS_RX_
#define _LMS_RX_

// ============================================================================================
/* UPLINK - From Vehicle to HLB */
BOOL LmsRxWheelTicks(uint8_t wheelId, uint32_t tickCount);
BOOL LmsRx9DData(int16_t* pAccXyz, int16_t* pGyrXyz, int16_t* pMagXyz);
BOOL LmsRxUsRangeSensor(uint8_t sensorId, uint32_t distanceToObjectMm);
BOOL LmsRxBatteryStatus(uint16_t temperature, uint16_t stateOfChargePercentage,
                        uint16_t estimatedTimeToEmpty, uint16_t voltage);
BOOL LmsRxCarStatus(BOOL emergencyStopFront,BOOL emergencyStopBack,uint32_t servoPsn,
                    int32_t motorSpeed);
BOOL LmsRxRevTicks(int32_t tickCount);
BOOL LmsRxUserbutton(BOOL isSet);
BOOL LmsRxDistance(uint16_t distance, uint16_t ToObject, uint16_t x, uint16_t y, uint16_t z);
BOOL LmsRxPQ(int16_t x, int16_t y, int16_t z, int16_t q0, int16_t q1, int16_t q2, int16_t q3, uint32_t tsP, uint32_t tsQ);

/* DOWNLINK - From HLB to Vehicle */
BOOL LmsRxSetSpeed(int32_t speed);
BOOL LmsRxSetServo(uint8_t servo, uint32_t value);
BOOL LmsRxSetLed(uint8_t led, uint8_t r, uint8_t g, uint8_t b);
BOOL LmsRxCmdLed(uint8_t command);
BOOL LmsRxCmdFlashUpdate(void);
// ============================================================================================
#endif
