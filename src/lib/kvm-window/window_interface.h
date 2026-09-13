#ifndef KVM_WINDOW_INTERFACE_H
#define KVM_WINDOW_INTERFACE_H

#include "lib/kvm-base/component_interface.h"

#define KVM_WINDOW_TITLE_CAPACITY 128u

typedef struct kvm_window kvm_window;

typedef struct kvm_window_options {
    kvm_component_options component;
    /* Copied during creation.  The application owns both initial product
     * identity and later title changes; kvm-window never supplies one. */
    const char *initial_title;
    /* Frozen forbids capture and cursor blink.  It does not own a capture;
     * an unfrozen Window still waits for a client-area click to acquire one. */
    lib_bool initial_frozen;
} kvm_window_options;

/* On failure *out_window remains NULL. A live worker that cannot be joined is
 * a terminal application infrastructure fault, not a caller-owned half object. */
lib_status kvm_window_create(kvm_window **out_window,
    const kvm_window_options *options);
lib_status kvm_window_publish_frame(kvm_window *window, const kvm_frame *frame);
/* Same checked destruction contract as kvm_component_destroy. */
lib_status kvm_window_destroy(kvm_window *window);
lib_status kvm_window_set_title(kvm_window *window, const char *title);
/* Freeze atomically prevents future capture, stops the Window-local cursor
 * blink, and releases any current capture in FIFO order. */
lib_status kvm_window_freeze(kvm_window *window);
/* Unfreeze permits a later client-area click to capture. It never captures
 * the mouse itself and resumes the Window-local cursor blink. The actual
 * frozen-to-unfrozen transition requests activation once; repeated unfreeze
 * does not request focus again. Activation remains subject to host policy. */
lib_status kvm_window_unfreeze(kvm_window *window);
lib_status kvm_window_release_mouse(kvm_window *window);

#endif
