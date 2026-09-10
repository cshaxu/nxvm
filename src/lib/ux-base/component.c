#include "lib/ux-base/component.h"

static atomic_uint_fast64_t ux_component_next_source_identity = 1u;

lib_status ux_component_allocate_source_identity(atomic_uint_fast64_t *next,
    lib_u64 *out_identity)
{
    uint_fast64_t identity;
    uint_fast64_t following;

    if (next == LIB_NULL || out_identity == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    identity = atomic_load_explicit(next, memory_order_relaxed);
    for (;;) {
        /* Zero is written only after UINT64_MAX has been issued.  Do not use
         * fetch-add here: its next failed call would wrap zero to one and
         * eventually reuse a source identity. */
        if (identity == 0u) return LIB_STATUS_LIMIT_EXCEEDED;
        following = identity == UINT64_MAX ? 0u : identity + 1u;
        if (atomic_compare_exchange_weak_explicit(next, &identity, following,
                memory_order_relaxed, memory_order_relaxed)) {
            *out_identity = (lib_u64)identity;
            return LIB_STATUS_OK;
        }
    }
}

static void ux_component_report_failure(ux_component *component, lib_status status)
{
    if (component != LIB_NULL && component->failure_sink != LIB_NULL &&
        status != LIB_STATUS_OK)
        component->failure_sink(component->failure_context,
            component->source_identity, status);
}

lib_status ux_component_initialize(ux_component *component,
    const ux_component_options *options, ux_component_native_stop_fn native_stop,
    ux_component_dispose_fn dispose)
{
    lib_u64 identity;
    if (component == LIB_NULL || options == LIB_NULL || options->input_sink == LIB_NULL ||
        options->failure_sink == LIB_NULL ||
        native_stop == LIB_NULL || dispose == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    component->input_context = options->input_context;
    component->input_sink = options->input_sink;
    component->failure_context = options->failure_context;
    component->failure_sink = options->failure_sink;
    component->native_stop = native_stop;
    component->dispose = dispose;
    if (ux_component_allocate_source_identity(&ux_component_next_source_identity,
            &identity) != LIB_STATUS_OK)
        return LIB_STATUS_LIMIT_EXCEEDED;
    component->source_identity = identity;
    ux_hotkey_matcher_initialize(&component->hotkey_matcher, &options->hotkeys);
    atomic_init(&component->stopping, 0);
    return ux_component_mailboxes_create(&component->mailboxes);
}

int ux_component_emit_to(ux_component *component, const ux_input_event *event,
    ux_input_sink delivery_sink, void *delivery_context)
{
    ux_input_event copied;
    if (component == LIB_NULL || event == LIB_NULL || delivery_sink == LIB_NULL ||
        atomic_load_explicit(&component->stopping, memory_order_acquire) != 0) return 0;
    copied = *event;
    ux_input_event_set_source(&copied, component, component->source_identity);
    if (!ux_hotkey_matcher_submit(&component->hotkey_matcher, &copied,
            delivery_sink, delivery_context)) {
        ux_component_report_failure(component, LIB_STATUS_IO_ERROR);
        return 0;
    }
    return 1;
}

int ux_component_emit(ux_component *component, const ux_input_event *event)
{
    if (component == LIB_NULL) return 0;
    return ux_component_emit_to(component, event, component->input_sink,
        component->input_context);
}

lib_status ux_component_enqueue_controls(ux_component *component,
    const ux_component_control *controls, lib_u32 control_count)
{
    lib_status status;

    if (component == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = ux_component_mailboxes_enqueue_controls(&component->mailboxes,
        controls, control_count);
    if (status != LIB_STATUS_OK) ux_component_report_failure(component, status);
    return status;
}

void ux_component_emit_source_retired(ux_component *component)
{
    ux_input_event event = { .type = UX_EVENT_SOURCE_RETIRED };
    if (component == LIB_NULL || component->input_sink == LIB_NULL) return;
    ux_input_event_set_source(&event, component, component->source_identity);
    atomic_store_explicit(&component->stopping, 1, memory_order_release);
    if (!component->input_sink(component->input_context, &event))
        ux_component_report_failure(component, LIB_STATUS_IO_ERROR);
}

lib_status ux_component_publish_frame(ux_component *component, const ux_frame *frame)
{
    return component == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        ux_component_mailboxes_publish_frame(&component->mailboxes, frame);
}

lib_status ux_component_request_stop(ux_component *component)
{
    ux_component_control control = { .kind = UX_COMPONENT_CONTROL_STOP };
    return component == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        ux_component_enqueue_controls(component, &control, 1u);
}

void ux_component_destroy(ux_component *component)
{
    if (component == LIB_NULL) return;
    if (ux_component_request_stop(component) != LIB_STATUS_OK) {
        ux_component_report_failure(component, LIB_STATUS_INVALID_STATE);
        return;
    }
    component->native_stop(component);
    component->dispose(component);
}
