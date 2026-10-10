#include "core/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#ifndef TEST_VM_EMULATOR_MACHINE_FIXTURE_H
#define TEST_VM_EMULATOR_MACHINE_FIXTURE_H

#include "core/machine/machine_private.h"
#include "core/machine/machine_interface.h"
#include "lib/base/clock_interface.h"
#include "lib/base/sync_interface.h"

typedef struct vm_test_emulator_machine_state_waiter {
    base_sync_event *event;
} vm_test_emulator_machine_state_waiter;

static inline void vm_test_emulator_machine_state_changed(void *context,
    emulator_machine_state state, lib_u32 run_generation)
{
    vm_test_emulator_machine_state_waiter *waiter = context;

    (void)state;
    (void)run_generation;
    if (waiter != LIB_NULL) (void)base_sync_event_signal(waiter->event);
}

static inline lib_status vm_test_emulator_machine_state_waiter_initialize(
    vm_machine *machine, vm_test_emulator_machine_state_waiter *waiter)
{
    if (machine == LIB_NULL || machine->executor == LIB_NULL || waiter == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *waiter = (vm_test_emulator_machine_state_waiter){0};
    if (base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, &waiter->event) !=
            LIB_STATUS_OK) return LIB_STATUS_INTERNAL_ERROR;
    emulator_machine_set_state_sink(machine->executor,
        vm_test_emulator_machine_state_changed, waiter);
    return LIB_STATUS_OK;
}

static inline void vm_test_emulator_machine_state_waiter_finalize(
    vm_test_emulator_machine_state_waiter *waiter)
{
    if (waiter == LIB_NULL) return;
    base_sync_event_destroy(waiter->event);
    waiter->event = LIB_NULL;
}

static inline lib_u8 vm_test_emulator_machine_wait_state(const vm_machine *machine,
    const vm_test_emulator_machine_state_waiter *waiter,
    emulator_machine_state expected, lib_u32 timeout_milliseconds)
{
    lib_u64 now;
    lib_u64 deadline;

    if (machine == LIB_NULL || machine->executor == LIB_NULL ||
        waiter == LIB_NULL || waiter->event == LIB_NULL ||
        base_clock_milliseconds(&now) != LIB_STATUS_OK) return LIB_FALSE;
    deadline = now + timeout_milliseconds;
    if (deadline < now) return LIB_FALSE;
    for (;;) {
        lib_u64 remaining;
        base_sync_wait_result result;

        if (emulator_machine_state_get(machine->executor) == expected)
            return LIB_TRUE;
        if (base_clock_milliseconds(&now) != LIB_STATUS_OK || now >= deadline)
            return LIB_FALSE;
        remaining = deadline - now;
        result = base_sync_event_wait(waiter->event,
            remaining > LIB_UINT32_MAX ? LIB_UINT32_MAX : (lib_u32)remaining);
        if (result != BASE_SYNC_WAIT_SIGNALED && result != BASE_SYNC_WAIT_TIMED_OUT)
            return LIB_FALSE;
    }
}

static lib_status vm_test_emulator_machine_bind(vm_machine *machine)
{
    emulator_machine_driver driver;
    emulator_machine *emulator_machine = LIB_NULL;

    if (vm_machine_describe_emulator_driver(machine, &driver) != LIB_STATUS_OK ||
        emulator_machine_create(&emulator_machine, &driver) != LIB_STATUS_OK ||
        vm_machine_bind_emulator_machine(machine, emulator_machine) != LIB_STATUS_OK) {
        emulator_machine_destroy(emulator_machine);
        return LIB_STATUS_INVALID_STATE;
    }
    return LIB_STATUS_OK;
}

static void vm_test_emulator_machine_unbind(vm_machine *machine)
{
    emulator_machine *emulator_machine;

    if (machine == LIB_NULL) return;
    emulator_machine = machine->executor;
    emulator_machine_destroy(emulator_machine);
    (void)vm_machine_bind_emulator_machine(machine, LIB_NULL);
}

#endif
