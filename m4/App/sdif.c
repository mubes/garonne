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
 * SD Card Interface module
 * ========================
 *
 * This module provides the glue between the SD Interface and the fatfs subsystem.
 *
 */

#include "config.h"

#ifdef INCLUDE_SDMMC
#include <string.h>
#include "diskio.h"
#include "rtc.h"
#include "sdif.h"
#include "ff.h"
#include "gio.h"

struct
{
    FATFS Fatfs;    /* File system object */
    FIL Fil;    /* File object */
    mci_card_struct c;
    SemaphoreHandle_t s;
} _sdc;

static volatile DSTATUS Stat = STA_NOINIT;  /* Low level Disk Status */

#define SDCMAXWAIT (1000)               // Max time to wait for a semaphore response
#define DISK_BLOCKSIZE (4UL * 1024)     // Disks always have a 4K blocksize, well for us, anyway
//#define CD_PRESENT                    // Define if you have the CD pin
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Internal routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Glue for FATTS routines
// ============================================================================================
static int _cardReadyWait(mci_card_struct *hCrd, int tout)

{
    uint32_t expireTime=xTaskGetTickCount()+MILLIS_TO_TICKS(tout);
    while ((Chip_SDMMC_GetState(LPC_SDMMC)==-1) && (xTaskGetTickCount()<expireTime))
        vTaskDelay(1);

    return Chip_SDMMC_GetState(LPC_SDMMC) != -1;
}
// ============================================================================================
static int _cardGetState(mci_card_struct *hCrd, uint8_t *buff)
{
    int state;
    state = Chip_SDMMC_GetState(LPC_SDMMC);
    if (state == -1) return 0;
    memcpy(buff, &state, sizeof(int));
    return 1;
}
// ============================================================================================
/* Initialize Disk Drive */
DSTATUS disk_initialize(BYTE drv)
{
    if (drv)
        {
            return STA_NOINIT;              /* Supports only single drive */
        }
    /*  if (Stat & STA_NODISK) return Stat; *//* No card in the socket */

    if (Stat != STA_NOINIT)
        {
            return Stat;                    /* card is already enumerated */

        }

#if !_FS_READONLY
    rtc_initialize();
#endif

    /* Reset */
    Stat = STA_NOINIT;

#ifdef CD_PRESENT
    while (Chip_SDIF_CardNDetect(LPC_SDMMC)) {} /* Wait for card to be inserted */
#endif

    /* Enumerate the card once detected. Note this function may block for a little while. */
    if (!Chip_SDMMC_Acquire(LPC_SDMMC, &_sdc.c))
        {
            return Stat;
        }

    Stat &= ~STA_NOINIT;
    return Stat;
}
// ============================================================================================
DRESULT disk_ioctl(BYTE drv, BYTE ctrl, void *buff)

/* Disk Drive miscellaneous Functions */

{
    DRESULT res;
    BYTE *ptr = buff;

    if (drv)
        {
            return RES_PARERR;
        }
    if (Stat & STA_NOINIT)
        {
            return RES_NOTRDY;
        }

    res = RES_ERROR;

    switch (ctrl)
        {
            case CTRL_SYNC: /* Make sure that no pending write process */
                if (_cardReadyWait(&_sdc.c, 50))
                    {
                        res = RES_OK;
                    }
                break;

            case GET_SECTOR_COUNT:  /* Get number of sectors on the disk (DWORD) */
                *(DWORD *) buff = _sdc.c.card_info.blocknr;
                res = RES_OK;
                break;

            case GET_SECTOR_SIZE:   /* Get R/W sector size (WORD) */
                *(WORD *) buff = _sdc.c.card_info.block_len;
                res = RES_OK;
                break;

            case GET_BLOCK_SIZE:/* Get erase block size in unit of sector (DWORD) */
                *(DWORD *) buff = DISK_BLOCKSIZE;
                res = RES_OK;
                break;

            case MMC_GET_TYPE:      /* Get card type flags (1 byte) */
                *ptr = _sdc.c.card_info.card_type;
                res = RES_OK;
                break;

            case MMC_GET_CSD:       /* Receive CSD as a data block (16 bytes) */
                *((uint32_t *) buff + 0) = _sdc.c.card_info.csd[0];
                *((uint32_t *) buff + 1) = _sdc.c.card_info.csd[1];
                *((uint32_t *) buff + 2) = _sdc.c.card_info.csd[2];
                *((uint32_t *) buff + 3) = _sdc.c.card_info.csd[3];
                res = RES_OK;
                break;

            case MMC_GET_CID:       /* Receive CID as a data block (16 bytes) */
                *((uint32_t *) buff + 0) = _sdc.c.card_info.cid[0];
                *((uint32_t *) buff + 1) = _sdc.c.card_info.cid[1];
                *((uint32_t *) buff + 2) = _sdc.c.card_info.cid[2];
                *((uint32_t *) buff + 3) = _sdc.c.card_info.cid[3];
                res = RES_OK;
                break;

            case MMC_GET_SDSTAT:/* Receive SD status as a data block (64 bytes) */
                if (_cardGetState(&_sdc.c, (uint8_t *) buff))
                    {
                        res = RES_OK;
                    }
                break;

            default:
                res = RES_PARERR;
                break;
        }

    return res;
}
// ============================================================================================
DRESULT disk_read(BYTE drv, BYTE *buff, DWORD sector, BYTE count)

/* Read Sector(s) */

{
    if (drv || !count)
        {
            return RES_PARERR;
        }
    if (Stat & STA_NOINIT)
        {
            return RES_NOTRDY;
        }


    GIOdebugLedSet(SD_RD_ACCESS_LED);
    if (Chip_SDMMC_ReadBlocks(LPC_SDMMC, buff, sector, count))
        {
            GIOdebugLedClear(SD_RD_ACCESS_LED);
            return RES_OK;
        }
    GIOdebugLedClear(SD_RD_ACCESS_LED);
    return RES_ERROR;
}
// ============================================================================================
DSTATUS disk_status(BYTE drv)

/* Get Disk Status */

{
    if (drv)
        {
            return STA_NOINIT;  /* Supports only single drive */

        }
    return Stat;
}
// ============================================================================================
DRESULT disk_write(BYTE drv, const BYTE *buff, DWORD sector, BYTE count)

/* Write Sector(s) */

{

    if (drv || !count)
        {
            return RES_PARERR;
        }
    if (Stat & STA_NOINIT)
        {
            return RES_NOTRDY;
        }

    GIOdebugLedSet(SD_WR_ACCESS_LED);
    if ( Chip_SDMMC_WriteBlocks(LPC_SDMMC, (void *) buff, sector, count))
        {
            GIOdebugLedClear(SD_WR_ACCESS_LED);
            return RES_OK;
        }

    GIOdebugLedClear(SD_WR_ACCESS_LED);
    return RES_ERROR;
}
// ============================================================================================
// Glue for ROM based MMC routines
// ============================================================================================
static void _waitms(uint32_t time)

/* Delay callback for timed SDIF/SDMMC functions */

{
    vTaskDelay(MILLIS_TO_TICKS(time));
}
// ============================================================================================
static void _setup_wakeup(void *bits)
{
    uint32_t bit_mask = *((uint32_t *)bits);
    NVIC_ClearPendingIRQ(SDIO_IRQn);
    Chip_SDIF_SetIntMask(LPC_SDMMC, bit_mask);
    ASSERT(xSemaphoreTake(_sdc.s,0)==0);
    NVIC_EnableIRQ(SDIO_IRQn);
}
// ============================================================================================
static uint32_t _irq_driven_wait(void)

{
    uint32_t status=-1;

    if (xSemaphoreTake(_sdc.s,MILLIS_TO_TICKS(SDCMAXWAIT)))
        {
            /* Get status and clear interrupts */
            status = Chip_SDIF_GetIntStatus(LPC_SDMMC);
            Chip_SDIF_ClrIntStatus(LPC_SDMMC, status);
            Chip_SDIF_SetIntMask(LPC_SDMMC, 0);
        }
    else
        {
            ASSERT(FALSE);
        }

    return status;
}
// ============================================================================================
void SDIO_IRQHandler(void)

{
    BaseType_t higherPriorityTaskWoken=FALSE;
    NVIC_DisableIRQ(SDIO_IRQn);
    xSemaphoreGiveFromISR(_sdc.s, &higherPriorityTaskWoken);
    portEND_SWITCHING_ISR(higherPriorityTaskWoken);
}
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Externally available routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
void sdifMount(void)

{
    f_mount(&_sdc.Fatfs,"",0);
}
// ============================================================================================
void sdifSetup(void)

{
    memset(&_sdc.c, 0, sizeof(_sdc.c));
    _sdc.c.card_info.evsetup_cb = _setup_wakeup;
    _sdc.c.card_info.waitfunc_cb = _irq_driven_wait;
    _sdc.c.card_info.msdelay_func = _waitms;
    _sdc.s = xSemaphoreCreateBinary();
    xSemaphoreTake(_sdc.s,0);

    Chip_SCU_PinMuxSet(GETPORT(SDMMC_SDIO_D0), GETPIN(SDMMC_SDIO_D0), GETFUNC(SDMMC_SDIO_D0));
    Chip_SCU_PinMuxSet(GETPORT(SDMMC_SDIO_D1), GETPIN(SDMMC_SDIO_D1), GETFUNC(SDMMC_SDIO_D1));
    Chip_SCU_PinMuxSet(GETPORT(SDMMC_SDIO_D2), GETPIN(SDMMC_SDIO_D2), GETFUNC(SDMMC_SDIO_D2));
    Chip_SCU_PinMuxSet(GETPORT(SDMMC_SDIO_D3), GETPIN(SDMMC_SDIO_D3), GETFUNC(SDMMC_SDIO_D3));
    Chip_SCU_ClockPinMuxSet(SDMMC_CLKPIN,SDMMC_CLKCFG);

    Chip_SCU_PinMuxSet(GETPORT(SDMMC_SDIO_CMD), GETPIN(SDMMC_SDIO_CMD), GETFUNC(SDMMC_SDIO_CMD));

    Chip_SDIF_Init(LPC_SDMMC);
    *((uint32_t *)0x40086D80)=0x9|(0xd<<8);

    /* Enable SD/MMC Interrupt */
    NVIC_SetPriority(SDIO_IRQn, SDIO_INTPRIORITY);
    NVIC_EnableIRQ(SDIO_IRQn);
}
// ============================================================================================
#endif
