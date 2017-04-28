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
 * Motor Control module
 * ====================
 *
 * This module controls the motor system.
 *
 */

#include <stdlib.h>
#include "motor.h"
#include "config.h"
#include "lms.h"
#include "ui.h"

static struct

{
    uint32_t stopDir;
    int32_t  speed;
    int32_t  period;
    uint32_t ticksPerMs;
    uint32_t servoAngle;
} _m;

enum motorDir {_MOTOR_OFF, _MOTOR_FORWARDS, _MOTOR_BACKWARDS};

/* Various PWM channels in use */
#define MOTOR_PWM_OUT        (1)
#define SERVO_PWM_OUT        (2)

/* ...and the rate for the PWM */
#define SCT_PWM_RATE         (50)        /* PWM frequency 50Hz (20mS) */

// ============================================================================================
void _runMotor(enum motorDir d)

{
    switch (d)
        {
            case _MOTOR_OFF:
                Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, GETGPIOPORT(MOTOR0_M0), GETGPIOPIN(MOTOR0_M0));
                Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, GETGPIOPORT(MOTOR0_M1), GETGPIOPIN(MOTOR0_M1));
                break;

            case _MOTOR_BACKWARDS:
                Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, GETGPIOPORT(MOTOR0_M0), GETGPIOPIN(MOTOR0_M0));
                Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, GETGPIOPORT(MOTOR0_M1), GETGPIOPIN(MOTOR0_M1));
                break;

            case _MOTOR_FORWARDS:
                Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, GETGPIOPORT(MOTOR0_M0), GETGPIOPIN(MOTOR0_M0));
                Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, GETGPIOPORT(MOTOR0_M1), GETGPIOPIN(MOTOR0_M1));
                break;
        }

    Chip_SCTPWM_SetDutyCycle(LPC_SCT, MOTOR_PWM_OUT,Chip_SCTPWM_PercentageToTicks(LPC_SCT, abs(_m.speed)));
}
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Publicly available routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
BOOL motorSpeed(int32_t speed)

{
    if (abs(speed)>100)
        return FALSE;

    _m.speed=speed;

    if  ((_m.speed==0) ||
            ((_m.speed>0) && (_m.stopDir&MOTOR_ESTOP_FORWARDS)) ||
            ((_m.speed<0) && (_m.stopDir&MOTOR_ESTOP_BACKWARDS)))
        _runMotor(_MOTOR_OFF);
    else if (_m.speed>0)
        _runMotor(_MOTOR_FORWARDS);
    else if (_m.speed<0)
        _runMotor(_MOTOR_BACKWARDS);

    return TRUE;
}
// ============================================================================================
BOOL motorSeteStop(uint32_t stopDirSet)

{
    _m.stopDir|=stopDirSet;

    /* motorSpeed performs eStop checking */
    return motorSpeed(_m.speed);
}
// ============================================================================================
BOOL motorCleareStop(uint32_t unstopDirSet)

{
    _m.stopDir&=~unstopDirSet;

    /* motorSpeed performs eStop checking */
    return motorSpeed(_m.speed);
}
// ============================================================================================
BOOL motorGeteStop(uint32_t direction)

{
    return (_m.stopDir&direction)!=0;
}
// ============================================================================================
int32_t motorGetSpeed(void)

{
    return _m.speed;
}
// ============================================================================================
BOOL motorServoSet(uint32_t level)

/* Set servo PWM level */

{
    /* level value between 0 and 1000 results in a pulse width between  1us and 2us */
    Chip_SCTPWM_SetDutyCycle(LPC_SCT, SERVO_PWM_OUT, _m.ticksPerMs+((_m.ticksPerMs*level)/PWM_SERVO_RANGE));
    _m.servoAngle=level;
    return TRUE;
}
// ============================================================================================
uint32_t motorServoGet(void)

/* Get servo PWM level */

{
    return _m.servoAngle;
}
// ============================================================================================
void motorServoReset(void)

{
    motorServoSet(PWM_SERVO_RANGE/2);
}
// ============================================================================================
void motorInit(void)

{
    // set direction gpios
    Chip_SCU_PinMuxSet(GETPORT(MOTOR0_M0),GETPIN(MOTOR0_M0),GETFUNC(MOTOR0_M0));
    Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, GETGPIOPORT(MOTOR0_M0), GETGPIOPIN(MOTOR0_M0));

    Chip_SCU_PinMuxSet(GETPORT(MOTOR0_M1),GETPIN(MOTOR0_M1),GETFUNC(MOTOR0_M1));
    Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, GETGPIOPORT(MOTOR0_M1), GETGPIOPIN(MOTOR0_M1));

    Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, GETGPIOPORT(MOTOR0_M0), GETGPIOPIN(MOTOR0_M0));
    Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, GETGPIOPORT(MOTOR0_M1), GETGPIOPIN(MOTOR0_M1));

    /* Initialize the PWM */
    //Set duty cycle for all channels
    Chip_SCTPWM_Init(LPC_SCT);
    Chip_SCTPWM_SetRate(LPC_SCT, SCT_PWM_RATE);

    /* Setup PWM for the main motor */
    Chip_SCU_PinMuxSet(GETPORT(MOTOR0_PIN),GETPIN(MOTOR0_PIN),GETFUNC(MOTOR0_PIN));
    Chip_SCTPWM_SetOutPin(LPC_SCT, MOTOR_PWM_OUT, GETCTOUT(MOTOR0_PIN));
    Chip_SCTPWM_SetDutyCycle(LPC_SCT, MOTOR_PWM_OUT,Chip_SCTPWM_PercentageToTicks(LPC_SCT, 50));

    /* Setup PWM for the steering servo */
    Chip_SCU_PinMuxSet(GETPORT(SERVO_PIN), GETPIN(SERVO_PIN), GETFUNC(SERVO_PIN));
    Chip_SCTPWM_SetOutPin(LPC_SCT, SERVO_PWM_OUT, GETCTOUT(SERVO_PIN));

    /* Calculate ticks per mS....needed for servo angle */
    _m.ticksPerMs=Chip_SCTPWM_GetTicksPerCycle(LPC_SCT)*SCT_PWM_RATE/1000;

    /* Make sure everything is off */
    motorSpeed(0);
    motorServoReset();

    Chip_SCTPWM_Start(LPC_SCT);
}
// ============================================================================================
