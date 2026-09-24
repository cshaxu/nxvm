#ifndef KVM_WINDOW_GEOMETRY_H
#define KVM_WINDOW_GEOMETRY_H
#include "lib/kvm-window/frame_interface.h"

typedef struct kvm_window_rect { lib_i32 left, top, right, bottom; } kvm_window_rect;
typedef enum kvm_window_edge {
    KVM_WINDOW_EDGE_LEFT, KVM_WINDOW_EDGE_RIGHT, KVM_WINDOW_EDGE_TOP,
    KVM_WINDOW_EDGE_BOTTOM, KVM_WINDOW_EDGE_TOPLEFT, KVM_WINDOW_EDGE_TOPRIGHT,
    KVM_WINDOW_EDGE_BOTTOMLEFT, KVM_WINDOW_EDGE_BOTTOMRIGHT
} kvm_window_edge;
void kvm_window_constrain_sizing(kvm_window_rect *outer, kvm_window_edge edge,
    lib_i32 frame_width, lib_i32 frame_height, lib_u32 source_width, lib_u32 source_height);
void kvm_window_map_dirty_rect(const kvm_window_rect *source, const kvm_window_rect *display,
    lib_u32 width, lib_u32 height, kvm_window_rect *out);
lib_i32 kvm_window_fit_outer_rect(const kvm_window_rect *work, lib_i32 width, lib_i32 height,
    kvm_window_rect *out);
lib_i32 kvm_window_fit_client_size(const kvm_window_rect *work, lib_i32 decoration_width,
    lib_i32 decoration_height, lib_i32 width, lib_i32 height, lib_i32 *out_width, lib_i32 *out_height);
/* Fit within bounds with upward pixel rounding; refitting the result is stable. */
lib_i32 kvm_window_fit_aspect_size(lib_i32 width, lib_i32 height, lib_u32 source_width,
    lib_u32 source_height, lib_i32 *out_width, lib_i32 *out_height);
lib_i32 kvm_window_cursor_rect(const kvm_window_frame *frame, const kvm_window_rect *display,
    kvm_window_rect *cursor);
#endif
