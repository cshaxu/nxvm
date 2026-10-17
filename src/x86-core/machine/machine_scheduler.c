#include "lib/types/types_interface.h"

#include "x86-core/machine/machine.h"

lib_status x86_core_machine_set_dma_bus_ready(x86_core_machine *machine, lib_i32 ready)
{
    if (machine == LIB_NULL || !x86_core_machine_mutable_operation_is_allowed(machine) ||
        !machine->transaction_contract.dma_cycle_bus_ready_gate_enabled) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    machine->dma_cycle_bus_ready = ready ? LIB_TRUE : LIB_FALSE;
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_set_cpu_bus_ready(x86_core_machine *machine, lib_i32 ready)
{
    if (machine == LIB_NULL || !x86_core_machine_mutable_operation_is_allowed(machine) ||
        !machine->transaction_contract.cpu_cycle_bus_ready_gate_enabled) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    machine->cpu_cycle_bus_ready = ready ? LIB_TRUE : LIB_FALSE;
    return LIB_STATUS_OK;
}

static lib_u8 x86_core_machine_deadline_consider_absolute(const x86_core_machine *machine,
    lib_u64 due_tick, lib_u64 *io_source_ticks)
{
    lib_u64 source_ticks;

    if (machine == LIB_NULL || io_source_ticks == LIB_NULL) return LIB_FALSE;
    if (due_tick <= machine->elapsed_ticks) return LIB_TRUE;
    source_ticks = due_tick - machine->elapsed_ticks;
    if (*io_source_ticks == 0u || source_ticks < *io_source_ticks) {
        *io_source_ticks = source_ticks;
    }
    return LIB_FALSE;
}

static void x86_core_machine_capture_time_with_board(const x86_core_machine *machine,
    x86_core_machine_time_observation *out_observation,
    x86_core_machine_attachment_deadline_observation *out_board)
{
    x86_core_machine_attachment_deadline_observation board = {0};
    lib_u64 source_ticks = 0u;
    lib_u64 device_ticks;
    lib_u64 timeline_due_tick;
    lib_u8 immediate_due = LIB_FALSE;

    if (machine == LIB_NULL || out_observation == LIB_NULL) return;
    out_observation->elapsed_ticks = machine->elapsed_ticks;
    out_observation->next_deadline_tick = 0u;
    out_observation->pacing_ticks_per_second = 0u;
    out_observation->physical_ticks_per_second = 0u;
    out_observation->next_deadline_valid = LIB_FALSE;
    out_observation->pacing_time_available = LIB_FALSE;
    out_observation->physical_time_available = LIB_FALSE;
    out_observation->progress_disposition = X86_CORE_MACHINE_TIME_PROGRESS_IDLE;
    if (machine->time_axis.kind == X86_CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL ||
        machine->time_axis.kind == X86_CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL) {
        out_observation->pacing_ticks_per_second = machine->time_axis.ticks_per_second;
        out_observation->pacing_time_available = LIB_TRUE;
    }
    if (machine->time_axis.kind == X86_CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL) {
        out_observation->physical_ticks_per_second = machine->time_axis.ticks_per_second;
        out_observation->physical_time_available = LIB_TRUE;
    }
    if (x86_core_machine_timeline_next_due(&machine->timeline, &timeline_due_tick) ==
        LIB_STATUS_OK) {
        if (x86_core_machine_deadline_consider_absolute(machine, timeline_due_tick,
                &source_ticks)) immediate_due = LIB_TRUE;
    }
    if (x86_fpu_ticks_until_completion(machine->fpu, &device_ticks) ==
        LIB_STATUS_OK) {
        if (device_ticks <= LIB_UINT64_MAX - machine->elapsed_ticks &&
            x86_core_machine_deadline_consider_absolute(machine,
                machine->elapsed_ticks + device_ticks, &source_ticks)) {
            immediate_due = LIB_TRUE;
        }
    }
    if (x86_core_cpu_execution_wait_poll_ticks(
            machine->executor_cpu_execution, &device_ticks) == LIB_STATUS_OK) {
        if (device_ticks <= LIB_UINT64_MAX - machine->elapsed_ticks &&
            x86_core_machine_deadline_consider_absolute(machine,
                machine->elapsed_ticks + device_ticks, &source_ticks)) {
            immediate_due = LIB_TRUE;
        }
    }
    if (machine->refresh_cycle_ticks_remaining != 0u &&
        (source_ticks == 0u || machine->refresh_cycle_ticks_remaining < source_ticks))
        source_ticks = machine->refresh_cycle_ticks_remaining;
    if (machine->attachment.deadline != LIB_NULL)
        machine->attachment.deadline(machine->attachment.context,
            machine->elapsed_ticks, &board);
    if (out_board != LIB_NULL) *out_board = board;
    if (board.immediate_due) immediate_due = LIB_TRUE;
    if (board.source_ticks != 0u &&
        (source_ticks == 0u || board.source_ticks < source_ticks))
        source_ticks = board.source_ticks;
    if (immediate_due) {
        out_observation->progress_disposition = X86_CORE_MACHINE_TIME_PROGRESS_IMMEDIATE;
        return;
    }
    if (board.l1_compatibility) {
        /* An unsourced owner may change before an unrelated deadline.  Its
         * bounded Core progression therefore takes precedence without
         * inventing a device duration or exposing controller state. */
        out_observation->progress_disposition = X86_CORE_MACHINE_TIME_PROGRESS_L1_COMPATIBILITY;
        return;
    }
    if (source_ticks != 0u) {
        out_observation->progress_disposition = X86_CORE_MACHINE_TIME_PROGRESS_DEADLINE;
    }
    if (board.fast_advance_blocked) return;
    if (source_ticks != 0u && source_ticks <= LIB_UINT64_MAX - machine->elapsed_ticks) {
        out_observation->next_deadline_tick = machine->elapsed_ticks + source_ticks;
        out_observation->next_deadline_valid = LIB_TRUE;
    }
}

void x86_core_machine_capture_time_observation_private(const x86_core_machine *machine,
    x86_core_machine_time_observation *out_observation)
{
    x86_core_machine_capture_time_with_board(machine, out_observation, LIB_NULL);
}

static void x86_core_machine_dma_grant_advance(x86_core_machine *machine)
{
    if (machine == LIB_NULL) return;
    if ((machine->cpu_profile == X86_CORE_CPU_PROFILE_80286 ||
        machine->cpu_profile == X86_CORE_CPU_PROFILE_80386 ||
        machine->cpu_profile == X86_CORE_CPU_PROFILE_80486SX) &&
        machine->attachment.dma_request != LIB_NULL &&
        machine->attachment.dma_request(machine->attachment.context) &&
        x86_core_machine_transaction_hold_request(&machine->transaction,
            X86_CORE_MACHINE_TRANSACTION_OWNER_DMA, 0u) == LIB_STATUS_OK) {
        if (x86_core_machine_transaction_hold_acknowledge(&machine->transaction,
                X86_CORE_MACHINE_TRANSACTION_OWNER_DMA) == LIB_STATUS_OK) {
            if (machine->attachment.dma_advance != LIB_NULL)
                machine->attachment.dma_advance(machine->attachment.context, 1u);
        }
        x86_core_machine_transaction_hold_release(&machine->transaction,
            X86_CORE_MACHINE_TRANSACTION_OWNER_DMA);
    } else {
        if (machine->attachment.dma_advance != LIB_NULL)
            machine->attachment.dma_advance(machine->attachment.context, 1u);
    }
}
void x86_core_machine_refresh_hold_advance(x86_core_machine *machine, lib_u64 source_ticks)
{
    lib_u8 address;

    if (machine == LIB_NULL) return;
    if (machine->refresh_cycle_ticks_remaining != 0u) {
        if (source_ticks < machine->refresh_cycle_ticks_remaining) {
            machine->refresh_cycle_ticks_remaining -= (lib_u32)source_ticks;
            return;
        }
        machine->refresh_cycle_ticks_remaining = 0u;
        x86_core_machine_transaction_commit(&machine->transaction);
        if (machine->attachment.refresh_complete != LIB_NULL)
            machine->attachment.refresh_complete(machine->attachment.context);
        x86_core_machine_transaction_hold_release(&machine->transaction,
            X86_CORE_MACHINE_TRANSACTION_OWNER_REFRESH);
        return;
    }
    if (machine->attachment.refresh_request == LIB_NULL ||
        !machine->attachment.refresh_request(machine->attachment.context, &address)) return;
    if (x86_core_machine_transaction_hold_request(&machine->transaction,
            X86_CORE_MACHINE_TRANSACTION_OWNER_REFRESH, address) !=
        LIB_STATUS_OK) return;
    if (x86_core_machine_transaction_hold_acknowledge(&machine->transaction,
            X86_CORE_MACHINE_TRANSACTION_OWNER_REFRESH) == LIB_STATUS_OK &&
        x86_core_machine_transaction_begin(&machine->transaction,
            X86_CORE_MACHINE_TRANSACTION_OWNER_REFRESH,
            X86_CORE_MACHINE_TRANSACTION_REFRESH_MEMORY_CYCLE,
            address, 0u, 0u) == LIB_STATUS_OK) {
        /* Core models bus occupation, not DRAM charge retention. */
        machine->refresh_cycle_ticks_remaining =
            machine->transaction_contract.refresh_cycle_source_ticks;
        if (machine->refresh_cycle_ticks_remaining != 0u) return;
        x86_core_machine_transaction_commit(&machine->transaction);
        if (machine->attachment.refresh_complete != LIB_NULL)
            machine->attachment.refresh_complete(machine->attachment.context);
    }
    x86_core_machine_transaction_hold_release(&machine->transaction,
        X86_CORE_MACHINE_TRANSACTION_OWNER_REFRESH);
}
static void x86_core_machine_arbitration_advance(x86_core_machine *machine,
    lib_u64 source_ticks)
{
    lib_u64 dma_ticks;
    lib_u8 refresh_pending;
    lib_u8 refresh_address;
    lib_bool refresh_occupied;
    lib_bool bus_used;

    if (machine == LIB_NULL || source_ticks == 0u) return;
    machine->bus_invalidation_cycles = 0u;
    dma_ticks = machine->attachment.dma_ticks != LIB_NULL ?
        machine->attachment.dma_ticks(machine->attachment.context, source_ticks) : 0u;
    refresh_pending = machine->attachment.refresh_request != LIB_NULL &&
        machine->attachment.refresh_request(machine->attachment.context,
            &refresh_address);
    refresh_occupied = machine->refresh_cycle_ticks_remaining != 0u;
    x86_core_machine_refresh_hold_advance(machine, source_ticks);
    refresh_occupied = refresh_occupied || machine->refresh_cycle_ticks_remaining != 0u;
    bus_used = refresh_pending || refresh_occupied;
    if (refresh_occupied) {
        /* The clock still advances; occupied intervals cannot become a later
         * DMA grant or advance a DMA service phase. */
    } else if (machine->transaction_contract.dma_cycle_wait_quanta != 0u && dma_ticks != 0u) {
        lib_u64 tick;
        for (tick = 0u; tick < dma_ticks; ++tick) {
            if (machine->attachment.dma_request != LIB_NULL &&
                machine->attachment.dma_request(machine->attachment.context)) {
                bus_used = LIB_TRUE;
                if (machine->transaction_contract.dma_cycle_bus_ready_gate_enabled &&
                    !machine->dma_cycle_bus_ready) {
                    continue;
                }
                if (machine->dma_cycle_wait_remaining <
                    machine->transaction_contract.dma_cycle_wait_quanta) {
                    ++machine->dma_cycle_wait_remaining;
                } else {
                    x86_core_machine_dma_grant_advance(machine);
                    machine->dma_cycle_wait_remaining = 0u;
                }
            }
        }
    } else if ((machine->cpu_profile == X86_CORE_CPU_PROFILE_80286 ||
        machine->cpu_profile == X86_CORE_CPU_PROFILE_80386 ||
        machine->cpu_profile == X86_CORE_CPU_PROFILE_80486SX) &&
        dma_ticks != 0u &&
        machine->attachment.dma_request != LIB_NULL &&
        machine->attachment.dma_request(machine->attachment.context) &&
        x86_core_machine_transaction_hold_request(&machine->transaction,
            X86_CORE_MACHINE_TRANSACTION_OWNER_DMA, 0u) == LIB_STATUS_OK) {
        bus_used = LIB_TRUE;
        if (x86_core_machine_transaction_hold_acknowledge(&machine->transaction,
                X86_CORE_MACHINE_TRANSACTION_OWNER_DMA) == LIB_STATUS_OK) {
            if (machine->attachment.dma_advance != LIB_NULL)
                machine->attachment.dma_advance(machine->attachment.context, dma_ticks);
        }
        x86_core_machine_transaction_hold_release(&machine->transaction,
            X86_CORE_MACHINE_TRANSACTION_OWNER_DMA);
    } else {
        if (machine->attachment.dma_advance != LIB_NULL)
            machine->attachment.dma_advance(machine->attachment.context, dma_ticks);
    }
    x86_core_cpu_execution_advance_bus(machine->executor_cpu_execution, source_ticks,
        !bus_used && (!machine->transaction_contract.cpu_cycle_bus_ready_gate_enabled ||
            machine->cpu_cycle_bus_ready) &&
        machine->transaction.owner == X86_CORE_MACHINE_TRANSACTION_OWNER_NONE &&
        machine->transaction.hold_owner == X86_CORE_MACHINE_TRANSACTION_OWNER_NONE,
        machine->cpu_retirement_source_ticks != 0u || machine->cpu_retirement_wait_pending ?
            machine->bus_invalidation_cycles : 0u);
    if (machine->transaction_contract.cpu_prefetch_reservation_enabled && !refresh_pending &&
        (machine->attachment.refresh_request == LIB_NULL ||
         !machine->attachment.refresh_request(machine->attachment.context,
             &refresh_address)) &&
        (machine->attachment.dma_request == LIB_NULL ||
         !machine->attachment.dma_request(machine->attachment.context)) &&
        machine->transaction.owner == X86_CORE_MACHINE_TRANSACTION_OWNER_NONE &&
        machine->transaction.hold_owner == X86_CORE_MACHINE_TRANSACTION_OWNER_NONE) {
        x86_core_cpu_execution_advance_prefetch_reservation(
            machine->executor_cpu_execution);
    }
    if (dma_ticks != 0u) {
        x86_core_machine_trace_record(machine, X86_CORE_MACHINE_TRACE_DMA_ADVANCE,
            0u, (lib_u32)dma_ticks, 0u);
    }
}

static void x86_core_machine_internal_advance(x86_core_machine *machine,
    lib_u64 source_ticks)
{
    if (machine == LIB_NULL || source_ticks == 0u) return;
    x86_fpu_advance(machine->fpu, source_ticks);
    x86_core_cpu_execution_advance_wait(machine->executor_cpu_execution,
        source_ticks);
}

static void x86_core_machine_advance_scheduler(x86_core_machine *machine,
    lib_u64 elapsed_ticks)
{
    lib_u64 provider_ticks;
    lib_u64 target_tick;

    if (machine == LIB_NULL || elapsed_ticks == 0u ||
        LIB_UINT64_MAX - machine->elapsed_ticks < elapsed_ticks) {
        return;
    }
    target_tick = machine->elapsed_ticks + elapsed_ticks;
    while (machine->elapsed_ticks < target_tick) {
        x86_core_machine_time_observation observation;
        x86_core_machine_attachment_deadline_observation board;
        lib_u64 due_tick = target_tick;
        lib_u64 source_ticks;

        if (machine->transaction_contract.refresh_cycle_source_ticks != 0u)
            x86_core_machine_refresh_hold_advance(machine, 0u);
        x86_core_machine_capture_time_with_board(machine, &observation, &board);
        if (board.l1_compatibility) {
            due_tick = machine->elapsed_ticks + 1u;
        } else if (observation.next_deadline_valid &&
            observation.next_deadline_tick > machine->elapsed_ticks &&
            observation.next_deadline_tick < due_tick) {
            due_tick = observation.next_deadline_tick;
        } else if (board.fast_advance_blocked) {
            /* An active L1 owner blocks fast advance, but a successful CPU
             * retirement still advances its existing causal route one tick. */
            due_tick = machine->elapsed_ticks + 1u;
        }
        source_ticks = due_tick - machine->elapsed_ticks;
        machine->elapsed_ticks = due_tick;
        (void)x86_core_machine_timeline_advance(&machine->timeline, due_tick);
        x86_core_machine_arbitration_advance(machine, source_ticks);
        if (machine->attachment.before_internal_advance != LIB_NULL)
            machine->attachment.before_internal_advance(machine->attachment.context,
                source_ticks, due_tick);
        x86_core_machine_internal_advance(machine, source_ticks);
        if (machine->attachment.after_internal_advance != LIB_NULL)
            machine->attachment.after_internal_advance(machine->attachment.context,
                source_ticks, due_tick);
    }
    provider_ticks = x86_core_machine_clock_domain_advance(&machine->provider_clock,
        elapsed_ticks);
    if (machine->execution_provider != LIB_NULL &&
        machine->execution_provider->advance_time != LIB_NULL) {
        machine->execution_provider->advance_time(
            machine->execution_provider_context, provider_ticks);
    }
}

lib_status x86_core_machine_publish_elapsed_ticks(x86_core_machine *machine,
    lib_u64 elapsed_ticks, x86_core_machine_time_publication_origin origin)
{
    if (machine == LIB_NULL || elapsed_ticks == 0u ||
        LIB_UINT64_MAX - machine->elapsed_ticks < elapsed_ticks) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    /* A partial instruction duration requires the same source qualification
     * as its final retirement. Event and external-wait origins stay closed. */
    if (machine->retirement_time_contract == X86_CORE_MACHINE_RETIREMENT_TIME_PHYSICAL &&
        origin != X86_CORE_MACHINE_TIME_PUBLICATION_CPU_RETIREMENT &&
        origin != X86_CORE_MACHINE_TIME_PUBLICATION_DEADLINE &&
        (origin != X86_CORE_MACHINE_TIME_PUBLICATION_CPU_EXECUTION ||
         !x86_core_machine_retirement_time_is_qualified(machine))) {
        return LIB_STATUS_INVALID_STATE;
    }
    if (origin == X86_CORE_MACHINE_TIME_PUBLICATION_CPU_RETIREMENT) {
        x86_core_machine_trace_record(machine, X86_CORE_MACHINE_TRACE_CPU_RETIRE,
            x86_core_machine_linear_pc(machine), (lib_u32)(machine->cpu_profile ==
                X86_CORE_CPU_PROFILE_80486SX ? machine->cpu_retirement_source_ticks :
                elapsed_ticks), 0u);
    } else {
        x86_core_machine_trace_record(machine, X86_CORE_MACHINE_TRACE_EXTERNAL_TIME,
            0u, (lib_u32)elapsed_ticks, 0u);
    }
    x86_core_machine_advance_scheduler(machine, elapsed_ticks);
    return LIB_STATUS_OK;
}
