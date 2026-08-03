/* Copyright 2012-2014 Neko. */

#ifndef NXVM_PLATFORM_H
#define NXVM_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "core/platform/host_surface_interface.h"
#include "core/product/wait_provider.h"
#include "vm/platform/execution.h"
#include "vm/platform/input.h"
#include "vm/platform/presentation_mailbox.h"

typedef struct vm_platform_run_context {
    const vm_platform_execution_transport *execution;
    const vm_platform_keyboard_transport *keyboard;
    const vm_platform_presentation_mailbox *presentation;
    const core_product_wait_scope *wait_scope;
    core_platform_host_surface_context console_surface;
    core_platform_host_surface_context window_surface;
    C_VOID *console_renderer;
    C_VOID *window_renderer;
    uint64_t terminal_displayed_generation;
    C_INT window_display;
    vm_platform_keyboard_state_sink keyboard_state_sink;
    C_VOID *keyboard_state_context;
} vm_platform_run_context;

typedef struct vm_platform_run_handle {
    const vm_platform_run_context *context;
    C_VOID *backend;
    C_INT active;
    C_INT window_display;
} vm_platform_run_handle;

C_VOID vm_platform_run_context_initialize(
    vm_platform_run_context *context,
    const vm_platform_execution_transport *execution,
    const vm_platform_keyboard_transport *keyboard,
    const vm_platform_presentation_mailbox *presentation,
    const core_product_wait_scope *wait_scope);
C_VOID vm_platform_run_context_bind_keyboard_state(
    vm_platform_run_context *context, vm_platform_keyboard_state_sink sink,
    C_VOID *sink_context);
C_INT vm_platform_run_context_get_window_display(
    const vm_platform_run_context *context);
C_VOID vm_platform_run_context_set_window_display(
    vm_platform_run_context *context, C_INT enabled);
C_VOID vm_platform_run_handle_initialize(vm_platform_run_handle *handle);
C_INT vm_platform_run_handle_is_active(const vm_platform_run_handle *handle);
C_INT vm_platform_run_handle_is_window_display(
    const vm_platform_run_handle *handle);
C_VOID vm_platform_run_handle_request_stop(vm_platform_run_handle *handle);
C_VOID vm_platform_run_handle_join(vm_platform_run_handle *handle);
C_VOID vm_platform_run_handle_finalize(vm_platform_run_handle *handle);

/* Device Operations */
C_VOID vm_platform_display_set_screen(const vm_platform_run_context *context);
C_VOID vm_platform_display_paint(const vm_platform_run_context *context);

ntvdm64_status vm_platform_start(const vm_platform_run_context *context,
    vm_platform_run_handle *handle);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
