#ifndef COMMON_MACHINE_INTERFACE_H
#define COMMON_MACHINE_INTERFACE_H

#include "lib/kvm-base/event_interface.h"
#include "lib/kvm-base/frame_interface.h"
#include "lib/types/types_interface.h"

#define COMMON_MACHINE_PATH_CAPACITY 1024u
#define COMMON_MACHINE_DEBUG_BYTES 32u

typedef struct common_machine common_machine;
typedef void (*common_machine_executor_callback)(void *context);

typedef enum common_machine_state {
    COMMON_MACHINE_STOPPED,
    COMMON_MACHINE_STARTING,
    COMMON_MACHINE_RUNNING,
    COMMON_MACHINE_PAUSED,
    COMMON_MACHINE_ERROR,
    COMMON_MACHINE_RESET_COMPLETED
} common_machine_state;

/* Debug is synchronous to the sole control-thread caller; execution occurs
 * on the existing paused executor. Lifecycle, media and debug requests must
 * be serialized by that caller, never issued from a driver/sink callback.
 * Common owns lease validity and rendezvous; the product owns CPU access. */
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
    lib_u8 bytes; /* Memory payload size; port I/O explicitly requires 1 (byte). */
    lib_u8 data[COMMON_MACHINE_DEBUG_BYTES];
} common_machine_debug_request;

#define COMMON_MACHINE_DEBUG_ACCESS_CAPACITY 32u
typedef struct common_machine_debug_memory_access {
    lib_bool write;
    lib_u32 linear;
    lib_u32 bytes;
    lib_u64 data; /* Lowest-addressed up to eight bytes, little endian. */
} common_machine_debug_memory_access;

typedef struct common_machine_debug_observation {
    common_machine_debug_memory_access accesses[COMMON_MACHINE_DEBUG_ACCESS_CAPACITY];
    lib_u8 count;
    lib_bool truncated;
    lib_bool watch_hit;
    common_machine_debug_watch_kind watch_kind;
    lib_u32 watch_address;
} common_machine_debug_observation;

typedef struct common_machine_debug_result {
    lib_u32 value;
    lib_bool enabled;
    lib_u8 bytes;
    lib_u8 data[COMMON_MACHINE_DEBUG_BYTES];
    common_machine_debug_cpu_snapshot cpu;
    common_machine_debug_observation observation;
} common_machine_debug_result;

typedef struct common_machine_debug_lease {
    lib_u64 generation;
} common_machine_debug_lease;

typedef lib_status (*common_machine_debug_execute)(void *context,
    const common_machine_debug_request *request,
    common_machine_debug_result *out_result);

/* State transitions and completed frame publication are independent facts.
 * Keeping their callbacks separate prevents an executor paint callback from
 * being mistaken for a lifecycle completion by the product control queue. */
typedef void (*common_machine_state_sink)(void *context,
    common_machine_state state, lib_u32 run_generation);
typedef void (*common_machine_frame_sink)(void *context,
    lib_u32 frame_sequence, lib_bool graphics, lib_u32 run_generation);

typedef struct common_machine_driver {
    void *context;
    lib_bool (*reset)(void *context);
    lib_bool (*run)(void *context);
    void (*request_stop)(void *context);
    void (*request_wake)(void *context);
    void (*set_heartbeat)(void *context, lib_bool enabled);
    void (*set_executor_callback)(void *context,
        common_machine_executor_callback callback, void *callback_context);
    void (*deliver_input)(void *context, const kvm_input_event *event);
    lib_bool (*copy_frame)(void *context, kvm_frame *out_frame);
    lib_bool (*set_removable_media)(void *context, const char *path);
    common_machine_debug_execute execute_debug;
    /* Executor-only stop notification and cancellation of product debug plans. */
    lib_bool (*take_debug_stop)(void *context);
    void (*cancel_debug)(void *context);
    /* Optional product-owned observation after a complete frame has been
     * published. It must not call machine lifecycle APIs. */
    void (*frame_published)(void *context, const kvm_frame *frame);
} common_machine_driver;

lib_status common_machine_create(common_machine **out_machine,
    const common_machine_driver *driver);
void common_machine_set_state_sink(common_machine *machine,
    common_machine_state_sink sink, void *context);
void common_machine_set_frame_sink(common_machine *machine,
    common_machine_frame_sink sink, void *context);
lib_bool common_machine_start(common_machine *machine);
lib_bool common_machine_pause(common_machine *machine);
lib_bool common_machine_resume(common_machine *machine);
lib_bool common_machine_stop(common_machine *machine);
lib_bool common_machine_reset(common_machine *machine);
lib_bool common_machine_set_removable_media(common_machine *machine,
    const char *path);
common_machine_state common_machine_state_get(const common_machine *machine);
lib_bool common_machine_enqueue_input(common_machine *machine,
    const kvm_input_event *event);
lib_bool common_machine_copy_published_frame(common_machine *machine,
    kvm_frame *destination, lib_u32 *out_run_generation);
lib_u32 common_machine_published_frame_sequence(const common_machine *machine);
lib_u32 common_machine_published_frame_run_generation(const common_machine *machine);
lib_u32 common_machine_run_generation(const common_machine *machine);
lib_status common_machine_debug_acquire(common_machine *machine,
    common_machine_debug_lease *out_lease);
lib_status common_machine_debug_execute_with_lease(common_machine *machine,
    const common_machine_debug_lease *lease,
    const common_machine_debug_request *request,
    common_machine_debug_result *out_result);
void common_machine_debug_invalidate(common_machine *machine);
/* Asynchronous cancellation, allowed in every state; uses the existing queue. */
void common_machine_debug_cancel(common_machine *machine);
void common_machine_destroy(common_machine *machine);

#endif
