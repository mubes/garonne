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
 * CAN I/O module
 * ==============
 *
 * This module is responsible for the setup and control of the CAN subsystem.
 *
 * This is only test code as the CAN is not in real use yet. It needs to be extended
 * to hook it into rest of the system once it is actually used in anger.
 *
 */

#include "config.h"

#ifdef INCLUDE_CAN
static struct
{
    CCAN_MSG_OBJ_T          send_obj;               /* Frame to be sent */
    CCAN_MSG_OBJ_T          recv_obj;               /* Frame received */
    uint32_t                can_stat;               /* Last status read from STAT register */
    BOOL                    newMsg[32];             /* Flag indicating new message is here */
    BOOL                    transmitting;           /* Am I curently transmitting? */
    xSemaphoreHandle        sendFrame;              /* CAN frame ready to be sent */
} _can;

// ============================================================================================
// ============================================================================================
// ============================================================================================
// Internal Routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
void CAN0_IRQHandler(void)

{
    uint32_t can_int;

    while ( (can_int = Chip_CCAN_GetIntID(CAN_IF)) != 0 )
        {
            if (can_int & CCAN_INT_STATUS)
                {
                    /* This is a status interrupt - deal with it */
                    _can.can_stat = Chip_CCAN_GetStatus(CAN_IF);

                    if (_can.can_stat & CCAN_STAT_TXOK)
                        {
                            _can.transmitting = FALSE;
                            Chip_CCAN_ClearStatus(CAN_IF, CCAN_STAT_TXOK);
                        }
                }
            else if ((CCAN_INT_MSG_NUM(can_int)>0) && (CCAN_INT_MSG_NUM(can_int) <= 0x20))
                {
                    /* This is a message update - mark it */
                    _can.newMsg[CCAN_INT_MSG_NUM(can_int)-1]=TRUE;
                }
        }
}
// ============================================================================================
static portTASK_FUNCTION( _canThread, pvParameters )

{
    while (1)
        {
            xSemaphoreTake(_can.sendFrame,portMAX_DELAY);
            Chip_CCAN_Send(CAN_IF, CAN_INTERFACE, FALSE, &_can.send_obj);
            Chip_CCAN_ClearStatus(CAN_IF, CCAN_STAT_TXOK);
        }
}
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Externally Available Routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
BOOL CANmsgUpdated(uint8_t msgID, CCAN_MSG_OBJ_T *c)

{
    BOOL t=_can.newMsg[msgID-1];
    _can.newMsg[msgID-1]=FALSE;

    if (t)
        Chip_CCAN_GetMsgObject(CAN_IF, CCAN_MSG_IF1, msgID, c);

    return t;
}
// ============================================================================================
BOOL CANupdated(void)

{
    if (!(_can.can_stat & CCAN_STAT_RXOK))
        return FALSE;

    Chip_CCAN_ClearStatus(CAN_IF, CCAN_STAT_RXOK);
    return TRUE;
}
// ============================================================================================
BOOL CANtransmit(uint32_t idSet, uint8_t lenSet, uint8_t *dataSet)

{
    if (_can.transmitting)
        return FALSE;

    _can.send_obj.id = idSet;
    _can.send_obj.dlc = lenSet;

    for (uint32_t i=0; i<lenSet; i++)
        _can.send_obj.data[i]=dataSet[i];

    _can.transmitting = TRUE;
    xSemaphoreTake(_can.sendFrame,0);
    xSemaphoreGive(_can.sendFrame);
    return TRUE;
}
// ============================================================================================
void CANSetup(void)

{
    /* Set CCAN peripheral clock under 100Mhz for working stable */
    /* Switch pins over to CAN */
    Chip_SCU_PinMuxSet(GETPORT(CAN_TD), GETPIN(CAN_TD), GETFUNC(CAN_TD));
    Chip_SCU_PinMuxSet(GETPORT(CAN_RD), GETPIN(CAN_RD), GETFUNC(CAN_RD));
    Chip_Clock_SetBaseClock(CLK_BASE_APB3, CLKIN_IDIVC, true, false);
    Chip_CCAN_Init(CAN_IF);
    Chip_CCAN_SetBitRate(CAN_IF, CAN_BITRATE);
    Chip_CCAN_EnableInt(CAN_IF, (CCAN_CTRL_IE | CCAN_CTRL_SIE | CCAN_CTRL_EIE));

    _can.sendFrame=xSemaphoreCreateBinary();

    xTaskCreate(_canThread, "CAN", 96, NULL,
                (tskIDLE_PRIORITY + 1UL), (xTaskHandle *) NULL);

    NVIC_EnableIRQ(C_CAN0_IRQn);
}
// ============================================================================================
#endif
