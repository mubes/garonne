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
 * Terminal Management
 * ===================
 *
 * This code was originally (c) Dave Marples. A copy passed to Technolution for independent development and
 * use without restriction on 13 Jun 2015.
 */

#ifndef _TERMCODES_
#define _TERMCODES_

#include "config.h"
#include "serport.h"

// These are VT100 codes ... they are reasonably universal

#define TERM_COLS 80
#define TERM_ROWS 25
#define TERM_RESET_DELAY 1000  /* Delay in mS after sending a reset before resuming comms */

#define TERM_BEEP        "\007"         // Output a beep
#define TERM_BACKDEL     "\b \b"        // Delete backwards
#define TERM_ERASEEOL    "\033[K"       // Erase to end of line
#define TERM_ERASESCREEN "\033[2J"   // Erase entire screen
#define TERM_CURSORON    "\033[?25h"    // Show cursor
#define TERM_CURSOROFF   "\033[?25l"  // Hide cursor
#define TERM_GOTOXY      "\033[%d;%dH"   // Goto specified line/col
#define TERM_RESIZE      "\033[8;%d;%dt" // Resize window
#define TERM_FORWARD     "\033[%dC"      // Cursor forwards
#define TERM_TITLE       "\033]2;%s: %s\007" // Set title
#define TERM_ICON        "\033]1;%s\007" // Set icon name
#define TERM_LINEDRAW_G1 "\033)0"        // Switch to linedrawing characters in G1
#define TERM_SEL_NORM     "\017"         // Select set G0
#define TERM_SEL_GRAP     "\016"         // Select set G1
#define TERM_SETCSI       "\033["        // Go to CSI mode to set colours
#define TERM_RESET        "\033c"        // Full terminal reset to defaults
#define TERM_ENABLEBOLD   ";1"           // Enable bold foreground
#define TERM_SCROLL       "\033[%d;%dr"  // Set scroll region
#define TERM_RESETSCROLL  "\033[r"       // Reset scroll region
#define TERM_ERASEEOL     "\033[K"       // Erase to end of line
#define TERM_IDLE_TEXT    "Inactive"     // Title and Icon for windows not in use
#define TERM_SAVECURSOR   "\0337"        // Save cursor and attributes
#define TERM_RESTORECURSOR "\0338"       // Restore cursor and attributes
#define TERM_SET132COLS    "\033[?3h"    // Switch to 132 column mode

typedef enum {TERM_CURSOR_BLINKBLOCK,TERM_CURSOR_DEFAULT,TERM_CURSOR_STEADYBLOCK,TERM_CURSOR_BLINKUNDER,TERM_CURSOR_STEADYUNDER,TERM_CURSOR_BLINKBAR,TERM_CURSOR_STEADYBAR} termCursorStyleEnum;
typedef enum {TERMBOXSTYLE_SINGLE, TERMBOXSTYLE_DOUBLE, TERMBOXSTYLE_MAXSTYLES } termBoxStyleEnum;
typedef enum {TERMBOXGRAPHIC_HORIZ,TERMBOXGRAPHIC_VERT,TERMBOX_CORNERLT,TERMBOX_CORNERRT,TERMBOX_CORNERLB,TERMBOX_CORNERRB,TERM_BOXMAX} termBoxElementEnum;
typedef enum {TERM_BLACK, TERM_RED, TERM_GREEN, TERM_YELLOW, TERM_BLUE, TERM_MAGENTA, TERM_CYAN, TERM_WHITE, TERM_MAX_COLOURS } termColour;
#define COLOURCHANGED (1<<31)
#define TERM_BOLD (1<<30)

typedef enum {TERM_EV_NONE=0, TERM_EV_OPEN=(1<<0), TERM_EV_CLOSE=(1<<1), TERM_EV_RX=(1<<2), TERM_EV_REFRESH=(1<<3)} termEventEnum;

// ============================================================================================
void termClear(void);
termEventEnum termWaitEvent(uint32_t waitTimeSet);
void termBeep(void);
void termCursorOn(BOOL visibility);
uint32_t termKeywaiting(void);
uint8_t termGetKey(void);
void termFlushKey(void);
void termGotoYX(uint32_t ySet, uint32_t xSet);
uint32_t termGotoYXPrintf(uint32_t ySet, uint32_t xSet, char *fmt, ...);
uint32_t termPrintf(char *fmt, ...);
void termTitle(char *title);
void termScroll(uint32_t ys, uint32_t ye);
void termResetAll(void);
void termIcon(char *icon);
void termSetGraphic(BOOL isGraphic);
void termMultiput(uint32_t count, char c);
void termMultiputXY(uint32_t xLoc, uint32_t yLoc, uint32_t count, char c);
void termMultiputv(uint32_t xLoc, uint32_t yLoc, uint32_t count, char c);
void termBox(termBoxStyleEnum b, uint32_t xs, uint32_t ys, uint32_t xe, uint32_t ye);
void termSetcolour(termColour fg, termColour bg);
void termSetupScroll(uint32_t ys, uint32_t ye);
void termSendLength(uint8_t *startPoint, uint32_t length);
void termPushState(void);
void termPopState(void);
void termBold(BOOL isBold);
BOOL termIsOpen(void);
void termClose(void);
void termForceClose(void);

void termSeize(void);
void termRelease(void);
void termInit(serportEnumType activePortSet, char *titleSet, char *iconSet);

// ============================================================================================
#endif
