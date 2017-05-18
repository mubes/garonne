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
 * Serialiser & Deserialiser module for M4/M0 comms
 * ================================================
 */

#include "config.h"
#include "ipcHandler.h"
#include "serdes.h"

enum serdesState {SD_WAIT_HEADER, SD_WAIT_TARGET_H, SD_WAIT_TARGET_L, SD_WAIT_LEN, SD_DATA, SD_WAIT_CRC};

#define SD_HEADER (0xA5)

#define PROTOCOL_OVERHEAD_LENGTH  (6)
#define MAX_PACKET_LEN (IPC_QUEUE_LEN-PROTOCOL_OVERHEAD_LENGTH-1)

static struct
{
    enum ipc port;
    uint32_t target;
    uint32_t rxLen;
    uint32_t seq;
    uint8_t data[MAX_PACKET_LEN];
    uint8_t *dwp;
    uint8_t sum;
    enum serdesState s;
    EVENT_CB(*cb);
} _s;

// ============================================================================================
// ============================================================================================
// ============================================================================================
// Private routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
uint8_t _sum(uint8_t *d, uint32_t len)

{
    uint8_t s=0;
    while (len--) s+=*d++;
    return s;
}
// ============================================================================================
void _processPacket(void)

{
    if (_s.dwp!=_s.data)
    {
        /* This is a packet with length associated - check it's OK */
        uint8_t calcSum = _sum(_s.data,(_s.dwp-_s.data));
        if (calcSum!=_s.sum)
        {
            return;
        }
    }
    _s.cb(_s.target);
}
// ============================================================================================
void _protocolPump(uint8_t c)

{
    switch (_s.s)
    {
        case SD_WAIT_HEADER:
            if (SD_HEADER!=c)
            {
                break;
            }
            _s.s = SD_WAIT_TARGET_H;
            break;
            // --------------------------
        case SD_WAIT_TARGET_H:
        _s.target=c<<8;
        _s.s = SD_WAIT_TARGET_L;
        break;
        // --------------------------
        case SD_WAIT_TARGET_L:
        _s.target|=c;
        _s.s = SD_WAIT_LEN;
        break;
        // --------------------------
        case SD_WAIT_LEN:
            if (c>MAX_PACKET_LEN)
            {
                _s.s = SD_WAIT_HEADER;
                break;
            }
            _s.rxLen=c;
            _s.dwp=_s.data;
            if (c)
                {
                    _s.s = SD_DATA;
                }
            else
            {
                _s.s = SD_WAIT_HEADER;
                _processPacket();

            }
            break;
            // --------------------------
            case SD_DATA:
                *_s.dwp++=c;

                if (--_s.rxLen)
                {
                    break;
                }
                _s.s = SD_WAIT_CRC;
                break;

            // --------------------------
        case SD_WAIT_CRC:
            _s.sum = c;
            _s.s = SD_WAIT_HEADER;
            _processPacket();
            break;
            // --------------------------
    }
}
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Publicly available routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
uint32_t serdesTarget(enum ipc port)

{
    return _s.target;
}
// ============================================================================================
uint32_t serdesLen(enum ipc port)

{
    return _s.dwp-_s.data;
}
// ============================================================================================
uint8_t *serdesData(enum ipc port)

{
    return _s.data;
}
// ============================================================================================
void serdesReceive(enum ipc port)

{
    while (ipcDataPending(_s.port))
    {
        _protocolPump(ipcGetRx(_s.port));
    }
}
// ============================================================================================
BOOL serdesSend(uint32_t target, uint8_t len, uint8_t *data)

{
    uint8_t s=SD_HEADER;
    target|=MSG_SEQ(_s.seq++);
    uint8_t *targetTx=(uint8_t[]){(target>>8)&0xFF,target&0xFF};

    if (!ipcTxRoomCheck(_s.port,len+PROTOCOL_OVERHEAD_LENGTH))
    {
        /* There isn't room for this message .... just make sure the other end knows there's stuff waiting */
        ipcAlertFarEnd();
        return FALSE;
    }

    ipcTx(_s.port,&s,1);
    ipcTx(_s.port,targetTx,2);
    ipcTx(_s.port,&len,1);
    if (len)
    {
        ipcTx(_s.port, data, len);
        s =_sum(data,len);
        ipcTx(_s.port,&s,1);
    }
    ipcAlertFarEnd();

    return TRUE;
}

// ============================================================================================
void serdesInit(enum ipc port, EVENT_CB(*cb_set))

{
    _s.port = port;
    _s.cb=cb_set;
    _s.s=SD_WAIT_HEADER;
    ipcOpenPort(port);
}
// ============================================================================================
// ============================================================================================
// ============================================================================================
