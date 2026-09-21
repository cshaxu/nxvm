#ifndef KVM_WINDOW_H
#define KVM_WINDOW_H

#include "lib/kvm-window/window_interface.h"
#include "lib/kvm-base/worker_interface.h"

/* Ordinary control kinds are private to this consumer. STOP remains the
 * shared transport marker; the FIFO does not interpret these payloads. */
enum {
    KVM_WINDOW_CONTROL_SET_TITLE = 1u,
    KVM_WINDOW_CONTROL_SET_FROZEN,
    KVM_WINDOW_CONTROL_RELEASE_MOUSE
};

_Static_assert(KVM_WINDOW_TITLE_CAPACITY <= KVM_COMPONENT_CONTROL_PAYLOAD_CAPACITY,
    "Window title must fit the copied control payload");

struct kvm_window {
    kvm_component base;
    kvm_window_frame pending_frame;
    char initial_title[KVM_WINDOW_TITLE_CAPACITY];
    lib_bool initial_frozen;
    void *worker_state;
};

lib_status kvm_window_worker_start(kvm_window *window);
lib_status kvm_window_worker_join(kvm_window *window, lib_u32 timeout_ms);

#endif
