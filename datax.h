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
 * Data Exchange
 * =============
 * Messages between the M0 and the M4 CPUs.
 */

#ifndef _DEX_
#define _DEX_

// Messages between M0 and M4 processors
// CC DDD SSS  MMMMMMMM

#define MSG_CLASS_MANAGEMENT         (0)
#define MSG_CLASS_DATA               (1)
#define MSG_CLASS_ACTION             (2)
#define MSG_CLASS(x)                 ((x&3)<<14)
#define MSG_GET_CLASS(x)             ((x>>14)&3)

#define ANY_TO_ANY                   (0)
#define M0APP_TO_M4                  (1)
#define M0APP_TO_M0SUB               (2)
#define M0SUB_TO_M4                  (3)
#define M0SUB_TO_M0APP               (4)
#define M4_TO_M0APP                  (5)
#define M4_TO_M0SUB                  (6)

#define MSG_DIR(x)                   ((x&7)<<11)
#define MSG_GET_DIR(x)               ((x>>11)&7)

#define MSG_SEQ(x)                   ((x&7)<<8)
#define MSG_GET_SEQ(x)               ((x>>8)&7)

#define MSG(x)                       (x&0xFF)
#define MSG_GET_MSG(x)               (x&0xFF)
// =============================================
#define MSG_NULL                    0
#define MSG_PING                    1
#define MSG_PONG                    2
#define MSG_ENC_NULL                (MSG(MSG_NULL)|MSG_DIR(ANY_TO_ANY)|MSG_CLASS(MSG_CLASS_MANAGEMENT))
#define MSG_ENC_PING                (MSG(MSG_PING)|MSG_DIR(ANY_TO_ANY)|MSG_CLASS(MSG_CLASS_MANAGEMENT))
#define MSG_ENC_PONG                (MSG(MSG_PONG)|MSG_DIR(ANY_TO_ANY)|MSG_CLASS(MSG_CLASS_MANAGEMENT))

#define MSG_STRING                  1
#define MSG_ENC_STRING              (MSG(MSG_STRING)|MSG_DIR(ANY_TO_ANY)|MSG_CLASS(MSG_CLASS_DATA))
#define MSG_PSNANDATT               2
#define MSG_ENC_PSNANDATT           (MSG(MSG_PSNANDATT)|MSG_DIR(M0APP_TO_M4)|MSG_CLASS(MSG_CLASS_DATA))
#define MSG_9D		                3
#define MSG_ENC_9D           		(MSG(MSG_9D)|MSG_DIR(M0APP_TO_M4)|MSG_CLASS(MSG_CLASS_DATA))

struct MSGpsnandatt

{
  int16_t psn[3];
  int16_t q[4];
  int16_t temp;
  uint32_t tsPsn;
  uint32_t tsQ;
};

struct MSG9d

{
	int16_t acc[3];
	int16_t gyr[3];
	int16_t mag[3];
};
// ============================================================================================
#endif

