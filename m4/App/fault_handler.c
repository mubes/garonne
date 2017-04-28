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
#include "config.h"
#include <stdio.h>

void printErrorMsg(const char * errMsg);
void printUsageErrorMsg(uint32_t CFSRValue);
void printBusFaultErrorMsg(uint32_t CFSRValue);
void printMemoryManagementErrorMsg(uint32_t CFSRValue);
void stackDump(uint32_t stack[]);

void Hard_Fault_Handler(uint32_t stack[])
{
    uint32_t spin=1;
    //if((CoreDebug->DHCSR & 0x01) != 0) {
    dbgprint("In Hard Fault Handler" EOL);
    dbgprint("SCB->HFSR = 0x%08lx" EOL, SCB->HFSR);
    if ((SCB->HFSR & (1 << 30)) != 0)
        {
            dbgprint("Forced Hard Fault" EOL);
            dbgprint("SCB->CFSR = 0x%08lx" EOL, SCB->CFSR );
            if((SCB->CFSR & 0xFFFF0000) != 0)
                {
                    printUsageErrorMsg(SCB->CFSR);
                }
            if((SCB->CFSR & 0xFF00) != 0)
                {
                    printBusFaultErrorMsg(SCB->CFSR);
                }
            if((SCB->CFSR & 0xFF) != 0)
                {
                    printMemoryManagementErrorMsg(SCB->CFSR);
                }
        }
    stackDump(stack);
    // __ASM volatile("BKPT #01");
    //}
    while(spin);
}

void printUsageErrorMsg(uint32_t CFSRValue)
{
    dbgprint("Usage fault: EOL");
    CFSRValue >>= 16; // right shift to lsb

    if((CFSRValue & (1<<9)) != 0)
        {
            dbgprint("Divide by zero" EOL);
        }
}

void printBusFaultErrorMsg(uint32_t CFSRValue)
{
    dbgprint("Bus fault: " EOL);
    CFSRValue = ((CFSRValue & 0x0000FF00) >> 8); // mask and right shift to lsb
}

void printMemoryManagementErrorMsg(uint32_t CFSRValue)
{
    dbgprint("Memory Management fault: " EOL);
    CFSRValue &= 0x000000FF; // mask just mem faults
}


void HardFault_Handler(void)
{
    __ASM("TST lr, #4");
    __ASM("ITE EQ");
    __ASM("MRSEQ r0, MSP");
    __ASM("MRSNE r0, PSP");
    __ASM("B Hard_Fault_Handler");
}

enum { r0, r1, r2, r3, r12, lr, pc, psr};

void stackDump(uint32_t stack[])
{
    dbgprint("r0 = 0x%08lx" EOL, stack[r0]);
    dbgprint("r1 = 0x%08lx" EOL, stack[r1]);
    dbgprint("r2 = 0x%08lx" EOL, stack[r2]);
    dbgprint("r3 = 0x%08lx" EOL, stack[r3]);
    dbgprint("r12 = 0x%08lx" EOL, stack[r12]);
    dbgprint("lr = 0x%08lx" EOL, stack[lr]);
    dbgprint("pc = 0x%08lx" EOL, stack[pc]);
    dbgprint("psr = 0x%08lx" EOL, stack[psr]);
}
