#ifndef KVM_CONSOLE_INTERFACE_H
#define KVM_CONSOLE_INTERFACE_H

#include "lib/console/console_interface.h"
#include "lib/kvm-base/component_interface.h"

typedef struct kvm_console kvm_console;

typedef kvm_component_options kvm_console_options;

/* On failure *out_console remains NULL. A live worker that cannot be joined is
 * a terminal application infrastructure fault, not a caller-owned half object. */
lib_status kvm_console_create(kvm_console **out_console,
    const kvm_console_options *options);
lib_status kvm_console_publish_frame(kvm_console *console, const kvm_frame *frame);
/* Same checked destruction contract as kvm_component_destroy. */
lib_status kvm_console_destroy(kvm_console *console);
/* Borrowed logical Console object. The application passes it to host for
 * Current Console registration before it permits raw input. */
lib_console *kvm_console_get_console(const kvm_console *console);

#endif
