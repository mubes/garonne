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
 * IPCMsg module
 * =============
 *
 * This module takes input from the ipc link and dispatches it appropriately. It also maintains local storage
 * for data delivered to it from the ipc link.
 *
 */

#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include "config.h"
#include "serport.h"
#include "serdes.h"
#include "mainloop.h"

// ============================================================================================
static struct MSGpsnandatt p;     /* Position and Attitude from the M0 */
// ============================================================================================
EVENT_CB(_ipcDispatch)

{
    uint32_t target=serdesTarget(IPC_APP);

    switch(MSG_GET_CLASS(target))
    {
        // ======================================================
        case MSG_CLASS_MANAGEMENT:
            if (MSG_GET_MSG(target)==MSG_PING)
            {
                serdesSend(MSG_ENC_PONG,0,NULL);
                return;
            }
            // ------
            DBG("Unhandled Management Message" EOL);
            return;
        // ======================================================
        case MSG_CLASS_DATA:
            if (MSG_GET_MSG(target)==MSG_STRING)
            {
                /* String output to the display */
                serportTx(TERMINAL_PORT, serdesData(IPC_APP), serdesLen(IPC_APP));
                return;
            }
            // ------
            if (MSG_GET_MSG(target)==MSG_PSNANDATT)
            {
                /* Copy the updated data into the receive buffer */
                ASSERT(serdesLen(IPC_APP)==sizeof(p));
                memcpy(&p,serdesData(IPC_APP),sizeof(p));
                return;
            }
            // ------
            DBG("Unhandled Data Message" EOL);
            // ======================================================
        case MSG_CLASS_ACTION:
            // ------
            DBG("Unhandled Action Message" EOL);
            return;
    }
}
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Publicly available routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
struct MSGpsnandatt *IPCMsgGetpsanandatt(void)

{
    return &p;
}
// ============================================================================================
void IPCMsgSetup(void)
{
    serdesInit(IPC_M4, &_ipcDispatch);
}
// ============================================================================================
