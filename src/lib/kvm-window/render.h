#ifndef KVM_WINDOW_RENDER_H
#define KVM_WINDOW_RENDER_H
#include "lib/kvm-window/geometry.h"
int kvm_window_frame_size(const kvm_window_frame *frame, lib_u32 *width, lib_u32 *height);

/* Compare decoded RGB against the existing surface. False means no damage or
 * rejected input; rejection leaves surface/valid/changed untouched. */
int kvm_window_render_frame(const kvm_window_frame *frame, lib_u32 *pixels,
    lib_u32 width, lib_u32 height, int *valid,
    kvm_window_rect *changed);
#endif
