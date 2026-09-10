#ifndef UX_BASE_COMPONENT_H
#define UX_BASE_COMPONENT_H

#include "lib/ux-base/component_interface.h"
#include "lib/ux-base/mailbox.h"

#include <stdatomic.h>

typedef void (*ux_component_native_stop_fn)(ux_component *component);
typedef void (*ux_component_dispose_fn)(ux_component *component);

struct ux_component {
    ux_component_mailboxes mailboxes;
    void *input_context;
    ux_input_sink input_sink;
    void *failure_context;
    ux_component_failure_sink failure_sink;
    ux_hotkey_matcher hotkey_matcher;
    lib_u64 source_identity;
    atomic_int stopping;
    ux_component_native_stop_fn native_stop;
    ux_component_dispose_fn dispose;
};

lib_status ux_component_initialize(ux_component *component,
    const ux_component_options *options, ux_component_native_stop_fn native_stop,
    ux_component_dispose_fn dispose);
/* A source identity is never recycled.  Zero is the permanent exhausted
 * sentinel, rather than the beginning of a second allocation epoch. */
lib_status ux_component_allocate_source_identity(atomic_uint_fast64_t *next,
    lib_u64 *out_identity);
int ux_component_emit(ux_component *component, const ux_input_event *event);
/* Uses the component's normal source attribution and source-local matcher,
 * but lets a leaf choose how to deliver matcher output.  This is internal:
 * leaves may filter delivery but never replace matching semantics. */
int ux_component_emit_to(ux_component *component, const ux_input_event *event,
    ux_input_sink delivery_sink, void *delivery_context);
lib_status ux_component_enqueue_controls(ux_component *component,
    const ux_component_control *controls, lib_u32 control_count);
void ux_component_emit_source_retired(ux_component *component);

#endif
