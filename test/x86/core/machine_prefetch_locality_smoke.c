#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "x86/core/machine.h"
#include "x86/core/transaction.h"
#include "x86/core/attachment_interface.h"

static core_machine_cpu_execution_context *prefetch_observed_cpu;
static lib_u32 prefetch_grants;

static lib_status prefetch_reset_mapping(core_machine *machine)
{
    const core_machine_memory_alias_config alias = {0xfffffff0u, 0xffff0u, 16u};
    return core_machine_install_memory_aliases(machine, &alias, 1u, LIB_FALSE);
}

static lib_bool prefetch_refresh_request(void *owner, lib_u8 *address)
{
    *address = 0u;
    return *(lib_bool *)owner;
}

static void prefetch_refresh_complete(void *owner)
{
    *(lib_bool *)owner = LIB_FALSE;
}

static void prefetch_finalize(void *owner)
{
    (void)owner;
}


/* The test-built scheduler calls this spy; all CPU behavior remains real. */
void test_cpu_prefetch_grant(core_machine_cpu_execution_context *cpu)
{
    if (cpu == prefetch_observed_cpu) ++prefetch_grants;
    core_machine_cpu_execution_advance_prefetch_reservation(cpu);
}

static void external_cycle_begin_and_commit(
    core_machine_cpu_external_cycle_provider provider, void *context,
    lib_u32 physical, lib_u8 bytes, lib_u8 write,
    core_machine_cpu_memory_access_provenance provenance)
{
    provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN, CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, physical,
        bytes, write, provenance);
    provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT, CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, physical,
        bytes, write, provenance);
}

static lib_i32 run_halt(const core_machine_external_cycle_timing *timing,
    lib_u64 *out_ticks)
{
    static const lib_u8 code[] = {
        0xebu, 0x01u, 0x90u, 0xb0u, 0x5au, 0xe6u, 0xe0u, 0xf4u
    };
    core_machine_executor_config config = {0};
    core_machine *machine = LIB_NULL;
    core_machine_run_result result;
    lib_i32 failed = 0;

    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    config.transaction_contract.external_cycle_timing = *timing;
    failed |= core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
    failed |= prefetch_reset_mapping(machine) != LIB_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0x000ffff0u, code, sizeof(code)) !=
        LIB_STATUS_OK;
    failed |= core_machine_run(machine, (core_machine_run_budget){5u, 0u},
        &result) != LIB_STATUS_OK;
    if (!failed && out_ticks != LIB_NULL) *out_ticks = result.ticks;
    core_machine_destroy(machine);
    return !failed;
}

static lib_i32 run_halt_with_port_wait(lib_u64 *out_ticks)
{
    static const lib_u8 code[] = {
        0xebu, 0x01u, 0x90u, 0xb0u, 0x5au, 0xe6u, 0xe0u, 0xf4u
    };
    core_machine_executor_config config = {0};
    core_machine *machine = LIB_NULL;
    core_machine_run_result result;
    lib_i32 failed = 0;

    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    config.transaction_contract.external_access_wait_windows[0] =
        (core_machine_external_access_wait_window) {
            CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x00e0u, 0x00e0u, 1u};
    failed |= core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
    failed |= prefetch_reset_mapping(machine) != LIB_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0x000ffff0u, code, sizeof(code)) !=
        LIB_STATUS_OK;
    failed |= core_machine_run(machine, (core_machine_run_budget){5u, 0u},
        &result) != LIB_STATUS_OK;
    if (!failed && out_ticks != LIB_NULL) *out_ticks = result.ticks;
    core_machine_destroy(machine);
    return !failed;
}
static lib_i32 run_write(const core_machine_external_cycle_timing *timing,
    lib_u64 *out_ticks)
{
    static const lib_u8 code[] = {
        0xb0u, 0x5au, 0xa2u, 0x10u, 0x00u, 0xf4u
    };
    core_machine_executor_config config = {0};
    core_machine *machine = LIB_NULL;
    core_machine_run_result result;
    lib_u8 value = 0u;
    lib_i32 failed = 0;

    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    config.transaction_contract.external_cycle_timing = *timing;
    failed |= core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
    failed |= prefetch_reset_mapping(machine) != LIB_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0x000ffff0u, code, sizeof(code)) !=
        LIB_STATUS_OK;
    failed |= core_machine_run(machine, (core_machine_run_budget){3u, 0u},
        &result) != LIB_STATUS_OK;
    failed |= core_machine_memory_read(machine, 0x10u, &value, 1u) !=
        LIB_STATUS_OK || value != 0x5au;
    if (!failed && out_ticks != LIB_NULL) *out_ticks = result.ticks;
    core_machine_destroy(machine);
    return !failed;
}

static lib_i32 run_read(const core_machine_external_cycle_timing *timing,
    lib_u64 *out_ticks)
{
    static const lib_u8 code[] = { 0xa0u, 0x10u, 0x00u, 0xf4u };
    static const lib_u8 data = 0x5au;
    core_machine_executor_config config = {0};
    core_machine *machine = LIB_NULL;
    core_machine_run_result result;
    lib_i32 failed = 0;

    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    config.transaction_contract.external_cycle_timing = *timing;
    failed |= core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
    failed |= prefetch_reset_mapping(machine) != LIB_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0x000ffff0u, code, sizeof(code)) !=
        LIB_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0x10u, &data, 1u) != LIB_STATUS_OK;
    failed |= core_machine_run(machine, (core_machine_run_budget){2u, 0u},
        &result) != LIB_STATUS_OK;
    if (!failed && out_ticks != LIB_NULL) *out_ticks = result.ticks;
    core_machine_destroy(machine);
    return !failed;
}

static lib_i32 external_cycle_observer_contract(void)
{
    static const core_machine_external_cycle_timing explicit_timing = {2048u,
        2u, 1u, CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_EXPLICIT_SEQUENTIAL, 0u, 0u};
    core_machine_executor_config config = {0};
    core_machine *machine = LIB_NULL;
    core_machine_cpu_external_cycle_provider provider;
    void *context;
    lib_i32 failed = 0;

    config.transaction_contract.external_cycle_timing = explicit_timing;
    failed |= core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    provider = core_machine_cpu_external_cycle_trace;
    context = machine;
    failed |= provider == LIB_NULL;
    if (!failed) {
        provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN, CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, 0u, 1u,
            LIB_FALSE, CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH);
        provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_OVERLAP_DECLARE,
            CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, 1u, 1u, LIB_FALSE,
            CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH);
        provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT, CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, 0u, 1u,
            LIB_FALSE, CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH);
        external_cycle_begin_and_commit(provider, context, 1u, 1u, LIB_FALSE,
            CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH);
        failed |= !machine->external_cycle_page_valid ||
            machine->external_cycle_round_ticks != 3u;
        machine->external_cycle_round_ticks = 0u;
        machine->external_cycle_page_valid = LIB_FALSE;
        provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN, CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, 0x800u, 4u,
            LIB_FALSE, CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ);
        provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL, CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, 0x800u, 4u,
            LIB_FALSE, CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ);
        failed |= machine->external_cycle_page_valid ||
            machine->external_cycle_pending_valid ||
            machine->external_cycle_round_ticks != 0u;
        external_cycle_begin_and_commit(provider, context, 0x800u, 4u, LIB_FALSE,
            CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ);
        external_cycle_begin_and_commit(provider, context, 0x804u, 4u, LIB_TRUE,
            CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_WRITE);
        failed |= !machine->external_cycle_page_valid ||
            machine->external_cycle_round_ticks != 4u;
        machine->external_cycle_round_ticks = 0u;
        provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN, CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, 0x808u, 4u,
            LIB_FALSE, CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ);
        failed |= core_machine_transaction_hold_request(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_DMA, 0u) != LIB_STATUS_OK;
        failed |= core_machine_transaction_hold_acknowledge(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_DMA) != LIB_STATUS_OK;
        failed |= machine->external_cycle_page_valid ||
            machine->external_cycle_pending_valid || machine->external_cycle_overlap_valid;
        core_machine_transaction_hold_release(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_DMA);
        external_cycle_begin_and_commit(provider, context, 0x808u, 4u, LIB_FALSE,
            CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ);
        failed |= !machine->external_cycle_page_valid ||
            machine->external_cycle_round_ticks != 2u;
        failed |= core_machine_reset(machine) != LIB_STATUS_OK;
        failed |= machine->external_cycle_page_valid ||
            machine->external_cycle_pending_valid || machine->external_cycle_overlap_valid ||
            machine->external_cycle_round_ticks != 0u;
    }
    core_machine_destroy(machine);
    return !failed;
}

static lib_i32 refresh_external_cycle_contract(void)
{
    const core_machine_external_cycle_timing timing = {2048u, 2u, 1u,
        CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED, 0u, 0u};
    const core_machine_executor_config config = {
        .transaction_contract = {.external_cycle_timing = timing}
    };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK;

    if (!failed) {
        external_cycle_begin_and_commit(core_machine_cpu_external_cycle_trace, machine,
            0x800u, 4u, LIB_FALSE, CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ);
        failed |= !machine->external_cycle_page_valid ||
            machine->external_cycle_round_ticks != 2u;
        machine->external_cycle_round_ticks = 0u;
        core_machine_cpu_bus_refresh_pulse(machine);
        failed |= machine->external_cycle_page_valid || machine->external_cycle_pending_valid;
        external_cycle_begin_and_commit(core_machine_cpu_external_cycle_trace, machine,
            0x804u, 4u, LIB_FALSE, CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ);
        failed |= !machine->external_cycle_page_valid ||
            machine->external_cycle_round_ticks != 2u;
        failed |= core_machine_reset(machine) != LIB_STATUS_OK;
        failed |= machine->external_cycle_page_valid || machine->external_cycle_pending_valid ||
            machine->external_cycle_round_ticks != 0u;
    }
    core_machine_destroy(machine);
    return !failed;
}

static lib_i32 retirement_wait_contract(void)
{
    static const lib_u8 code[] = {0xa0u, 0x10u, 0x00u, 0xf4u};
    static const lib_u8 data = 0x5au;
    static const core_machine_external_cycle_timing timing = {2048u, 2u, 0u,
        CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED, 0u, 0u};
    core_machine_executor_config config = {0};
    core_machine *machine = LIB_NULL;
    core_machine_run_result result;
    lib_i32 failed = 0;

    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    config.transaction_contract.external_cycle_timing = timing;
    config.transaction_contract.cpu_cycle_bus_ready_gate_enabled = LIB_TRUE;
    failed |= core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
    failed |= prefetch_reset_mapping(machine) != LIB_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0x000ffff0u, code, sizeof(code)) !=
        LIB_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0x10u, &data, 1u) != LIB_STATUS_OK;
    machine->maximum_instruction_ticks = 1u;
    failed |= core_machine_run(machine, (core_machine_run_budget){0u, 1u},
        &result) != LIB_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 0u ||
        result.ticks != 1u || machine->cpu_retirement_wait_pending == LIB_FALSE ||
        machine->cpu_retirement_wait_ticks == 0u;
    failed |= core_machine_set_cpu_bus_ready(machine, 0) != LIB_STATUS_OK;
    failed |= core_machine_run(machine, (core_machine_run_budget){0u, 1u},
        &result) != LIB_STATUS_OK;
    failed |= result.executed != 0u || result.ticks != 1u ||
        machine->cpu_retirement_wait_ticks == 0u;
    failed |= core_machine_set_cpu_bus_ready(machine, 1) != LIB_STATUS_OK;
    failed |= core_machine_run(machine, (core_machine_run_budget){1u, 0u},
        &result) != LIB_STATUS_OK;
    failed |= result.executed != 1u || result.ticks <= 1u ||
        machine->cpu_retirement_wait_pending != LIB_FALSE;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK ||
        machine->cpu_cycle_bus_ready != LIB_TRUE;
    /* A deferred board wait has no physical source disposition until T470 S4.
     * It must therefore stop before publishing a synthetic physical tick. */
    machine->retirement_time_contract = CORE_MACHINE_RETIREMENT_TIME_PHYSICAL;
    machine->time_axis = (core_machine_time_axis) {
        CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL, 8000000u };
    failed |= core_machine_memory_write(machine, 0x000ffff0u, code, sizeof(code)) !=
        LIB_STATUS_OK || core_machine_memory_write(machine, 0x10u, &data, 1u) !=
        LIB_STATUS_OK;
    machine->maximum_instruction_ticks = 1u;
    failed |= core_machine_run(machine, (core_machine_run_budget){0u, 1u},
        &result) != LIB_STATUS_INTERNAL_ERROR || result.elapsed_ticks != 0u ||
        machine->elapsed_ticks != 0u;
    core_machine_destroy(machine);
    return !failed;
}
static lib_i32 cecg_aperture_wait_contract(void)
{
    core_machine_executor_config config = {0};
    core_machine *machine = LIB_NULL;
    core_machine_cpu_external_cycle_provider provider;
    void *context;
    lib_i32 failed = 0;

    config.transaction_contract.external_access_wait_windows[0] =
        (core_machine_external_access_wait_window) {
            CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, 0x000a0000u,
            0x000affffu, 1u};
    failed |= core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    provider = core_machine_cpu_external_cycle_trace;
    context = machine;
    if (!failed) {
        external_cycle_begin_and_commit(provider, context, 0x000a0000u, 1u,
            LIB_TRUE, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA);
        failed |= machine->external_cycle_round_ticks != 1u;
        provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN,
            CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, 0x000a0001u, 1u,
            LIB_FALSE, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA);
        provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL,
            CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, 0x000a0001u, 1u,
            LIB_FALSE, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA);
        failed |= machine->external_cycle_round_ticks != 1u;
        failed |= core_machine_reset(machine) != LIB_STATUS_OK ||
            machine->external_cycle_round_ticks != 0u;
    }
    core_machine_destroy(machine);
    return !failed;
}
static lib_i32 d4_cecg_memory_class_contract(void)
{
    core_machine_executor_config config = {0};
    core_machine *machine = LIB_NULL;
    core_machine_cpu_external_cycle_provider provider;
    void *context;
    lib_i32 failed = 0;

    config.transaction_contract.external_cycle_timing = (core_machine_external_cycle_timing) {
        2048u, 2u, 0u, CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED,
        0u, 0x0009ffffu};
    failed |= core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    provider = core_machine_cpu_external_cycle_trace;
    context = machine;
    if (!failed) {
        external_cycle_begin_and_commit(provider, context, 0x0009ff00u, 1u,
            LIB_FALSE, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA);
        failed |= machine->external_cycle_round_ticks != 2u;
        machine->external_cycle_round_ticks = 0u;
        external_cycle_begin_and_commit(provider, context, 0x000a0000u, 1u,
            LIB_FALSE, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA);
        failed |= machine->external_cycle_round_ticks != 0u;
    }
    core_machine_destroy(machine);
    return !failed;
}
static lib_i32 cecg_port_wait_contract(void)
{
    core_machine_executor_config config = {0};
    core_machine *machine = LIB_NULL;
    core_machine_cpu_external_cycle_provider provider;
    void *context;
    lib_i32 failed = 0;

    config.transaction_contract.external_access_wait_windows[0] =
        (core_machine_external_access_wait_window) {
            CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x03c0u, 0x03cfu, 1u};
    failed |= core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    provider = core_machine_cpu_external_cycle_trace;
    context = machine;
    failed |= provider == LIB_NULL;
    if (!failed) {
        provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN,
            CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x03c0u, 1u, LIB_TRUE,
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA);
        provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT,
            CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x03c0u, 1u, LIB_TRUE,
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA);
        failed |= machine->external_cycle_round_ticks != 1u ||
            machine->external_cycle_pending_valid;
        provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN,
            CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x03d0u, 1u, LIB_FALSE,
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA);
        provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT,
            CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x03d0u, 1u, LIB_FALSE,
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA);
        failed |= machine->external_cycle_round_ticks != 1u;
        provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN,
            CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x03c1u, 1u, LIB_FALSE,
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA);
        provider(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL,
            CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x03c1u, 1u, LIB_FALSE,
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA);
        failed |= machine->external_cycle_round_ticks != 1u ||
            machine->external_cycle_pending_valid;
        failed |= core_machine_reset(machine) != LIB_STATUS_OK ||
            machine->external_cycle_round_ticks != 0u ||
            machine->external_cycle_pending_valid;
    }
    core_machine_destroy(machine);
    return !failed;
}
static lib_i32 prefetch_grant_contract(void)
{
    static const core_machine_external_cycle_timing timing = {2048u, 2u, 0u,
        CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED, 0u, 0u};
    core_machine_executor_config config = {0};
    core_machine *machine = LIB_NULL;
    lib_bool refresh_pending = LIB_FALSE;
    const core_machine_attachment attachment = {
        .refresh_request = prefetch_refresh_request,
        .refresh_complete = prefetch_refresh_complete,
        .finalize_devices = prefetch_finalize,
        .context = &refresh_pending
    };
    lib_i32 failed = 0;

    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    config.transaction_contract.external_cycle_timing = timing;
    config.transaction_contract.cpu_prefetch_reservation_enabled = LIB_TRUE;
    failed |= core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
    failed |= !failed && core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    if (!failed) {
        prefetch_observed_cpu = machine->executor_cpu_execution;
        prefetch_grants = 0u;
        failed |= core_machine_advance_time(machine, 1u) != LIB_STATUS_OK;
        failed |= prefetch_grants != 1u ||
            machine->external_cycle_overlap_valid ||
            machine->external_cycle_round_ticks != 0u;
        failed |= core_machine_transaction_hold_request(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_DMA, 0u) != LIB_STATUS_OK;
        failed |= core_machine_transaction_hold_acknowledge(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_DMA) != LIB_STATUS_OK;
        failed |= core_machine_advance_time(machine, 1u) != LIB_STATUS_OK ||
            prefetch_grants != 1u;
        core_machine_transaction_hold_release(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_DMA);
        refresh_pending = LIB_TRUE;
        failed |= core_machine_advance_time(machine, 1u) != LIB_STATUS_OK ||
            prefetch_grants != 1u;
        failed |= core_machine_advance_time(machine, 1u) != LIB_STATUS_OK ||
            prefetch_grants != 2u;
        failed |= core_machine_reset(machine) != LIB_STATUS_OK;
        prefetch_grants = 0u;
        failed |= core_machine_advance_time(machine, 1u) != LIB_STATUS_OK ||
            prefetch_grants != 1u;
    }
    prefetch_observed_cpu = LIB_NULL;
    core_machine_destroy(machine);
    return !failed;
}
lib_i32 main(void)
{
    static const core_machine_external_cycle_timing disabled = {0u, 0u, 0u,
        CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED, 0u, 0u};
    static const core_machine_external_cycle_timing timing = {2048u, 2u, 1u,
        CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED, 0u, 0u};
    lib_u64 baseline_ticks = 0u;
    lib_u64 timing_ticks = 0u;
    lib_u64 port_wait_ticks = 0u;
    lib_u64 write_baseline_ticks = 0u;
    lib_u64 write_timing_ticks = 0u;
    lib_u64 read_baseline_ticks = 0u;
    lib_u64 read_timing_ticks = 0u;
    lib_i32 failed = 0;

    failed |= !run_halt(&disabled, &baseline_ticks);
    failed |= !run_halt(&timing, &timing_ticks);
    failed |= !run_halt_with_port_wait(&port_wait_ticks);
    failed |= !run_write(&disabled, &write_baseline_ticks);
    failed |= !run_write(&timing, &write_timing_ticks);
    failed |= !run_read(&disabled, &read_baseline_ticks);
    failed |= !run_read(&timing, &read_timing_ticks);
    /* Completed adjacency is not overlap, so every normal synchronous cycle misses. */
    /* Two reset-segment windows; no speculative wrapped tail past CS.limit. */
    failed |= timing_ticks != baseline_ticks + 4u;
    failed |= port_wait_ticks != baseline_ticks + 1u;
    failed |= write_timing_ticks != write_baseline_ticks + 4u;
    failed |= read_timing_ticks != read_baseline_ticks + 4u;
    failed |= !external_cycle_observer_contract();
    failed |= !refresh_external_cycle_contract();
    failed |= !retirement_wait_contract();
    failed |= !prefetch_grant_contract();
    failed |= !cecg_port_wait_contract();
    failed |= !d4_cecg_memory_class_contract();
    failed |= !cecg_aperture_wait_contract();
    if (failed != 0) return 1;
    lib_c_printf("M5:T412:S1:EXTERNAL-READ-LOCALITY:OK\n");
    lib_c_printf("M5:T413:S1:EXTERNAL-WRITE-BRIDGE:OK\n");
    lib_c_printf("M5:T414:S1:DATA-READ-LOCALITY:OK\n");
    lib_c_printf("M5:T415:S1:PAGE-WALK-LOCALITY:OK\n");
    lib_c_printf("M5:T416:S1:DMA-HOLD-LOCALITY:OK\n");
    lib_c_printf("M5:T417:S1:REFRESH-LOCALITY:OK\n");
    lib_c_printf("M5:T418:S1:INSTRUCTION-BOUNDARY-LOCALITY:OK\n");
    lib_c_printf("M5:T419:S5:EXTERNAL-CYCLE-OVERLAP:OK\n");
    lib_c_printf("M5:T423:S1:CPU-BOARD-TRANSACTION:OK\n");
    lib_c_printf("M5:T428:S1:GENERIC-PREFETCH-PRODUCER:OK\n");
    lib_c_printf("M5:T429:S1:CECG-8BIT-BUS-WAIT:OK\n");
    lib_c_printf("M5:T429:S2:D4-CECG-MEMORY-CLASS:OK\n");
    lib_c_printf("M5:T429:S3:CECG-APERTURE-WAIT:OK\n");
    return 0;
}
