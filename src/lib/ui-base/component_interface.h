#ifndef UI_BASE_COMPONENT_INTERFACE_H
#define UI_BASE_COMPONENT_INTERFACE_H

#include "lib/ui-base/hotkey_interface.h"
#include "lib/ui-base/frame_interface.h"

typedef struct ui_component ui_component;

/* Delivery is deliberately reported without exposing an application queue.
 * The callback must be non-blocking; it is the application's fault boundary
 * for an input record that could not be copied into its control path. */
typedef void (*ui_component_failure_sink)(void *context,
    lib_u64 source_identity, lib_status status);

/* Identical creation contract for every UI leaf. Leaf-specific capability is
 * deliberately absent here: Window title/mouse and Console logical-handle
 * access remain explicit leaf APIs. */
typedef struct ui_component_options {
    void *input_context;
    ui_input_sink input_sink;
    void *failure_context;
    ui_component_failure_sink failure_sink;
    ui_hotkey_registry hotkeys;
} ui_component_options;

/* Copies into this component's one-slot latest-wins frame mailbox. */
lib_status ui_component_publish_frame(ui_component *component,
    const ui_frame *frame);
/* Appends one FIFO STOP record. A repeated request is idempotent; a full
 * ordinary control queue still has its reserved STOP slot. Other control
 * enqueue failures are returned and reported through the failure sink. */
lib_status ui_component_request_stop(ui_component *component);
/* Synchronous destruction: returns only after the worker consumed STOP,
 * emitted SOURCE_RETIRED, and no worker remains. */
void ui_component_destroy(ui_component *component);

#endif
