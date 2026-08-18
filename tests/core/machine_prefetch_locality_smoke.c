#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/machine/transaction.h"
#include "../support/core_machine_cpu_fixture.h"

static C_VOID external_cycle_begin_and_commit(
    core_machine_cpu_external_cycle_provider provider, C_VOID *context,
    type_unsigned_32 physical, type_unsigned_8 bytes, type_bool write,
    core_machine_cpu_memory_access_provenance provenance)
{
    provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN, physical,
        bytes, write, provenance);
    provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT, physical,
        bytes, write, provenance);
}

static C_INT run_halt(const core_machine_external_cycle_timing *timing,
    type_unsigned_64 *out_ticks)
{
    static const type_unsigned_8 code[] = {
        0xebu, 0x01u, 0x90u, 0xb0u, 0x5au, 0xe6u, 0xe0u, 0xf4u
    };
    core_machine_config config = {0};
    core_machine *machine = STD_NULL;
    core_machine_run_result result;
    C_INT failed = 0;

    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    config.external_cycle_timing = *timing;
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= test_core_machine_fixture_register_reset_mapping(machine,
        0xfffffff0u, 0x000ffff0u, 16u) != TYPE_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0x000ffff0u, code, sizeof(code)) !=
        TYPE_STATUS_OK;
    failed |= core_machine_run(machine, (core_machine_run_budget){5u, 0u},
        &result) != TYPE_STATUS_OK;
    if (!failed && out_ticks != STD_NULL) *out_ticks = result.ticks;
    core_machine_destroy(machine);
    return !failed;
}

static C_INT run_write(const core_machine_external_cycle_timing *timing,
    type_unsigned_64 *out_ticks)
{
    static const type_unsigned_8 code[] = {
        0xb0u, 0x5au, 0xa2u, 0x10u, 0x00u, 0xf4u
    };
    core_machine_config config = {0};
    core_machine *machine = STD_NULL;
    core_machine_run_result result;
    type_unsigned_8 value = 0u;
    C_INT failed = 0;

    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    config.external_cycle_timing = *timing;
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= test_core_machine_fixture_register_reset_mapping(machine,
        0xfffffff0u, 0x000ffff0u, 16u) != TYPE_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0x000ffff0u, code, sizeof(code)) !=
        TYPE_STATUS_OK;
    failed |= core_machine_run(machine, (core_machine_run_budget){3u, 0u},
        &result) != TYPE_STATUS_OK;
    failed |= core_machine_memory_read(machine, 0x10u, &value, 1u) !=
        TYPE_STATUS_OK || value != 0x5au;
    if (!failed && out_ticks != STD_NULL) *out_ticks = result.ticks;
    core_machine_destroy(machine);
    return !failed;
}

static C_INT run_read(const core_machine_external_cycle_timing *timing,
    type_unsigned_64 *out_ticks)
{
    static const type_unsigned_8 code[] = { 0xa0u, 0x10u, 0x00u, 0xf4u };
    static const type_unsigned_8 data = 0x5au;
    core_machine_config config = {0};
    core_machine *machine = STD_NULL;
    core_machine_run_result result;
    C_INT failed = 0;

    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    config.external_cycle_timing = *timing;
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= test_core_machine_fixture_register_reset_mapping(machine,
        0xfffffff0u, 0x000ffff0u, 16u) != TYPE_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0x000ffff0u, code, sizeof(code)) !=
        TYPE_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0x10u, &data, 1u) != TYPE_STATUS_OK;
    failed |= core_machine_run(machine, (core_machine_run_budget){2u, 0u},
        &result) != TYPE_STATUS_OK;
    if (!failed && out_ticks != STD_NULL) *out_ticks = result.ticks;
    core_machine_destroy(machine);
    return !failed;
}

static C_INT external_cycle_observer_contract(C_VOID)
{
    static const core_machine_external_cycle_timing explicit_timing = {2048u,
        2u, 1u, CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_EXPLICIT_SEQUENTIAL};
    core_machine_config config = {0};
    core_machine *machine = STD_NULL;
    core_machine_cpu_external_cycle_provider provider;
    C_VOID *context;
    C_INT failed = 0;

    config.external_cycle_timing = explicit_timing;
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    provider = machine == STD_NULL ? STD_NULL :
        machine->executor_cpu_execution.external_cycle_provider;
    context = machine == STD_NULL ? STD_NULL :
        machine->executor_cpu_execution.external_cycle_context;
    failed |= provider == STD_NULL;
    if (!failed) {
        provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN, 0u, 1u,
            TYPE_FALSE, CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH);
        provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_OVERLAP_DECLARE,
            1u, 1u, TYPE_FALSE,
            CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH);
        provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT, 0u, 1u,
            TYPE_FALSE, CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH);
        external_cycle_begin_and_commit(provider, context, 1u, 1u, TYPE_FALSE,
            CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH);
        failed |= !machine->external_cycle_page_valid ||
            machine->external_cycle_round_ticks != 3u;
        machine->external_cycle_round_ticks = 0u;
        machine->external_cycle_page_valid = TYPE_FALSE;
        provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN, 0x800u, 4u,
            TYPE_FALSE, CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ);
        provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL, 0x800u, 4u,
            TYPE_FALSE, CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ);
        failed |= machine->external_cycle_page_valid ||
            machine->external_cycle_pending_valid ||
            machine->external_cycle_round_ticks != 0u;
        external_cycle_begin_and_commit(provider, context, 0x800u, 4u, TYPE_FALSE,
            CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ);
        external_cycle_begin_and_commit(provider, context, 0x804u, 4u, TYPE_TRUE,
            CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_WRITE);
        failed |= !machine->external_cycle_page_valid ||
            machine->external_cycle_round_ticks != 4u;
        machine->external_cycle_round_ticks = 0u;
        provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN, 0x808u, 4u,
            TYPE_FALSE, CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ);
        failed |= core_machine_transaction_hold_request(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_DMA, 0u) != TYPE_STATUS_OK;
        failed |= core_machine_transaction_hold_acknowledge(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_DMA) != TYPE_STATUS_OK;
        failed |= machine->external_cycle_page_valid ||
            machine->external_cycle_pending_valid || machine->external_cycle_overlap_valid;
        core_machine_transaction_hold_release(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_DMA);
        external_cycle_begin_and_commit(provider, context, 0x808u, 4u, TYPE_FALSE,
            CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ);
        failed |= !machine->external_cycle_page_valid ||
            machine->external_cycle_round_ticks != 2u;
        failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
        failed |= machine->external_cycle_page_valid ||
            machine->external_cycle_pending_valid || machine->external_cycle_overlap_valid ||
            machine->external_cycle_round_ticks != 0u;
    }
    core_machine_destroy(machine);
    return !failed;
}

static C_INT d4_refresh_external_cycle_contract(C_VOID)
{
    static const core_machine_external_cycle_timing timing = {2048u, 2u, 1u,
        CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED};
    core_machine_config config = {0};
    core_machine_d4_platform_config d4 = {CORE_MACHINE_PC_AT_PORT_B, 0u, 2u};
    core_machine *machine = STD_NULL;
    core_machine_cpu_external_cycle_provider provider;
    C_VOID *context;
    C_INT failed = 0;

    config.external_cycle_timing = timing;
    config.auxiliary_pit_present = TYPE_TRUE;
    config.auxiliary_pit_base_port = 0x0048u;
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= core_machine_configure_d4_platform(machine, &d4) != TYPE_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    provider = machine == STD_NULL ? STD_NULL :
        machine->executor_cpu_execution.external_cycle_provider;
    context = machine == STD_NULL ? STD_NULL :
        machine->executor_cpu_execution.external_cycle_context;
    failed |= provider == STD_NULL;
    if (!failed) {
        external_cycle_begin_and_commit(provider, context, 0x800u, 4u, TYPE_FALSE,
            CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ);
        failed |= !machine->external_cycle_page_valid ||
            machine->external_cycle_round_ticks != 2u;
        machine->external_cycle_round_ticks = 0u;
        failed |= core_machine_advance_time(machine, 18u) != TYPE_STATUS_OK;
        failed |= machine->external_cycle_page_valid || machine->external_cycle_pending_valid;
        external_cycle_begin_and_commit(provider, context, 0x804u, 4u, TYPE_FALSE,
            CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ);
        failed |= !machine->external_cycle_page_valid ||
            machine->external_cycle_round_ticks != 2u;
        failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
        failed |= machine->external_cycle_page_valid || machine->external_cycle_pending_valid ||
            machine->external_cycle_round_ticks != 0u;
    }
    core_machine_destroy(machine);
    return !failed;
}

static C_INT retirement_wait_contract(C_VOID)
{
    static const type_unsigned_8 code[] = {0xa0u, 0x10u, 0x00u, 0xf4u};
    static const type_unsigned_8 data = 0x5au;
    static const core_machine_external_cycle_timing timing = {2048u, 2u, 0u,
        CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED};
    core_machine_config config = {0};
    core_machine *machine = STD_NULL;
    core_machine_run_result result;
    C_INT failed = 0;

    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    config.external_cycle_timing = timing;
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= test_core_machine_fixture_register_reset_mapping(machine,
        0xfffffff0u, 0x000ffff0u, 16u) != TYPE_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0x000ffff0u, code, sizeof(code)) !=
        TYPE_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0x10u, &data, 1u) != TYPE_STATUS_OK;
    machine->maximum_instruction_ticks = 1u;
    failed |= core_machine_run(machine, (core_machine_run_budget){0u, 1u},
        &result) != TYPE_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 0u ||
        result.ticks != 1u || machine->cpu_retirement_wait_pending == TYPE_FALSE ||
        machine->cpu_retirement_wait_ticks == 0u;
    failed |= core_machine_run(machine, (core_machine_run_budget){1u, 0u},
        &result) != TYPE_STATUS_OK;
    failed |= result.executed != 1u || result.ticks <= 1u ||
        machine->cpu_retirement_wait_pending != TYPE_FALSE;
    core_machine_destroy(machine);
    return !failed;
}
C_INT main(C_VOID)
{
    static const core_machine_external_cycle_timing disabled = {0u, 0u, 0u,
        CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED};
    static const core_machine_external_cycle_timing timing = {2048u, 2u, 1u,
        CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED};
    type_unsigned_64 baseline_ticks = 0u;
    type_unsigned_64 timing_ticks = 0u;
    type_unsigned_64 write_baseline_ticks = 0u;
    type_unsigned_64 write_timing_ticks = 0u;
    type_unsigned_64 read_baseline_ticks = 0u;
    type_unsigned_64 read_timing_ticks = 0u;
    C_INT failed = 0;

    failed |= !run_halt(&disabled, &baseline_ticks);
    failed |= !run_halt(&timing, &timing_ticks);
    failed |= !run_write(&disabled, &write_baseline_ticks);
    failed |= !run_write(&timing, &write_timing_ticks);
    failed |= !run_read(&disabled, &read_baseline_ticks);
    failed |= !run_read(&timing, &read_timing_ticks);
    /* Completed adjacency is not overlap, so every normal synchronous cycle misses. */
    failed |= timing_ticks != baseline_ticks + 6u;
    failed |= write_timing_ticks != write_baseline_ticks + 4u;
    failed |= read_timing_ticks != read_baseline_ticks + 4u;
    failed |= !external_cycle_observer_contract();
    failed |= !d4_refresh_external_cycle_contract();
    failed |= !retirement_wait_contract();
    if (failed != 0) return 1;
    STD_PRINTF("M5:T412:S1:EXTERNAL-READ-LOCALITY:OK\n");
    STD_PRINTF("M5:T413:S1:EXTERNAL-WRITE-BRIDGE:OK\n");
    STD_PRINTF("M5:T414:S1:DATA-READ-LOCALITY:OK\n");
    STD_PRINTF("M5:T415:S1:PAGE-WALK-LOCALITY:OK\n");
    STD_PRINTF("M5:T416:S1:DMA-HOLD-LOCALITY:OK\n");
    STD_PRINTF("M5:T417:S1:REFRESH-LOCALITY:OK\n");
    STD_PRINTF("M5:T418:S1:INSTRUCTION-BOUNDARY-LOCALITY:OK\n");
    STD_PRINTF("M5:T419:S5:EXTERNAL-CYCLE-OVERLAP:OK\n");
    STD_PRINTF("M5:T423:S1:CPU-BOARD-TRANSACTION:OK\n");
    return 0;
}
