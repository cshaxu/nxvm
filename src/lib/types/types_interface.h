#ifndef LIB_TYPES_INTERFACE_H
#define LIB_TYPES_INTERFACE_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t lib_u8;
typedef uint16_t lib_u16;
typedef uint32_t lib_u32;
typedef uint64_t lib_u64;
typedef int32_t lib_i32;
typedef int64_t lib_i64;
typedef size_t lib_size;
typedef intptr_t lib_iptr;
typedef int lib_bool;

typedef int lib_status;


enum {
    LIB_STATUS_OK = 0,
    LIB_STATUS_INVALID_ARGUMENT = 1,
    LIB_STATUS_INVALID_STATE = 2,
    LIB_STATUS_UNSUPPORTED = 3,
    LIB_STATUS_NO_MEMORY = 4,
    LIB_STATUS_IO_ERROR = 5,
    LIB_STATUS_LIMIT_EXCEEDED = 7
};

#define LIB_FALSE 0
#define LIB_TRUE 1
#define LIB_NULL NULL
#define LIB_INT32_MAX INT32_MAX
#define LIB_INT32_MIN INT32_MIN
#define LIB_UINT32_MAX UINT32_MAX
#define LIB_UINT64_MAX UINT64_MAX
#define LIB_SIZE_MAX SIZE_MAX

/* Cross-platform C runtime vocabulary.  These functions deliberately expose
 * no platform handle, product state, or I/O policy. */
static inline void *lib_memory_set(void *destination, int value,
    lib_size byte_count)
{ return memset(destination, value, byte_count); }

static inline void *lib_memory_copy(void *destination, const void *source,
    lib_size byte_count)
{ return memcpy(destination, source, byte_count); }

static inline void *lib_memory_move(void *destination, const void *source,
    lib_size byte_count)
{ return memmove(destination, source, byte_count); }

static inline int lib_memory_compare(const void *left, const void *right,
    lib_size byte_count)
{ return memcmp(left, right, byte_count); }

static inline const void *lib_memory_find(const void *bytes, int value,
    lib_size byte_count)
{ return memchr(bytes, value, byte_count); }

static inline lib_size lib_text_length(const char *text)
{ return strlen(text); }

static inline void *lib_allocate(lib_size byte_count)
{ return malloc(byte_count); }

static inline void *lib_allocate_zero(lib_size count, lib_size byte_count)
{ return calloc(count, byte_count); }

static inline void *lib_reallocate(void *memory, lib_size byte_count)
{ return realloc(memory, byte_count); }

static inline void lib_release(void *memory)
{ free(memory); }

#include "lib/types/atomic.h"

#endif
