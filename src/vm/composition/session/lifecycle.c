/* Copyright 2012-2014 Neko. */

/* MACHINE controls machine status. */

#include "type.h"

#include "vm/composition/session/control.h"

#include "vm/composition/session/provider_lifecycle.h"

#include "vm/composition/session/session.h"

#include "core/product/debug/debug_target.h"

#include "core/product/wait_provider.h"

#include "core/platform/sleep.h"

#include "vm/composition/session/debug_target.h"

#include "vm/machine/debug.h"

#include "vm/profile/default_profile/firmware/qdcga.h"

#include "vm/profile/default_profile/firmware/qdkeyb.h"


#include "core/machine/machine_interface.h"

#include "vm/platform/execution.h"

#include "vm/platform/input.h"

#include "vm/platform/platform.h"


#include "vm/composition/session/display.h"


#include "vm/composition/session/lifecycle.h"

static C_VOID vm_session_wait(C_VOID *context, uint32_t milliseconds)
{
    (C_VOID)context;
    core_platform_sleep_milliseconds(milliseconds);
}

static C_VOID vm_session_keyboard_receive_key_event(C_VOID *context,
    uint16_t scan_code, uint16_t virtual_key, C_INT pressed)
{
    vm_session *machine =
        (vm_session *)context;
    vm_platform_request request;

    if (machine == STD_NULL) return;
    request.kind = VM_PLATFORM_REQUEST_KEY_EVENT;
    request.data.key_event.scan_code = scan_code;
    request.data.key_event.virtual_key = virtual_key;
    request.data.key_event.pressed = pressed;
    (C_VOID)vm_platform_request_transport_enqueue_ingress(
        &machine->request_transport, &request);
}

static C_VOID vm_session_execution_provider_refresh(C_VOID *context)
{
    vm_session_provider_lifecycle_refresh((vm_session *)context);
}

static C_VOID vm_session_execution_provider_reset(C_VOID *context)
{
    vm_session_provider_lifecycle_reset((vm_session *)context);
}

static const core_machine_execution_provider vm_session_execution_provider = {
    vm_session_execution_provider_reset,
    vm_session_execution_provider_refresh
};

C_INT vm_session_bind_execution_provider(vm_session *machine)
{
    return machine != STD_NULL && machine->core_machine != STD_NULL &&
        core_machine_bind_execution_provider(machine->core_machine,
            &vm_session_execution_provider, machine) == TYPE_STATUS_OK &&
        core_machine_freeze_execution_providers(machine->core_machine) ==
            TYPE_STATUS_OK;
}

static const vm_platform_keyboard_sink vm_session_keyboard_sink = {
    vm_session_keyboard_receive_key_event
};

static C_INT vm_session_execution_is_running(C_VOID *context)
{
    return vm_session_control_is_running(
        &((vm_session *)context)->control);
}

static C_INT vm_session_execution_get_flip(C_VOID *context)
{
    return vm_session_control_get_flip(
        &((vm_session *)context)->control);
}

static C_VOID vm_session_execution_start(C_VOID *context)
{
    vm_session_control_start(&((vm_session *)context)->control);
}

static C_VOID vm_session_execution_stop(C_VOID *context)
{
    vm_session_control_stop(&((vm_session *)context)->control);
}

static const vm_platform_execution_sink vm_session_execution_sink = {
    vm_session_execution_is_running,
    vm_session_execution_get_flip,
    vm_session_execution_start,
    vm_session_execution_stop
};

static C_VOID vm_session_debug_request_pause(C_VOID *context,
    vm_machine_debug_pause_reason reason)
{
    vm_session_control_request_pause(
        &((vm_session *)context)->control,
        reason == VM_MACHINE_DEBUG_PAUSE_TRACE ?
        VM_SESSION_PAUSE_TRACE : VM_SESSION_PAUSE_BREAKPOINT);
}

C_VOID vm_session_start(vm_session *machine) {
    vm_session_reset(machine);
    vm_session_resume(machine);
}

C_VOID vm_session_reset(vm_session *machine) {
    if (machine == STD_NULL) return;
    if (vm_platform_run_handle_is_active(&machine->platform_run_handle) &&
        !vm_session_control_is_running(&machine->control)) {
        vm_platform_run_handle_join(&machine->platform_run_handle);
        vm_platform_run_handle_finalize(&machine->platform_run_handle);
    }
    vm_session_control_reset(&machine->control);
    if (!vm_session_control_is_running(&machine->control)) {
        vm_session_publish_display(machine, 1);
    }
}

C_VOID vm_session_stop(vm_session *machine) {
    if (machine == STD_NULL) return;
    if (!vm_platform_run_handle_is_active(&machine->platform_run_handle)) {
        vm_session_control_stop(&machine->control);
        return;
    }
    vm_platform_run_handle_request_stop(&machine->platform_run_handle);
    /* Console runs synchronously in vm_session_resume(), which remains its
     * sole joiner. Window runs return here and need their async teardown. */
    if (vm_platform_run_handle_is_window_display(&machine->platform_run_handle)) {
        vm_platform_run_handle_join(&machine->platform_run_handle);
        vm_platform_run_handle_finalize(&machine->platform_run_handle);
    }
}

C_VOID vm_session_resume(vm_session *machine) {
    if (machine == STD_NULL) return;
    if (vm_session_control_is_paused(&machine->control)) {
        vm_session_control_continue(&machine->control);
    } else {
        do {
            if (vm_platform_start(&machine->platform_run_context,
                    &machine->platform_run_handle) != TYPE_STATUS_OK) return;
            if (vm_platform_run_handle_is_window_display(
                    &machine->platform_run_handle)) {
                break;
            }
            vm_platform_run_handle_join(&machine->platform_run_handle);
            vm_platform_run_handle_finalize(&machine->platform_run_handle);
        } while (vm_platform_run_context_take_auto_promotion(
            &machine->platform_run_context));
    }
}

C_VOID vm_session_initialize(vm_session *machine) {
    if (machine == STD_NULL) return;
    if (machine->active) return;
    vm_session_storage_initialize(machine);
    if (machine->core_machine == STD_NULL) return;
    core_product_wait_scope_initialize(&machine->wait_scope,
        vm_session_wait, STD_NULL);
    vm_session_control_initialize(&machine->control, machine);
    vm_session_bind_display(machine);
    vm_machine_debug_bind_pause(&machine->debug,
        vm_session_debug_request_pause, STD_NULL);
    vm_platform_keyboard_transport_initialize(&machine->keyboard_transport,
        &vm_session_keyboard_sink, machine);
    vm_platform_execution_transport_initialize(&machine->execution_transport,
        &vm_session_execution_sink, machine);
    vm_platform_run_context_initialize(&machine->platform_run_context,
        &machine->execution_transport, &machine->keyboard_transport,
        &machine->presentation_mailbox, &machine->wait_scope);
    vm_platform_run_handle_initialize(&machine->platform_run_handle);
    vm_platform_request_transport_initialize(&machine->request_transport);
    vm_platform_request_transport_bind_consumer(&machine->request_transport,
        vm_session_consume_request, machine);
    vm_session_control_bind_command_boundary(&machine->control,
        vm_platform_request_transport_observe_execution_boundary,
        &machine->request_transport);
    machine->active = 1;
}

C_VOID vm_session_finalize(vm_session *machine) {
    if (machine == STD_NULL || machine->core_machine == STD_NULL) return;
    if (vm_platform_run_handle_is_active(&machine->platform_run_handle)) {
        vm_platform_run_handle_request_stop(&machine->platform_run_handle);
        vm_platform_run_handle_join(&machine->platform_run_handle);
        vm_platform_run_handle_finalize(&machine->platform_run_handle);
    }
    vm_session_control_bind_command_boundary(&machine->control, STD_NULL, STD_NULL);
    vm_platform_request_transport_close(&machine->request_transport);
    vm_platform_request_transport_discard(&machine->request_transport);
    machine->active = 0;
    vm_session_control_finalize(&machine->control, machine);
    core_machine_display_provider_slot_finalize(&machine->display_provider);
    vm_machine_debug_bind_pause(&machine->debug, STD_NULL, STD_NULL);
    vm_session_debug_target_finalize(machine);
    vm_session_storage_finalize(machine);
}
