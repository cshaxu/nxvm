#ifndef KVM_WIN32_MOUSE_H
#define KVM_WIN32_MOUSE_H

#include "lib/kvm-window/motion.h"
#include "lib/types/win32/window.h"

/* Host-only relative-pointer state. The caller owns input-device buttons and
 * lifecycle policy; this component owns explicit Win32 client capture only. */
typedef struct kvm_win32_mouse {
    kvm_window_motion motion;
    lib_bool captured;
    lib_win32_hwnd window;
} kvm_win32_mouse;

void kvm_win32_mouse_reset(kvm_win32_mouse *mouse);
lib_status kvm_win32_mouse_release(kvm_win32_mouse *mouse);
int kvm_win32_mouse_refresh_bounds(kvm_win32_mouse *mouse);
lib_status kvm_win32_mouse_capture(kvm_win32_mouse *mouse,
    lib_win32_hwnd window, lib_win32_lparam position);
int kvm_win32_mouse_move(kvm_win32_mouse *mouse,
    lib_win32_lparam position, int client_width, int client_height,
    unsigned int content_width, unsigned int content_height, int *dx, int *dy);
int kvm_win32_mouse_captured(const kvm_win32_mouse *mouse);
#endif
