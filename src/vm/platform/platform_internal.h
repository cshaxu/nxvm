#ifndef VM_PLATFORM_INTERNAL_H
#define VM_PLATFORM_INTERNAL_H

#include "vm/platform/platform.h"
#include "lib/host/console_interface.h"
#include "lib/base/console_interface.h"
#include "lib/ux-base/hotkey_interface.h"
#include "lib/ux-console/console_interface.h"
#include "lib/ux-window/window_interface.h"

#define VM_PLATFORM_START_TIMEOUT_MILLISECONDS 5000u

struct vm_platform_run_context {
    const vm_platform_execution *execution;
    vm_platform_host_input_sink input_sink;
    const core_machine_guest_presentation_mailbox *presentation;
    const core_utils_wait_scope *wait_scope;
    ux_window *window;
    ux_console *console;
    vm_platform_console_binding console_binding;
    core_machine_guest_display_frame *core_frame;
    ux_frame *ux_frame;
    ux_hotkey_registry hotkeys;
    vm_platform_run_handle *run_handle;
    C_INT window_active;
    type_unsigned_32 console_text_frames;
    vm_platform_display_mode display_mode;
};

struct vm_platform_run_handle {
    vm_platform_run_context *context;
    C_VOID *backend;
    STD_ATOMIC_INT last_event;
    STD_ATOMIC_BOOL stop_reported;
    STD_ATOMIC_BOOL pause_reported;
    STD_ATOMIC_BOOL window_close_reported;
    C_INT active;
    type_bool ux_pressed_keys[512u];
    type_unsigned_64 ux_key_sources[512u];
};

C_VOID vm_platform_run_handle_initialize(vm_platform_run_handle *handle);
C_INT vm_platform_run_context_handle_ux_input(C_VOID *opaque,
    const ux_input_event *event);

#endif
