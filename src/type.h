/* Copyright 2012-2014 Neko. */

/* Shared project types and compile-time settings. */

#ifndef NTVDM64_TYPE_H
#define NTVDM64_TYPE_H

#ifdef __cplusplus
extern "C" {
#endif

#define GLOBAL_PLATFORM_WIN32 0
#define GLOBAL_PLATFORM_LINUX 1

/* COMPILING OPTIONS ******************************* */
#if 1
#define GLOBAL_SIZE_INTEGER 64
#else
#define GLOBAL_SIZE_INTEGER 32
#endif
#ifdef _WIN32
#define GLOBAL_PLATFORM GLOBAL_PLATFORM_WIN32
#else
#define GLOBAL_PLATFORM GLOBAL_PLATFORM_LINUX
#endif
/* ************************************************* */

/* DEBUGGING OPTIONS ******************************* */
#define NTVDM64_TYPE_TRACE_ENABLED 0 /* enable trancer */
#define NTVDM64_TYPE_TRACE_DEBUG   0 /* debug tracer itself */
/* ************************************************* */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdatomic.h>
#include <string.h>
#include <memory.h>
#include <time.h>
#include <ctype.h>

/* COMPATIBILITY DEFINITIONS *********************************************** */
#if GLOBAL_PLATFORM == GLOBAL_PLATFORM_WIN32
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

typedef void C_VOID;
typedef char C_CHAR;
typedef signed char C_SCHAR;
typedef unsigned char C_UCHAR;
typedef short C_SHORT;
typedef unsigned short C_USHORT;
typedef int C_INT;
typedef unsigned int C_UINT;
typedef long C_LONG;
typedef unsigned long C_ULONG;
typedef long long C_LLONG;
typedef unsigned long long C_ULLONG;
typedef float C_FLOAT;
typedef double C_DOUBLE;

typedef _Bool STD_BOOL;
typedef size_t STD_SIZE_T;
typedef ptrdiff_t STD_PTRDIFF_T;
typedef FILE STD_FILE;
typedef time_t STD_TIME_T;
typedef va_list STD_VA_LIST;

#define STD_NULL NULL
#define STD_EOF EOF
#define STD_SEEK_SET SEEK_SET
#define STD_SEEK_END SEEK_END
#define STD_STDIN stdin
#define STD_STDOUT stdout
#define STD_STDERR stderr
#define STD_ISSPACE(value) isspace((unsigned char)(value))
#define STD_TOUPPER(value) toupper((unsigned char)(value))

/* Retained NXVM numeric aliases. New code should prefer stdint names. */
#define MAXLINE   0x100

typedef char      ntvdm64_type_string_buffer[MAXLINE];
typedef char     *ntvdm64_type_string_pointer;
typedef uint8_t   ntvdm64_type_unsigned_1;
typedef uint8_t   ntvdm64_type_unsigned_4;
typedef uint8_t   ntvdm64_type_unsigned_8;
typedef int8_t    ntvdm64_type_signed_8;
typedef uint16_t  ntvdm64_type_unsigned_16;
typedef int16_t   ntvdm64_type_signed_16;
typedef uint32_t  ntvdm64_type_unsigned_24;
typedef int32_t   ntvdm64_type_signed_24;
typedef uint32_t  ntvdm64_type_unsigned_32;
typedef int32_t   ntvdm64_type_signed_32;
typedef uint64_t  ntvdm64_type_unsigned_48;
typedef int64_t   ntvdm64_type_signed_48;
typedef uint64_t  ntvdm64_type_unsigned_64;
typedef int64_t   ntvdm64_type_signed_64;
typedef float     ntvdm64_type_float_32;
typedef double    ntvdm64_type_float_64;
#if GLOBAL_SIZE_INTEGER == 64
typedef ntvdm64_type_unsigned_64 ntvdm64_type_native_unsigned;
typedef ntvdm64_type_signed_64 ntvdm64_type_native_signed;
#else
typedef ntvdm64_type_unsigned_32 ntvdm64_type_native_unsigned;
typedef ntvdm64_type_signed_32 ntvdm64_type_native_signed;
#endif
typedef ntvdm64_type_unsigned_1  ntvdm64_type_bool;
typedef ntvdm64_type_native_unsigned ntvdm64_type_virtual_address;
typedef ntvdm64_type_native_unsigned ntvdm64_type_flat_address;

#define NTVDM64_TYPE_REFERENCE_OF(n) ((ntvdm64_type_virtual_address)(&(n)))

#define NTVDM64_TYPE_POINTER_UNSIGNED_1  (ntvdm64_type_unsigned_1 *)
#define NTVDM64_TYPE_POINTER_UNSIGNED_4  (ntvdm64_type_unsigned_4 *)
#define NTVDM64_TYPE_POINTER_UNSIGNED_8  (ntvdm64_type_unsigned_8 *)
#define NTVDM64_TYPE_POINTER_SIGNED_8  (ntvdm64_type_signed_8 *)
#define NTVDM64_TYPE_POINTER_UNSIGNED_16 (ntvdm64_type_unsigned_16 *)
#define NTVDM64_TYPE_POINTER_SIGNED_16 (ntvdm64_type_signed_16 *)
#define NTVDM64_TYPE_POINTER_UNSIGNED_24 (ntvdm64_type_unsigned_24 *)
#define NTVDM64_TYPE_POINTER_SIGNED_24 (ntvdm64_type_signed_24 *)
#define NTVDM64_TYPE_POINTER_UNSIGNED_32 (ntvdm64_type_unsigned_32 *)
#define NTVDM64_TYPE_POINTER_SIGNED_32 (ntvdm64_type_signed_32 *)
#define NTVDM64_TYPE_POINTER_UNSIGNED_48 (ntvdm64_type_unsigned_48 *)
#define NTVDM64_TYPE_POINTER_SIGNED_48 (ntvdm64_type_signed_48 *)
#define NTVDM64_TYPE_POINTER_UNSIGNED_64 (ntvdm64_type_unsigned_64 *)
#define NTVDM64_TYPE_POINTER_SIGNED_64 (ntvdm64_type_signed_64 *)
#define NTVDM64_TYPE_POINTER_FLOAT_32 (ntvdm64_type_float_32 *)
#define NTVDM64_TYPE_POINTER_FLOAT_64 (ntvdm64_type_float_64 *)
#define NTVDM64_TYPE_POINTER_NATIVE_UNSIGNED (ntvdm64_type_native_unsigned *)
#define NTVDM64_TYPE_POINTER_NATIVE_SIGNED (ntvdm64_type_native_signed *)
#define NTVDM64_TYPE_POINTER_BOOL    (ntvdm64_type_bool *)

#define NTVDM64_TYPE_DEREFERENCE_UNSIGNED_1(n)  (*(ntvdm64_type_unsigned_1 *)(n))
#define NTVDM64_TYPE_DEREFERENCE_UNSIGNED_4(n)  (*(ntvdm64_type_unsigned_4 *)(n))
#define NTVDM64_TYPE_DEREFERENCE_UNSIGNED_8(n)  (*(ntvdm64_type_unsigned_8 *)(n))
#define NTVDM64_TYPE_DEREFERENCE_SIGNED_8(n)  (*(ntvdm64_type_signed_8 *)(n))
#define NTVDM64_TYPE_DEREFERENCE_UNSIGNED_16(n) (*(ntvdm64_type_unsigned_16 *)(n))
#define NTVDM64_TYPE_DEREFERENCE_SIGNED_16(n) (*(ntvdm64_type_signed_16 *)(n))
#define NTVDM64_TYPE_DEREFERENCE_UNSIGNED_24(n) (*(ntvdm64_type_unsigned_24 *)(n))
#define NTVDM64_TYPE_DEREFERENCE_SIGNED_24(n) (*(ntvdm64_type_signed_24 *)(n))
#define NTVDM64_TYPE_DEREFERENCE_UNSIGNED_32(n) (*(ntvdm64_type_unsigned_32 *)(n))
#define NTVDM64_TYPE_DEREFERENCE_SIGNED_32(n) (*(ntvdm64_type_signed_32 *)(n))
#define NTVDM64_TYPE_DEREFERENCE_UNSIGNED_48(n) (*(ntvdm64_type_unsigned_48 *)(n))
#define NTVDM64_TYPE_DEREFERENCE_SIGNED_48(n) (*(ntvdm64_type_signed_48 *)(n))
#define NTVDM64_TYPE_DEREFERENCE_UNSIGNED_64(n) (*(ntvdm64_type_unsigned_64 *)(n))
#define NTVDM64_TYPE_DEREFERENCE_SIGNED_64(n) (*(ntvdm64_type_signed_64 *)(n))
#define NTVDM64_TYPE_DEREFERENCE_FLOAT_32(n) (*(ntvdm64_type_float_32 *)(n))
#define NTVDM64_TYPE_DEREFERENCE_FLOAT_64(n) (*(ntvdm64_type_float_64 *)(n))
#define NTVDM64_TYPE_DEREFERENCE_NATIVE_UNSIGNED(n) (*(ntvdm64_type_native_unsigned *)(n))
#define NTVDM64_TYPE_DEREFERENCE_NATIVE_SIGNED(n) (*(ntvdm64_type_native_signed *)(n))
#define NTVDM64_TYPE_DEREFERENCE_BOOL(n)    (*(ntvdm64_type_bool *)(n))

#define NTVDM64_TYPE_GET_BIT(s, f) (!!((s) & (f)))
#define NTVDM64_TYPE_SET_BIT(d, s) ((d) |= (s))
#define NTVDM64_TYPE_CLEAR_BIT(d, s) ((d) &= ~(s))
#define NTVDM64_TYPE_MAKE_BIT(d, s, f) ((f) ? NTVDM64_TYPE_SET_BIT((d), (s)) : NTVDM64_TYPE_CLEAR_BIT((d), (s)))
#define NTVDM64_TYPE_GET_MSB(n, b) (NTVDM64_TYPE_GET_BIT((n), (1 << ((b) - 1))))
#define NTVDM64_TYPE_GET_LSB(n) (NTVDM64_TYPE_GET_BIT((n), 1))
#define NTVDM64_TYPE_FALSE  0x00
#define NTVDM64_TYPE_TRUE   0x01
#define NTVDM64_TYPE_ZERO_1  0x00
#define NTVDM64_TYPE_ZERO_4  0x00
#define NTVDM64_TYPE_ZERO_8  0x00
#define NTVDM64_TYPE_ZERO_16 0x0000
#define NTVDM64_TYPE_ZERO_32 0x00000000
#define NTVDM64_TYPE_ZERO_64 0x0000000000000000
#define NTVDM64_TYPE_MAX_UNSIGNED_1  0x01
#define NTVDM64_TYPE_MAX_UNSIGNED_4  0x0f
#define NTVDM64_TYPE_MAX_UNSIGNED_8  0xff
#define NTVDM64_TYPE_MAX_UNSIGNED_16 0xffff
#define NTVDM64_TYPE_MAX_UNSIGNED_24 0x00ffffff
#define NTVDM64_TYPE_MAX_UNSIGNED_32 0xffffffff
#define NTVDM64_TYPE_MAX_UNSIGNED_48 0x0000ffffffffffff
#define NTVDM64_TYPE_MAX_UNSIGNED_64 0xffffffffffffffff
#define NTVDM64_TYPE_MSB_7  0x40
#define NTVDM64_TYPE_MSB_8  0x80
#define NTVDM64_TYPE_MSB_15 0x4000
#define NTVDM64_TYPE_MSB_16 0x8000
#define NTVDM64_TYPE_MSB_31 0x40000000
#define NTVDM64_TYPE_MSB_32 0x80000000
#define NTVDM64_TYPE_MSB_47 0x0000400000000000
#define NTVDM64_TYPE_MSB_48 0x0000800000000000
#define NTVDM64_TYPE_MSB_63 0x4000000000000000
#define NTVDM64_TYPE_MSB_64 0x8000000000000000
#if GLOBAL_SIZE_INTEGER == 64
#define NTVDM64_TYPE_ZERO_NATIVE NTVDM64_TYPE_ZERO_64
#define NTVDM64_TYPE_MAX_NATIVE_UNSIGNED  NTVDM64_TYPE_MAX_UNSIGNED_64
#define NTVDM64_TYPE_MSB_NATIVE  NTVDM64_TYPE_MSB_64
#else
#define NTVDM64_TYPE_ZERO_NATIVE NTVDM64_TYPE_ZERO_32
#define NTVDM64_TYPE_MAX_NATIVE_UNSIGNED  NTVDM64_TYPE_MAX_UNSIGNED_32
#define NTVDM64_TYPE_MSB_NATIVE  NTVDM64_TYPE_MSB_32
#endif
#define NTVDM64_TYPE_MASK_UNSIGNED_1(n)  ((ntvdm64_type_unsigned_1 )(n) & NTVDM64_TYPE_MAX_UNSIGNED_1 )
#define NTVDM64_TYPE_MASK_UNSIGNED_4(n)  ((ntvdm64_type_unsigned_4 )(n) & NTVDM64_TYPE_MAX_UNSIGNED_4 )
#define NTVDM64_TYPE_MASK_UNSIGNED_8(n)  ((ntvdm64_type_unsigned_8 )(n) & NTVDM64_TYPE_MAX_UNSIGNED_8 )
#define NTVDM64_TYPE_MASK_UNSIGNED_16(n) ((ntvdm64_type_unsigned_16)(n) & NTVDM64_TYPE_MAX_UNSIGNED_16)
#define NTVDM64_TYPE_MASK_UNSIGNED_24(n) ((ntvdm64_type_unsigned_24)(n) & NTVDM64_TYPE_MAX_UNSIGNED_24)
#define NTVDM64_TYPE_MASK_UNSIGNED_32(n) ((ntvdm64_type_unsigned_32)(n) & NTVDM64_TYPE_MAX_UNSIGNED_32)
#define NTVDM64_TYPE_MASK_UNSIGNED_48(n) ((ntvdm64_type_unsigned_48)(n) & NTVDM64_TYPE_MAX_UNSIGNED_48)
#define NTVDM64_TYPE_MASK_UNSIGNED_64(n) ((ntvdm64_type_unsigned_64)(n) & NTVDM64_TYPE_MAX_UNSIGNED_64)
#define NTVDM64_TYPE_MASK_NATIVE_UNSIGNED(n) ((ntvdm64_type_native_unsigned)(n) & NTVDM64_TYPE_MAX_NATIVE_UNSIGNED)
#define NTVDM64_TYPE_GET_MSB_7(n)  ((ntvdm64_type_unsigned_8 )(n) & NTVDM64_TYPE_MSB_7 )
#define NTVDM64_TYPE_GET_MSB_8(n)  ((ntvdm64_type_unsigned_8 )(n) & NTVDM64_TYPE_MSB_8 )
#define NTVDM64_TYPE_GET_MSB_15(n) ((ntvdm64_type_unsigned_16)(n) & NTVDM64_TYPE_MSB_15)
#define NTVDM64_TYPE_GET_MSB_16(n) ((ntvdm64_type_unsigned_16)(n) & NTVDM64_TYPE_MSB_16)
#define NTVDM64_TYPE_GET_MSB_31(n) ((ntvdm64_type_unsigned_32)(n) & NTVDM64_TYPE_MSB_31)
#define NTVDM64_TYPE_GET_MSB_32(n) ((ntvdm64_type_unsigned_32)(n) & NTVDM64_TYPE_MSB_32)
#define NTVDM64_TYPE_GET_MSB_47(n) ((ntvdm64_type_unsigned_48)(n) & NTVDM64_TYPE_MSB_47)
#define NTVDM64_TYPE_GET_MSB_48(n) ((ntvdm64_type_unsigned_48)(n) & NTVDM64_TYPE_MSB_48)
#define NTVDM64_TYPE_GET_MSB_63(n) ((ntvdm64_type_unsigned_64)(n) & NTVDM64_TYPE_MSB_63)
#define NTVDM64_TYPE_GET_MSB_64(n) ((ntvdm64_type_unsigned_64)(n) & NTVDM64_TYPE_MSB_64)
#define NTVDM64_TYPE_GET_MSB_NATIVE(n) ((ntvdm64_type_native_unsigned)(n) & NTVDM64_TYPE_MSB_NATIVE)
#define NTVDM64_TYPE_GET_LSB_UNSIGNED_8(n)  ((ntvdm64_type_unsigned_8 )(n) & 1)
#define NTVDM64_TYPE_GET_LSB_UNSIGNED_16(n) ((ntvdm64_type_unsigned_16)(n) & 1)
#define NTVDM64_TYPE_GET_LSB_UNSIGNED_32(n) ((ntvdm64_type_unsigned_32)(n) & 1)
#define NTVDM64_TYPE_GET_LSB_UNSIGNED_48(n) ((ntvdm64_type_unsigned_48)(n) & 1)
#define NTVDM64_TYPE_GET_LSB_UNSIGNED_64(n) ((ntvdm64_type_unsigned_64)(n) & 1)
#define NTVDM64_TYPE_GET_LSB_NATIVE(n) ((ntvdm64_type_native_unsigned)(n) & 1)

#define NTVDM64_TYPE_HEX_TO_BCD(x)  ((((x) / 10) << 4) | ((x) % 10))
#define NTVDM64_TYPE_BCD_TO_HEX(x)  (((x) & 0x0f) + ((((x) & 0xf0) >> 4) * 10))

#define NTVDM64_TYPE_EXECUTE_FUNCTION(faddr) ((faddr) ? ((*(void (*)(void))(faddr))()) : 0)

typedef enum ntvdm64_status {
    NTVDM64_STATUS_OK = 0,
    NTVDM64_STATUS_INVALID_ARGUMENT,
    NTVDM64_STATUS_INVALID_STATE,
    NTVDM64_STATUS_UNSUPPORTED,
    NTVDM64_STATUS_NO_MEMORY,
    NTVDM64_STATUS_FAULT
} ntvdm64_status;

/* Legacy C-runtime wrappers are a product-neutral root foundation. */
struct tm* STD_LOCALTIME(const STD_TIME_T *_Time);
C_CHAR* STD_STRCAT(C_CHAR *_Dest, const C_CHAR *_Source);
C_CHAR* STD_STRCPY(C_CHAR *_Dest, const C_CHAR *_Source);
C_CHAR* STD_STRTOK(C_CHAR *_Str, const C_CHAR *_Delim);
C_INT STD_STRCMP(const C_CHAR *_Str1, const C_CHAR *_Str2);
STD_SIZE_T STD_STRLEN(const C_CHAR *_Str);
C_INT STD_PRINTF(const C_CHAR *_Format, ...);
C_INT STD_FPRINTF(STD_FILE *_File, const C_CHAR *_Format, ...);
C_INT STD_SPRINTF(C_CHAR *_Dest, const C_CHAR *_Format, ...);
C_INT STD_SNPRINTF(C_CHAR *_Dest, STD_SIZE_T _Size, const C_CHAR *_Format, ...);
STD_FILE* STD_FOPEN(const C_CHAR *_Filename, const C_CHAR *_Mode);
C_INT STD_FCLOSE(STD_FILE *_File);
STD_SIZE_T STD_FREAD(C_VOID *_DstBuf, STD_SIZE_T _ElementSize, STD_SIZE_T _Count, STD_FILE *_File);
STD_SIZE_T STD_FWRITE(const C_VOID *_Buffer, STD_SIZE_T _Size, STD_SIZE_T _Count, STD_FILE *_File);
C_CHAR* STD_FGETS(C_CHAR *_Buf, C_INT _MaxCount, STD_FILE *_File);
C_INT STD_FSEEK(STD_FILE *_File, C_LONG _Offset, C_INT _Origin);
C_LONG STD_FTELL(STD_FILE *_File);
C_INT STD_FGETC(STD_FILE *_File);
C_INT STD_FPUTC(C_INT _Character, STD_FILE *_File);
C_INT STD_FEOF(STD_FILE *_File);
STD_TIME_T STD_TIME(STD_TIME_T *_Time);
C_VOID* STD_CALLOC(STD_SIZE_T _Count, STD_SIZE_T _Size);
C_VOID* STD_MALLOC(STD_SIZE_T _Size);
C_VOID STD_FREE(C_VOID *_Memory);
C_VOID* STD_MEMSET(C_VOID *_Dst, C_INT _Val, STD_SIZE_T _Size);
C_VOID* STD_MEMCPY(C_VOID *_Dst, const C_VOID *_Src, STD_SIZE_T _Size);
C_INT STD_MEMCMP(const C_VOID *_Buf1, const C_VOID *_Buf2, STD_SIZE_T _Size);
void ntvdm64_type_string_lower(char *str);

/* Legacy trace support is shared root diagnostic infrastructure. */
#define NTVDM64_TYPE_TRACE_MAX_STACK 0x100
typedef struct {
    char* blockStack[NTVDM64_TYPE_TRACE_MAX_STACK];
    size_t blockCount;
    char *callName;
} ntvdm64_type_trace_call;
typedef struct {
    ntvdm64_type_trace_call callStack[NTVDM64_TYPE_TRACE_MAX_STACK];
    size_t callCount;
    int flagError;
} ntvdm64_type_trace;

void ntvdm64_type_trace_print(ntvdm64_type_trace *rtrace);
void ntvdm64_type_trace_initialize(ntvdm64_type_trace *rtrace);
void ntvdm64_type_trace_finalize(ntvdm64_type_trace *rtrace);
void ntvdm64_type_trace_call_begin(ntvdm64_type_trace *rtrace, char *callName);
void ntvdm64_type_trace_call_end(ntvdm64_type_trace *rtrace);
void ntvdm64_type_trace_block_begin(ntvdm64_type_trace *rtrace, char *blockName);
void ntvdm64_type_trace_block_end(ntvdm64_type_trace *rtrace);

/* #define NTVDM64_TYPE_TRACE_CONTEXT    tracer variable */
/* #define NTVDM64_TYPE_TRACE_ERROR  error condition */
/* #define NTVDM64_TYPE_TRACE_SET_ERROR set error statement */

#if NTVDM64_TYPE_TRACE_ENABLED == 1
#define NTVDM64_TYPE_TRACE_CALL_BEGIN(callName)  ntvdm64_type_trace_call_begin(&(NTVDM64_TYPE_TRACE_CONTEXT), (callName))
#define NTVDM64_TYPE_TRACE_BLOCK_BEGIN(blockName) ntvdm64_type_trace_block_begin(&(NTVDM64_TYPE_TRACE_CONTEXT), (blockName))
#define NTVDM64_TYPE_TRACE_CALL_END ntvdm64_type_trace_call_end(&(NTVDM64_TYPE_TRACE_CONTEXT))
#define NTVDM64_TYPE_TRACE_BLOCK_END ntvdm64_type_trace_block_end(&(NTVDM64_TYPE_TRACE_CONTEXT))
#define NTVDM64_TYPE_TRACE_CHECK_BREAK(n) \
    if (1) { \
        (n); \
        if (NTVDM64_TYPE_TRACE_ERROR) { \
            (NTVDM64_TYPE_TRACE_CONTEXT).flagError = 1; \
            ntvdm64_type_trace_finalize(&(NTVDM64_TYPE_TRACE_CONTEXT)); \
            break; \
        } \
    } else
#define NTVDM64_TYPE_TRACE_CHECK_RETURN(n) \
    do { \
        (n); \
        if (NTVDM64_TYPE_TRACE_ERROR) { \
            (NTVDM64_TYPE_TRACE_CONTEXT).flagError = 1; \
            ntvdm64_type_trace_finalize(&(NTVDM64_TYPE_TRACE_CONTEXT)); \
            return; \
        } \
    } while (0)
#define NTVDM64_TYPE_TRACE_CHECK_RETURN_ZERO(n) \
    do { \
        (n); \
        if (NTVDM64_TYPE_TRACE_ERROR) { \
            (NTVDM64_TYPE_TRACE_CONTEXT).flagError = 1; \
            ntvdm64_type_trace_finalize(&(NTVDM64_TYPE_TRACE_CONTEXT)); \
            return 0; \
        } \
    } while (0)
#else
#define NTVDM64_TYPE_TRACE_CALL_BEGIN(callName)
#define NTVDM64_TYPE_TRACE_BLOCK_BEGIN(blockName)
#define NTVDM64_TYPE_TRACE_CALL_END
#define NTVDM64_TYPE_TRACE_BLOCK_END
#define NTVDM64_TYPE_TRACE_CHECK_BREAK(n) \
    if (1) { \
        (n); \
        if (NTVDM64_TYPE_TRACE_ERROR) { \
            break; \
        } \
    } else
#define NTVDM64_TYPE_TRACE_CHECK_RETURN(n) \
    do { \
        (n); \
        if (NTVDM64_TYPE_TRACE_ERROR) { \
            return; \
        } \
    } while (0)
#define NTVDM64_TYPE_TRACE_CHECK_RETURN_ZERO(n) \
    do { \
        (n); \
        if (NTVDM64_TYPE_TRACE_ERROR) { \
            return 0; \
        } \
    } while (0)
#endif

#define NTVDM64_TYPE_TRACE_IMPOSSIBLE_BREAK  NTVDM64_TYPE_TRACE_CHECK_BREAK(NTVDM64_TYPE_TRACE_SET_ERROR);
#define NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN  NTVDM64_TYPE_TRACE_CHECK_RETURN(NTVDM64_TYPE_TRACE_SET_ERROR);
#define NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO NTVDM64_TYPE_TRACE_CHECK_RETURN_ZERO(NTVDM64_TYPE_TRACE_SET_ERROR);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
