#ifndef UI_BASE_COMPONENT_H
#define UI_BASE_COMPONENT_H

#include "lib/ui-base/component_interface.h"
#include "lib/ui-base/mailbox.h"

#include "lib/types/atomic.h"

typedef void (*ui_component_native_stop_fn)(ui_component *component);
typedef void (*ui_component_dispose_fn)(ui_component *component);

struct ui_component {
    ui_component_mailboxes mailboxes;
    void *input_context;
    ui_input_sink input_sink;
    void *failure_context;
    ui_component_failure_sink failure_sink;
    ui_hotkey_matcher hotkey_matcher;
    lib_u64 source_identity;
    lib_atomic_i32 stopping;
    ui_component_native_stop_fn native_stop;
    ui_component_dispose_fn dispose;
};

lib_status ui_component_initialize(ui_component *component,
    const ui_component_options *options, ui_component_native_stop_fn native_stop,
    ui_component_dispose_fn dispose);
/* A source identity is never recycled.  Zero is the permanent exhausted
 * sentinel, rather than the beginning of a second allocation epoch. */
lib_status ui_component_allocate_source_identity(lib_atomic_u64 *next,
    lib_u64 *out_identity);
int ui_component_emit(ui_component *component, const ui_input_event *event);
/* Uses the component's normal source attribution and source-local matcher,
 * but lets a leaf choose how to deliver matcher output.  This is internal:
 * leaves may filter delivery but never replace matching semantics. */
int ui_component_emit_to(ui_component *component, const ui_input_event *event,
    ui_input_sink delivery_sink, void *delivery_context);
lib_status ui_component_enqueue_controls(ui_component *component,
    const ui_component_control *controls, lib_u32 control_count);
void ui_component_emit_source_retired(ui_component *component);

#endif
