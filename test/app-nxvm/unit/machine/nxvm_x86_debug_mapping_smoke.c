#include "core/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "core/x86/debug_interface.h"
#include "product/debug/protocol_interface.h"
#include "core/machine/lifecycle.h"
#include "core/machine/debug_adapter.h"
#include "core/machine/machine_private.h"
#include "../support/ibmpc/machine/support/emulator_machine_fixture.h"
#include "../support/rom/session_assets.h"

static lib_i32 vm_debug_execute(vm_machine *machine,
    const emulator_machine_debug_lease *lease,
    const product_debug_request *request,
    product_debug_response *result)
{
    lib_size response_size = 0u;

    return emulator_machine_debug_execute_with_lease(machine->executor, lease,
        request, sizeof(*request), result, sizeof(*result), &response_size) ==
            LIB_STATUS_OK && response_size == sizeof(*result);
}

static lib_i32 vm_debug_wait_paused(const vm_machine *machine,
    const vm_test_emulator_machine_state_waiter *waiter)
{
    return vm_test_emulator_machine_wait_state(machine, waiter,
        EMULATOR_MACHINE_PAUSED, 2000u);
}

lib_i32 main(void)
{
    vm_machine *machine = LIB_NULL;
    vm_test_emulator_machine_state_waiter waiter = {0};
    emulator_machine_debug_lease lease;
    product_debug_response result;
    lib_u32 register_id;
    lib_u32 watch_address;
    lib_u8 watch_enabled;
    lib_u8 byte = 0x5au;
    vm_machine_debug_stop_reason stop_reason;
    lib_u64 executed;
    lib_size response_size;
    core_machine *saved_core_machine;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &machine) !=
            LIB_STATUS_OK || machine == LIB_NULL ||
        vm_test_emulator_machine_bind(machine) != LIB_STATUS_OK ||
        vm_test_emulator_machine_state_waiter_initialize(machine, &waiter) !=
            LIB_STATUS_OK) return 1;
    if (!emulator_machine_reset(machine->executor) ||
        !vm_debug_wait_paused(machine, &waiter))
        goto failed;
    /* Force a Core-originated classification through the adapter.  The
     * paused Common lease prevents concurrent Core use; restore before any
     * normal request or teardown. */
    saved_core_machine = machine->core_machine;
    machine->core_machine = LIB_NULL;
    if (core_machine_debug_read_register(machine->core_machine,
            CORE_MACHINE_DEBUG_EAX, &register_id) != LIB_STATUS_INVALID_ARGUMENT) {
        machine->core_machine = saved_core_machine;
        goto failed;
    }
    response_size = sizeof(result);
    if (vm_machine_debug_execute(machine, &(product_debug_request) {
            .operation = PRODUCT_DEBUG_READ_REGISTER,
            .register_id = CORE_MACHINE_DEBUG_EAX
        }, sizeof(product_debug_request), &result, sizeof(result), &response_size) !=
            LIB_STATUS_INVALID_ARGUMENT || response_size != 0u) {
        machine->core_machine = saved_core_machine;
        goto failed;
    }
    machine->core_machine = saved_core_machine;
    if (emulator_machine_debug_acquire(machine->executor, &lease) != LIB_STATUS_OK)
        goto failed;
    for (register_id = CORE_MACHINE_DEBUG_EAX;
         register_id < CORE_MACHINE_DEBUG_REGISTER_COUNT; ++register_id) {
        product_debug_request read = {
            .operation = PRODUCT_DEBUG_READ_REGISTER,
            .register_id = register_id
        };
        product_debug_request write = {
            .operation = PRODUCT_DEBUG_WRITE_REGISTER,
            .register_id = register_id
        };

        if (!vm_debug_execute(machine, &lease, &read, &result)) goto failed;
        write.address = result.value;
        if (!vm_debug_execute(machine, &lease, &write, &result)) goto failed;
    }
    if (!vm_debug_execute(machine, &lease, &(product_debug_request){
            .operation = PRODUCT_DEBUG_WRITE_REAL,
            .segment = 0u,
            .offset = 0x500u,
            .bytes = 1u,
            .data = { byte }
        }, &result) || !vm_debug_execute(machine, &lease,
            &(product_debug_request){
                .operation = PRODUCT_DEBUG_READ_REAL,
                .segment = 0u,
                .offset = 0x500u,
                .bytes = 1u
            }, &result) || result.data[0] != byte) goto failed;
    if (!vm_debug_execute(machine, &lease, &(product_debug_request){
            .operation = PRODUCT_DEBUG_SET_WATCH,
            .watch_kind = PRODUCT_DEBUG_WATCH_READ,
            .address = 0x600u
        }, &result) || core_machine_debug_get_watchpoint(machine->core_machine,
            CORE_MACHINE_DEBUG_WATCH_READ, &watch_enabled, &watch_address) !=
            LIB_STATUS_OK || !watch_enabled || watch_address != 0x600u)
        goto failed;
    if (!vm_debug_execute(machine, &lease, &(product_debug_request){
            .operation = PRODUCT_DEBUG_CLEAR_WATCH,
            .watch_kind = PRODUCT_DEBUG_WATCH_READ
        }, &result) || core_machine_debug_get_watchpoint(machine->core_machine,
            CORE_MACHINE_DEBUG_WATCH_READ, &watch_enabled, &watch_address) !=
            LIB_STATUS_OK || watch_enabled)
        goto failed;
    if (!vm_debug_execute(machine, &lease, &(product_debug_request){
            .operation = PRODUCT_DEBUG_SET_WATCH,
            .watch_kind = PRODUCT_DEBUG_WATCH_WRITE,
            .address = 0x700u
        }, &result) || !vm_debug_execute(machine, &lease,
            &(product_debug_request){
                .operation = PRODUCT_DEBUG_GET_WATCH,
                .watch_kind = PRODUCT_DEBUG_WATCH_WRITE
            }, &result) || !result.enabled || result.value != 0x700u)
        goto failed;
    if (!vm_debug_execute(machine, &lease, &(product_debug_request){
            .operation = PRODUCT_DEBUG_SET_EXECUTION_PLAN,
            .execution_kind = PRODUCT_DEBUG_EXECUTION_TRACE,
            .instruction_count = 5u
        }, &result) || machine->debug.plan.kind !=
            PRODUCT_DEBUG_EXECUTION_TRACE ||
        machine->debug.plan.remaining != 5u ||
        vm_machine_debug_limit_instruction_budget(&machine->debug, 256u) != 5u)
        goto failed;
    vm_machine_debug_complete_run(&machine->debug, 2u);
    if (machine->debug.plan.remaining != 3u ||
        vm_machine_debug_completion_pending(&machine->debug, &stop_reason))
        goto failed;
    vm_machine_debug_complete_run(&machine->debug, 3u);
    if (!vm_machine_debug_completion_pending(&machine->debug, &stop_reason) ||
        stop_reason != VM_MACHINE_DEBUG_STOP_TRACE ||
        !vm_machine_debug_take_completion(&machine->debug, &stop_reason,
            &executed) || stop_reason != VM_MACHINE_DEBUG_STOP_TRACE || executed != 5u)
        goto failed;
    if (!vm_debug_execute(machine, &lease, &(product_debug_request){
            .operation = PRODUCT_DEBUG_SET_EXECUTION_PLAN,
            .execution_kind = PRODUCT_DEBUG_EXECUTION_BREAK_LINEAR,
            .address = 0x1234u
        }, &result)) goto failed;
    machine->debug.observation_valid = LIB_TRUE;
    machine->debug.observation.cs_base = 0x1000u;
    machine->debug.observation.eip = 0x234u;
    vm_machine_debug_complete_run(&machine->debug, 7u);
    if (!vm_machine_debug_breakpoint_due(&machine->debug)) goto failed;
    vm_machine_debug_complete_breakpoint(&machine->debug);
    if (!vm_machine_debug_take_completion(&machine->debug, &stop_reason,
            &executed) || stop_reason != VM_MACHINE_DEBUG_STOP_BREAKPOINT ||
        executed != 7u) goto failed;
    if (!vm_debug_execute(machine, &lease, &(product_debug_request){
            .operation = PRODUCT_DEBUG_CLEAR_EXECUTION_PLAN
        }, &result) || machine->debug.plan.kind !=
            PRODUCT_DEBUG_EXECUTION_NONE) goto failed;
    machine->debug.observation_valid = LIB_TRUE;
    machine->debug.observation.memory_access_count = 1u;
    machine->debug.observation.memory_accesses[0] =
        (core_machine_debug_memory_access) {
            .write = LIB_TRUE,
            .linear = 0x4567u,
            .bytes = 1u,
            .data = 0x5au
        };
    machine->debug.observation.watch_hit = LIB_TRUE;
    machine->debug.observation.watch_kind = CORE_MACHINE_DEBUG_WATCH_WRITE;
    machine->debug.observation.watch_address = 0x4567u;
    vm_machine_debug_complete_watchpoint(&machine->debug);
    if (!vm_debug_execute(machine, &lease, &(product_debug_request){
            .operation = PRODUCT_DEBUG_GET_EXECUTION_RESULT
        }, &result) || !result.enabled || !result.observation.watch_hit ||
        result.observation.watch_kind != PRODUCT_DEBUG_WATCH_WRITE ||
        result.observation.watch_address != 0x4567u ||
        result.observation.count != 1u || !result.observation.accesses[0].write ||
        result.observation.accesses[0].linear != 0x4567u ||
        result.observation.accesses[0].data != 0x5au) goto failed;
    if (!emulator_machine_stop(machine->executor)) goto failed;
    if (emulator_machine_debug_execute_with_lease(machine->executor, &lease,
            &(product_debug_request){
                .operation = PRODUCT_DEBUG_READ_REGISTER,
                .register_id = CORE_MACHINE_DEBUG_EIP
            }, sizeof(product_debug_request), &result, sizeof(result),
            &(lib_size){0u}) != LIB_STATUS_INVALID_STATE) goto failed;
    vm_test_emulator_machine_unbind(machine);
    vm_test_emulator_machine_state_waiter_finalize(&waiter);
    vm_machine_destroy(machine);
    puts("M5:T531:S27:VM-X86-DEBUG-MAPPING:OK");
    return 0;

failed:
    vm_test_emulator_machine_unbind(machine);
    vm_test_emulator_machine_state_waiter_finalize(&waiter);
    vm_machine_destroy(machine);
    return 1;
}
