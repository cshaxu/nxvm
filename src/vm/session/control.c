#include "type.h"

#include "lib/host/sync_interface.h"
#include "vm/session/control.h"

#define VM_SESSION_CONTROL_FACT_CAPACITY 64u
#define VM_SESSION_CONTROL_PRESSED_CAPACITY 256u

typedef struct vm_session_control_pressed_key {
    lib_u64 source_identity;
    ui_input_event event;
} vm_session_control_pressed_key;

struct vm_session_control {
    host_sync_event *ready;
    STD_ATOMIC_FLAG lock;
    vm_session_fact facts[VM_SESSION_CONTROL_FACT_CAPACITY];
    STD_SIZE_T first;
    STD_SIZE_T count;
    C_INT accepting;
    C_INT delivery_failed;
    vm_machine_display_event latest_display;
    C_INT display_ready;
    type_unsigned_32 latest_display_generation;
    type_unsigned_32 run_generation;
    vm_machine_result_kind lifecycle;
    vm_session_control_pressed_key pressed[VM_SESSION_CONTROL_PRESSED_CAPACITY];
    STD_SIZE_T pressed_count;
};

static C_VOID vm_session_control_lock(vm_session_control *control)
{
    while (STD_ATOMIC_FLAG_TEST_AND_SET_EXPLICIT(&control->lock,
        STD_MEMORY_ORDER_ACQUIRE)) { }
}

static C_VOID vm_session_control_unlock(vm_session_control *control)
{
    STD_ATOMIC_FLAG_CLEAR_EXPLICIT(&control->lock, STD_MEMORY_ORDER_RELEASE);
}

type_status vm_session_control_create(vm_session_control **out_control)
{
    vm_session_control *control;

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
    *out_control = control;
    return TYPE_STATUS_OK;
}

C_VOID vm_session_control_destroy(vm_session_control *control)
{
    if (control == STD_NULL) return;
    host_sync_event_destroy(control->ready);
    STD_FREE(control);
}

static type_status vm_session_control_publish(vm_session_control *control,
    const vm_session_fact *fact)
{
    STD_SIZE_T index;

    if (control == STD_NULL || fact == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    vm_session_control_lock(control);
    if (!control->accepting) {
        vm_session_control_unlock(control);
        return TYPE_STATUS_INVALID_STATE;
    }
    if (control->count == VM_SESSION_CONTROL_FACT_CAPACITY) {
        control->delivery_failed = TYPE_TRUE;
        host_sync_event_signal(control->ready);
        vm_session_control_unlock(control);
        return TYPE_STATUS_NO_MEMORY;
    }
    index = (control->first + control->count) % VM_SESSION_CONTROL_FACT_CAPACITY;
    control->facts[index] = *fact;
    control->facts[index].run_generation = control->run_generation;
    ++control->count;
    host_sync_event_signal(control->ready);
    vm_session_control_unlock(control);
    return TYPE_STATUS_OK;
}

type_status vm_session_control_publish_console_line(vm_session_control *control,
    const C_CHAR *line)
{
    vm_session_fact fact = {0};
    STD_SIZE_T length;

    if (line == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    length = STD_STRLEN(line);
    if (length >= sizeof(fact.value.line)) return TYPE_STATUS_INVALID_ARGUMENT;
    fact.kind = VM_SESSION_FACT_CONSOLE_LINE;
    STD_MEMCPY(fact.value.line, line, length + 1u);
    return vm_session_control_publish(control, &fact);
}

type_status vm_session_control_publish_presentation_input(vm_session_control *control,
    const ui_input_event *event)
{
    vm_session_fact fact = {0};

    if (event == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    fact.kind = VM_SESSION_FACT_PRESENTATION_INPUT;
    fact.value.presentation_input = *event;
    return vm_session_control_publish(control, &fact);
}

type_status vm_session_control_publish_machine_result(vm_session_control *control,
    const vm_machine_result *result)
{
    vm_session_fact fact = {0};

    if (control == STD_NULL || result == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (result->kind == VM_MACHINE_RESULT_DISPLAY) {
        vm_session_control_lock(control);
        if (!control->accepting) {
            vm_session_control_unlock(control);
            return TYPE_STATUS_INVALID_STATE;
        }
        control->latest_display = result->value.display;
        control->display_ready = TYPE_TRUE;
        control->latest_display_generation = control->run_generation;
        host_sync_event_signal(control->ready);
        vm_session_control_unlock(control);
        return TYPE_STATUS_OK;
    }
    fact.kind = VM_SESSION_FACT_MACHINE_RESULT;
    fact.value.machine.kind = result->kind;
    fact.value.machine.status = result->status;
    return vm_session_control_publish(control, &fact);
}

C_VOID vm_session_control_machine_result_sink(C_VOID *context,
    const vm_machine_result *result)
{
    (C_VOID)vm_session_control_publish_machine_result(
        (vm_session_control *)context, result);
}

type_unsigned_32 vm_session_control_begin_run(vm_session_control *control)
{
    type_unsigned_32 generation;

    if (control == STD_NULL) return 0u;
    vm_session_control_lock(control);
    ++control->run_generation;
    if (control->run_generation == 0u) ++control->run_generation;
    control->display_ready = TYPE_FALSE;
    control->pressed_count = 0u;
    generation = control->run_generation;
    if (control->count == 0u) host_sync_event_reset(control->ready);
    vm_session_control_unlock(control);
    return generation;
}

type_status vm_session_control_take(vm_session_control *control,
    vm_session_fact *out_fact, vm_machine_display_event *out_display,
    type_unsigned_32 timeout_milliseconds)
{
    if (control == STD_NULL || out_fact == STD_NULL || out_display == STD_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    for (;;) {
        if (host_sync_event_wait(control->ready, timeout_milliseconds) !=
            HOST_SYNC_WAIT_SIGNALED) return TYPE_STATUS_INVALID_STATE;
        vm_session_control_lock(control);
        if (control->delivery_failed) {
            vm_session_control_unlock(control);
            return TYPE_STATUS_NO_MEMORY;
        }
        if (control->count != 0u) {
            *out_fact = control->facts[control->first];
            control->first = (control->first + 1u) % VM_SESSION_CONTROL_FACT_CAPACITY;
            --control->count;
            if (out_fact->run_generation != control->run_generation &&
                out_fact->kind != VM_SESSION_FACT_CONSOLE_LINE) {
                if (control->count == 0u && !control->display_ready)
                    host_sync_event_reset(control->ready);
                vm_session_control_unlock(control);
                continue;
            }
        } else if (control->display_ready) {
            if (control->latest_display_generation != control->run_generation) {
                control->display_ready = TYPE_FALSE;
                host_sync_event_reset(control->ready);
                vm_session_control_unlock(control);
                continue;
            }
            STD_MEMSET(out_fact, 0, sizeof(*out_fact));
            out_fact->kind = VM_SESSION_FACT_DISPLAY;
            *out_display = control->latest_display;
            control->display_ready = TYPE_FALSE;
        } else {
            vm_session_control_unlock(control);
            return TYPE_STATUS_INVALID_STATE;
        }
        if (control->count == 0u && !control->display_ready)
            host_sync_event_reset(control->ready);
        vm_session_control_unlock(control);
        return TYPE_STATUS_OK;
    }
}

C_VOID vm_session_control_close(vm_session_control *control)
{
    if (control == STD_NULL) return;
    vm_session_control_lock(control);
    control->accepting = TYPE_FALSE;
    host_sync_event_signal(control->ready);
    vm_session_control_unlock(control);
}

const C_CHAR *vm_session_control_note_machine_result(vm_session_control *control,
    vm_machine_result_kind kind)
{
    const C_CHAR *name;

    if (control == STD_NULL) return "stopped";
    if (kind == VM_MACHINE_RESULT_RESET) return "reset";
    if (kind == VM_MACHINE_RESULT_RUNNING) {
        name = control->lifecycle == VM_MACHINE_RESULT_PAUSED ? "resumed" : "started";
    } else if (kind == VM_MACHINE_RESULT_PAUSED) {
        name = "paused";
    } else {
        name = "stopped";
    }
    control->lifecycle = kind;
    return name;
}

static STD_SIZE_T vm_session_control_pressed_find(const vm_session_control *control,
    const ui_input_event *event)
{
    STD_SIZE_T index;

    if (control == STD_NULL || event == STD_NULL) return 0u;
    for (index = 0u; index < control->pressed_count; ++index) {
        const vm_session_control_pressed_key *pressed = &control->pressed[index];

        if (pressed->source_identity == event->source_identity &&
            pressed->event.data.key.scan_code == event->data.key.scan_code &&
            pressed->event.data.key.key == event->data.key.key) return index;
    }
    return control->pressed_count;
}

static C_VOID vm_session_control_pressed_forget(vm_session_control *control,
    const ui_input_event *event)
{
    STD_SIZE_T index;

    if (control == STD_NULL || event == STD_NULL) return;
    index = vm_session_control_pressed_find(control, event);
    if (index == control->pressed_count) return;
    control->pressed[index] = control->pressed[--control->pressed_count];
}

static type_status vm_session_control_pressed_remember(vm_session_control *control,
    const ui_input_event *event)
{
    STD_SIZE_T index;

    if (control == STD_NULL || event == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    index = vm_session_control_pressed_find(control, event);
    if (index != control->pressed_count) return TYPE_STATUS_OK;
    if (control->pressed_count == VM_SESSION_CONTROL_PRESSED_CAPACITY)
        return TYPE_STATUS_NO_MEMORY;
    control->pressed[control->pressed_count].source_identity = event->source_identity;
    control->pressed[control->pressed_count].event = *event;
    ++control->pressed_count;
    return TYPE_STATUS_OK;
}

type_status vm_session_control_dispatch_host_input(vm_session_control *control,
    const ui_input_event *event, C_INT machine_running,
    vm_session_control_input_sink sink, C_VOID *sink_context)
{
    STD_SIZE_T index = 0u;
    type_status status = TYPE_STATUS_OK;

    if (control == STD_NULL || event == STD_NULL || sink == STD_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    if (event->type == UI_EVENT_SOURCE_RETIRED) {
        while (index < control->pressed_count) {
            vm_session_control_pressed_key *pressed = &control->pressed[index];

            if (pressed->source_identity != event->source_identity) {
                ++index;
                continue;
            }
            pressed->event.data.key.pressed = TYPE_FALSE;
            if (machine_running && status == TYPE_STATUS_OK)
                status = sink(sink_context, &pressed->event);
            control->pressed[index] = control->pressed[--control->pressed_count];
        }
        return status;
    }
    if (!machine_running) return TYPE_STATUS_OK;
    if (event->type == UI_EVENT_KEY && event->data.key.pressed) {
        status = vm_session_control_pressed_remember(control, event);
        if (status != TYPE_STATUS_OK) return status;
        status = sink(sink_context, event);
        if (status != TYPE_STATUS_OK) vm_session_control_pressed_forget(control, event);
        return status;
    }
    if (event->type == UI_EVENT_KEY && !event->data.key.pressed) {
        vm_session_control_pressed_forget(control, event);
        return sink(sink_context, event);
    }
    return (event->type == UI_EVENT_MOUSE || event->type == UI_EVENT_TEXT) ?
        sink(sink_context, event) : TYPE_STATUS_OK;
}
