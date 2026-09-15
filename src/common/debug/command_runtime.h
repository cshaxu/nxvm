#ifndef COMMON_DEBUG_COMMAND_RUNTIME_H
#define COMMON_DEBUG_COMMAND_RUNTIME_H

#include "lib/types/types_interface.h"
#include "lib/types/file.h"

/* The retained DOS DEBUG command body keeps its original table-oriented
 * vocabulary while using the Lib Types C-runtime definitions. */
typedef void C_VOID;
typedef char C_CHAR;
typedef int C_INT;
typedef lib_size STD_SIZE_T;

typedef lib_u8 type_unsigned_8;
typedef lib_u16 type_unsigned_16;
typedef lib_u32 type_unsigned_32;
typedef lib_u64 type_unsigned_64;
typedef lib_status type_status;

#define STD_NULL LIB_NULL
#define STD_SIZE_MAX ((STD_SIZE_T)-1)
#define STD_MEMSET lib_memory_set
#define STD_MEMCPY lib_memory_copy
#define STD_MEMMOVE lib_memory_move
#define STD_MEMCMP lib_memory_compare
#define STD_STRLEN lib_text_length
#define STD_STRCMP lib_c_strcmp
#define STD_STRTOK lib_c_strtok
#define STD_CALLOC lib_allocate_zero
#define STD_MALLOC lib_allocate
#define STD_FREE lib_release
#define STD_SNPRINTF lib_c_snprintf
#define TYPE_FALSE 0
#define TYPE_TRUE 1
#define TYPE_STATUS_OK LIB_STATUS_OK
#define TYPE_STATUS_INVALID_ARGUMENT LIB_STATUS_INVALID_ARGUMENT
#define TYPE_STATUS_INVALID_STATE LIB_STATUS_INVALID_STATE
#define TYPE_STATUS_UNSUPPORTED LIB_STATUS_UNSUPPORTED
#define TYPE_STATUS_NO_MEMORY LIB_STATUS_NO_MEMORY
#define TYPE_STATUS_FAULT LIB_STATUS_UNSUPPORTED

static inline void command_string_lower(char *text)
{
    while (text != STD_NULL && *text != '\0') {
        if (*text >= 'A' && *text <= 'Z')
            *text = (char)(*text + ('a' - 'A'));
        ++text;
    }
}

static inline int command_format_append_result(char **cursor,
    STD_SIZE_T *remaining, int written)
{
    if (cursor == STD_NULL || *cursor == STD_NULL || remaining == STD_NULL ||
        *remaining == 0u) return -1;
    if (written < 0 || (STD_SIZE_T)written >= *remaining) return -1;
    *cursor += written;
    *remaining -= (STD_SIZE_T)written;
    return written;
}

#define type_string_lower command_string_lower
#define STD_SNPRINTF_APPEND(cursor, remaining, ...) \
    ((cursor) == STD_NULL || *(cursor) == STD_NULL || (remaining) == STD_NULL || \
        *(remaining) == 0u ? -1 : command_format_append_result((cursor), \
            (remaining), lib_c_snprintf(*(cursor), *(remaining), __VA_ARGS__)))

#endif
