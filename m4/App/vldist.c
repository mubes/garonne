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
 * Dist based on VL53L0X module
 * ============================
 *
 * This module is used for measuring distances using the VL53L0X module and the ST provided API.  It also provides
 * the 'glue' functions from the ST library to our i2c implementation.
 *
 *
 */

#include <string.h>
#include "vldist.h"
#include "lms.h"
#include "mainloop.h"
#include "motor.h"
#include "ui.h"
#include "vl53l0x_platform.h"
#include "vl53l0x_api.h"
#include "i2chandler.h"

#define MAX_TFR_LEN 63                      // Maximum transfer length
#define VL_COMMS_SPEED_KHZ          400     // Speed of comms with VL devices
#define I2CMAXWAIT (MILLIS_TO_TICKS(10))    // Length of time to wait for a I2C communication
#define STARTING_ID                 0x29    // Starting ID where all VL chips wake up
#define COMPUTE_OVERHEAD            10000   // Time needed in addition to ranging time to create reading

static const struct
{
    uint32_t i2cID;
    uint32_t pinConfig;
} _vl[NUM_VLS] = {VLSLIST};                 // Static pin config per VL head

static struct

{
    VL53L0X_Dev_t vlDev;
    BOOL available;
    VL53L0X_DeviceInfo_t deviceInfo;
} _d[NUM_VLS];                              // Information for VL head

static uint8_t _dat[MAX_TFR_LEN
                    +1];         // Data transfer buffer - only need one as this lot is synchronous
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Tie-in functions to give access to I2C to ST API Library
// ============================================================================================
// ============================================================================================
VL53L0X_Error VL53L0X_LockSequenceAccess(VL53L0X_DEV Dev)

{
    /* Despite this being in the API, it never gets called, so let's assume its not used and do it ourselves */
    return VL53L0X_ERROR_NONE;
}
// ============================================================================================
VL53L0X_Error VL53L0X_UnlockSequenceAccess(VL53L0X_DEV Dev)

{
    /* Despite this being in the API, it never gets called, so let's assume its not used and do it ourselves */
    return VL53L0X_ERROR_NONE;
}
// ============================================================================================
VL53L0X_Error VL53L0X_WriteMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count)

{
    _dat[0]=index;
    memcpy(&(_dat[1]),pdata,count);

    if (i2cTransfer(Dev->I2cDevAddr, _dat, count+1, NULL, 0))
        {
            return VL53L0X_ERROR_NONE;
        }
    return VL53L0X_ERROR_INVALID_PARAMS;
}
// ============================================================================================
VL53L0X_Error VL53L0X_ReadMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count)

{
    if (i2cTransfer((Dev->I2cDevAddr), (uint8_t [])
    {
        index
    },1,pdata, count))
    return VL53L0X_ERROR_NONE;

    return  VL53L0X_ERROR_CONTROL_INTERFACE;
}
// ============================================================================================
VL53L0X_Error VL53L0X_WrByte(VL53L0X_DEV Dev, uint8_t index, uint8_t data)

{
    _dat[0]=index;
    _dat[1]=data;

    if (i2cTransfer(Dev->I2cDevAddr, _dat, 2, NULL, 0))
        {
            return VL53L0X_ERROR_NONE;
        }

    return VL53L0X_ERROR_CONTROL_INTERFACE;
}
// ============================================================================================
VL53L0X_Error VL53L0X_WrWord(VL53L0X_DEV Dev, uint8_t index, uint16_t data)

{
    _dat[0]=index;
    _dat[1]=(data>>8)&0xFF;
    _dat[2]=(data)&0xFF;

    if (i2cTransfer(Dev->I2cDevAddr, _dat, 3, NULL, 0))
        {
            return VL53L0X_ERROR_NONE;
        }

    return VL53L0X_ERROR_CONTROL_INTERFACE;
}
// ============================================================================================
VL53L0X_Error VL53L0X_WrDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t data)

{
    _dat[0]=index;
    _dat[1]=(data>>24)&0xFF;
    _dat[2]=(data>>16)&0xFF;
    _dat[3]=(data>>8)&0xFF;
    _dat[4]=data&0xFF;

    if (i2cTransfer(Dev->I2cDevAddr, _dat, 5, NULL, 0))
        {
            return VL53L0X_ERROR_NONE;
        }

    return VL53L0X_ERROR_CONTROL_INTERFACE;
}
// ============================================================================================
VL53L0X_Error VL53L0X_UpdateByte(VL53L0X_DEV Dev, uint8_t index, uint8_t AndData, uint8_t OrData)

{
    uint8_t data;

    if (VL53L0X_ERROR_NONE != VL53L0X_RdByte(Dev, index, &data))
        {
            return VL53L0X_ERROR_CONTROL_INTERFACE;
        }

    data = (data & AndData) | OrData;

    if (VL53L0X_ERROR_NONE != VL53L0X_WrByte(Dev, index, data))
        {
            return VL53L0X_ERROR_CONTROL_INTERFACE;
        }

    return VL53L0X_ERROR_NONE;
}
// ============================================================================================
VL53L0X_Error VL53L0X_RdByte(VL53L0X_DEV Dev, uint8_t index, uint8_t *data)

{
    if (i2cTransfer((Dev->I2cDevAddr), (uint8_t [])
    {
        index
    },1,data,1))
    return VL53L0X_ERROR_NONE;

    return  VL53L0X_ERROR_CONTROL_INTERFACE;
}
// ============================================================================================
VL53L0X_Error VL53L0X_RdWord(VL53L0X_DEV Dev, uint8_t index, uint16_t *data)

{
    if (i2cTransfer((Dev->I2cDevAddr), (uint8_t [])
    {
        index
    },1,_dat,2))
    {
        *data=(_dat[0]<<8)|_dat[1];
        return VL53L0X_ERROR_NONE;
    }

    return  VL53L0X_ERROR_CONTROL_INTERFACE;
}
// ============================================================================================
VL53L0X_Error  VL53L0X_RdDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t *data)

{
    if (i2cTransfer((Dev->I2cDevAddr), (uint8_t [])
    {
        index
    },1,_dat,4))
    {
        *data=(_dat[0]<<24)|(_dat[1]<<16)|(_dat[2]<<8)|_dat[3];
        return VL53L0X_ERROR_NONE;
    }

    return  VL53L0X_ERROR_CONTROL_INTERFACE;
}
// ============================================================================================
VL53L0X_Error VL53L0X_PollingDelay(VL53L0X_DEV Dev)

{
    vTaskDelay(MILLIS_TO_TICKS(1));
    return VL53L0X_ERROR_NONE;
}
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Publicly available routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
uint32_t VLDISTgetChannel(enum VLDIST_CHANNEL_ENUM c)

/* Get latest reading from channel */

{
    VL53L0X_RangingMeasurementData_t    RangingMeasurementData;

    if (!_d[c].available)
        {
            return 0;
        }

    RangingMeasurementData.RangeMilliMeter=0;

    VL53L0X_GetRangingMeasurementData(&_d[c].vlDev, &RangingMeasurementData);

    return RangingMeasurementData.RangeMilliMeter;
}
// ============================================================================================
void VLDISTCheckOutput(void)

/* Provide output and check for eStop */

{
    uint32_t d;
    for (enum VLDIST_CHANNEL_ENUM c=0; c<VLDISTNONE; c++)
        {
            d=VLDISTgetChannel(c);
            if (d<MINDIST)
                motorSeteStop(c==VLDIST_FRONT?MOTOR_ESTOP_FORWARDS:MOTOR_ESTOP_BACKWARDS);
            else
                motorCleareStop(c==VLDIST_FRONT?MOTOR_ESTOP_FORWARDS:MOTOR_ESTOP_BACKWARDS);

            MLUpdateAvailable(SENSOR_TYPE_CAR_STATUS);
            LmsSendUsRangeSensor(c, d);
        }
}

// ============================================================================================
BOOL VLDISTInit(uint32_t intervalSet)

{
    uint8_t VhvSettings;
    uint8_t PhaseCal;
    uint32_t refSpadCount;
    uint8_t isApertureSpads;
    BOOL AtLeastOneGood=FALSE;
    VL53L0X_Error Status;

    /* Bring each individual VL out of init and allocate it an address. */
    /* This is done using the pullup resistor of the adaptor board because */
    /* The chip Vdd (3v3) is higher than the Vdd of the board (2v8) */

    /* Firstly, put all chips into reset */
    for (uint32_t i=0; i<NUM_VLS; i++)
        {

            Chip_SCU_PinMuxSet(GETPORT(_vl[i].pinConfig), GETPIN(_vl[i].pinConfig), GETFUNC(_vl[i].pinConfig));
            Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, GETGPIOPORT(_vl[i].pinConfig), GETGPIOPIN(_vl[i].pinConfig));
            Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, GETGPIOPORT(_vl[i].pinConfig),
                                      GETGPIOPIN(_vl[i].pinConfig));
        }

    /* Now bring them out one at a time and initialise them */
    vTaskDelay(2);
    for (uint32_t i=0; i<NUM_VLS; i++)
        {
            /* This will cause the Inhibit pin to go high due to the pullup */
            Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, GETGPIOPORT(_vl[i].pinConfig),
                                     GETGPIOPIN(_vl[i].pinConfig));
            vTaskDelay(2);

            _d[i].vlDev.I2cDevAddr=STARTING_ID;
            _d[i].vlDev.comms_speed_khz=VL_COMMS_SPEED_KHZ;
            VL53L0X_SetDeviceAddress (&_d[i].vlDev, (_vl[i].i2cID)<<1);
            _d[i].vlDev.I2cDevAddr=_vl[i].i2cID;
            _d[i].available=FALSE;
            /* Now perform the initialisation, only marking the device as available if it passes all steps */
            Status=VL53L0X_ERROR_NONE;

            Status|=VL53L0X_DataInit(&_d[i].vlDev);
            Status|=VL53L0X_GetDeviceInfo(&_d[i].vlDev, &_d[i].deviceInfo);
            Status|=VL53L0X_StaticInit(&_d[i].vlDev);
            Status|=VL53L0X_PerformRefCalibration(&_d[i].vlDev, &VhvSettings, &PhaseCal);
            Status|=VL53L0X_SetMeasurementTimingBudgetMicroSeconds (&_d[i].vlDev,
                    intervalSet*1000-COMPUTE_OVERHEAD );
            Status|=VL53L0X_SetInterMeasurementPeriodMilliSeconds(&_d[i].vlDev, intervalSet);
            Status|=VL53L0X_PerformRefSpadManagement(&_d[i].vlDev, &refSpadCount, &isApertureSpads);
            Status|=VL53L0X_SetDeviceMode(&_d[i].vlDev, VL53L0X_DEVICEMODE_CONTINUOUS_TIMED_RANGING);
            Status|=VL53L0X_StartMeasurement(&_d[i].vlDev);

            if (Status==VL53L0X_ERROR_NONE)
                {
                    _d[i].available=TRUE;
                    AtLeastOneGood=TRUE;
                }
        }

    /* Yup, all is good .... we have sensors */
    return AtLeastOneGood;
}
// ============================================================================================
