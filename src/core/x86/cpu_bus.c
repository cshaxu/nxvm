#include "lib/types/types_interface.h"
#include "core/x86/machine.h"

static lib_status core_machine_cpu_bus_read_memory(void *opaque,
    lib_u32 address, void *destination, lib_u8 bytes,
    x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    core_machine *machine = (core_machine *)opaque;
    lib_status status;
    const lib_bool locked = machine != LIB_NULL &&
        machine->transaction.owner == CORE_MACHINE_TRANSACTION_OWNER_CPU &&
        machine->transaction.kind == CORE_MACHINE_TRANSACTION_CPU_MEMORY_LOCK;

    if (machine == LIB_NULL ||
        (cache_control & ~(X86_CORE_CPU_MEMORY_CACHE_PWT | X86_CORE_CPU_MEMORY_CACHE_PCD)))
        return LIB_STATUS_INVALID_ARGUMENT;
    if (observe_only) {
        return core_machine_memory_inspect_physical(&machine->executor_memory,
            address, (lib_uptr)destination, bytes, reset_fetch);
    }
    if (!locked) {
        status = core_machine_transaction_begin(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_CPU, CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ,
            address, bytes, (lib_u32)provenance | ((lib_u32)cache_control << 8u));
        if (status != LIB_STATUS_OK) return status;
    }
    if (machine->attachment.memory_cache_control != LIB_NULL) {
        status = machine->attachment.memory_cache_control(machine->attachment.context,
            address, bytes, LIB_FALSE, cache_control);
        if (status != LIB_STATUS_OK) {
            if (!locked) core_machine_transaction_cancel(&machine->transaction);
            return status;
        }
    }
    status = reset_fetch ?
        core_machine_memory_read_reset_physical(&machine->executor_memory,
            address, (lib_uptr)destination, bytes) :
        core_machine_memory_read_physical(&machine->executor_memory,
            address, (lib_uptr)destination, bytes);
    if (status == LIB_STATUS_UNSUPPORTED) {
        status = core_machine_memory_read_physical(&machine->executor_memory,
            address, (lib_uptr)destination, bytes);
    }
    if (!locked) {
        if (status == LIB_STATUS_OK) core_machine_transaction_commit(&machine->transaction);
        else core_machine_transaction_cancel(&machine->transaction);
    }
    return status;
}

static lib_status core_machine_cpu_bus_write_memory(void *opaque,
    lib_u32 address, const void *source, lib_u8 bytes,
    x86_core_cpu_memory_access_provenance provenance, lib_u8 cache_control)
{
    core_machine *machine = (core_machine *)opaque;
    lib_status status;
    const lib_bool locked = machine != LIB_NULL &&
        machine->transaction.owner == CORE_MACHINE_TRANSACTION_OWNER_CPU &&
        machine->transaction.kind == CORE_MACHINE_TRANSACTION_CPU_MEMORY_LOCK;

    if (machine == LIB_NULL ||
        (cache_control & ~(X86_CORE_CPU_MEMORY_CACHE_PWT | X86_CORE_CPU_MEMORY_CACHE_PCD)))
        return LIB_STATUS_INVALID_ARGUMENT;
    if (!locked) {
        status = core_machine_transaction_begin(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_CPU, CORE_MACHINE_TRANSACTION_CPU_MEMORY_WRITE,
            address, bytes, (lib_u32)provenance | ((lib_u32)cache_control << 8u));
        if (status != LIB_STATUS_OK) return status;
    }
    if (machine->attachment.memory_cache_control != LIB_NULL) {
        status = machine->attachment.memory_cache_control(machine->attachment.context,
            address, bytes, LIB_TRUE, cache_control);
        if (status != LIB_STATUS_OK) {
            if (!locked) core_machine_transaction_cancel(&machine->transaction);
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
    status = core_machine_memory_write_physical(&machine->executor_memory,
        address, (lib_uptr)source, bytes);
    if (locked) {
        machine->transaction.address = lock_address;
        machine->transaction.value = 0u;
    } else {
        if (status == LIB_STATUS_OK) core_machine_transaction_commit(&machine->transaction);
        else core_machine_transaction_cancel(&machine->transaction);
    }
    return status;
}

void core_machine_cpu_bus_snoop_write(void *opaque, lib_u32 physical,
    lib_size bytes)
{
    core_machine *machine = opaque;

    if (machine->transaction.owner == CORE_MACHINE_TRANSACTION_OWNER_CPU &&
        (machine->transaction.kind == CORE_MACHINE_TRANSACTION_CPU_MEMORY_WRITE ||
         machine->transaction.kind == CORE_MACHINE_TRANSACTION_CPU_MEMORY_LOCK) &&
        physical >= machine->transaction.address &&
        (lib_u64)physical + bytes <= (lib_u64)machine->transaction.address +
            machine->transaction.value)
        return;
    x86_core_cpu_signal_cache_write(machine->executor_cpu_execution,
        physical, bytes);
}

static void core_machine_cpu_bus_port_phase(core_machine *machine,
    x86_core_cpu_external_cycle_phase phase, lib_u16 port, lib_u8 bytes,
    lib_bool write)
{
    core_machine_cpu_external_cycle_trace(machine, phase,
        X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT, port, bytes, write,
        X86_CORE_CPU_MEMORY_ACCESS_DATA);
}

static lib_status core_machine_cpu_bus_transfer_port(void *opaque,
    lib_u16 port, lib_u8 bytes, lib_bool write, lib_u32 *value)
{
    core_machine *machine = (core_machine *)opaque;
    t_port *ports = &machine->executor_port;
    lib_status status;

    /* Preserve the board latch before beginning the externally visible cycle. */
    if (write) {
        switch (bytes) {
        case 1u: ports->data.ioByte = (lib_u8)*value; break;
        case 2u: ports->data.ioWord = (lib_u16)*value; break;
        case 4u: ports->data.ioDWord = *value; break;
        default: return LIB_STATUS_INVALID_ARGUMENT;
        }
    }
    core_machine_cpu_bus_port_phase(machine,
        X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN, port, bytes, write);
    status = core_machine_transaction_begin(&machine->transaction,
        CORE_MACHINE_TRANSACTION_OWNER_CPU,
        write ? CORE_MACHINE_TRANSACTION_CPU_PORT_WRITE :
            CORE_MACHINE_TRANSACTION_CPU_PORT_READ,
        port, write ? *value : bytes, write ? bytes : 0u);
    if (status != LIB_STATUS_OK) {
        core_machine_cpu_bus_port_phase(machine,
            X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL, port, bytes, write);
        return status;
    }
    status = write ? core_machine_port_execute_write_width(ports, port, bytes) :
        core_machine_port_execute_read_width(ports, port, bytes, machine->elapsed_ticks);
    if (status != LIB_STATUS_OK) {
        core_machine_transaction_cancel(&machine->transaction);
        core_machine_cpu_bus_port_phase(machine,
            X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL, port, bytes, write);
        return status;
    }
    if (!write) {
        *value = bytes == 1u ? ports->data.ioByte :
            (bytes == 2u ? ports->data.ioWord : ports->data.ioDWord);
        core_machine_transaction_set_value(&machine->transaction, *value);
    }
    return LIB_STATUS_OK;
}

static void core_machine_cpu_bus_complete_port(void *opaque,
    lib_u16 port, lib_u8 bytes, lib_bool write)
{
    core_machine *machine = (core_machine *)opaque;

    core_machine_transaction_commit(&machine->transaction);
    core_machine_cpu_bus_port_phase(machine,
        X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT, port, bytes, write);
}

static lib_bool core_machine_cpu_bus_interrupt_pending(void *opaque)
{
    core_machine *machine = (core_machine *)opaque;

    return machine->attachment.pic_pending != LIB_NULL ?
        machine->attachment.pic_pending(machine->attachment.context) : LIB_FALSE;
}

static lib_bool core_machine_cpu_bus_wait_test_asserted(void *opaque)
{
    core_machine *machine = (core_machine *)opaque;

    if (machine == LIB_NULL) return LIB_FALSE;
    if (machine->attachment.wait_test_asserted != LIB_NULL)
        return machine->attachment.wait_test_asserted(machine->attachment.context);
    return x86_fpu_ticks_until_completion(machine->fpu, &(lib_u64){0u}) ==
        LIB_STATUS_OK;
}

static lib_bool core_machine_cpu_bus_escape_trap_enabled(void *opaque)
{
    core_machine *machine = (core_machine *)opaque;

    return machine != LIB_NULL && machine->attachment.escape_trap_enabled != LIB_NULL &&
        machine->attachment.escape_trap_enabled(machine->attachment.context);
}

static lib_status core_machine_cpu_bus_acknowledge_interrupt(void *opaque,
    lib_u8 *vector)
{
    core_machine *machine = (core_machine *)opaque;
    lib_status status = core_machine_transaction_begin(&machine->transaction,
        CORE_MACHINE_TRANSACTION_OWNER_CPU,
        CORE_MACHINE_TRANSACTION_CPU_INTERRUPT_ACKNOWLEDGE, 0u, 0u, 0u);

    if (status != LIB_STATUS_OK) return status;
    /* The PIC owns the IRR-to-ISR transition before CPU vector consumption. */
    *vector = machine->attachment.pic_acknowledge != LIB_NULL ?
        machine->attachment.pic_acknowledge(machine->attachment.context) : 0u;
    core_machine_transaction_set_value(&machine->transaction, *vector);
    core_machine_transaction_commit(&machine->transaction);
    return LIB_STATUS_OK;
}

void core_machine_cpu_bus_refresh_pulse(void *core_owner)
{
    core_machine *machine = core_owner;
    if (machine != LIB_NULL) core_machine_external_cycle_invalidate(machine);
}

static void core_machine_cpu_bus_extension_command(void *opaque,
    lib_u8 opcode, lib_u8 modrm)
{
    core_machine *machine = (core_machine *)opaque;

    if (core_machine_transaction_begin(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_CPU, CORE_MACHINE_TRANSACTION_CPU_FPU_COMMAND,
            opcode, modrm, (lib_u32)x86_fpu_get_profile(machine->fpu)) == LIB_STATUS_OK) {
        core_machine_transaction_commit(&machine->transaction);
    }
}

static lib_status core_machine_cpu_bus_query_cache_memory(void *opaque,
    lib_u32 address, x86_core_cpu_cache_query query,
    x86_core_cpu_cache_memory *out_memory)
{
    core_machine *machine = opaque;

    if (machine == LIB_NULL || out_memory == LIB_NULL ||
        (query != X86_CORE_CPU_CACHE_LOOKUP &&
         query != X86_CORE_CPU_CACHE_RESET_LOOKUP &&
         query != X86_CORE_CPU_CACHE_FILL_PROBE))
        return LIB_STATUS_INVALID_ARGUMENT;
    return core_machine_memory_query_cpu_cache(&machine->executor_memory,
        address, query == X86_CORE_CPU_CACHE_RESET_LOOKUP,
        query == X86_CORE_CPU_CACHE_FILL_PROBE,
        &out_memory->address, &out_memory->fill_enabled);
}

static lib_status core_machine_cpu_bus_cache_operation(void *opaque,
    x86_core_cpu_cache_operation operation)
{
    core_machine *machine = opaque;
    lib_status status;

    if (machine == LIB_NULL ||
        (operation != X86_CORE_CPU_CACHE_WRITEBACK &&
         operation != X86_CORE_CPU_CACHE_FLUSH))
        return LIB_STATUS_INVALID_ARGUMENT;
    status = core_machine_transaction_begin(&machine->transaction,
        CORE_MACHINE_TRANSACTION_OWNER_CPU, CORE_MACHINE_TRANSACTION_CPU_CACHE_OPERATION,
        0u, (lib_u32)operation, 0u);
    if (status != LIB_STATUS_OK) return status;
    status = machine->attachment.cache_operation != LIB_NULL ?
        machine->attachment.cache_operation(machine->attachment.context, operation) :
        LIB_STATUS_OK;
    if (status == LIB_STATUS_OK) core_machine_transaction_commit(&machine->transaction);
    else core_machine_transaction_cancel(&machine->transaction);
    return status;
}

static lib_status core_machine_cpu_bus_begin_memory_lock(void *opaque)
{
    core_machine *machine = opaque;

    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return core_machine_transaction_begin(&machine->transaction,
        CORE_MACHINE_TRANSACTION_OWNER_CPU, CORE_MACHINE_TRANSACTION_CPU_MEMORY_LOCK,
        0u, 0u, 0u);
}

static void core_machine_cpu_bus_complete_memory_lock(void *opaque, lib_bool commit)
{
    core_machine *machine = opaque;

    if (machine == LIB_NULL || machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_CPU ||
        machine->transaction.kind != CORE_MACHINE_TRANSACTION_CPU_MEMORY_LOCK) return;
    if (commit) core_machine_transaction_commit(&machine->transaction);
    else core_machine_transaction_cancel(&machine->transaction);
}

static void core_machine_cpu_bus_system_management_active(void *opaque,
    lib_bool active)
{
    core_machine *machine = opaque;

    if (machine->attachment.system_management_active != LIB_NULL)
        machine->attachment.system_management_active(machine->attachment.context, active);
}

static lib_status core_machine_cpu_bus_special_cycle(void *opaque,
    x86_core_cpu_special_cycle cycle, lib_bool *acknowledged)
{
    core_machine *machine = opaque;
    lib_bool ready = LIB_TRUE;
    lib_status status;

    if (machine == LIB_NULL || acknowledged == LIB_NULL ||
        cycle < X86_CORE_CPU_SPECIAL_HALT || cycle > X86_CORE_CPU_SPECIAL_STOP_GRANT)
        return LIB_STATUS_INVALID_ARGUMENT;
    *acknowledged = LIB_FALSE;
    if (machine->transaction.owner == CORE_MACHINE_TRANSACTION_OWNER_NONE) {
        if (machine->transaction.hold_owner != CORE_MACHINE_TRANSACTION_OWNER_NONE)
            return LIB_STATUS_OK;
        status = core_machine_transaction_begin(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_CPU, CORE_MACHINE_TRANSACTION_CPU_SPECIAL_CYCLE,
            cycle == X86_CORE_CPU_SPECIAL_STOP_GRANT ? 0x10u : 0u,
            (lib_u32)cycle, cycle == X86_CORE_CPU_SPECIAL_SHUTDOWN ? 0x0eu : 0x0bu);
        if (status != LIB_STATUS_OK) return status;
    } else if (machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_CPU ||
        machine->transaction.kind != CORE_MACHINE_TRANSACTION_CPU_SPECIAL_CYCLE ||
        machine->transaction.value != (lib_u32)cycle) return LIB_STATUS_INVALID_STATE;
    if (machine->attachment.special_cycle != LIB_NULL) {
        status = machine->attachment.special_cycle(machine->attachment.context, cycle, &ready);
        if (status != LIB_STATUS_OK) return status;
    }
    if (!ready || (machine->transaction_contract.cpu_cycle_bus_ready_gate_enabled &&
        !machine->cpu_cycle_bus_ready)) return LIB_STATUS_OK;
    if (machine->attachment.complete_special_cycle != LIB_NULL)
        machine->attachment.complete_special_cycle(machine->attachment.context, cycle, LIB_TRUE);
    core_machine_transaction_commit(&machine->transaction);
    *acknowledged = LIB_TRUE;
    return LIB_STATUS_OK;
}

static void core_machine_cpu_bus_cancel_special_cycle(void *opaque,
    x86_core_cpu_special_cycle cycle)
{
    core_machine *machine = opaque;

    if (machine == LIB_NULL || machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_CPU ||
        machine->transaction.kind != CORE_MACHINE_TRANSACTION_CPU_SPECIAL_CYCLE ||
        machine->transaction.value != (lib_u32)cycle) return;
    if (machine->attachment.complete_special_cycle != LIB_NULL)
        machine->attachment.complete_special_cycle(machine->attachment.context, cycle, LIB_FALSE);
    core_machine_transaction_cancel(&machine->transaction);
}

const x86_core_cpu_bus_provider core_machine_cpu_bus = {
    .read_memory = core_machine_cpu_bus_read_memory,
    .write_memory = core_machine_cpu_bus_write_memory,
    .transfer_port = core_machine_cpu_bus_transfer_port,
    .complete_port = core_machine_cpu_bus_complete_port,
    .wait_test_asserted = core_machine_cpu_bus_wait_test_asserted,
    .escape_trap_enabled = core_machine_cpu_bus_escape_trap_enabled,
    .interrupt_pending = core_machine_cpu_bus_interrupt_pending,
    .acknowledge_interrupt = core_machine_cpu_bus_acknowledge_interrupt,
    .extension_command = core_machine_cpu_bus_extension_command,
    .query_cache_memory = core_machine_cpu_bus_query_cache_memory,
    .cache_operation = core_machine_cpu_bus_cache_operation,
    .begin_memory_lock = core_machine_cpu_bus_begin_memory_lock,
    .complete_memory_lock = core_machine_cpu_bus_complete_memory_lock,
    .system_management_active = core_machine_cpu_bus_system_management_active,
    .special_cycle = core_machine_cpu_bus_special_cycle,
    .cancel_special_cycle = core_machine_cpu_bus_cancel_special_cycle
};

/* Board bus arbitration, wait windows and trace publication are not CPU state. */
void core_machine_transaction_trace(void *opaque,
    core_machine_transaction_owner owner, core_machine_transaction_kind kind,
    core_machine_transaction_phase phase, lib_u32 address,
    lib_u32 value, lib_u32 detail)
{
    core_machine *machine = (core_machine *)opaque;
    core_machine_trace_event_type type;

    if (machine == LIB_NULL) return;
    /* Generic-AT policy: an acknowledged DMA bus handoff breaks CPU-side
     * locality. D4 establishes the HOLD/HLDA topology, not this page-retention
     * behavior or any physical phase duration. */
    if (phase == CORE_MACHINE_TRANSACTION_PHASE_HOLD_ACKNOWLEDGE &&
        owner == CORE_MACHINE_TRANSACTION_OWNER_DMA) {
        core_machine_external_cycle_invalidate(machine);
    }
    switch (phase) {
    case CORE_MACHINE_TRANSACTION_PHASE_BEGIN:
        type = CORE_MACHINE_TRACE_TRANSACTION_BEGIN;
        break;
    case CORE_MACHINE_TRANSACTION_PHASE_COMMIT:
        type = CORE_MACHINE_TRACE_TRANSACTION_COMMIT;
        break;
    case CORE_MACHINE_TRANSACTION_PHASE_CANCEL:
        type = CORE_MACHINE_TRACE_TRANSACTION_CANCEL;
        break;
    case CORE_MACHINE_TRANSACTION_PHASE_HOLD_REQUEST:
        type = CORE_MACHINE_TRACE_TRANSACTION_HOLD_REQUEST;
        break;
    case CORE_MACHINE_TRANSACTION_PHASE_HOLD_ACKNOWLEDGE:
        type = CORE_MACHINE_TRACE_TRANSACTION_HOLD_ACKNOWLEDGE;
        break;
    case CORE_MACHINE_TRANSACTION_PHASE_HOLD_RELEASE:
        type = CORE_MACHINE_TRACE_TRANSACTION_HOLD_RELEASE;
        break;
    default:
        return;
    }
    core_machine_trace_record(machine, type, address, value,
        (lib_u32)owner | ((lib_u32)kind << 8u) |
        (detail << 16u));
}

static lib_i32 core_machine_external_cycle_access_is_chargeable(lib_u8 write,
    x86_core_cpu_memory_access_provenance provenance)
{
    return (!write && (provenance == X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH ||
        provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA ||
        provenance == X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ)) ||
        (write && (provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA ||
        provenance == X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_WRITE));
}

static lib_i32 core_machine_external_cycle_pending_matches(const core_machine *machine,
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

static lib_u32 core_machine_external_access_wait_ticks(
    const core_machine *machine, x86_core_cpu_external_cycle_space space,
    lib_u32 address)
{
    lib_size index;

    if (machine == LIB_NULL) return 0u;
    for (index = 0u; index < CORE_MACHINE_EXTERNAL_ACCESS_WAIT_WINDOW_CAPACITY;
            ++index) {
        const core_machine_external_access_wait_window *window =
            &machine->transaction_contract.external_access_wait_windows[index];
        if (window->wait_ticks != 0u && window->space == space &&
            address >= window->first_address && address <= window->last_address) {
            return window->wait_ticks;
        }
    }
    return 0u;
}

void core_machine_external_cycle_invalidate(core_machine *machine)
{
    if (machine == LIB_NULL) return;
    machine->external_cycle_page_valid = LIB_FALSE;
    machine->external_cycle_pending_valid = LIB_FALSE;
    machine->external_cycle_overlap_valid = LIB_FALSE;
}

void core_machine_cpu_external_cycle_trace(void *opaque,
    x86_core_cpu_external_cycle_phase phase,
    x86_core_cpu_external_cycle_space space, lib_u32 address,
    lib_u8 bytes, lib_u8 write,
    x86_core_cpu_memory_access_provenance provenance)
{
    core_machine *machine = (core_machine *)opaque;
    core_machine_trace_event_type type;
    lib_i32 pending_matches;
    lib_u8 page_timing_enabled;

    if (machine == LIB_NULL) return;
    page_timing_enabled = space == X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY &&
        machine->transaction_contract.external_cycle_timing.page_bytes != 0u &&
        ((machine->transaction_contract.external_cycle_timing.first_eligible_address == 0u &&
          machine->transaction_contract.external_cycle_timing.last_eligible_address == 0u) ||
         (address >= machine->transaction_contract.external_cycle_timing.first_eligible_address &&
          address <= machine->transaction_contract.external_cycle_timing.last_eligible_address));
    switch (phase) {
    case X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN:
        if (machine->external_cycle_pending_valid) {
            core_machine_external_cycle_invalidate(machine);
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
        type = CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_BEGIN;
        break;
    case X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_OVERLAP_DECLARE:
        if (page_timing_enabled && machine->transaction_contract.external_cycle_timing.overlap_policy ==
                CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_EXPLICIT_SEQUENTIAL &&
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
        type = CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_OVERLAP_DECLARE;
        break;
    case X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT:
        pending_matches = core_machine_external_cycle_pending_matches(machine,
            space, address, bytes, write, provenance);
        if (pending_matches && page_timing_enabled &&
            core_machine_external_cycle_access_is_chargeable(write, provenance)) {
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
            if (LIB_UINT64_MAX - machine->external_cycle_round_ticks < wait_ticks) {
                machine->external_cycle_round_overflow = LIB_TRUE;
            } else {
                machine->external_cycle_round_ticks += wait_ticks;
            }
        }
        if (pending_matches) {
            lib_u32 wait_ticks = core_machine_external_access_wait_ticks(
                machine, space, address);
            if (LIB_UINT64_MAX - machine->external_cycle_round_ticks < wait_ticks) {
                machine->external_cycle_round_overflow = LIB_TRUE;
            } else {
                machine->external_cycle_round_ticks += wait_ticks;
            }
            machine->external_cycle_pending_valid = LIB_FALSE;
        }
        type = CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_COMMIT;
        break;
    case X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL:
        if (core_machine_external_cycle_pending_matches(machine, space, address,
                bytes, write, provenance)) {
            core_machine_external_cycle_invalidate(machine);
        }
        type = CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_CANCEL;
        break;
    default:
        return;
    }
    core_machine_trace_record(machine, type, address, bytes,
        (lib_u32)provenance);
}
