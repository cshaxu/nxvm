#ifndef VM_PLATFORM_INTERNAL_H
#define VM_PLATFORM_INTERNAL_H

#include "vm/platform/platform.h"
#include "lib/ux/actions.h"
#include "lib/ux/presenter.h"

#define VM_PLATFORM_START_TIMEOUT_MILLISECONDS 5000u

struct vm_platform_run_context {
    const vm_platform_execution *execution;
    vm_platform_host_input_sink input_sink;
    const core_machine_guest_presentation_mailbox *presentation;
    const core_utils_wait_scope *wait_scope;
    ux_presenter *ux_presenter;
    core_machine_guest_display_frame *core_frame;
    ux_frame *ux_frame;
    ux_action_registry ux_actions;
    ux_target requested_target;
    type_unsigned_32 console_text_frames;
    vm_platform_display_mode display_mode;
};

struct vm_platform_run_handle {
    vm_platform_run_context *context;
    C_VOID *backend;
    STD_ATOMIC_INT last_event;
    STD_ATOMIC_BOOL stop_reported;
    STD_ATOMIC_BOOL pause_reported;
    C_INT active;
    type_bool ux_pressed_keys[512u];
};

C_VOID vm_platform_run_handle_initialize(vm_platform_run_handle *handle);

#endif
