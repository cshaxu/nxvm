#include "type.h"

#include "core/machine/machine.h"
#include "vm/machine/runtime/lifecycle.h"
#include "vm/machine/runtime/machine_private.h"
#include "../support/rom/session_assets.h"

static C_INT vm_debug_execute(vm_machine *machine,
    const common_machine_debug_lease *lease,
    const common_machine_debug_request *request,
    common_machine_debug_result *result)
{
    return common_machine_debug_execute_with_lease(machine->executor, lease,
        request, result) == LIB_STATUS_OK;
}

C_INT main(C_VOID)
{
    vm_machine *machine = STD_NULL;
    common_machine_debug_lease lease;
    common_machine_debug_result result;
    type_unsigned_32 register_id;
    type_unsigned_8 byte = 0x5au;
    vm_machine_pause_reason pause_reason;
    type_unsigned_64 executed;

    if (vm_test_default_pc_at_session_create(STD_NULL, &machine) !=
            TYPE_STATUS_OK || machine == STD_NULL) return 1;
    (C_VOID)vm_machine_reset(machine);
    vm_machine_executor_state_start(machine->control.state);
    vm_machine_executor_state_acknowledge_pause(machine->control.state);
    if (common_machine_debug_acquire(machine->executor, &lease) != LIB_STATUS_OK)
        goto failed;
    for (register_id = CORE_MACHINE_DEBUG_EAX;
         register_id < CORE_MACHINE_DEBUG_REGISTER_COUNT; ++register_id) {
        common_machine_debug_request read = {
            .operation = COMMON_MACHINE_DEBUG_READ_REGISTER,
            .register_id = register_id
        };
        common_machine_debug_request write = {
            .operation = COMMON_MACHINE_DEBUG_WRITE_REGISTER,
            .register_id = register_id
        };

        if (!vm_debug_execute(machine, &lease, &read, &result)) goto failed;
        write.address = result.value;
        if (!vm_debug_execute(machine, &lease, &write, &result)) goto failed;
    }
    if (!vm_debug_execute(machine, &lease, &(common_machine_debug_request){
            .operation = COMMON_MACHINE_DEBUG_WRITE_REAL,
            .segment = 0u,
            .offset = 0x500u,
            .bytes = 1u,
            .data = { byte }
        }, &result) || !vm_debug_execute(machine, &lease,
            &(common_machine_debug_request){
                .operation = COMMON_MACHINE_DEBUG_READ_REAL,
                .segment = 0u,
                .offset = 0x500u,
                .bytes = 1u
            }, &result) || result.data[0] != byte) goto failed;
    if (!vm_debug_execute(machine, &lease, &(common_machine_debug_request){
            .operation = COMMON_MACHINE_DEBUG_SET_WATCH,
            .watch_kind = COMMON_MACHINE_DEBUG_WATCH_READ,
            .address = 0x600u
        }, &result) || !machine->core_machine->executor_cpu_instructions.data.flagWR ||
        machine->core_machine->executor_cpu_instructions.data.wrLinear != 0x600u)
        goto failed;
    if (!vm_debug_execute(machine, &lease, &(common_machine_debug_request){
            .operation = COMMON_MACHINE_DEBUG_CLEAR_WATCH,
            .watch_kind = COMMON_MACHINE_DEBUG_WATCH_READ
        }, &result) || machine->core_machine->executor_cpu_instructions.data.flagWR)
        goto failed;
    if (!vm_debug_execute(machine, &lease, &(common_machine_debug_request){
            .operation = COMMON_MACHINE_DEBUG_SET_WATCH,
            .watch_kind = COMMON_MACHINE_DEBUG_WATCH_WRITE,
            .address = 0x700u
        }, &result) || !vm_debug_execute(machine, &lease,
            &(common_machine_debug_request){
                .operation = COMMON_MACHINE_DEBUG_GET_WATCH,
                .watch_kind = COMMON_MACHINE_DEBUG_WATCH_WRITE
            }, &result) || !result.enabled || result.value != 0x700u)
        goto failed;
    if (!vm_debug_execute(machine, &lease, &(common_machine_debug_request){
            .operation = COMMON_MACHINE_DEBUG_SET_EXECUTION_PLAN,
            .execution_kind = COMMON_MACHINE_DEBUG_EXECUTION_TRACE,
            .instruction_count = 5u
        }, &result) || machine->debug.plan.kind !=
            COMMON_MACHINE_DEBUG_EXECUTION_TRACE ||
        machine->debug.plan.remaining != 5u ||
        vm_machine_debug_limit_instruction_budget(&machine->debug, 256u) != 5u)
        goto failed;
    vm_machine_debug_complete_run(&machine->debug, 2u);
    if (machine->debug.plan.remaining != 3u ||
        vm_machine_debug_completion_pending(&machine->debug, &pause_reason))
        goto failed;
    vm_machine_debug_complete_run(&machine->debug, 3u);
    if (!vm_machine_debug_completion_pending(&machine->debug, &pause_reason) ||
        pause_reason != VM_MACHINE_PAUSE_TRACE ||
        !vm_machine_debug_take_completion(&machine->debug, &pause_reason,
            &executed) || pause_reason != VM_MACHINE_PAUSE_TRACE || executed != 5u)
        goto failed;
    if (!vm_debug_execute(machine, &lease, &(common_machine_debug_request){
            .operation = COMMON_MACHINE_DEBUG_SET_EXECUTION_PLAN,
            .execution_kind = COMMON_MACHINE_DEBUG_EXECUTION_BREAK_LINEAR,
            .address = 0x1234u
        }, &result)) goto failed;
    machine->debug.observation_valid = TYPE_TRUE;
    machine->debug.observation.cs_base = 0x1000u;
    machine->debug.observation.eip = 0x234u;
    vm_machine_debug_complete_run(&machine->debug, 7u);
    if (!vm_machine_debug_breakpoint_due(&machine->debug)) goto failed;
    vm_machine_debug_complete_breakpoint(&machine->debug);
    if (!vm_machine_debug_take_completion(&machine->debug, &pause_reason,
            &executed) || pause_reason != VM_MACHINE_PAUSE_BREAKPOINT ||
        executed != 7u) goto failed;
    if (!vm_debug_execute(machine, &lease, &(common_machine_debug_request){
            .operation = COMMON_MACHINE_DEBUG_CLEAR_EXECUTION_PLAN
        }, &result) || machine->debug.plan.kind !=
            COMMON_MACHINE_DEBUG_EXECUTION_NONE) goto failed;
    common_machine_debug_invalidate(machine->executor);
    if (common_machine_debug_execute_with_lease(machine->executor, &lease,
            &(common_machine_debug_request){
                .operation = COMMON_MACHINE_DEBUG_READ_REGISTER,
                .register_id = CORE_MACHINE_DEBUG_EIP
            }, &result) != LIB_STATUS_INVALID_STATE) goto failed;
    vm_machine_destroy(machine);
    puts("M5:T527:S5:VM-COMMON-DEBUG-MAPPING:OK");
    return 0;

failed:
    vm_machine_destroy(machine);
    return 1;
}
