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
 * NineD module
 * ============
 *
 * Handle 9D sensor readings
 *
 */

#include "config.h"
#include "mainloop.h"
#include "nined.h"
#include "lms.h"

#define I2CMAXWAIT				MILLIS_TO_TICKS(100)	// Max time for a transfer before fail is assumed
#define MAXFAILS				(10)					// Maximum number of fails before outage declared

#define    GYRO_FULL_SCALE_250_DPS    0x00
#define    GYRO_FULL_SCALE_500_DPS    0x08
#define    GYRO_FULL_SCALE_1000_DPS   0x10
#define    GYRO_FULL_SCALE_2000_DPS   0x18

#define    ACC_FULL_SCALE_2_G        0x00
#define    ACC_FULL_SCALE_4_G        0x08
#define    ACC_FULL_SCALE_8_G        0x10
#define    ACC_FULL_SCALE_16_G       0x18

static struct {
	BOOL available;
	uint32_t fails;
    I2CM_XFER_T xfer;
    SemaphoreHandle_t s;
    TimerHandle_t interval;
    int16_t acc[3];
    int16_t gyr[3];
    uint16_t temp;
    int16_t mag[3];
} _i;

// ============================================================================================
void I2C1_IRQHandler(void)

{
    /* Call I2CM ISR function with the I2C device and transfer rec */
    Chip_I2CM_XferHandler(LPC_I2C1, &_i.xfer);
    if (_i.xfer.status!=I2CM_STATUS_BUSY)
    {
    	BaseType_t higherPriorityTaskWoken=FALSE;
    	xSemaphoreGiveFromISR(_i.s, &higherPriorityTaskWoken);
    	portEND_SWITCHING_ISR(higherPriorityTaskWoken);
    }
}
// ============================================================================================
BOOL _transfer(uint8_t *txBuff, uint32_t txLen, uint8_t *rxBuff, uint32_t rxLen)

{
	_i.xfer.slaveAddr=NINED_SENSOR_ID;
	_i.xfer.options=0;
	_i.xfer.status=0;
	_i.xfer.rxBuff=rxBuff;
	_i.xfer.txBuff=txBuff;
	_i.xfer.txSz=txLen;
	_i.xfer.rxSz=rxLen;

	Chip_I2CM_Xfer(LPC_I2C1, &_i.xfer);
	xSemaphoreTake(_i.s,I2CMAXWAIT);

	/* If this is all taking too long consider terminating the functionality */
	if (_i.xfer.status!=I2C_STATUS_DONE)
	{
		if (++_i.fails>=MAXFAILS)
			_i.available=FALSE;
	}
	else
	{
		_i.fails=0;
	}
	return _i.xfer.status==I2C_STATUS_DONE;
}
// ============================================================================================
BOOL _transferMAG(uint8_t *txBuff, uint32_t txLen, uint8_t *rxBuff, uint32_t rxLen)

{
	_i.xfer.slaveAddr=MAG_SENSOR_ID;
	_i.xfer.options=0;
	_i.xfer.status=0;
	_i.xfer.rxBuff=rxBuff;
	_i.xfer.txBuff=txBuff;
	_i.xfer.txSz=txLen;
	_i.xfer.rxSz=rxLen;

	Chip_I2CM_Xfer(LPC_I2C1, &_i.xfer);
	xSemaphoreTake(_i.s,portMAX_DELAY);
	return _i.xfer.status==I2C_STATUS_DONE;
}
// ============================================================================================
BOOL _grabSample(void)

{
    uint8_t rxBuff[14];

    /* Read Accel. and Gyro */
    if (_transfer((uint8_t[]){0x3B},1,rxBuff,14))
    {

        _i.acc[0]=(rxBuff[0]<<8)|rxBuff[1];
        _i.acc[1]=(rxBuff[2]<<8)|rxBuff[3];
        _i.acc[2]=(rxBuff[4]<<8)|rxBuff[5];
        _i.temp=((((rxBuff[6]<<8)|rxBuff[7])*100)/333)+2100;

        _i.gyr[0]=(rxBuff[8]<<8)|rxBuff[9];
        _i.gyr[1]=(rxBuff[10]<<8)|rxBuff[11];
        _i.gyr[2]=(rxBuff[12]<<8)|rxBuff[13];
    }
    else
    {
        return FALSE;
    }
    /* Read magnetometer and include ST2 */
    if (_transferMAG((uint8_t[]){3},1,rxBuff,7))
    {
        _i.mag[0]=((rxBuff[1]<<8)|rxBuff[0]);
        _i.mag[1]=((rxBuff[3]<<8)|rxBuff[2]);
        _i.mag[2]=((rxBuff[5]<<8)|rxBuff[4]);
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Publicly available routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
void ninedCheckOutput(void)

{
	if (!_i.available)
		return;
	_grabSample();
	LmsSend9DData(_i.acc, _i.gyr, _i.mag);
}
// ============================================================================================
int16_t ninedGyr(enum axis xyz)

{
    ASSERT(xyz<MAXAXES);
    return _i.gyr[xyz];
}
// ============================================================================================
int16_t ninedMag(enum axis xyz)

{
    ASSERT(xyz<MAXAXES);
    return _i.mag[xyz];
}
// ============================================================================================
int16_t ninedAcc(enum axis xyz)

{
    ASSERT(xyz<MAXAXES);
    return _i.acc[xyz];
}
// ============================================================================================
int16_t ninedTemp(void)

{
	return _i.temp;
}
// ============================================================================================
void ninedInit(void)

{
    Chip_SCU_PinMuxSet(GETPORT(NINED_SDA), GETPIN(NINED_SDA),GETFUNC(NINED_SDA));
    Chip_SCU_PinMuxSet(GETPORT(NINED_SCL), GETPIN(NINED_SCL),GETFUNC(NINED_SCL));

	_i.s = xSemaphoreCreateBinary();

    /* Initialize I2C */
    Chip_I2C_Init(I2C1);
    Chip_I2C_SetClockRate(I2C1, NINED_SENSOR_SPEED);
    Chip_I2C_SetMasterEventHandler(I2C1, Chip_I2C_EventHandler);
    NVIC_SetPriority(I2C1_IRQn,6);
    NVIC_EnableIRQ(I2C1_IRQn);

    /* Set DLPF to 10Hz BW */
    if (!_transfer((uint8_t[]){26,5},2,NULL,0))
    {
        DBG("Failed to initialise DLPF\n");
        return;
    }

    _i.available=TRUE;

    CHECK(TRUE,_transfer((uint8_t[]){27,10},2,NULL,0));
    CHECK(TRUE,_transfer((uint8_t[]){29,6},2,NULL,0));
    /* Set gyro to 250 DPS */
    CHECK(TRUE,_transfer((uint8_t[]){27,GYRO_FULL_SCALE_500_DPS},2,NULL,0));

    /* Set accelerometer to 2G */
    CHECK(TRUE,_transfer((uint8_t[]){28,ACC_FULL_SCALE_4_G},2,NULL,0));

    /* Set bypass mode for the magnetometer */
    CHECK(TRUE,_transfer((uint8_t[]){0x37,2},2,NULL,0));

    vTaskDelay(MAG_WAKEUP_TIME);
    /* Request first magnetometer continious measurement */
    CHECK(TRUE,_transferMAG((uint8_t[]){0x0A,0x12},2,NULL,0));
}
// ============================================================================================

