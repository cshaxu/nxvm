#ifndef COMMON_MACHINE_INTERFACE_H
#define COMMON_MACHINE_INTERFACE_H

#include "lib/kvm-base/event_interface.h"
#include "common/machine/frame_interface.h"
#include "lib/storage/medium_interface.h"
#include "lib/types/types_interface.h"

#define COMMON_MACHINE_PATH_CAPACITY 1024u
#define COMMON_MACHINE_DEBUG_REQUEST_CAPACITY 128u
#define COMMON_MACHINE_DEBUG_RESPONSE_CAPACITY 1536u

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
 * on the existing paused executor. Lifecycle, media, debug and state read/write
 * requests must be serialized by that caller, never issued from a driver/sink
 * callback. Internal admission locking does not serialize caller payloads.
 * Common owns lease validity and rendezvous; the product owns CPU access. */

typedef struct common_machine_debug_lease {
    lib_u64 generation;
} common_machine_debug_lease;

typedef lib_status (*common_machine_debug_execute)(void *context,
    const void *request, lib_size request_size,
    void *response, lib_size response_capacity, lib_size *response_size);

typedef lib_status (*common_machine_state_write_callback)(void *context,
    const lib_u8 *bytes, lib_size byte_count);
typedef lib_status (*common_machine_state_read_callback)(void *context,
    lib_u8 *bytes, lib_size byte_count);

typedef struct common_machine_state_writer {
    common_machine_state_write_callback write;
    void *context;
} common_machine_state_writer;

typedef struct common_machine_state_reader {
    common_machine_state_read_callback read;
    void *context;
} common_machine_state_reader;

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
    /* Return a complete frame on display change (including palette/geometry).
     * First ready output must be published. OK with window.valid == 0 means
     * no new ready frame. Publications may be skipped by latest-wins consumers;
     * no incremental damage contract crosses this boundary. Failure does not
     * publish staging and terminates through the existing ERROR fact. */
    lib_status (*copy_frame)(void *context, common_machine_frame *out_frame);
    lib_bool (*set_removable_media)(void *context, const char *path,
        lib_storage_medium_mode mode);
    /* Common invokes only these state-specific driver hooks on its existing
     * executor. The driver owns safe-boundary detection and image semantics. */
    lib_status (*begin_state_read)(void *context,
        const common_machine_state_writer *writer);
    lib_bool (*take_state_read_result)(void *context,
        lib_status *out_status);
    lib_status (*write_state)(void *context,
        const common_machine_state_reader *reader);
    common_machine_debug_execute execute_debug;
    /* Executor-only stop notification and cancellation of product debug plans. */
    lib_bool (*take_debug_stop)(void *context);
    void (*cancel_debug)(void *context);
    /* Optional product-owned observation after a complete frame has been
     * published. It must not call machine lifecycle APIs. */
    void (*frame_published)(void *context, const common_machine_frame *frame);
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
    const char *path, lib_storage_medium_mode mode);
/* Synchronous state I/O uses the same sole control caller as other requests.
 * Writer/reader descriptors are copied; their contexts remain caller-owned
 * and valid through completion. On failure, retain contexts until successful
 * shutdown unless completion is independently established: a failed native
 * wait alone does not establish executor quiescence.
 * Callbacks run on the existing executor and must not reenter request APIs.
 * This restriction does not apply to input enqueue or published-frame reads. */
lib_status common_machine_read_state(common_machine *machine,
    const common_machine_state_writer *writer);
lib_status common_machine_write_state(common_machine *machine,
    const common_machine_state_reader *reader);
common_machine_state common_machine_state_get(const common_machine *machine);
lib_bool common_machine_enqueue_input(common_machine *machine,
    const kvm_input_event *event);
/* Copy one complete frame only from the requested run. On rejection the
 * destination is unchanged; content, sequence and route are one snapshot. */
lib_bool common_machine_copy_published_frame(common_machine *machine,
    common_machine_frame *destination, lib_u32 run_generation);
lib_u32 common_machine_published_frame_sequence(const common_machine *machine);
lib_u32 common_machine_published_frame_run_generation(const common_machine *machine);
lib_u32 common_machine_run_generation(const common_machine *machine);
lib_status common_machine_debug_acquire(common_machine *machine,
    common_machine_debug_lease *out_lease);
/* Opaque, pointer-free in-process values; the driver owns their protocol.
 * Request bytes are copied before dispatch. NULL is valid only for zero size
 * or capacity; response_size is required and is zero on failure. Output bytes
 * are unchanged on failure. No caller buffer is retained by the executor.
 * The driver receives at most the caller's capacity and must report the bytes
 * it actually initialized, never write beyond capacity or retain these buffers.
 * After a failed completion wait, shut down successfully before further calls:
 * wait failure does not prove that the internal request slot is no longer used. */
lib_status common_machine_debug_execute_with_lease(common_machine *machine,
    const common_machine_debug_lease *lease,
    const void *request, lib_size request_size,
    void *response, lib_size response_capacity, lib_size *response_size);
/* Asynchronous cancellation, allowed in every state; uses the existing queue. */
void common_machine_debug_cancel(common_machine *machine);
/* Permanently stop/join the worker, including all in-flight callbacks, but
 * retain the object. NULL/repeated calls are harmless. The owner serializes
 * this with other API calls; never call from a worker callback. Callback
 * contexts and the driver must remain alive until this succeeds. Failure
 * retains the machine and worker; destroy also retains them on failure. It uses
 * this same path; product stop remains a separate restartable operation. */
lib_status common_machine_shutdown(common_machine *machine);
lib_status common_machine_destroy(common_machine *machine);

#endif
