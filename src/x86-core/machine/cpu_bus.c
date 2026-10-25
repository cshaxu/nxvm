#include "lib/types/types_interface.h"
#include "x86-core/machine/machine.h"

static lib_status x86_core_machine_cpu_bus_read_memory(void *opaque,
    lib_u32 address, void *destination, lib_u8 bytes,
    x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    x86_core_machine *machine = (x86_core_machine *)opaque;
    lib_status status;
    const lib_bool locked = machine != LIB_NULL &&
        machine->transaction.owner == X86_CORE_MACHINE_TRANSACTION_OWNER_CPU &&
        machine->transaction.kind == X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_LOCK;

    if (machine == LIB_NULL ||
        (cache_control & ~(X86_CORE_CPU_MEMORY_CACHE_PWT | X86_CORE_CPU_MEMORY_CACHE_PCD)))
        return LIB_STATUS_INVALID_ARGUMENT;
    if (observe_only) {
        return x86_core_machine_memory_inspect_physical(&machine->executor_memory,
            address, (lib_uptr)destination, bytes, reset_fetch);
    }
    if (!locked) {
        status = x86_core_machine_transaction_begin(&machine->transaction,
            X86_CORE_MACHINE_TRANSACTION_OWNER_CPU, X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ,
            address, bytes, (lib_u32)provenance | ((lib_u32)cache_control << 8u));
        if (status != LIB_STATUS_OK) return status;
    }
    if (machine->attachment.memory_cache_control != LIB_NULL) {
        status = machine->attachment.memory_cache_control(machine->attachment.context,
            address, bytes, LIB_FALSE, cache_control);
        if (status != LIB_STATUS_OK) {
            if (!locked) x86_core_machine_transaction_cancel(&machine->transaction);
            return status;
        }
    }
    status = reset_fetch ?
        x86_core_machine_memory_read_reset_physical(&machine->executor_memory,
            address, (lib_uptr)destination, bytes) :
        (provenance == X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_FETCH ||
         provenance == X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH) ?
        x86_core_machine_memory_read_instruction_physical(&machine->executor_memory,
            address, (lib_uptr)destination, bytes) :
        x86_core_machine_memory_read_physical(&machine->executor_memory,
            address, (lib_uptr)destination, bytes, LIB_TRUE);
    if (status == LIB_STATUS_UNSUPPORTED) {
        status = x86_core_machine_memory_read_physical(&machine->executor_memory,
            address, (lib_uptr)destination, bytes, LIB_TRUE);
    }
    if (!locked) {
        if (status == LIB_STATUS_OK) x86_core_machine_transaction_commit(&machine->transaction);
        else x86_core_machine_transaction_cancel(&machine->transaction);
    }
    return status;
}

static lib_status x86_core_machine_cpu_bus_write_memory(void *opaque,
    lib_u32 address, const void *source, lib_u8 bytes,
    x86_core_cpu_memory_access_provenance provenance, lib_u8 cache_control)
{
    x86_core_machine *machine = (x86_core_machine *)opaque;
    lib_status status;
    const lib_bool locked = machine != LIB_NULL &&
        machine->transaction.owner == X86_CORE_MACHINE_TRANSACTION_OWNER_CPU &&
        machine->transaction.kind == X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_LOCK;

    if (machine == LIB_NULL ||
        (cache_control & ~(X86_CORE_CPU_MEMORY_CACHE_PWT | X86_CORE_CPU_MEMORY_CACHE_PCD)))
        return LIB_STATUS_INVALID_ARGUMENT;
    if (!locked) {
        status = x86_core_machine_transaction_begin(&machine->transaction,
            X86_CORE_MACHINE_TRANSACTION_OWNER_CPU, X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_WRITE,
            address, bytes, (lib_u32)provenance | ((lib_u32)cache_control << 8u));
        if (status != LIB_STATUS_OK) return status;
    }
    if (machine->attachment.memory_cache_control != LIB_NULL) {
        status = machine->attachment.memory_cache_control(machine->attachment.context,
            address, bytes, LIB_TRUE, cache_control);
        if (status != LIB_STATUS_OK) {
            if (!locked) x86_core_machine_transaction_cancel(&machine->transaction);
            return status;
        }
    }
    const lib_u32 lock_address = machine->transaction.address;
    if (locked) {
        /* Only this actual write range suppresses its raw self-snoop. Alias
         * backing or provider side effects retain their own invalidation. */
        machine->transaction.address = address;
        machine->transaction.value = bytes;
    }
    status = x86_core_machine_memory_write_physical(&machine->executor_memory,
        address, (lib_uptr)source, bytes, LIB_TRUE);
    if (locked) {
        machine->transaction.address = lock_address;
        machine->transaction.value = 0u;
    } else {
        if (status == LIB_STATUS_OK) x86_core_machine_transaction_commit(&machine->transaction);
        else x86_core_machine_transaction_cancel(&machine->transaction);
    }
    return status;
}

void x86_core_machine_cpu_bus_snoop_write(void *opaque, lib_u32 physical,
    lib_size bytes)
{
    x86_core_machine *machine = opaque;

    if (bytes != 0u && !machine->bus_invalidation_seen &&
        machine->transaction.owner == X86_CORE_MACHINE_TRANSACTION_OWNER_DMA &&
        machine->transaction.kind == X86_CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE) {
        machine->bus_invalidation_seen = LIB_TRUE;
        if (machine->bus_invalidation_cycles != LIB_UINT64_MAX)
            ++machine->bus_invalidation_cycles;
    }
    if (machine->transaction.owner == X86_CORE_MACHINE_TRANSACTION_OWNER_CPU &&
        (machine->transaction.kind == X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_WRITE ||
         machine->transaction.kind == X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_LOCK) &&
        physical >= machine->transaction.address &&
        (lib_u64)physical + bytes <= (lib_u64)machine->transaction.address +
            machine->transaction.value)
        return;
    x86_core_cpu_signal_cache_write(machine->executor_cpu_execution,
        physical, bytes);
}

static lib_status x86_core_machine_cpu_bus_transfer_port(void *opaque,
    lib_u16 port, lib_u8 bytes, lib_bool write, lib_u32 *value)
{
    x86_core_machine *machine = (x86_core_machine *)opaque;
    t_port *ports = &machine->executor_port;
    lib_status status;

    /* Prepare the write latch before transaction admission and device access. */
    if (write) {
        switch (bytes) {
        case 1u: ports->data.ioDWord = (lib_u8)*value; break;
        case 2u: ports->data.ioDWord = (lib_u16)*value; break;
        case 4u: ports->data.ioDWord = *value; break;
        default: return LIB_STATUS_INVALID_ARGUMENT;
        }
    }
    const lib_bool locked = machine->transaction.owner == X86_CORE_MACHINE_TRANSACTION_OWNER_CPU &&
        machine->transaction.kind == X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_LOCK;
    if (!locked) {
        status = x86_core_machine_transaction_begin(&machine->transaction,
            X86_CORE_MACHINE_TRANSACTION_OWNER_CPU,
            write ? X86_CORE_MACHINE_TRANSACTION_CPU_PORT_WRITE :
                X86_CORE_MACHINE_TRANSACTION_CPU_PORT_READ,
            port, write ? *value : bytes, write ? bytes : 0u);
        if (status != LIB_STATUS_OK) return status;
    }
    status = write ? x86_core_machine_port_execute_write_width(ports, port, bytes) :
        x86_core_machine_port_execute_read_width(ports, port, bytes, machine->elapsed_ticks);
    if (status != LIB_STATUS_OK) {
        if (!locked) x86_core_machine_transaction_cancel(&machine->transaction);
        return status;
    }
    if (!write) {
        *value = bytes == 1u ? ports->data.ioByte :
            (bytes == 2u ? ports->data.ioWord : ports->data.ioDWord);
        if (!locked) x86_core_machine_transaction_set_value(&machine->transaction, *value);
    }
    return LIB_STATUS_OK;
}

static void x86_core_machine_cpu_bus_complete_port(void *opaque,
    lib_u16 port, lib_u8 bytes, lib_bool write)
{
    x86_core_machine *machine = (x86_core_machine *)opaque;

    if (machine->transaction.kind != X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_LOCK)
        x86_core_machine_transaction_commit(&machine->transaction);
    (void)port;
    (void)bytes;
    (void)write;
}

static lib_bool x86_core_machine_cpu_bus_interrupt_pending(void *opaque)
{
    x86_core_machine *machine = (x86_core_machine *)opaque;

    return machine->attachment.interrupt_pending != LIB_NULL ?
        machine->attachment.interrupt_pending(machine->attachment.context) : LIB_FALSE;
}

static lib_bool x86_core_machine_cpu_bus_wait_test_asserted(void *opaque)
{
    x86_core_machine *machine = (x86_core_machine *)opaque;

    if (machine == LIB_NULL) return LIB_FALSE;
    if (machine->attachment.wait_test_asserted != LIB_NULL)
        return machine->attachment.wait_test_asserted(machine->attachment.context);
    return x86_fpu_ticks_until_completion(machine->fpu, &(lib_u64){0u}) ==
        LIB_STATUS_OK;
}

static lib_bool x86_core_machine_cpu_bus_escape_trap_enabled(void *opaque)
{
    x86_core_machine *machine = (x86_core_machine *)opaque;

    return machine != LIB_NULL && machine->attachment.escape_trap_enabled != LIB_NULL &&
        machine->attachment.escape_trap_enabled(machine->attachment.context);
}

static lib_status x86_core_machine_cpu_bus_acknowledge_interrupt(void *opaque,
    lib_u8 *vector)
{
    x86_core_machine *machine = (x86_core_machine *)opaque;
    lib_status status = x86_core_machine_transaction_begin(&machine->transaction,
        X86_CORE_MACHINE_TRANSACTION_OWNER_CPU,
        X86_CORE_MACHINE_TRANSACTION_CPU_INTERRUPT_ACKNOWLEDGE, 0u, 0u, 0u);

    if (status != LIB_STATUS_OK) return status;
    /* The interrupt source completes acknowledgement before vector consumption. */
    *vector = machine->attachment.interrupt_acknowledge != LIB_NULL ?
        machine->attachment.interrupt_acknowledge(machine->attachment.context) : 0u;
    x86_core_machine_transaction_set_value(&machine->transaction, *vector);
    x86_core_machine_transaction_commit(&machine->transaction);
    return LIB_STATUS_OK;
}

void x86_core_machine_cpu_bus_refresh_pulse(void *core_owner)
{
    x86_core_machine *machine = core_owner;
    if (machine != LIB_NULL) x86_core_machine_external_cycle_invalidate(machine);
}

static void x86_core_machine_cpu_bus_extension_command(void *opaque,
    lib_u8 opcode, lib_u8 modrm)
{
    x86_core_machine *machine = (x86_core_machine *)opaque;

    if (x86_core_machine_transaction_begin(&machine->transaction,
            X86_CORE_MACHINE_TRANSACTION_OWNER_CPU, X86_CORE_MACHINE_TRANSACTION_CPU_FPU_COMMAND,
            opcode, modrm, (lib_u32)x86_fpu_get_profile(machine->fpu)) == LIB_STATUS_OK) {
        x86_core_machine_transaction_commit(&machine->transaction);
    }
}

static lib_status x86_core_machine_cpu_bus_query_cache_memory(void *opaque,
    lib_u32 address, x86_core_cpu_cache_query query,
    x86_core_cpu_cache_memory *out_memory)
{
    x86_core_machine *machine = opaque;

    if (machine == LIB_NULL || out_memory == LIB_NULL ||
        (query != X86_CORE_CPU_CACHE_LOOKUP &&
         query != X86_CORE_CPU_CACHE_RESET_LOOKUP &&
         query != X86_CORE_CPU_CACHE_FILL_PROBE))
        return LIB_STATUS_INVALID_ARGUMENT;
    return x86_core_machine_memory_query_cpu_cache(&machine->executor_memory,
        address, query == X86_CORE_CPU_CACHE_RESET_LOOKUP,
        query == X86_CORE_CPU_CACHE_FILL_PROBE,
        &out_memory->address, &out_memory->fill_enabled);
}

static lib_status x86_core_machine_cpu_bus_cache_operation(void *opaque,
    x86_core_cpu_cache_operation operation)
{
    x86_core_machine *machine = opaque;
    lib_status status;

    if (machine == LIB_NULL ||
        (operation != X86_CORE_CPU_CACHE_WRITEBACK &&
         operation != X86_CORE_CPU_CACHE_FLUSH))
        return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_core_machine_transaction_begin(&machine->transaction,
        X86_CORE_MACHINE_TRANSACTION_OWNER_CPU, X86_CORE_MACHINE_TRANSACTION_CPU_CACHE_OPERATION,
        0u, (lib_u32)operation, 0u);
    if (status != LIB_STATUS_OK) return status;
    status = machine->attachment.cache_operation != LIB_NULL ?
        machine->attachment.cache_operation(machine->attachment.context, operation) :
        LIB_STATUS_OK;
    if (status == LIB_STATUS_OK) x86_core_machine_transaction_commit(&machine->transaction);
    else x86_core_machine_transaction_cancel(&machine->transaction);
    return status;
}

static lib_status x86_core_machine_cpu_bus_begin_memory_lock(void *opaque)
{
    x86_core_machine *machine = opaque;

    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return x86_core_machine_transaction_begin(&machine->transaction,
        X86_CORE_MACHINE_TRANSACTION_OWNER_CPU, X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_LOCK,
        0u, 0u, 0u);
}

static void x86_core_machine_cpu_bus_complete_memory_lock(void *opaque, lib_bool commit)
{
    x86_core_machine *machine = opaque;

    if (machine == LIB_NULL || machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_CPU ||
        machine->transaction.kind != X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_LOCK) return;
    if (commit) x86_core_machine_transaction_commit(&machine->transaction);
    else x86_core_machine_transaction_cancel(&machine->transaction);
}

static void x86_core_machine_cpu_bus_system_management_active(void *opaque,
    lib_bool active)
{
    x86_core_machine *machine = opaque;

    if (machine->attachment.system_management_active != LIB_NULL)
        machine->attachment.system_management_active(machine->attachment.context, active);
}

static lib_status x86_core_machine_cpu_bus_special_cycle(void *opaque,
    x86_core_cpu_special_cycle cycle, lib_bool *acknowledged)
{
    x86_core_machine *machine = opaque;
    lib_bool ready = LIB_TRUE;
    lib_status status;

    if (machine == LIB_NULL || acknowledged == LIB_NULL ||
        cycle < X86_CORE_CPU_SPECIAL_HALT || cycle > X86_CORE_CPU_SPECIAL_STOP_GRANT)
        return LIB_STATUS_INVALID_ARGUMENT;
    *acknowledged = LIB_FALSE;
    if (machine->transaction.owner == X86_CORE_MACHINE_TRANSACTION_OWNER_NONE) {
        if (machine->transaction.hold_owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE)
            return LIB_STATUS_OK;
        status = x86_core_machine_transaction_begin(&machine->transaction,
            X86_CORE_MACHINE_TRANSACTION_OWNER_CPU, X86_CORE_MACHINE_TRANSACTION_CPU_SPECIAL_CYCLE,
            cycle == X86_CORE_CPU_SPECIAL_STOP_GRANT ? 0x10u : 0u,
            (lib_u32)cycle, cycle == X86_CORE_CPU_SPECIAL_SHUTDOWN ? 0x0eu : 0x0bu);
        if (status != LIB_STATUS_OK) return status;
    } else if (machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_CPU ||
        machine->transaction.kind != X86_CORE_MACHINE_TRANSACTION_CPU_SPECIAL_CYCLE ||
        machine->transaction.value != (lib_u32)cycle) return LIB_STATUS_INVALID_STATE;
    if (machine->attachment.special_cycle != LIB_NULL) {
        status = machine->attachment.special_cycle(machine->attachment.context, cycle, &ready);
        if (status != LIB_STATUS_OK) return status;
    }
    if (!ready || (machine->transaction_contract.cpu_cycle_bus_ready_gate_enabled &&
        !machine->cpu_cycle_bus_ready)) return LIB_STATUS_OK;
    if (machine->attachment.complete_special_cycle != LIB_NULL)
        machine->attachment.complete_special_cycle(machine->attachment.context, cycle, LIB_TRUE);
    x86_core_machine_transaction_commit(&machine->transaction);
    *acknowledged = LIB_TRUE;
    return LIB_STATUS_OK;
}

static void x86_core_machine_cpu_bus_cancel_special_cycle(void *opaque,
    x86_core_cpu_special_cycle cycle)
{
    x86_core_machine *machine = opaque;

    if (machine == LIB_NULL || machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_CPU ||
        machine->transaction.kind != X86_CORE_MACHINE_TRANSACTION_CPU_SPECIAL_CYCLE ||
        machine->transaction.value != (lib_u32)cycle) return;
    if (machine->attachment.complete_special_cycle != LIB_NULL)
        machine->attachment.complete_special_cycle(machine->attachment.context, cycle, LIB_FALSE);
    x86_core_machine_transaction_cancel(&machine->transaction);
}

const x86_core_cpu_bus_provider x86_core_machine_cpu_bus = {
    .read_memory = x86_core_machine_cpu_bus_read_memory,
    .write_memory = x86_core_machine_cpu_bus_write_memory,
    .transfer_port = x86_core_machine_cpu_bus_transfer_port,
    .complete_port = x86_core_machine_cpu_bus_complete_port,
    .wait_test_asserted = x86_core_machine_cpu_bus_wait_test_asserted,
    .escape_trap_enabled = x86_core_machine_cpu_bus_escape_trap_enabled,
    .interrupt_pending = x86_core_machine_cpu_bus_interrupt_pending,
    .acknowledge_interrupt = x86_core_machine_cpu_bus_acknowledge_interrupt,
    .extension_command = x86_core_machine_cpu_bus_extension_command,
    .query_cache_memory = x86_core_machine_cpu_bus_query_cache_memory,
    .cache_operation = x86_core_machine_cpu_bus_cache_operation,
    .begin_memory_lock = x86_core_machine_cpu_bus_begin_memory_lock,
    .complete_memory_lock = x86_core_machine_cpu_bus_complete_memory_lock,
    .system_management_active = x86_core_machine_cpu_bus_system_management_active,
    .special_cycle = x86_core_machine_cpu_bus_special_cycle,
    .cancel_special_cycle = x86_core_machine_cpu_bus_cancel_special_cycle
};

/* Board bus arbitration, wait windows and trace publication are not CPU state. */
void x86_core_machine_transaction_trace(void *opaque,
    x86_core_machine_transaction_owner owner, x86_core_machine_transaction_kind kind,
    x86_core_machine_transaction_phase phase, lib_u32 address,
    lib_u32 value, lib_u32 detail)
{
    x86_core_machine *machine = (x86_core_machine *)opaque;
    x86_core_machine_trace_event_type type;

    if (machine == LIB_NULL) return;
    if (phase == X86_CORE_MACHINE_TRANSACTION_PHASE_BEGIN)
        machine->bus_invalidation_seen = LIB_FALSE;
    /* Generic-AT policy: an acknowledged DMA bus handoff breaks CPU-side
     * locality. D4 establishes the HOLD/HLDA topology, not this page-retention
     * behavior or any physical phase duration. */
    if (phase == X86_CORE_MACHINE_TRANSACTION_PHASE_HOLD_ACKNOWLEDGE &&
        owner == X86_CORE_MACHINE_TRANSACTION_OWNER_DMA) {
        x86_core_machine_external_cycle_invalidate(machine);
    }
    switch (phase) {
    case X86_CORE_MACHINE_TRANSACTION_PHASE_BEGIN:
        type = X86_CORE_MACHINE_TRACE_TRANSACTION_BEGIN;
        break;
    case X86_CORE_MACHINE_TRANSACTION_PHASE_COMMIT:
        type = X86_CORE_MACHINE_TRACE_TRANSACTION_COMMIT;
        break;
    case X86_CORE_MACHINE_TRANSACTION_PHASE_CANCEL:
        type = X86_CORE_MACHINE_TRACE_TRANSACTION_CANCEL;
        break;
    case X86_CORE_MACHINE_TRANSACTION_PHASE_HOLD_REQUEST:
        type = X86_CORE_MACHINE_TRACE_TRANSACTION_HOLD_REQUEST;
        break;
    case X86_CORE_MACHINE_TRANSACTION_PHASE_HOLD_ACKNOWLEDGE:
        type = X86_CORE_MACHINE_TRACE_TRANSACTION_HOLD_ACKNOWLEDGE;
        break;
    case X86_CORE_MACHINE_TRANSACTION_PHASE_HOLD_RELEASE:
        type = X86_CORE_MACHINE_TRACE_TRANSACTION_HOLD_RELEASE;
        break;
    default:
        return;
    }
    x86_core_machine_trace_record(machine, type, address, value,
        (lib_u32)owner | ((lib_u32)kind << 8u) |
        (detail << 16u));
}

static lib_i32 x86_core_machine_external_cycle_access_is_chargeable(lib_u8 write,
    x86_core_cpu_memory_access_provenance provenance)
{
    return (!write && (provenance == X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH ||
        provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA ||
        provenance == X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ)) ||
        (write && (provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA ||
        provenance == X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_WRITE));
}

static lib_i32 x86_core_machine_external_cycle_pending_matches(const x86_core_machine *machine,
    x86_core_cpu_external_cycle_space space, lib_u32 address,
    lib_u8 bytes, lib_u8 write,
    x86_core_cpu_memory_access_provenance provenance)
{
    return machine != LIB_NULL && machine->external_cycle_pending_valid &&
        machine->external_cycle_pending_space == space &&
        machine->external_cycle_pending_physical == address &&
        machine->external_cycle_pending_bytes == bytes &&
        machine->external_cycle_pending_write == write &&
        machine->external_cycle_pending_provenance == provenance;
}

static lib_u32 x86_core_machine_external_access_wait_ticks(
    const x86_core_machine *machine, x86_core_cpu_external_cycle_space space,
    lib_u32 address)
{
    lib_size index;

    if (machine == LIB_NULL) return 0u;
    for (index = 0u; index < X86_CORE_MACHINE_EXTERNAL_ACCESS_WAIT_WINDOW_CAPACITY;
            ++index) {
        const x86_core_machine_external_access_wait_window *window =
            &machine->transaction_contract.external_access_wait_windows[index];
        if (window->wait_ticks != 0u && window->space == space &&
            address >= window->first_address && address <= window->last_address) {
            return window->wait_ticks;
        }
    }
    return 0u;
}

void x86_core_machine_external_cycle_invalidate(x86_core_machine *machine)
{
    if (machine == LIB_NULL) return;
    machine->external_cycle_page_valid = LIB_FALSE;
    machine->external_cycle_pending_valid = LIB_FALSE;
    machine->external_cycle_overlap_valid = LIB_FALSE;
}

lib_u64 x86_core_machine_cpu_external_cycle_trace(void *opaque,
    x86_core_cpu_external_cycle_phase phase,
    x86_core_cpu_external_cycle_space space, lib_u32 address,
    lib_u8 bytes, lib_u8 write,
    x86_core_cpu_memory_access_provenance provenance)
{
    x86_core_machine *machine = (x86_core_machine *)opaque;
    x86_core_machine_trace_event_type type;
    lib_i32 pending_matches;
    lib_u8 page_timing_enabled;
    lib_u64 cycle_ticks = 0u;

    if (machine == LIB_NULL) return 0u;
    page_timing_enabled = space == X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY &&
        machine->transaction_contract.external_cycle_timing.page_bytes != 0u &&
        ((machine->transaction_contract.external_cycle_timing.first_eligible_address == 0u &&
          machine->transaction_contract.external_cycle_timing.last_eligible_address == 0u) ||
         (address >= machine->transaction_contract.external_cycle_timing.first_eligible_address &&
          address <= machine->transaction_contract.external_cycle_timing.last_eligible_address));
    switch (phase) {
    case X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN:
        if (machine->external_cycle_pending_valid) {
            x86_core_machine_external_cycle_invalidate(machine);
        } else if (page_timing_enabled && machine->external_cycle_overlap_valid &&
            machine->external_cycle_overlap_next_physical != address) {
            machine->external_cycle_overlap_valid = LIB_FALSE;
        }
        machine->external_cycle_pending_valid = LIB_TRUE;
        machine->external_cycle_pending_space = space;
        machine->external_cycle_pending_physical = address;
        machine->external_cycle_pending_bytes = bytes;
        machine->external_cycle_pending_write = write;
        machine->external_cycle_pending_provenance = provenance;
        type = X86_CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_BEGIN;
        break;
    case X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_OVERLAP_DECLARE:
        if (page_timing_enabled && machine->transaction_contract.external_cycle_timing.overlap_policy ==
                X86_CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_EXPLICIT_SEQUENTIAL &&
            machine->external_cycle_pending_valid &&
            machine->external_cycle_pending_space ==
                X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY &&
            !machine->external_cycle_pending_write && !write &&
            machine->external_cycle_pending_provenance ==
                X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH &&
            provenance == X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH &&
            machine->external_cycle_pending_physical <= LIB_UINT32_MAX -
                machine->external_cycle_pending_bytes &&
            address == machine->external_cycle_pending_physical +
                machine->external_cycle_pending_bytes) {
            machine->external_cycle_overlap_valid = LIB_TRUE;
            machine->external_cycle_overlap_next_physical = address;
        }
        type = X86_CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_OVERLAP_DECLARE;
        break;
    case X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT:
        pending_matches = x86_core_machine_external_cycle_pending_matches(machine,
            space, address, bytes, write, provenance);
        if (pending_matches && page_timing_enabled &&
            x86_core_machine_external_cycle_access_is_chargeable(write, provenance)) {
            lib_u32 page_tag = address /
                machine->transaction_contract.external_cycle_timing.page_bytes;
            lib_u32 wait_ticks = !machine->external_cycle_page_valid ||
                !machine->external_cycle_overlap_valid ||
                machine->external_cycle_overlap_next_physical != address ||
                machine->external_cycle_page_tag != page_tag ?
                machine->transaction_contract.external_cycle_timing.page_miss_ticks :
                machine->transaction_contract.external_cycle_timing.page_hit_ticks;
            if (machine->external_cycle_overlap_valid &&
                machine->external_cycle_overlap_next_physical == address) {
                machine->external_cycle_overlap_valid = LIB_FALSE;
            }
            machine->external_cycle_page_valid = LIB_TRUE;
            machine->external_cycle_page_tag = page_tag;
            cycle_ticks += wait_ticks;
        }
        if (pending_matches) {
            lib_u32 wait_ticks = x86_core_machine_external_access_wait_ticks(
                machine, space, address);
            cycle_ticks += wait_ticks;
            machine->external_cycle_pending_valid = LIB_FALSE;
        }
        /* SX records service and deferred writes in its sole CPU bus owner.
         * Older families retain the separate Core wait envelope. */
        if (machine->cpu_profile != X86_CORE_CPU_PROFILE_80486SX) {
            if (LIB_UINT64_MAX - machine->external_cycle_round_ticks < cycle_ticks) {
                machine->external_cycle_round_overflow = LIB_TRUE;
            } else {
                machine->external_cycle_round_ticks += cycle_ticks;
            }
        }
        type = X86_CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_COMMIT;
        break;
    case X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL:
        if (x86_core_machine_external_cycle_pending_matches(machine, space, address,
                bytes, write, provenance)) {
            x86_core_machine_external_cycle_invalidate(machine);
        }
        type = X86_CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_CANCEL;
        break;
    default:
        return 0u;
    }
    x86_core_machine_trace_record(machine, type, address, bytes,
        (lib_u32)provenance);
    return cycle_ticks;
}
