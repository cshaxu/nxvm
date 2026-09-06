#ifndef VM_PLATFORM_INTERNAL_H
#define VM_PLATFORM_INTERNAL_H

#include "vm/platform/platform.h"
#include "lib/ux/mailbox.h"
#include "lib/ux/actions.h"
#include "lib/ux/router.h"

struct vm_platform_run_context {
    const lib_session_executor *execution;
    vm_platform_host_input_sink input_sink;
    const core_platform_presentation_mailbox *presentation;
    const core_utils_wait_scope *wait_scope;
    ux_mailbox *ux_mailbox;
    core_platform_display_frame *core_frame;
    ux_frame *ux_frame;
    ux_action_registry ux_actions;
    ux_router ux_router;
    type_unsigned_32 console_text_frames;
    vm_platform_display_mode display_mode;
};

struct vm_platform_run_handle {
    const vm_platform_run_context *context;
    C_VOID *backend;
    STD_ATOMIC_INT last_event;
    STD_ATOMIC_BOOL stop_reported;
    STD_ATOMIC_BOOL pause_reported;
    STD_ATOMIC_BOOL mouse_release_reported;
    C_INT active;
    C_INT window_display;
    type_bool ux_pressed_keys[512u];
};

C_VOID vm_platform_run_handle_initialize(vm_platform_run_handle *handle);

#endif
