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
* Audio module
* ============
*
* This module controls the audio output. The code automatically deals with different
* sample rates etc....but it's only mono.
*
* In a 'full' implementation this probably needs extending to read directly from a file rather
* than from memory....that'll run out quickly.
*
*/
#ifndef __AUDIO_H_
#define __AUDIO_H_

// ============================================================================================
BOOL audioPlay( uint32_t playLenSet, uint32_t sampleRateSet, BOOL waitSet,
                const uint32_t *dataToPlay );        /// <Play an audio file, either wait for it to finish or return immediate.
BOOL audioPlaying( void );                          /// <Indicate if something is playing
void audioStop( void );                             /// <Stop audio playing (at next sample point)

void audioInit( void );                             /// <Start up the audio subsystem
// ============================================================================================
#endif /* __AUDIO_H_ */
