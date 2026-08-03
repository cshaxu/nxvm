/* Copyright 2012-2014 Neko. */

/* MACHINE controls machine status. */

#include "type.h"

#include "vm/composition_control.h"

#include "vm/composition.h"

#include "vm/composition_live_machine.h"

#include "core/product/debug/debug_target.h"

#include "core/product/wait_provider.h"

#include "core/platform/sleep.h"

#include "vm/composition_debug.h"

#include "vm/machine/debug.h"

#include "vm/profile/default_profile/firmware/qdcga.h"

#include "vm/profile/default_profile/firmware/qdkeyb.h"

#include "core/machine/keyboard_interface.h"

#include "core/machine/machine.h"

#include "vm/platform/execution.h"

#include "vm/platform/input.h"

#include "vm/platform/platform.h"


#include "vm/composition_display.h"


#include "vm/composition_machine.h"

static C_VOID vm_composition_wait(C_VOID *context, uint32_t milliseconds)
{
    (C_VOID)context;
    core_platform_sleep_milliseconds(milliseconds);
}

static C_INT vm_composition_keyboard_get_modifier(
    C_VOID *context, vm_platform_keyboard_modifier modifier)
{
    vm_composition_live_machine *machine =
        (vm_composition_live_machine *)context;
    switch (modifier) {
    case VM_PLATFORM_KEYBOARD_MODIFIER_ALT:
        return core_machine_keyboard_get_modifier_from(machine->keyboard_provider,
            CORE_MACHINE_KEYBOARD_MODIFIER_ALT);
    case VM_PLATFORM_KEYBOARD_MODIFIER_CONTROL:
        return core_machine_keyboard_get_modifier_from(machine->keyboard_provider,
            CORE_MACHINE_KEYBOARD_MODIFIER_CONTROL);
    case VM_PLATFORM_KEYBOARD_MODIFIER_SHIFT:
        return core_machine_keyboard_get_modifier_from(machine->keyboard_provider,
            CORE_MACHINE_KEYBOARD_MODIFIER_SHIFT);
    case VM_PLATFORM_KEYBOARD_MODIFIER_CAPS_LOCK:
        return core_machine_keyboard_get_modifier_from(machine->keyboard_provider,
            CORE_MACHINE_KEYBOARD_MODIFIER_CAPS_LOCK);
    case VM_PLATFORM_KEYBOARD_MODIFIER_NUM_LOCK:
        return core_machine_keyboard_get_modifier_from(machine->keyboard_provider,
            CORE_MACHINE_KEYBOARD_MODIFIER_NUM_LOCK);
    }
    return 0;
}

static C_VOID vm_composition_keyboard_apply_host_state(
    C_VOID *context, uint32_t asynchronous_keys, uint32_t toggle_keys)
{
    vm_composition_live_machine *machine =
        (vm_composition_live_machine *)context;
    core_machine_keyboard_apply_host_state_to(machine->keyboard_provider,
        asynchronous_keys, toggle_keys);
}

static C_VOID vm_composition_keyboard_receive_key_press(C_VOID *context, uint16_t code)
{
    vm_composition_live_machine *machine =
        (vm_composition_live_machine *)context;
    core_machine_keyboard_receive_key_press_to(machine->keyboard_provider, code);
}

static C_VOID vm_composition_keyboard_request_stop(C_VOID *context)
{
    vm_composition_control_stop(((vm_composition_live_machine *)context)->control);
}

static C_VOID vm_composition_execution_provider_refresh(C_VOID *context)
{
    vm_composition_providers_refresh((vm_composition_live_machine *)context);
}

static C_VOID vm_composition_execution_provider_reset(C_VOID *context)
{
    vm_composition_providers_reset((vm_composition_live_machine *)context);
}

static const core_machine_execution_provider vm_composition_execution_provider = {
    vm_composition_execution_provider_reset,
    vm_composition_execution_provider_refresh
};

C_INT vm_composition_bind_execution_provider(vm_composition_live_machine *machine)
{
    return machine != NULL && machine->core_machine != NULL &&
        core_machine_bind_execution_provider(machine->core_machine,
            &vm_composition_execution_provider, machine) == NTVDM64_STATUS_OK &&
        core_machine_freeze_execution_providers(machine->core_machine) ==
            NTVDM64_STATUS_OK;
}

static const vm_platform_keyboard_sink vm_composition_keyboard_sink = {
    vm_composition_keyboard_get_modifier,
    vm_composition_keyboard_apply_host_state,
    vm_composition_keyboard_receive_key_press,
    vm_composition_keyboard_request_stop
};

static C_INT vm_composition_execution_is_running(C_VOID *context)
{
    return vm_composition_control_is_running(
        ((vm_composition_live_machine *)context)->control);
}

static C_INT vm_composition_execution_get_flip(C_VOID *context)
{
    return vm_composition_control_get_flip(
        ((vm_composition_live_machine *)context)->control);
}

static C_VOID vm_composition_execution_start(C_VOID *context)
{
    vm_composition_control_start(((vm_composition_live_machine *)context)->control);
}

static C_VOID vm_composition_execution_stop(C_VOID *context)
{
    vm_composition_control_stop(((vm_composition_live_machine *)context)->control);
}

static const vm_platform_execution_sink vm_composition_execution_sink = {
    vm_composition_execution_is_running,
    vm_composition_execution_get_flip,
    vm_composition_execution_start,
    vm_composition_execution_stop
};

static C_VOID vm_composition_debug_request_pause(C_VOID *context,
    vm_machine_debug_pause_reason reason)
{
    vm_composition_control_request_pause(
        ((vm_composition_live_machine *)context)->control,
        reason == VM_MACHINE_DEBUG_PAUSE_TRACE ?
        VM_COMPOSITION_PAUSE_TRACE : VM_COMPOSITION_PAUSE_BREAKPOINT);
}

C_VOID vm_composition_start(vm_composition_live_machine *machine) {
    vm_composition_reset(machine);
    vm_composition_resume(machine);
}

C_VOID vm_composition_reset(vm_composition_live_machine *machine) {
    if (machine == NULL) return;
    vm_composition_control_reset(machine->control);
    if (!vm_composition_control_is_running(machine->control)) {
        vm_composition_publish_display(machine, 1);
    }
}

C_VOID vm_composition_stop(vm_composition_live_machine *machine) {
    if (machine == NULL) return;
    vm_composition_control_stop(machine->control);
}

C_VOID vm_composition_resume(vm_composition_live_machine *machine) {
    if (machine == NULL) return;
    if (vm_composition_control_is_paused(machine->control)) {
        vm_composition_control_continue(machine->control);
    } else {
        vm_platform_start(machine->platform_run_context);
    }
}

C_VOID vm_composition_initialize(vm_composition_live_machine *machine) {
    if (machine == NULL) return;
    vm_platform_initialize();
    vm_composition_live_machine_initialize(machine);
    if (machine->core_machine == NULL) return;
    core_product_wait_scope_initialize(machine->wait_scope,
        vm_composition_wait, NULL);
    vm_composition_control_initialize(machine->control, machine);
    core_machine_keyboard_provider_slot_bind(machine->keyboard_provider,
        machine->default_profile_context,
        vm_profile_default_keyboard_provider());
    core_machine_keyboard_provider_slot_freeze(machine->keyboard_provider);
    vm_composition_bind_display(machine);
    vm_machine_debug_bind_pause(machine->debug,
        vm_composition_debug_request_pause, NULL);
    vm_platform_keyboard_transport_initialize(machine->keyboard_transport,
        &vm_composition_keyboard_sink, machine);
    vm_platform_execution_transport_initialize(machine->execution_transport,
        &vm_composition_execution_sink, machine);
    vm_platform_run_context_initialize(machine->platform_run_context,
        machine->execution_transport, machine->keyboard_transport,
        machine->presentation_mailbox, machine->wait_scope);
}

C_VOID vm_composition_finalize(vm_composition_live_machine *machine) {
    if (machine == NULL || machine->core_machine == NULL) return;
    vm_composition_control_finalize(machine->control, machine);
    core_machine_keyboard_provider_slot_finalize(machine->keyboard_provider);
    core_machine_display_provider_slot_finalize(machine->display_provider);
    vm_machine_debug_bind_pause(machine->debug, NULL, NULL);
    vm_composition_debug_target_finalize(machine);
    vm_composition_live_machine_finalize(machine);
    vm_platform_finalize();
}
