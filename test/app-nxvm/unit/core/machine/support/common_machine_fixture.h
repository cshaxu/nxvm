#ifndef TEST_VM_COMMON_MACHINE_FIXTURE_H
#define TEST_VM_COMMON_MACHINE_FIXTURE_H

#include "app-nxvm/machine/machine_private.h"
#include "app-nxvm/machine/machine_interface.h"
#include "lib/base/clock_interface.h"
#include "lib/base/sync_interface.h"

typedef struct vm_test_common_machine_state_waiter {
    base_sync_event *event;
} vm_test_common_machine_state_waiter;

static inline C_VOID vm_test_common_machine_state_changed(C_VOID *context,
    common_machine_state state, lib_u32 run_generation)
{
    vm_test_common_machine_state_waiter *waiter = context;

    (C_VOID)state;
    (C_VOID)run_generation;
    if (waiter != STD_NULL) (C_VOID)base_sync_event_signal(waiter->event);
}

static inline type_status vm_test_common_machine_state_waiter_initialize(
    vm_machine *machine, vm_test_common_machine_state_waiter *waiter)
{
    if (machine == STD_NULL || machine->executor == LIB_NULL || waiter == STD_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    *waiter = (vm_test_common_machine_state_waiter){0};
    if (base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, &waiter->event) !=
            LIB_STATUS_OK) return TYPE_STATUS_FAULT;
    common_machine_set_state_sink(machine->executor,
        vm_test_common_machine_state_changed, waiter);
    return TYPE_STATUS_OK;
}

static inline C_VOID vm_test_common_machine_state_waiter_finalize(
    vm_test_common_machine_state_waiter *waiter)
{
    if (waiter == STD_NULL) return;
    base_sync_event_destroy(waiter->event);
    waiter->event = LIB_NULL;
}

static inline type_bool vm_test_common_machine_wait_state(const vm_machine *machine,
    const vm_test_common_machine_state_waiter *waiter,
    common_machine_state expected, lib_u32 timeout_milliseconds)
{
    lib_u64 now;
    lib_u64 deadline;

    if (machine == STD_NULL || machine->executor == LIB_NULL ||
        waiter == STD_NULL || waiter->event == LIB_NULL ||
        base_clock_milliseconds(&now) != LIB_STATUS_OK) return TYPE_FALSE;
    deadline = now + timeout_milliseconds;
    if (deadline < now) return TYPE_FALSE;
    for (;;) {
        lib_u64 remaining;
        base_sync_wait_result result;

        if (common_machine_state_get(machine->executor) == expected)
            return TYPE_TRUE;
        if (base_clock_milliseconds(&now) != LIB_STATUS_OK || now >= deadline)
            return TYPE_FALSE;
        remaining = deadline - now;
        result = base_sync_event_wait(waiter->event,
            remaining > LIB_UINT32_MAX ? LIB_UINT32_MAX : (lib_u32)remaining);
        if (result != BASE_SYNC_WAIT_SIGNALED && result != BASE_SYNC_WAIT_TIMED_OUT)
            return TYPE_FALSE;
    }
}

static type_status vm_test_common_machine_bind(vm_machine *machine)
{
    common_machine_driver driver;
    common_machine *common_machine = LIB_NULL;

    if (vm_machine_describe_common_driver(machine, &driver) != TYPE_STATUS_OK ||
        common_machine_create(&common_machine, &driver) != LIB_STATUS_OK ||
        vm_machine_bind_common_machine(machine, common_machine) != TYPE_STATUS_OK) {
        common_machine_destroy(common_machine);
        return TYPE_STATUS_INVALID_STATE;
    }
    return TYPE_STATUS_OK;
}

static C_VOID vm_test_common_machine_unbind(vm_machine *machine)
{
    common_machine *common_machine;

    if (machine == STD_NULL) return;
    common_machine = machine->executor;
    common_machine_destroy(common_machine);
    (C_VOID)vm_machine_bind_common_machine(machine, LIB_NULL);
}

#endif
