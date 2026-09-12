#ifndef COMMON_XASM32_INTERNAL_H
#define COMMON_XASM32_INTERNAL_H

#include "lib/types/types_interface.h"

#define XASM32_TEXT_CAPACITY 0x100u

static inline void xasm32_string_lower(char *text)
{
    lib_text_ascii_lower(text);
}

static inline lib_status xasm32_copy_text(char *destination,
    lib_size destination_capacity, const char *source)
{
    lib_size source_bytes;

    if (destination == LIB_NULL || source == LIB_NULL || destination_capacity == 0u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    source_bytes = lib_text_length(source);
    if (source_bytes >= destination_capacity) return LIB_STATUS_LIMIT_EXCEEDED;
    lib_memory_copy(destination, source, source_bytes + 1u);
    return LIB_STATUS_OK;
}

static inline lib_status xasm32_append_text(char *destination,
    lib_size destination_capacity, const char *source)
{
    lib_size destination_bytes;
    lib_size source_bytes;

    if (destination == LIB_NULL || source == LIB_NULL || destination_capacity == 0u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    destination_bytes = lib_text_length(destination);
    source_bytes = lib_text_length(source);
    if (destination_bytes >= destination_capacity ||
        source_bytes >= destination_capacity - destination_bytes) {
        return LIB_STATUS_LIMIT_EXCEEDED;
    }
    lib_memory_copy(destination + destination_bytes, source, source_bytes + 1u);
    return LIB_STATUS_OK;
}

/* These legacy parser-flow markers preserve the original table-driven return
 * behavior.  They do not collect, print, or retain a development trace. */
#define XASM32_TRACE_ERROR flagError
#define XASM32_TRACE_SET_ERROR (flagError = LIB_TRUE)
#define XASM32_TRACE_CALL_BEGIN(name)
#define XASM32_TRACE_BLOCK_BEGIN(name)
#define XASM32_TRACE_CALL_END
#define XASM32_TRACE_BLOCK_END
#define XASM32_TRACE_CHECK_RETURN(expression) \
    do { (expression); if (XASM32_TRACE_ERROR) return; } while (0)
#define XASM32_TRACE_CHECK_RETURN_ZERO(expression) \
    do { (expression); if (XASM32_TRACE_ERROR) return 0u; } while (0)
#define XASM32_TRACE_CHECK_BREAK(expression) \
    if (1) { (expression); if (XASM32_TRACE_ERROR) break; } else
#define XASM32_TRACE_IMPOSSIBLE_RETURN XASM32_TRACE_CHECK_RETURN(XASM32_TRACE_SET_ERROR)

#endif
