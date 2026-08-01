/* Copyright 2012-2014 Neko. */

#ifndef NXVM_UTILS_H
#define NXVM_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

/* Code Tracer */
#define UTILS_TRACE_MAX_STACK 0x100
typedef struct {
    char* blockStack[UTILS_TRACE_MAX_STACK];
    size_t blockCount;
    char *callName;
} t_utils_trace_call;
typedef struct {
    t_utils_trace_call callStack[UTILS_TRACE_MAX_STACK];
    size_t callCount;
    int flagError;
} t_utils_trace;

void utilsTracePrint(t_utils_trace *rtrace);
void utilsTraceInit(t_utils_trace *rtrace);
void utilsTraceFinal(t_utils_trace *rtrace);
void utilsTraceCallBegin(t_utils_trace *rtrace, char *callName);
void utilsTraceCallEnd(t_utils_trace *rtrace);
void utilsTraceBlockBegin(t_utils_trace *rtrace, char *blockName);
void utilsTraceBlockEnd(t_utils_trace *rtrace);

/* #define UTILS_TRACE_VAR    tracer variable */
/* #define UTILS_TRACE_ERROR  error condition */
/* #define UTILS_TRACE_SETERR set error statement */

#if UTILS_TRACE_ENABLED == 1
#define _cb(callName)  utilsTraceCallBegin(&(UTILS_TRACE_VAR), (callName))
#define _bb(blockName) utilsTraceBlockBegin(&(UTILS_TRACE_VAR), (blockName))
#define _ce utilsTraceCallEnd(&(UTILS_TRACE_VAR))
#define _be utilsTraceBlockEnd(&(UTILS_TRACE_VAR))
#define _chb(n) \
    if (1) { \
        (n); \
        if (UTILS_TRACE_ERROR) { \
            (UTILS_TRACE_VAR).flagError = 1; \
            utilsTraceFinal(&(UTILS_TRACE_VAR)); \
            break; \
        } \
    } else
#define _chr(n) \
    do { \
        (n); \
        if (UTILS_TRACE_ERROR) { \
            (UTILS_TRACE_VAR).flagError = 1; \
            utilsTraceFinal(&(UTILS_TRACE_VAR)); \
            return; \
        } \
    } while (0)
#define _chrz(n) \
    do { \
        (n); \
        if (UTILS_TRACE_ERROR) { \
            (UTILS_TRACE_VAR).flagError = 1; \
            utilsTraceFinal(&(UTILS_TRACE_VAR)); \
            return 0; \
        } \
    } while (0)
#else
#define _cb(callName)
#define _bb(blockName)
#define _ce
#define _be
#define _chb(n) \
    if (1) { \
        (n); \
        if (UTILS_TRACE_ERROR) { \
            break; \
        } \
    } else
#define _chr(n) \
    do { \
        (n); \
        if (UTILS_TRACE_ERROR) { \
            return; \
        } \
    } while (0)
#define _chrz(n) \
    do { \
        (n); \
        if (UTILS_TRACE_ERROR) { \
            return 0; \
        } \
    } while (0)
#endif

#define _impossible_b_  _chb(UTILS_TRACE_SETERR);
#define _impossible_r_  _chr(UTILS_TRACE_SETERR);
#define _impossible_rz_ _chrz(UTILS_TRACE_SETERR);

/* NXVM Library */
void utilsSleep(uint32_t milisec);

/* NXVM Assembler Library */
uint8_t utilsAasm32(const char *stmt, uint8_t *rcode, int flag32);
uint32_t utilsAasm32x(const char *stmt, uint8_t *rcode, int flag32);
uint8_t utilsDasm32(char *stmt, uint8_t *rcode, int flag32);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
