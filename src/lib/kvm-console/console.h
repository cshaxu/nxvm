#ifndef KVM_CONSOLE_H
#define KVM_CONSOLE_H

#include "lib/kvm-console/console_interface.h"
#include "lib/kvm-base/worker_interface.h"
#include "lib/kvm-base/input_interface.h"

struct kvm_console {
    kvm_component base;
    lib_console *logical_console;
    void *worker_state;
    lib_i32 previous_mouse_x, previous_mouse_y;
    lib_bool previous_mouse_valid;
};

lib_status kvm_console_publish_text_frame(kvm_console *console,
    const kvm_console_text_frame *frame, const kvm_console_text_frame *previous);

void kvm_console_receive_event(void *context, const lib_console_event *event);

lib_status kvm_console_worker_start(kvm_console *console);
lib_status kvm_console_worker_join(kvm_console *console, lib_u32 timeout_ms);

#endif
