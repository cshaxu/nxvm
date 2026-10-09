#ifndef EMULATOR_SESSION_CONTROL_H
#define EMULATOR_SESSION_CONTROL_H

#include "lib/base/sync_interface.h"
#include "lib/console/console_interface.h"
#include "lib/kvm-base/event_interface.h"
#include "emulator/session/session_interface.h"

typedef enum emulator_session_event_kind {
    EMULATOR_SESSION_EVENT_KVM_INPUT,
    EMULATOR_SESSION_EVENT_MONITOR_LINE,
    /* Completion records are intentionally distinct from user input.  The
     * sole control consumer feeds them to the reducer as actual facts. */
    EMULATOR_SESSION_EVENT_RUNTIME_COMPLETED,
    EMULATOR_SESSION_EVENT_FRAME_COMPLETED,
    EMULATOR_SESSION_EVENT_COMPONENT_COMPLETED,
    EMULATOR_SESSION_EVENT_BROKER_COMPLETED,
    EMULATOR_SESSION_EVENT_KVM_DELIVERY_FAILED,
    /* The queue could not retain a control fact.  Continuing would leave the
       reducer waiting for an event which was silently lost. */
    EMULATOR_SESSION_EVENT_QUEUE_DELIVERY_FAILED,
    EMULATOR_SESSION_EVENT_CONSOLE_FAILED
} emulator_session_event_kind;

typedef enum emulator_session_component_kind {
    EMULATOR_SESSION_EVENT_COMPONENT_WINDOW,
    EMULATOR_SESSION_EVENT_COMPONENT_VM_CONSOLE
} emulator_session_component_kind;

typedef struct emulator_session_event {
    emulator_session_event_kind kind;
    /* Zero is monitor/local input.  KVM producers stamp the currently active
     * machine run so a queued old input cannot affect a later start. */
    lib_u32 run_generation;
    lib_bool monitor_line_rejected;
    union {
        kvm_input_event kvm;
        lib_console_line line;
        emulator_session_machine_state runtime_state;
        struct { lib_u32 sequence; lib_bool graphics; } frame;
        struct { emulator_session_component_kind component; lib_bool exists; } component;
        lib_bool broker_vm_console_current;
        struct { lib_u64 source_identity; lib_status status; } delivery_failure;
        lib_status queue_delivery_status;
    } value;
} emulator_session_event;

#define EMULATOR_SESSION_EVENT_PRESSED_CAPACITY 256u

typedef struct emulator_session_queue {
    base_sync_mutex *lock;
    base_sync_event *available;
    emulator_session_event *events;
    lib_size first;
    lib_size count;
    lib_size capacity;
    kvm_input_event pressed[EMULATOR_SESSION_EVENT_PRESSED_CAPACITY];
    lib_size pressed_count;
    emulator_session_event faults[2];
    lib_bool fault_pending[2];
} emulator_session_queue;

/* Caller owns zeroed or disposed storage; failure releases partial resources. */
lib_bool emulator_session_queue_initialize(emulator_session_queue *queue);
void emulator_session_queue_dispose(emulator_session_queue *queue);
lib_bool emulator_session_queue_push_kvm_for_run(emulator_session_queue *queue,
    const kvm_input_event *event, lib_u32 run_generation);
lib_bool emulator_session_queue_push_monitor_line(emulator_session_queue *queue,
    const lib_console_line *line, lib_bool rejected);
lib_bool emulator_session_queue_push_console_failed(emulator_session_queue *queue);
lib_bool emulator_session_queue_push_runtime_completed(emulator_session_queue *queue,
    emulator_session_machine_state state, lib_u32 run_generation);
lib_bool emulator_session_queue_push_frame_completed(emulator_session_queue *queue,
    lib_u32 sequence, lib_bool graphics, lib_u32 run_generation);
lib_bool emulator_session_queue_push_component_completed(emulator_session_queue *queue,
    emulator_session_component_kind component, lib_bool exists, lib_u32 run_generation);
lib_bool emulator_session_queue_push_broker_completed(emulator_session_queue *queue,
    lib_bool vm_console_current, lib_u32 run_generation);
lib_bool emulator_session_queue_push_kvm_delivery_failed(emulator_session_queue *queue,
    lib_u64 source_identity, lib_status status, lib_u32 run_generation);
lib_bool emulator_session_queue_take(emulator_session_queue *queue,
    emulator_session_event *out_event, lib_u32 timeout_ms);
/* A KVM producer belongs to one VM run. Paused admits cleanup/lifecycle and
 * registered-hotkey records for product handling, but ordinary guest input
 * remains rejected. Monitor lines and completion facts use separate rules. */
lib_bool emulator_session_accept_kvm_event(const emulator_session_event *event,
    lib_u32 current_run_generation, emulator_session_machine_state runtime_state);
typedef lib_bool (*emulator_session_input_sink)(void *context,
    const kvm_input_event *event);
lib_bool emulator_session_dispatch_input(emulator_session_queue *queue, const kvm_input_event *event,
    emulator_session_machine_state runtime_state, emulator_session_input_sink sink,
    void *sink_context);

#endif
