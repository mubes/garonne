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

#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "ui.h"
#include "leds.h"
#include "serport.h"
#include "dist.h"
#include "motor.h"
#include "rotenc.h"
#include "gio.h"
#include "lms.h"
#include "mainloop.h"
#include "git_version_info.h"
#ifdef INCLUDE_SDMMC
#include "ff.h"
#endif
#include "stats.h"
#include "vldist.h"
#include "ipcMsg.h"

#define MAXLINELEN 80

typedef struct /* Dynamic state of command handler */

{
    uint32_t cp;
    uint32_t paramCount;
    SemaphoreHandle_t output;

    char readLine[MAXLINELEN];
    char *param[MAX_PARAMS];
} UIStateStruct;

/* ------------------------- */
/* Module specific variables */
/* ------------------------- */
static UIStateStruct UIState;

COMMAND(_default);
COMMAND(_help);
COMMAND(_calib);
COMMAND(_commit);
COMMAND(_dump);
COMMAND(_id);
COMMAND(_info);
COMMAND(_servo);
COMMAND(_estop);
COMMAND(_machine);
COMMAND(_setled);
COMMAND(_clearleds);
COMMAND(_velocity);

COMMAND(_dir);
COMMAND(_writeblocks);

#define OUTPUT_SEIZE_WAIT MILLIS_TO_TICKS(100)

/* These filter messages from a BT modem on connect events and stop them being processed as commands */
#define COMMS_ESCAPES                       \
        { "AT+GCAP", &_default, NULL },    \
        { "AT", &_default, NULL },         \
        {"+CONNECTED", &_default, NULL}

/* Default UI commands */
#define UI_COMMANDS                                                                                                 \
        { "Calib", &_calib, "<M|A|G> ox,oy,oz,sx,sy,sz Enter calib offsets and scale for selected sensor" },        \
        { "Commit", &_commit, "Store settings permanently" },                                                       \
        { "Dump", &_dump, "<M|A|G|*> <count> Dump count of Mag, Acc, Gyr or *All readings" },                       \
        { "HELP", &_help,"This help text" },                                                                        \
        { "Id",&_id, "<N> Set <N>umber of this node, in hex" },                                                     \
        { "Info", &_info, "Get information about the system" },                                                     \
        { "LC", &_clearleds,"Clear all LEDs in string" },                                                           \
        { "LS", &_setled, "x r g b Set LED x to r,g,b value" },                                                     \
        { "S",  &_servo, "x Set servo to value x" },                                                                \
        { "V",  &_velocity, "x Set vehicle velocity to x" }

#ifdef INCLUDE_SDMMC
#define DISK_COMMANDS                                                   \
                { "DIR", &_dir, "Produce directory listing of MMC" },   \
                { "W", &_writeblocks, "x y Write y bytes to disk in file x" }
#endif
/* The available commands - edit this list to add new classes, following the template above */
const commandList commands[] = {
COMMS_ESCAPES,
UIOS_COMMANDS,
#ifdef INCLUDE_SDMMC
                DISK_COMMANDS,
#endif
                UI_COMMANDS, { 0, FALSE, NULL } };
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Individual command handlers
// ============================================================================================
// ============================================================================================
// ============================================================================================
COMMAND(_default)

{
    return UI_OK;
}
// ============================================================================================
COMMAND(_help)

#define HELP_FIELDLEN 10

// Issue list of commands

{
    const commandList *c = commands;
    uint32_t l;

    while (c->handler)
    {
        if (c->desc)
        {
            serportPrintf (TERMINAL_PORT, "%s", c->command);
            l = strlen (c->command);
            if (l < HELP_FIELDLEN)
                serportMultiput (TERMINAL_PORT, SERIAL_TX_WAIT, ' ',
                HELP_FIELDLEN - l);
            serportPrintf (TERMINAL_PORT, "%s" EOL, c->desc);
        }
        c++;
    }
    serportPrintf (TERMINAL_PORT, EOL "!    Emergency stop all" EOL "?    Report all current status" EOL);
    return UI_OK;
}
// ============================================================================================
COMMAND(_info)

{
    struct MSGpsnandatt *p = IPCMsgGetpsanandatt (); /* Collect latest position, quat and temp info */

    if (nparams > 1)
    {
        return UI_ERROR_TOO_MANY_PARAMETERS;
    }

    serportPrintf (TERMINAL_PORT, EOL PRODUCTNAME " (Version %d.%02x)" EOL BUILD_DATE " ", VERSION_NUMBER);

#ifdef DEBUG
    serportPrintf (TERMINAL_PORT, "DEBUG BUILD" EOL);
#else
    serportPrintf(TERMINAL_PORT,"Release Build" EOL);
#endif

    serportPrintf (TERMINAL_PORT, "(%s) %s %08X" EOL, GIT_DIRTY ? "Dirty" : "Clean", GIT_BRANCH, GIT_HASH);
    if (ConfigID () != NOENTITY)
    {
        serportPrintf (TERMINAL_PORT, "ID %04x", ConfigID ());
    }
    else
    {
        serportPrintf (TERMINAL_PORT, "NO ID");
    }

    if (ConfigNomadic ())
    {
        serportPrintf (TERMINAL_PORT, ", Nomadic");
        if (!((ConfigLocx () == NOLOCATION) || (ConfigLocy () == NOLOCATION) || (ConfigLocz () == NOLOCATION)))
            serportPrintf (TERMINAL_PORT, ", at [%dmm,%dmm,%dmm]", ConfigLocx (), ConfigLocy (), ConfigLocz ());
    }
    else
    {
        if ((ConfigLocx () == NOLOCATION) || (ConfigLocy () == NOLOCATION) || (ConfigLocz () == NOLOCATION))
        {
            serportPrintf (TERMINAL_PORT, ", No Location");
        }
        else
        {
            serportPrintf (TERMINAL_PORT, ", [%dmm,%dmm,%dmm]", ConfigLocx (), ConfigLocy (), ConfigLocz ());
        }
    }

    const uint32_t *t = ConfigGetSerialNumber ();
    serportPrintf (TERMINAL_PORT, EOL "S/N:%08x-%08x-%08x-%08x-%d" EOL, t[0], t[1], t[2], t[3], ConfigID ());

    serportPrintf (TERMINAL_PORT, "Calib: ");

    for (enum CalibFrameInstance c = 0; c < CF_Max; c++)
    {
        struct calibFrame *f = ConfigGetCalibFrame (c);
        serportPrintf (TERMINAL_PORT, "%s [%4d,%4d,%4d],[%4d,%4d,%4d] " EOL, AXIS_NAME [c], f->ofs[0], f->ofs[1], f->ofs[2], f->scale[0],
                       f->scale[1], f->scale[2]);
        if (c < (CF_Max - 1))
            serportPrintf (TERMINAL_PORT, "       ");
    }

    serportPrintf (TERMINAL_PORT, "Config %sSaved, ", (ConfigIsSaved () ? "" : "Not "));

    serportPrintf (TERMINAL_PORT, "Vdd=%d.%03dV, Temp=%ddegC" EOL, GIOBattery () / 1000, GIOBattery () % 1000, p->temp / 1000);

    serportPrintf (TERMINAL_PORT, "Ready" EOL);

    return UI_OK;
}
// ============================================================================================
COMMAND(_dump)

{
    uint32_t c;

    if (nparams > 3)
    {
        return UI_ERROR_TOO_MANY_PARAMETERS;
    }

    if (nparams < 3)
    {
        return UI_ERROR_TOO_FEW_PARAMETERS;
    }

    if ((strlen (param[1]) > 1)
                    || (((toupper (*param[1])) != 'M') && (toupper (*param[1]) != 'A') && (toupper (*param[1]) != 'G')
                                    && (toupper (*param[1]) != '*')))
    {
        return UI_ERROR_PARAM_FORMAT;
    }

    c = strtol (param[2], NULL, 10);

    if (!c)
    {
        return UI_ERROR_PARAM_FORMAT;
    }

    if (toupper (*param[1]) == 'M')
    {
        serportPrintf (TERMINAL_PORT, "MagX,MagY,MagZ" EOL);
        // ninedDump(DumpMag,c);
        serportPrintf (TERMINAL_PORT, "==END==" EOL);
    }

    if (toupper (*param[1]) == 'A')
    {
        serportPrintf (TERMINAL_PORT, "AccX,AccY,AccZ" EOL);
        //  ninedDump(DumpAcc,c);
        serportPrintf (TERMINAL_PORT, "==END==" EOL);
    }

    if (toupper (*param[1]) == 'G')
    {
        serportPrintf (TERMINAL_PORT, "GyrX,GyrY,GyrZ" EOL);
        // ninedDump(DumpGyr,c);
        serportPrintf (TERMINAL_PORT, "==END==" EOL);
    }

    if (toupper (*param[1]) == '*')
    {
        serportPrintf (TERMINAL_PORT, "AccX,AccY,AccZ,MagX,MagY,MagZ,GyrX,GyrY,GyrZ" EOL);
        //ninedDump(DumpAll,c);
        serportPrintf (TERMINAL_PORT, "==END==" EOL);
    }
    return UI_OK;
}
// ============================================================================================
COMMAND(_id)

{
    char *endptr;
    uint32_t idSet;

    if (nparams < 2)
    {
        return UI_ERROR_TOO_FEW_PARAMETERS;
    }

    if (nparams > 2)
    {
        return UI_ERROR_TOO_MANY_PARAMETERS;
    }

    /* Perform conversion and make sure the number was valid */
    idSet = strtol (param[1], &endptr, 16);
    if (*endptr != 0)
    {
        return UI_ERROR_PARAM_FORMAT;
    }

    ConfigSetID(idSet);
    return UI_OK;
}
// ============================================================================================
COMMAND(_calib)

{
    int16_t d[6];
    char *endptr;
    enum CalibFrameInstance g;

    if (nparams > 3)
    {
        return UI_ERROR_TOO_MANY_PARAMETERS;
    }

    if (nparams < 3)
    {
        return UI_ERROR_TOO_FEW_PARAMETERS;
    }

    if (strlen (param[1]) > 1)
    {
        return UI_ERROR_PARAM_FORMAT;
    }

    switch (toupper (*param[1]))
        {
        case 'M':
            g = CF_Mag;
            break;
        case 'A':
            g = CF_Acc;
            break;
        case 'G':
            g = CF_Gyr;
            break;
        default:
            return UI_ERROR_PARAM_FORMAT;
        }

    endptr = param[2];

    for (uint32_t i = 0; i < 6; i++)
    {
        d[i] = strtol (endptr, &endptr, 10);
        if ((!(*endptr) && (i < 5)) || ((*endptr != ',') && (i < 5) && (*endptr != ' ')))
        {
            return UI_ERROR_PARAM_FORMAT;
        }
        endptr++;
    }

    if (ConfigSetCalibFrame (g, d[0], d[1], d[2], d[3], d[4], d[5]))
    {
        return UI_OK;
    }

    return UI_ERROR_PARAM_FORMAT;
}
// ============================================================================================
COMMAND(_commit)

{
    if (ConfigIsSaved ())
    {
        return UI_NO_ACTION;
    }

    if (ConfigCommit ())
    {
        return UI_OK;
    }
    else
    {
        return UI_COMMAND_FAILED;
    }
}
// ============================================================================================

COMMAND(_setled)

{
    char *endptr;
    uint32_t l, r, g, b;

    if (nparams > 5)
        return UI_ERROR_TOO_MANY_PARAMETERS;
    if (nparams < 5)
        return UI_ERROR_TOO_FEW_PARAMETERS;

    l = strtol (param[1], &endptr, 10);
    if (*endptr != 0)
        return UI_ERROR_PARAM_FORMAT;
    r = strtol (param[2], &endptr, 10);
    if (*endptr != 0)
        return UI_ERROR_PARAM_FORMAT;
    g = strtol (param[3], &endptr, 10);
    if (*endptr != 0)
        return UI_ERROR_PARAM_FORMAT;
    b = strtol (param[4], &endptr, 10);
    if (*endptr != 0)
        return UI_ERROR_PARAM_FORMAT;

    GIOSetConnected(TRUE);
    if (!LEDsetColour (l, RED(r) | GREEN(g) | BLUE(b)))
        return UI_COMMAND_FAILED;
    else
    {
        LEDPrint();
        return UI_OK;
    }
}
// ============================================================================================
#ifdef INCLUDE_SDMMC
COMMAND(_dir)

{
    DIR dir;
    FILINFO fno;
    uint32_t rc;

    if (nparams > 1)
    return UI_ERROR_TOO_MANY_PARAMETERS;

    rc = f_opendir(&dir, "");
    if (rc)
    {
        return UI_ERROR_CANNOT_OPEN_DIR;
    }

    for (;;)
    {
        /* Read a directory item */
        rc = f_readdir(&dir, &fno);
        if (rc || !fno.fname[0])
        {
            break; /* Error or end of dir */
        }
        if (fno.fattrib & AM_DIR)
        {
            serportPrintf(TERMINAL_PORT, "   <dir>  %s\r\n", fno.fname);
        }
        else
        {
            serportPrintf(TERMINAL_PORT, "   %8lu  %s\r\n", fno.fsize, fno.fname);
        }
    }
    return UI_OK;
}
// ============================================================================================
COMMAND(_writeblocks)

{
    FIL fil;
    FRESULT r;
    unsigned int bw;
    uint8_t b[8192];

    if (nparams > 3)
    return UI_ERROR_TOO_MANY_PARAMETERS;
    if (nparams < 3)
    return UI_ERROR_TOO_FEW_PARAMETERS;

    char *endptr;
    uint32_t e;

    e = strtol(param[2], &endptr, 10);
    if (*endptr != 0)
    return UI_ERROR_PARAM_FORMAT;

    if (f_open(&fil, param[1], FA_WRITE | FA_CREATE_ALWAYS))
    return UI_COMMAND_FAILED;

    while (e)
    {
        if ((r=f_write(&fil,b,(e>4*8192)?4*8192:e,&bw)))
        {
            dbgprint("Error %d" EOL,r);
            return UI_COMMAND_FAILED;
        }
        e-=bw;
    }
    f_close(&fil);

    return UI_OK;
}
#endif
// ============================================================================================
COMMAND(_clearleds)

{
    if (nparams > 1)
        return UI_ERROR_TOO_MANY_PARAMETERS;

    GIOSetConnected(TRUE);
    LEDclearAll();
    LEDPrint();

    return UI_OK;
}
// ============================================================================================
COMMAND(_servo)

/* Set servo angle */

{
    char *endptr;
    uint32_t l;

    if (nparams > 2)
        return UI_ERROR_TOO_MANY_PARAMETERS;
    if (nparams < 2)
        return UI_ERROR_TOO_FEW_PARAMETERS;

    l = strtol (param[1], &endptr, 10);
    if (*endptr != 0)
        return UI_ERROR_PARAM_FORMAT;

    if (motorServoSet (l))
        return UI_OK;
    else
        return UI_ERROR_PARAM_FORMAT;
}
// ============================================================================================
COMMAND(_velocity)

/* Set vehicle speed */

{
    char *endptr;
    int32_t v;

    if (nparams > 2)
        return UI_ERROR_TOO_MANY_PARAMETERS;
    if (nparams < 2)
        return UI_ERROR_TOO_FEW_PARAMETERS;

    v = strtol (param[1], &endptr, 10);
    if (*endptr != 0)
        return UI_ERROR_PARAM_FORMAT;

    if (motorSpeed (v))
        return UI_OK;
    else
        return UI_ERROR_PARAM_FORMAT;
}
// ========================================================================================
COMMAND(_estop)

{
    motorServoReset ();
    return (UI_OK);
}
// ============================================================================================
void
_status (void)

{
    struct MSGpsnandatt *p = IPCMsgGetpsanandatt (); /* Collect latest position, quat and temp info */

    serportPrintf (TERMINAL_PORT, "S:%0d,P:%d,V:%d,", motorServoGet (), RotencGetPsn (), motorGetSpeed ());

#ifndef VL_DISTANCE
    serportPrintf(TERMINAL_PORT, "D:%d,%d,", (DISTgetChannel(DIST_FRONT)),
                    (DISTgetChannel(DIST_BACK)));
#else
    serportPrintf (TERMINAL_PORT, "D:%d,", (VLDISTgetChannel (VLDIST_FRONT)));
//    serportPrintf(TERMINAL_PORT, "D:%d,%d,", (VLDISTgetChannel(VLDIST_FRONT)),   FIXME
//            (DISTgetChannel(VLDIST_BACK)));

#endif
    serportPrintf (TERMINAL_PORT, "E:%d%d,", (0 == motorGeteStop (MOTOR_ESTOP_FORWARDS)), (0 == motorGeteStop (MOTOR_ESTOP_BACKWARDS)));

    serportPrintf (TERMINAL_PORT, "P:%4d,%4d,%4d,", p->psn[0], p->psn[1], p->psn[2]);
    serportPrintf (TERMINAL_PORT, "Q:%6d,%6d,%6d,%6d,", p->q[0], p->q[1], p->q[2], p->q[3]);
    serportPrintf (TERMINAL_PORT, "H:%d,", p->temp);
    serportPrintf (TERMINAL_PORT, "B:%d,N:%d" EOL, GIOBattery (), GIOUserButtonState ());
}
// ============================================================================================
void
_parse (UIStateStruct *u)

/* Parse the entered line, breaking it into tokens, then dispatch it */

{
    char *p = u->readLine;

    const commandList *c = commands;
    uint32_t retVal;

    u->paramCount = 0;

    /* Find the partitions */
    while (*p)
    {
        while (isspace ((int) *p))
            p++;
        if (*p)
        {
            /* Deal with parameter escaped with " */
            if (*p == '"')
            {
                p++;
                if (*p)
                {
                    u->param[u->paramCount++] = p;
                    while ((*p) && (*p != '"'))
                        p++;
                    if (*p)
                        *p++ = 0;
                    continue;
                }
            }
            u->param[u->paramCount++] = p; /* end of whitespace - so squirrel it */
            while ((*p) && (!isspace ((int) *p))) // ....Add this if you want commas to delimit --> && (*p != ','))
                p++;
            if (*p)
                *p++ = 0;
        }
    }

    /* Go through looking for a match */
    while ((c->handler) && (strcasecmp ((char *) c->command, (char *) u->param[0])))
        c++;

    if (c->handler)
        retVal = c->handler (u->paramCount, u->param);
    else
        retVal = UI_ERROR_NO_SUCH_COMMAND;

    switch (ERROR_CLASS(retVal))
        {
        case UI_OK:
            break;

        case UI_ERROR_TOO_FEW_PARAMETERS:
            serportPrintf (TERMINAL_PORT, "Too few Parameters" EOL);
            break;

        case UI_ERROR_TOO_MANY_PARAMETERS:
            serportPrintf (TERMINAL_PORT, "Too many Parameters" EOL);
            break;

        case UI_USER_INTERRUPT:
            serportPrintf (TERMINAL_PORT, "User Interrupt" EOL);
            break;

        case UI_ERROR_PARAM_FORMAT:
            serportPrintf (TERMINAL_PORT, "Parameter error" EOL);
            break;

        case UI_COMMAND_FAILED:
            serportPrintf (TERMINAL_PORT, "Command Failed" EOL);
            break;

        case UI_NO_ACTION:
            serportPrintf (TERMINAL_PORT, "No Action Performed" EOL);
            break;

        case UI_ERROR_NO_SUCH_COMMAND:
            serportPrintf (TERMINAL_PORT, "No such command" EOL);
            break;

        case UI_ERROR_CANNOT_OPEN_DIR:
            serportPrintf (TERMINAL_PORT, "Cannot open directory" EOL);
            break;

        default:
            serportPrintf (TERMINAL_PORT, "Unknown Error" EOL);
            break;
        }
}

// ============================================================================================
// ============================================================================================
// ============================================================================================
// Publicly available routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
void
UIProcessHandler (uint32_t e)

{
    uint8_t j;

    if ((e & SERPORT_EV_CONNECT) || (e & SERPORT_EV_CLOSE))
    {
        UIState.readLine[UIState.cp = 0] = 0;
        if (e & SERPORT_EV_CONNECT)
        {
            serportPrintf (TERMINAL_PORT, EOL "# " PRODUCTNAME " " VERSION_STRING EOL);
            serportPrintf (TERMINAL_PORT, "# HW version: 0x%02X" EOL, HARDWARE_VERSION);
            serportPrintf (TERMINAL_PORT, "# SW version: 0x%08X Branch: \"%s\"" EOL "# Hash: %x - %s [\"%s\"]"
            EOL,
                           VERSION_SEQ, GIT_BRANCH, GIT_HASH, (GIT_DIRTY ? "!!DIRTY!!" : "CLEAN"), BUILD_DATE);
        serportPrintf(TERMINAL_PORT, "# " COPYRIGHT_STATEMENT EOL "# Ready" EOL TERMINAL_PROMPT);
    }
}

if (e & SERPORT_EV_DATARX)
{
    while (serportDataPending (TERMINAL_PORT))
    {
        j = serportGetRx (TERMINAL_PORT);
        switch (j)
            {
            // ---------------------------------
            // --- Newline
            // ---------------------------------
            case '\n':
            case '\r':
                UIState.readLine[UIState.cp] = 0;
                serportPrintf (TERMINAL_PORT, EOL);
                if (UIState.cp)
                    _parse (&UIState);
                UIState.readLine[UIState.cp = 0] = 0;
                serportPrintf (TERMINAL_PORT, TERMINAL_PROMPT);
                break;

                // ---------------------------------
                // --- Delete key
                // ---------------------------------
            case 8:
            case 127:
                if (UIState.cp)
                {
                    UIState.readLine[--UIState.cp] = 0;
                    serportPrintf (TERMINAL_PORT, "\010 \010");
                }
                break;

                // ---------------------------------
                // --- Emergency Stop
                // ---------------------------------
            case '!':
                motorServoReset ();
                motorSpeed (0);
                break;

                // ---------------------------------
                // --- Status Request
                // ---------------------------------
            case '?':
                if (UIState.cp > 0)
                    serportPrintf (TERMINAL_PORT, EOL);
                _status ();
                UIState.readLine[UIState.cp] = 0;
                serportPrintf(TERMINAL_PORT, TERMINAL_PROMPT "%s", UIState.readLine);
                break;

                // ---------------------------------
                // --- Everything else
                // ---------------------------------
            default:
                if ((j > 31) && (UIState.cp < (MAXLINELEN - 1)))
                {
                    UIState.readLine[UIState.cp++] = j;
                    serportPrintf (TERMINAL_PORT, "%c", j);
                }
                break;
            }
    }

}
}
// ============================================================================================
BOOL
UISeize (BOOL Seize)

{
if (Seize)
{
    return (xSemaphoreTake(UIState.output, OUTPUT_SEIZE_WAIT));
}
else
{
    return (xSemaphoreGive(UIState.output));
}

}
// ============================================================================================
void
UISetup (void)
{
UIState.output = xSemaphoreCreateMutex();
UISeize (FALSE);

serportOpenPort (TERMINAL_PORT, TERMINAL_BAUDRATE,
UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS);
}
// ============================================================================================
