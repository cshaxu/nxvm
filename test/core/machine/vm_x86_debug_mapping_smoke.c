#include "type.h"

#include "core/core/machine.h"
#include "lib/base/sync_interface.h"
#include "x86/debug/protocol_interface.h"
#include "core/machine/lifecycle.h"
#include "core/machine/machine_private.h"
#include "support/common_machine_fixture.h"
#include "support/rom/session_assets.h"

static C_INT vm_debug_execute(vm_machine *machine,
    const common_machine_debug_lease *lease,
    const x86_debug_request *request,
    x86_debug_response *result)
{
    lib_size response_size = 0u;

    return common_machine_debug_execute_with_lease(machine->executor, lease,
        request, sizeof(*request), result, sizeof(*result), &response_size) ==
            LIB_STATUS_OK && response_size == sizeof(*result);
}

static C_INT vm_debug_wait_paused(const vm_machine *machine)
{
    C_UINT waited;

    for (waited = 0u; waited < 2000u; ++waited) {
        if (common_machine_state_get(machine->executor) == COMMON_MACHINE_PAUSED)
            return TYPE_TRUE;
        base_sync_sleep_milliseconds(1u);
    }
    return TYPE_FALSE;
}

C_INT main(C_VOID)
{
    vm_machine *machine = STD_NULL;
    common_machine_debug_lease lease;
    x86_debug_response result;
    type_unsigned_32 register_id;
    type_unsigned_8 byte = 0x5au;
    vm_machine_pause_reason pause_reason;
    type_unsigned_64 executed;

    if (vm_test_default_pc_at_session_create(STD_NULL, &machine) !=
            TYPE_STATUS_OK || machine == STD_NULL ||
        vm_test_common_machine_bind(machine) != TYPE_STATUS_OK) return 1;
    if (!common_machine_reset(machine->executor) || !vm_debug_wait_paused(machine))
        goto failed;
    if (common_machine_debug_acquire(machine->executor, &lease) != LIB_STATUS_OK)
        goto failed;
    for (register_id = CORE_MACHINE_DEBUG_EAX;
         register_id < CORE_MACHINE_DEBUG_REGISTER_COUNT; ++register_id) {
        x86_debug_request read = {
            .operation = X86_DEBUG_READ_REGISTER,
            .register_id = register_id
        };
        x86_debug_request write = {
            .operation = X86_DEBUG_WRITE_REGISTER,
            .register_id = register_id
        };

        if (!vm_debug_execute(machine, &lease, &read, &result)) goto failed;
        write.address = result.value;
        if (!vm_debug_execute(machine, &lease, &write, &result)) goto failed;
    }
    if (!vm_debug_execute(machine, &lease, &(x86_debug_request){
            .operation = X86_DEBUG_WRITE_REAL,
            .segment = 0u,
            .offset = 0x500u,
            .bytes = 1u,
            .data = { byte }
        }, &result) || !vm_debug_execute(machine, &lease,
            &(x86_debug_request){
                .operation = X86_DEBUG_READ_REAL,
                .segment = 0u,
                .offset = 0x500u,
                .bytes = 1u
            }, &result) || result.data[0] != byte) goto failed;
    if (!vm_debug_execute(machine, &lease, &(x86_debug_request){
            .operation = X86_DEBUG_SET_WATCH,
            .watch_kind = X86_DEBUG_WATCH_READ,
            .address = 0x600u
        }, &result) || !machine->core_machine->executor_cpu_instructions.data.flagWR ||
        machine->core_machine->executor_cpu_instructions.data.wrLinear != 0x600u)
        goto failed;
    if (!vm_debug_execute(machine, &lease, &(x86_debug_request){
            .operation = X86_DEBUG_CLEAR_WATCH,
            .watch_kind = X86_DEBUG_WATCH_READ
        }, &result) || machine->core_machine->executor_cpu_instructions.data.flagWR)
        goto failed;
    if (!vm_debug_execute(machine, &lease, &(x86_debug_request){
            .operation = X86_DEBUG_SET_WATCH,
            .watch_kind = X86_DEBUG_WATCH_WRITE,
            .address = 0x700u
        }, &result) || !vm_debug_execute(machine, &lease,
            &(x86_debug_request){
                .operation = X86_DEBUG_GET_WATCH,
                .watch_kind = X86_DEBUG_WATCH_WRITE
            }, &result) || !result.enabled || result.value != 0x700u)
        goto failed;
    if (!vm_debug_execute(machine, &lease, &(x86_debug_request){
            .operation = X86_DEBUG_SET_EXECUTION_PLAN,
            .execution_kind = X86_DEBUG_EXECUTION_TRACE,
            .instruction_count = 5u
        }, &result) || machine->debug.plan.kind !=
            X86_DEBUG_EXECUTION_TRACE ||
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
    if (!vm_debug_execute(machine, &lease, &(x86_debug_request){
            .operation = X86_DEBUG_SET_EXECUTION_PLAN,
            .execution_kind = X86_DEBUG_EXECUTION_BREAK_LINEAR,
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
    if (!vm_debug_execute(machine, &lease, &(x86_debug_request){
            .operation = X86_DEBUG_CLEAR_EXECUTION_PLAN
        }, &result) || machine->debug.plan.kind !=
            X86_DEBUG_EXECUTION_NONE) goto failed;
    if (!common_machine_stop(machine->executor)) goto failed;
    if (common_machine_debug_execute_with_lease(machine->executor, &lease,
            &(x86_debug_request){
                .operation = X86_DEBUG_READ_REGISTER,
                .register_id = CORE_MACHINE_DEBUG_EIP
            }, sizeof(x86_debug_request), &result, sizeof(result),
            &(lib_size){0u}) != LIB_STATUS_INVALID_STATE) goto failed;
    vm_test_common_machine_unbind(machine);
    vm_machine_destroy(machine);
    puts("M5:T531:S27:VM-X86-DEBUG-MAPPING:OK");
    return 0;

failed:
    vm_test_common_machine_unbind(machine);
    vm_machine_destroy(machine);
    return 1;
}
