#include "type.h"

#include "core/machine/machine.h"

static C_INT core_machine_dma_deadline_is_available(const core_machine *machine)
{
    return machine != STD_NULL && machine->timing_plan_copied &&
        machine->timing_plan.configuration.clock_plan.dma.numerator != 0u &&
        machine->timing_plan.configuration.clock_plan.dma.denominator != 0u;
}

static C_INT core_machine_deadline_is_blocked(const core_machine *machine)
{
    if (machine == STD_NULL ||
        (core_machine_dma_has_pending_request(&machine->shared_dma_primary,
            &machine->shared_dma_secondary) &&
         !core_machine_dma_deadline_is_available(machine))) {
        return 1;
    }
    return machine->fdc.data.phase == core_machine_fdc_PHASE_PENDING_COMMAND ||
        machine->fdc.data.phase == core_machine_fdc_PHASE_PENDING_COMPLETE;
}

static C_INT core_machine_fast_advance_is_blocked(const core_machine *machine)
{
    return core_machine_deadline_is_blocked(machine);
}

static C_INT core_machine_l1_compatibility_is_eligible(const core_machine *machine)
{
    return machine != STD_NULL &&
        ((core_machine_dma_has_pending_request(&machine->shared_dma_primary,
            &machine->shared_dma_secondary) &&
          !core_machine_dma_deadline_is_available(machine)));
}

static type_bool core_machine_deadline_consider_clock(const core_machine_clock_domain *clock,
    type_unsigned_64 device_ticks, type_unsigned_64 *io_source_ticks)
{
    type_unsigned_64 source_ticks;

    if (device_ticks == 0u) return TYPE_TRUE;
    if (io_source_ticks == STD_NULL ||
        core_machine_clock_domain_source_ticks_until(clock, device_ticks,
            &source_ticks) != TYPE_STATUS_OK) {
        return TYPE_FALSE;
    }
    if (*io_source_ticks == 0u || source_ticks < *io_source_ticks) {
        *io_source_ticks = source_ticks;
    }
    return TYPE_FALSE;
}

static type_bool core_machine_deadline_consider_absolute(const core_machine *machine,
    type_unsigned_64 due_tick, type_unsigned_64 *io_source_ticks)
{
    type_unsigned_64 source_ticks;

    if (machine == STD_NULL || io_source_ticks == STD_NULL) return TYPE_FALSE;
    if (due_tick <= machine->elapsed_ticks) return TYPE_TRUE;
    source_ticks = due_tick - machine->elapsed_ticks;
    if (*io_source_ticks == 0u || source_ticks < *io_source_ticks) {
        *io_source_ticks = source_ticks;
    }
    return TYPE_FALSE;
}

static type_bool core_machine_deadline_consider_pit(const t_pit *pit,
    const core_machine_clock_domain *clock, type_unsigned_64 *io_source_ticks)
{
    type_unsigned_8 counter;
    type_bool immediate_due = TYPE_FALSE;

    for (counter = 0u; counter < 3u; ++counter) {
        type_unsigned_64 device_ticks;

        if (core_machine_pit_ticks_until_output(pit, counter, &device_ticks) ==
            TYPE_STATUS_OK) {
            if (core_machine_deadline_consider_clock(clock, device_ticks,
                    io_source_ticks)) immediate_due = TYPE_TRUE;
        }
    }
    return immediate_due;
}

C_VOID core_machine_capture_time_observation_private(const core_machine *machine,
    core_machine_time_observation *out_observation)
{
    type_unsigned_64 source_ticks = 0u;
    type_unsigned_64 device_ticks;
    type_unsigned_64 fdc_due_tick;
    type_unsigned_64 hdc_due_tick;
    type_unsigned_64 timeline_due_tick;
    type_bool immediate_due = TYPE_FALSE;

    if (machine == STD_NULL || out_observation == STD_NULL) return;
    out_observation->elapsed_ticks = machine->elapsed_ticks;
    out_observation->next_deadline_tick = 0u;
    out_observation->pacing_ticks_per_second = 0u;
    out_observation->physical_ticks_per_second = 0u;
    out_observation->next_deadline_valid = TYPE_FALSE;
    out_observation->pacing_time_available = TYPE_FALSE;
    out_observation->physical_time_available = TYPE_FALSE;
    out_observation->progress_disposition = CORE_MACHINE_TIME_PROGRESS_IDLE;
    if (machine->time_axis.kind == CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL ||
        machine->time_axis.kind == CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL) {
        out_observation->pacing_ticks_per_second = machine->time_axis.ticks_per_second;
        out_observation->pacing_time_available = TYPE_TRUE;
    }
    if (machine->time_axis.kind == CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL) {
        out_observation->physical_ticks_per_second = machine->time_axis.ticks_per_second;
        out_observation->physical_time_available = TYPE_TRUE;
    }
    if (core_machine_timeline_next_due(&machine->timeline, &timeline_due_tick) ==
        TYPE_STATUS_OK) {
        if (core_machine_deadline_consider_absolute(machine, timeline_due_tick,
                &source_ticks)) immediate_due = TYPE_TRUE;
    }
    /* A frozen fallback ratio remains an L2 timing claim, but it is still a
     * usable Core-local conversion.  Provenance decides what we promise about
     * the edge, not whether a programmed PIT may wake a halted guest. */
    if (machine->timing_plan_copied) {
        if (core_machine_deadline_consider_pit(&machine->shared_pit,
                &machine->pit_clock, &source_ticks)) immediate_due = TYPE_TRUE;
        if (machine->auxiliary_pit_configured) {
            if (core_machine_deadline_consider_pit(&machine->auxiliary_pit,
                    &machine->auxiliary_pit_clock, &source_ticks)) immediate_due =
                TYPE_TRUE;
        }
    }
    if (machine->timing_plan_copied && machine->rtc_cmos_configured &&
        core_machine_rtc_ticks_until_irq(&machine->shared_rtc, &device_ticks) ==
            TYPE_STATUS_OK) {
        if (core_machine_deadline_consider_clock(&machine->rtc_clock, device_ticks,
                &source_ticks)) immediate_due = TYPE_TRUE;
    }
    if (core_machine_dma_has_pending_request(&machine->shared_dma_primary,
            &machine->shared_dma_secondary) &&
        core_machine_dma_deadline_is_available(machine)) {
        if (core_machine_deadline_consider_clock(&machine->dma_clock, 1u,
                &source_ticks)) immediate_due = TYPE_TRUE;
    }
    if (core_machine_fdc_next_due_tick(&machine->fdc, &fdc_due_tick) ==
        TYPE_STATUS_OK) {
        if (core_machine_deadline_consider_absolute(machine, fdc_due_tick,
                &source_ticks)) immediate_due = TYPE_TRUE;
    }
    if (core_machine_hdc_next_due_tick(&machine->hdc, &hdc_due_tick) ==
        TYPE_STATUS_OK) {
        if (core_machine_deadline_consider_absolute(machine, hdc_due_tick,
                &source_ticks)) immediate_due = TYPE_TRUE;
    }
    if (core_machine_fpu_ticks_until_completion(&machine->fpu, &device_ticks) ==
        TYPE_STATUS_OK) {
        if (device_ticks <= UINT64_MAX - machine->elapsed_ticks &&
            core_machine_deadline_consider_absolute(machine,
                machine->elapsed_ticks + device_ticks, &source_ticks)) {
            immediate_due = TYPE_TRUE;
        }
    }
    if (machine->d4_refresh_hold_pending &&
        core_machine_deadline_consider_absolute(machine,
            machine->elapsed_ticks + 1u, &source_ticks)) {
        immediate_due = TYPE_TRUE;
    }
    if (core_machine_kbc_ticks_until_event(&machine->shared_kbc, &device_ticks) ==
        TYPE_STATUS_OK) {
        if (core_machine_deadline_consider_clock(&machine->kbc_clock, device_ticks,
                &source_ticks)) immediate_due = TYPE_TRUE;
    }
    if (machine->keyboard_topology == CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI &&
        core_machine_xt_keyboard_ticks_until_event(&machine->xt_keyboard,
            &device_ticks) == TYPE_STATUS_OK) {
        if (device_ticks == 0u) immediate_due = TYPE_TRUE;
        else if (source_ticks == 0u || device_ticks < source_ticks) {
            source_ticks = device_ticks;
        }
    }
    if (immediate_due) {
        out_observation->progress_disposition = CORE_MACHINE_TIME_PROGRESS_IMMEDIATE;
        return;
    }
    if (core_machine_l1_compatibility_is_eligible(machine)) {
        /* An unsourced owner may change before an unrelated deadline.  Its
         * bounded Core progression therefore takes precedence without
         * inventing a device duration or exposing controller state. */
        out_observation->progress_disposition = CORE_MACHINE_TIME_PROGRESS_L1_COMPATIBILITY;
        return;
    }
    if (source_ticks != 0u) {
        out_observation->progress_disposition = CORE_MACHINE_TIME_PROGRESS_DEADLINE;
    }
    if (core_machine_fast_advance_is_blocked(machine)) return;
    if (source_ticks != 0u && source_ticks <= UINT64_MAX - machine->elapsed_ticks) {
        out_observation->next_deadline_tick = machine->elapsed_ticks + source_ticks;
        out_observation->next_deadline_valid = TYPE_TRUE;
    }
}

static C_VOID core_machine_dma_grant_advance(core_machine *machine)
{
    if (machine == STD_NULL) return;
    if ((machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ||
        machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386) &&
        core_machine_dma_has_pending_request(&machine->shared_dma_primary,
            &machine->shared_dma_secondary) &&
        core_machine_transaction_hold_request(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_DMA, 0u) == TYPE_STATUS_OK) {
        if (core_machine_transaction_hold_acknowledge(&machine->transaction,
                CORE_MACHINE_TRANSACTION_OWNER_DMA) == TYPE_STATUS_OK) {
            core_machine_dma_advance_transaction(&machine->shared_dma_latch,
                &machine->shared_dma_primary, &machine->shared_dma_secondary,
                &machine->executor_memory, &machine->transaction, 1u);
        }
        core_machine_transaction_hold_release(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_DMA);
    } else {
        core_machine_dma_advance_transaction(&machine->shared_dma_latch,
            &machine->shared_dma_primary, &machine->shared_dma_secondary,
            &machine->executor_memory, &machine->transaction, 1u);
    }
}
static C_VOID core_machine_d4_refresh_hold_advance(core_machine *machine)
{
    if (machine == STD_NULL || !machine->d4_refresh_hold_pending) return;
    if (core_machine_transaction_hold_request(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_REFRESH, machine->d4_refresh_address) !=
        TYPE_STATUS_OK) return;
    if (core_machine_transaction_hold_acknowledge(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_REFRESH) == TYPE_STATUS_OK &&
        core_machine_transaction_begin(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_REFRESH,
            CORE_MACHINE_TRANSACTION_REFRESH_MEMORY_CYCLE,
            machine->d4_refresh_address, 0u, 0u) == TYPE_STATUS_OK) {
        /* Bus occupation only: Core has no DRAM electrical refresh model. */
        core_machine_transaction_commit(&machine->transaction);
        machine->d4_refresh_address = (type_unsigned_8)(machine->d4_refresh_address + 1u);
        machine->d4_refresh_hold_pending = TYPE_FALSE;
    }
    core_machine_transaction_hold_release(&machine->transaction,
        CORE_MACHINE_TRANSACTION_OWNER_REFRESH);
}
static C_VOID core_machine_arbitration_advance(core_machine *machine,
    type_unsigned_64 source_ticks)
{
    type_unsigned_64 dma_ticks;
    type_unsigned_64 pit_ticks;
    type_unsigned_64 auxiliary_pit_ticks;
    type_bool refresh_pending;

    if (machine == STD_NULL || source_ticks == 0u) return;
    dma_ticks = core_machine_clock_domain_advance(&machine->dma_clock, source_ticks);
    pit_ticks = core_machine_clock_domain_advance(&machine->pit_clock, source_ticks);
    auxiliary_pit_ticks = core_machine_clock_domain_advance(
        &machine->auxiliary_pit_clock, source_ticks);
    refresh_pending = machine->d4_refresh_hold_pending;
    core_machine_d4_refresh_hold_advance(machine);
    if (machine->transaction_contract.dma_cycle_wait_quanta != 0u && dma_ticks != 0u) {
        type_unsigned_64 tick;
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
            CORE_MACHINE_TRANSACTION_OWNER_DMA, 0u) == TYPE_STATUS_OK) {
        if (core_machine_transaction_hold_acknowledge(&machine->transaction,
                CORE_MACHINE_TRANSACTION_OWNER_DMA) == TYPE_STATUS_OK) {
            core_machine_dma_advance_transaction(&machine->shared_dma_latch,
                &machine->shared_dma_primary, &machine->shared_dma_secondary,
                &machine->executor_memory, &machine->transaction, dma_ticks);
        }
        core_machine_transaction_hold_release(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_DMA);
    } else {
        core_machine_dma_advance_transaction(&machine->shared_dma_latch,
            &machine->shared_dma_primary, &machine->shared_dma_secondary,
            &machine->executor_memory, &machine->transaction, dma_ticks);
    }
    if (machine->transaction_contract.cpu_prefetch_reservation_enabled && !refresh_pending &&
        !machine->d4_refresh_hold_pending &&
        !core_machine_dma_has_pending_request(&machine->shared_dma_primary,
            &machine->shared_dma_secondary) &&
        machine->transaction.owner == CORE_MACHINE_TRANSACTION_OWNER_NONE &&
        machine->transaction.hold_owner == CORE_MACHINE_TRANSACTION_OWNER_NONE) {
        core_machine_cpu_execution_advance_prefetch_reservation(
            &machine->executor_cpu_execution);
    }
    if (dma_ticks != 0u) {
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_DMA_ADVANCE,
            0u, (type_unsigned_32)dma_ticks, 0u);
    }
    core_machine_pit_advance(&machine->shared_pit, pit_ticks);
    if (machine->auxiliary_pit_configured) {
        core_machine_pit_advance(&machine->auxiliary_pit, auxiliary_pit_ticks);
    }
    if (pit_ticks != 0u) {
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_PIT_ADVANCE,
            0u, (type_unsigned_32)pit_ticks, 0u);
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
static C_VOID core_machine_readiness_advance(core_machine *machine,
    type_unsigned_64 source_ticks, type_unsigned_64 due_tick)
{
    type_unsigned_64 rtc_ticks;

    if (machine == STD_NULL || source_ticks == 0u) return;
    if (machine->fdc_configured) {
        core_machine_fdc_advance_at(&machine->fdc, due_tick);
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_FDC_ADVANCE,
            0u, 0u, 0u);
    }
    if (machine->hdc_configured) {
        core_machine_hdc_advance_elapsed(&machine->hdc, source_ticks);
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_HDC_ADVANCE,
            0u, 0u, 0u);
    }
    core_machine_fpu_advance(&machine->fpu, source_ticks);
    rtc_ticks = core_machine_clock_domain_advance(&machine->rtc_clock, source_ticks);
    if (machine->rtc_cmos_configured) {
        core_machine_rtc_advance(&machine->shared_rtc, rtc_ticks);
    }
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_RTC_ADVANCE,
        0u, (type_unsigned_32)rtc_ticks, 0u);
}

/*
 * Guest input and video state advance after the readiness boundary.  Host
 * presentation consumes only copied snapshots outside this callback and does
 * not participate in machine time.
 */
static C_VOID core_machine_peripheral_advance(core_machine *machine,
    type_unsigned_64 source_ticks)
{
    type_unsigned_64 kbc_ticks;
    type_unsigned_64 vadp_ticks;

    if (machine == STD_NULL || source_ticks == 0u) return;
    kbc_ticks = core_machine_clock_domain_advance(&machine->kbc_clock, source_ticks);
    if (machine->keyboard_topology == CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI) {
        core_machine_xt_keyboard_advance(&machine->xt_keyboard, source_ticks);
    } else {
        core_machine_kbc_advance(&machine->shared_kbc, kbc_ticks);
    }
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_KBC_ADVANCE,
        0u, (type_unsigned_32)kbc_ticks, 0u);
    vadp_ticks = core_machine_clock_domain_advance(&machine->vadp_clock, source_ticks);
    core_machine_vadp_advance(&machine->shared_vadp, &machine->executor_memory,
        vadp_ticks);
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_VADP_ADVANCE,
        0u, (type_unsigned_32)vadp_ticks, 0u);
}

static C_VOID core_machine_advance_scheduler(core_machine *machine,
    type_unsigned_64 elapsed_ticks)
{
    type_unsigned_64 provider_ticks;
    type_unsigned_64 target_tick;

    if (machine == STD_NULL || elapsed_ticks == 0u ||
        UINT64_MAX - machine->elapsed_ticks < elapsed_ticks) {
        return;
    }
    target_tick = machine->elapsed_ticks + elapsed_ticks;
    while (machine->elapsed_ticks < target_tick) {
        core_machine_time_observation observation;
        type_unsigned_64 due_tick = target_tick;
        type_unsigned_64 source_ticks;

        core_machine_capture_time_observation_private(machine, &observation);
        if (core_machine_l1_compatibility_is_eligible(machine)) {
            due_tick = machine->elapsed_ticks + 1u;
        } else if (observation.next_deadline_valid &&
            observation.next_deadline_tick > machine->elapsed_ticks &&
            observation.next_deadline_tick < due_tick) {
            due_tick = observation.next_deadline_tick;
        } else if (core_machine_fast_advance_is_blocked(machine)) {
            /* An active L1 owner blocks fast advance, but a successful CPU
             * retirement still advances its existing causal route one tick. */
            due_tick = machine->elapsed_ticks + 1u;
        }
        source_ticks = due_tick - machine->elapsed_ticks;
        machine->elapsed_ticks = due_tick;
        (C_VOID)core_machine_timeline_advance(&machine->timeline, due_tick);
        core_machine_arbitration_advance(machine, source_ticks);
        core_machine_readiness_advance(machine, source_ticks, due_tick);
        core_machine_peripheral_advance(machine, source_ticks);
    }
    provider_ticks = core_machine_clock_domain_advance(&machine->provider_clock,
        elapsed_ticks);
    if (machine->execution_provider != STD_NULL &&
        machine->execution_provider->advance_time != STD_NULL) {
        machine->execution_provider->advance_time(
            machine->execution_provider_context, provider_ticks);
    }
}

type_status core_machine_publish_elapsed_ticks(core_machine *machine,
    type_unsigned_64 elapsed_ticks, core_machine_time_publication_origin origin)
{
    if (machine == STD_NULL || elapsed_ticks == 0u ||
        UINT64_MAX - machine->elapsed_ticks < elapsed_ticks) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    /* Physical publication is closed to the two owners whose current source
     * rules establish a Core-axis duration. Remaining origins stay blocked
     * until their owner supplies an exact S4 disposition. */
    if (machine->retirement_time_contract == CORE_MACHINE_RETIREMENT_TIME_PHYSICAL &&
        origin != CORE_MACHINE_TIME_PUBLICATION_CPU_RETIREMENT &&
        origin != CORE_MACHINE_TIME_PUBLICATION_DEADLINE) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if (origin == CORE_MACHINE_TIME_PUBLICATION_CPU_RETIREMENT) {
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_CPU_RETIRE,
            core_machine_linear_pc(machine), (type_unsigned_32)elapsed_ticks, 0u);
    } else {
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_EXTERNAL_TIME,
            0u, (type_unsigned_32)elapsed_ticks, 0u);
    }
    core_machine_advance_scheduler(machine, elapsed_ticks);
    return TYPE_STATUS_OK;
}
