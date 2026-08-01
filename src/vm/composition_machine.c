/* Copyright 2012-2014 Neko. */

/* MACHINE controls machine status. */

#include "vm/machine/device.h"
#include "core/product/wait.h"
#include "vm/platform/input.h"
#include "vm/platform/platform.h"

#include "vm/composition_display.h"

#include "vm/composition_machine.h"

static void vm_composition_wait(void *context, uint32_t milliseconds)
{
    (void)context;
    platformSleep(milliseconds);
}

static int vm_composition_keyboard_get_modifier(
    void *context, vm_platform_keyboard_modifier modifier)
{
    (void)context;
    switch (modifier) {
    case VM_PLATFORM_KEYBOARD_MODIFIER_ALT:
        return deviceConnectKeyboardGetFlag0Alt();
    case VM_PLATFORM_KEYBOARD_MODIFIER_CONTROL:
        return deviceConnectKeyboardGetFlag0Ctrl();
    case VM_PLATFORM_KEYBOARD_MODIFIER_SHIFT:
        return deviceConnectKeyboardGetFlag0Shift();
    case VM_PLATFORM_KEYBOARD_MODIFIER_CAPS_LOCK:
        return deviceConnectKeyboardGetFlag0CapsLock();
    case VM_PLATFORM_KEYBOARD_MODIFIER_NUM_LOCK:
        return deviceConnectKeyboardGetFlag0NumLock();
    }
    return 0;
}

static void vm_composition_keyboard_apply_host_state(
    void *context, uint32_t asynchronous_keys, uint32_t toggle_keys)
{
    (void)context;
    deviceConnectKeyboardApplyHostState(asynchronous_keys, toggle_keys);
}

static void vm_composition_keyboard_receive_key_press(void *context, uint16_t code)
{
    (void)context;
    deviceConnectKeyboardRecvKeyPress(code);
}

static void vm_composition_keyboard_request_stop(void *context)
{
    (void)context;
    deviceStop();
}

static const vm_platform_keyboard_sink vm_composition_keyboard_sink = {
    vm_composition_keyboard_get_modifier,
    vm_composition_keyboard_apply_host_state,
    vm_composition_keyboard_receive_key_press,
    vm_composition_keyboard_request_stop
};

void machineStart() {
    machineReset();
    machineResume();
}

void machineReset() {
    deviceReset();
    if (!device.flagRun) vm_composition_publish_display(1);
}

void machineStop() {
    deviceStop();
}

void machineResume() {
    platformStart();
}

void machineInit() {
    platformInit();
    core_product_wait_bind(vm_composition_wait, NULL);
    deviceInit();
    vm_platform_keyboard_bind(&vm_composition_keyboard_sink, NULL);
}

void machineFinal() {
    deviceFinal();
    vm_platform_keyboard_bind(NULL, NULL);
    core_product_wait_bind(NULL, NULL);
    platformFinal();
}
