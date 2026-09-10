#ifndef UX_BASE_COMPONENT_H
#define UX_BASE_COMPONENT_H

#include "lib/ux-base/hotkey.h"
#include "lib/ux-base/frame.h"

typedef struct ux_component ux_component;

/* Delivery is deliberately reported without exposing an application queue.
 * The callback must be non-blocking; it is the application's fault boundary
 * for an input record that could not be copied into its control path. */
typedef void (*ux_component_failure_sink)(void *context,
    lib_u64 source_identity, lib_status status);

/* Identical creation contract for every UX leaf. Leaf-specific capability is
 * deliberately absent here: Window title/mouse and Console logical-handle
 * access remain explicit leaf APIs. */
typedef struct ux_component_options {
    void *input_context;
    ux_input_sink input_sink;
    void *failure_context;
    ux_component_failure_sink failure_sink;
    ux_hotkey_registry hotkeys;
} ux_component_options;

/* Copies into this component's one-slot latest-wins frame mailbox. */
lib_status ux_component_publish_frame(ux_component *component,
    const ux_frame *frame);
/* Appends one FIFO STOP record. A repeated request is idempotent; a full
 * ordinary control queue still has its reserved STOP slot. Other control
 * enqueue failures are returned and reported through the failure sink. */
lib_status ux_component_request_stop(ux_component *component);
/* Synchronous destruction: returns only after the worker consumed STOP,
 * emitted SOURCE_RETIRED, and no worker remains. */
void ux_component_destroy(ux_component *component);

#endif
