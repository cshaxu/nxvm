#ifndef KVM_CONSOLE_H
#define KVM_CONSOLE_H

#include "lib/kvm-console/console_interface.h"
#include "lib/kvm-base/worker_interface.h"
#include "lib/kvm-base/input_interface.h"

struct kvm_console {
    kvm_component base;
    lib_console *logical_console;
    void *worker_state;
    kvm_keyboard_normalizer keyboard;
    lib_i32 previous_mouse_x, previous_mouse_y;
    int previous_mouse_valid;
};

lib_status kvm_console_publish_text_frame(kvm_console *console, const kvm_frame *frame);

void kvm_console_receive_event(void *context, const lib_console_event *event);
lib_u8 kvm_console_mouse_buttons(lib_u32 buttons);

lib_status kvm_console_worker_start(kvm_console *console);
lib_status kvm_console_worker_join(kvm_console *console, lib_u32 timeout_ms);

#endif
