/* Copyright 2012-2014 Neko. */

#ifndef VM_PLATFORM_H
#define VM_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "core/machine/guest_input_interface.h"
#include "core/machine/guest_presentation_mailbox_interface.h"
#include "core/utils/wait_provider.h"
#include "lib/session/state.h"

typedef type_status (*vm_platform_host_input_submit)(C_VOID *context,
    const core_machine_guest_input_event *event);

typedef struct vm_platform_host_input_sink {
    vm_platform_host_input_submit submit;
    C_VOID *context;
} vm_platform_host_input_sink;

/* This is the NXVM product execution boundary.  The library does not own it:
 * a product supplies its own execution turn and stop action around the
 * neutral lifecycle state. */
typedef void (*vm_platform_execution_run)(C_VOID *context);
typedef void (*vm_platform_execution_stop)(C_VOID *context);

typedef struct vm_platform_execution {
    lib_session_state *state;
    vm_platform_execution_run run;
    vm_platform_execution_stop stop;
    C_VOID *context;
} vm_platform_execution;

typedef enum vm_platform_display_mode {
    VM_PLATFORM_DISPLAY_CONSOLE,
    VM_PLATFORM_DISPLAY_WINDOW
} vm_platform_display_mode;

typedef struct vm_platform_run_context vm_platform_run_context;

typedef enum vm_platform_run_event {
    VM_PLATFORM_RUN_EVENT_NONE,
    VM_PLATFORM_RUN_EVENT_STOP_REQUESTED,
    VM_PLATFORM_RUN_EVENT_PAUSE_REQUESTED,
    VM_PLATFORM_RUN_EVENT_MOUSE_RELEASE_REQUESTED,
    VM_PLATFORM_RUN_EVENT_KERNEL_COMPLETED,
    VM_PLATFORM_RUN_EVENT_DISPLAY_COMPLETED,
    VM_PLATFORM_RUN_EVENT_STARTUP_FAILED
} vm_platform_run_event;

typedef struct vm_platform_run_handle vm_platform_run_handle;

type_status vm_platform_run_context_create(
    const vm_platform_execution *execution,
    const vm_platform_host_input_sink *input_sink,
    const core_machine_guest_presentation_mailbox *presentation,
    const core_utils_wait_scope *wait_scope,
    vm_platform_run_context **out_context);
C_VOID vm_platform_run_context_destroy(vm_platform_run_context *context);
type_status vm_platform_run_context_publish_ux_frame(
    vm_platform_run_context *context);
type_status vm_platform_host_input_sink_submit(
    const vm_platform_host_input_sink *sink,
    const core_machine_guest_input_event *event);
C_INT vm_platform_run_context_get_window_display(
    const vm_platform_run_context *context);
C_INT vm_platform_run_context_get_display_mode(
    const vm_platform_run_context *context);
C_VOID vm_platform_run_context_set_display_mode(
    vm_platform_run_context *context, vm_platform_display_mode mode);
C_VOID vm_platform_run_context_set_window_display(
    vm_platform_run_context *context, C_INT enabled);
type_status vm_platform_run_handle_create(vm_platform_run_handle **out_handle);
C_VOID vm_platform_run_handle_destroy(vm_platform_run_handle *handle);
C_INT vm_platform_run_handle_is_active(const vm_platform_run_handle *handle);
C_INT vm_platform_run_handle_is_window_display(
    const vm_platform_run_handle *handle);
C_VOID vm_platform_run_handle_report(
    vm_platform_run_handle *handle, vm_platform_run_event event);
vm_platform_run_event vm_platform_run_handle_get_last_event(
    const vm_platform_run_handle *handle);
C_INT vm_platform_run_handle_take_stop_report(
    vm_platform_run_handle *handle);
C_INT vm_platform_run_handle_take_pause_report(
    vm_platform_run_handle *handle);
C_INT vm_platform_run_handle_take_mouse_release_report(
    vm_platform_run_handle *handle);
C_VOID vm_platform_run_handle_request_stop(vm_platform_run_handle *handle);
C_VOID vm_platform_run_handle_request_presenter_stop(
    vm_platform_run_handle *handle);
C_VOID vm_platform_run_handle_join(vm_platform_run_handle *handle);
C_VOID vm_platform_run_handle_finalize(vm_platform_run_handle *handle);

type_status vm_platform_start(vm_platform_run_context *context,
    vm_platform_run_handle *handle);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
