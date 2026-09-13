#ifndef KVM_WINDOW_RENDER_H
#define KVM_WINDOW_RENDER_H
#include "lib/kvm-window/geometry.h"
int kvm_window_frame_size(const kvm_frame *frame, lib_u32 *width, lib_u32 *height);

void kvm_window_render_text(const kvm_frame *frame, lib_u32 *pixels,
    lib_u32 width, lib_u32 height);
int kvm_window_render_graphics(const kvm_frame *frame, lib_u32 *pixels,
    lib_u32 width, lib_u32 height, lib_u32 *palette, int *valid,
    kvm_window_rect *changed);
#endif
