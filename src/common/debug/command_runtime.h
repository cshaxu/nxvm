#ifndef COMMON_DEBUG_COMMAND_RUNTIME_H
#define COMMON_DEBUG_COMMAND_RUNTIME_H

#include "lib/types/types_interface.h"

/* The retained DOS DEBUG command body predates lib/types.  These local
 * spellings preserve its table-driven source without creating a second
 * C-runtime facade: each maps directly to the public lib/types vocabulary. */
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
#define STD_STRCMP lib_text_compare
#define STD_STRTOK lib_text_tokenize
#define STD_CALLOC lib_allocate_zero
#define STD_MALLOC lib_allocate
#define STD_FREE lib_release
#define STD_SNPRINTF lib_text_format
#define TYPE_FALSE 0
#define TYPE_TRUE 1
#define TYPE_STATUS_OK LIB_STATUS_OK
#define TYPE_STATUS_INVALID_ARGUMENT LIB_STATUS_INVALID_ARGUMENT
#define TYPE_STATUS_INVALID_STATE LIB_STATUS_INVALID_STATE
#define TYPE_STATUS_UNSUPPORTED LIB_STATUS_UNSUPPORTED
#define TYPE_STATUS_NO_MEMORY LIB_STATUS_NO_MEMORY
#define TYPE_STATUS_FAULT LIB_STATUS_UNSUPPORTED

#define type_string_lower lib_text_ascii_lower
#define STD_SNPRINTF_APPEND lib_text_format_append

#endif
