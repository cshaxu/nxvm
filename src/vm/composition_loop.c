/* Copyright 2012-2014 Neko. */

/*
 * DEVICE provides the device interface to drive the device thread
 * and access the virtual devices.
 */

#include "core/product/utils.h"

#include "vm/machine/vdebug.h"
#include "vm/composition.h"
#include "core/product/runtime/execution_context.h"
#include "core/machine/vcpu.h"
#include "core/machine/vram.h"
#include "core/machine/vport.h"
#include "core/machine/keyboard_state.h"

#include "vm/machine/device.h"

t_device device;
static nxvm_execution_context device_execution_context;

static void device_execution_context_reset(void)
{
    vdebugReset();
    vmachineReset();
}

static const nxvm_execution_context_callbacks device_execution_callbacks = {
    device_execution_context_reset,
    vdebugRefresh,
    vmachineRefresh
};

/* Starts device thread */
void deviceStart() {
    nxvm_execution_context_enter(&device_execution_context);
    device.flagRun = True;
    device.flagFlip = !device.flagFlip;
    while (device.flagRun) {
        if (device.flagReset) {
            nxvm_execution_context_reset(&device_execution_context);
            device.flagReset = False;
        }
        nxvm_execution_context_run_command_boundary(&device_execution_context);
        nxvm_execution_context_debug_refresh(&device_execution_context);
        if (!device.flagRun) {
            break;
        }
        nxvm_execution_context_machine_refresh(&device_execution_context);
        if (vcpuConsumeResetRequest()) {
            deviceReset();
        }
        if (vcpuConsumeStopRequest()) {
            deviceStop();
        }
    }
    nxvm_execution_context_leave(&device_execution_context);
}

/* Issues resetting signal to device thread */
void deviceReset() {
    if (device.flagRun) {
        device.flagReset = True;
    } else {
        nxvm_execution_context_reset(&device_execution_context);
        device.flagReset = False;
    }
}

/* Issues stopping signal to device thread */
void deviceStop()  {
    device.flagRun = False;
}

void deviceConnectBindCommandBoundary(
    void (*callback)(void *opaque), void *opaque)
{
    nxvm_execution_context_bind_command_boundary(
        &device_execution_context, callback, opaque);
}

/* Initializes devices */
void deviceInit() {
    MEMSET((void *)(&device), Zero8, sizeof(t_device));
    nxvm_execution_context_initialize(&device_execution_context);
    nxvm_execution_context_bind_machine_state(
        &device_execution_context, &vcpu, &vram, &vport, &device);
    nxvm_execution_context_bind_callbacks(
        &device_execution_context, &device_execution_callbacks);
    nxvm_execution_context_enter(&device_execution_context);
    vdebugInit();
    vmachineInit();
}

/* Finalizes devices */
void deviceFinal() {
    nxvm_execution_context_leave(&device_execution_context);
    vdebugFinal();
    vmachineFinal();
}

void devicePrintStatus() {
    PRINTF("Recording: %s\n", vdebug.connect.recordFile ? "Yes" : "No");
    PRINTF("Running:   %s\n", device.flagRun  ? "Yes" : "No");
}

void deviceConnectKeyboardApplyHostState(uint32_t asynchronous_keys,
                                         uint32_t toggle_keys)
{
#define NXVM_SET_KEYBOARD_FLAG(mask, set_call, clear_call) \
    do { \
        if ((mask) != 0u) set_call(); \
        else clear_call(); \
    } while (0)
    NXVM_SET_KEYBOARD_FLAG(asynchronous_keys & NXVM_KEYBOARD_ASYNC_RIGHT_SHIFT,
                           deviceConnectKeyboardSetFlag0RightShift,
                           deviceConnectKeyboardClrFlag0RightShift);
    NXVM_SET_KEYBOARD_FLAG(asynchronous_keys & NXVM_KEYBOARD_ASYNC_LEFT_SHIFT,
                           deviceConnectKeyboardSetFlag0LeftShift,
                           deviceConnectKeyboardClrFlag0LeftShift);
    NXVM_SET_KEYBOARD_FLAG(asynchronous_keys & NXVM_KEYBOARD_ASYNC_CONTROL,
                           deviceConnectKeyboardSetFlag0Ctrl,
                           deviceConnectKeyboardClrFlag0Ctrl);
    NXVM_SET_KEYBOARD_FLAG(asynchronous_keys & NXVM_KEYBOARD_ASYNC_ALT,
                           deviceConnectKeyboardSetFlag0Alt,
                           deviceConnectKeyboardClrFlag0Alt);
    NXVM_SET_KEYBOARD_FLAG(asynchronous_keys & NXVM_KEYBOARD_ASYNC_SCROLL_LOCK,
                           deviceConnectKeyboardSetFlag1ScrLck,
                           deviceConnectKeyboardClrFlag1ScrLck);
    NXVM_SET_KEYBOARD_FLAG(asynchronous_keys & NXVM_KEYBOARD_ASYNC_NUM_LOCK,
                           deviceConnectKeyboardSetFlag1NumLck,
                           deviceConnectKeyboardClrFlag1NumLck);
    NXVM_SET_KEYBOARD_FLAG(asynchronous_keys & NXVM_KEYBOARD_ASYNC_CAPS_LOCK,
                           deviceConnectKeyboardSetFlag1CapLck,
                           deviceConnectKeyboardClrFlag1CapLck);
    NXVM_SET_KEYBOARD_FLAG(asynchronous_keys & NXVM_KEYBOARD_ASYNC_INSERT,
                           deviceConnectKeyboardSetFlag1Insert,
                           deviceConnectKeyboardClrFlag1Insert);
    NXVM_SET_KEYBOARD_FLAG(toggle_keys & NXVM_KEYBOARD_TOGGLE_SCROLL_LOCK,
                           deviceConnectKeyboardSetFlag0ScrLck,
                           deviceConnectKeyboardClrFlag0ScrLck);
    NXVM_SET_KEYBOARD_FLAG(toggle_keys & NXVM_KEYBOARD_TOGGLE_NUM_LOCK,
                           deviceConnectKeyboardSetFlag0NumLck,
                           deviceConnectKeyboardClrFlag0NumLck);
    NXVM_SET_KEYBOARD_FLAG(toggle_keys & NXVM_KEYBOARD_TOGGLE_CAPS_LOCK,
                           deviceConnectKeyboardSetFlag0CapLck,
                           deviceConnectKeyboardClrFlag0CapLck);
    NXVM_SET_KEYBOARD_FLAG(toggle_keys & NXVM_KEYBOARD_TOGGLE_INSERT,
                           deviceConnectKeyboardSetFlag0Insert,
                           deviceConnectKeyboardClrFlag0Insert);
    NXVM_SET_KEYBOARD_FLAG(toggle_keys & NXVM_KEYBOARD_TOGGLE_PAUSE,
                           deviceConnectKeyboardSetFlag1Pause,
                           deviceConnectKeyboardClrFlag1Pause);
#undef NXVM_SET_KEYBOARD_FLAG
}
