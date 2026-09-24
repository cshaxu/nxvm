#ifndef KVM_WIN32_MOUSE_H
#define KVM_WIN32_MOUSE_H

#include "lib/kvm-window/motion.h"
#include "lib/types/win32/window.h"
#include "lib/types/win32/scalar.h"

/* Host-only relative-pointer state. The caller owns input-device buttons and
 * lifecycle policy; this component owns explicit Win32 client capture only. */
typedef struct kvm_win32_mouse {
    kvm_window_motion motion;
    lib_bool captured;
    lib_win32_hwnd window;
    lib_win32_rect bounds;
    lib_win32_handle device;
    lib_win32_word coordinate_flags;
    lib_i32 desktop_width, desktop_height;
} kvm_win32_mouse;

void kvm_win32_mouse_reset(kvm_win32_mouse *mouse);
lib_status kvm_win32_mouse_release(kvm_win32_mouse *mouse);
lib_i32 kvm_win32_mouse_refresh_bounds(kvm_win32_mouse *mouse);
lib_status kvm_win32_mouse_capture(kvm_win32_mouse *mouse,
    lib_win32_hwnd window);
lib_i32 kvm_win32_mouse_move(kvm_win32_mouse *mouse,
    lib_win32_lparam record, lib_i32 client_width, lib_i32 client_height,
    lib_u32 content_width, lib_u32 content_height, lib_i32 *dx, lib_i32 *dy);
lib_i32 kvm_win32_mouse_captured(const kvm_win32_mouse *mouse);
#endif
