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
 * LMS Msg module
 * ==============
 *
 * Send/receive messages over serial link. This module is responsible for the L2 packetisation
 * and depacketisation of messages and is implementation specific.
 *
 */

#include "config.h"
#include "serport.h"
#include "lms.h"
#include "lmsmsg.h"

enum rcStateEnum {RX_IDLE, RX_STX_L, RX_VERSION, RX_GETLEN, RX_GETPACKET, RX_GETCHECKSUMH, RX_GETCHECKSUML };

#define RXTIMEOUT_TIME (MILLIS_TO_TICKS(50))

static struct LmsStruct

{
    struct LmsMsgStats stats;

    uint32_t rxingLen;
    uint32_t PktLen;
    uint8_t *pktptr;
    uint16_t checksum;
    uint8_t pkt[LMS_MAX_PACKET_LEN];

    enum rcStateEnum rxState;
    TimerHandle_t rxTimeout;
} _l;
// ============================================================================================
// ============================================================================================
// ============================================================================================
uint16_t _crc16(const uint8_t *data_p, uint8_t length)

/* Calculates CCITT CRC (Polynomial 0x1021) */

{
    uint8_t x;
    uint16_t crc = 0xFFFF;

    while (length--)
        {
            x = crc >> 8 ^ *data_p++;
            x ^= x>>4;
            crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
        }
    return crc;
}
// ============================================================================================
void _rxtoCallback( TimerHandle_t pxTimer )

/* If a receive timeout occurs just reset the state machine */

{
    _l.rxState=RX_IDLE;
    _l.stats.toPkt++;
}
// ============================================================================================
// ============================================================================================
// ============================================================================================
struct LmsMsgStats *LmsMsgGetStats(void)

{
    return &_l.stats;
}
// ============================================================================================
BOOL LmsMsgTx( uint8_t *d, uint32_t len)

{
    uint32_t c;

    // Length does not include STX, len or CRC bytes - so shorten
    d[3]=len-6;
    c=_crc16(d+4,len-6);

    // ...and put the CRC into the footer bytes
    d[len-2]=(c>>8)&0xFF;
    d[len-1]=(c&0xFF);

    return (len!=serportTx(LLB_IF_PORT, d,len));
}
// ============================================================================================
void LmsMsgReceive(void)

/* This routine is only called while there are data pending */

{
    uint32_t j;

    while (serportDataPending(LLB_IF_PORT))
        {
            j = serportGetRx(LLB_IF_PORT);

            switch (_l.rxState)
                {
                    // ------------------------------------------
                    case RX_IDLE:
                        if (STX_H==j)
                            {
                                _l.rxState=RX_STX_L;

                                /* Start the timer to make sure the whole packet is received */
                                xTimerStart( _l.rxTimeout, 2 );
                            }
                        else
                            _l.stats.duffSTX++;
                        break;
                    // ------------------------------------------
                    case RX_STX_L:
                        if (STX_L==j)
                            _l.rxState=RX_VERSION;
                        else
                            {
                                _l.stats.duffSTX++;
                                _l.rxState=RX_IDLE;
                                xTimerStop( _l.rxTimeout, 2 );
                            }
                        break;
                    // ------------------------------------------
                    case RX_VERSION:
                        if (VERSION==j)
                            _l.rxState=RX_GETLEN;
                        else
                            {
                                _l.stats.badPkt++;
                                _l.rxState=RX_IDLE;
                                xTimerStop( _l.rxTimeout, 2 );
                            }
                        break;
                    // ------------------------------------------
                    case RX_GETLEN:
                        _l.rxingLen=_l.PktLen=j;
                        _l.pktptr=_l.pkt;

                        if ((j) && (j<=LMS_MAX_PACKET_LEN))
                            _l.rxState=RX_GETPACKET;
                        else
                            {
                                _l.rxState=RX_IDLE;
                                _l.stats.badPkt++;
                                xTimerStop( _l.rxTimeout, 2 );
                            }
                        break;
                    // ------------------------------------------
                    case RX_GETPACKET:
                        *_l.pktptr++=j;
                        if (!--_l.rxingLen)
                            _l.rxState=RX_GETCHECKSUMH;
                        break;
                    // ------------------------------------------
                    case RX_GETCHECKSUMH:
                        _l.checksum=j<<8;
                        _l.rxState=RX_GETCHECKSUML;
                        break;

                    // ------------------------------------------
                    case RX_GETCHECKSUML:
                        _l.checksum|=j;
                        xTimerStop( _l.rxTimeout, 2 );
                        if (_crc16(_l.pkt,_l.PktLen)!=_l.checksum)
                            {
                                _l.stats.badPkt++;
                            }
                        else
                            {
                                _l.stats.goodPkt++;

                                /* We don't deal with the actual message here, that's done upstairs */
                                if (!LmsDecode(_l.PktLen,_l.pkt))
                                    _l.stats.badContent++;
                            }
                        _l.rxState=RX_IDLE;
                        break;
                        // ------------------------------------------
                }
        }
}
// ============================================================================================
void LmsMsgInit(void)

{
    _l.rxTimeout=xTimerCreate("RXTO",RXTIMEOUT_TIME,pdFALSE, 0, _rxtoCallback);
    serportOpenPort(LLB_IF_PORT, LLB_IF_BAUDRATE,
                    UART_LCR_WLEN8 | UART_LCR_SBS_1BIT| UART_LCR_PARITY_DIS);
}
// ============================================================================================
