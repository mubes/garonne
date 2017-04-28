// ============================================================================================
// ============================================================================================
// ============================================================================================
// Start of LPC4370 specific setup
// ============================================================================================
// ============================================================================================
// ============================================================================================

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

// ---- CAN Parameters (can.c)
// ---------------------------
#define CAN_BITRATE 	(500000)
#define CAN_IF			LPC_C_CAN0
#define CAN_INTERFACE 	(CCAN_MSG_IF1)
#define CAN_RD			PINDEF(0x3, 1, (SCU_MODE_FUNC2|SCU_MODE_INBUFF_EN))
#define CAN_TD			PINDEF(0x3, 2, (SCU_MODE_FUNC2))

// ----- Stats timer (stats.c & FreeRTOSConfig.h)
// ----------------------------------------------
#define STATS_TIMER LPC_TIMER3

// ----- I2C (i2chandler.c)
// ------------------------
#define I2C0_MODE 						I2C0_FAST_MODE_PLUS
#define I2C0_BITRATE                    (400000)
#define I2C0_INTPRIORITY                (7)

// ----- DIST setup (either dist.c or vldist.c)
// --------------------------------------------
#define MINDIST                         (150)                           /* Minimum distance at which vehicle will eStop */
#ifndef VL_DISTANCE
// ----- DIST setup (dist.c)
#ifdef AM1
#define DIST_TRIG_FRONT                 GPIOPINDEF(0x0E, 9, SCU_MODE_FUNC4, 7, 9)
#define DIST_CAP_FRONT                  GIMAPINDEF(0x05, 0, (SCU_MODE_FUNC5 | SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN), (2 << 4))        /* MOTOR[2].PWM pin used. (timer 1 capture input 0, GIMA select T1_CAP0) */
#define DIST_CAP_TIMER_INPUT_FRONT      (0)
#define DIST_TRIG_BACK                	GPIOPINDEF(0x0E,12, SCU_MODE_FUNC4, 7, 12)
#define DIST_CAP_BACK                 	GIMAPINDEF(0x02,13,(SCU_MODE_FUNC1 | SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN), (0 << 4))
#else
#ifdef AM2
#define DIST_TRIG_FRONT                 GPIOPINDEF(0x0E, 13, SCU_MODE_FUNC4, 7, 13) // PATCH required!!
#define DIST_CAP_FRONT                  GIMAPINDEF(0x05, 3, (SCU_MODE_FUNC5 | SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN), (1 << 4)) // patch required!    // T8 (timer 1 capture input 3, GIMA select T1_CAP3)
#define DIST_CAP_TIMER_INPUT_FRONT      3
#define DIST_TRIG_BACK                  GPIOPINDEF(0x05, 2, SCU_MODE_FUNC0, 2, 11) //ball R4
#define DIST_CAP_BACK                   GIMAPINDEF(0x02, 13, (SCU_MODE_FUNC1 | SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN), (0 << 4))      // C16 (Timer 1 capture input 2, GIMA select CTIN_4)
#else
#error "Unrecognised board version"
#endif
#endif
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



// Sound Output (audio.c)
// ----------------------
#define AUDIO_ENABLE  					GPIOPINDEF(0x01, 3, SCU_MODE_FUNC0, 0, 10)
#define AUDIO_OUT	  					GPIOPINDEF(0x04, 4, (SCU_MODE_FUNC0|SCU_MODE_PULLUP), 2, 4)
#define AUDIO_INT_PRIORITY				(7)

// Motor Encoder (rotenc.c)
// -------------------------
#define ROT_ENC_INT_PRIORITY            (6)
#define ROT_A                           GPIOPINDEF(0x0C, 13, (SCU_MODE_FUNC4 | SCU_MODE_INBUFF_EN | SCU_MODE_PULLUP), 6, 12)
#define ROT_A_CHANNEL                   6
#define ROT_A_HANDLER                   GPIO6_IRQHandler
#define ROT_A_IRQn                      PIN_INT6_IRQn
#define ROT_B                           GPIOPINDEF(0x04, 10, (SCU_MODE_FUNC4 | SCU_MODE_INBUFF_EN | SCU_MODE_PULLUP), 5, 14 )
#define ROT_B_CHANNEL                   7
#define ROT_B_HANDLER                   GPIO7_IRQHandler
#define ROT_B_IRQn                      PIN_INT7_IRQn

// Connection to HLB warning (gio.c)
// ---------------------------------
#define CONNECTED_TICKS                 (MILLIS_TO_TICKS(5000))

// HW version pins (gio.c)
// -----------------------
#define HW_VERSION_BIT_0				GPIOPINDEF(0x07, 3, (SCU_MODE_FUNC0 | SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN), 3, 11)
#define HW_VERSION_BIT_1				GPIOPINDEF(0x0E, 7, (SCU_MODE_FUNC4 | SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN), 7, 7)

// HW version pins (gio.c)
// -----------------------
#define HW_VERSION_BIT_0				GPIOPINDEF(0x07, 3, (SCU_MODE_FUNC0 | SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN), 3, 11)
#define HW_VERSION_BIT_1				GPIOPINDEF(0x0E, 7, (SCU_MODE_FUNC4 | SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN), 7, 7)

// ----- Motor Control (motor.c)
// -----------------------------
#ifdef AM1
#define MOTOR0_M0                       GPIOPINDEF(0x01, 1, (SCU_MODE_FUNC0 | SCU_MODE_PULLDOWN), 0, 8)
#define MOTOR_PWM_CHANNEL_MAP           {0}
#else
#ifdef AM2
#define MOTOR0_M0                       GPIOPINDEF(0x0D, 5, (SCU_MODE_FUNC4 | SCU_MODE_PULLDOWN), 6, 19)
#else
#error "Unrecognised board version"
#endif
#endif

#define MOTOR0_M1                       GPIOPINDEF(0x01, 0, (SCU_MODE_FUNC0 | SCU_MODE_PULLDOWN), 0, 4)
#define MOTOR0_PIN                      PWM_PINDEF(0x05, 4, (SCU_MODE_FUNC1), 0)       // MCOB0
#define MOTOR_PWM                       LPC_MCPWM

// ----- Servo Setup (servo.c)
// ---------------------------                 port  pin  func            ctout
#ifdef AM1
#define SERVO_PIN                       PINDEF(0x05,7,SCU_MODE_FUNC1)   /*  MCOA2 */
#define SERVO_PWM                       LPC_MCPWM
#define SERVO_PWM_CHANNEL_MAP           {2}                             /* Servo to channel mapping (uses channel 2 (MCOA2))*/

#else
#ifdef AM2
#define SERVO_PIN                       SCT_PINDEF(0x0B, 3,   SCU_MODE_FUNC5, 8)
#define SERVO_PWM                       LPC_SCT
#else
#error "Unrecognised board version"
#endif
#endif

// ----- LSM9DS1 9D sensor Setup (nined.c)
// ---------------------------------------

#define ACC_GYR_SENSOR_ID               (0x6A) /* SA0 strapping */
#define MAG_SENSOR_ID                   (0x1C) /* SA0 strapping */
#define NINED_DEN_A_G                   GPIOPINDEF(0x07, 0, (SCU_MODE_FUNC0 | SCU_MODE_INBUFF_EN | SCU_MODE_INACT), 3, 8)
#define NINED_DRDY_M                    GPIOPINDEF(0x07, 2, (SCU_MODE_FUNC0 | SCU_MODE_INBUFF_EN | SCU_MODE_INACT), 3, 10)
#define NINED_INT_M                     GPIOPINDEF(0x0B, 1, (SCU_MODE_FUNC4 | SCU_MODE_INBUFF_EN | SCU_MODE_INACT), 5, 21)
#ifdef AM1
#define NINED_INT1_A_G                  GPIOPINDEF(0x0E, 9, (SCU_MODE_FUNC4 | SCU_MODE_INBUFF_EN | SCU_MODE_INACT), 7, 9)
#else
#ifdef AM2
#define NINED_INT1_A_G                  GPIOPINDEF(0x0B, 5, (SCU_MODE_FUNC4 | SCU_MODE_INBUFF_EN | SCU_MODE_INACT), 5, 25)
#else
#error "Unrecognised board version"
#endif
#endif

#define NINED_WAKEUP_DELAY				MILLIS_TO_TICKS(200)	/* Time needed for NINED to wake up ... very conservative */
#define NINED_INT2_A_G                  GPIOPINDEF(0x0B, 5, (SCU_MODE_FUNC4 | SCU_MODE_INBUFF_EN | SCU_MODE_INACT), 5, 25)






/* OBSOLETE: interrupt pin setup, and LED elements
#define IR0_RX0                         GPIOPINDEF(0x0E, 15,(SCU_MODE_FUNC4 | SCU_MODE_ZIF_DIS | SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN), 7, 15)
#define IR0_RX1                         GPIOPINDEF(0x06, 5, (SCU_MODE_FUNC0 | SCU_MODE_ZIF_DIS | SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN), 3, 4)
#define IR0_RX2                         GPIOPINDEF(0x0E, 4, (SCU_MODE_FUNC4 | SCU_MODE_ZIF_DIS | SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN), 7, 4)
#define IR0_RX3                         GPIOPINDEF(0x06, 3, (SCU_MODE_FUNC0 | SCU_MODE_ZIF_DIS | SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN), 3, 2)
#define IR0_RX4                         GPIOPINDEF(0x0E, 14,(SCU_MODE_FUNC4 | SCU_MODE_ZIF_DIS | SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN), 7, 14)
#define IR0_RX5                         GPIOPINDEF(0x0A, 1, (SCU_MODE_FUNC0 | SCU_MODE_ZIF_DIS | SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN), 4, 8)

#define IR1_RX0                         GPIOPINDEF(0x08, 0, (SCU_MODE_FUNC0 | SCU_MODE_ZIF_DIS | SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN), 4, 0)
#define IR1_RX1                         GPIOPINDEF(0x04, 8, (SCU_MODE_FUNC4 | SCU_MODE_ZIF_DIS | SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN), 5, 12)
#define IR1_RX2                         GPIOPINDEF(0x08, 5, (SCU_MODE_FUNC0 | SCU_MODE_ZIF_DIS | SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN), 4, 5)
#define IR1_RX3                         GPIOPINDEF(0x08, 4, (SCU_MODE_FUNC0 | SCU_MODE_ZIF_DIS | SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN), 4, 4)
#define IR1_RX4                         GPIOPINDEF(0x08, 7, (SCU_MODE_FUNC0 | SCU_MODE_ZIF_DIS | SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN), 4, 7)
#define IR1_RX5                         GPIOPINDEF(0x08, 3, (SCU_MODE_FUNC0 | SCU_MODE_ZIF_DIS | SCU_MODE_INBUFF_EN | SCU_MODE_PULLDOWN), 4, 3)
*/

#define IR0_LED_R                       GPIOPINDEF(0x06, 9,  SCU_MODE_FUNC0, 3, 5)
#define IR0_LED_G                       GPIOPINDEF(0x0D, 14, SCU_MODE_FUNC4, 6, 28)
#define IR0_LED_B                       GPIOPINDEF(0x02, 6,  SCU_MODE_FUNC4, 5, 6)

#define IR1_LED_R                       GPIOPINDEF(0x08, 6,  SCU_MODE_FUNC0, 4, 6)
#define IR1_LED_G                       GPIOPINDEF(0x04, 2,  SCU_MODE_FUNC0, 2, 2)
#define IR1_LED_B                       GPIOPINDEF(0x04, 6,  SCU_MODE_FUNC0, 2, 6)


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

/* For J5 connection via SSP - remember to change definitions for serial0
 * ======================================================================
 */
#define DW_MISO                    		PINDEF(0x01, 3, (SCU_MODE_FUNC5 | SCU_MODE_INBUFF_EN))
#define DW_MOSI                    		PINDEF(0x01, 4, (SCU_MODE_FUNC5))
#define DW_SEL                     		GPIOPINDEF(0x01, 5, (SCU_MODE_FUNC0 | SCU_MODE_INBUFF_EN),1, 8)
#define DW_CLK                     		PINDEF(0x0f, 4, (SCU_MODE_FUNC0))
#define DW_RST                     		GPIOPINDEF(0x02, 1, (SCU_MODE_FUNC4|SCU_MODE_INACT | SCU_MODE_INBUFF_EN), 5, 1)
#define DW_INT                     		GPIOPINDEF(0x06, 4, (SCU_MODE_FUNC0|SCU_MODE_INBUFF_EN|SCU_MODE_ZIF_DIS|SCU_MODE_PULLDOWN), 3, 3)


/* For J6 connection and bit bashing
 * =================================
 */
//#define DW_MISO                    		GPIOPINDEF(0x01, 16, (SCU_MODE_FUNC0 | SCU_MODE_INBUFF_EN), 0, 3)  /* Ball M7 */
//#define DW_MOSI                    		GPIOPINDEF(0x07, 1, (SCU_MODE_FUNC5), 3, 9)  /* Ball C14 */
//#define DW_SEL                     		GPIOPINDEF(0x02, 2, (SCU_MODE_FUNC4 | SCU_MODE_INBUFF_EN), 5, 2)  /* Ball M15 */
//#define DW_CLK                     		GPIOPINDEF(0x04, 5, (SCU_MODE_FUNC0), 2, 5)  /* Ball D2 */
//#define DW_RST                     		GPIOPINDEF(0x02, 4, (SCU_MODE_FUNC4| SCU_MODE_INACT | SCU_MODE_INBUFF_EN), 5, 4) /* Ball K11 */
//#define DW_INT                     		GPIOPINDEF(0x02, 3, (SCU_MODE_FUNC4|SCU_MODE_INBUFF_EN|SCU_MODE_ZIF_DIS|SCU_MODE_PULLDOWN), 5, 3) /* Ball J12 */


// --- RGB LEDS
// ---------------------------------
#ifdef AM1
#define FRONT_LEFT_LED_DRIVER_ID        0x0A /* U13  */
#define FRONT_RIGHT_LED_DRIVER_ID       0x4A /* U14  */
#define BACK_RIGHT_LED_DRIVER_ID        0x2A /* U15  */
#define BACK_LEFT_LED_DRIVER_ID         0x62 /* U16  */
#else
#ifdef AM2
#define FRONT_LEFT_LED_DRIVER_ID        0x28 // U13
#define FRONT_RIGHT_LED_DRIVER_ID       0x29 // U14  This clashes with the VL Distance sensors, but we move those away
#define BACK_RIGHT_LED_DRIVER_ID        0x2A // U15
#define BACK_LEFT_LED_DRIVER_ID         0x2B // U16
#else
#error "Unrecognised board version"
#endif
#endif

/*                                              I2C slave addr            R  G  B   */
#define RGBLED0                         LEDDEF(FRONT_RIGHT_LED_DRIVER_ID, 0, 1, 2)  // front right outside
#define RGBLED1                         LEDDEF(FRONT_RIGHT_LED_DRIVER_ID, 3, 4, 5)  // front right inside
#define RGBLED2                         LEDDEF(FRONT_LEFT_LED_DRIVER_ID,  3, 4, 5)  // front left outside
#define RGBLED3                         LEDDEF(FRONT_LEFT_LED_DRIVER_ID,  0, 1, 2)  // front left inside
#define RGBLED4                         LEDDEF(BACK_RIGHT_LED_DRIVER_ID,  0, 1, 2)  // back right outside
#define RGBLED5                         LEDDEF(BACK_RIGHT_LED_DRIVER_ID,  3, 4, 5)  // back right inside
#define RGBLED6                         LEDDEF(BACK_LEFT_LED_DRIVER_ID,   3, 4, 5)  // back left outside
#define RGBLED7                         LEDDEF(BACK_LEFT_LED_DRIVER_ID,   0, 1, 2)  // back left inside

// --- DEBUG LEDS (gio.c)
// ----------------------
#ifdef AM1
#define DEBUG_LED0                      GPIOPINDEF(0x05, 1,  (SCU_MODE_FUNC0 | SCU_MODE_PULLDOWN), 2, 10)
#define DEBUG_LED1                      GPIOPINDEF(0x05, 2,  (SCU_MODE_FUNC0 | SCU_MODE_PULLDOWN), 2, 11)
#define DEBUG_LED2                      GPIOPINDEF(0x0D, 3,  (SCU_MODE_FUNC4 | SCU_MODE_PULLDOWN), 6, 17)
#define DEBUG_LED3                      GPIOPINDEF(0x0D, 6,  (SCU_MODE_FUNC4 | SCU_MODE_PULLDOWN), 6, 20)
#define DEBUG_LED4                      GPIOPINDEF(0x01, 9,  (SCU_MODE_FUNC0 | SCU_MODE_PULLDOWN), 1, 2)
#define DEBUG_LED5                      GPIOPINDEF(0x05, 3,  (SCU_MODE_FUNC0 | SCU_MODE_PULLDOWN), 2, 12)
#else
#ifdef AM2
#define DEBUG_LED0                      GPIOPINDEF(0x05, 1,  (SCU_MODE_FUNC0 | SCU_MODE_PULLDOWN), 2, 10)
#define DEBUG_LED1                      GPIOPINDEF(0x01, 8,  (SCU_MODE_FUNC0 | SCU_MODE_PULLDOWN), 1, 1)
#define DEBUG_LED2                      GPIOPINDEF(0x0D, 3,  (SCU_MODE_FUNC4 | SCU_MODE_PULLDOWN), 6, 17)
#define DEBUG_LED3                      GPIOPINDEF(0x0D, 6,  (SCU_MODE_FUNC4 | SCU_MODE_PULLDOWN), 6, 20)
#define DEBUG_LED4                      GPIOPINDEF(0x01, 9,  (SCU_MODE_FUNC0 | SCU_MODE_PULLDOWN), 1, 2)
#define DEBUG_LED5                      GPIOPINDEF(0x01, 11, (SCU_MODE_FUNC0 | SCU_MODE_PULLDOWN), 1, 4)
#else
#error "Unrecognised board version"
#endif
#endif

/* LED function allocations to pins */
#define HB_LED							(DBG5_LED)
#define USER_BUTTON_LED					(DBG4_LED)
#define DW_ANNLED						(DBG3_LED)
#define DW_RXLED                        (DBG2_LED)
#define SD_RD_ACCESS_LED				(DBG1_LED)
#define SD_WR_ACCESS_LED				(DBG0_LED)


// ----- UART configuration (uarthandler.c)
// ----------------------------------------

#define UART0_TX                        PINDEF(0x09, 6,SCU_MODE_FUNC2)
#define UART0_RX                        PINDEF(0x09, 5,(SCU_MODE_FUNC7 | SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS))
#define UART1_TX                        PINDEF(0x0E,11,SCU_MODE_FUNC2)
#define UART1_RX                        PINDEF(0x01,14,(SCU_MODE_FUNC1 | SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS))
#define UART2_TX                        PINDEF(0x02,10,SCU_MODE_FUNC2)      /* J32 pin4 */
#define UART2_RX                        PINDEF(0x02,11,(SCU_MODE_FUNC2 | SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS)) /* J32 pin3 */

/* Normally UART3 would be on P2_3 & P2_4 but these are used for I2C1 in AM1 & AM2, so we move it */

#ifndef AM3
#define UART3_TX                        PINDEF(0x0F, 2,SCU_MODE_FUNC1)
#define UART3_RX                        PINDEF(0x0F, 3,(SCU_MODE_FUNC1 | SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS))
#else
#define UART3_TX                        PINDEF(0x02, 3,SCU_MODE_FUNC2)
#define UART3_RX                        PINDEF(0x02, 4,(SCU_MODE_FUNC2 | SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS))
#endif

#define UART_INTPRIORITY                (7)



// ---- Serial port that is used for HLB communication (lmsmsg.c)
// --------------------------------------------------------------
#define LLB_IF_PORT                     SERPORT_UART1 /* raspberry header (J4) */
#define LLB_IF_BAUDRATE                 (115200)

// ---- Smoke Maker (Used for Camera power at the moment) (gio.c)
// --------------------------------------------------------------
#define SMOKE                           GPIOPINDEF(0x0B,4, (SCU_MODE_FUNC4 | SCU_MODE_INBUFF_EN | SCU_MODE_INACT), 5, 24)
#define ISPMODE                         GPIOPINDEF(0x02,7, (SCU_MODE_FUNC4 | SCU_MODE_INBUFF_EN | SCU_MODE_INACT), 0, 7)

/* User button setup for interrupt */
#define USERBUTTON						GPIOPINDEF(0x06,1, (SCU_MODE_FUNC0 | SCU_MODE_INBUFF_EN | SCU_MODE_PULLUP), 3, 0)
#define USERBUTTON_PININT_INDEX			(5)
#define USERBUTTON_IRQ_HANDLER  		GPIO5_IRQHandler
#define USERBUTTON_NVIC_IRQ	    		PIN_INT5_IRQn
#define USERBUTTON_NVIC_PRIORITY		(7)

// ---- Serial port that is used for the debug & interactive terminal (ui.c)
// -------------------------------------------------------------------------

#define TERMINAL_PORT                   SERPORT_UART2  /* GPIO header (J32) */
#define TERMINAL_BAUDRATE               (115200)
#define TERMINAL_PROMPT					">"

// --- ADC Configuration for Battery Level (mainloop.c)
// ----------------------------------------------------
#define BATTERY_ADC_PORT            LPC_ADC0
#define BATTERY_ADC_CHANNEL         ADC_CH1
#define BATTERY_PIN                 GPIOPINDEF(0x04 , 1, (SCU_MODE_FUNC0 | SCU_MODE_INBUFF_EN | SCU_MODE_INACT), 2, 1)

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
// End of LPC4370 specific setup
// ============================================================================================
// ============================================================================================
// ============================================================================================
