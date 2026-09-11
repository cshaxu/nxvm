#ifndef COMMON_DEBUG_COMMAND_RUNTIME_H
#define COMMON_DEBUG_COMMAND_RUNTIME_H

#include "lib/types/types_interface.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The retained DOS DEBUG command body predates lib/types.  These local
 * spellings deliberately exist only while that body is migrated; they do
 * not form a common API or pull the retired project type facade into common. */
typedef void C_VOID;
typedef char C_CHAR;
typedef int C_INT;
typedef size_t STD_SIZE_T;

typedef lib_u8 type_unsigned_8;
typedef lib_u16 type_unsigned_16;
typedef lib_u32 type_unsigned_32;
typedef lib_u64 type_unsigned_64;
typedef lib_status type_status;

#define STD_NULL LIB_NULL
#define STD_SIZE_MAX ((STD_SIZE_T)-1)
#define STD_MEMSET memset
#define STD_MEMCPY memcpy
#define STD_MEMMOVE memmove
#define STD_MEMCMP memcmp
#define STD_STRLEN strlen
#define STD_STRCMP strcmp
#define STD_STRTOK strtok
#define STD_CALLOC calloc
#define STD_MALLOC malloc
#define STD_FREE free
#define STD_SNPRINTF snprintf
#define TYPE_FALSE 0
#define TYPE_TRUE 1
#define TYPE_STATUS_OK LIB_STATUS_OK
#define TYPE_STATUS_INVALID_ARGUMENT LIB_STATUS_INVALID_ARGUMENT
#define TYPE_STATUS_INVALID_STATE LIB_STATUS_INVALID_STATE
#define TYPE_STATUS_UNSUPPORTED LIB_STATUS_UNSUPPORTED
#define TYPE_STATUS_NO_MEMORY LIB_STATUS_NO_MEMORY
#define TYPE_STATUS_FAULT LIB_STATUS_FAULT

static inline void command_string_lower(C_CHAR *text)
{
    if (text == STD_NULL) return;
    while (*text != '\0') {
        *text = (C_CHAR)tolower((unsigned char)*text);
        ++text;
    }
}

static inline C_INT command_snprintf_append(C_CHAR **cursor,
    STD_SIZE_T *remaining, const C_CHAR *format, ...)
{
    C_INT written;
    va_list arguments;

    if (cursor == STD_NULL || *cursor == STD_NULL || remaining == STD_NULL ||
        *remaining == 0u || format == STD_NULL) return -1;
    va_start(arguments, format);
    written = vsnprintf(*cursor, *remaining, format, arguments);
    va_end(arguments);
    if (written < 0 || (STD_SIZE_T)written >= *remaining) return -1;
    *cursor += written;
    *remaining -= (STD_SIZE_T)written;
    return written;
}

#define type_string_lower command_string_lower
#define STD_SNPRINTF_APPEND command_snprintf_append

#endif
