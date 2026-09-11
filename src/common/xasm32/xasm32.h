#ifndef COMMON_XASM32_INTERNAL_H
#define COMMON_XASM32_INTERNAL_H

#include "lib/types/types_interface.h"

#include <ctype.h>
#include <string.h>

#define XASM32_TEXT_CAPACITY 0x100u

typedef struct xasm32_trace {
    int flag_error;
} xasm32_trace;

static inline void xasm32_trace_initialize(xasm32_trace *trace)
{
    trace->flag_error = LIB_FALSE;
}

static inline void xasm32_trace_finalize(xasm32_trace *trace)
{
    (void)trace;
}

static inline void xasm32_string_lower(char *text)
{
    while (*text != '\0') {
        *text = (char)tolower((unsigned char)*text);
        ++text;
    }
}

static inline lib_status xasm32_copy_text(char *destination,
    lib_size destination_capacity, const char *source)
{
    lib_size source_bytes;

    if (destination == LIB_NULL || source == LIB_NULL || destination_capacity == 0u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    source_bytes = strlen(source);
    if (source_bytes >= destination_capacity) return LIB_STATUS_LIMIT_EXCEEDED;
    memcpy(destination, source, source_bytes + 1u);
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
    destination_bytes = strlen(destination);
    source_bytes = strlen(source);
    if (destination_bytes >= destination_capacity ||
        source_bytes >= destination_capacity - destination_bytes) {
        return LIB_STATUS_LIMIT_EXCEEDED;
    }
    memcpy(destination + destination_bytes, source, source_bytes + 1u);
    return LIB_STATUS_OK;
}

#define XASM32_TRACE_ENABLED 0
#define XASM32_TRACE_CONTEXT trace
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
