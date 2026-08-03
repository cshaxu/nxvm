/* Copyright 2012-2014 Neko. */

/* Shared project types and compile-time settings. */

#ifndef NTVDM64_TYPE_H
#define NTVDM64_TYPE_H

#ifdef __cplusplus
extern "C" {
#endif

#define GLOBAL_VAR_WIN32 0
#define GLOBAL_VAR_LINUX 1

/* COMPILING OPTIONS ******************************* */
#if 1
#define GLOBAL_SIZE_INTEGER 64
#else
#define GLOBAL_SIZE_INTEGER 32
#endif
#ifdef _WIN32
#define GLOBAL_PLATFORM GLOBAL_VAR_WIN32
#else
#define GLOBAL_PLATFORM GLOBAL_VAR_LINUX
#endif
/* ************************************************* */

/* DEBUGGING OPTIONS ******************************* */
#define UTILS_TRACE_ENABLED 0 /* enable trancer */
#define UTILS_TRACE_DEBUG   0 /* debug tracer itself */
/* ************************************************* */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <memory.h>
#include <time.h>

/* COMPATIBILITY DEFINITIONS *********************************************** */
#if GLOBAL_PLATFORM == GLOBAL_VAR_WIN32
typedef unsigned char    uint8_t;
typedef unsigned short   uint16_t;
typedef unsigned int     uint32_t;
typedef unsigned __int64 uint64_t;
typedef   signed char    int8_t;
typedef   signed short   int16_t;
typedef   signed int     int32_t;
typedef   signed __int64 int64_t;
#else
#include <stdint.h>
#endif
/* ************************************************************************* */

/* Retained NXVM numeric aliases. New code should prefer stdint names. */
#define MAXLINE   0x100

typedef char      t_string[MAXLINE];
typedef char     *t_strptr;
typedef uint8_t   t_nubit1;
typedef uint8_t   t_nubit4;
typedef uint8_t   t_nubit8;
typedef int8_t    t_nsbit8;
typedef uint16_t  t_nubit16;
typedef int16_t   t_nsbit16;
typedef uint32_t  t_nubit24;
typedef int32_t   t_nsbit24;
typedef uint32_t  t_nubit32;
typedef int32_t   t_nsbit32;
typedef uint64_t  t_nubit48;
typedef int64_t   t_nsbit48;
typedef uint64_t  t_nubit64;
typedef int64_t   t_nsbit64;
typedef float     t_float32;
typedef double    t_float64;
#if GLOBAL_SIZE_INTEGER == 64
typedef t_nubit64 t_nubitcc;
typedef t_nsbit64 t_nsbitcc;
#else
typedef t_nubit32 t_nubitcc;
typedef t_nsbit32 t_nsbitcc;
#endif
typedef t_nubit1  t_bool;
typedef t_nubitcc t_vaddrcc;
typedef t_nubitcc t_faddrcc;

#define GetRef(n) ((t_vaddrcc)(&(n)))

#define p_nubit1  (t_nubit1 *)
#define p_nubit4  (t_nubit4 *)
#define p_nubit8  (t_nubit8 *)
#define p_nsbit8  (t_nsbit8 *)
#define p_nubit16 (t_nubit16 *)
#define p_nsbit16 (t_nsbit16 *)
#define p_nubit24 (t_nubit24 *)
#define p_nsbit24 (t_nsbit24 *)
#define p_nubit32 (t_nubit32 *)
#define p_nsbit32 (t_nsbit32 *)
#define p_nubit48 (t_nubit48 *)
#define p_nsbit48 (t_nsbit48 *)
#define p_nubit64 (t_nubit64 *)
#define p_nsbit64 (t_nsbit64 *)
#define p_float32 (t_float32 *)
#define p_float64 (t_float64 *)
#define p_nubitcc (t_nubitcc *)
#define p_nsbitcc (t_nsbitcc *)
#define p_bool    (t_bool *)

#define d_nubit1(n)  (*(t_nubit1 *)(n))
#define d_nubit4(n)  (*(t_nubit4 *)(n))
#define d_nubit8(n)  (*(t_nubit8 *)(n))
#define d_nsbit8(n)  (*(t_nsbit8 *)(n))
#define d_nubit16(n) (*(t_nubit16 *)(n))
#define d_nsbit16(n) (*(t_nsbit16 *)(n))
#define d_nubit24(n) (*(t_nubit24 *)(n))
#define d_nsbit24(n) (*(t_nsbit24 *)(n))
#define d_nubit32(n) (*(t_nubit32 *)(n))
#define d_nsbit32(n) (*(t_nsbit32 *)(n))
#define d_nubit48(n) (*(t_nubit48 *)(n))
#define d_nsbit48(n) (*(t_nsbit48 *)(n))
#define d_nubit64(n) (*(t_nubit64 *)(n))
#define d_nsbit64(n) (*(t_nsbit64 *)(n))
#define d_float32(n) (*(t_float32 *)(n))
#define d_float64(n) (*(t_float64 *)(n))
#define d_nubitcc(n) (*(t_nubitcc *)(n))
#define d_nsbitcc(n) (*(t_nsbitcc *)(n))
#define d_bool(n)    (*(t_bool *)(n))

#define GetBit(s, f) (!!((s) & (f)))
#define SetBit(d, s) ((d) |= (s))
#define ClrBit(d, s) ((d) &= ~(s))
#define MakeBit(d, s, f) ((f) ? SetBit((d), (s)) : ClrBit((d), (s)))
#define GetMSB(n, b) (GetBit((n), (1 << ((b) - 1))))
#define GetLSB(n) (GetBit((n), 1))
#define False  0x00
#define True   0x01
#define Zero1  0x00
#define Zero4  0x00
#define Zero8  0x00
#define Zero16 0x0000
#define Zero32 0x00000000
#define Zero64 0x0000000000000000
#define Max1  0x01
#define Max4  0x0f
#define Max8  0xff
#define Max16 0xffff
#define Max24 0x00ffffff
#define Max32 0xffffffff
#define Max48 0x0000ffffffffffff
#define Max64 0xffffffffffffffff
#define MSB7  0x40
#define MSB8  0x80
#define MSB15 0x4000
#define MSB16 0x8000
#define MSB31 0x40000000
#define MSB32 0x80000000
#define MSB47 0x0000400000000000
#define MSB48 0x0000800000000000
#define MSB63 0x4000000000000000
#define MSB64 0x8000000000000000
#if GLOBAL_SIZE_INTEGER == 64
#define ZeroCC Zero64
#define MaxCC  Max64
#define MSBCC  MSB64
#else
#define ZeroCC Zero32
#define MaxCC  Max32
#define MSBCC  MSB32
#endif
#define GetMax1(n)  ((t_nubit1 )(n) & Max1 )
#define GetMax4(n)  ((t_nubit4 )(n) & Max4 )
#define GetMax8(n)  ((t_nubit8 )(n) & Max8 )
#define GetMax16(n) ((t_nubit16)(n) & Max16)
#define GetMax24(n) ((t_nubit24)(n) & Max24)
#define GetMax32(n) ((t_nubit32)(n) & Max32)
#define GetMax48(n) ((t_nubit48)(n) & Max48)
#define GetMax64(n) ((t_nubit64)(n) & Max64)
#define GetMaxCC(n) ((t_nubitcc)(n) & MaxCC)
#define GetMSB7(n)  ((t_nubit8 )(n) & MSB7 )
#define GetMSB8(n)  ((t_nubit8 )(n) & MSB8 )
#define GetMSB15(n) ((t_nubit16)(n) & MSB15)
#define GetMSB16(n) ((t_nubit16)(n) & MSB16)
#define GetMSB31(n) ((t_nubit32)(n) & MSB31)
#define GetMSB32(n) ((t_nubit32)(n) & MSB32)
#define GetMSB47(n) ((t_nubit48)(n) & MSB47)
#define GetMSB48(n) ((t_nubit48)(n) & MSB48)
#define GetMSB63(n) ((t_nubit64)(n) & MSB63)
#define GetMSB64(n) ((t_nubit64)(n) & MSB64)
#define GetMSBCC(n) ((t_nubitcc)(n) & MSBCC)
#define GetLSB8(n)  ((t_nubit8 )(n) & 1)
#define GetLSB16(n) ((t_nubit16)(n) & 1)
#define GetLSB32(n) ((t_nubit32)(n) & 1)
#define GetLSB48(n) ((t_nubit48)(n) & 1)
#define GetLSB64(n) ((t_nubit64)(n) & 1)
#define GetLSBCC(n) ((t_nubitcc)(n) & 1)

#define Hex2BCD(x)  ((((x) / 10) << 4) | ((x) % 10))
#define BCD2Hex(x)  (((x) & 0x0f) + ((((x) & 0xf0) >> 4) * 10))

#define ExecFun(faddr) ((faddr) ? ((*(void (*)(void))(faddr))()) : 0)

typedef enum nxvm_core_status {
    NXVM_CORE_STATUS_OK = 0,
    NXVM_CORE_STATUS_INVALID_ARGUMENT,
    NXVM_CORE_STATUS_INVALID_STATE,
    NXVM_CORE_STATUS_UNSUPPORTED,
    NXVM_CORE_STATUS_NO_MEMORY,
    NXVM_CORE_STATUS_FAULT
} nxvm_core_status;

/* Legacy C-runtime wrappers are a product-neutral root foundation. */
struct tm* LOCALTIME(const time_t *_Time);
char* STRCAT(char *_Dest, const char *_Source);
char* STRCPY(char *_Dest, const char *_Source);
char* STRTOK(char *_Str, const char *_Delim);
int STRCMP(const char *_Str1, const char *_Str2);
size_t STRLEN(const char *_Str);
int PRINTF(const char *_Format, ...);
int FPRINTF(FILE *_File, const char *_Format, ...);
int SPRINTF(char *_Dest, const char *_Format, ...);
FILE* FOPEN(const char *_Filename, const char *_Mode);
int FCLOSE(FILE *_File);
size_t FREAD(void *_DstBuf, size_t _ElementSize, size_t _Count, FILE *_File);
size_t FWRITE(void *_Str, size_t _Size, size_t _Count, FILE *_File);
char* FGETS(char *_Buf, int _MaxCount, FILE *_File);
void* MALLOC(size_t _Size);
void FREE(void *_Memory);
void* MEMSET(void *_Dst, int _Val, size_t _Size);
void* MEMCPY(void *_Dst, const void *_Src, size_t _Size);
int MEMCMP(const void *_Buf1, const void *_Buf2, size_t _Size);
void ntvdm64_type_string_lower(char *str);

/* Legacy trace support is shared root diagnostic infrastructure. */
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

void ntvdm64_type_trace_print(t_utils_trace *rtrace);
void ntvdm64_type_trace_initialize(t_utils_trace *rtrace);
void ntvdm64_type_trace_finalize(t_utils_trace *rtrace);
void ntvdm64_type_trace_call_begin(t_utils_trace *rtrace, char *callName);
void ntvdm64_type_trace_call_end(t_utils_trace *rtrace);
void ntvdm64_type_trace_block_begin(t_utils_trace *rtrace, char *blockName);
void ntvdm64_type_trace_block_end(t_utils_trace *rtrace);

/* #define UTILS_TRACE_VAR    tracer variable */
/* #define UTILS_TRACE_ERROR  error condition */
/* #define UTILS_TRACE_SETERR set error statement */

#if UTILS_TRACE_ENABLED == 1
#define _cb(callName)  ntvdm64_type_trace_call_begin(&(UTILS_TRACE_VAR), (callName))
#define _bb(blockName) ntvdm64_type_trace_block_begin(&(UTILS_TRACE_VAR), (blockName))
#define _ce ntvdm64_type_trace_call_end(&(UTILS_TRACE_VAR))
#define _be ntvdm64_type_trace_block_end(&(UTILS_TRACE_VAR))
#define _chb(n) \
    if (1) { \
        (n); \
        if (UTILS_TRACE_ERROR) { \
            (UTILS_TRACE_VAR).flagError = 1; \
            ntvdm64_type_trace_finalize(&(UTILS_TRACE_VAR)); \
            break; \
        } \
    } else
#define _chr(n) \
    do { \
        (n); \
        if (UTILS_TRACE_ERROR) { \
            (UTILS_TRACE_VAR).flagError = 1; \
            ntvdm64_type_trace_finalize(&(UTILS_TRACE_VAR)); \
            return; \
        } \
    } while (0)
#define _chrz(n) \
    do { \
        (n); \
        if (UTILS_TRACE_ERROR) { \
            (UTILS_TRACE_VAR).flagError = 1; \
            ntvdm64_type_trace_finalize(&(UTILS_TRACE_VAR)); \
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

/* Legacy C-runtime wrappers are a product-neutral root foundation. */
struct tm* LOCALTIME(const time_t *_Time);
char* STRCAT(char *_Dest, const char *_Source);
char* STRCPY(char *_Dest, const char *_Source);
char* STRTOK(char *_Str, const char *_Delim);
int STRCMP(const char *_Str1, const char *_Str2);
size_t STRLEN(const char *_Str);
int PRINTF(const char *_Format, ...);
int FPRINTF(FILE *_File, const char *_Format, ...);
int SPRINTF(char *_Dest, const char *_Format, ...);
FILE* FOPEN(const char *_Filename, const char *_Mode);
int FCLOSE(FILE *_File);
size_t FREAD(void *_DstBuf, size_t _ElementSize, size_t _Count, FILE *_File);
size_t FWRITE(void *_Str, size_t _Size, size_t _Count, FILE *_File);
char* FGETS(char *_Buf, int _MaxCount, FILE *_File);
void* MALLOC(size_t _Size);
void FREE(void *_Memory);
void* MEMSET(void *_Dst, int _Val, size_t _Size);
void* MEMCPY(void *_Dst, const void *_Src, size_t _Size);
int MEMCMP(const void *_Buf1, const void *_Buf2, size_t _Size);
void ntvdm64_type_string_lower(char *str);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
