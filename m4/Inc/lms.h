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
 *
 * Format and send Serial messages according spec.
 *
 */


#ifndef LMS_H
#define LMS_H

#include "config.h"
#include "lmsmsg.h"

#define LMS_WHEEL_ID_FRONT_LEFT         (0)
#define LMS_WHEEL_ID_FRONT_RIGHT        (1)
#define LMS_WHEEL_ID_REAR_LEFT          (2)
#define LMS_WHEEL_ID_REAR_RIGHT         (3)

/* Flags used for signals outside of sensors types */
#define EVENT_SERIAL0                   (SERPORT_UART0) /* == 0 */
#define EVENT_SERIAL1                   (SERPORT_UART1) /* == 1 */
#define EVENT_SERIAL2                   (SERPORT_UART2) /* == 2 */
#define EVENT_SERIAL3                   (SERPORT_UART3) /* == 3 */
#define EVENT_USB0                      (SERPORT_USB)   /* == 4 */
#define EVENT_USERBUTTON                (5)
#define EVENT_REQSETDONE                (6)
#define EVENT_M0APP                     (7)
#define EVENT_M0SUB                     (8)

#define SENSOR_TYPE_US_RANGE            (10)
#define SENSOR_TYPE_CAR_STATUS          (11)
#define SENSOR_TYPE_CAR_DRIVE_VECTOR    (12)
#define SENSOR_TYPE_RANGE_SCANNER       (13)
#define SENSOR_TYPE_WHEEL_ENCODER       (14)
//#define SENSOR_TYPE_TURRET_DATA         (15) // OBSOLETE - CODE REMOVED. Can be reused.
#define SENSOR_TYPE_9D_SENSOR           (16)
#define SENSOR_TYPE_BATTERY_STATUS      (17)
#define SENSOR_TYPE_REV_TICKS           (18)
#define SENSOR_TYPE_DISTANCE            (19)
#define SENSOR_TYPE_USERBUTTON          (20)


/* Commands from HLB to LLB */
#define CMD_LED_NONE                    0
#define CMD_LED_PRINT                   1
#define CMD_LED_CLEAR                   2

#define CMD_TYPE_SETSPEED               (24)
#define CMD_TYPE_SETSERVO               (25)
#define CMD_TYPE_SETLED                 (26)
#define CMD_TYPE_CMDLED                 (27)
#define CMD_TYPE_MGMT                   (31)

#define MGMT_CMD_FLASHUPDATE            0

// ============================================================================================

/* These are the transmit side messages */

/* UPLINK - From Vehicle to HLB */
BOOL LmsSendWheelTicks(uint8_t wheelId, uint32_t tickCount);
BOOL LmsSendRevTicks(int32_t tickCount);
BOOL LmsSend9DData(int16_t* pAccXyz, int16_t* pGyrXyz, int16_t* pMagXyz);
BOOL LmsSendUsRangeSensor(uint8_t sensorId, uint32_t distanceToObjectMm);
BOOL LmsSendBatteryStatus(uint16_t temperature, uint16_t stateOfChargePercentage,
                          uint16_t estimatedTimeToEmpty, uint16_t voltage);
BOOL LmsSendCarStatus(BOOL emergencyStopFront,BOOL emergencyStopBack,uint32_t servoPsn,
                      int32_t motorSpeed);
BOOL LmsSendDistance(uint32_t distance, uint32_t ToObject, int32_t x, int32_t y, int32_t z);
BOOL LmsSendUserbutton(BOOL isSet);

/* DOWNLINK - From HLB to Vehicle */
BOOL LmsSendSetSpeed(int32_t speed);
BOOL LmsSendSetServo(uint8_t servo, uint32_t value);
BOOL LmsSendSetLed(uint8_t led, uint8_t r, uint8_t g, uint8_t b);
BOOL LmsSendCmdLed(uint8_t command);

BOOL LmsDecode(uint8_t len, uint8_t *pkt);
void LmsInit(void);
// ============================================================================================
#endif
