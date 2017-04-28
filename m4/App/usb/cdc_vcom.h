/*
 * @brief Programming API used with Virtual Communication port
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#ifndef __CDC_VCOM_H_
#define __CDC_VCOM_H_

#include "app_usbd_cfg.h"
#include "config.h"
#include "terminal.h"

BOOL vcom_connected(void);

/**
 * @brief   Virtual com port init routine
 * @param   hUsb        : Handle to USBD stack instance
 * @param   pDesc       : Pointer to configuration descriptor
 * @param   pUsbParam   : Pointer USB param structure returned by previous init call
 * @return  Always returns LPC_OK.
 */
ErrorCode_t vcom_init(USBD_HANDLE_T hUsb, USB_CORE_DESCS_T *pDesc, USBD_API_INIT_PARAM_T *pUsbParam,
                      EVENT_CB(*cb_set));

/**
 * @brief   Virtual com port write routine
 * @param   pBuf    : Pointer to buffer to be written
 * @param   buf_len : Length of the buffer passed
 * @return  Number of bytes written
 */
uint32_t vcom_write (uint8_t *pBuf, uint32_t buf_len);
void USBInit(EVENT_CB(*cb_set));
void vcom_flush(void);
BOOL vcomDataPending(void);
uint8_t vcomGetRx(void);
#endif /* __CDC_VCOM_H_ */
