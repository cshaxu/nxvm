#ifndef KVM_MAILBOX_WAKE_INTERFACE_H
#define KVM_MAILBOX_WAKE_INTERFACE_H

#include "lib/types/types_interface.h"

typedef struct kvm_mailbox_wake kvm_mailbox_wake;

typedef enum kvm_mailbox_wake_wait_result {
    KVM_MAILBOX_WAKE_WAIT_WAKE,
    KVM_MAILBOX_WAKE_WAIT_TIMED_OUT
} kvm_mailbox_wake_wait_result;

lib_status kvm_mailbox_wake_create(kvm_mailbox_wake **out_wake);
void kvm_mailbox_wake_destroy(kvm_mailbox_wake *wake);
lib_status kvm_mailbox_wake_signal(kvm_mailbox_wake *wake);
lib_status kvm_mailbox_wake_wait(const kvm_mailbox_wake *wake,
    lib_u32 timeout_milliseconds, kvm_mailbox_wake_wait_result *out_result);
#endif
