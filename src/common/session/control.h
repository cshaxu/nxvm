#ifndef COMMON_SESSION_CONTROL_H
#define COMMON_SESSION_CONTROL_H

#include "lib/console/console_interface.h"
#include "lib/kvm-base/event_interface.h"
#include "common/session/session_interface.h"

typedef struct common_session_queue common_session_queue;

typedef enum common_session_event_kind {
    COMMON_SESSION_EVENT_KVM_INPUT,
    COMMON_SESSION_EVENT_MONITOR_LINE,
    /* Completion records are intentionally distinct from user input.  The
     * sole control consumer feeds them to the reducer as actual facts. */
    COMMON_SESSION_EVENT_RUNTIME_COMPLETED,
    COMMON_SESSION_EVENT_FRAME_COMPLETED,
    COMMON_SESSION_EVENT_COMPONENT_COMPLETED,
    COMMON_SESSION_EVENT_BROKER_COMPLETED,
    COMMON_SESSION_EVENT_KVM_DELIVERY_FAILED,
    /* The queue could not retain a control fact.  Continuing would leave the
       reducer waiting for an event which was silently lost. */
    COMMON_SESSION_EVENT_QUEUE_DELIVERY_FAILED,
    COMMON_SESSION_EVENT_CONSOLE_FAILED
} common_session_event_kind;

typedef enum common_session_component_kind {
    COMMON_SESSION_EVENT_COMPONENT_WINDOW,
    COMMON_SESSION_EVENT_COMPONENT_VM_CONSOLE
} common_session_component_kind;

typedef struct common_session_event {
    common_session_event_kind kind;
    /* Zero is monitor/local input.  KVM producers stamp the currently active
     * machine run so a queued old input cannot affect a later start. */
    lib_u32 run_generation;
    int monitor_line_rejected;
    union {
        kvm_input_event kvm;
        lib_console_line line;
        common_session_machine_state runtime_state;
        struct { lib_u32 sequence; int graphics; } frame;
        struct { common_session_component_kind component; int exists; } component;
        int broker_vm_console_current;
        struct { lib_u64 source_identity; lib_status status; } delivery_failure;
        lib_status queue_delivery_status;
    } value;
} common_session_event;

int common_session_queue_create(common_session_queue **out_queue);
void common_session_queue_destroy(common_session_queue *queue);
int common_session_queue_push_ux(common_session_queue *queue,
    const kvm_input_event *event);
int common_session_queue_push_kvm_for_run(common_session_queue *queue,
    const kvm_input_event *event, lib_u32 run_generation);
int common_session_queue_push_monitor_line(common_session_queue *queue,
    const lib_console_line *line, int rejected);
int common_session_queue_push_console_failed(common_session_queue *queue);
int common_session_queue_push_runtime_completed(common_session_queue *queue,
    common_session_machine_state state, lib_u32 run_generation);
int common_session_queue_push_frame_completed(common_session_queue *queue,
    lib_u32 sequence, int graphics, lib_u32 run_generation);
int common_session_queue_push_component_completed(common_session_queue *queue,
    common_session_component_kind component, int exists, lib_u32 run_generation);
int common_session_queue_push_broker_completed(common_session_queue *queue,
    int vm_console_current, lib_u32 run_generation);
int common_session_queue_push_kvm_delivery_failed(common_session_queue *queue,
    lib_u64 source_identity, lib_status status, lib_u32 run_generation);
int common_session_queue_take(common_session_queue *queue,
    common_session_event *out_event, lib_u32 timeout_ms);
/* A KVM producer belongs to one VM run. Paused admits cleanup/lifecycle and
 * registered-hotkey records for product handling, but ordinary guest input
 * remains rejected. Monitor lines and completion facts use separate rules. */
int common_session_accept_kvm_event(const common_session_event *event,
    lib_u32 current_run_generation, common_session_machine_state runtime_state);
typedef int (*common_session_input_sink)(void *context,
    const kvm_input_event *event);
int common_session_dispatch_input(common_session_queue *queue, const kvm_input_event *event,
    common_session_machine_state runtime_state, common_session_input_sink sink,
    void *sink_context);

#endif
