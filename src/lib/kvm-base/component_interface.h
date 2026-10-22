#ifndef KVM_BASE_COMPONENT_INTERFACE_H
#define KVM_BASE_COMPONENT_INTERFACE_H

#include "lib/kvm-base/hotkey_interface.h"

typedef struct kvm_component kvm_component;

/* Delivery is deliberately reported without exposing an application queue.
 * The callback must be non-blocking; it is the application's fault boundary
 * for an input record that could not be copied into its control path.
 * May run on any detecting thread, before retirement. Copy/enqueue only;
 * never synchronously destroy or rebind from this callback. */
typedef void (*kvm_component_failure_sink)(void *context,
    lib_u64 source_identity, lib_status status);

/* Identical creation contract for every KVM leaf. Leaf-specific capability is
 * deliberately absent here: Window title/mouse and Console logical-handle
 * access remain explicit leaf APIs. */
typedef struct kvm_component_options {
    void *input_context;
    kvm_input_sink input_sink;
    void *failure_context;
    kvm_component_failure_sink failure_sink;
    kvm_hotkey_registry hotkeys;
} kvm_component_options;

/* Appends one FIFO STOP record. A repeated request is idempotent; a full
 * ordinary control queue still has its reserved STOP slot. Other control
 * enqueue failures are returned. A rejected request is not a worker fault. */
lib_status kvm_component_request_stop(kvm_component *component);
/* Synchronous destruction: request terminal stop, wait for the worker and its
 * callbacks to exit, then free storage. The owner serializes destroy and never
 * calls it from a component callback or worker. Every normal return consumes
 * the object; errors report cleanup failures after quiescence. If worker exit
 * cannot be established, the process terminates without callbacks. NULL is OK. */
lib_status kvm_component_destroy(kvm_component *component);

#endif
