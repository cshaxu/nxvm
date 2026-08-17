#include "type.h"

#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct retirement_probe {
    core_machine *machine;
    core_machine_retirement_observation records[2];
    type_unsigned_32 count;
    type_status set_while_running;
} retirement_probe;

static C_VOID retirement_capture(C_VOID *opaque,
    const core_machine_retirement_observation *observation)
{
    retirement_probe *probe = (retirement_probe *)opaque;

    if (probe == STD_NULL || observation == STD_NULL) return;
    if (probe->count < 2u) probe->records[probe->count] = *observation;
    ++probe->count;
    probe->set_while_running = core_machine_set_retirement_observation_provider(
        probe->machine, STD_NULL);
}

static C_INT retirement_prepare(core_machine **out_machine,
    const core_machine_config *config, const type_unsigned_8 *code,
    STD_SIZE_T bytes)
{
    return out_machine != STD_NULL &&
        core_machine_create(config, out_machine) == TYPE_STATUS_OK &&
        test_core_machine_fixture_register_reset_mapping(*out_machine, 0xfffffff0u,
            0x000ffff0u, 16u) == TYPE_STATUS_OK &&
        core_machine_freeze_execution_providers(*out_machine) == TYPE_STATUS_OK &&
        core_machine_reset(*out_machine) == TYPE_STATUS_OK &&
        core_machine_memory_write(*out_machine, 0xfffffff0u, code, bytes) ==
            TYPE_STATUS_OK;
}

C_INT main(C_VOID)
{
    core_machine *machine = STD_NULL;
    core_machine_config deterministic = { .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386 };
    core_machine_config physical = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .retirement_time_contract = CORE_MACHINE_RETIREMENT_TIME_PHYSICAL
    };
    core_machine_retirement_observation_provider provider;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_timeline_observation timeline;
    retirement_probe probe = { STD_NULL, { { 0 } }, 0u, TYPE_STATUS_OK };
    type_unsigned_8 nop = 0x90u;
    type_unsigned_8 rep_nop[] = { 0xf3u, 0x90u };
    C_INT failed = 0;

    provider.callback = retirement_capture;
    provider.context = &probe;
    failed |= !retirement_prepare(&machine, &deterministic, &nop, 1u);
    probe.machine = machine;
    failed |= core_machine_set_retirement_observation_provider(machine,
        &provider) != TYPE_STATUS_OK;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0xfffffff0u, &nop, 1u) !=
        TYPE_STATUS_OK;
    failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
        probe.count != 1u || probe.set_while_running != TYPE_STATUS_INVALID_STATE ||
        probe.records[0].sequence != 0u ||
        probe.records[0].point.linear_pc != 0xfffffff0u ||
        probe.records[0].point.byte_count != CORE_MACHINE_CPU_DIAGNOSTIC_BYTES ||
        probe.records[0].point.bytes[0] != nop ||
        probe.records[0].cpu_profile != CORE_MACHINE_CPU_PROFILE_80386 ||
        probe.records[0].timing_disposition !=
            CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
        probe.records[0].source_timing_form_id != 0u ||
        probe.records[0].elapsed_ticks != 0u || probe.records[0].timeline_ticks != 0u ||
        probe.records[0].source_ticks == 0u || probe.records[0].protected_mode ||
        probe.records[0].virtual_8086_mode || probe.records[0].operand_size_32 ||
        probe.records[0].address_size_32 || probe.records[0].lock_prefix ||
        probe.records[0].repeat_prefix != 0u;
    failed |= core_machine_set_retirement_observation_provider(machine, STD_NULL) !=
        TYPE_STATUS_OK;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0xfffffff0u, &nop, 1u) !=
        TYPE_STATUS_OK;
    failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || probe.count != 1u;
    core_machine_destroy(machine);
    machine = STD_NULL;

    probe.machine = STD_NULL;
    probe.count = 0u;
    probe.set_while_running = TYPE_STATUS_OK;
    failed |= !retirement_prepare(&machine, &physical, rep_nop, sizeof(rep_nop));
    probe.machine = machine;
    failed |= core_machine_set_retirement_observation_provider(machine,
        &provider) != TYPE_STATUS_OK;
    failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_FAULT;
    failed |= result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
        probe.count != 1u || probe.set_while_running != TYPE_STATUS_INVALID_STATE ||
        probe.records[0].point.byte_count != CORE_MACHINE_CPU_DIAGNOSTIC_BYTES ||
        probe.records[0].point.bytes[0] != rep_nop[0] ||
        probe.records[0].point.bytes[1] != rep_nop[1] ||
        probe.records[0].timing_disposition !=
            CORE_MACHINE_RETIREMENT_TIMING_SOURCE_UNALLOCATED ||
        probe.records[0].source_timing_form_id !=
            CORE_MACHINE_RETIREMENT_SOURCE_FORM_UNATTRIBUTED ||
        probe.records[0].elapsed_ticks != 0u || probe.records[0].timeline_ticks != 0u;
    failed |= core_machine_get_timeline_observation(machine, &timeline) != TYPE_STATUS_OK ||
        timeline.now != 0u;
    core_machine_destroy(machine);

    if (failed) return 1;
    STD_PRINTF("M5:T390:S3:RETIREMENT-OBSERVATION:OK\n");
    return 0;
}