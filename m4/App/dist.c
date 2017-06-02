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
* Dist module
* ===========
*
* This module is used for measuring distances. It uses the same timer resources as
* the PWM module, so this module should be initialized after that one.
*
* In fact, all it depends on is that the timer is free running with a defined refresh
* interval.  The refresh interval constraints the maximum distance that can be measured
* but with 19.5mS refresh that amounts to about 6.6m...more than the range of the sensor.
*
* This module is only incorporated/made active if VL_DISTANCE isn't defined (i.e. if the VL
* distance module isn't active.). This can be changed if needed.
*
*
*/

#include "dist.h"
#include "motor.h"
#include "motor.h"
#include "lms.h"
#include "mainloop.h"
#include "ui.h"

#ifndef VL_DISTANCE

/* These do not belong in the .h file as they are only used in this file */
#define DISTCOUNTS_PER_US    (SystemCoreClock/1000000)      /* How long it takes us to get through 1uS */
#define SOUND_MM_PER_MS (343)                               /* Distance sound travels per mS  ... */

#define MINREPDIST  20                                      /* Minimum reported distance */
#define MAXDIST     4000                                    /* Maximum reported distance */

#define DIST_INTERVAL (MILLIS_TO_TICKS(60))
#define HIGH_INTERVAL (MILLIS_TO_TICKS(10))

/* State of the capture process */
enum CAPTURE_STATE_ENUM {CS_NORESULT, CS_WAIT_HIGH, CS_WAIT_LOW, CS_COMPLETE};

typedef struct

{
    uint32_t      cMin;
    uint32_t      last;
    BOOL          isNew;
    portTickType  lastTime;
    enum CAPTURE_STATE_ENUM s;
} DISTStateType;

/* Map distance sensors numbers to capture input numbers */
const uint32_t sensorToCaptureInput[] = {DIST_CAP_TIMER_INPUT_FRONT, DIST_CAP_TIMER_INPUT_BACK};

/* Trigger and capture pins */
const uint32_t _capt[DISTNONE] = {DIST_CAP_FRONT,  DIST_CAP_BACK};
const uint32_t _trig[DISTNONE] = {DIST_TRIG_FRONT, DIST_TRIG_BACK};

static volatile DISTStateType _s[DISTNONE];
static TimerHandle_t _trigTimer;
static TimerHandle_t _intervalTimer;
static uint32_t maxcount;

// ============================================================================================
void DIST_TIMER_INTERRUPT( void )

{
    /* Check each capture channel to see there's anything interesting for us */
    for ( enum DIST_CHANNEL_ENUM c = 0; c < DISTNONE; c++ )
    {
        if ( Chip_TIMER_CapturePending( DIST_TIMER, sensorToCaptureInput[c] ) )
        {
            Chip_TIMER_ClearCapture( DIST_TIMER, sensorToCaptureInput[c] );

            switch ( _s[c].s )
            {
                // ------------------------
                case CS_WAIT_HIGH:
                    ASSERT ( DIST_TIMER->CCR & TIMER_CAP_RISING(
                                         sensorToCaptureInput[c] ) ); // sanity check if the expected edge has been found

                    _s[c].cMin = Chip_TIMER_ReadCapture( DIST_TIMER, sensorToCaptureInput[c] );
                    _s[c].s++;
                    Chip_TIMER_CaptureRisingEdgeDisable( DIST_TIMER, sensorToCaptureInput[c] );
                    Chip_TIMER_CaptureFallingEdgeEnable( DIST_TIMER, sensorToCaptureInput[c] );
                    break;

                // ------------------------
                case CS_WAIT_LOW:
                    ASSERT ( DIST_TIMER->CCR & TIMER_CAP_FALLING(
                                         sensorToCaptureInput[c] ) ); // sanity check if the expected edge has been found
                    _s[c].last = ( ( Chip_TIMER_ReadCapture( DIST_TIMER,
                                     sensorToCaptureInput[c] ) + maxcount - _s[c].cMin ) % maxcount );
                    _s[c].lastTime = xTaskGetTickCountFromISR();
                    _s[c].isNew = TRUE;
                    _s[c].s++;
                    Chip_TIMER_CaptureFallingEdgeDisable( DIST_TIMER, sensorToCaptureInput[c] );
                    break;

                // ------------------------
                default:
                    break;
                    // ------------------------
            }
        }
    }
}
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Publicly available routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
uint32_t DISTgetChannel( enum DIST_CHANNEL_ENUM c )

/* Get latest reading from channel */

{
#ifndef DISABLE_DISTANCE_SENSORS
    uint32_t r;
    ASSERT( c < DISTNONE );
    r = ( ( SOUND_MM_PER_MS / 2 ) * _s[c].last ) / 1000; //   /2 is for trigger and echo

    if ( r > MINREPDIST )
    {
        return r;
    }
    else
#endif
        return MAXDIST;
}
// ============================================================================================
uint32_t DISTgetChannelAge( enum DIST_CHANNEL_ENUM c )

/* Get time of latest reading from channel */

{
    ASSERT( c < DISTNONE );
    return TICKS_TO_MILLIS( _s[c].lastTime );
}
// ============================================================================================
void _trigCompCallback( TimerHandle_t pxTimer )

{
    for ( enum DIST_CHANNEL_ENUM c = 0; c < DISTNONE; c++ )
    {
        _s[c].s = CS_WAIT_HIGH;
        Chip_GPIO_SetPinOutLow( LPC_GPIO_PORT, GETGPIOPORT( _trig[c] ), GETGPIOPIN( _trig[c] ) );
        Chip_TIMER_CaptureRisingEdgeEnable( DIST_TIMER, sensorToCaptureInput[c] );
    }
}
// ============================================================================================
void _trigDistCallback( TimerHandle_t pxTimer )

/* Start a distance reading process */

{
    for ( enum DIST_CHANNEL_ENUM c = 0; c < DISTNONE; c++ )
    {
        Chip_TIMER_CaptureFallingEdgeDisable( DIST_TIMER, sensorToCaptureInput[c] );

        if ( _s[c].s != CS_COMPLETE )
        {
            _s[c].last = 2000;
        }

        MLUpdateAvailable( SENSOR_TYPE_US_RANGE );

        Chip_GPIO_SetPinOutHigh( LPC_GPIO_PORT, GETGPIOPORT( _trig[c] ), GETGPIOPIN( _trig[c] ) );
    }

    xTimerStart( _trigTimer, 0 );
}
// ============================================================================================
void DISTCheckOutput( void )

{
    for ( enum DIST_CHANNEL_ENUM c = 0; c < DISTNONE; c++ )
    {
        if ( _s[c].isNew )
        {
            //TODO OSW: add emergency stop deglitch filter to prevent false positives

            if ( DISTgetChannel( c ) < MINDIST )
            {
                motorSeteStop( c == DIST_FRONT ? MOTOR_ESTOP_FORWARDS : MOTOR_ESTOP_BACKWARDS );
            }
            else
            {
                motorCleareStop( c == DIST_FRONT ? MOTOR_ESTOP_FORWARDS : MOTOR_ESTOP_BACKWARDS );
            }

            _s[c].isNew = FALSE;
            MLUpdateAvailable( SENSOR_TYPE_CAR_STATUS );

            LmsSendUsRangeSensor( c, DISTgetChannel( c ) );
        }
    }
}
// ============================================================================================
void DISTInit( void )

{
    Chip_TIMER_Init( DIST_TIMER );
    Chip_TIMER_PrescaleSet( DIST_TIMER, DISTCOUNTS_PER_US );
    Chip_TIMER_ResetOnMatchEnable( DIST_TIMER, 1 );
    maxcount = ( Chip_Clock_GetRate( CLK_MX_TIMER1 ) / DIST_SAMPLE_RATE );

    // set sample rate
    Chip_TIMER_SetMatch( DIST_TIMER, 1, maxcount );

    // set capture pins
    for ( enum DIST_CHANNEL_ENUM c = 0; c < DISTNONE; c++ )
    {
        Chip_TIMER_ClearCapture( DIST_TIMER, sensorToCaptureInput[c] );
        Chip_TIMER_CaptureEnableInt( DIST_TIMER, sensorToCaptureInput[c] );
        //  configure trigger output
        Chip_SCU_PinMuxSet( GETPORT( _trig[c] ), GETPIN( _trig[c] ), GETFUNC( _trig[c] ) );
        Chip_GPIO_SetPinDIROutput( LPC_GPIO_PORT, GETGPIOPORT( _trig[c] ), GETGPIOPIN( _trig[c] ) );
        Chip_GPIO_SetPinOutLow( LPC_GPIO_PORT, GETGPIOPORT( _trig[c] ), GETGPIOPIN( _trig[c] ) );
        // configure capture input
        Chip_SCU_PinMuxSet( GETPORT( _capt[c] ), GETPIN( _capt[c] ), GETFUNC( _capt[c] ) );
        LPC_GIMA->CAP0_IN[1][sensorToCaptureInput[c]] = GETGIMASELECT(
                            _capt[c] ); // configure GIMA mux to use the correct capture pins
    }

    // configure GIMA mux to use the correct capture pins
    Chip_TIMER_Enable( DIST_TIMER );

    NVIC_SetPriority( DIST_TIMER_IRQ, DIST_INTPRIORITY );
    NVIC_ClearPendingIRQ( DIST_TIMER_IRQ );
    NVIC_EnableIRQ( DIST_TIMER_IRQ );

    _intervalTimer = xTimerCreate( "TrigInt", DIST_INTERVAL, pdTRUE, 0, _trigDistCallback );
    _trigTimer = xTimerCreate( "Trig", HIGH_INTERVAL, pdFALSE, 0, _trigCompCallback );
    xTimerStart( _intervalTimer, 0 );
}
// ============================================================================================
#endif
