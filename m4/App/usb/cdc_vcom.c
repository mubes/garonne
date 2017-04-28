/*
 * @brief Virtual Comm port call back routines
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
 * All rights reserved.
 *
 * Heavily extended and modified to integrate with the serport arrangements of Daves code.
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
#include <string.h>
#include "app_usbd_cfg.h"
#include "config.h"
#include "cdc_vcom.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#define VCOM_RX_BUF_SZ (USB_FS_MAX_BULK_PACKET)
#define VCOM_CONNECTED      (1<<0)

/**
 * Structure containing Virtual Comm port control data
 */
typedef struct VCOM_DATA
{
    volatile uint16_t tx_flags;
    USBD_HANDLE_T hUsb;
    USBD_HANDLE_T hCdc;
    uint8_t *rx_buff;       /* Buffer for received data */
    uint32_t rx_count;      /* Received count in data buffer */
    uint32_t rx_read;       /* Current read position in data buffer */
    EVENT_CB(*cb);          /* Callback to let folks know something happened */
} VCOM_DATA_T;


/* Part of WORKAROUND for artf42016. */
static USB_EP_HANDLER_T g_defaultCdcHdlr;

/* Endpoint 0 patch that prevents nested NAK event processing */
static uint32_t g_ep0RxBusy = 0;/* flag indicating whether EP0 OUT/RX buffer is busy. */
static USB_EP_HANDLER_T g_Ep0BaseHdlr;  /* variable to store the pointer to base EP0 handler */

static SemaphoreHandle_t _txSem=NULL;
static VCOM_DATA_T g_vCOM;
static USBD_HANDLE_T g_hUsb;                    /* Handle to USB instance */
const  USBD_API_T *g_pUsbApi;                   /* Pointer to ROM table */

/*****************************************************************************
 * Private functions
 ****************************************************************************/
ErrorCode_t EP0_patch(USBD_HANDLE_T hUsb, void *data, uint32_t event)

/* EP0_patch part of WORKAROUND for artf45032. */

{
    switch (event)
        {
            case USB_EVT_OUT_NAK:
                if (g_ep0RxBusy)
                    {
                        /* we already queued the buffer so ignore this NAK event. */
                        return LPC_OK;
                    }
                else
                    {
                        /* Mark EP0_RX buffer as busy and allow base handler to queue the buffer. */
                        g_ep0RxBusy = 1;
                    }
                break;

            case USB_EVT_SETUP:   /* reset the flag when new setup sequence starts */
            case USB_EVT_OUT:
                /* we received the packet so clear the flag. */
                g_ep0RxBusy = 0;
                break;
        }
    return g_Ep0BaseHdlr(hUsb, data, event);
}
// ============================================================================================
void USB_IRQHandler(void)

{
    USBD_API->hw->ISR(g_hUsb);
}
// ============================================================================================
static ErrorCode_t VCOM_bulk_in_hdlr(USBD_HANDLE_T hUsb, void *data, uint32_t event)

/* VCOM bulk EP_IN endpoint handler */

{
    portBASE_TYPE xTaskWoken = pdFALSE;

    if (event == USB_EVT_IN)
        xSemaphoreGiveFromISR(_txSem, &xTaskWoken);
    portEND_SWITCHING_ISR(xTaskWoken);
    return LPC_OK;
}
// ============================================================================================
static ErrorCode_t VCOM_bulk_out_hdlr(USBD_HANDLE_T hUsb, void *data, uint32_t event)

/* VCOM bulk EP_OUT endpoint handler */

{
    switch (event)
        {
            case USB_EVT_OUT:
                g_vCOM.rx_count = USBD_API->hw->ReadEP(hUsb, USB_CDC_OUT_EP, g_vCOM.rx_buff);
                g_vCOM.rx_read = 0;
                CALLBACK(g_vCOM.cb,SERPORT_EV_DATARX);
                break;

            case USB_EVT_OUT_NAK:
                if (g_vCOM.rx_count==g_vCOM.rx_read)
                    /* queue free buffer for RX */
                    USBD_API->hw->ReadReqEP(hUsb, USB_CDC_OUT_EP, g_vCOM.rx_buff, VCOM_RX_BUF_SZ);
                break;

            default:
                break;
        }
    return LPC_OK;
}
// ============================================================================================
static ErrorCode_t VCOM_SetLineCode(USBD_HANDLE_T hCDC, CDC_LINE_CODING *line_coding)

/* Set line coding call back routine */

{
    return LPC_OK;
}
// ============================================================================================
static ErrorCode_t VCOMSetCtrlLineState(USBD_HANDLE_T hCDC, uint16_t state)

{
    /* Bit 0 is DTR */
    if (state&1)
        {
            g_vCOM.tx_flags |= VCOM_CONNECTED;
            CALLBACK(g_vCOM.cb,SERPORT_EV_CONNECT);
        }
    else
        {
            g_vCOM.tx_flags &= ~VCOM_CONNECTED;
            CALLBACK(g_vCOM.cb,SERPORT_EV_CLOSE);
        }
    return LPC_OK;
}
// ============================================================================================
static ErrorCode_t CDC_ep0_override_hdlr(USBD_HANDLE_T hUsb, void *data, uint32_t event)

/* CDC EP0_patch part of WORKAROUND for artf42016. */

{
    USB_CORE_CTRL_T *pCtrl = (USB_CORE_CTRL_T *) hUsb;
    USB_CDC_CTRL_T *pCdcCtrl = (USB_CDC_CTRL_T *) data;
    USB_CDC0_CTRL_T *pCdc0Ctrl = (USB_CDC0_CTRL_T *) data;
    uint8_t cif_num, dif_num;
    CIC_SetRequest_t setReq;
    ErrorCode_t ret = ERR_USBD_UNHANDLED;

    if ( (event == USB_EVT_OUT) &&
            (pCtrl->SetupPacket.bmRequestType.BM.Type == REQUEST_CLASS) &&
            (pCtrl->SetupPacket.bmRequestType.BM.Recipient == REQUEST_TO_INTERFACE) )
        {

            /* Check which CDC control structure to use. If epin_num doesn't have BIT7 set then we are
               at wrong offset so use the old CDC control structure. BIT7 is set for all EP_IN endpoints.
            */
            if ((pCdcCtrl->epin_num & 0x80) == 0)
                {
                    cif_num = pCdc0Ctrl->cif_num;
                    dif_num = pCdc0Ctrl->dif_num;
                    setReq = pCdc0Ctrl->CIC_SetRequest;
                }
            else
                {
                    cif_num = pCdcCtrl->cif_num;
                    dif_num = pCdcCtrl->dif_num;
                    setReq = pCdcCtrl->CIC_SetRequest;
                }
            /* is the request target is our interfaces */
            if (((pCtrl->SetupPacket.wIndex.WB.L == cif_num)  ||
                    (pCtrl->SetupPacket.wIndex.WB.L == dif_num)) )
                {

                    pCtrl->EP0Data.pData -= pCtrl->SetupPacket.wLength;
                    ret = setReq(pCdcCtrl, &pCtrl->SetupPacket, &pCtrl->EP0Data.pData,
                                 pCtrl->SetupPacket.wLength);
                    if ( ret == LPC_OK)
                        {
                            /* send Acknowledge */
                            USBD_API->core->StatusInStage(pCtrl);
                        }
                }

        }
    else
        {
            ret = g_defaultCdcHdlr(hUsb, data, event);
        }
    return ret;
}
// ============================================================================================
void updateStringDescriptorSN(uint8_t *p)

{
    uint32_t index=
        3;                     /* Magic number - the Serial number is stored in the third string */
    const uint32_t *n=ConfigGetSerialNumber();
    const uint32_t *q=n;

    /* Cycle through until we get to third descriptor (Serial Number) */
    while (index--)
        p+=*p;

    /* Now move on 2 bytes to start of the serial number */
    p+=2;

    /* OK, now write the serial number into the USB String Descriptors */
    do
        {
            index=0;
            uint32_t c=*n++;

            do
                {
                    *p++="0123456789abcdef"[(c>>28) & 0x0F];
                    *p++=0;
                    c=c<<4;
                }
            while (++index<8);
            *p++='-';
            *p++=0;
        }
    while (n-q<4);

    /* Finally write the config variant */
    *p++='0';
    *p++=0;
}
// ============================================================================================
USB_INTERFACE_DESCRIPTOR *find_IntfDesc(const uint8_t *pDesc, uint32_t intfClass)

/* Find the address of interface descriptor for given class type. */

{
    USB_COMMON_DESCRIPTOR *pD;
    USB_INTERFACE_DESCRIPTOR *pIntfDesc = 0;
    uint32_t next_desc_adr;

    pD = (USB_COMMON_DESCRIPTOR *) pDesc;
    next_desc_adr = (uint32_t) pDesc;

    while (pD->bLength)
        {
            /* is it interface descriptor */
            if (pD->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE)
                {

                    pIntfDesc = (USB_INTERFACE_DESCRIPTOR *) pD;
                    /* did we find the right interface descriptor */
                    if (pIntfDesc->bInterfaceClass == intfClass)
                        {
                            break;
                        }
                }
            pIntfDesc = 0;
            next_desc_adr = (uint32_t) pD + pD->bLength;
            pD = (USB_COMMON_DESCRIPTOR *) next_desc_adr;
        }

    return pIntfDesc;
}
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Externally available routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
ErrorCode_t vcom_init(USBD_HANDLE_T hUsb, USB_CORE_DESCS_T *pDesc, USBD_API_INIT_PARAM_T *pUsbParam,
                      EVENT_CB(*cb_set))

/* Virtual com port init routine */

{
    USBD_CDC_INIT_PARAM_T cdc_param;
    ErrorCode_t ret = LPC_OK;
    uint32_t ep_indx;
    USB_CORE_CTRL_T *pCtrl = (USB_CORE_CTRL_T *) hUsb;

    g_vCOM.cb=cb_set;
    g_vCOM.hUsb = hUsb;
    memset((void *) &cdc_param, 0, sizeof(USBD_CDC_INIT_PARAM_T));
    cdc_param.mem_base = pUsbParam->mem_base;
    cdc_param.mem_size = pUsbParam->mem_size;
    cdc_param.cif_intf_desc = (uint8_t *) find_IntfDesc(pDesc->high_speed_desc,
                              CDC_COMMUNICATION_INTERFACE_CLASS);
    cdc_param.dif_intf_desc = (uint8_t *) find_IntfDesc(pDesc->high_speed_desc,
                              CDC_DATA_INTERFACE_CLASS);
    cdc_param.SetLineCode = VCOM_SetLineCode;
    cdc_param.SetCtrlLineState = VCOMSetCtrlLineState;

    ret = USBD_API->cdc->init(hUsb, &cdc_param, &g_vCOM.hCdc);
    if (ret != LPC_OK)
        {
            return ret;
        }

    /*    WORKAROUND for artf42016 ROM driver BUG:
    The default CDC class handler in initial ROM (REV A silicon) was not
    sending proper handshake after processing SET_REQUEST messages targeted
    to CDC interfaces. The workaround will send the proper handshake to host.
    Due to this bug some terminal applications such as Putty have problem
    establishing connection.
    */
    g_defaultCdcHdlr = pCtrl->ep0_hdlr_cb[pCtrl->num_ep0_hdlrs - 1];
    /* store the default CDC handler and replace it with ours */
    pCtrl->ep0_hdlr_cb[pCtrl->num_ep0_hdlrs - 1] = CDC_ep0_override_hdlr;

    /* allocate transfer buffers */
    if (cdc_param.mem_size < VCOM_RX_BUF_SZ)
        {
            return ERR_FAILED;
        }

    g_vCOM.rx_buff = (uint8_t *) cdc_param.mem_base;
    cdc_param.mem_base += VCOM_RX_BUF_SZ;
    cdc_param.mem_size -= VCOM_RX_BUF_SZ;

    /* register endpoint interrupt handler */
    ep_indx = (((USB_CDC_IN_EP & 0x0F) << 1) + 1);
    ret = USBD_API->core->RegisterEpHandler(hUsb, ep_indx, VCOM_bulk_in_hdlr, &g_vCOM);
    if (ret != LPC_OK)
        {
            return ret;
        }

    /* register endpoint interrupt handler */
    ep_indx = ((USB_CDC_OUT_EP & 0x0F) << 1);
    ret = USBD_API->core->RegisterEpHandler(hUsb, ep_indx, VCOM_bulk_out_hdlr, &g_vCOM);
    if (ret != LPC_OK)
        {
            return ret;
        }

    /* update mem_base and size variables for cascading calls. */
    pUsbParam->mem_base = cdc_param.mem_base;
    pUsbParam->mem_size = cdc_param.mem_size;

    return ret;
}
// ============================================================================================
static ErrorCode_t USBReset(USBD_HANDLE_T hUsb)

{
    portBASE_TYPE xTaskWoken = pdFALSE;
    g_vCOM.tx_flags=0;
    xSemaphoreGiveFromISR(_txSem,&xTaskWoken);
    CALLBACK(g_vCOM.cb,SERPORT_EV_CLOSE);
    portEND_SWITCHING_ISR(xTaskWoken);
    return LPC_OK;
}
// ============================================================================================
static ErrorCode_t USBSuspend(USBD_HANDLE_T hUsb)

{
    return LPC_OK;
}
// ============================================================================================
uint32_t vcom_write(uint8_t *pBuf, uint32_t len)

/* Virtual com port write routine */

{
    uint32_t ret = len;
    uint32_t t;

    while ((ret) && (g_vCOM.tx_flags&VCOM_CONNECTED) && (xSemaphoreTake(_txSem, MILLIS_TO_TICKS(100))))
        {
            t = USBD_API->hw->WriteEP(g_vCOM.hUsb, USB_CDC_IN_EP, pBuf, ret);
            pBuf += t;
            ret -= t;
        }

    return len-ret;
}
// ============================================================================================
BOOL vcom_connected(void)

{
    return (g_vCOM.tx_flags&VCOM_CONNECTED)!=0;
}
// ============================================================================================
BOOL vcomDataPending(void)

{
    return (g_vCOM.rx_read<g_vCOM.rx_count);
}
// ============================================================================================
uint8_t vcomGetRx(void)

{
    if (vcomDataPending())
        return g_vCOM.rx_buff[g_vCOM.rx_read++];

    return 0;
}
// ============================================================================================
void vcom_flush(void)

/* Flush all data from rx and tx queue */

{
    g_vCOM.rx_count=g_vCOM.rx_read=0;
}
// ============================================================================================
void USBInit(EVENT_CB(*cb_set))

{
    USBD_API_INIT_PARAM_T usb_param;
    USB_CORE_DESCS_T desc;
    USB_CORE_CTRL_T *pCtrl;

    /* enable clocks and pinmux */
    Chip_USB0_Init();

    /* Init USB API structure */
    g_pUsbApi = (const USBD_API_T *) LPC_ROM_API->usbdApiBase;

    /* initialize call back structures */
    memset((void *) &usb_param, 0, sizeof(USBD_API_INIT_PARAM_T));
    usb_param.usb_reg_base = LPC_USB_BASE;
    usb_param.max_num_ep = 4;
    usb_param.mem_base = USB_STACK_MEM_BASE;
    usb_param.mem_size = USB_STACK_MEM_SIZE;
    usb_param.USB_Reset_Event=USBReset;
    usb_param.USB_Suspend_Event=USBSuspend;

    /* Set the USB descriptors */
    desc.device_desc = (uint8_t *) USB_DeviceDescriptor;
    updateStringDescriptorSN(&USB_StringDescriptor[0]);
    desc.string_desc = (uint8_t *) USB_StringDescriptor;

#ifdef USE_USB0
    desc.high_speed_desc = USB_HsConfigDescriptor;
    desc.full_speed_desc = USB_FsConfigDescriptor;
    desc.device_qualifier= (uint8_t *)USB_DeviceQualifier;
#else
    /* Note, to pass USBCV test full-speed only devices should have both
     * descriptor arrays point to same location and device_qualifier set
     * to 0.
     */
    desc.high_speed_desc = USB_FsConfigDescriptor;
    desc.full_speed_desc = USB_FsConfigDescriptor;
    desc.device_qualifier = 0;
#endif

    /* USB Initialization */
    if (USBD_API->hw->Init(&g_hUsb, &desc, &usb_param)!=LPC_OK) return;

    /*    WORKAROUND for artf45032 ROM driver BUG:
    Due to a race condition there is the chance that a second NAK event will
    occur before the default endpoint0 handler has completed its preparation
    of the DMA engine for the first NAK event. This can cause certain fields
    in the DMA descriptors to be in an invalid state when the USB controller
    reads them, thereby causing a hang.
    */
    pCtrl = (USB_CORE_CTRL_T *) g_hUsb;   /* convert the handle to control structure */
    g_Ep0BaseHdlr = pCtrl->ep_event_hdlr[0];/* retrieve the default EP0_OUT handler */
    pCtrl->ep_event_hdlr[0] = EP0_patch;/* set our patch routine as EP0_OUT handler */

    /* Init queue storage */
    vcom_flush();
    _txSem=xSemaphoreCreateBinary();
    xSemaphoreGive(_txSem);

    /* Init VCOM interface */
    if (vcom_init(g_hUsb, &desc, &usb_param,cb_set)!= LPC_OK) return;
    NVIC_SetPriority(LPC_USB_IRQ,USB_INTERRUPT_PRIORITY);
    /*  enable USB interrupts */
    NVIC_EnableIRQ(LPC_USB_IRQ);

    /* now connect */
    USBD_API->hw->Connect(g_hUsb, TRUE);
}
// ============================================================================================
