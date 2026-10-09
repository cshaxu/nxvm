#include "composition_fixture.h"
#include "core/x86/machine.h"

test_core_preview_publication test_core_capture_preview_publication(const core_machine *machine)
{
    return (test_core_preview_publication){
        machine->transaction.committed_count,
        machine->transaction.cancelled_count,
        machine->trace.count
    };
}

lib_bool test_core_preview_lexeme(core_machine *machine,
    core_machine_cpu_instruction_lexeme *lexeme)
{
    return core_machine_cpu_execution_preview_lexeme(
        machine->executor_cpu_execution, lexeme);
}

core_machine *test_core_port_owner_create(void)
{
    core_machine *machine = lib_allocate_zero(1u, sizeof(*machine));

    if (machine != LIB_NULL) {
        machine->lifecycle = CORE_MACHINE_INITIALIZED;
        core_machine_port_initialize(&machine->executor_port);
    }
    return machine;
}

lib_bool test_core_fpu_command_handoff(const core_machine *machine,
    lib_u8 opcode, lib_u8 modrm, lib_u64 *remaining_ticks)
{
    return x86_fpu_ticks_until_completion(machine->fpu, remaining_ticks) == LIB_STATUS_OK &&
        machine->transaction.address == opcode &&
        machine->transaction.value == modrm &&
        machine->transaction.kind == CORE_MACHINE_TRANSACTION_CPU_FPU_COMMAND;
}

void test_core_port_owner_destroy(core_machine *machine)
{
    if (machine == LIB_NULL) return;
    core_machine_port_finalize(&machine->executor_port);
    lib_release(machine);
}

void test_core_set_lifecycle(core_machine *machine, core_machine_lifecycle state)
{
    machine->lifecycle = state;
}

void test_core_set_firmware_operation_active(core_machine *machine, lib_bool active)
{
    machine->firmware_operation_active = active;
}

void test_core_replace_firmware_provider(core_machine *machine,
    const core_machine_firmware_provider *provider)
{
    machine->firmware_provider = provider;
}

/* Preserve execution-time diagnostic access, including physical-route effects;
 * it is not a paused-debug inspection operation. */
lib_status test_core_read_physical(core_machine *machine, lib_u32 address,
    lib_uptr destination, lib_uptr bytes)
{
    return core_machine_memory_read_physical(&machine->executor_memory,
        address, destination, bytes);
}

lib_bool test_core_dma_wait_remaining_matches(const core_machine *machine,
    lib_u32 expected)
{
    return machine->dma_cycle_wait_remaining == expected;
}

lib_status test_core_write_physical(core_machine *machine, lib_u32 address,
    lib_uptr source, lib_uptr bytes)
{
    return core_machine_memory_write_physical(&machine->executor_memory,
        address, source, bytes);
}

void test_core_invoke_parity_fault(core_machine *machine, lib_u32 physical)
{
    machine->executor_memory.connect.parity_fault(
        machine->executor_memory.connect.parity_owner, physical);
}

lib_bool test_core_refresh_request(core_machine *machine, lib_u8 *address)
{
    return machine->attachment.refresh_request(machine->attachment.context, address);
}

lib_bool test_core_dma_hold_begin(core_machine *machine)
{
    lib_i32 failed = core_machine_transaction_hold_request(&machine->transaction,
        CORE_MACHINE_TRANSACTION_OWNER_DMA, 0u) != LIB_STATUS_OK;
    failed = failed || core_machine_transaction_hold_acknowledge(&machine->transaction,
        CORE_MACHINE_TRANSACTION_OWNER_DMA) != LIB_STATUS_OK;
    return !failed;
}

lib_bool test_core_dma_hold_excludes_cpu(core_machine *machine)
{
    lib_i32 failed = !test_core_dma_hold_begin(machine);
    failed = failed || core_machine_transaction_begin(&machine->transaction,
        CORE_MACHINE_TRANSACTION_OWNER_CPU,
        CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ, 0u, 0u, 0u) !=
        LIB_STATUS_INVALID_ARGUMENT;
    core_machine_transaction_hold_release(&machine->transaction,
        CORE_MACHINE_TRANSACTION_OWNER_DMA);
    return !failed;
}

lib_bool test_core_instances_are_distinct(const core_machine *first,
    const core_machine *second)
{
    return first != LIB_NULL && second != LIB_NULL && first != second &&
        first->executor_cpu_execution != second->executor_cpu_execution &&
        &first->executor_memory != &second->executor_memory &&
        &first->executor_port != &second->executor_port;
}

lib_bool test_core_port_has_read(const core_machine *machine, lib_u16 port)
{
    return core_machine_port_has_read(&machine->executor_port, port);
}

lib_bool test_core_port_has_write(const core_machine *machine, lib_u16 port)
{
    return core_machine_port_has_write(&machine->executor_port, port);
}

/* Retain raw test I/O after a budget stop, when the
 * production debug bus deliberately rejects a still-running executor. */
void test_core_write_port_after_run(core_machine *machine, lib_u16 port, lib_u32 value)
{
    core_machine_port_write(&machine->executor_port, port, value);
}

lib_bool test_core_transaction_timing_is_disabled(const core_machine *machine)
{
    return !(machine->transaction_contract.cpu_cycle_bus_ready_gate_enabled ||
        machine->transaction_contract.cpu_prefetch_reservation_enabled ||
        machine->transaction_contract.external_cycle_timing.page_bytes != 0u ||
        machine->transaction_contract.external_cycle_timing.page_miss_ticks != 0u ||
        machine->transaction_contract.external_cycle_timing.page_hit_ticks != 0u ||
        machine->transaction_contract.external_cycle_timing.overlap_policy !=
            CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED ||
        machine->transaction_contract.external_access_wait_windows[0].wait_ticks != 0u);
}

lib_bool test_core_a20_is_enabled(const core_machine *machine)
{
    return machine->executor_memory.data.flagA20;
}

lib_u64 test_core_elapsed_ticks(const core_machine *machine)
{
    return machine->elapsed_ticks;
}

lib_bool test_core_cpu_is_halted(const core_machine *machine)
{
    return core_machine_cpu_is_halted(machine->executor_cpu_execution);
}

void test_core_request_cpu_shutdown(core_machine *machine)
{
    core_machine_cpu_execution_request_shutdown(machine->executor_cpu_execution);
}

lib_bool test_core_has_parity_storage(const core_machine *machine)
{
    return machine->executor_memory.connect.parity != 0u;
}

void test_core_flip_parity(core_machine *machine, lib_u32 physical)
{
    ((lib_u8 *)machine->executor_memory.connect.parity)[physical] ^= 1u;
}

lib_bool test_core_dma_timing_matches(const core_machine *machine,
    lib_u32 wait_quanta, lib_bool gate_enabled, lib_bool ready)
{
    return machine->transaction_contract.dma_cycle_wait_quanta == wait_quanta &&
        (machine->transaction_contract.dma_cycle_bus_ready_gate_enabled != LIB_FALSE) == gate_enabled &&
        (machine->dma_cycle_bus_ready != LIB_FALSE) == ready;
}

core_machine_transaction_contract test_core_capture_transaction_contract(const core_machine *machine)
{
    return machine->transaction_contract;
}

core_machine_retirement_time_contract test_core_retirement_contract(const core_machine *machine)
{
    return machine->retirement_time_contract;
}

lib_bool test_core_has_rom_mapping_start(const core_machine *machine, lib_u32 physical_start)
{
    for (lib_size mapping = 0u; mapping < machine->immutable_rom_mapping_count; ++mapping) {
        if (machine->immutable_rom_mappings[mapping].physical_start == physical_start)
            return LIB_TRUE;
    }
    return LIB_FALSE;
}

lib_u32 test_core_read_port_after_run(core_machine *machine, lib_u16 port)
{
    return core_machine_port_read(&machine->executor_port, port);
}
