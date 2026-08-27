#include "type.h"

#include "core/machine/machine.h"

static C_INT core_machine_deadline_is_blocked(const core_machine *machine)
{
    type_native_unsigned drive;

    if (machine == STD_NULL ||
        core_machine_dma_has_pending_request(&machine->shared_dma_primary,
            &machine->shared_dma_secondary) ||
        machine->shared_kbc.data.typematic_remaining_ticks != 0u ||
        machine->shared_kbc.data.response_remaining_ticks != 0u ||
        machine->shared_kbc.data.serial_delivery_remaining_ticks != 0u ||
        machine->hdc.data.phase != CORE_MACHINE_HDC_PHASE_IDLE) {
        return 1;
    }
    for (drive = 0u; drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++drive) {
        if (machine->fdc.data.seek_pending[drive]) return 1;
    }
    return machine->fdc.data.dma_byte_gate_pending ||
        machine->fdc.data.ndma_byte_gate_pending ||
        machine->fdc.data.phase != core_machine_fdc_PHASE_COMMAND;
}

static C_VOID core_machine_deadline_consider_clock(const core_machine_clock_domain *clock,
    type_unsigned_64 device_ticks, type_unsigned_64 *io_source_ticks)
{
    type_unsigned_64 source_ticks;

    if (io_source_ticks == STD_NULL ||
        core_machine_clock_domain_source_ticks_until(clock, device_ticks,
            &source_ticks) != TYPE_STATUS_OK) {
        return;
    }
    if (*io_source_ticks == 0u || source_ticks < *io_source_ticks) {
        *io_source_ticks = source_ticks;
    }
}

static C_VOID core_machine_deadline_consider_pit(const t_pit *pit,
    const core_machine_clock_domain *clock, type_unsigned_64 *io_source_ticks)
{
    type_unsigned_8 counter;

    for (counter = 0u; counter < 3u; ++counter) {
        type_unsigned_64 device_ticks;

        if (core_machine_pit_ticks_until_output(pit, counter, &device_ticks) ==
            TYPE_STATUS_OK) {
            core_machine_deadline_consider_clock(clock, device_ticks,
                io_source_ticks);
        }
    }
}

C_VOID core_machine_capture_time_observation_private(const core_machine *machine,
    core_machine_time_observation *out_observation)
{
    type_unsigned_64 source_ticks = 0u;
    type_unsigned_64 device_ticks;

    if (machine == STD_NULL || out_observation == STD_NULL) return;
    out_observation->elapsed_ticks = machine->elapsed_ticks;
    out_observation->next_deadline_tick = 0u;
    out_observation->pacing_ticks_per_second = 0u;
    out_observation->physical_ticks_per_second = 0u;
    out_observation->next_deadline_valid = TYPE_FALSE;
    out_observation->pacing_time_available = TYPE_FALSE;
    out_observation->physical_time_available = TYPE_FALSE;
    if (machine->time_axis.kind == CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL ||
        machine->time_axis.kind == CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL) {
        out_observation->pacing_ticks_per_second = machine->time_axis.ticks_per_second;
        out_observation->pacing_time_available = TYPE_TRUE;
    }
    if (machine->time_axis.kind == CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL) {
        out_observation->physical_ticks_per_second = machine->time_axis.ticks_per_second;
        out_observation->physical_time_available = TYPE_TRUE;
    }
    if (!machine->timing_plan_copied || core_machine_deadline_is_blocked(machine)) {
        return;
    }
    if (machine->timing_plan.controller_timing.pit_clock ==
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK) {
        core_machine_deadline_consider_pit(&machine->shared_pit,
            &machine->pit_clock, &source_ticks);
        if (machine->auxiliary_pit_configured) {
            core_machine_deadline_consider_pit(&machine->auxiliary_pit,
                &machine->pit_clock, &source_ticks);
        }
    }
    if (machine->rtc_cmos_configured &&
        machine->rtc_cmos_config.timing.provenance == CORE_MACHINE_RTC_TIMING_L3_SOURCE &&
        machine->timing_plan.controller_timing.rtc_clock ==
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK &&
        core_machine_rtc_ticks_until_irq(&machine->shared_rtc, &device_ticks) ==
            TYPE_STATUS_OK) {
        core_machine_deadline_consider_clock(&machine->rtc_clock, device_ticks,
            &source_ticks);
    }
    if (machine->keyboard_topology == CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI &&
        core_machine_xt_keyboard_ticks_until_event(&machine->xt_keyboard,
            &device_ticks) == TYPE_STATUS_OK &&
        (source_ticks == 0u || device_ticks < source_ticks)) {
        source_ticks = device_ticks;
    }
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
C_VOID core_machine_arbitration_tick(C_VOID *opaque,
    type_unsigned_64 due_tick)
{
    core_machine *machine = (core_machine *)opaque;
    type_unsigned_64 dma_ticks;
    type_unsigned_64 pit_ticks;
    type_bool refresh_pending;
    core_machine_timeline_token next;

    if (machine == STD_NULL) {
        return;
    }
    dma_ticks = core_machine_clock_domain_advance(&machine->dma_clock, 1u);
    pit_ticks = core_machine_clock_domain_advance(&machine->pit_clock, 1u);
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
        core_machine_pit_advance(&machine->auxiliary_pit, pit_ticks);
    }
    if (pit_ticks != 0u) {
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_PIT_ADVANCE,
            0u, (type_unsigned_32)pit_ticks, 0u);
    }
    core_machine_pic_refresh(&machine->shared_pic_master,
        &machine->shared_pic_slave);
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_PIC_REFRESH,
        0u, 0u, 0u);
    if (due_tick != UINT64_MAX) {
        (C_VOID)core_machine_timeline_schedule(&machine->timeline,
            due_tick + 1u, core_machine_arbitration_tick, machine, &next);
    }
}

/*
 * RTC progression and removable-media observation have a distinct readiness
 * boundary.  This callback intentionally follows the immediate DMA/PIT/PIC
 * arbitration callback at a shared due tick: sources made ready here become
 * eligible for PIC arbitration at the following due tick. FDC and ATA command
 * and completion service are advanced here before their retained observation
 * refresh paths.
 */
C_VOID core_machine_readiness_tick(C_VOID *opaque,
    type_unsigned_64 due_tick)
{
    core_machine *machine = (core_machine *)opaque;
    type_unsigned_64 rtc_ticks;
    core_machine_timeline_token next;

    if (machine == STD_NULL) {
        return;
    }
    core_machine_fdc_advance_at(&machine->fdc, due_tick);
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_FDC_ADVANCE,
        0u, 0u, 0u);
    core_machine_fdc_refresh(&machine->fdc);
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_FDC_REFRESH,
        0u, 0u, 0u);
    core_machine_hdc_advance(&machine->hdc);
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_HDC_ADVANCE,
        0u, 0u, 0u);
    core_machine_hdc_refresh(&machine->hdc);
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_HDC_REFRESH,
        0u, 0u, 0u);
    rtc_ticks = core_machine_clock_domain_advance(&machine->rtc_clock, 1u);
    if (machine->rtc_cmos_configured) {
        core_machine_rtc_advance(&machine->shared_rtc, rtc_ticks);
    }
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_RTC_ADVANCE,
        0u, (type_unsigned_32)rtc_ticks, 0u);
    if (due_tick != UINT64_MAX) {
        (C_VOID)core_machine_timeline_schedule(&machine->timeline,
            due_tick + 1u, core_machine_readiness_tick, machine, &next);
    }
}

/*
 * Guest input and video state advance after the readiness boundary.  Host
 * presentation consumes only copied snapshots outside this callback and does
 * not participate in machine time.
 */
C_VOID core_machine_peripheral_tick(C_VOID *opaque,
    type_unsigned_64 due_tick)
{
    core_machine *machine = (core_machine *)opaque;
    type_unsigned_64 kbc_ticks;
    type_unsigned_64 vadp_ticks;
    core_machine_timeline_token next;

    if (machine == STD_NULL) {
        return;
    }
    kbc_ticks = core_machine_clock_domain_advance(&machine->kbc_clock, 1u);
    if (machine->keyboard_topology == CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI) {
        core_machine_xt_keyboard_advance(&machine->xt_keyboard, 1u);
    } else {
        core_machine_kbc_advance(&machine->shared_kbc, kbc_ticks);
    }
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_KBC_ADVANCE,
        0u, (type_unsigned_32)kbc_ticks, 0u);
    vadp_ticks = core_machine_clock_domain_advance(&machine->vadp_clock, 1u);
    core_machine_vadp_advance(&machine->shared_vadp, &machine->executor_memory,
        vadp_ticks);
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_VADP_ADVANCE,
        0u, (type_unsigned_32)vadp_ticks, 0u);
    if (due_tick != UINT64_MAX) {
        (C_VOID)core_machine_timeline_schedule(&machine->timeline,
            due_tick + 1u, core_machine_peripheral_tick, machine, &next);
    }
}

static C_VOID core_machine_advance_scheduler(core_machine *machine,
    type_unsigned_64 elapsed_ticks)
{
    type_unsigned_64 provider_ticks;

    (C_VOID)core_machine_timeline_advance(&machine->timeline,
        machine->elapsed_ticks);
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
    machine->elapsed_ticks += elapsed_ticks;
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
