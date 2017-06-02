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
* MAINLOOP module
* ===============
*
* This module is the main control heartbeat for the system.
*
*/


#include "../Inc/ipcMsg.h"
#include "gio.h"
#include "uartHandler.h"
#include "ui.h"
#include "stats.h"
#include "iap.h"
#include "i2chandler.h"
#include "ui.h"
#include "serport.h"
#include "dist.h"
#include "vldist.h"
#include "motor.h"
#include "rotenc.h"
#include "lms.h"
#include "mainloop.h"
#include "leds.h"
#include "sdif.h"
#include "enet.h"
#include "audio.h"
#include "can.h"
#include "generics.h"
#include "ipcHandler.h"
#include "serdes.h"
#include "ipcMsg.h"

/* Intervals at which data are asynchronously sent to HLB */
#define BATTERY_STATUS_INTERVAL (MILLIS_TO_TICKS(1000))        /* How often to check battery status */
#define LEDCHECK_INTERVAL       (MILLIS_TO_TICKS(100))         /* How often to check LED status */
#define REVS_ENC_TX_INTERVAL    (MILLIS_TO_TICKS(100))         /* How often to send revs (encoder) status */
#define VEHICLE_STATUS_INTERVAL (MILLIS_TO_TICKS(50))          /* How often to send vehicle status */
#define VL_INTERVAL             (MILLIS_TO_TICKS(100))         /* How often to send laser distance data */
#define PSNQ_TX_INTERNVAL       (MILLIS_TO_TICKS(50))          /* How often to send position and quaternion data */
#define NINEDENC_INTERVAL       (MILLIS_TO_TICKS(105))         /* How often to send nined data */
#define M0WATCHDOG_INTERVAL     (MILLIS_TO_TICKS(2000))        /* Maximum time since we last heard from the watchdog */
#define M0WATCHDOG_CHKINTERVAL  (MILLIS_TO_TICKS(4000))        /* How often the M0 watchdog is checked */

struct MLStruct
{
    xTaskHandle task;
    xTaskHandle i2ctask;
} _m;
// ============================================================================================
static portTASK_FUNCTION( _i2cThread, pvParameters )

/* A task dedicated to i2c handler activities */

{
    uint32_t evSet;
    uint32_t nowTicks;
    uint32_t lastvl = 0;     /* Last time VL sensor was read */

    BOOL haveVLSensor = FALSE;

    /* It's important that the VLInit is done before LEDInit 'cos the I2C IDs clash...so VLInit moves them out of the way */
#ifdef VL_DISTANCE
    haveVLSensor = VLDISTInit( TICKS_TO_MILLIS( VL_INTERVAL ) );
#endif

    LEDInit();
    vTaskDelay( GYRO_WAKEUP_TIME );

    while ( 1 )
    {
        xTaskNotifyWait( 0, 0xFFFFFFFF, &evSet, MILLIS_TO_TICKS( 10 ) );
        nowTicks = xTaskGetTickCount();

#ifdef VL_DISTANCE

        if ( ( haveVLSensor ) && ( nowTicks - lastvl > VL_INTERVAL ) )
        {
            VLDISTCheckOutput();
            lastvl = nowTicks;
        }

#endif
    }
}
// ============================================================================================
EVENT_CB( _ipcEv )

/* routine that is called when an IPC transaction is triggered */

{
    BaseType_t xTaskWoken = MLUpdateAvailableFromISR( ( j == IPC_APP ) ? EVENT_M0APP : EVENT_M0SUB );
    portEND_SWITCHING_ISR( xTaskWoken );
}
// ============================================================================================
static portTASK_FUNCTION( _mainThread, pvParameters )

/* The main startup block, run as a task to ensure the scheduler is active */

{
    uint32_t lastRevsEnc = 0;         /* Last time revs were sent */
    uint32_t lastStatusEnc = 0;       /* Last time vehicle status was sent */
    uint32_t lastBatteryEnc = 0;      /* Last time battery status was sent */
    uint32_t lastPsnQ = 0;            /* Last time position and quaternion was sent */
    uint32_t last9DEnc = 0;           /* Last time 9D was sent */
    uint32_t lastM0WatchdogCheck;     /* Last time M0 was checked */
    uint32_t nowTicks;                /* Current tick count */

    uint32_t evSet;                   /* Events that have been set */

    GIOSetup(); /* Its best to have the generic IO setup as the scheduler starts */
    statsInit(); /* Initialize statistics package */

    serportInit();
    LmsInit(); /* Initialize the messaging system in case we need it */
    motorInit();
#ifndef VL_DISTANCE
    DISTInit();
#endif
    RotencInit();
    UISetup();
    IPCMsgSetup();
    ipcInit( _ipcEv );

    /* Optional components */
#ifdef INCLUDE_ETHERNET
    enetInit();
#endif
#ifdef INCLUDE_SDMMC
    sdifSetup();
    sdifMount();
#endif
#ifdef INCLUDE_AUDIO
    audioInit();
#endif
#ifdef INCLUDE_CAN
    CANSetup();
#endif

    /* Don't even think about checking the M0 until it's had time to wake up */
    lastM0WatchdogCheck = xTaskGetTickCount();

    while ( 1 )
    {
        xTaskNotifyWait( 0, 0xFFFFFFFF, &evSet, MILLIS_TO_TICKS( 10 ) );
        nowTicks = xTaskGetTickCount();

        // ---------------------
        /* Most urgent to report on distance as a new reading can arrive any time */
        if ( evSet & ( 1 << SENSOR_TYPE_DISTANCE ) )
        {
            // distCheckOutput();
        }

        // ---------------------
        /* Check for anything happening over the serial ports */
        if ( evSet & ( 1 << LLB_IF_PORT ) )
        {
            LmsMsgReceive();
        }

        // ---------------------
        if ( evSet & ( 1 << TERMINAL_PORT ) )
        {
            CHECK( TRUE, UISeize( TRUE ) );
            UIProcessHandler( serportGetEvent( TERMINAL_PORT ) );
            CHECK( TRUE, UISeize( FALSE ) );
        }

        // ---------------------
        if ( evSet & ( 1 << EVENT_M0APP ) )
        {
            serdesReceive( IPC_APP );
        }

        // ---------------------
        if ( evSet & ( 1 << EVENT_M0SUB ) )
        {
            serdesReceive( IPC_SUB );
        }

        // ---------------------
        if ( evSet & ( 1 << EVENT_PRINT_LEDS ) )
        {
            LEDDoPrint();
        }

#ifndef VL_DISTANCE

        if ( evSet & ( 1 << SENSOR_TYPE_US_RANGE ) )
        {
            DISTCheckOutput();
        }

#endif
        // ---------------------
        if ( evSet & ( 1 << EVENT_USERBUTTON ) )
        {
            LmsSendUserbutton( GIOUserButtonState() );
        }

        /* =============================================== */
        /* Now deal with all of the interval based reports */
        /* =============================================== */
        if ( nowTicks - lastBatteryEnc > BATTERY_STATUS_INTERVAL )
        {
            LmsSendBatteryStatus( IPCMsgGetpsanandatt()->temp, 0, 0, GIOBattery() );
            lastBatteryEnc = nowTicks;
        }

        // ---------------------
        if ( nowTicks - lastStatusEnc > VEHICLE_STATUS_INTERVAL )
        {
            LmsSendCarStatus( motorGeteStop( MOTOR_ESTOP_FORWARDS ),
                              motorGeteStop( MOTOR_ESTOP_BACKWARDS ), motorServoGet(),
                              motorGetSpeed() );
            lastStatusEnc = nowTicks;
        }

        // ---------------------
        if ( nowTicks - lastRevsEnc > REVS_ENC_TX_INTERVAL )
        {
            RotencCheckOutput();
            lastRevsEnc = nowTicks;
        }

        // ---------------------
        if ( nowTicks - last9DEnc > NINEDENC_INTERVAL )
        {
            struct MSG9d *m = IPCMsgGet9d();

            LmsSend9DData( m->acc, m->gyr, m->mag );
            last9DEnc = nowTicks;
        }

        // ---------------------
        if ( nowTicks - lastPsnQ > PSNQ_TX_INTERNVAL )
        {
            struct MSGpsnandatt *m = IPCMsgGetpsanandatt();
            LmsSendPosandQ( m->psn, m->q, m->tsPsn, m->tsQ );
            lastPsnQ = nowTicks;
        }

        // ---------------------
        if ( nowTicks - lastM0WatchdogCheck > M0WATCHDOG_CHKINTERVAL )
        {
            if ( nowTicks - IPCMsgLastM0Ping() > M0WATCHDOG_INTERVAL )
            {
                /* We haven't heard for the M0 for a while - reset it */
                Chip_RGU_TriggerReset( RGU_M0APP_RST );
                Chip_Clock_Enable( CLK_M4_M0APP );

                /* Keep in mind the M0 image must be aligned on a 4K boundary */
                Chip_CREG_SetM0AppMemMap( M0_APPBASE );
                Chip_RGU_ClearReset( RGU_M0APP_RST );
            }

            lastM0WatchdogCheck = nowTicks;
        }

        // ---------------------

    }
}
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Externally available routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
void MLDistCheckOutput( void )

{
    // FIXME
    LmsSendBatteryStatus( IPCMsgGetpsanandatt()->temp, 0, 0, GIOBattery() ); // temperature actually read from 9D sensor...
}
// ============================================================================================
void MLUpdateAvailable( uint32_t sensor )

{
    xTaskNotify( _m.task, ( 1 << sensor ), eSetBits );
}
// ============================================================================================
void MLi2cUpdateAvailable( uint32_t event )

{
    xTaskNotify( _m.i2ctask, ( 1 << event ), eSetBits );
}
// ============================================================================================
BOOL MLUpdateAvailableFromISR( uint32_t sensor )

{
    BaseType_t xTaskWoken = FALSE;
    xTaskNotifyFromISR( _m.task, ( 1 << sensor ), eSetBits, &xTaskWoken );
    return xTaskWoken;
}
// ============================================================================================
void MLInit( void )

{
    ConfigInit(); /* Get the configuration (or default) before doing anything else */

    i2cInit();

    xTaskCreate( _mainThread, "mainLP", 512, NULL,
                 ( tskIDLE_PRIORITY + 1UL ), &_m.task );

    /* A loop for the time-intensive i2c handler */
    xTaskCreate( _i2cThread, "i2cLoop", 396, NULL,
                 ( tskIDLE_PRIORITY + 1UL ), &_m.i2ctask );
}
// ============================================================================================
// ============================================================================================
// ============================================================================================
