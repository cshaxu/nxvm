#include "lib/ui-base/worker_interface.h"

static lib_atomic_u64 ui_component_next_source_identity = 1u;

lib_status ui_component_allocate_source_identity(lib_atomic_u64 *next,
    lib_u64 *out_identity)
{
    lib_u64 identity;
    lib_u64 following;

    if (next == LIB_NULL || out_identity == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    identity = lib_atomic_u64_load_explicit(next, LIB_MEMORY_ORDER_RELAXED);
    for (;;) {
        /* Zero is written only after UINT64_MAX has been issued.  Do not use
         * fetch-add here: its next failed call would wrap zero to one and
         * eventually reuse a source identity. */
        if (identity == 0u) return LIB_STATUS_LIMIT_EXCEEDED;
        following = identity == LIB_UINT64_MAX ? 0u : identity + 1u;
        if (lib_atomic_u64_compare_exchange_weak_explicit(next, &identity, following,
                LIB_MEMORY_ORDER_RELAXED, LIB_MEMORY_ORDER_RELAXED)) {
            *out_identity = (lib_u64)identity;
            return LIB_STATUS_OK;
        }
    }
}

static void ui_component_report_failure(ui_component *component, lib_status status)
{
    if (component != LIB_NULL && component->failure_sink != LIB_NULL &&
        status != LIB_STATUS_OK)
        component->failure_sink(component->failure_context,
            component->source_identity, status);
}

lib_status ui_component_initialize(ui_component *component,
    const ui_component_options *options, ui_component_join_fn join_worker,
    ui_component_dispose_fn dispose)
{
    lib_u64 identity;
    if (component == LIB_NULL || options == LIB_NULL || options->input_sink == LIB_NULL ||
        options->failure_sink == LIB_NULL ||
        join_worker == LIB_NULL || dispose == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    component->input_context = options->input_context;
    component->input_sink = options->input_sink;
    component->failure_context = options->failure_context;
    component->failure_sink = options->failure_sink;
    component->join_worker = join_worker;
    component->dispose = dispose;
    if (ui_component_allocate_source_identity(&ui_component_next_source_identity,
            &identity) != LIB_STATUS_OK)
        return LIB_STATUS_LIMIT_EXCEEDED;
    component->source_identity = identity;
    ui_hotkey_matcher_initialize(&component->hotkey_matcher, &options->hotkeys);
    lib_atomic_i32_initialize(&component->stopping, 0);
    lib_atomic_i32_initialize(&component->failure, LIB_STATUS_OK);
    return ui_component_mailboxes_create(&component->mailboxes);
}

int ui_component_emit_to(ui_component *component, const ui_input_event *event,
    ui_input_sink delivery_sink, void *delivery_context, lib_bool allow_replay)
{
    ui_input_event copied;
    if (component == LIB_NULL || event == LIB_NULL || delivery_sink == LIB_NULL ||
        lib_atomic_i32_load_explicit(&component->stopping,
            LIB_MEMORY_ORDER_ACQUIRE) != 0) return 0;
    copied = *event;
    ui_input_event_set_source(&copied, component, component->source_identity);
    if (!ui_hotkey_matcher_submit(&component->hotkey_matcher, &copied,
            delivery_sink, delivery_context, allow_replay)) {
        ui_hotkey_matcher_discard(&component->hotkey_matcher);
        ui_component_fail(component, LIB_STATUS_IO_ERROR);
        return 0;
    }
    return 1;
}

int ui_component_emit(ui_component *component, const ui_input_event *event)
{
    if (component == LIB_NULL) return 0;
    return ui_component_emit_to(component, event, component->input_sink,
        component->input_context, LIB_TRUE);
}

lib_status ui_component_enqueue_controls(ui_component *component,
    const ui_component_control *controls, lib_u32 control_count)
{
    lib_status status;

    if (component == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = ui_component_mailboxes_enqueue_controls(&component->mailboxes,
        controls, control_count);
    if (status != LIB_STATUS_OK) ui_component_report_failure(component, status);
    return status;
}

void ui_component_retire(ui_component *component, lib_status status)
{
    ui_input_event event = { .type = UI_EVENT_SOURCE_RETIRED };
    if (component == LIB_NULL || component->input_sink == LIB_NULL) return;
    ui_input_event_set_source(&event, component, component->source_identity);
    lib_atomic_i32_store_explicit(&component->stopping, 1,
        LIB_MEMORY_ORDER_RELEASE);
    ui_component_mailboxes_close(&component->mailboxes);
    ui_hotkey_matcher_discard(&component->hotkey_matcher);
    if (lib_atomic_i32_load_explicit(&component->failure, LIB_MEMORY_ORDER_ACQUIRE) != LIB_STATUS_OK)
        status = lib_atomic_i32_load_explicit(&component->failure, LIB_MEMORY_ORDER_ACQUIRE);
    ui_component_report_failure(component, status);
    if (!component->input_sink(component->input_context, &event) && status == LIB_STATUS_OK)
        ui_component_report_failure(component, LIB_STATUS_IO_ERROR);
}

lib_status ui_component_publish_frame(ui_component *component, const ui_frame *frame)
{
    return component == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        ui_component_mailboxes_publish_frame(&component->mailboxes, frame);
}

lib_status ui_component_request_stop(ui_component *component)
{
    ui_component_control control = { .kind = UI_COMPONENT_CONTROL_STOP };
    return component == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        ui_component_enqueue_controls(component, &control, 1u);
}

void ui_component_destroy(ui_component *component)
{
    if (component == LIB_NULL) return;
    if (ui_component_request_stop(component) != LIB_STATUS_OK) {
        ui_component_report_failure(component, LIB_STATUS_INVALID_STATE);
        return;
    }
    component->join_worker(component);
    ui_hotkey_matcher_discard(&component->hotkey_matcher);
    component->dispose(component);
}

void ui_component_fail(ui_component *component, lib_status status)
{
    lib_atomic_i32_store_explicit(&component->failure, status, LIB_MEMORY_ORDER_RELEASE);
    ui_component_mailboxes_close(&component->mailboxes);
    lib_atomic_i32_store_explicit(&component->stopping, 1, LIB_MEMORY_ORDER_RELEASE);
    ui_mailbox_wake_signal(ui_component_mailboxes_wake(&component->mailboxes));
}
