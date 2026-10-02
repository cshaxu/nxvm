#include "lib/types/types_interface.h"

#include "app-nxvm/devices/machine.h"

static lib_u8 core_machine_deadline_consider_absolute(const core_machine *machine,
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

static void core_machine_capture_time_with_board(const core_machine *machine,
    core_machine_time_observation *out_observation,
    core_machine_board_deadline_observation *out_board)
{
    core_machine_board_deadline_observation board = {0};
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
    out_observation->progress_disposition = CORE_MACHINE_TIME_PROGRESS_IDLE;
    if (machine->time_axis.kind == CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL ||
        machine->time_axis.kind == CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL) {
        out_observation->pacing_ticks_per_second = machine->time_axis.ticks_per_second;
        out_observation->pacing_time_available = LIB_TRUE;
    }
    if (machine->time_axis.kind == CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL) {
        out_observation->physical_ticks_per_second = machine->time_axis.ticks_per_second;
        out_observation->physical_time_available = LIB_TRUE;
    }
    if (core_machine_timeline_next_due(&machine->timeline, &timeline_due_tick) ==
        LIB_STATUS_OK) {
        if (core_machine_deadline_consider_absolute(machine, timeline_due_tick,
                &source_ticks)) immediate_due = LIB_TRUE;
    }
    if (x86_fpu_ticks_until_completion(machine->fpu, &device_ticks) ==
        LIB_STATUS_OK) {
        if (device_ticks <= UINT64_MAX - machine->elapsed_ticks &&
            core_machine_deadline_consider_absolute(machine,
                machine->elapsed_ticks + device_ticks, &source_ticks)) {
            immediate_due = LIB_TRUE;
        }
    }
    if (machine->board_deadline_provider != LIB_NULL)
        machine->board_deadline_provider(machine->board_owner,
            machine->elapsed_ticks, &board);
    if (out_board != LIB_NULL) *out_board = board;
    if (board.immediate_due) immediate_due = LIB_TRUE;
    if (board.source_ticks != 0u &&
        (source_ticks == 0u || board.source_ticks < source_ticks))
        source_ticks = board.source_ticks;
    if (immediate_due) {
        out_observation->progress_disposition = CORE_MACHINE_TIME_PROGRESS_IMMEDIATE;
        return;
    }
    if (board.l1_compatibility) {
        /* An unsourced owner may change before an unrelated deadline.  Its
         * bounded Core progression therefore takes precedence without
         * inventing a device duration or exposing controller state. */
        out_observation->progress_disposition = CORE_MACHINE_TIME_PROGRESS_L1_COMPATIBILITY;
        return;
    }
    if (source_ticks != 0u) {
        out_observation->progress_disposition = CORE_MACHINE_TIME_PROGRESS_DEADLINE;
    }
    if (board.fast_advance_blocked) return;
    if (source_ticks != 0u && source_ticks <= UINT64_MAX - machine->elapsed_ticks) {
        out_observation->next_deadline_tick = machine->elapsed_ticks + source_ticks;
        out_observation->next_deadline_valid = LIB_TRUE;
    }
}

void core_machine_capture_time_observation_private(const core_machine *machine,
    core_machine_time_observation *out_observation)
{
    core_machine_capture_time_with_board(machine, out_observation, LIB_NULL);
}

static void core_machine_dma_grant_advance(core_machine *machine)
{
    if (machine == LIB_NULL) return;
    if ((machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ||
        machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386) &&
        core_machine_dma_has_pending_request(&machine->shared_dma_primary,
            &machine->shared_dma_secondary) &&
        core_machine_transaction_hold_request(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_DMA, 0u) == LIB_STATUS_OK) {
        if (core_machine_transaction_hold_acknowledge(&machine->transaction,
                CORE_MACHINE_TRANSACTION_OWNER_DMA) == LIB_STATUS_OK) {
            core_machine_dma_advance_transaction(&machine->shared_dma_latch,
                &machine->shared_dma_primary, &machine->shared_dma_secondary,
                machine, 1u);
        }
        core_machine_transaction_hold_release(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_DMA);
    } else {
        core_machine_dma_advance_transaction(&machine->shared_dma_latch,
            &machine->shared_dma_primary, &machine->shared_dma_secondary,
            machine, 1u);
    }
}
static void core_machine_d4_refresh_hold_advance(core_machine *machine)
{
    if (machine == LIB_NULL || !machine->d4_refresh_hold_pending) return;
    if (core_machine_transaction_hold_request(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_REFRESH, machine->d4_refresh_address) !=
        LIB_STATUS_OK) return;
    if (core_machine_transaction_hold_acknowledge(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_REFRESH) == LIB_STATUS_OK &&
        core_machine_transaction_begin(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_REFRESH,
            CORE_MACHINE_TRANSACTION_REFRESH_MEMORY_CYCLE,
            machine->d4_refresh_address, 0u, 0u) == LIB_STATUS_OK) {
        /* Bus occupation only: Core has no DRAM electrical refresh model. */
        core_machine_transaction_commit(&machine->transaction);
        machine->d4_refresh_address = (lib_u8)(machine->d4_refresh_address + 1u);
        machine->d4_refresh_hold_pending = LIB_FALSE;
    }
    core_machine_transaction_hold_release(&machine->transaction,
        CORE_MACHINE_TRANSACTION_OWNER_REFRESH);
}
static void core_machine_arbitration_advance(core_machine *machine,
    lib_u64 source_ticks)
{
    lib_u64 dma_ticks;
    lib_u64 pit_ticks;
    lib_u64 auxiliary_pit_ticks;
    lib_u8 refresh_pending;

    if (machine == LIB_NULL || source_ticks == 0u) return;
    dma_ticks = core_machine_clock_domain_advance(&machine->dma_clock, source_ticks);
    pit_ticks = core_machine_clock_domain_advance(&machine->pit_clock, source_ticks);
    auxiliary_pit_ticks = core_machine_clock_domain_advance(
        &machine->auxiliary_pit_clock, source_ticks);
    refresh_pending = machine->d4_refresh_hold_pending;
    core_machine_d4_refresh_hold_advance(machine);
    if (machine->transaction_contract.dma_cycle_wait_quanta != 0u && dma_ticks != 0u) {
        lib_u64 tick;
        for (tick = 0u; tick < dma_ticks; ++tick) {
            if (core_machine_dma_has_pending_request(&machine->shared_dma_primary,
                    &machine->shared_dma_secondary)) {
                if (machine->transaction_contract.dma_cycle_bus_ready_gate_enabled &&
                    !machine->dma_cycle_bus_ready) {
                    continue;
                }
                if (machine->dma_cycle_wait_remaining <
                    machine->transaction_contract.dma_cycle_wait_quanta) {
                    ++machine->dma_cycle_wait_remaining;
                } else {
                    core_machine_dma_grant_advance(machine);
                    machine->dma_cycle_wait_remaining = 0u;
                }
            }
        }
    } else if ((machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ||
        machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386) &&
        dma_ticks != 0u &&
        core_machine_dma_has_pending_request(&machine->shared_dma_primary,
            &machine->shared_dma_secondary) &&
        core_machine_transaction_hold_request(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_DMA, 0u) == LIB_STATUS_OK) {
        if (core_machine_transaction_hold_acknowledge(&machine->transaction,
                CORE_MACHINE_TRANSACTION_OWNER_DMA) == LIB_STATUS_OK) {
            core_machine_dma_advance_transaction(&machine->shared_dma_latch,
                &machine->shared_dma_primary, &machine->shared_dma_secondary,
                machine, dma_ticks);
        }
        core_machine_transaction_hold_release(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_DMA);
    } else {
        core_machine_dma_advance_transaction(&machine->shared_dma_latch,
            &machine->shared_dma_primary, &machine->shared_dma_secondary,
            machine, dma_ticks);
    }
    if (machine->transaction_contract.cpu_prefetch_reservation_enabled && !refresh_pending &&
        !machine->d4_refresh_hold_pending &&
        !core_machine_dma_has_pending_request(&machine->shared_dma_primary,
            &machine->shared_dma_secondary) &&
        machine->transaction.owner == CORE_MACHINE_TRANSACTION_OWNER_NONE &&
        machine->transaction.hold_owner == CORE_MACHINE_TRANSACTION_OWNER_NONE) {
        core_machine_cpu_execution_advance_prefetch_reservation(
            machine->executor_cpu_execution);
    }
    if (dma_ticks != 0u) {
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_DMA_ADVANCE,
            0u, (lib_u32)dma_ticks, 0u);
    }
    x86_pit_advance(machine->shared_pit.device, pit_ticks);
    if (machine->auxiliary_pit_configured) {
        x86_pit_advance(machine->auxiliary_pit.device, auxiliary_pit_ticks);
    }
    if (pit_ticks != 0u) {
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_PIT_ADVANCE,
            0u, (lib_u32)pit_ticks, 0u);
    }
    core_machine_pic_refresh(&machine->shared_pic_master,
        &machine->shared_pic_slave);
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_PIC_REFRESH,
        0u, 0u, 0u);
}

/*
 * RTC progression and removable-media observation have a distinct readiness
 * boundary.  This callback intentionally follows the immediate DMA/PIT/PIC
 * arbitration callback at a shared due tick: sources made ready here become
 * eligible for PIC arbitration at the following due tick. FDC and ATA command
 * and completion service are advanced here before their retained observation
 * refresh paths.
 */
static void core_machine_readiness_advance(core_machine *machine,
    lib_u64 source_ticks, lib_u64 due_tick)
{
    if (machine == LIB_NULL || source_ticks == 0u) return;
    if (machine->board_media_provider != LIB_NULL)
        machine->board_media_provider(machine->board_owner, source_ticks, due_tick);
    x86_fpu_advance(machine->fpu, source_ticks);
    if (machine->board_rtc_provider != LIB_NULL)
        machine->board_rtc_provider(machine->board_owner, source_ticks);
}

static void core_machine_advance_scheduler(core_machine *machine,
    lib_u64 elapsed_ticks)
{
    lib_u64 provider_ticks;
    lib_u64 target_tick;

    if (machine == LIB_NULL || elapsed_ticks == 0u ||
        UINT64_MAX - machine->elapsed_ticks < elapsed_ticks) {
        return;
    }
    target_tick = machine->elapsed_ticks + elapsed_ticks;
    while (machine->elapsed_ticks < target_tick) {
        core_machine_time_observation observation;
        core_machine_board_deadline_observation board;
        lib_u64 due_tick = target_tick;
        lib_u64 source_ticks;

        core_machine_capture_time_with_board(machine, &observation, &board);
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
        (void)core_machine_timeline_advance(&machine->timeline, due_tick);
        core_machine_arbitration_advance(machine, source_ticks);
        core_machine_readiness_advance(machine, source_ticks, due_tick);
        if (machine->board_peripheral_provider != LIB_NULL)
            machine->board_peripheral_provider(machine->board_owner, source_ticks);
    }
    provider_ticks = core_machine_clock_domain_advance(&machine->provider_clock,
        elapsed_ticks);
    if (machine->execution_provider != LIB_NULL &&
        machine->execution_provider->advance_time != LIB_NULL) {
        machine->execution_provider->advance_time(
            machine->execution_provider_context, provider_ticks);
    }
}

lib_status core_machine_publish_elapsed_ticks(core_machine *machine,
    lib_u64 elapsed_ticks, core_machine_time_publication_origin origin)
{
    if (machine == LIB_NULL || elapsed_ticks == 0u ||
        UINT64_MAX - machine->elapsed_ticks < elapsed_ticks) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    /* Physical publication is closed to the two owners whose current source
     * rules establish a Core-axis duration. Remaining origins stay blocked
     * until their owner supplies an exact S4 disposition. */
    if (machine->retirement_time_contract == CORE_MACHINE_RETIREMENT_TIME_PHYSICAL &&
        origin != CORE_MACHINE_TIME_PUBLICATION_CPU_RETIREMENT &&
        origin != CORE_MACHINE_TIME_PUBLICATION_DEADLINE) {
        return LIB_STATUS_INVALID_STATE;
    }
    if (origin == CORE_MACHINE_TIME_PUBLICATION_CPU_RETIREMENT) {
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_CPU_RETIRE,
            core_machine_linear_pc(machine), (lib_u32)elapsed_ticks, 0u);
    } else {
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_EXTERNAL_TIME,
            0u, (lib_u32)elapsed_ticks, 0u);
    }
    core_machine_advance_scheduler(machine, elapsed_ticks);
    return LIB_STATUS_OK;
}
