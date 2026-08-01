/* Copyright 2012-2014 Neko. */

/* UTILS implements code tracer and all utility functions. */

#include "core/product/debug/xasm32/aasm32.h"
#include "core/product/debug/xasm32/dasm32.h"
#include "vm/platform/platform.h"

#include "core/product/utils.h"

static void printTraceCall(t_utils_trace_call *rtracecall) {
    size_t i;
    PRINTF("%s", rtracecall->callName);
    for (i = 0; i < rtracecall->blockCount; ++i) {
        PRINTF("::%s", rtracecall->blockStack[i]);
    }
    PRINTF("\n");
}
void utilsTracePrint(t_utils_trace *rtrace) {
    size_t i;
    if (rtrace->callCount) {
        for (i = 0; i < rtrace->callCount; ++i) {
            printTraceCall(&(rtrace->callStack[rtrace->callCount - 1 - i]));
        }
    }
}
void utilsTraceInit(t_utils_trace *rtrace) {
    rtrace->callCount = 0;
    rtrace->flagError = 0;
}
void utilsTraceFinal(t_utils_trace *rtrace) {
    if (!rtrace->flagError && rtrace->callCount) {
        PRINTF("trace_final: call stack is not balanced. (call: %d, block: %d)\n",
               rtrace->callCount, rtrace->callStack[rtrace->callCount].blockCount);
        rtrace->flagError = 1;
    }
    if (rtrace->flagError) {
        utilsTracePrint(rtrace);
    }
    rtrace->callCount = 0;
    rtrace->flagError = 0;
}
void utilsTraceCallBegin(t_utils_trace *rtrace, char *callName) {
    if (rtrace->flagError) {
        return;
    }
    if (rtrace->callCount < UTILS_TRACE_MAX_STACK) {
#if UTILS_TRACE_DEBUG == 1
        PRINTF("enter call(%d): %s\n", rtrace->callCount, callName);
#endif
        rtrace->callStack[rtrace->callCount].callName = callName;
        rtrace->callStack[rtrace->callCount].blockCount = 0;
        rtrace->callCount++;
    } else {
        PRINTF("trace_call_begin: call stack is full.\n");
        rtrace->flagError = 1;
    }
}
void utilsTraceCallEnd(t_utils_trace *rtrace) {
    if (rtrace->flagError) {
        return;
    }
    if (rtrace->callCount) {
        rtrace->callCount--;
#if UTILS_TRACE_DEBUG == 1
        PRINTF("leave call(%d): %s\n", rtrace->callCount,
               rtrace->callStack[rtrace->callCount].callName);
#endif
        if (rtrace->callStack[rtrace->callCount].blockCount != 0) {
            PRINTF("trace_call_end: call stack is not balanced. (call: %d, block: %d)\n",
                   rtrace->callCount, rtrace->callStack[rtrace->callCount].blockCount);
            rtrace->callCount++;
            rtrace->flagError = 1;
        }
    } else {
        PRINTF("trace_call_end: call stack is empty.\n");
        rtrace->flagError = 1;
    }
}
void utilsTraceBlockBegin(t_utils_trace *rtrace, char *blockName) {
    if (rtrace->flagError) {
        return;
    }
    if (rtrace->callStack[rtrace->callCount - 1].blockCount < UTILS_TRACE_MAX_STACK) {
#if UTILS_TRACE_DEBUG == 1
        PRINTF("enter block(%d): %s\n", rtrace->callStack[rtrace->callCount - 1].blockCount, blockName);
#endif
        rtrace->callStack[rtrace->callCount - 1].
        blockStack[rtrace->callStack[rtrace->callCount - 1].blockCount++] = blockName;
    } else {
        PRINTF("trace_block_begin: block stack is full.\n");
        rtrace->flagError = 1;
    }
}
void utilsTraceBlockEnd(t_utils_trace *rtrace) {
    if (rtrace->flagError) {
        return;
    }
    if (rtrace->callStack[rtrace->callCount - 1].blockCount) {
        rtrace->callStack[rtrace->callCount - 1].blockCount--;
#if UTILS_TRACE_DEBUG == 1
        PRINTF("leave block(%d): %s\n",
               rtrace->callStack[rtrace->callCount - 1].blockCount,
               rtrace->callStack[rtrace->callCount - 1].blockStack[rtrace->callStack[rtrace->callCount - 1].blockCount]);
#endif
    } else {
        PRINTF("trace_block_end: block stack is empty.\n");
        rtrace->flagError = 1;
    }
}

/* General Functions */
void utilsSleep(uint32_t milisec) {
    platformSleep(milisec);
}

/* NXVM Assembler Library */
uint8_t utilsAasm32(const char *stmt, uint8_t *rcode, int flag32) {
    return aasm32(stmt, rcode, flag32);
}
uint32_t utilsAasm32x(const char *stmt, uint8_t *rcode, int flag32) {
    return aasm32x(stmt, rcode, flag32);
}
uint8_t utilsDasm32(char *stmt, uint8_t *rcode, int flag32) {
    return dasm32(stmt, rcode, flag32);
}
