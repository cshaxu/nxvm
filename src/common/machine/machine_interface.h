#ifndef COMMON_MACHINE_INTERFACE_H
#define COMMON_MACHINE_INTERFACE_H

#include "lib/types/types_interface.h"

#define COMMON_MACHINE_DEBUG_BYTES 32u

typedef struct common_machine common_machine;

typedef enum common_machine_input_kind {
    COMMON_MACHINE_INPUT_KEY,
    COMMON_MACHINE_INPUT_RELATIVE_MOUSE
} common_machine_input_kind;

typedef struct common_machine_input {
    common_machine_input_kind kind;
    union {
        struct { lib_u16 scan_code; lib_u16 virtual_key; lib_bool pressed; } key;
        struct { lib_i32 delta_x; lib_i32 delta_y; lib_u8 buttons; } mouse;
    } value;
} common_machine_input;

typedef enum common_machine_request_kind {
    COMMON_MACHINE_REQUEST_INPUT,
    COMMON_MACHINE_REQUEST_PAUSE,
    COMMON_MACHINE_REQUEST_RESET,
    COMMON_MACHINE_REQUEST_RESUME,
    COMMON_MACHINE_REQUEST_STEP,
    COMMON_MACHINE_REQUEST_STOP
} common_machine_request_kind;

typedef enum common_machine_pause_reason {
    COMMON_MACHINE_PAUSE_EXPLICIT,
    COMMON_MACHINE_PAUSE_BREAKPOINT,
    COMMON_MACHINE_PAUSE_TRACE,
    COMMON_MACHINE_PAUSE_STEP
} common_machine_pause_reason;

typedef struct common_machine_request {
    common_machine_request_kind kind;
    common_machine_pause_reason pause_reason;
    lib_u32 run_id;
    common_machine_input input;
} common_machine_request;

typedef enum common_machine_debug_operation {
    COMMON_MACHINE_DEBUG_READ_REGISTER,
    COMMON_MACHINE_DEBUG_WRITE_REGISTER,
    COMMON_MACHINE_DEBUG_READ_LINEAR,
    COMMON_MACHINE_DEBUG_WRITE_LINEAR,
    COMMON_MACHINE_DEBUG_READ_REAL,
    COMMON_MACHINE_DEBUG_WRITE_REAL,
    COMMON_MACHINE_DEBUG_READ_PORT,
    COMMON_MACHINE_DEBUG_WRITE_PORT,
    COMMON_MACHINE_DEBUG_GET_CODE_DEFAULT_SIZE,
    COMMON_MACHINE_DEBUG_GET_CODE_BASE,
    COMMON_MACHINE_DEBUG_GET_CPU_SNAPSHOT,
    COMMON_MACHINE_DEBUG_SET_WATCH,
    COMMON_MACHINE_DEBUG_CLEAR_WATCH,
    COMMON_MACHINE_DEBUG_GET_WATCH,
    COMMON_MACHINE_DEBUG_SET_EXECUTION_PLAN,
    COMMON_MACHINE_DEBUG_CLEAR_EXECUTION_PLAN,
    COMMON_MACHINE_DEBUG_GET_EXECUTION_RESULT
} common_machine_debug_operation;

typedef enum common_machine_debug_watch_kind {
    COMMON_MACHINE_DEBUG_WATCH_READ,
    COMMON_MACHINE_DEBUG_WATCH_WRITE,
    COMMON_MACHINE_DEBUG_WATCH_EXECUTE
} common_machine_debug_watch_kind;

/* The target owns enforcement.  Debug supplies only a bounded plan while
 * paused; it never counts instructions from an observation callback. */
typedef enum common_machine_debug_execution_plan_kind {
    COMMON_MACHINE_DEBUG_EXECUTION_NONE,
    COMMON_MACHINE_DEBUG_EXECUTION_TRACE,
    COMMON_MACHINE_DEBUG_EXECUTION_BREAK_REAL,
    COMMON_MACHINE_DEBUG_EXECUTION_BREAK_LINEAR
} common_machine_debug_execution_plan_kind;

typedef struct common_machine_debug_segment_snapshot {
    lib_u16 selector;
    lib_u32 base;
    lib_u32 limit;
    lib_u8 dpl;
    lib_u8 type;
    lib_bool accessed;
    lib_bool executable;
    lib_bool conform;
    lib_bool readable;
    lib_bool defsize;
    lib_bool big;
    lib_bool expdown;
    lib_bool writable;
} common_machine_debug_segment_snapshot;

typedef struct common_machine_debug_cpu_snapshot {
    common_machine_debug_segment_snapshot es;
    common_machine_debug_segment_snapshot cs;
    common_machine_debug_segment_snapshot ss;
    common_machine_debug_segment_snapshot ds;
    common_machine_debug_segment_snapshot fs;
    common_machine_debug_segment_snapshot gs;
    common_machine_debug_segment_snapshot tr;
    common_machine_debug_segment_snapshot ldtr;
    common_machine_debug_segment_snapshot gdtr;
    common_machine_debug_segment_snapshot idtr;
    lib_u32 cr0;
    lib_u32 cr2;
    lib_u32 cr3;
} common_machine_debug_cpu_snapshot;

typedef struct common_machine_debug_request {
    common_machine_debug_operation operation;
    lib_u32 register_id;
    lib_u32 address;
    lib_u16 segment;
    lib_u16 offset;
    lib_u16 port;
    common_machine_debug_watch_kind watch_kind;
    common_machine_debug_execution_plan_kind execution_kind;
    lib_u64 instruction_count;
    lib_u8 bytes;
    lib_u8 data[COMMON_MACHINE_DEBUG_BYTES];
} common_machine_debug_request;

typedef struct common_machine_debug_result {
    lib_u32 value;
    lib_bool enabled;
    lib_u8 bytes;
    lib_u8 data[COMMON_MACHINE_DEBUG_BYTES];
    common_machine_debug_cpu_snapshot cpu;
} common_machine_debug_result;

typedef struct common_machine_debug_lease {
    lib_u64 generation;
} common_machine_debug_lease;

typedef void (*common_machine_request_consumer)(void *context,
    const common_machine_request *request);
typedef lib_bool (*common_machine_is_paused)(void *context);
typedef lib_status (*common_machine_debug_execute)(void *context,
    const common_machine_debug_request *request,
    common_machine_debug_result *out_result);

typedef struct common_machine_driver {
    common_machine_request_consumer consume_request;
    common_machine_is_paused is_paused;
    common_machine_debug_execute execute_debug;
    void *context;
} common_machine_driver;

lib_status common_machine_create(common_machine **out_machine);
void common_machine_destroy(common_machine *machine);
void common_machine_close(common_machine *machine);
lib_status common_machine_bind_driver(common_machine *machine,
    const common_machine_driver *driver);
lib_status common_machine_bind_run(common_machine *machine, lib_u32 run_id);
lib_status common_machine_submit(common_machine *machine,
    const common_machine_request *request);
lib_status common_machine_observe_safe_point(common_machine *machine);
lib_status common_machine_wait(common_machine *machine,
    lib_u32 timeout_milliseconds);
lib_status common_machine_debug_acquire(common_machine *machine,
    common_machine_debug_lease *out_lease);
lib_status common_machine_debug_execute_with_lease(common_machine *machine,
    const common_machine_debug_lease *lease,
    const common_machine_debug_request *request,
    common_machine_debug_result *out_result);
void common_machine_debug_invalidate(common_machine *machine);

#endif
