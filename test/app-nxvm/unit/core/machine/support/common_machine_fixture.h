#include "lib/types/types_interface.h"
#ifndef TEST_VM_COMMON_MACHINE_FIXTURE_H
#define TEST_VM_COMMON_MACHINE_FIXTURE_H

#include "app-nxvm/machine/machine_private.h"
#include "app-nxvm/machine/machine_interface.h"
#include "lib/base/clock_interface.h"
#include "lib/base/sync_interface.h"

typedef struct vm_test_common_machine_state_waiter {
    base_sync_event *event;
} vm_test_common_machine_state_waiter;

static inline void vm_test_common_machine_state_changed(void *context,
    common_machine_state state, lib_u32 run_generation)
{
    vm_test_common_machine_state_waiter *waiter = context;

    (void)state;
    (void)run_generation;
    if (waiter != LIB_NULL) (void)base_sync_event_signal(waiter->event);
}

static inline lib_status vm_test_common_machine_state_waiter_initialize(
    vm_machine *machine, vm_test_common_machine_state_waiter *waiter)
{
    if (machine == LIB_NULL || machine->executor == LIB_NULL || waiter == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *waiter = (vm_test_common_machine_state_waiter){0};
    if (base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, &waiter->event) !=
            LIB_STATUS_OK) return LIB_STATUS_INTERNAL_ERROR;
    common_machine_set_state_sink(machine->executor,
        vm_test_common_machine_state_changed, waiter);
    return LIB_STATUS_OK;
}

static inline void vm_test_common_machine_state_waiter_finalize(
    vm_test_common_machine_state_waiter *waiter)
{
    if (waiter == LIB_NULL) return;
    base_sync_event_destroy(waiter->event);
    waiter->event = LIB_NULL;
}

static inline lib_u8 vm_test_common_machine_wait_state(const vm_machine *machine,
    const vm_test_common_machine_state_waiter *waiter,
    common_machine_state expected, lib_u32 timeout_milliseconds)
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

        if (common_machine_state_get(machine->executor) == expected)
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

static lib_status vm_test_common_machine_bind(vm_machine *machine)
{
    common_machine_driver driver;
    common_machine *common_machine = LIB_NULL;

    if (vm_machine_describe_common_driver(machine, &driver) != LIB_STATUS_OK ||
        common_machine_create(&common_machine, &driver) != LIB_STATUS_OK ||
        vm_machine_bind_common_machine(machine, common_machine) != LIB_STATUS_OK) {
        common_machine_destroy(common_machine);
        return LIB_STATUS_INVALID_STATE;
    }
    return LIB_STATUS_OK;
}

static void vm_test_common_machine_unbind(vm_machine *machine)
{
    common_machine *common_machine;

    if (machine == LIB_NULL) return;
    common_machine = machine->executor;
    common_machine_destroy(common_machine);
    (void)vm_machine_bind_common_machine(machine, LIB_NULL);
}

#endif
