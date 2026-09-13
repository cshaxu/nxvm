#ifndef KVM_CONSOLE_H
#define KVM_CONSOLE_H

#include "lib/kvm-console/console_interface.h"
#include "lib/kvm-base/worker_interface.h"

struct kvm_console {
    kvm_component base;
    lib_console *logical_console;
    void *worker_state;
};

lib_status kvm_console_publish_text_frame(kvm_console *console, const kvm_frame *frame);

lib_status kvm_console_worker_start(kvm_console *console);
lib_status kvm_console_worker_join(kvm_console *console, lib_u32 timeout_ms);

#endif
