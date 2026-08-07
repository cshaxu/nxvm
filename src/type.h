/* Copyright 2012-2014 Neko. */

/* Shared project types and compile-STD_TIME settings. */

#ifndef TYPE_H
#define TYPE_H

#ifdef __cplusplus
extern "C"
{
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
#define TYPE_TRACE_ENABLED 0 /* enable trancer */
#define TYPE_TRACE_DEBUG 0   /* debug tracer itself */
    /* ************************************************* */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdatomic.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdint.h>

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
#define STD_SEEK_CUR SEEK_CUR
#define STD_SEEK_END SEEK_END
#define STD_STDIN stdin
#define STD_STDOUT stdout
#define STD_STDERR stderr
#define STD_ISSPACE(value) isspace((unsigned char)(value))
#define STD_TOUPPER(value) toupper((unsigned char)(value))
#define STD_ISALPHA(value) isalpha((unsigned char)(value))
#define STD_ISPRINT(value) isprint((unsigned char)(value))

    typedef atomic_bool STD_ATOMIC_BOOL;
    typedef atomic_int STD_ATOMIC_INT;
    typedef atomic_uintptr_t STD_ATOMIC_UINTPTR_T;
    typedef atomic_flag STD_ATOMIC_FLAG;

#define STD_ATOMIC_INIT atomic_init
#define STD_ATOMIC_LOAD atomic_load
#define STD_ATOMIC_STORE atomic_store
#define STD_ATOMIC_EXCHANGE atomic_exchange
#define STD_ATOMIC_COMPARE_EXCHANGE_STRONG atomic_compare_exchange_strong
#define STD_ATOMIC_EXCHANGE_EXPLICIT atomic_exchange_explicit
#define STD_ATOMIC_STORE_EXPLICIT atomic_store_explicit
#define STD_ATOMIC_FLAG_TEST_AND_SET_EXPLICIT atomic_flag_test_and_set_explicit
#define STD_ATOMIC_FLAG_CLEAR_EXPLICIT atomic_flag_clear_explicit
#define STD_MEMORY_ORDER_RELAXED memory_order_relaxed
#define STD_MEMORY_ORDER_CONSUME memory_order_consume
#define STD_MEMORY_ORDER_ACQUIRE memory_order_acquire
#define STD_MEMORY_ORDER_RELEASE memory_order_release
#define STD_MEMORY_ORDER_ACQ_REL memory_order_acq_rel
#define STD_MEMORY_ORDER_SEQ_CST memory_order_seq_cst

/* Retained VM numeric aliases. New code should prefer stdint names. */
#define MAXLINE 0x100

    typedef C_CHAR type_string_buffer[MAXLINE];
    typedef C_CHAR *type_string_pointer;
    typedef uint8_t type_unsigned_1;
    typedef uint8_t type_unsigned_4;
    typedef uint8_t type_unsigned_8;
    typedef int8_t type_signed_8;
    typedef uint16_t type_unsigned_16;
    typedef int16_t type_signed_16;
    typedef uint32_t type_unsigned_24;
    typedef int32_t type_signed_24;
    typedef uint32_t type_unsigned_32;
    typedef int32_t type_signed_32;
    typedef uint64_t type_unsigned_48;
    typedef int64_t type_signed_48;
    typedef uint64_t type_unsigned_64;
    typedef int64_t type_signed_64;
    typedef C_FLOAT type_float_32;
    typedef C_DOUBLE type_float_64;
#if GLOBAL_SIZE_INTEGER == 64
    typedef type_unsigned_64 type_native_unsigned;
    typedef type_signed_64 type_native_signed;
#else
typedef type_unsigned_32 type_native_unsigned;
typedef type_signed_32 type_native_signed;
#endif
    typedef type_unsigned_1 type_bool;
    typedef type_native_unsigned type_virtual_address;
    typedef type_native_unsigned type_flat_address;

#define TYPE_REFERENCE_OF(n) ((type_virtual_address)(&(n)))

#define TYPE_POINTER_UNSIGNED_1 (type_unsigned_1 *)
#define TYPE_POINTER_UNSIGNED_4 (type_unsigned_4 *)
#define TYPE_POINTER_UNSIGNED_8 (type_unsigned_8 *)
#define TYPE_POINTER_SIGNED_8 (type_signed_8 *)
#define TYPE_POINTER_UNSIGNED_16 (type_unsigned_16 *)
#define TYPE_POINTER_SIGNED_16 (type_signed_16 *)
#define TYPE_POINTER_UNSIGNED_24 (type_unsigned_24 *)
#define TYPE_POINTER_SIGNED_24 (type_signed_24 *)
#define TYPE_POINTER_UNSIGNED_32 (type_unsigned_32 *)
#define TYPE_POINTER_SIGNED_32 (type_signed_32 *)
#define TYPE_POINTER_UNSIGNED_48 (type_unsigned_48 *)
#define TYPE_POINTER_SIGNED_48 (type_signed_48 *)
#define TYPE_POINTER_UNSIGNED_64 (type_unsigned_64 *)
#define TYPE_POINTER_SIGNED_64 (type_signed_64 *)
#define TYPE_POINTER_FLOAT_32 (type_float_32 *)
#define TYPE_POINTER_FLOAT_64 (type_float_64 *)
#define TYPE_POINTER_NATIVE_UNSIGNED (type_native_unsigned *)
#define TYPE_POINTER_NATIVE_SIGNED (type_native_signed *)
#define TYPE_POINTER_BOOL (type_bool *)

#define TYPE_DEREFERENCE_UNSIGNED_1(n) (*(type_unsigned_1 *)(n))
#define TYPE_DEREFERENCE_UNSIGNED_4(n) (*(type_unsigned_4 *)(n))
#define TYPE_DEREFERENCE_UNSIGNED_8(n) (*(type_unsigned_8 *)(n))
#define TYPE_DEREFERENCE_SIGNED_8(n) (*(type_signed_8 *)(n))
#define TYPE_DEREFERENCE_UNSIGNED_16(n) (*(type_unsigned_16 *)(n))
#define TYPE_DEREFERENCE_SIGNED_16(n) (*(type_signed_16 *)(n))
#define TYPE_DEREFERENCE_UNSIGNED_24(n) (*(type_unsigned_24 *)(n))
#define TYPE_DEREFERENCE_SIGNED_24(n) (*(type_signed_24 *)(n))
#define TYPE_DEREFERENCE_UNSIGNED_32(n) (*(type_unsigned_32 *)(n))
#define TYPE_DEREFERENCE_SIGNED_32(n) (*(type_signed_32 *)(n))
#define TYPE_DEREFERENCE_UNSIGNED_48(n) (*(type_unsigned_48 *)(n))
#define TYPE_DEREFERENCE_SIGNED_48(n) (*(type_signed_48 *)(n))
#define TYPE_DEREFERENCE_UNSIGNED_64(n) (*(type_unsigned_64 *)(n))
#define TYPE_DEREFERENCE_SIGNED_64(n) (*(type_signed_64 *)(n))
#define TYPE_DEREFERENCE_FLOAT_32(n) (*(type_float_32 *)(n))
#define TYPE_DEREFERENCE_FLOAT_64(n) (*(type_float_64 *)(n))
#define TYPE_DEREFERENCE_NATIVE_UNSIGNED(n) (*(type_native_unsigned *)(n))
#define TYPE_DEREFERENCE_NATIVE_SIGNED(n) (*(type_native_signed *)(n))
#define TYPE_DEREFERENCE_BOOL(n) (*(type_bool *)(n))

#define TYPE_GET_BIT(s, f) (!!((s) & (f)))
#define TYPE_SET_BIT(d, s) ((d) |= (s))
#define TYPE_CLEAR_BIT(d, s) ((d) &= ~(s))
#define TYPE_MAKE_BIT(d, s, f) ((f) ? TYPE_SET_BIT((d), (s)) : TYPE_CLEAR_BIT((d), (s)))
#define TYPE_GET_MSB(n, b) (TYPE_GET_BIT((n), (1 << ((b) - 1))))
#define TYPE_GET_LSB(n) (TYPE_GET_BIT((n), 1))
#define TYPE_FALSE 0x00
#define TYPE_TRUE 0x01
#define TYPE_ZERO_1 0x00
#define TYPE_ZERO_4 0x00
#define TYPE_ZERO_8 0x00
#define TYPE_ZERO_16 0x0000
#define TYPE_ZERO_32 0x00000000
#define TYPE_ZERO_64 0x0000000000000000
#define TYPE_MAX_UNSIGNED_1 0x01
#define TYPE_MAX_UNSIGNED_4 0x0f
#define TYPE_MAX_UNSIGNED_8 0xff
#define TYPE_MAX_UNSIGNED_16 0xffff
#define TYPE_MAX_UNSIGNED_24 0x00ffffff
#define TYPE_MAX_UNSIGNED_32 0xffffffff
#define TYPE_MAX_UNSIGNED_48 0x0000ffffffffffff
#define TYPE_MAX_UNSIGNED_64 0xffffffffffffffff
#define TYPE_MSB_7 0x40
#define TYPE_MSB_8 0x80
#define TYPE_MSB_15 0x4000
#define TYPE_MSB_16 0x8000
#define TYPE_MSB_31 0x40000000
#define TYPE_MSB_32 0x80000000
#define TYPE_MSB_47 0x0000400000000000
#define TYPE_MSB_48 0x0000800000000000
#define TYPE_MSB_63 0x4000000000000000
#define TYPE_MSB_64 0x8000000000000000
#if GLOBAL_SIZE_INTEGER == 64
#define TYPE_ZERO_NATIVE TYPE_ZERO_64
#define TYPE_MAX_NATIVE_UNSIGNED TYPE_MAX_UNSIGNED_64
#define TYPE_MSB_NATIVE TYPE_MSB_64
#else
#define TYPE_ZERO_NATIVE TYPE_ZERO_32
#define TYPE_MAX_NATIVE_UNSIGNED TYPE_MAX_UNSIGNED_32
#define TYPE_MSB_NATIVE TYPE_MSB_32
#endif
#define TYPE_MASK_UNSIGNED_1(n) ((type_unsigned_1)(n) & TYPE_MAX_UNSIGNED_1)
#define TYPE_MASK_UNSIGNED_4(n) ((type_unsigned_4)(n) & TYPE_MAX_UNSIGNED_4)
#define TYPE_MASK_UNSIGNED_8(n) ((type_unsigned_8)(n) & TYPE_MAX_UNSIGNED_8)
#define TYPE_MASK_UNSIGNED_16(n) ((type_unsigned_16)(n) & TYPE_MAX_UNSIGNED_16)
#define TYPE_MASK_UNSIGNED_24(n) ((type_unsigned_24)(n) & TYPE_MAX_UNSIGNED_24)
#define TYPE_MASK_UNSIGNED_32(n) ((type_unsigned_32)(n) & TYPE_MAX_UNSIGNED_32)
#define TYPE_MASK_UNSIGNED_48(n) ((type_unsigned_48)(n) & TYPE_MAX_UNSIGNED_48)
#define TYPE_MASK_UNSIGNED_64(n) ((type_unsigned_64)(n) & TYPE_MAX_UNSIGNED_64)
#define TYPE_MASK_NATIVE_UNSIGNED(n) ((type_native_unsigned)(n) & TYPE_MAX_NATIVE_UNSIGNED)
#define TYPE_GET_MSB_7(n) ((type_unsigned_8)(n) & TYPE_MSB_7)
#define TYPE_GET_MSB_8(n) ((type_unsigned_8)(n) & TYPE_MSB_8)
#define TYPE_GET_MSB_15(n) ((type_unsigned_16)(n) & TYPE_MSB_15)
#define TYPE_GET_MSB_16(n) ((type_unsigned_16)(n) & TYPE_MSB_16)
#define TYPE_GET_MSB_31(n) ((type_unsigned_32)(n) & TYPE_MSB_31)
#define TYPE_GET_MSB_32(n) ((type_unsigned_32)(n) & TYPE_MSB_32)
#define TYPE_GET_MSB_47(n) ((type_unsigned_48)(n) & TYPE_MSB_47)
#define TYPE_GET_MSB_48(n) ((type_unsigned_48)(n) & TYPE_MSB_48)
#define TYPE_GET_MSB_63(n) ((type_unsigned_64)(n) & TYPE_MSB_63)
#define TYPE_GET_MSB_64(n) ((type_unsigned_64)(n) & TYPE_MSB_64)
#define TYPE_GET_MSB_NATIVE(n) ((type_native_unsigned)(n) & TYPE_MSB_NATIVE)
#define TYPE_GET_LSB_UNSIGNED_8(n) ((type_unsigned_8)(n) & 1)
#define TYPE_GET_LSB_UNSIGNED_16(n) ((type_unsigned_16)(n) & 1)
#define TYPE_GET_LSB_UNSIGNED_32(n) ((type_unsigned_32)(n) & 1)
#define TYPE_GET_LSB_UNSIGNED_48(n) ((type_unsigned_48)(n) & 1)
#define TYPE_GET_LSB_UNSIGNED_64(n) ((type_unsigned_64)(n) & 1)
#define TYPE_GET_LSB_NATIVE(n) ((type_native_unsigned)(n) & 1)

#define TYPE_HEX_TO_BCD(x) ((((x) / 10) << 4) | ((x) % 10))
#define TYPE_BCD_TO_HEX(x) (((x) & 0x0f) + ((((x) & 0xf0) >> 4) * 10))

#define TYPE_EXECUTE_FUNCTION(faddr) ((faddr) ? ((*(C_VOID (*)(C_VOID))(faddr))()) : 0)

    typedef enum type_status
    {
        TYPE_STATUS_OK = 0,
        TYPE_STATUS_INVALID_ARGUMENT,
        TYPE_STATUS_INVALID_STATE,
        TYPE_STATUS_UNSUPPORTED,
        TYPE_STATUS_NO_MEMORY,
        TYPE_STATUS_FAULT
    } type_status;

    /* Legacy C-runtime wrappers are a product-neutral root foundation. */
    struct tm *STD_LOCALTIME(const STD_TIME_T *_Time);
    C_CHAR *STD_STRCAT(C_CHAR *_Dest, const C_CHAR *_Source);
    C_CHAR *STD_STRCPY(C_CHAR *_Dest, const C_CHAR *_Source);
    C_CHAR *STD_STRTOK(C_CHAR *_Str, const C_CHAR *_Delim);
    C_INT STD_STRCMP(const C_CHAR *_Str1, const C_CHAR *_Str2);
    STD_SIZE_T STD_STRLEN(const C_CHAR *_Str);
    C_INT STD_PRINTF(const C_CHAR *_Format, ...);
    C_INT STD_FPRINTF(STD_FILE *_File, const C_CHAR *_Format, ...);
    C_INT STD_SNPRINTF(C_CHAR *_Dest, STD_SIZE_T _Size, const C_CHAR *_Format, ...);
    C_INT STD_SNPRINTF_APPEND(C_CHAR **_Cursor, STD_SIZE_T *_Remaining,
        const C_CHAR *_Format, ...);
    STD_FILE *STD_FOPEN(const C_CHAR *_Filename, const C_CHAR *_Mode);
    C_INT STD_FCLOSE(STD_FILE *_File);
    STD_SIZE_T STD_FREAD(C_VOID *_DstBuf, STD_SIZE_T _ElementSize, STD_SIZE_T _Count, STD_FILE *_File);
    STD_SIZE_T STD_FWRITE(const C_VOID *_Buffer, STD_SIZE_T _Size, STD_SIZE_T _Count, STD_FILE *_File);
    C_CHAR *STD_FGETS(C_CHAR *_Buf, C_INT _MaxCount, STD_FILE *_File);
    C_INT STD_FSEEK(STD_FILE *_File, C_LONG _Offset, C_INT _Origin);
    C_LONG STD_FTELL(STD_FILE *_File);
    C_INT STD_FSEEK_64(STD_FILE *_File, int64_t _Offset, C_INT _Origin);
    int64_t STD_FTELL_64(STD_FILE *_File);
    C_INT STD_FGETC(STD_FILE *_File);
    C_INT STD_FPUTC(C_INT _Character, STD_FILE *_File);
    C_INT STD_FPUTS(const C_CHAR *_String, STD_FILE *_File);
    C_INT STD_FEOF(STD_FILE *_File);
    C_INT STD_REMOVE(const C_CHAR *_Filename);
    C_INT STD_ATOI(const C_CHAR *_String);
    STD_TIME_T STD_TIME(STD_TIME_T *_Time);
    C_VOID *STD_CALLOC(STD_SIZE_T _Count, STD_SIZE_T _Size);
    C_VOID *STD_MALLOC(STD_SIZE_T _Size);
    C_VOID STD_FREE(C_VOID *_Memory);
    C_VOID *STD_MEMSET(C_VOID *_Dst, C_INT _Val, STD_SIZE_T _Size);
    C_VOID *STD_MEMCPY(C_VOID *_Dst, const C_VOID *_Src, STD_SIZE_T _Size);
    C_VOID *STD_MEMMOVE(C_VOID *_Dst, const C_VOID *_Src, STD_SIZE_T _Size);
    C_INT STD_MEMCMP(const C_VOID *_Buf1, const C_VOID *_Buf2, STD_SIZE_T _Size);
    C_VOID type_string_lower(C_CHAR *str);

/* Legacy trace support is shared root diagnostic infrastructure. */
#define TYPE_TRACE_MAX_STACK 0x100
    typedef struct
    {
        C_CHAR *blockStack[TYPE_TRACE_MAX_STACK];
        STD_SIZE_T blockCount;
        C_CHAR *callName;
    } type_trace_call;
    typedef struct
    {
        type_trace_call callStack[TYPE_TRACE_MAX_STACK];
        STD_SIZE_T callCount;
        C_INT flagError;
    } type_trace;

    C_VOID type_trace_print(type_trace *rtrace);
    C_VOID type_trace_initialize(type_trace *rtrace);
    C_VOID type_trace_finalize(type_trace *rtrace);
    C_VOID type_trace_call_begin(type_trace *rtrace, C_CHAR *callName);
    C_VOID type_trace_call_end(type_trace *rtrace);
    C_VOID type_trace_block_begin(type_trace *rtrace, C_CHAR *blockName);
    C_VOID type_trace_block_end(type_trace *rtrace);

    /* #define TYPE_TRACE_CONTEXT    tracer variable */
    /* #define TYPE_TRACE_ERROR  error condition */
    /* #define TYPE_TRACE_SET_ERROR set error statement */

#if TYPE_TRACE_ENABLED == 1
#define TYPE_TRACE_CALL_BEGIN(callName) type_trace_call_begin(&(TYPE_TRACE_CONTEXT), (callName))
#define TYPE_TRACE_BLOCK_BEGIN(blockName) type_trace_block_begin(&(TYPE_TRACE_CONTEXT), (blockName))
#define TYPE_TRACE_CALL_END type_trace_call_end(&(TYPE_TRACE_CONTEXT))
#define TYPE_TRACE_BLOCK_END type_trace_block_end(&(TYPE_TRACE_CONTEXT))
#define TYPE_TRACE_CHECK_BREAK(n)                       \
    if (1)                                              \
    {                                                   \
        (n);                                            \
        if (TYPE_TRACE_ERROR)                           \
        {                                               \
            (TYPE_TRACE_CONTEXT).flagError = 1;         \
            type_trace_finalize(&(TYPE_TRACE_CONTEXT)); \
            break;                                      \
        }                                               \
    }                                                   \
    else
#define TYPE_TRACE_CHECK_RETURN(n)                      \
    do                                                  \
    {                                                   \
        (n);                                            \
        if (TYPE_TRACE_ERROR)                           \
        {                                               \
            (TYPE_TRACE_CONTEXT).flagError = 1;         \
            type_trace_finalize(&(TYPE_TRACE_CONTEXT)); \
            return;                                     \
        }                                               \
    } while (0)
#define TYPE_TRACE_CHECK_RETURN_ZERO(n)                 \
    do                                                  \
    {                                                   \
        (n);                                            \
        if (TYPE_TRACE_ERROR)                           \
        {                                               \
            (TYPE_TRACE_CONTEXT).flagError = 1;         \
            type_trace_finalize(&(TYPE_TRACE_CONTEXT)); \
            return 0;                                   \
        }                                               \
    } while (0)
#else
#define TYPE_TRACE_CALL_BEGIN(callName)
#define TYPE_TRACE_BLOCK_BEGIN(blockName)
#define TYPE_TRACE_CALL_END
#define TYPE_TRACE_BLOCK_END
#define TYPE_TRACE_CHECK_BREAK(n) \
    if (1)                        \
    {                             \
        (n);                      \
        if (TYPE_TRACE_ERROR)     \
        {                         \
            break;                \
        }                         \
    }                             \
    else
#define TYPE_TRACE_CHECK_RETURN(n) \
    do                             \
    {                              \
        (n);                       \
        if (TYPE_TRACE_ERROR)      \
        {                          \
            return;                \
        }                          \
    } while (0)
#define TYPE_TRACE_CHECK_RETURN_ZERO(n) \
    do                                  \
    {                                   \
        (n);                            \
        if (TYPE_TRACE_ERROR)           \
        {                               \
            return 0;                   \
        }                               \
    } while (0)
#endif

#define TYPE_TRACE_IMPOSSIBLE_BREAK TYPE_TRACE_CHECK_BREAK(TYPE_TRACE_SET_ERROR);
#define TYPE_TRACE_IMPOSSIBLE_RETURN TYPE_TRACE_CHECK_RETURN(TYPE_TRACE_SET_ERROR);
#define TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO TYPE_TRACE_CHECK_RETURN_ZERO(TYPE_TRACE_SET_ERROR);

#ifdef __cplusplus
} /*_EOCD_*/
#endif

#endif
