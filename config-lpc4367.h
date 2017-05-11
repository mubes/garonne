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
 */

// ============================================================================================
// ============================================================================================
// ============================================================================================
// Start of LPC4367 specific setup
// ============================================================================================
// ============================================================================================
// ============================================================================================

#define HARDWARE_VERSION  (3)  // 0, 1 & 2 were AM0, AM1 & AM2. There are no sense pins on this board, so set in software */

// ----- Clocks (llb_init.c)
// -------------------------
#define CLOCK0    PINDEF(0, 0,  (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC0))
#define CLOCK1    PINDEF(0, 1,  (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC0))
#define CLOCK2    PINDEF(0, 2,  (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC0))
#define CLOCK3    PINDEF(0, 3,  (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC0))

// ----- FLASH (llb_init.c)
// ------------------------
#define SPIFI_SCK    PINDEF(0x3, 3,  (SCU_PINIO_FAST | SCU_MODE_FUNC3))
#define SPIFI_SIO3   PINDEF(0x3, 4,  (SCU_PINIO_FAST | SCU_MODE_FUNC3))
#define SPIFI_SIO2   PINDEF(0x3, 5,  (SCU_PINIO_FAST | SCU_MODE_FUNC3))
#define SPIFI_MISO   PINDEF(0x3, 6,  (SCU_PINIO_FAST | SCU_MODE_FUNC3))
#define SPIFI_MOSI   PINDEF(0x3, 7,  (SCU_PINIO_FAST | SCU_MODE_FUNC3))
#define SPIFI_CS     PINDEF(0x3, 8,  (SCU_PINIO_FAST | SCU_MODE_FUNC3))

// ----- Stats timer (stats.c & FreeRTOSConfig.h)
// ----------------------------------------------
#define STATS_TIMER LPC_TIMER3

// ----- I2C (i2chandler.c)
// ------------------------
#define I2C0_MODE 			            I2C0_FAST_MODE_PLUS
#define I2C0_BITRATE                    (400000)
#define I2C0_INTPRIORITY                (7)

// ----- DIST setup (either dist.c or vldist.c)
// --------------------------------------------
#define MINDIST                         (150)                           /* Minimum distance at which vehicle will eStop */

#ifndef VL_DISTANCE
// ----- DIST setup (dist.c)
// -------------------------
#define DIST_TRIG_FRONT                 GPIOPINDEF(0x0E, 13, SCU_MODE_FUNC4, 7, 13) // PATCH required!!
#define DIST_CAP_FRONT                  GIMAPINDEF(0x05, 3, (SCU_MODE_FUNC5 | SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN), (1 << 4)) // patch required!    // T8 (timer 1 capture input 3, GIMA select T1_CAP3)
#define DIST_CAP_TIMER_INPUT_FRONT      3
#define DIST_TRIG_BACK                  GPIOPINDEF(0x05, 2, SCU_MODE_FUNC0, 2, 11) //ball R4
#define DIST_CAP_BACK                   GIMAPINDEF(0x07, 13, (SCU_MODE_FUNC1 | SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN), (0 << 4))      // C16 (Timer 1 capture input 2, GIMA select CTIN_4)

#define DIST_CAP_TIMER_INPUT_BACK       (2)
#define DIST_INTPRIORITY                (7)
#define DIST_SAMPLE_RATE                20                                // 20 Hz
#define DIST_TIMER                      LPC_TIMER1                      /* Timer used for distance measurement*/
#define DIST_TIMER_INTERRUPT            TIMER1_IRQHandler               /* Ensure this matches with the line below!! */
#define DIST_TIMER_IRQ                  TIMER1_IRQn                     /* Ensure this matches with the line below!! */
#else
// ----- VL Distance Sensor (vldist.c)
// -----------------------------------

enum VLDIST_CHANNEL_ENUM {VLDIST_FRONT, VLDIST_REAR, VLDISTNONE };

#define NUM_VLS                         (VLDISTNONE)

#define VLS0ID                          0x10
#define VLS0EN                          GPIOPINDEF(0x01, 16, SCU_MODE_FUNC0, 0, 3)
#define VLS1ID                          0x11
#define VLS1EN                          GPIOPINDEF(0x07, 1, SCU_MODE_FUNC0, 3, 9)

#define VLSLIST                         {VLS0ID,VLS0EN},{VLS1ID,VLS1EN}

#endif


// Motor Encoder (rotenc.c)
// -------------------------
#define ROT_ENC_INT_PRIORITY            (6)
#define ROT_A                           GPIOPINDEF(0x2, 2, (SCU_MODE_FUNC4 | SCU_MODE_INBUFF_EN | SCU_MODE_PULLUP), 5, 2)
#define ROT_A_CHANNEL                   5
#define ROT_A_HANDLER                   GPIO5_IRQHandler
#define ROT_A_IRQn                      PIN_INT5_IRQn

#define ROT_B                           GPIOPINDEF(0x1, 8, (SCU_MODE_FUNC0 | SCU_MODE_INBUFF_EN | SCU_MODE_PULLUP), 1, 1 )
#define ROT_B_CHANNEL                   3
#define ROT_B_HANDLER                   GPIO3_IRQHandler
#define ROT_B_IRQn                      PIN_INT3_IRQn

// Connection to HLB warning (gio.c)
// ---------------------------------
#define CONNECTED_TICKS                 (MILLIS_TO_TICKS(5000))

// ----- Motor Control (motor.c)
// -----------------------------
#define MOTOR0_M0                       GPIOPINDEF(0x2,12, (SCU_MODE_FUNC0 | SCU_MODE_PULLDOWN), 1, 12)
#define MOTOR0_M1                       GPIOPINDEF(0x2, 9, (SCU_MODE_FUNC0 | SCU_MODE_PULLDOWN), 1, 10)
#define MOTOR0_PIN                      SCT_PINDEF(0x6, 5,   SCU_MODE_FUNC1, 6)
#define SERVO_PIN                       SCT_PINDEF(0x1, 5,   SCU_MODE_FUNC1, 10)

// ----- MPU9250 9D sensor Setup (nined.c)
// ---------------------------------------

#define NINED_INT                       GPIOPINDEF(0x2,13, (SCU_MODE_FUNC0 | SCU_MODE_INBUFF_EN | SCU_MODE_INACT), 1,13)
#define NINED_SENSOR_ID                 (0x68)
#define MAG_SENSOR_ID                   (0x0C)
#define NINED_SENSOR_SPEED              (400000)
#define NINED_SDA                       PINDEF(0x2, 3, (SCU_MODE_FUNC1 | SCU_MODE_ZIF_DIS | SCU_MODE_INBUFF_EN))
#define NINED_SCL                       PINDEF(0x2, 4, (SCU_MODE_FUNC1 | SCU_MODE_ZIF_DIS | SCU_MODE_INBUFF_EN))
#define MAG_WAKEUP_TIME                 (MILLIS_TO_TICKS(500))
#define GYRO_WAKEUP_TIME                (MILLIS_TO_TICKS(100))

// DWM1000 module (dw1000.c &dwif.c)
// ---------------------------------

//#define BE_VERBOSE  // Define if you want information about the success of transmission
#define RANGE_ON_TERMINAL  // Define if you want ranging information to be output
//#define REPORT_STATE_TRANSITIONS // Define to get information about state changes

#define MAX_FRAME_SIZE 					127
#define DW_SPI_PORT                     LPC_SSP1
#define DW_SSP_CLKINDEX                 SYSCTL_CLOCK_SSP1
#define DW_SSP_RESETINDEX               RESET_SSP1

#define DW_PININT_HANDLER               GPIO4_IRQHandler
#define DW_PININT_CHANNEL               (4)
#define DW_PININT_NVIC_NAME     		PIN_INT4_IRQn
#define DW_PININT_PRIORITY              (5)
#define DW_LOW_SPI_SPEED 				(3000000)
#define DW_HIGH_SPI_SPEED 				(10000000)

// Pin Allocations...
#define DW_MISO                    		PINDEF(0x01, 3, (SCU_MODE_FUNC5 | SCU_MODE_INBUFF_EN))
#define DW_MOSI                    		PINDEF(0x01, 4, (SCU_MODE_FUNC5))
#define DW_SEL                     		GPIOPINDEF(0x06, 1, (SCU_MODE_FUNC0),3, 0)
#define DW_CLK                     		PINDEF(0x0f, 4, (SCU_MODE_FUNC0))
#define DW_INT                     		GPIOPINDEF(0x03, 5, (SCU_MODE_FUNC0|SCU_MODE_INBUFF_EN|SCU_MODE_ZIF_DIS|SCU_MODE_PULLDOWN), 1, 15)
#define DW_RST                       GPIOPINDEF(0x2, 6, (SCU_MODE_FUNC4 | SCU_MODE_INBUFF_EN | SCU_MODE_INACT), 5, 6)


// --- DEBUG LEDS (gio.c)
// ----------------------

#define DEBUG_LED0                      GPIOPINDEF(0x01, 1, (SCU_MODE_FUNC0 | SCU_MODE_PULLDOWN), 0, 8)
#define RGB_LED_B                       GPIOPINDEF(0x06, 9, SCU_MODE_FUNC0, 3, 5)
/* This pin does double duty as the ISP switch, so it needs its pullup on */
#define RGB_LED_G                       USERBUTTON
#define RGB_LED_R                       GPIOPINDEF(0x06,11, SCU_MODE_FUNC0, 3, 7)

/* LED function allocations to pins */
#define HB_LED							(DBG0_LED)

#define USER_BUTTON_LED					(DBG0_LED)
#define DW_ANNLED						(DBG0_LED)
#define DW_RXLED                        (DBG0_LED)
#define SD_RD_ACCESS_LED				(DBG0_LED)
#define SD_WR_ACCESS_LED				(DBG0_LED)

/*                                              I2C slave addr            R  G  B   */
#define RGBLED0                         LEDDEF(FRONT_RIGHT_LED_DRIVER_ID, 0, 1, 2)  // front right outside
#define RGBLED1                         LEDDEF(FRONT_RIGHT_LED_DRIVER_ID, 3, 4, 5)  // front right inside
#define RGBLED2                         LEDDEF(FRONT_LEFT_LED_DRIVER_ID,  3, 4, 5)  // front left outside
#define RGBLED3                         LEDDEF(FRONT_LEFT_LED_DRIVER_ID,  0, 1, 2)  // front left inside
#define RGBLED4                         LEDDEF(BACK_RIGHT_LED_DRIVER_ID,  0, 1, 2)  // back right outside
#define RGBLED5                         LEDDEF(BACK_RIGHT_LED_DRIVER_ID,  3, 4, 5)  // back right inside
#define RGBLED6                         LEDDEF(BACK_LEFT_LED_DRIVER_ID,   3, 4, 5)  // back left outside
#define RGBLED7                         LEDDEF(BACK_LEFT_LED_DRIVER_ID,   0, 1, 2)  // back left inside

#define FRONT_LEFT_LED_DRIVER_ID        0x28 // U13
#define FRONT_RIGHT_LED_DRIVER_ID       0x29 // U14  This clashes with the VL Distance sensors, but we move those away
#define BACK_RIGHT_LED_DRIVER_ID        0x2A // U15
#define BACK_LEFT_LED_DRIVER_ID         0x2B // U16ß
// ----- UART configuration (uarthandler.c)
// ----------------------------------------

/* TX is FROM the MCU to the Host (i.e. it's an output) */

/* This is the FTDI Header: Make sure JP11 is fitted to allow P6_4 to be the TX Pin!! */
#define UART0_TX                        PINDEF(0x06, 4,(SCU_MODE_FUNC2 | SCU_MODE_INACT))
#define UART0_RX                        PINDEF(0x02, 1,(SCU_MODE_FUNC1 | SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS))

/* This is J2 18 & 19 on the Arduino header */
#define UART1_TX                        PINDEF(0x01,13,SCU_MODE_FUNC1)
#define UART1_RX                        PINDEF(0x01,14,(SCU_MODE_FUNC1 | SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS))

/* This is J2 13 & 15 on the Arduino header */
#define UART2_TX                        PINDEF(0x02,10,SCU_MODE_FUNC2)
#define UART2_RX                        PINDEF(0x02,11,(SCU_MODE_FUNC2 | SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS))

/* This is not pinned on the 100 pin package, but it can be put onto P2_3/P2_4 if you don't need I2C there */
#define UART3_TX                        PINDEF(0x04, 1,SCU_MODE_FUNC6)
#define UART3_RX                        PINDEF(0x04, 2,(SCU_MODE_FUNC6 | SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS))

#define UART_INTPRIORITY                (7)


// ---- Serial port that is used for HLB communication (lmsmsg.c)
// --------------------------------------------------------------
#define LLB_IF_PORT                     SERPORT_UART1
#define LLB_IF_BAUDRATE                 (115200)

// ---- Serial port that is used for the debug & interactive terminal (ui.c)
// -------------------------------------------------------------------------

#define TERMINAL_PORT                   SERPORT_USB
#define TERMINAL_BAUDRATE               (115200)
#define TERMINAL_PROMPT                 ">"

// ---- Smoke Maker (Used for Camera power at the moment) (gio.c)
// --------------------------------------------------------------
#define SMOKE                           GPIOPINDEF(0x1,7, (SCU_MODE_FUNC0 | SCU_MODE_INACT), 1, 0)

/* User button setup for interrupt */
#define USERBUTTON						GPIOPINDEF(0x02,7, (SCU_MODE_FUNC0 | SCU_MODE_INBUFF_EN | SCU_MODE_PULLUP), 0, 7)

// --- ADC Configuration for Battery Level (mainloop.c)
// ----------------------------------------------------
#define BATTERY_ADC_PORT            LPC_ADC0
#define BATTERY_ADC_CHANNEL         ADC_CH0

#ifdef INCLUDE_SDMMC
// --- SDMMC Configuration (sdif.c)
// --------------------------------

#define SDMMC_SDIO_D0 				PINDEF(0xC, 4, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_PULLUP | SCU_MODE_FUNC7))
#define SDMMC_SDIO_D1				PINDEF(0xC, 5, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_PULLUP | SCU_MODE_FUNC7))
#define SDMMC_SDIO_D2				PINDEF(0xC, 6, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_PULLUP | SCU_MODE_FUNC7))
#define SDMMC_SDIO_D3				PINDEF(0xC, 7, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_PULLUP | SCU_MODE_FUNC7))
#define SDMMC_CLKPIN				(0)
#define SDMMC_CLKCFG				(SCU_MODE_INACT | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INBUFF_EN | SCU_MODE_FUNC4)
#define SDMMC_SDIO_CMD				PINDEF(1, 6, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_PULLUP | SCU_MODE_FUNC7))
#define SDIO_INTPRIORITY			(7)
#endif

#ifdef INCLUDE_ETHERNET
// ----- Ethernet (enet.c)
// -----------------------


#define ENET_TXD0 PINDEF(0x1, 18, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INACT | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC3))
#define ENET_TXD1 PINDEF(0x1, 20, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INACT | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC3))
#define ENET_TXEN PINDEF(0x0, 1,  (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INACT | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC6))
#define ENET_RXD0 PINDEF(0x1, 15, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC3))
#define ENET_RXD1 PINDEF(0x0, 0,  (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC2))
#define ENET_RX_DV PINDEF(0xC, 8, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC3))
#define ENET_MDIO PINDEF(0x1, 17, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC3))
#define ENET_MDC  PINDEF(0x2, 0,  (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_PULLUP | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC7))
#define ENET_TXCLK PINDEF(0x1, 19, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC0))
#define ENET_RESET GPIOPINDEF(0xD, 13, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC4),6,27)

#define BOARD_ENET_PHY_ADDR 0x00            /* Address of the RMII Phy */
#define ENET_INTERRUPT_PRIORITY (6)
#define USE_RMII
#endif

#ifdef INCLUDE_CAN
// ---- CAN Parameters (can.c)
// ---------------------------
#define CAN_BITRATE     (500000)
#define CAN_IF          LPC_C_CAN0
#define CAN_INTERFACE   (CCAN_MSG_IF1)
#define CAN_RD          PINDEF(0x3, 1, (SCU_MODE_FUNC2|SCU_MODE_INBUFF_EN))
#define CAN_TD          PINDEF(0x3, 2, (SCU_MODE_FUNC2))
#endif


#ifdef INCLUDE_AUDIO
// Sound Output (audio.c)
// ----------------------
#define AUDIO_ENABLE                    GPIOPINDEF(0x01, 3, SCU_MODE_FUNC0, 0, 10)
#define AUDIO_OUT                       GPIOPINDEF(0x04, 4, (SCU_MODE_FUNC0|SCU_MODE_PULLUP), 2, 4)
#define AUDIO_INT_PRIORITY              (7)
#endif


// Radio subsystem (radio.c)
// -------------------------
/* Addressing constructs for vehicles and infrastructure */
#define NOENTITY            0
#define NODISTANCE          0
#define NOLOCATION          (0xFFFF)
#define VEHICLEFLAG         0x8000
#define ALL                 (0x7FFF)
#define BEACON_ADDRESS(x)   (x&~VEHICLEFLAG)
#define VEHICLE_ADDRESS(x)  (x|VEHICLEFLAG)

#define ISA_BEACON(x)       (!(x&VEHICLEFLAG))
#define ISA_VEHICLE(x)      (!ISA_BEACON(x))

#define PURE_ADDRESS(x) (x&~VEHICLEFLAG)

// Sets the frame response timeout size ... keep as low as possible */
#define NUM_ADDRESSES         (8)

// ============================================================================================
// ============================================================================================
// ============================================================================================
// End of LPC4367 specific setup
// ============================================================================================
// ============================================================================================
// ============================================================================================
