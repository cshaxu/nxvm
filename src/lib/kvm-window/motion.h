#ifndef KVM_WINDOW_MOTION_H
#define KVM_WINDOW_MOTION_H
#include "lib/types/types_interface.h"
typedef struct kvm_window_motion {
    lib_i32 x, y, valid;
    lib_i64 remainder_x, remainder_y;
    lib_i32 client_width, client_height;
    lib_u32 content_width, content_height;
} kvm_window_motion;
lib_i32 kvm_window_motion_move(kvm_window_motion *motion, lib_i32 x, lib_i32 y,
    lib_i32 client_width, lib_i32 client_height, lib_u32 content_width,
    lib_u32 content_height, lib_i32 *dx, lib_i32 *dy);
lib_i32 kvm_window_motion_scale(kvm_window_motion *motion, lib_i32 x, lib_i32 y,
    lib_i32 client_width, lib_i32 client_height, lib_u32 content_width,
    lib_u32 content_height, lib_i32 *dx, lib_i32 *dy);
#endif
