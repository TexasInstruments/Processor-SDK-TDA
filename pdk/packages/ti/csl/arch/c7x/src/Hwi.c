/*
 * Copyright (c) 2013-2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 *  ======== Hwi.c ========
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>

#include <c6x_migration.h>
#include <c7x.h>

#include <ti/csl/csl_error.h>
#include <ti/osal/DebugP.h>
#include <Hwi.h>
#include <csl_types.h>

/* Object__sizingError */
typedef char Hwi_Object__sizingError[(sizeof(Hwi_Object) > sizeof(HwiC7x_Struct)) ? -1 : 1];

extern uint32_t Hwi_disable(void);
extern uint32_t Hwi_enable(void);
extern void Hwi_restore(uint32_t key);

extern void Hwi_dispatchAlways(void);
extern void Hwi_switchAndRunFunc(void (*func)(void));
extern void Hwi_refreshNLC(void);


extern  char _stack[0x10001];

/*
 * ECSP stores an 8 KB context for each interrupt, up to a maximum of 8
 * nesting levels (1 per priority level).
 */
#define HWI_ECSP_SIZE (0x10000U)

extern const Hwi_Params Hwi_Object__PARAMS__C;

#if Hwi_bootToNonSecure__D
extern void Hwi_switchToNS(void);
#endif

void Hwi_secureStart(void)
{
#if Hwi_bootToNonSecure__D
    /* Initialize the vector table pointer, ESTP */
    __ESTP_S = (uintptr_t)Hwi_vectorTableBase;
    __ESTP_SS = (uintptr_t)Hwi_vectorTableBase_SS;

    __ECSP_S = (uintptr_t)_stack;
    __ECSP_SS = (uintptr_t)_stack;
    __TCSP = (uintptr_t)(_stack + HWI_ECSP_SIZE);

    /* start with a clean slate */
    __set_indexed(__EFCLR, 0, 0xFFFFFFFFFFFFFFFFUL);

    Hwi_switchToNS();
#endif
}

/*
 *  ======== Hwi_Module_startup ========
 */
void Hwi_Module_startup(void)
{
    uint32_t i;

    /* Task and Swi APIs used not called until BIOS_start() */
    /* okay to proceed with initialization */

    /* Initialize the vector table pointer, ESTP */
    __ESTP_S = (uintptr_t)Hwi_vectorTableBase;

    /*
     * Initialize the pointer to the isrStack. These symbols are part of the
     * Hwi_module (instead of floating) in order to support ROM.
     * Leave room for two 32-bit values pushed by xdc_runtime_Startup_reset()
     * (for cases where intentionally reset as resume from power down),
     * and maintain double word alignment.
     */
    Hwi_Module_state.isrStack = Hwi_getIsrStackAddress() - 16;
    __ECSP_S = (uintptr_t)_stack;
    __TCSP = (uintptr_t)(_stack + HWI_ECSP_SIZE);

    /* signal that we're executing on the ISR stack */
    Hwi_Module_state.taskSP = (char *)-1;

    /* initialize event mapping */
    for (i = 0U; i < Hwi_NUM_INTERRUPTS; i++) {
        if (Hwi_Module_state.intEvents[i] != (-1)) {
            Hwi_eventMap(i, (uint32_t)Hwi_Module_state.intEvents[i]);
        }
        else {
            /* keep intEvents[] current for ROV */
            Hwi_Module_state.intEvents[i] = Hwi_getEventId(i);
        }
    }

    /* start with a clean slate */
    __set_indexed(__EFCLR, 0, 0xFFFFFFFFFFFFFFFFUL);

    Hwi_enableIER((uintptr_t)Hwi_Module_state.ierMask); /* IER per static Hwi settings */

    Hwi_setCOP(0xFF);

    return;
}

/*
 *  ======== Hwi_Instance_init ========
 */
int32_t Hwi_Instance_init(Hwi_Object *hwi, uint32_t intNum,
                      Hwi_FuncPtr fxn, const Hwi_Params *params)
{
    int32_t retval = CSL_PASS;
    /* there are 64 "interrupt" events in C7x */
    if (intNum > 63U) {
        retval = CSL_EFAIL;
    }

    if (retval != CSL_EFAIL) {
        if (NULL != Hwi_Module_state.dispatchTable[intNum]) {
            retval = CSL_EFAIL;
        }

        if (retval != CSL_EFAIL) {
            Hwi_Module_state.dispatchTable[intNum] = hwi;

            Hwi_reconfig(hwi, fxn, params);
            hwi->intNum = intNum;

            hwi->irp = 0;

            retval = CSL_PASS;
        }
    }
    return (retval);
}

/*
 *  ======== Hwi_Instance_finalize ========
 *  Here on Hwi_delete() or failed Hwi_create().
 */
void Hwi_Instance_finalize(Hwi_Object *hwi, int32_t status)
{
    uint32_t intNum;
    bool retval = BFALSE;

    if (CSL_EFAIL == status) {  /* failed early in Hwi_Instance_init() */
        retval = BTRUE;
    }

    if (retval != BTRUE) {
        for (intNum = 0; intNum < Hwi_NUM_INTERRUPTS; intNum++) {
            if (Hwi_Module_state.dispatchTable[intNum] == hwi) {
                break;
            }
        }

        if (intNum == Hwi_NUM_INTERRUPTS) {
            retval = BTRUE;   /* never reached, but keep Coverity happy */
        }

        if (retval != BTRUE) {
            Hwi_disableInterrupt(intNum);
            Hwi_Module_state.dispatchTable[intNum] = NULL;
        }    
    }
}

/*
 *  ======== Hwi_eventMap ========
 *  Function maps an event to a Hwi vector so that the event will be the
 *  source of the interrupt of the vector.
 */
void Hwi_eventMap(uint32_t vectId, uint32_t eventId)
{
    uint32_t mask;          /* Interrupt mask value */

    if (vectId < 64U) {
        mask = Hwi_disable();

        /* Program CLEC to map external eventId to internal interrupt (event) */

        /* clear any residual interrupt */
        __set_indexed(__EFCLR, 0, 1UL << vectId);

        Hwi_restore(mask);
    }
}

/*
 *  ======== Hwi_getEventId ========
 */
int32_t Hwi_getEventId(uint32_t intNum)
{
    int32_t retval = CSL_PASS;
    
    if (intNum > 63U) {
        retval = CSL_EFAIL;
    }

    /* TODO */

    return retval;
}

/*
 * Below APIs provided for backward compatability.
 */

/*
 *  ======== Hwi_disableIER ========
 */
uintptr_t Hwi_disableIER(uint64_t mask)
{
    uint32_t  key;
    uintptr_t oldEER;

    key = Hwi_disable();
    oldEER = __get_indexed(__EER, 0);
    __set_indexed(__EECLR, 0, mask);
    Hwi_restore(key);

    return oldEER;
}

/*
 *  ======== Hwi_enableIER ========
 */
uintptr_t Hwi_enableIER(uintptr_t mask)
{
    uint32_t  key;
    uintptr_t oldEER;

    key = Hwi_disable();
    oldEER = __get_indexed(__EER, 0);
    __set_indexed(__EESET, 0, mask);
    Hwi_restore(key);

    return oldEER;
}

/*
 *  ======== Hwi_disableInterrupt ========
 */
uint32_t Hwi_disableInterrupt(uint32_t intNum)
{
    uintptr_t mask = 1UL << intNum;
    uint32_t retval;
    
    /* Hwi_disableIER() returns old EER */
    if ((Hwi_disableIER(mask) & mask) != 0UL)
    {
        retval = 0U;
    }
    else
    {
        retval = 1U;
    }
    
    return retval;
}

/*
 *  ======== Hwi_enableInterrupt ========
 */
uint32_t Hwi_enableInterrupt(uint32_t intNum)
{
    uintptr_t mask = 1UL << intNum;
    uint32_t  retval;
  
    if ((Hwi_enableIER(mask) & mask) != 0UL) 
    {
        retval = 0U;
    }
    else
    {
        retval = 1U;
    }
    return retval ;
}

/*
 *  ======== Hwi_clearInterrupt ========
 */
void Hwi_clearInterrupt(uint32_t intNum)
{
    __set_indexed(__EFCLR, 0, 1UL << intNum);
}

/*
 *  ======== Hwi_getHandle ========
 */
Hwi_Handle Hwi_getHandle(uint32_t intNum)
{
    return (Hwi_Module_state.dispatchTable[intNum]);
}

/*
 *  ======== Hwi_post ========
 */
void Hwi_post(uint32_t intNum)
{
    __set_indexed(__EFSET, 0, 1UL << intNum);
}

/*
 *  ======== Hwi_reconfig ========
 *  Reconfigure a dispatched interrupt.
 */
void Hwi_reconfig(Hwi_Object *hwi, Hwi_FuncPtr fxn, const Hwi_Params *params)
{
    uint32_t intNum;
    bool     retval = BTRUE;

    for (intNum = 0U; intNum < Hwi_NUM_INTERRUPTS; intNum++) {
        if (Hwi_Module_state.dispatchTable[intNum] == hwi) {
            break;
        }
    }

    if (intNum == Hwi_NUM_INTERRUPTS) {
        retval = BFALSE;
    }

    if (retval != BFALSE) {
        Hwi_disableInterrupt(intNum);

        hwi->fxn = fxn;
        hwi->arg = params->arg;

        if (-1 == params->priority) {
            hwi->priority = (int32_t)Hwi_DEFAULT_INT_PRIORITY;
        }
        else {
            hwi->priority = params->priority;
        }

        if (Hwi_MaskingOption_NONE == params->maskSetting) {
            hwi->disableMask = 0ULL;
            hwi->restoreMask = 0ULL;
        }
        else if (Hwi_MaskingOption_ALL == params->maskSetting) {
            hwi->disableMask = 0xFFFFFFFFFFFFFFFFUL;
            hwi->restoreMask = 0xFFFFFFFFFFFFFFFFUL;
        }
        else if (Hwi_MaskingOption_SELF == params->maskSetting) {
            hwi->disableMask = 1ULL << intNum;
            hwi->restoreMask = 1ULL << intNum;
        }
        else if (Hwi_MaskingOption_BITMASK == params->maskSetting) {
            hwi->disableMask = params->disableMask;
            hwi->restoreMask = params->restoreMask;
        }
        else
        {
            /* Do nothing. */
        }

        if (params->eventId != CSL_INVALID_EVENT_ID) {
            Hwi_eventMap(intNum, params->eventId);
        }

    /* keep intEvents[] current for ROV */
    Hwi_Module_state.intEvents[intNum] = Hwi_getEventId(intNum);

        if (params->enableInt) {
            Hwi_enableInterrupt(intNum);
        }
  }
  return;
}

/*
 *  ======== switchFromBootStack ========
 *  Indicate that we are leaving the boot stack and
 *  are about to switch to a task stack.
 */
void Hwi_switchFromBootStack(void)
{
    Hwi_Module_state.taskSP = 0;
}

/*
 *  ======== Hwi_getIsrStackAddress ========
 */
char *Hwi_getIsrStackAddress(void)
{
    extern uint8_t __TI_STACK_SIZE;
    uint64_t isrStack;

    isrStack = (uint64_t)_stack;
    isrStack += (uint64_t)_symval(&__TI_STACK_SIZE);
    isrStack -= 0x1U;

    isrStack &= ~0x7U;   /* align to long word */

    return ((char *)isrStack);
}

/*
 *  ======== Hwi_getStackInfo ========
 *  Used to get Hwi stack usage info.
 */
bool Hwi_getStackInfo(Hwi_StackInfo *stkInfo, bool computeStackDepth)
{
    extern  uint8_t __TI_STACK_SIZE;

    uintptr_t isrSP;
    bool      stackOverflow;

    /*
     * Copy the stack base address and size.
     *
     * Adjust for HW ECSP/TCSP portions.  TCSP only temporarily occupies
     * 0x2000 bytes of stack starting at _stack + HWI_ECSP_SIZE, until
     * the first Task switch.
     */
    stkInfo->hwiStackSize = (size_t)_symval(&__TI_STACK_SIZE) - 
                            (HWI_ECSP_SIZE + 0x2000U);
    stkInfo->hwiStackBase = (uintptr_t)_stack + HWI_ECSP_SIZE + 0x2000U;

    isrSP = stkInfo->hwiStackBase;

    /* Check for stack overflow */
    if(*((uint8_t*)isrSP) != 0xBEU)
    {
        stackOverflow = BTRUE;
    }
    else
    {
        stackOverflow = BFALSE;
    }

    if (computeStackDepth && !stackOverflow) {
        /* Compute stack depth */
        do {
        } while((*((uint8_t*)isrSP))++ == 0xBEU);
        --isrSP;
        stkInfo->hwiStackPeak = stkInfo->hwiStackSize - (size_t)(isrSP - stkInfo->hwiStackBase);
    }
    else {
        stkInfo->hwiStackPeak = 0U;
    }

    return stackOverflow;
}

/* construct */
int32_t Hwi_construct(HwiC7x_Struct *objp, uint32_t intNum, Hwi_FuncPtr hwiFxn, const Hwi_Params *paramsPtr)
{
    int32_t iStat;

    /* common instance initialization */
    memset(objp, 0, sizeof(*objp));

    /* module-specific initialization */
    iStat = Hwi_Instance_init((void *)objp, intNum, hwiFxn, paramsPtr);
    if (CSL_PASS != iStat) {
        Hwi_Instance_finalize(objp, iStat);
    }
    return iStat;
}

/* destruct */
void Hwi_destruct(HwiC7x_Struct *obj)
{
    Hwi_Instance_finalize(obj, CSL_PASS);
}

/* Params__init__S */
void Hwi_Params__init__S( Hwi_Params *prms)
{
    memcpy(prms, &Hwi_Object__PARAMS__C, sizeof(*prms));
}


void Hwi_Params_init( Hwi_Params *prms ) 
{
    if (NULL != prms) {
        Hwi_Params__init__S(prms);
    }
}

#pragma FUNC_EXT_CALLED(Hwi_dispatchCore);

/*
 *  ======== Hwi_dispatchCore ========
 *  Configurable dispatcher.
 * This API should not be called from Secure Supervisor mode.
 */
void Hwi_dispatchCore(uint32_t intNum)
{
    /* Errata i2399: C7x NLC module does not clear its internal "reload pending"
     * state when an interrupt is taken.  Issue NLCINIT here (as part of context
     * saving) to clear that state before any task-context work is done, so that
     * a subsequent task switch cannot corrupt ILCNT in the interrupted task. */
    Hwi_refreshNLC();

     /* Enable Branch Prediction if not already enabled
      * BP is disabled in RTOS Idle Hook as a pre-requisite to enter IDLE mode
      * and is enabled after exiting it.
      * If there is a async interrupt while in IDLE mode, BP will remain disabled */
    if (0x0U != __BPCR) {

    __BPCR = 0x0U;

    }
    /*
     * Enough room is reserved above the isr stack to handle
     * as many as 16 32-bit stack resident local variables.
     * If the dispatcher requires more than this, you must
     * handle this in Hwi_Module_startup().
     */

    Hwi_Object *hwi;
    Hwi_FuncPtr fxn;
    uintptr_t arg;

    /* save away intNum in module state because it might be saved on stack */
    Hwi_Module_state.intNum = intNum;

    /*
     * pre-read local copies of the variables used
     * within to eliminate memory fetch nops
     */
    hwi = Hwi_Module_state.dispatchTable[intNum];
    fxn = hwi->fxn;
    arg = hwi->arg;

    uint64_t *contextStack;
    uint64_t ncnt;

    /*
        * There is no IRP register on C7x.  The interrupt return pointer
        * is stored on the "Context Stack" at offset 0.  The context stack
        * is pointed to by:
        *     - TCSP if interrupt occurred while CPU in "Thread Mode"
        *     - ECSP[n] if interrupt occurred while in "Event Handler Mode"
        *       where n is the interrupt nesting count minus one (i.e., first
        *       nested interrupt uses ECSP[0] since the original non-nested
        *       interrupt was saved on TCSP)
        * If ncnt (ECSP nesting count, bits 15-13) is 0 then TCSP must be the
        * one because ncnt would be > 0 if an interrupt's context had already
        * been stored on it.
        */
    ncnt = (((uint64_t)__ECSP_S & 0xE000ULL) >> 13ULL);
    if (ncnt > 0ULL) {
        contextStack = (uint64_t *)(((uint64_t)__ECSP_S) +
                                    ((ncnt - 1U) * 0x2000U));
    }
    else  {
        contextStack = (uint64_t *)__TCSP;
    }

    hwi->irp = (uint32_t)contextStack[0];

    DebugP_log3("Hwi_begin fxn:%p, intNum:%p, IRP:%p", (uintptr_t)hwi->fxn,
                (uintptr_t)intNum, (uintptr_t)hwi->irp);

    /* call the user's isr */
    (fxn)(arg);

    DebugP_log1("Hwi_end intNum:%p", (uintptr_t)intNum);

    /* open up interrupt priorities */
    Hwi_setCOP(0xFF);

}

/* enableException */
#pragma DATA_SECTION(Hwi_enableException, ".const:Hwi_enableException");
const bool Hwi_enableException = BTRUE;

/* dispatcherIrpTrackingSupport */
#pragma DATA_SECTION(Hwi_dispatcherIrpTrackingSupport, ".const:Hwi_dispatcherIrpTrackingSupport");
const bool Hwi_dispatcherIrpTrackingSupport = BTRUE;

/* Object__PARAMS__C */
#pragma DATA_SECTION(Hwi_Object__PARAMS__C, ".const:Hwi_Object__PARAMS__C");
const Hwi_Params Hwi_Object__PARAMS__C = {
    Hwi_MaskingOption_SELF,  /* maskSetting */
    ((uint32_t)(0x0)),       /* arg */
    BTRUE,                   /* enableInt */
    CSL_INVALID_EVENT_ID,    /* eventId */
    (int32_t)(-0x0 - 1),     /* priority */
    (uint64_t)0x0UL,    /* disableMask */
    (uint64_t)0x0UL,    /* restoreMask */
};


/* Module__state__V */
#ifdef __ti__
struct Hwi_Module_State Hwi_Module_state __attribute__ ((section(".data:Hwi_Module_state")));
#elif !(defined(__MACH__) && defined(__APPLE__))
struct Hwi_Module_State Hwi_Module_state __attribute__ ((section(".data:Hwi_Module_state")));
#endif
struct Hwi_Module_State Hwi_Module_state = {
    {
        (int32_t)(-0x0 - 1),  /* [0] */
        (int32_t)(-0x0 - 1),  /* [1] */
        (int32_t)(-0x0 - 1),  /* [2] */
        (int32_t)(-0x0 - 1),  /* [3] */
        (int32_t)(-0x0 - 1),  /* [4] */
        (int32_t)(-0x0 - 1),  /* [5] */
        (int32_t)(-0x0 - 1),  /* [6] */
        (int32_t)(-0x0 - 1),  /* [7] */
        (int32_t)(-0x0 - 1),  /* [8] */
        (int32_t)(-0x0 - 1),  /* [9] */
        (int32_t)(-0x0 - 1),  /* [10] */
        (int32_t)(-0x0 - 1),  /* [11] */
        (int32_t)(-0x0 - 1),  /* [12] */
        (int32_t)(-0x0 - 1),  /* [13] */
        (int32_t)(0x4E0),     /* [14] */
        (int32_t)(-0x0 - 1),  /* [15] */
        (int32_t)(-0x0 - 1),  /* [16] */
        (int32_t)(-0x0 - 1),  /* [17] */
        (int32_t)(-0x0 - 1),  /* [18] */
        (int32_t)(-0x0 - 1),  /* [19] */
        (int32_t)(-0x0 - 1),  /* [20] */
        (int32_t)(-0x0 - 1),  /* [21] */
        (int32_t)(-0x0 - 1),  /* [22] */
        (int32_t)(-0x0 - 1),  /* [23] */
        (int32_t)(-0x0 - 1),  /* [24] */
        (int32_t)(-0x0 - 1),  /* [25] */
        (int32_t)(-0x0 - 1),  /* [26] */
        (int32_t)(-0x0 - 1),  /* [27] */
        (int32_t)(-0x0 - 1),  /* [28] */
        (int32_t)(-0x0 - 1),  /* [29] */
        (int32_t)(-0x0 - 1),  /* [30] */
        (int32_t)(-0x0 - 1),  /* [31] */
        (int32_t)(-0x0 - 1),  /* [32] */
        (int32_t)(-0x0 - 1),  /* [33] */
        (int32_t)(-0x0 - 1),  /* [34] */
        (int32_t)(-0x0 - 1),  /* [35] */
        (int32_t)(-0x0 - 1),  /* [36] */
        (int32_t)(-0x0 - 1),  /* [37] */
        (int32_t)(-0x0 - 1),  /* [38] */
        (int32_t)(-0x0 - 1),  /* [39] */
        (int32_t)(-0x0 - 1),  /* [40] */
        (int32_t)(-0x0 - 1),  /* [41] */
        (int32_t)(-0x0 - 1),  /* [42] */
        (int32_t)(-0x0 - 1),  /* [43] */
        (int32_t)(-0x0 - 1),  /* [44] */
        (int32_t)(-0x0 - 1),  /* [45] */
        (int32_t)(-0x0 - 1),  /* [46] */
        (int32_t)(-0x0 - 1),  /* [47] */
        (int32_t)(-0x0 - 1),  /* [48] */
        (int32_t)(-0x0 - 1),  /* [49] */
        (int32_t)(-0x0 - 1),  /* [50] */
        (int32_t)(-0x0 - 1),  /* [51] */
        (int32_t)(-0x0 - 1),  /* [52] */
        (int32_t)(-0x0 - 1),  /* [53] */
        (int32_t)(-0x0 - 1),  /* [54] */
        (int32_t)(-0x0 - 1),  /* [55] */
        (int32_t)(-0x0 - 1),  /* [56] */
        (int32_t)(-0x0 - 1),  /* [57] */
        (int32_t)(-0x0 - 1),  /* [58] */
        (int32_t)(-0x0 - 1),  /* [59] */
        (int32_t)(-0x0 - 1),  /* [60] */
        (int32_t)(-0x0 - 1),  /* [61] */
        (int32_t)(-0x0 - 1),  /* [62] */
        (int32_t)(-0x0 - 1),  /* [63] */
    },  /* intEvents */
    (uint64_t)0x4003UL,  /* ierMask */
    (int32_t)0x0,  /* intNum */
    ((char*)NULL),  /* taskSP */
    ((char*)NULL),  /* isrStack */
    (int32_t)0x0,  /* scw */
    {
        0,  /* [0] */
        0,  /* [1] */
        0,  /* [2] */
        0,  /* [3] */
        0,  /* [4] */
        0,  /* [5] */
        0,  /* [6] */
        0,  /* [7] */
        0,  /* [8] */
        0,  /* [9] */
        0,  /* [10] */
        0,  /* [11] */
        0,  /* [12] */
        0,  /* [13] */
        0,  /* [14] */
        0,  /* [15] */
        0,  /* [16] */
        0,  /* [17] */
        0,  /* [18] */
        0,  /* [19] */
        0,  /* [20] */
        0,  /* [21] */
        0,  /* [22] */
        0,  /* [23] */
        0,  /* [24] */
        0,  /* [25] */
        0,  /* [26] */
        0,  /* [27] */
        0,  /* [28] */
        0,  /* [29] */
        0,  /* [30] */
        0,  /* [31] */
        0,  /* [32] */
        0,  /* [33] */
        0,  /* [34] */
        0,  /* [35] */
        0,  /* [36] */
        0,  /* [37] */
        0,  /* [38] */
        0,  /* [39] */
        0,  /* [40] */
        0,  /* [41] */
        0,  /* [42] */
        0,  /* [43] */
        0,  /* [44] */
        0,  /* [45] */
        0,  /* [46] */
        0,  /* [47] */
        0,  /* [48] */
        0,  /* [49] */
        0,  /* [50] */
        0,  /* [51] */
        0,  /* [52] */
        0,  /* [53] */
        0,  /* [54] */
        0,  /* [55] */
        0,  /* [56] */
        0,  /* [57] */
        0,  /* [58] */
        0,  /* [59] */
        0,  /* [60] */
        0,  /* [61] */
        0,  /* [62] */
        0,  /* [63] */
    },  /* dispatchTable */
};
