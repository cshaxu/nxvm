#include "common/session/control.h"

#define COMMON_SESSION_EVENT_QUEUE_INITIAL_CAPACITY 64u

static int common_session_queue_push(common_session_queue *queue,
    const common_session_event *event);

/* Fixed storage survives allocation failure. Each class keeps its first fault;
 * KVM delivery (slot 0) precedes control delivery (slot 1) after the FIFO drains. */
static void common_session_queue_latch_failure(common_session_queue *queue,
    unsigned int slot, const common_session_event *event)
{
    if (queue == NULL) return;
    base_sync_mutex_lock(queue->lock);
    if (!queue->fault_pending[slot]) {
        queue->faults[slot] = *event;
        queue->fault_pending[slot] = 1;
    }
    base_sync_event_signal(queue->available);
    base_sync_mutex_unlock(queue->lock);
}

static int common_session_queue_push_required(common_session_queue *queue,
    const common_session_event *event)
{
    common_session_event fault = { 0 };
    if (common_session_queue_push(queue, event)) return 1;
    fault.kind = COMMON_SESSION_EVENT_QUEUE_DELIVERY_FAILED;
    fault.run_generation = event == NULL ? 0u : event->run_generation;
    fault.value.queue_delivery_status = LIB_STATUS_NO_MEMORY;
    common_session_queue_latch_failure(queue, 1u, &fault);
    return 0;
}

static int common_session_queue_push(common_session_queue *queue,
    const common_session_event *event)
{
    common_session_event *expanded;
    unsigned int index;
    if (queue == NULL || event == NULL) return 0;
    base_sync_mutex_lock(queue->lock);
    if (queue->count == queue->capacity) {
        unsigned int next_capacity = queue->capacity * 2u;
        if (next_capacity <= queue->capacity ||
            next_capacity > LIB_UINT_MAX / sizeof(*expanded)) {
            base_sync_mutex_unlock(queue->lock);
            return 0;
        }
        expanded = lib_allocate_zero(next_capacity, sizeof(*expanded));
        if (expanded == NULL) {
            base_sync_mutex_unlock(queue->lock);
            return 0;
        }
        for (index = 0u; index < queue->count; ++index)
            expanded[index] = queue->events[(queue->first + index) % queue->capacity];
        lib_release(queue->events);
        queue->events = expanded;
        queue->first = 0u;
        queue->capacity = next_capacity;
    }
    queue->events[(queue->first + queue->count) % queue->capacity] =
        *event;
    ++queue->count;
    base_sync_event_signal(queue->available);
    base_sync_mutex_unlock(queue->lock);
    return 1;
}

int common_session_queue_initialize(common_session_queue *queue)
{
    if (queue == NULL) return 0;
    *queue = (common_session_queue) { 0 };
    if (base_sync_mutex_create(&queue->lock) != LIB_STATUS_OK) return 0;
    queue->capacity = COMMON_SESSION_EVENT_QUEUE_INITIAL_CAPACITY;
    queue->events = lib_allocate_zero(queue->capacity, sizeof(*queue->events));
    if (queue->events == NULL ||
        base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &queue->available) != LIB_STATUS_OK) {
        common_session_queue_dispose(queue);
        return 0;
    }
    return 1;
}

void common_session_queue_dispose(common_session_queue *queue)
{
    if (queue == NULL) return;
    base_sync_event_destroy(queue->available);
    lib_release(queue->events);
    base_sync_mutex_destroy(queue->lock);
    *queue = (common_session_queue) { 0 };
}

int common_session_queue_push_kvm_for_run(common_session_queue *queue,
    const kvm_input_event *event, lib_u32 run_generation)
{
    common_session_event copied = { 0 };
    if (event == NULL) return 0;
    copied.kind = COMMON_SESSION_EVENT_KVM_INPUT;
    copied.run_generation = run_generation;
    copied.value.kvm = *event;
    if (common_session_queue_push(queue, &copied)) return 1;
    copied.kind = COMMON_SESSION_EVENT_KVM_DELIVERY_FAILED;
    copied.value.delivery_failure.source_identity = event->source_identity;
    copied.value.delivery_failure.status = LIB_STATUS_NO_MEMORY;
    common_session_queue_latch_failure(queue, 0u, &copied);
    return 0;
}

int common_session_queue_push_monitor_line(common_session_queue *queue,
    const lib_console_line *line, int rejected)
{
    common_session_event copied = { 0 };
    if (line == NULL) return 0;
    copied.kind = COMMON_SESSION_EVENT_MONITOR_LINE;
    copied.value.line = *line;
    copied.monitor_line_rejected = rejected;
    return common_session_queue_push_required(queue, &copied);
}

int common_session_queue_push_console_failed(common_session_queue *queue)
{
    common_session_event event = { 0 };
    event.kind = COMMON_SESSION_EVENT_CONSOLE_FAILED;
    return common_session_queue_push_required(queue, &event);
}

int common_session_queue_push_runtime_completed(common_session_queue *queue,
    common_session_machine_state state, lib_u32 run_generation)
{
    common_session_event event = { .kind = COMMON_SESSION_EVENT_RUNTIME_COMPLETED, .run_generation = run_generation };
    event.value.runtime_state = state;
    return common_session_queue_push_required(queue, &event);
}

int common_session_queue_push_frame_completed(common_session_queue *queue,
    lib_u32 sequence, int graphics, lib_u32 run_generation)
{
    common_session_event event = { .kind = COMMON_SESSION_EVENT_FRAME_COMPLETED, .run_generation = run_generation };
    event.value.frame.sequence = sequence;
    event.value.frame.graphics = graphics != 0;
    return common_session_queue_push_required(queue, &event);
}

int common_session_queue_push_component_completed(common_session_queue *queue,
    common_session_component_kind component, int exists, lib_u32 run_generation)
{
    common_session_event event = { .kind = COMMON_SESSION_EVENT_COMPONENT_COMPLETED, .run_generation = run_generation };
    event.value.component.component = component;
    event.value.component.exists = exists != 0;
    return common_session_queue_push_required(queue, &event);
}

int common_session_queue_push_broker_completed(common_session_queue *queue,
    int vm_console_current, lib_u32 run_generation)
{
    common_session_event event = { .kind = COMMON_SESSION_EVENT_BROKER_COMPLETED, .run_generation = run_generation };
    event.value.broker_vm_console_current = vm_console_current != 0;
    return common_session_queue_push_required(queue, &event);
}

int common_session_queue_push_kvm_delivery_failed(common_session_queue *queue,
    lib_u64 source_identity, lib_status status, lib_u32 run_generation)
{
    common_session_event event = { .kind = COMMON_SESSION_EVENT_KVM_DELIVERY_FAILED, .run_generation = run_generation };
    event.value.delivery_failure.source_identity = source_identity;
    event.value.delivery_failure.status = status;
    return common_session_queue_push_required(queue, &event);
}

int common_session_queue_take(common_session_queue *queue,
    common_session_event *out_event, lib_u32 timeout_ms)
{
    int taken = 0;
    unsigned int slot;
    if (queue == NULL || out_event == NULL ||
        base_sync_event_wait(queue->available, timeout_ms) != BASE_SYNC_WAIT_SIGNALED)
        return 0;
    base_sync_mutex_lock(queue->lock);
    if (queue->count != 0u) {
        *out_event = queue->events[queue->first];
        queue->first = (queue->first + 1u) % queue->capacity;
        --queue->count;
        taken = 1;
    } else {
        for (slot = 0u; slot < 2u; ++slot) {
            if (!queue->fault_pending[slot]) continue;
            *out_event = queue->faults[slot];
            queue->fault_pending[slot] = 0;
            taken = 1;
            break;
        }
    }
    if (queue->count == 0u && !queue->fault_pending[0] && !queue->fault_pending[1])
        base_sync_event_reset(queue->available);
    base_sync_mutex_unlock(queue->lock);
    return taken;
}

int common_session_accept_kvm_event(const common_session_event *event,
    lib_u32 current_run_generation, common_session_machine_state runtime_state)
{
    const kvm_input_event *input;

    if (event == NULL || event->kind != COMMON_SESSION_EVENT_KVM_INPUT) return 0;
    input = &event->value.kvm;
    /* Source identity is globally monotonic.  Retirement is not guest input:
       it must always reach the ledger, even when the component belonged to a
       retired run, so a later allocation cannot inherit its held keys. */
    if (input->type == KVM_EVENT_SOURCE_RETIRED) return 1;
    if (event->run_generation != 0u && event->run_generation !=
        current_run_generation) return 0;
    if (runtime_state == COMMON_SESSION_MACHINE_RUNNING) return 1;
    if (runtime_state != COMMON_SESSION_MACHINE_PAUSED) return 0;
    return input->type == KVM_EVENT_WINDOW_CLOSE ||
        input->type == KVM_EVENT_SOURCE_RETIRED ||
        input->type == KVM_EVENT_HOTKEY ||
        (input->type == KVM_EVENT_KEY && input->data.key.pressed == 0u) ||
        (input->type == KVM_EVENT_MOUSE && input->data.mouse.buttons == 0u);
}

static void common_session_forget_pressed(common_session_queue *queue,
    const kvm_input_event *event)
{
    unsigned int index;
    for (index = 0u; index < queue->pressed_count; ++index) {
        kvm_input_event *pressed = &queue->pressed[index];
        if (pressed->source_identity == event->source_identity &&
            (pressed->data.key.flags & KVM_KEY_FLAG_EXTENDED) ==
                (event->data.key.flags & KVM_KEY_FLAG_EXTENDED) &&
            ((pressed->data.key.scan_code != 0u || event->data.key.scan_code != 0u)
                ? pressed->data.key.scan_code == event->data.key.scan_code
                : pressed->data.key.key == event->data.key.key)) {
            pressed[0] = queue->pressed[--queue->pressed_count];
            return;
        }
    }
}

static void common_session_remember_pressed(common_session_queue *queue,
    const kvm_input_event *event)
{
    common_session_forget_pressed(queue, event);
    if (queue->pressed_count == COMMON_SESSION_EVENT_PRESSED_CAPACITY) return;
    queue->pressed[queue->pressed_count++] = *event;
}

static int common_session_release_source(common_session_queue *queue,
    lib_u64 source, common_session_machine_state runtime_state,
    common_session_input_sink sink, void *sink_context)
{
    unsigned int index = 0u;
    while (index < queue->pressed_count) {
        kvm_input_event *pressed = &queue->pressed[index];
        if (pressed->source_identity != source) {
            ++index;
            continue;
        }
        pressed->data.key.pressed = 0u;
        /* Source retirement is ledger cleanup.  It can reach the control
           queue after pause, but must not turn into a late guest release in
           the paused or stopped VM. */
        if (runtime_state == COMMON_SESSION_MACHINE_RUNNING &&
            !sink(sink_context, pressed)) return 0;
        queue->pressed[index] = queue->pressed[--queue->pressed_count];
    }
    return 1;
}

int common_session_dispatch_input(common_session_queue *queue, const kvm_input_event *event,
    common_session_machine_state runtime_state, common_session_input_sink sink,
    void *sink_context)
{
    if (queue == NULL || event == NULL || sink == NULL) return 0;
    if (event->type == KVM_EVENT_KEY) {
        if (event->data.key.pressed != 0u) common_session_remember_pressed(queue, event);
        else common_session_forget_pressed(queue, event);
        return runtime_state != COMMON_SESSION_MACHINE_RUNNING ||
            sink(sink_context, event);
    }
    if (event->type == KVM_EVENT_MOUSE || event->type == KVM_EVENT_TEXT)
        return runtime_state != COMMON_SESSION_MACHINE_RUNNING ||
            sink(sink_context, event);
    if (event->type == KVM_EVENT_SOURCE_RETIRED)
        return common_session_release_source(queue, event->source_identity,
            runtime_state, sink, sink_context);
    if (event->type == KVM_EVENT_WINDOW_CLOSE)
        return 1;
    if (event->type != KVM_EVENT_HOTKEY) return 1;
    /* Product adapters consume registered hotkeys before this generic input
       ledger.  No identifier or guest protocol belongs in common/session. */
    return 1;
}
