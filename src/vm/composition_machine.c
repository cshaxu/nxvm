/* Copyright 2012-2014 Neko. */

/* MACHINE controls machine status. */

#include "vm/composition_control.h"
#include "vm/composition_live_machine.h"
#include "core/product/debug/debug_target.h"
#include "core/product/wait_provider.h"
#include "core/platform/sleep.h"
#include "vm/composition_debug.h"
#include "vm/machine/debug.h"
#include "vm/profile/default_profile/firmware/qdcga.h"
#include "vm/profile/default_profile/firmware/qdkeyb.h"
#include "core/machine/keyboard_interface.h"
#include "vm/platform/execution.h"
#include "vm/platform/input.h"
#include "vm/platform/platform.h"

#include "vm/composition_display.h"

#include "vm/composition_machine.h"

static void vm_composition_wait(void *context, uint32_t milliseconds)
{
    (void)context;
    core_platform_sleep_milliseconds(milliseconds);
}

static int vm_composition_keyboard_get_modifier(
    void *context, vm_platform_keyboard_modifier modifier)
{
    (void)context;
    switch (modifier) {
    case VM_PLATFORM_KEYBOARD_MODIFIER_ALT:
        return core_machine_keyboard_get_modifier(CORE_MACHINE_KEYBOARD_MODIFIER_ALT);
    case VM_PLATFORM_KEYBOARD_MODIFIER_CONTROL:
        return core_machine_keyboard_get_modifier(CORE_MACHINE_KEYBOARD_MODIFIER_CONTROL);
    case VM_PLATFORM_KEYBOARD_MODIFIER_SHIFT:
        return core_machine_keyboard_get_modifier(CORE_MACHINE_KEYBOARD_MODIFIER_SHIFT);
    case VM_PLATFORM_KEYBOARD_MODIFIER_CAPS_LOCK:
        return core_machine_keyboard_get_modifier(CORE_MACHINE_KEYBOARD_MODIFIER_CAPS_LOCK);
    case VM_PLATFORM_KEYBOARD_MODIFIER_NUM_LOCK:
        return core_machine_keyboard_get_modifier(CORE_MACHINE_KEYBOARD_MODIFIER_NUM_LOCK);
    }
    return 0;
}

static void vm_composition_keyboard_apply_host_state(
    void *context, uint32_t asynchronous_keys, uint32_t toggle_keys)
{
    (void)context;
    core_machine_keyboard_apply_host_state(asynchronous_keys, toggle_keys);
}

static void vm_composition_keyboard_receive_key_press(void *context, uint16_t code)
{
    (void)context;
    core_machine_keyboard_receive_key_press(code);
}

static void vm_composition_keyboard_request_stop(void *context)
{
    vm_composition_control_stop(((vm_composition_live_machine *)context)->control);
}

static const vm_platform_keyboard_sink vm_composition_keyboard_sink = {
    vm_composition_keyboard_get_modifier,
    vm_composition_keyboard_apply_host_state,
    vm_composition_keyboard_receive_key_press,
    vm_composition_keyboard_request_stop
};

static int vm_composition_execution_is_running(void *context)
{
    return vm_composition_control_is_running(
        ((vm_composition_live_machine *)context)->control);
}

static int vm_composition_execution_get_flip(void *context)
{
    return vm_composition_control_get_flip(
        ((vm_composition_live_machine *)context)->control);
}

static void vm_composition_execution_start(void *context)
{
    vm_composition_control_start(((vm_composition_live_machine *)context)->control);
}

static void vm_composition_execution_stop(void *context)
{
    vm_composition_control_stop(((vm_composition_live_machine *)context)->control);
}

static const vm_platform_execution_sink vm_composition_execution_sink = {
    vm_composition_execution_is_running,
    vm_composition_execution_get_flip,
    vm_composition_execution_start,
    vm_composition_execution_stop
};

static void vm_composition_debug_request_pause(void *context,
    vm_machine_debug_pause_reason reason)
{
    vm_composition_control_request_pause(
        ((vm_composition_live_machine *)context)->control,
        reason == VM_MACHINE_DEBUG_PAUSE_TRACE ?
        VM_COMPOSITION_PAUSE_TRACE : VM_COMPOSITION_PAUSE_BREAKPOINT);
}

void machineStart(vm_composition_live_machine *machine) {
    machineReset(machine);
    machineResume(machine);
}

void machineReset(vm_composition_live_machine *machine) {
    if (machine == NULL) return;
    vm_composition_control_reset(machine->control);
    if (!vm_composition_control_is_running(machine->control)) vm_composition_publish_display(1);
}

void machineStop(vm_composition_live_machine *machine) {
    if (machine == NULL) return;
    vm_composition_control_stop(machine->control);
}

void machineResume(vm_composition_live_machine *machine) {
    if (machine == NULL) return;
    if (vm_composition_control_is_paused(machine->control)) {
        vm_composition_control_continue(machine->control);
    } else {
        platformStart();
    }
}

void machineInit(vm_composition_live_machine *machine) {
    if (machine == NULL) return;
    platformInit();
    core_product_wait_bind(vm_composition_wait, NULL);
    vm_composition_live_machine_initialize(machine);
    vm_composition_live_machine_bind_legacy(machine);
    vm_composition_control_initialize(machine->control, machine);
    core_machine_keyboard_bind(NULL, vm_profile_default_keyboard_provider());
    core_machine_display_bind_snapshot_provider(NULL,
        vm_profile_default_display_capture);
    vm_machine_debug_bind_pause(vm_composition_debug_request_pause, NULL);
    core_product_debug_bind_target(vm_composition_debug_target(machine));
    vm_platform_keyboard_bind(&vm_composition_keyboard_sink, machine);
    vm_platform_execution_bind(&vm_composition_execution_sink, machine);
}

void machineFinal(vm_composition_live_machine *machine) {
    if (machine == NULL) return;
    vm_composition_control_finalize(machine->control, machine);
    core_machine_keyboard_bind(NULL, NULL);
    core_machine_display_bind_snapshot_provider(NULL, NULL);
    vm_machine_debug_bind_pause(NULL, NULL);
    core_product_debug_bind_target(NULL);
    vm_composition_debug_target_finalize(machine);
    vm_platform_execution_bind(NULL, NULL);
    vm_platform_keyboard_bind(NULL, NULL);
    core_product_wait_bind(NULL, NULL);
    vm_composition_live_machine_finalize(machine);
    platformFinal();
}
