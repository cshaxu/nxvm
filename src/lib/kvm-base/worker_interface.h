#ifndef KVM_BASE_WORKER_INTERFACE_H
#define KVM_BASE_WORKER_INTERFACE_H

#include "lib/kvm-base/component_interface.h"
#include "lib/kvm-base/mailbox_interface.h"

#include "lib/types/atomic.h"

#define KVM_COMPONENT_DESTROY_TIMEOUT_MS 5000u
typedef lib_status (*kvm_component_join_fn)(kvm_component *component, lib_u32 timeout_ms);
typedef void (*kvm_component_dispose_fn)(kvm_component *component);

struct kvm_component {
    kvm_component_mailboxes mailboxes;
    void *input_context;
    kvm_input_sink input_sink;
    void *failure_context;
    kvm_component_failure_sink failure_sink;
    kvm_hotkey_matcher hotkey_matcher;
    lib_u64 source_identity;
    lib_atomic_i32 stopping;
    lib_atomic_i32 failure;
    kvm_component_join_fn join_worker;
    kvm_component_dispose_fn dispose;
};

lib_status kvm_component_initialize(kvm_component *component,
    const kvm_component_options *options, kvm_component_join_fn join_worker,
    kvm_component_dispose_fn dispose, void *frame_storage, lib_size frame_capacity);
/* Copied opaque payload. The leaf validates its value before this entry.
 * STOP or terminal fault rejects later publications with INVALID_STATE. */
lib_status kvm_component_publish_frame(kvm_component *component,
    const void *frame, lib_size bytes);
/* A source identity is never recycled.  Zero is the permanent exhausted
 * sentinel, rather than the beginning of a second allocation epoch. */
lib_status kvm_component_allocate_source_identity(lib_atomic_u64 *next,
    lib_u64 *out_identity);
int kvm_component_emit(kvm_component *component, const kvm_input_event *event);
/* Uses the component's normal source attribution and source-local matcher,
 * but lets a leaf choose how to deliver matcher output.  This is internal:
 * leaves may filter delivery but never replace matching semantics.
 * allow_replay tags this make's eligibility for later ordinary replay. */
int kvm_component_emit_to(kvm_component *component, const kvm_input_event *event,
    kvm_input_sink delivery_sink, void *delivery_context, lib_bool allow_replay);
lib_status kvm_component_enqueue_control(kvm_component *component,
    const kvm_component_control *control);
void kvm_component_retire(kvm_component *component, lib_status status);
/* Terminal failure: closes admission and reports once on the detecting thread,
 * then wakes the worker. Retirement follows input quiescence, not reporting. */
void kvm_component_fail(kvm_component *component, lib_status status);

#endif
