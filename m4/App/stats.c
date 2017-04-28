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
 * Runtime Stats
 * =============
 *
 * This code was originally (c) Dave Marples. A copy passed to Technolution for independent development and
 * use without restriction on 13 Jun 2015.
 *
 */

#include <string.h>
#include <stdio.h>
#include "config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stats.h"
#include "serport.h"

/* --- Overall accounting state of all tasks */
typedef struct
{
    TaskHandle_t *thandle;
    eTaskState state;
    UBaseType_t shwm;
    UBaseType_t pri;
    uint64_t accTime;
    uint32_t readyAt;
    uint32_t worstRunDelay;
} TaskStatType;

/* --- Buffer for a snapshot */
typedef struct
{
    uint32_t nowTime;               /* Time at which the snapshot was taken (end of period) */
    uint32_t intervalTime;          /* Amount of time covered by this snapshot */
    TaskStatType ttable[MAX_TASKS]; /* Tasks and status */
} snapBufferType;


const char *taskStateName[]= {"Rdy","Run","Blk","Sus","Del"};

static uint32_t idleTaskTotal;              /* Accumulator for idle task */
static uint32_t intervalTotalTime;
static uint32_t totalTasks;                 /* Number of tasks active in the system */
static uint32_t inTime;                     /* Time at which this task was entered */
static TaskStatType ttable[MAX_TASKS];      /* Task table */
// ============================================================================================
void taskReady(TaskHandle_t *readyTask)

/* Routine called by OS when task enters the ready state */

{
    TaskStatType *t=ttable;
    uint32_t readyAt=portGET_RUN_TIME_COUNTER_VALUE();

    /* Search for this task in the table */
    for (; ((t->thandle) && (t->thandle!=readyTask)); t++);

    if (t->thandle)
        t->readyAt=readyAt;
}
// ============================================================================================
void taskIn(void)

/* Routine called when task starts to run */

{
    inTime=portGET_RUN_TIME_COUNTER_VALUE();
}
// ============================================================================================
void taskOut(void)

/* Routine called when task leaves running state - account for it's time */

{
    uint32_t outTime=portGET_RUN_TIME_COUNTER_VALUE();

    uint32_t delayTime;
    uint32_t elapsedTime;

    TaskHandle_t th=xTaskGetCurrentTaskHandle();
    TaskStatType *t=ttable;

    /* Deal with rollover */
    if (outTime>=inTime)
        elapsedTime=outTime-inTime;
    else
        elapsedTime=inTime-outTime;

    /* Find the task in the task list */
    while ((t->thandle) && (t->thandle!=th)) t++;

    if (!t->thandle)
        return;

    if (t->readyAt)
        {
            /* Get delay from being ready to being scheduled */
            if (inTime>=t->readyAt)
                delayTime=inTime-t->readyAt;
            else
                delayTime=t->readyAt-inTime;

            /* If we waited longer than any previous time for this task to start record this */
            if (delayTime>t->worstRunDelay)
                t->worstRunDelay=delayTime;
        }
    else
        {
            /* Only the idle task doesn't have a readyAt time */
            idleTaskTotal+=elapsedTime;
        }
    t->accTime+=((uint64_t)elapsedTime);
}
// ============================================================================================
void taskNewTask(TaskHandle_t *newTask)

/* New task creation - add it to the list so we can account with it later */

{
    if (totalTasks>(MAX_TASKS-2)) return;
    ttable[totalTasks++].thandle=newTask;
}
// ============================================================================================
void _snapShot(snapBufferType *b)

{
    taskDISABLE_INTERRUPTS();

    taskOut();  /* Grab remaining time for this task */

    if (!b)
        return;

    b->nowTime=portGET_RUN_TIME_COUNTER_VALUE();
    b->intervalTime=intervalTotalTime;
    memcpy(b->ttable, ttable, sizeof(ttable));

    intervalTotalTime=inTime=portGET_RUN_TIME_COUNTER_VALUE(); /* Start grabbing time again */

    /* Now capture the states and other info... */
    for (uint32_t i=0; i<totalTasks;)
        {
            b->ttable[i].state=eTaskGetState(b->ttable[i].thandle);
            b->ttable[i].shwm=uxTaskGetStackHighWaterMark(b->ttable[i].thandle);
            b->ttable[i].pri=uxTaskPriorityGet(b->ttable[i].thandle);

            ttable[i].worstRunDelay=0;
            ttable[i].accTime=0;
            ttable[i].readyAt=0;
            i++;
        }

    taskENABLE_INTERRUPTS();
}
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Publicly available routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
COMMAND(uiTaskList)

{
    snapBufferType b;   /* Buffer to keep the state snapshot in */
    TaskStatType *t;    /* Iterator for each task to be output */

    char op[60];            /* Output construction buffer */
    uint64_t totalTime=0;   /* Total time spent, according to accounting records */
    uint32_t errAmount;     /* Total time spent, according to clock */
    uint32_t l;

    bzero(&b,sizeof(b));

    /* Grab the stats as quickly as possible */
    _snapShot(&b);

    /* Calculate the total time spent */
    t=b.ttable;
    for (t=b.ttable; t->thandle; t++)
        totalTime+=t->accTime;

    errAmount=((b.nowTime-b.intervalTime-totalTime)*10000)/(b.nowTime-b.intervalTime);

    /* Now adjust total time to incorporate the OS overhead (Which is not accounted in the task totals) */
    totalTime=b.nowTime-b.intervalTime;

    serportPrintf(TERMINAL_PORT,"Interval Time = %4d.%03d Secs " EOL,
                  ((uint32_t)TIMER_TO_MS(totalTime)/1000),((uint32_t)TIMER_TO_MS(totalTime)%1000));
    serportPrintf(TERMINAL_PORT,"       Uptime = %4d.%03d Secs" EOL,
                  ((uint32_t)TIMER_TO_MS(b.nowTime)/1000),((uint32_t)TIMER_TO_MS(b.nowTime)%1000));
    serportPrintf(TERMINAL_PORT,"Name    Pri State  Rtc     Shwm     WRD" EOL);
    serportPrintf(TERMINAL_PORT,"------------------------------------------" EOL);
    serportPrintf(TERMINAL_PORT,"RTOS      -   %6d.%02d%%" EOL,errAmount/100,errAmount%100);

    /* Iterate over the table outputting data */
    t=b.ttable;

    for (t=b.ttable; t->thandle; t++)
        {
            l=serportPrintf(TERMINAL_PORT,"%s ",pcTaskGetTaskName( t->thandle ));
            serportMultiput(TERMINAL_PORT, MILLIS_TO_TICKS(100), ' ',2+configMAX_TASK_NAME_LEN-l);
            if (t->worstRunDelay)
                siprintf(op,"%6lu uS",TIMER_TO_uS(t->worstRunDelay));
            else
                *op=0;

            uint32_t ati=(t->accTime*100L)/totalTime;
            uint32_t atf=((t->accTime*10000L)/totalTime)%100;
            serportPrintf(TERMINAL_PORT,"%d   %s %2d.%02d%% %6d %s" EOL,t->pri, taskStateName[t->state],
                          ati,atf,t->shwm,op);
        }

    serportPrintf(TERMINAL_PORT,EOL "Free Heap = %d" EOL,xPortGetFreeHeapSize());
    return UI_OK;
}
// ============================================================================================
uint32_t statsGetIdlePercentage(void)

/* Get the idle percentage of time and reset the counters - used for display */

{
    static uint32_t lastSampleTime=0;
    uint32_t timeDiff;
    uint32_t retval;
    uint32_t nowTime=portGET_RUN_TIME_COUNTER_VALUE();

    timeDiff=(lastSampleTime<nowTime)?nowTime-lastSampleTime:lastSampleTime-nowTime;
    lastSampleTime=nowTime;

    retval=(idleTaskTotal*100)/timeDiff;
    taskDISABLE_INTERRUPTS();
    idleTaskTotal=0;
    taskENABLE_INTERRUPTS();
    return retval;
}
// ============================================================================================
void statsReset(void)

{
    _snapShot(NULL);
}
// ============================================================================================
void statsInit(void)

{
    portCONFIGURE_TIMER_FOR_RUN_TIME_STATS();
}
// ============================================================================================
