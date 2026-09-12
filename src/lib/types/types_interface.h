#ifndef LIB_TYPES_INTERFACE_H
#define LIB_TYPES_INTERFACE_H

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>

typedef uint8_t lib_u8;
typedef uint16_t lib_u16;
typedef uint32_t lib_u32;
typedef uint64_t lib_u64;
typedef int32_t lib_i32;
typedef int64_t lib_i64;
typedef size_t lib_size;
typedef int lib_bool;
typedef va_list lib_format_arguments;

typedef int lib_status;

enum {
    LIB_STATUS_OK = 0,
    LIB_STATUS_INVALID_ARGUMENT = 1,
    LIB_STATUS_INVALID_STATE = 2,
    LIB_STATUS_UNSUPPORTED = 3,
    LIB_STATUS_NO_MEMORY = 4,
    LIB_STATUS_IO_ERROR = 5,
    LIB_STATUS_NOT_CURRENT = 6,
    LIB_STATUS_LIMIT_EXCEEDED = 7
};

#define LIB_FALSE 0
#define LIB_TRUE 1
#define LIB_NULL NULL

/* Cross-platform C runtime vocabulary.  These functions deliberately expose
 * no platform handle, product state, or I/O policy. */
void *lib_memory_set(void *destination, int value, lib_size byte_count);
void *lib_memory_copy(void *destination, const void *source, lib_size byte_count);
void *lib_memory_move(void *destination, const void *source, lib_size byte_count);
int lib_memory_compare(const void *left, const void *right, lib_size byte_count);

lib_size lib_text_length(const char *text);
int lib_text_compare(const char *left, const char *right);
char *lib_text_tokenize(char *text, const char *delimiters);
void lib_text_ascii_lower(char *text);
int lib_text_format(char *destination, lib_size destination_capacity,
    const char *format, ...);
int lib_text_format_v(char *destination, lib_size destination_capacity,
    const char *format, lib_format_arguments arguments);
int lib_text_format_append(char **cursor, lib_size *remaining,
    const char *format, ...);
int lib_text_format_append_v(char **cursor, lib_size *remaining,
    const char *format, lib_format_arguments arguments);

void *lib_allocate(lib_size byte_count);
void *lib_allocate_zero(lib_size count, lib_size byte_count);
void lib_release(void *memory);

#include "lib/types/atomic.h"

#endif
