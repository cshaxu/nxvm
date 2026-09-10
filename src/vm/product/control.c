#include "type.h"

#include "lib/host/sync_interface.h"
#include "vm/product/control.h"

#define VM_PRODUCT_CONTROL_FACT_CAPACITY 64u
#define VM_PRODUCT_CONTROL_PRESSED_CAPACITY 256u

typedef struct vm_product_control_pressed_key {
    lib_u64 source_identity;
    ux_input_event event;
} vm_product_control_pressed_key;

struct vm_product_control {
    host_sync_event *ready;
    STD_ATOMIC_FLAG lock;
    vm_product_control_fact facts[VM_PRODUCT_CONTROL_FACT_CAPACITY];
    STD_SIZE_T first;
    STD_SIZE_T count;
    C_INT accepting;
    C_INT delivery_failed;
    core_machine_guest_display_frame latest_display;
    C_INT display_ready;
    C_UINT latest_display_generation;
    C_UINT run_generation;
    vm_session_lifecycle lifecycle;
    vm_product_control_pressed_key pressed[VM_PRODUCT_CONTROL_PRESSED_CAPACITY];
    STD_SIZE_T pressed_count;
};

static C_VOID vm_product_control_lock(vm_product_control *control)
{
    while (STD_ATOMIC_FLAG_TEST_AND_SET_EXPLICIT(&control->lock,
        STD_MEMORY_ORDER_ACQUIRE)) { }
}

static C_VOID vm_product_control_unlock(vm_product_control *control)
{
    STD_ATOMIC_FLAG_CLEAR_EXPLICIT(&control->lock, STD_MEMORY_ORDER_RELEASE);
}

type_status vm_product_control_create(vm_product_control **out_control)
{
    vm_product_control *control;

    if (out_control == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_control = STD_NULL;
    control = STD_CALLOC(1u, sizeof(*control));
    if (control == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    control->lock = (STD_ATOMIC_FLAG)ATOMIC_FLAG_INIT;
    STD_ATOMIC_FLAG_CLEAR_EXPLICIT(&control->lock, STD_MEMORY_ORDER_RELEASE);
    if (host_sync_event_create(&control->ready) != LIB_STATUS_OK) {
        STD_FREE(control);
        return TYPE_STATUS_NO_MEMORY;
    }
    control->accepting = TYPE_TRUE;
    control->lifecycle = VM_SESSION_STOPPED;
    *out_control = control;
    return TYPE_STATUS_OK;
}

C_VOID vm_product_control_destroy(vm_product_control *control)
{
    if (control == STD_NULL) return;
    host_sync_event_destroy(control->ready);
    STD_FREE(control);
}

type_status vm_product_control_publish(vm_product_control *control,
    const vm_product_control_fact *fact)
{
    STD_SIZE_T index;

    if (control == STD_NULL || fact == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    vm_product_control_lock(control);
    if (!control->accepting) {
        vm_product_control_unlock(control);
        return TYPE_STATUS_INVALID_STATE;
    }
    if (control->count == VM_PRODUCT_CONTROL_FACT_CAPACITY) {
        control->delivery_failed = TYPE_TRUE;
        host_sync_event_signal(control->ready);
        vm_product_control_unlock(control);
        return TYPE_STATUS_NO_MEMORY;
    }
    index = (control->first + control->count) % VM_PRODUCT_CONTROL_FACT_CAPACITY;
    control->facts[index] = *fact;
    if (fact->kind == VM_PRODUCT_CONTROL_FACT_LIFECYCLE ||
        fact->kind == VM_PRODUCT_CONTROL_FACT_HOST_INPUT) {
        control->facts[index].run_generation = control->run_generation;
    }
    ++control->count;
    host_sync_event_signal(control->ready);
    vm_product_control_unlock(control);
    return TYPE_STATUS_OK;
}

/* Frames are a latest-state mailbox, deliberately separate from the bounded
 * FIFO of commands and lifecycle facts.  A producer can never starve a pause
 * or input request by publishing a faster display cadence. */
type_status vm_product_control_publish_display(vm_product_control *control,
    const core_machine_guest_display_frame *frame)
{
    if (control == STD_NULL || frame == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    vm_product_control_lock(control);
    if (!control->accepting) {
        vm_product_control_unlock(control);
        return TYPE_STATUS_INVALID_STATE;
    }
    control->latest_display = *frame;
    control->display_ready = TYPE_TRUE;
    control->latest_display_generation = control->run_generation;
    host_sync_event_signal(control->ready);
    vm_product_control_unlock(control);
    return TYPE_STATUS_OK;
}

C_UINT vm_product_control_begin_run(vm_product_control *control)
{
    C_UINT generation;

    if (control == STD_NULL) return 0u;
    vm_product_control_lock(control);
    ++control->run_generation;
    if (control->run_generation == 0u) ++control->run_generation;
    control->display_ready = TYPE_FALSE;
    /* A new guest run never inherits physical keys held by a retired run. */
    control->pressed_count = 0u;
    generation = control->run_generation;
    if (control->count == 0u) host_sync_event_reset(control->ready);
    vm_product_control_unlock(control);
    return generation;
}

static STD_SIZE_T vm_product_control_pressed_find(const vm_product_control *control,
    const ux_input_event *event)
{
    STD_SIZE_T index;

    if (control == STD_NULL || event == STD_NULL) return 0u;
    for (index = 0u; index < control->pressed_count; ++index) {
        const vm_product_control_pressed_key *pressed = &control->pressed[index];

        if (pressed->source_identity == event->source_identity &&
            pressed->event.data.key.scan_code == event->data.key.scan_code &&
            pressed->event.data.key.key == event->data.key.key) return index;
    }
    return control->pressed_count;
}

static C_VOID vm_product_control_pressed_forget(vm_product_control *control,
    const ux_input_event *event)
{
    STD_SIZE_T index;

    if (control == STD_NULL || event == STD_NULL) return;
    index = vm_product_control_pressed_find(control, event);
    if (index == control->pressed_count) return;
    control->pressed[index] = control->pressed[--control->pressed_count];
}

static type_status vm_product_control_pressed_remember(vm_product_control *control,
    const ux_input_event *event)
{
    STD_SIZE_T index;

    if (control == STD_NULL || event == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    index = vm_product_control_pressed_find(control, event);
    if (index != control->pressed_count) return TYPE_STATUS_OK;
    if (control->pressed_count == VM_PRODUCT_CONTROL_PRESSED_CAPACITY)
        return TYPE_STATUS_NO_MEMORY;
    control->pressed[control->pressed_count].source_identity = event->source_identity;
    control->pressed[control->pressed_count].event = *event;
    ++control->pressed_count;
    return TYPE_STATUS_OK;
}

type_status vm_product_control_dispatch_host_input(vm_product_control *control,
    const ux_input_event *event, C_INT session_running,
    vm_product_control_input_sink sink, C_VOID *sink_context)
{
    STD_SIZE_T index = 0u;
    type_status status = TYPE_STATUS_OK;

    if (control == STD_NULL || event == STD_NULL || sink == STD_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    if (event->type == UX_EVENT_SOURCE_RETIRED) {
        while (index < control->pressed_count) {
            vm_product_control_pressed_key *pressed = &control->pressed[index];

            if (pressed->source_identity != event->source_identity) {
                ++index;
                continue;
            }
            pressed->event.data.key.pressed = TYPE_FALSE;
            if (session_running && status == TYPE_STATUS_OK)
                status = sink(sink_context, &pressed->event);
            control->pressed[index] = control->pressed[--control->pressed_count];
        }
        return status;
    }
    if (!session_running) return TYPE_STATUS_OK;
    if (event->type == UX_EVENT_KEY && event->data.key.pressed) {
        status = vm_product_control_pressed_remember(control, event);
        if (status != TYPE_STATUS_OK) return status;
        status = sink(sink_context, event);
        if (status != TYPE_STATUS_OK) vm_product_control_pressed_forget(control, event);
        return status;
    }
    if (event->type == UX_EVENT_KEY && !event->data.key.pressed) {
        vm_product_control_pressed_forget(control, event);
        return sink(sink_context, event);
    }
    return (event->type == UX_EVENT_MOUSE || event->type == UX_EVENT_TEXT) ?
        sink(sink_context, event) : TYPE_STATUS_OK;
}

type_status vm_product_control_take(vm_product_control *control,
    vm_product_control_fact *out_fact,
    core_machine_guest_display_frame *out_display,
    C_UINT timeout_milliseconds)
{
    if (control == STD_NULL || out_fact == STD_NULL || out_display == STD_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    for (;;) {
        if (host_sync_event_wait(control->ready, timeout_milliseconds) !=
            HOST_SYNC_WAIT_SIGNALED) return TYPE_STATUS_INVALID_STATE;
        vm_product_control_lock(control);
        if (control->delivery_failed) {
            vm_product_control_unlock(control);
            return TYPE_STATUS_NO_MEMORY;
        }
        if (control->count != 0u) {
            *out_fact = control->facts[control->first];
            control->first = (control->first + 1u) % VM_PRODUCT_CONTROL_FACT_CAPACITY;
            --control->count;
            if (out_fact->kind == VM_PRODUCT_CONTROL_FACT_HOST_INPUT &&
                out_fact->run_generation != control->run_generation) {
                if (control->count == 0u && !control->display_ready)
                    host_sync_event_reset(control->ready);
                vm_product_control_unlock(control);
                continue;
            }
        } else if (control->display_ready) {
            if (control->latest_display_generation != control->run_generation) {
                control->display_ready = TYPE_FALSE;
                host_sync_event_reset(control->ready);
                vm_product_control_unlock(control);
                continue;
            }
            STD_MEMSET(out_fact, 0, sizeof(*out_fact));
            out_fact->kind = VM_PRODUCT_CONTROL_FACT_DISPLAY;
            *out_display = control->latest_display;
            control->display_ready = TYPE_FALSE;
        } else {
            vm_product_control_unlock(control);
            return TYPE_STATUS_INVALID_STATE;
        }
        if (control->count == 0u && !control->display_ready)
            host_sync_event_reset(control->ready);
        vm_product_control_unlock(control);
        return TYPE_STATUS_OK;
    }
}

C_VOID vm_product_control_close(vm_product_control *control)
{
    if (control == STD_NULL) return;
    vm_product_control_lock(control);
    control->accepting = TYPE_FALSE;
    host_sync_event_signal(control->ready);
    vm_product_control_unlock(control);
}

const C_CHAR *vm_product_control_note_lifecycle(vm_product_control *control,
    vm_session_lifecycle lifecycle)
{
    const C_CHAR *name;

    if (control == STD_NULL) return "stopped";
    if (lifecycle == VM_SESSION_RESET) return "reset";
    if (lifecycle == VM_SESSION_RUNNING) {
        name = control->lifecycle == VM_SESSION_PAUSED ? "resumed" : "started";
    } else if (lifecycle == VM_SESSION_PAUSED) {
        name = "paused";
    } else {
        name = "stopped";
    }
    control->lifecycle = lifecycle;
    return name;
}
