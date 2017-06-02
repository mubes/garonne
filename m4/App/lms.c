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
* LMS module
* ==========
*
* Format and send/receive messages over serial link
*
*/

#include "config.h"
#include "serport.h"
#include "lms.h"
#include "lmsmsg.h"
#include "lms_rx.h"
#include "gio.h"

// ============================================================================================
// ============================================================================================
// ============================================================================================
// Messages from HLB to Vehicle
// ============================================================================================
// ============================================================================================
// ============================================================================================
BOOL LmsSendSetSpeed( int32_t speed )

{
    uint8_t b[] = {HEADER( CMD_TYPE_SETSPEED, 0 ), INT16TO8( speed ), FOOTER};
    return LmsMsgTx( b, sizeof( b ) );
}

BOOL _decodeCMD_TYPE_SETSPEED( uint8_t sensorID, uint8_t *pkt )

{
    return LmsRxSetSpeed( sint8to16( pkt ) );
}
// ============================================================================================
BOOL LmsSendSetServo( uint8_t servo, uint32_t value )

{
    uint8_t b[] = {HEADER( CMD_TYPE_SETSERVO, servo ), INT16TO8( value ), FOOTER};
    return LmsMsgTx( b, sizeof( b ) );
}

BOOL _decodeCMD_TYPE_SETSERVO( uint8_t sensorID, uint8_t *pkt )

{
    return LmsRxSetServo( sensorID, sint8to16( pkt ) );
}
// ============================================================================================
BOOL LmsSendSetLed( uint8_t led, uint8_t red, uint8_t green, uint8_t blue )

/* In this routine only one LED is set, but the protocol allows up to 12 to be set and
 * a command to be simultaneously given....the decoder below deals with that correctly.
 */

{
    uint8_t b[] = {HEADER( CMD_TYPE_SETLED, 0 ), led, red, green, blue, FOOTER};
    return LmsMsgTx( b, sizeof( b ) );
}


BOOL _decodeCMD_TYPE_SETLED( uint8_t sensorID, uint8_t len, uint8_t *pkt )

{
    len -= 6; // Remove header

    while ( len )
    {
        LmsRxSetLed( *pkt, *( pkt + 1 ), *( pkt + 2 ), *( pkt + 3 ) );
        pkt += 4;
        len -= 4;
    }

    /* In addition a command can be sent in the packet - action that too (useful for set and print in one operation */
    return LmsRxCmdLed( sensorID );
}
// ============================================================================================
BOOL LmsSendCmdLed( uint8_t command )

{
    uint8_t b[] = {HEADER( CMD_TYPE_CMDLED, command ), FOOTER};
    return LmsMsgTx( b, sizeof( b ) );
}


BOOL _decodeCMD_TYPE_CMDLED( uint8_t sensorID, uint8_t *pkt )
{
    return LmsRxCmdLed( sensorID );
}
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Messages from Vehicle to HLB, plus associated decoders
// ============================================================================================
// ============================================================================================
// ============================================================================================
BOOL LmsSendWheelTicks( uint8_t wheelId, uint32_t tickCount )

{
    uint8_t b[] = {HEADER( SENSOR_TYPE_WHEEL_ENCODER, wheelId ), INT32TO8( tickCount ), FOOTER };
    return LmsMsgTx( b, sizeof( b ) );
}

BOOL _decodeSENSOR_TYPE_WHEEL_ENCODER( uint8_t sensorID, uint8_t *pkt )
{
    return LmsRxWheelTicks( sensorID, int8to32( pkt ) );
}
// ============================================================================================
BOOL LmsSendBatteryStatus( uint16_t temperature, uint16_t stateOfChargePercentage,
                           uint16_t estimatedTimeToEmpty, uint16_t voltage )

{
    uint8_t b[] = {HEADER( SENSOR_TYPE_BATTERY_STATUS, 0 ), INT16TO8( temperature ), INT16TO8( stateOfChargePercentage ), INT16TO8( estimatedTimeToEmpty ), INT16TO8( voltage ), FOOTER};
    return LmsMsgTx( b, sizeof( b ) );
}

BOOL _decodeSENSOR_TYPE_BATTERY_STATUS( uint8_t sensorID, uint8_t *pkt )

{
    return LmsRxBatteryStatus( int8to16( pkt ), int8to16( pkt + 2 ), int8to16( pkt + 4 ), int8to16( pkt + 6 ) );
}
// ============================================================================================
BOOL LmsSendUsRangeSensor( uint8_t sensorId, uint32_t distanceToObjectMm )

{
    uint8_t b[] = { HEADER( SENSOR_TYPE_US_RANGE, sensorId ), INT32TO8( distanceToObjectMm ), FOOTER };
    return LmsMsgTx( b, sizeof( b ) );
}

BOOL _decodeSENSOR_TYPE_US_RANGE( uint8_t sensorID, uint8_t *pkt )

{
    return LmsRxUsRangeSensor( sensorID, int8to32( pkt ) );
}
// ============================================================================================
BOOL LmsSendCarStatus( BOOL emergencyStopFront, BOOL emergencyStopBack, uint32_t servoPsn,
                       int32_t motorSpeed )

{
    uint8_t  b[] = { HEADER( SENSOR_TYPE_CAR_STATUS, 0 ), emergencyStopFront, emergencyStopBack,
                     INT32TO8( servoPsn ), INT32TO8( motorSpeed ), FOOTER
                   };
    return LmsMsgTx( b, sizeof( b ) );
}

BOOL _decodeSENSOR_TYPE_CAR_STATUS( uint8_t sensorID, uint8_t *pkt )

{
    return LmsRxCarStatus( *pkt, *( pkt + 1 ), int8to32( pkt + 2 ), sint8to32( pkt + 6 ) );
}
// ============================================================================================
BOOL LmsSend9DData( int16_t *pAccXyz, int16_t *pGyrXyz, int16_t *pMagXyz )

/**
 * Accelerometer and Gyrometer data read at once
 *
 * @param pAccXyz Array of three elements for storing Accelerometer XYZ data
 * @param pGyrXyz Array of three elements for storing Gyrometer XYZ data
 * @param pMagXyz Array of three elements for storing Magnetometer XYZ data
 */
{

    uint8_t b[] = {HEADER( SENSOR_TYPE_9D_SENSOR, 0 ),
                   INT16TO8( pAccXyz[0] ), INT16TO8( pAccXyz[1] ), INT16TO8( pAccXyz[2] ),
                   INT16TO8( pGyrXyz[0] ), INT16TO8( pGyrXyz[1] ), INT16TO8( pGyrXyz[2] ),
                   INT16TO8( pMagXyz[0] ), INT16TO8( pMagXyz[1] ), INT16TO8( pMagXyz[2] ),
                   FOOTER
                  };
    return LmsMsgTx( b, sizeof( b ) );
}

BOOL _decodeSENSOR_TYPE_9D_SENSOR( uint8_t sensorID, uint8_t *pkt )

{
    static int16_t AccXyz[3];
    static int16_t GryXyz[3];
    static int16_t MagXyz[3];

    AccXyz[0] = int8to16( pkt );
    AccXyz[1] = int8to16( pkt + 2 );
    AccXyz[2] = int8to16( pkt + 4 );
    GryXyz[0] = int8to16( pkt + 6 );
    GryXyz[1] = int8to16( pkt + 8 );
    GryXyz[2] = int8to16( pkt + 10 );
    MagXyz[0] = int8to16( pkt + 12 );
    MagXyz[1] = int8to16( pkt + 14 );
    MagXyz[2] = int8to16( pkt + 16 );

    return LmsRx9DData( AccXyz, GryXyz, MagXyz );
}
// ============================================================================================
BOOL LmsSendRevTicks( int32_t tickCount )

{
    uint8_t b[] = { HEADER( SENSOR_TYPE_REV_TICKS, 0 ), INT32TO8( tickCount ), FOOTER };
    return LmsMsgTx( b, sizeof( b ) );
}

BOOL _decodeSENSOR_TYPE_REV_TICKS( uint8_t sensorID, uint8_t *pkt )

{
    return LmsRxRevTicks( int8to32( pkt ) );
}
// ============================================================================================
BOOL LmsSendDistance( uint32_t distance, uint32_t ToObject, int32_t x, int32_t y, int32_t z )

{
    uint8_t b[] = { HEADER( SENSOR_TYPE_DISTANCE, 0 ), INT16TO8( distance ), INT16TO8( ToObject ), INT16TO8( x ), INT16TO8( y ), INT16TO8( z ), FOOTER };
    return LmsMsgTx( b, sizeof( b ) );
}

BOOL _decodeSENSOR_TYPE_DISTANCE( uint8_t sensorID, uint8_t *pkt )

{
    return LmsRxDistance( int8to16( pkt ), int8to16( pkt + 2 ), int8to16( pkt + 4 ), int8to16( pkt + 6 ),
                          int8to16( pkt + 8 ) );
}
// ============================================================================================
BOOL LmsSendPosandQ( int16_t *p, int16_t *q, uint32_t tsP, uint32_t tsQ )

{
    uint8_t b[] = { HEADER( SENSOR_TYPE_PQ, 0 ), INT16TO8( p[0] ), INT16TO8( p[1] ), INT16TO8( p[2] ),
                    INT16TO8( q[0] ), INT16TO8( q[1] ), INT16TO8( q[2] ), INT16TO8( q[3] ),
                    INT32TO8( tsP ), INT32TO8( tsQ ), FOOTER
                  };
    return LmsMsgTx( b, sizeof( b ) );
}

BOOL _decodeSENSOR_TYPE_PQ( uint8_t sensorID, uint8_t *pkt )

{
    return LmsRxPQ( int8to16( pkt ), int8to16( pkt + 2 ), int8to16( pkt + 4 ),
                    int8to16( pkt + 6 ), int8to16( pkt + 8 ), int8to16( pkt + 10 ), int8to16( pkt + 12 ),
                    int8to32( pkt + 14 ), int8to32( pkt + 18 ) );
}
// ============================================================================================
BOOL LmsSendUserbutton( BOOL isSet )

{
    uint8_t b[] = {HEADER( SENSOR_TYPE_USERBUTTON, isSet ), FOOTER};
    return LmsMsgTx( b, sizeof( b ) );
}

BOOL _decodeSENSOR_TYPE_USERBUTTON( uint8_t isSet, uint8_t *pkt )
{
    return LmsRxUserbutton( isSet );
}
// ============================================================================================
BOOL _decodeCMD_TYPE_MGMT( uint8_t sensorID, uint8_t *pkt )

{
    switch ( sensorID )
    {
        case MGMT_CMD_FLASHUPDATE:

            /* Make sure command signature is good */
            for ( uint8_t i = 0; i < 16; i++ )
                if ( *pkt++ != idSeq[i] )
                {
                    return FALSE;
                }

            return LmsRxCmdFlashUpdate();

        default:
            return FALSE;
    }
}
// ============================================================================================
BOOL LmsDecode( uint8_t len, uint8_t *pkt )

{
    uint32_t messageType = *pkt++;
    uint8_t sensorID = *pkt++;
    uint32_t tStamp = int8to32( pkt );
    pkt += 4;

    ( void )tStamp; /* Just to remove the unused variable warning for now */

    GIOSetConnected( TRUE ); /* indicate we are connected */

    switch ( messageType )
    {
        // -------------------------------------------------
        // -------------------------------------------------
        // UPLINK MESSAGES - HLB to Vehicle
        // -------------------------------------------------
        // -------------------------------------------------
        case SENSOR_TYPE_WHEEL_ENCODER:
            return _decodeSENSOR_TYPE_WHEEL_ENCODER( sensorID, pkt );

        case SENSOR_TYPE_BATTERY_STATUS:
            return _decodeSENSOR_TYPE_BATTERY_STATUS( sensorID, pkt );

        case SENSOR_TYPE_US_RANGE:
            return _decodeSENSOR_TYPE_US_RANGE( sensorID, pkt );

        case SENSOR_TYPE_CAR_STATUS:
            return _decodeSENSOR_TYPE_CAR_STATUS( sensorID, pkt );

        case SENSOR_TYPE_9D_SENSOR:
            return _decodeSENSOR_TYPE_9D_SENSOR( sensorID, pkt );

        case SENSOR_TYPE_REV_TICKS:
            return _decodeSENSOR_TYPE_REV_TICKS( sensorID, pkt );

        case SENSOR_TYPE_DISTANCE:
            return _decodeSENSOR_TYPE_DISTANCE( sensorID, pkt );

        case SENSOR_TYPE_USERBUTTON:
            return _decodeSENSOR_TYPE_USERBUTTON( sensorID, pkt );

        case SENSOR_TYPE_PQ:
            return _decodeSENSOR_TYPE_PQ( sensorID, pkt );

        // -------------------------------------------------
        // -------------------------------------------------
        // DOWNLINK MESSAGES - HLB to Vehicle
        // -------------------------------------------------
        // -------------------------------------------------
        case CMD_TYPE_SETSPEED:
            return _decodeCMD_TYPE_SETSPEED( sensorID, pkt );

        case CMD_TYPE_SETSERVO:
            return _decodeCMD_TYPE_SETSERVO( sensorID, pkt );

        case CMD_TYPE_SETLED:
            return _decodeCMD_TYPE_SETLED( sensorID, len, pkt );

        case CMD_TYPE_CMDLED:
            return _decodeCMD_TYPE_CMDLED( sensorID, pkt );

        case CMD_TYPE_MGMT:
            return _decodeCMD_TYPE_MGMT( sensorID, pkt );

        default:
            return FALSE;
    }
}
// ============================================================================================
void LmsInit( void )

{
    LmsMsgInit();
}
// ============================================================================================
