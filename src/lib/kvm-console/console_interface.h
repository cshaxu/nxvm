#ifndef KVM_CONSOLE_INTERFACE_H
#define KVM_CONSOLE_INTERFACE_H

#include "lib/console/console_interface.h"
#include "lib/kvm-base/component_interface.h"
#include "lib/kvm-base/frame_exchange_interface.h"
#include "lib/kvm-console/frame_interface.h"

typedef struct kvm_console kvm_console;

typedef kvm_component_options kvm_console_options;

/* On failure *out_console remains NULL. A live worker that cannot be joined is
 * an internal fatal failure: the process exits without returning a half object. */
lib_status kvm_console_create(kvm_console **out_console,
    const kvm_console_options *options);
/* Notify after committing a validated Console frame. Use a four-slot exchange
 * whose slots hold kvm_console_text_frame. Graphics have no representation.
 * The consumer retains displayed input until successful finish; activation can redraw
 * it without a new publication. No separate historical frame is retained.
 * The exchange stays fixed through destroy, including a cleanup-error return.
 * Notifications after STOP return INVALID_STATE. */
lib_status kvm_console_publish_frame(kvm_console *console, kvm_frame_exchange *frames);
/* Same checked destruction contract as kvm_component_destroy. */
lib_status kvm_console_destroy(kvm_console *console);
/* Borrowed logical Console object. The application passes it to the broker for
 * Current Console registration before it permits raw input. */
lib_console *kvm_console_get_console(const kvm_console *console);

#endif
