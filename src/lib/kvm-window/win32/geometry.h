#ifndef KVM_WIN32_GEOMETRY_H
#define KVM_WIN32_GEOMETRY_H

#include "lib/types/types_interface.h"

#include "lib/types/win32/window.h"

#include "lib/kvm-window/geometry.h"

/* Native rectangle marshalling only; all arithmetic belongs to geometry.c. */
static inline kvm_window_rect kvm_win32_rect_value(const lib_win32_rect *r)
{
    return (kvm_window_rect) { r->left, r->top, r->right, r->bottom };
}
static inline void kvm_win32_rect_store(lib_win32_rect *to, const kvm_window_rect *r)
{
    to->left = r->left;
    to->top = r->top;
    to->right = r->right;
    to->bottom = r->bottom;
}
lib_status kvm_win32_resize_client(lib_win32_hwnd window, lib_u32 width,
    lib_u32 height, lib_bool fit_work_area);
typedef enum kvm_win32_snap {
    KVM_WIN32_SNAP_NONE,
    KVM_WIN32_SNAP_TOP,
    KVM_WIN32_SNAP_SIDE
} kvm_win32_snap;
typedef enum kvm_win32_size_basis {
    KVM_WIN32_SIZE_WIDTH,
    KVM_WIN32_SIZE_HEIGHT,
    KVM_WIN32_SIZE_FIT
} kvm_win32_size_basis;
kvm_win32_snap kvm_win32_snap_state(lib_win32_hwnd window);
lib_status kvm_win32_enforce_client_aspect(lib_win32_hwnd window, lib_u32 source_width,
    lib_u32 source_height, kvm_win32_size_basis basis);
lib_status kvm_win32_maximize_client(lib_win32_hwnd window, lib_u32 source_width,
    lib_u32 source_height);
lib_status kvm_win32_constrain_sizing(lib_win32_hwnd window, lib_win32_wparam edge,
    lib_win32_rect *outer, lib_u32 source_width, lib_u32 source_height);
#endif
