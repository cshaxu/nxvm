#ifndef KVM_WINDOW_MOTION_H
#define KVM_WINDOW_MOTION_H
#include "lib/types/types_interface.h"
typedef struct kvm_window_motion {
    int x, y, valid;
    lib_i64 remainder_x, remainder_y;
    int client_width, client_height;
    lib_u32 content_width, content_height;
} kvm_window_motion;
int kvm_window_motion_move(kvm_window_motion *motion, int x, int y,
    int client_width, int client_height, lib_u32 content_width,
    lib_u32 content_height, int *dx, int *dy);
#endif
