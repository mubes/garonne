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
 * UI module
 * =========
 *
 * This module takes input from the serial link and dispatches it appropriately.
 *
 */

#ifndef APP_UI_H_
#define APP_UI_H_

#include "config.h"

enum UIERROR_ENUM {UI_OK,UI_ERROR_TOO_FEW_PARAMETERS,
                   UI_ERROR_TOO_MANY_PARAMETERS,
                   UI_USER_INTERRUPT,
                   UI_ERROR_PARAM_FORMAT,
                   UI_COMMAND_FAILED,
                   UI_NO_ACTION,
                   UI_ERROR_NO_SUCH_COMMAND,
                   UI_ERROR_CANNOT_OPEN_DIR,
                   UI_MAXERRORS
                  };

#define MAX_PARAMS 8 // Maximum number of parameters to be passed in any routine
#define COMMAND(x) uint32_t (x)(uint32_t nparams, char **param) // A command handler

#define ERROR_CLASS(x) (x&0xFFFF)
#define ERROR_DETAIL(x) ((x>>16)&0xFFFF)
#define ENCODE_ERROR(x,y) (((x&0xFFFF)<<16)|(y&0xFFFF))

typedef struct
{
    char *command;
    COMMAND(*handler);
    char *desc;
} commandList;

extern const commandList commands[];

// ============================================================================================
void UISetup(void);
BOOL UISeize(BOOL Seize);
void UIProcessHandler(uint32_t e);
// ============================================================================================
#endif /* APP_UI_H_ */
