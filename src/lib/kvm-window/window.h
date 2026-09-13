#ifndef KVM_WINDOW_H
#define KVM_WINDOW_H

#include "lib/kvm-window/window_interface.h"
#include "lib/kvm-base/worker_interface.h"

struct kvm_window {
    kvm_component base;
    char initial_title[KVM_WINDOW_TITLE_CAPACITY];
    lib_bool initial_frozen;
    void *worker_state;
};

lib_status kvm_window_worker_start(kvm_window *window);
lib_status kvm_window_worker_join(kvm_window *window, lib_u32 timeout_ms);

#endif
