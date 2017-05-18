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
 * All primary configuration is in this file...if a module includes this file it
 * will have the basic configuration it needs.
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
 */

#ifndef _CONFIG_
#define _CONFIG_

#ifdef CORE_M4
#define IAM_M4
#define INCLUDE_M0_APP
#define M0_APPBASE (0x1B000000)
#endif

#ifdef CORE_M0
#define IAM_M0APP
#endif

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "cmsis.h"
#include "datax.h"

/* Define one of these for the board revision you're building for */
#define VL_DISTANCE         // <<< Using VL distance sensor rather than Ultrasonics

//#define DISABLE_DISTANCE_SENSORS  // <<< Define to disable the ultrasonic distance sensors

#define LLB             /* Just to confirm we're building for the LLB */
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Configuration Setup
// ============================================================================================
// ============================================================================================
// ============================================================================================

#define DEBUG 1
#define DEBUG_ON_TERMINAL

// ----- Component Inclusion is done via the Makefile

// ----- Version numbering
// -----------------------
#define VERSION_STRING          "3.70"
#define VERSION_NUMBER          0x03,0x70
#define COPYRIGHT_STATEMENT     "(C) 2015-2016 Garonne"
#define PRODUCTNAME             "Icarus_III"
#define UUID                     0x53,0xce,0x1b,0x11,0xdd,0x8f,0x47,0x94,0xbf,0x1c,0xb0,0x62,0x64,0x71,0x4e,0xa1
#define MAGICNUMBER             0x12190405      /* Magic number showing that there's valid data in the store */

#define VERSION_SEQ              0x16122301

extern const uint8_t idSeq[];           /* Unique identifier for management operations */

// ----- CPU Targeting
// -------------------
#define CLOCK_INPUT CLKIN_CRYSTAL
#define XTAL_FREQ 12000000

// ---- USB String Definitions
// ---------------------------

// Include these definitions even if USB isn't in use. We leave it to the linker to remove
// the unused code.
//
// it is ESSENTIAL that the lengths match the definitions!!!
#define USB_VENDOR_ID        (0x1FC9)
#define USB_DEVICE_ID        (0xD083)

#define USB_MANUFACTURER     'T',0, 'e',0, 'c',0, 'h',0, 'n',0, 'o',0, 'l',0, 'u',0, 'i',0, 'o',0, 'n',0
#define USB_MANUFACTURER_LEN (11)

#define USB_PRODUCT          'G',0, 'a',0, 'r',0, 'o',0, 'n',0, 'n',0, 'e',0
#define USB_PRODUCT_LEN      (7)

#define USB_SERIAL_NUMBER    '0',0, '0',0, '0',0, '0',0, '0',0, '0',0, '0',0, '0',0, '-',0,  \
    '0',0, '0',0, '0',0, '0',0, '0',0, '0',0, '0',0, '0',0 ,'-',0, \
    '0',0, '0',0, '0',0, '0',0, '0',0, '0',0, '0',0, '0',0 ,'-',0,\
    '0',0, '0',0, '0',0, '0',0, '0',0, '0',0, '0',0, '0',0 ,'-',0,\
    '0',0
#define USB_SERIAL_NUMBER_LEN  (4*9+1)

#define USB_SETTING_NAME     'S',0, 'e',0, 'r',0, 'i',0, 'a',0, 'l',0
#define USB_SETTING_NAME_LEN (6)
#define USB_INTERRUPT_PRIORITY (7)

// Radio subsystem (radio.c)
// -------------------------
/* Addressing constructs for vehicles and infrastructure */
#define NOENTITY            0
#define NODISTANCE          0
#define NOLOCATION          (0xFFFF)
#define ALL                 (0xFFFE)

// Sets the frame response timeout size ... keep as low as possible */
#define NUM_ADDRESSES         (8)

#if defined(LPC4370)
#include "config-lpc4370.h"
#endif

#if defined(LPC4367)
#include "config-lpc4367.h"
#endif

// Flags describing remote entities
#define FLAG_CHARGING               (1<<0)  /* Device is charging its battery at the moment */
#define FLAG_EXTPWR                 (1<<1)  /* Device is running on external power */
#define FLAG_NOMADIC                (1<<2)  /* Device is nomadic (i.e. should not be used for range calculations) */

// ============================================================================================
// ============================================================================================
// ============================================================================================
//Internal Setup
// ============================================================================================
// ============================================================================================
// ============================================================================================
typedef uint32_t BOOL;
#define EOL                             "\n\r"

// --- System Level Glue
// ---------------------
#ifndef FALSE
#define FALSE 0
#define TRUE  (!FALSE)
#endif

#define OFF   FALSE
#define ON    TRUE

// --- Encode ports and pins a bit more compact
// --------------------------------------------
#define PINDEF(port,pin,iocon) ((port&0xFF)|((pin&0xFF)<<8)|((iocon&0xFFFF)<<16))
#define GPIOPINDEF(port,pin,iocon,gpioPort,gpioPin) ((port&0xFF) | ((pin&0xFF)<<8) | ((iocon&0xFFFF)<<16) | ((gpioPort&0x07)<<24) | ((gpioPin&0x1F)<<27))
#define GETPORT(x)      (x       & 0xFF)
#define GETPIN(x)       ((x>>8)  & 0xFF)
#define GETFUNC(x)      ((x>>16) & 0xFFFF)
#define GETGPIOPORT(x)  ((x>>24) & 0x07)
#define GETGPIOPIN(x)   ((x>>27) & 0x1F)


#define GIMAPINDEF(port,pin,iocon,gimaSelect) ((port&0xFF) | ((pin&0xFF)<<8) | ((iocon&0xFFFF)<<16) | ((gimaSelect&0xFF)<<24))
#define GETGIMASELECT(x)   ((x>>24) & 0xFF)

#define PWM_PINDEF(port,pin,iocon,channel) ((port&0xFF) | ((pin&0xFF)<<8) | ((iocon&0xFFFF)<<16) | ((channel&0xFF)<<24))
#define GETPWMCHANNEL(x)   ((x>>24) & 0xFF)

#define SCT_PINDEF(port, pin, iocon, ctout) ((port&0xFF) | ((pin&0xFF)<<8) | ((iocon&0xFFFF)<<16) | ((ctout&0xFF)<<24))
#define GETCTOUT(x)         ((x>>24) & 0xFF)

// --- Encode leds a bit more compact
// ----------------------------------
#define LEDDEF(driver_id,r,g,b) ( ((driver_id&0x7F)<<24) | ((r&0xFF)<<16) | ((g&0xFF)<<8) | (b&0xFF))
#define LED_GET_DRIVER_ID(x) ((x>>24)&0x7f)

#define LED_WAS_UPDATED(x) (((x)&(1<<31))!=0)
#define LED_UPDATED(x,y) if (y) (x)|=(1<<31); else (x)&=~(1<<31);
#define GET_R(x) ((x>>16)&0xff)
#define GET_G(x) ((x>>8)&0xff)
#define GET_B(x) (x&0xff)

// --- a few FreeRTOS helpers
// --------------------------
#define TICKS_TO_MILLIS(ticks) ((ticks) * portTICK_RATE_MS)
#define MILLIS_TO_TICKS(millis) ((millis) / portTICK_RATE_MS)

// --- Some debug magic
// --------------------
extern void dbgprint(char *fmt, ...);

#ifdef DEBUG
#define DBG(x,...) dbgprint(x, ##__VA_ARGS__)
#define DBGQ(x,...) dbgprint(x, ##__VA_ARGS__)
#else
#define DBG(x,...) (void)x;
#endif
#ifdef DEBUG
#define CHECK(a,b) if (a!=b) { dbgprint("Check failed in %s (%s)",__FILE__,__func__); ASSERT(FALSE); }
#else
#define CHECK(a,b) b
#endif

// String printing macro ... XSTR(VAR) will preprint VAR value
// -----------------------------------------------------------
#define XSTR(x) STR(x)
#define STR(x) #x

// --- Simple event callbacks
// --------------------------
#define EVENT_CB(x) void (x)(uint32_t j)
#define CALLBACK(x,y) if ((x)) (x)((y))
#define EVENT_ISSET(x,y) (x&(1<<y))
#define EVENT_CLEAR(x,y) (x&=~(1<<y))

// --- Incantations for code marking
// ---------------------------------
#define PACKED __attribute__((packed))
#define WEAK __attribute__((weak))
#define ALIAS(f) __attribute__ ((weak, alias (#f)))
#define ALWAYS_INLINE inline __attribute__((always_inline))

#define __SECTION_EXT(type, bank, name) __attribute__ ((section("." #type ".$" #bank "." #name)))
#define __SECTION(type, bank) __attribute__ ((section("." #type ".$" #bank)))

// Macros to be used in preference to __RAM_FUNC to better match __DATA behaviour
#define __RAMFUNC_EXT(bank, name) __SECTION_EXT(ramfunc, bank, name)
#define __RAMFUNC(bank) __SECTION(ramfunc, bank)


// ============================================================================================
// ============================================================================================
// ============================================================================================
// Config Control Routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
struct calibFrame
{
  int16_t ofs[3];
  int16_t scale[3];
};

enum CalibFrameInstance { CF_Acc, CF_Mag, CF_Gyr, CF_Max };
#define AXIS_NAME (char *[]){"Acc","Mag","Gyr"}

typedef struct

{
    uint32_t versionNumber;
    uint32_t supplyV;
    uint32_t id;
    uint32_t x;
    uint32_t y;
    uint32_t z;
    uint32_t distcheckInterval;
    BOOL isNomadic;
  struct calibFrame c[CF_Max];
} ConfigType;

// FIXME - need a real ID
#define DEFAULT_CONFIG { \
      .versionNumber=VERSION_SEQ, \
      .distcheckInterval = 100, \
      .supplyV=3300, \
      .id=32, \
      .x=NOLOCATION, .y=NOLOCATION, .z=NOLOCATION, .isNomadic=TRUE,\
      .c[CF_Mag] = { .ofs={291,-135,289},  .scale={128,128,128} }, \
      .c[CF_Acc] = { .ofs={0,0,0},         .scale={128,128,128} }, \
      .c[CF_Gyr] = { .ofs={0,0,0},         .scale={128,128,128} }, \
	 }

extern ConfigType ConfigStore;
extern BOOL wasDefaulted;
extern BOOL isSaved;

ALWAYS_INLINE struct calibFrame *ConfigGetCalibFrame(enum CalibFrameInstance i)

{
  if (i>=CF_Max)
    return NULL;
  return &ConfigStore.c[i];
}
ALWAYS_INLINE BOOL ConfigWasDefaulted(void) { return wasDefaulted; }
ALWAYS_INLINE BOOL ConfigIsSaved(void) { return isSaved; }
ALWAYS_INLINE uint32_t ConfigVersionNumber(void) { return ConfigStore.versionNumber; }
ALWAYS_INLINE uint32_t ConfigSupplyV(void) { return ConfigStore.supplyV; }
ALWAYS_INLINE uint32_t ConfigDistcheckInterval(void) { return ConfigStore.distcheckInterval; }

ALWAYS_INLINE uint32_t ConfigID(void) { return ConfigStore.id; }
ALWAYS_INLINE uint32_t ConfigLocx(void) { return ConfigStore.x; }
ALWAYS_INLINE uint32_t ConfigLocy(void) { return ConfigStore.y; }
ALWAYS_INLINE uint32_t ConfigLocz(void) { return ConfigStore.z; }
ALWAYS_INLINE BOOL ConfigNomadic(void) { return ConfigStore.isNomadic; }


ALWAYS_INLINE BOOL ConfigSetdistcheckInterval(BOOL distcheckSet)
{
    isSaved = FALSE;
    ConfigStore.distcheckInterval = distcheckSet;
    return TRUE;
}

ALWAYS_INLINE BOOL ConfigSetID(uint32_t idSet)

{
    isSaved = FALSE;
    ConfigStore.id = idSet;
    return TRUE;
}

ALWAYS_INLINE BOOL ConfigSetCalibFrame(enum CalibFrameInstance i, int16_t ox, int16_t oy, int16_t oz, int16_t sx, int16_t sy, int16_t sz)

{
    if (i>=CF_Max)
        return FALSE;

    ConfigStore.c[i].ofs[0]=ox;
    ConfigStore.c[i].ofs[1]=oy;
    ConfigStore.c[i].ofs[2]=oz;
    ConfigStore.c[i].scale[0]=sx;
    ConfigStore.c[i].scale[1]=sy;
    ConfigStore.c[i].scale[2]=sz;
    isSaved=FALSE;
    return TRUE;
}

ALWAYS_INLINE BOOL ConfigSetNomadic(BOOL NomadicSet)
{
    isSaved = FALSE;
    ConfigStore.isNomadic = NomadicSet;
    return TRUE;
}

ALWAYS_INLINE BOOL ConfigSetSupplyV(uint32_t SupplyVSet)
{
    isSaved = FALSE;
    ConfigStore.supplyV = SupplyVSet;
    return TRUE;
}

ALWAYS_INLINE BOOL ConfigSetLocation(uint32_t xSet, uint32_t ySet) { ConfigStore.x=xSet; ConfigStore.y=ySet; return TRUE; }



void ConfigAssertDA(char *msg, char *file, uint32_t line);
BOOL ConfigCommit(void);
const uint32_t *ConfigGetSerialNumber(void);
void ConfigInit(void);

#endif

