#ifndef CORE_PLATFORM_INPUT_INTERFACE_H
#define CORE_PLATFORM_INPUT_INTERFACE_H

#include "type.h"

typedef enum core_platform_input_kind {
    CORE_PLATFORM_INPUT_KEY,
    CORE_PLATFORM_INPUT_RELATIVE_MOUSE
} core_platform_input_kind;

typedef struct core_platform_input_event {
    core_platform_input_kind kind;
    union {
        struct {
            type_unsigned_16 scan_code;
            type_unsigned_16 virtual_key;
            C_INT pressed;
        } key;
        struct {
            type_signed_16 delta_x;
            type_signed_16 delta_y;
            type_unsigned_8 buttons;
        } relative_mouse;
    } data;
} core_platform_input_event;

typedef struct core_platform_input_sink {
    C_VOID (*submit)(C_VOID *context,
        const core_platform_input_event *event);
} core_platform_input_sink;

/* This source transfers copied event values only. stop() synchronizes with an
 * in-flight submit callback and prevents all later submissions. */
typedef struct core_platform_input_source core_platform_input_source;

type_status core_platform_input_source_create(const core_platform_input_sink *sink,
    C_VOID *context, core_platform_input_source **out_source);
type_status core_platform_input_source_submit(core_platform_input_source *source,
    const core_platform_input_event *event);
C_VOID core_platform_input_source_destroy(core_platform_input_source *source);

#endif
