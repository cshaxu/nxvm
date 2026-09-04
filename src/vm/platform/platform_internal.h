#ifndef VM_PLATFORM_INTERNAL_H
#define VM_PLATFORM_INTERNAL_H

#include "vm/platform/platform.h"

struct vm_platform_host_surface_context {
    vm_platform_host_surface_kind kind;
    C_VOID *native_handle;
};

struct vm_platform_host_surface_lease {
    STD_ATOMIC_UINTPTR_T owner;
};

struct vm_platform_run_context {
    const vm_platform_execution_transport *execution;
    vm_platform_host_input_sink input_sink;
    const core_platform_presentation_mailbox *presentation;
    const core_utils_wait_scope *wait_scope;
    const C_CHAR *font_path;
    vm_platform_host_surface_context console_surface;
    vm_platform_host_surface_context window_surface;
    C_VOID *console_renderer;
    C_VOID *window_renderer;
    type_unsigned_64 terminal_displayed_generation;
    vm_platform_display_mode display_mode;
    STD_ATOMIC_BOOL console_window_active;
    STD_ATOMIC_BOOL console_window_start_pending;
    STD_ATOMIC_BOOL console_window_stop_pending;
};

struct vm_platform_run_handle {
    const vm_platform_run_context *context;
    C_VOID *backend;
    STD_ATOMIC_INT last_event;
    STD_ATOMIC_BOOL stop_reported;
    STD_ATOMIC_BOOL pause_reported;
    STD_ATOMIC_BOOL mouse_release_reported;
    STD_ATOMIC_INT reserved_virtual_key;
    STD_ATOMIC_INT pending_modifier_keys;
    STD_ATOMIC_INT suppressed_modifier_keys;
    STD_ATOMIC_INT pending_control_scan_code;
    STD_ATOMIC_INT pending_alt_scan_code;
    C_INT active;
    C_INT window_display;
};

C_VOID vm_platform_run_handle_initialize(vm_platform_run_handle *handle);

#endif
