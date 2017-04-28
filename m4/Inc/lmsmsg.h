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
 * Send and receive messages over serial link
 */


#ifndef LMSMSG_H
#define LMSMSG_H

#include "config.h"

#define VERSION ((0<<4)|(1))            // Major, Minor

/* Header format is MAGIC_H, MAGIC_L, VERSION, LEN, SENSORTYPE, SENSORID, TIME */
#define STX_H 0x7E
#define STX_L 0x8E
#define HEADER(sensorType, sensorID) STX_H, STX_L, VERSION, 0,sensorType,sensorID,INT32TO8(TICKS_TO_MILLIS(xTaskGetTickCount()))
#define FOOTER 0,0
#define INT32TO8(x) ((x>>24)&0xff),((x>>16)&0xff),((x>>8)&0xff),(x&0xff)
#define INT16TO8(x) ((x>>8)&0xff),(x&0xff)


ALWAYS_INLINE uint16_t int8to16(uint8_t *p)
{
    return ((*p)<<8)|(*(p+1));
}
ALWAYS_INLINE uint32_t int8to32(uint8_t *p)
{
    return (((*p)<<24)|((*(p+1))<<16)|((*(p+2))<<8)|(*(p+3)));
}
ALWAYS_INLINE int32_t sint8to32(uint8_t *p)
{
    return (((*p)<<24)|((*(p+1))<<16)|((*(p+2))<<8)|(*(p+3)));
}
ALWAYS_INLINE int16_t sint8to16(uint8_t *p)
{
    return (((*(p))<<8)|(*(p+1)));
}

#define LMS_MAX_PACKET_LEN (64)

struct LmsMsgStats

{
    uint32_t duffSTX;
    uint32_t goodPkt;
    uint32_t badPkt;
    uint32_t toPkt;
    uint32_t badContent;
};

// ============================================================================================
struct LmsMsgStats *LmsMsgGetStats(void);
BOOL LmsMsgTx( uint8_t *d, uint32_t len);
void LmsMsgReceive(void);

void LmsMsgInit(void);
// ============================================================================================

#endif
