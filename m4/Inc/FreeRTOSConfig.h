/*
    FreeRTOS V9.0.0 - Copyright (C) 2014 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that has become a de facto standard.             *
     *                                                                       *
     *    Help yourself get started quickly and support the FreeRTOS         *
     *    project by purchasing a FreeRTOS tutorial book, reference          *
     *    manual, or both from: http://www.FreeRTOS.org/Documentation        *
     *                                                                       *
     *    Thank you!                                                         *
     *                                                                       *
    ***************************************************************************

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    >>! NOTE: The modification to the GPL is included to allow you to distribute
    >>! a combined work that includes FreeRTOS without being obliged to provide
    >>! the source code for proprietary components outside of the FreeRTOS
    >>! kernel.

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available from the following
    link: http://www.freertos.org/a00114.html

    1 tab == 4 spaces!

    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?"                                     *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org - Documentation, books, training, latest versions,
    license and Real Time Engineers Ltd. contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.OpenRTOS.com - Real Time Engineers ltd license FreeRTOS to High
    Integrity Systems to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/


#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "chip.h"


extern uint32_t SystemCoreClock;

#define configUSE_PREEMPTION            1
#define configUSE_IDLE_HOOK             1
#define configUSE_TICK_HOOK             0
#define configCPU_CLOCK_HZ              ( SystemCoreClock )
#define configTICK_RATE_HZ              ( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES            ( 4 )
#define configMINIMAL_STACK_SIZE        ( ( unsigned short ) 100 )
#define configTOTAL_HEAP_SIZE           ( ( size_t ) ( 17000 ) )
#define configMAX_TASK_NAME_LEN         ( 8 )
#define configUSE_TRACE_FACILITY        0
#define configUSE_16_BIT_TICKS          0
#define configIDLE_SHOULD_YIELD         0
#define configUSE_MUTEXES               1
#define configQUEUE_REGISTRY_SIZE       0
#define configCHECK_FOR_STACK_OVERFLOW  2
#define configUSE_RECURSIVE_MUTEXES     0
#define configUSE_MALLOC_FAILED_HOOK    1
#define configUSE_APPLICATION_TASK_TAG  0
#define configUSE_COUNTING_SEMAPHORES   1
#define configGENERATE_RUN_TIME_STATS   0

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES           0
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/* Software timer definitions. */
#define configUSE_TIMERS                1
#define configTIMER_TASK_PRIORITY       ( 2 )
#define configTIMER_QUEUE_LENGTH        20
#define configTIMER_TASK_STACK_DEPTH    ( 512 )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_vTaskPrioritySet        1
#define INCLUDE_uxTaskPriorityGet       1
#define INCLUDE_vTaskDelete             1
#define INCLUDE_vTaskCleanUpResources   1
#define INCLUDE_vTaskSuspend            1
#define INCLUDE_vTaskDelayUntil         1
#define INCLUDE_vTaskDelay              1
#define INCLUDE_eTaskGetState           1
#define INCLUDE_pcTaskGetTaskName       1
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define INCLUDE_xTaskGetIdleTaskHandle  1
#define INCLUDE_xEventGroupSetBitFromISR 1
#define INCLUDE_xTimerPendFunctionCall 1

/* Normal assert() semantics without relying on the provision of an assert.h
header file. */

#ifdef DEBUG
void ConfigAssertDA(char *msg, char *file, uint32_t line);
#define ASSERT(x) if (!(x)) ConfigAssertDA(NULL, __FILE__, __LINE__)
#else
#define ASSERT(x)
#endif

#define configASSERT( x ) ASSERT( (x) )

/* Use the system definition, if there is one */
#ifdef __NVIC_PRIO_BITS
#define configPRIO_BITS       __NVIC_PRIO_BITS
#else
#define configPRIO_BITS       5        /* 32 priority levels */
#endif

/* The lowest priority. */
#define configKERNEL_INTERRUPT_PRIORITY         ( 31 << (8 - configPRIO_BITS) )
/* Priority 5, or 160 as only the top three bits are implemented. */
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( 5 << (8 - configPRIO_BITS) )

/* Priorities passed to NVIC_SetPriority() do not require shifting as the
function does the shifting itself.  Note these priorities need to be equal to
or lower than configMAX_SYSCALL_INTERRUPT_PRIORITY - therefore the numeric
value needs to be equal to or greater than 5 (on the Cortex-M3 the lower the
numeric value the higher the interrupt priority). */

/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
standard names - or at least those used in the unmodified vector table. */

#define vPortSVCHandler SVC_Handler
#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

/* Definitions for task tracing support
 *
 * Note that _using_ these timers will incur delays of course.
 */

/* If you change this timer update in config.h too!! */
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() { Chip_TIMER_Init(LPC_TIMER3); Chip_TIMER_PrescaleSet(LPC_TIMER3, (SystemCoreClock/1000000)-1); Chip_TIMER_Enable(LPC_TIMER3); }
#define portGET_RUN_TIME_COUNTER_VALUE() (Chip_TIMER_ReadCount(LPC_TIMER3))

#define TIMER_TO_uS(x) (x)
#define TIMER_TO_MS(x) (x/1000)

/* Don't like this, but it avoids compiler warnings */
extern void taskOut();
extern void taskIn();
extern void taskNewTask();
extern void taskReady();

#define traceTASK_SWITCHED_OUT() taskOut()
#define traceTASK_SWITCHED_IN() taskIn()
#define traceTASK_CREATE(xTask) taskNewTask(xTask)
#define traceMOVED_TASK_TO_READY_STATE(xTask) taskReady(xTask); /* Note the ';' is needed */

/* End of definitions for task tracing support */

#endif /* FREERTOS_CONFIG_H */

