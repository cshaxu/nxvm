#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "x86-core/machine/machine.h"
#include "x86-core/machine/debug_interface.h"
#include "x86-core/machine/transaction.h"
#include "x86-core/machine/attachment_interface.h"

static x86_core_cpu_execution_context *prefetch_observed_cpu;
static lib_u32 prefetch_grants;
static lib_u64 bus_granted_ticks;
static lib_u64 bus_blocked_ticks;
static lib_u64 bus_invalidation_cycles;

static lib_status prefetch_reset_mapping(x86_core_machine *machine)
{
    const x86_core_machine_memory_alias_config alias = {0xfffffff0u, 0xffff0u, 16u};
    return x86_core_machine_install_memory_aliases(machine, &alias, 1u, LIB_FALSE);
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
void test_cpu_prefetch_grant(x86_core_cpu_execution_context *cpu)
{
    if (cpu == prefetch_observed_cpu) ++prefetch_grants;
    x86_core_cpu_execution_advance_prefetch_reservation(cpu);
}

void test_cpu_bus_service(x86_core_cpu_execution_context *cpu,
    lib_u64 ticks, lib_bool available, lib_u64 invalidation_cycles)
{
    if (cpu == prefetch_observed_cpu) {
        if (available) bus_granted_ticks += ticks;
        else bus_blocked_ticks += ticks;
        bus_invalidation_cycles += invalidation_cycles;
    }
    x86_core_cpu_execution_advance_bus(cpu, ticks, available, invalidation_cycles);
}

static lib_u64 external_cycle_begin_and_commit(
    x86_core_cpu_external_cycle_provider provider, void *context,
    lib_u32 physical, lib_u8 bytes, lib_u8 write,
    x86_core_cpu_memory_access_provenance provenance)
{
    provider(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN, X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, physical,
        bytes, write, provenance);
    return provider(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT, X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, physical,
        bytes, write, provenance);
}

static lib_i32 run_halt(const x86_core_machine_external_cycle_timing *timing,
    lib_u64 *out_ticks)
{
    static const lib_u8 code[] = {
        0xebu, 0x01u, 0x90u, 0xb0u, 0x5au, 0xe6u, 0xe0u, 0xf4u
    };
    x86_core_machine_executor_config config = { .memory_bytes = 16u * 1024u * 1024u };
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_run_result result;
    lib_i32 failed = 0;

    config.cpu_profile = X86_CORE_CPU_PROFILE_80386;
    config.transaction_contract.external_cycle_timing = *timing;
    failed |= x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
    failed |= prefetch_reset_mapping(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_memory_write(machine, 0x000ffff0u, code, sizeof(code)) !=
        LIB_STATUS_OK;
    failed |= x86_core_machine_run(machine, (x86_core_machine_run_budget){5u, 0u},
        &result) != LIB_STATUS_OK;
    if (!failed && out_ticks != LIB_NULL) *out_ticks = result.ticks;
    x86_core_machine_destroy(machine);
    return !failed;
}

static lib_i32 run_halt_with_port_wait(lib_u64 *out_ticks)
{
    static const lib_u8 code[] = {
        0xebu, 0x01u, 0x90u, 0xb0u, 0x5au, 0xe6u, 0xe0u, 0xf4u
    };
    x86_core_machine_executor_config config = { .memory_bytes = 16u * 1024u * 1024u };
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_run_result result;
    lib_i32 failed = 0;

    config.cpu_profile = X86_CORE_CPU_PROFILE_80386;
    config.transaction_contract.external_access_wait_windows[0] =
        (x86_core_machine_external_access_wait_window) {
            X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x00e0u, 0x00e0u, 1u};
    failed |= x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
    failed |= prefetch_reset_mapping(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_memory_write(machine, 0x000ffff0u, code, sizeof(code)) !=
        LIB_STATUS_OK;
    failed |= x86_core_machine_run(machine, (x86_core_machine_run_budget){5u, 0u},
        &result) != LIB_STATUS_OK;
    if (!failed && out_ticks != LIB_NULL) *out_ticks = result.ticks;
    x86_core_machine_destroy(machine);
    return !failed;
}
static lib_i32 run_write(const x86_core_machine_external_cycle_timing *timing,
    lib_u64 *out_ticks)
{
    static const lib_u8 code[] = {
        0xb0u, 0x5au, 0xa2u, 0x10u, 0x00u, 0xf4u
    };
    x86_core_machine_executor_config config = { .memory_bytes = 16u * 1024u * 1024u };
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_run_result result;
    lib_u8 value = 0u;
    lib_i32 failed = 0;

    config.cpu_profile = X86_CORE_CPU_PROFILE_80386;
    config.transaction_contract.external_cycle_timing = *timing;
    failed |= x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
    failed |= prefetch_reset_mapping(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_memory_write(machine, 0x000ffff0u, code, sizeof(code)) !=
        LIB_STATUS_OK;
    failed |= x86_core_machine_run(machine, (x86_core_machine_run_budget){3u, 0u},
        &result) != LIB_STATUS_OK;
    failed |= x86_core_machine_memory_read(machine, 0x10u, &value, 1u) !=
        LIB_STATUS_OK || value != 0x5au;
    if (!failed && out_ticks != LIB_NULL) *out_ticks = result.ticks;
    x86_core_machine_destroy(machine);
    return !failed;
}

static lib_i32 run_read(const x86_core_machine_external_cycle_timing *timing,
    lib_u64 *out_ticks)
{
    static const lib_u8 code[] = { 0xa0u, 0x10u, 0x00u, 0xf4u };
    static const lib_u8 data = 0x5au;
    x86_core_machine_executor_config config = { .memory_bytes = 16u * 1024u * 1024u };
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_run_result result;
    lib_i32 failed = 0;

    config.cpu_profile = X86_CORE_CPU_PROFILE_80386;
    config.transaction_contract.external_cycle_timing = *timing;
    failed |= x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
    failed |= prefetch_reset_mapping(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_memory_write(machine, 0x000ffff0u, code, sizeof(code)) !=
        LIB_STATUS_OK;
    failed |= x86_core_machine_memory_write(machine, 0x10u, &data, 1u) != LIB_STATUS_OK;
    failed |= x86_core_machine_run(machine, (x86_core_machine_run_budget){2u, 0u},
        &result) != LIB_STATUS_OK;
    if (!failed && out_ticks != LIB_NULL) *out_ticks = result.ticks;
    x86_core_machine_destroy(machine);
    return !failed;
}

static lib_i32 external_cycle_observer_contract(void)
{
    static const x86_core_machine_external_cycle_timing explicit_timing = {2048u,
        2u, 1u, X86_CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_EXPLICIT_SEQUENTIAL, 0u, 0u};
    x86_core_machine_executor_config config = { .memory_bytes = 16u * 1024u * 1024u, .cpu_profile = X86_CORE_CPU_PROFILE_80386 };
    x86_core_machine *machine = LIB_NULL;
    x86_core_cpu_external_cycle_provider provider;
    void *context;
    lib_i32 failed = 0;

    config.transaction_contract.external_cycle_timing = explicit_timing;
    config.transaction_contract.external_access_wait_windows[0] =
        (x86_core_machine_external_access_wait_window) {
            X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, 0x2000u, 0x2fffu, 3u};
    failed |= x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
    provider = x86_core_machine_cpu_external_cycle_trace;
    context = machine;
    failed |= provider == LIB_NULL;
    if (!failed) {
        failed |= provider(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN, X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, 0u, 1u,
            LIB_FALSE, X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH) != 0u;
        failed |= provider(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_OVERLAP_DECLARE,
            X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, 1u, 1u, LIB_FALSE,
            X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH) != 0u;
        failed |= provider(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT, X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, 0u, 1u,
            LIB_FALSE, X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH) != 2u;
        failed |= external_cycle_begin_and_commit(provider, context, 1u, 1u, LIB_FALSE,
            X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH) != 1u;
        failed |= !machine->external_cycle_page_valid ||
            machine->external_cycle_round_ticks != 3u;
        machine->external_cycle_round_ticks = 0u;
        machine->external_cycle_page_valid = LIB_FALSE;
        provider(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN, X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, 0x800u, 4u,
            LIB_FALSE, X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ);
        failed |= provider(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL, X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, 0x800u, 4u,
            LIB_FALSE, X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ) != 0u;
        failed |= provider(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT, X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, 0x800u, 4u,
            LIB_FALSE, X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ) != 0u;
        failed |= machine->external_cycle_page_valid ||
            machine->external_cycle_pending_valid ||
            machine->external_cycle_round_ticks != 0u;
        external_cycle_begin_and_commit(provider, context, 0x800u, 4u, LIB_FALSE,
            X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ);
        external_cycle_begin_and_commit(provider, context, 0x804u, 4u, LIB_TRUE,
            X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_WRITE);
        failed |= !machine->external_cycle_page_valid ||
            machine->external_cycle_round_ticks != 4u;
        machine->external_cycle_round_ticks = 0u;
        provider(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN, X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, 0x808u, 4u,
            LIB_FALSE, X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ);
        failed |= x86_core_machine_transaction_hold_request(&machine->transaction,
            X86_CORE_MACHINE_TRANSACTION_OWNER_DMA, 0u) != LIB_STATUS_OK;
        failed |= x86_core_machine_transaction_hold_acknowledge(&machine->transaction,
            X86_CORE_MACHINE_TRANSACTION_OWNER_DMA) != LIB_STATUS_OK;
        failed |= machine->external_cycle_page_valid ||
            machine->external_cycle_pending_valid || machine->external_cycle_overlap_valid;
        x86_core_machine_transaction_hold_release(&machine->transaction,
            X86_CORE_MACHINE_TRANSACTION_OWNER_DMA);
        external_cycle_begin_and_commit(provider, context, 0x808u, 4u, LIB_FALSE,
            X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ);
        failed |= !machine->external_cycle_page_valid ||
            machine->external_cycle_round_ticks != 2u;
        failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
        failed |= machine->external_cycle_page_valid ||
            machine->external_cycle_pending_valid || machine->external_cycle_overlap_valid ||
            machine->external_cycle_round_ticks != 0u;
        failed |= external_cycle_begin_and_commit(provider, context, 0x2000u, 1u, LIB_TRUE,
            X86_CORE_CPU_MEMORY_ACCESS_DATA) != 5u || machine->external_cycle_round_ticks != 5u;
        machine->external_cycle_round_ticks = LIB_UINT64_MAX;
        failed |= external_cycle_begin_and_commit(provider, context, 0x2000u, 1u, LIB_TRUE,
            X86_CORE_CPU_MEMORY_ACCESS_DATA) != 5u || !machine->external_cycle_round_overflow ||
            machine->external_cycle_round_ticks != LIB_UINT64_MAX;
    }
    x86_core_machine_destroy(machine);
    return !failed;
}

static lib_i32 refresh_external_cycle_contract(void)
{
    const x86_core_machine_external_cycle_timing timing = {2048u, 2u, 1u,
        X86_CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED, 0u, 0u};
    const x86_core_machine_executor_config config = {
        .memory_bytes = 16u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .transaction_contract = {.external_cycle_timing = timing}
    };
    x86_core_machine *machine = LIB_NULL;
    lib_i32 failed = x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK;

    if (!failed) {
        external_cycle_begin_and_commit(x86_core_machine_cpu_external_cycle_trace, machine,
            0x800u, 4u, LIB_FALSE, X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ);
        failed |= !machine->external_cycle_page_valid ||
            machine->external_cycle_round_ticks != 2u;
        machine->external_cycle_round_ticks = 0u;
        x86_core_machine_cpu_bus_refresh_pulse(machine);
        failed |= machine->external_cycle_page_valid || machine->external_cycle_pending_valid;
        external_cycle_begin_and_commit(x86_core_machine_cpu_external_cycle_trace, machine,
            0x804u, 4u, LIB_FALSE, X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ);
        failed |= !machine->external_cycle_page_valid ||
            machine->external_cycle_round_ticks != 2u;
        failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
        failed |= machine->external_cycle_page_valid || machine->external_cycle_pending_valid ||
            machine->external_cycle_round_ticks != 0u;
    }
    x86_core_machine_destroy(machine);
    return !failed;
}

static lib_i32 retirement_wait_contract(x86_core_cpu_profile profile)
{
    static const lib_u8 code[] = {0xa0u, 0x10u, 0x00u, 0xf4u};
    static const lib_u8 data = 0x5au;
    static const x86_core_machine_external_cycle_timing timing = {2048u, 2u, 0u,
        X86_CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED, 0u, 0u};
    x86_core_machine_executor_config config = { .memory_bytes = 16u * 1024u * 1024u };
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_run_result result;
    lib_i32 failed = 0;

    config.cpu_profile = X86_CORE_CPU_PROFILE_80386;
    config.transaction_contract.external_cycle_timing = timing;
    config.transaction_contract.cpu_cycle_bus_ready_gate_enabled = LIB_TRUE;
    failed |= x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
    failed |= prefetch_reset_mapping(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
    /* Internal CPU/Core wait qualification, not production SX admission. */
    const x86_core_cpu_instruction_timing instruction_timing = {.base_ticks = 1u};
    machine->cpu_profile = profile;
    x86_core_cpu_execution_context_bind_profiles(machine->executor_cpu_execution,
        profile, X86_FPU_PROFILE_NONE, LIB_FALSE, &instruction_timing);
    failed |= x86_core_machine_memory_write(machine, 0x000ffff0u, code, sizeof(code)) !=
        LIB_STATUS_OK;
    failed |= x86_core_machine_memory_write(machine, 0x10u, &data, 1u) != LIB_STATUS_OK;
    machine->maximum_instruction_ticks = profile == X86_CORE_CPU_PROFILE_80486SX ? 0u : 1u;
    failed |= x86_core_machine_run(machine, (x86_core_machine_run_budget){0u, 1u},
        &result) != LIB_STATUS_OK;
    failed |= result.reason != X86_CORE_MACHINE_STOP_BUDGET || result.executed != 0u ||
        result.ticks != 1u || machine->cpu_retirement_wait_pending == LIB_FALSE ||
        (profile == X86_CORE_CPU_PROFILE_80486SX ?
            machine->cpu_retirement_completion_ticks == 0u : machine->cpu_retirement_wait_ticks == 0u);
    failed |= x86_core_machine_set_cpu_bus_ready(machine, 0) != LIB_STATUS_OK;
    failed |= x86_core_machine_run(machine, (x86_core_machine_run_budget){0u, 1u},
        &result) != LIB_STATUS_OK;
    failed |= result.executed != 0u || result.ticks != 1u ||
        (profile == X86_CORE_CPU_PROFILE_80486SX ?
            machine->cpu_retirement_completion_ticks == 0u : machine->cpu_retirement_wait_ticks == 0u);
    failed |= x86_core_machine_set_cpu_bus_ready(machine, 1) != LIB_STATUS_OK;
    failed |= x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u},
        &result) != LIB_STATUS_OK;
    failed |= result.executed != 1u || result.ticks <= 1u ||
        machine->cpu_retirement_wait_pending != LIB_FALSE;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        machine->cpu_cycle_bus_ready != LIB_TRUE;
    /* A deferred board wait has no physical source disposition yet.
     * It must therefore stop before publishing a synthetic physical tick. */
    machine->retirement_time_contract = X86_CORE_MACHINE_RETIREMENT_TIME_PHYSICAL;
    machine->time_axis = (x86_core_machine_time_axis) {
        X86_CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL, 8000000u };
    failed |= x86_core_machine_memory_write(machine, 0x000ffff0u, code, sizeof(code)) !=
        LIB_STATUS_OK || x86_core_machine_memory_write(machine, 0x10u, &data, 1u) !=
        LIB_STATUS_OK;
    machine->maximum_instruction_ticks = profile == X86_CORE_CPU_PROFILE_80486SX ? 0u : 1u;
    failed |= x86_core_machine_run(machine, (x86_core_machine_run_budget){0u, 1u},
        &result) != LIB_STATUS_INTERNAL_ERROR || result.elapsed_ticks != 0u ||
        machine->elapsed_ticks != 0u;
    x86_core_machine_destroy(machine);
    return !failed;
}

static lib_i32 faulted_external_wait_contract(x86_core_cpu_profile profile)
{
    static const lib_u8 code[] = {0x67u, 0xa4u, 0xf4u};
    static const lib_u8 source = 0x5au;
    x86_core_machine_executor_config config = { .memory_bytes = 16u * 1024u * 1024u };
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_run_result result;
    lib_status status;
    lib_i32 failed = 0;
    const x86_core_cpu_instruction_timing instruction_timing = {.base_ticks = 1u};
    const lib_bool sx = profile == X86_CORE_CPU_PROFILE_80486SX;
    /* SX preflights both MOVS operands before its data read; its preceding
     * committed fetch, rather than an absent data cycle, supplies the wait. */
    const lib_u32 wait_address = sx ? 0xfffffff0u : 0x100u;

    config.cpu_profile = X86_CORE_CPU_PROFILE_80386;
    config.transaction_contract.external_access_wait_windows[0] =
        (x86_core_machine_external_access_wait_window) {
            X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, wait_address, wait_address, 1u};
    failed |= x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
    failed |= prefetch_reset_mapping(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
    /* Internal CPU/Executor timing proof; production SX admission stays closed. */
    machine->cpu_profile = profile;
    x86_core_cpu_execution_context_bind_profiles(machine->executor_cpu_execution,
        profile, X86_FPU_PROFILE_NONE, LIB_FALSE, &instruction_timing);
    failed |= x86_core_machine_memory_write(machine, 0x000ffff0u, code, sizeof(code)) !=
        LIB_STATUS_OK;
    failed |= x86_core_machine_memory_write(machine, 0x100u, &source, 1u) != LIB_STATUS_OK;
    machine->maximum_instruction_ticks = sx ? 0u : 1u;
    failed |= x86_core_machine_debug_write_register(machine, X86_CORE_CPU_DEBUG_ESI, 0x100u) !=
        LIB_STATUS_OK;
    failed |= x86_core_machine_debug_write_register(machine, X86_CORE_CPU_DEBUG_EDI, 0x00010000u) !=
        LIB_STATUS_OK;
    status = x86_core_machine_run(machine, (x86_core_machine_run_budget){0u, 1u},
        &result);
    failed |= status != LIB_STATUS_OK;
    failed |= result.reason != X86_CORE_MACHINE_STOP_BUDGET || result.executed != 0u ||
        result.ticks != 1u || result.elapsed_ticks != 1u ||
        machine->elapsed_ticks != 1u || machine->cpu_retirement_wait_pending != sx ||
        machine->cpu_retirement_wait_retires != LIB_FALSE;
    if (sx && !failed) {
        failed |= machine->cpu_retirement_wait_ticks != 0u ||
            machine->cpu_retirement_completion_ticks != 36u;
        /* Entry elapsed time can span budgets without another retirement or
         * losing the committed prefix; completion owns the one remaining tick. */
        failed |= x86_core_machine_run(machine, (x86_core_machine_run_budget){0u, 35u},
            &result) != LIB_STATUS_OK || result.executed != 0u || result.ticks != 35u ||
            result.elapsed_ticks != 36u || !machine->cpu_retirement_wait_pending ||
            machine->cpu_retirement_completion_ticks != 1u;
        failed |= x86_core_machine_run(machine, (x86_core_machine_run_budget){0u, 1u},
            &result) != LIB_STATUS_OK || result.executed != 0u || result.ticks != 1u ||
            result.elapsed_ticks != 37u || machine->elapsed_ticks != 37u ||
            machine->cpu_retirement_wait_pending || machine->cpu_retirement_wait_retires;
    }
    if (failed)
        lib_c_printf("Fault wait: profile=%u status=%u ticks=%llu elapsed=%llu executed=%llu pending=%u wait=%llu completion=%llu\n",
            (lib_u32)profile, (lib_u32)status, result.ticks, result.elapsed_ticks, result.executed,
            machine->cpu_retirement_wait_pending, machine->cpu_retirement_wait_ticks,
            machine->cpu_retirement_completion_ticks);
    x86_core_machine_destroy(machine);
    return !failed;
}
static lib_i32 cecg_aperture_wait_contract(void)
{
    x86_core_machine_executor_config config = { .memory_bytes = 16u * 1024u * 1024u, .cpu_profile = X86_CORE_CPU_PROFILE_80386 };
    x86_core_machine *machine = LIB_NULL;
    x86_core_cpu_external_cycle_provider provider;
    void *context;
    lib_i32 failed = 0;

    config.transaction_contract.external_access_wait_windows[0] =
        (x86_core_machine_external_access_wait_window) {
            X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, 0x000a0000u,
            0x000affffu, 1u};
    failed |= x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
    provider = x86_core_machine_cpu_external_cycle_trace;
    context = machine;
    if (!failed) {
        external_cycle_begin_and_commit(provider, context, 0x000a0000u, 1u,
            LIB_TRUE, X86_CORE_CPU_MEMORY_ACCESS_DATA);
        failed |= machine->external_cycle_round_ticks != 1u;
        provider(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN,
            X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, 0x000a0001u, 1u,
            LIB_FALSE, X86_CORE_CPU_MEMORY_ACCESS_DATA);
        provider(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL,
            X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, 0x000a0001u, 1u,
            LIB_FALSE, X86_CORE_CPU_MEMORY_ACCESS_DATA);
        failed |= machine->external_cycle_round_ticks != 1u;
        failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK ||
            machine->external_cycle_round_ticks != 0u;
    }
    x86_core_machine_destroy(machine);
    return !failed;
}
static lib_i32 d4_cecg_memory_class_contract(void)
{
    x86_core_machine_executor_config config = { .memory_bytes = 16u * 1024u * 1024u, .cpu_profile = X86_CORE_CPU_PROFILE_80386 };
    x86_core_machine *machine = LIB_NULL;
    x86_core_cpu_external_cycle_provider provider;
    void *context;
    lib_i32 failed = 0;

    config.transaction_contract.external_cycle_timing = (x86_core_machine_external_cycle_timing) {
        2048u, 2u, 0u, X86_CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED,
        0u, 0x0009ffffu};
    failed |= x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
    provider = x86_core_machine_cpu_external_cycle_trace;
    context = machine;
    if (!failed) {
        external_cycle_begin_and_commit(provider, context, 0x0009ff00u, 1u,
            LIB_FALSE, X86_CORE_CPU_MEMORY_ACCESS_DATA);
        failed |= machine->external_cycle_round_ticks != 2u;
        machine->external_cycle_round_ticks = 0u;
        external_cycle_begin_and_commit(provider, context, 0x000a0000u, 1u,
            LIB_FALSE, X86_CORE_CPU_MEMORY_ACCESS_DATA);
        failed |= machine->external_cycle_round_ticks != 0u;
    }
    x86_core_machine_destroy(machine);
    return !failed;
}
static lib_i32 cecg_port_wait_contract(void)
{
    x86_core_machine_executor_config config = { .memory_bytes = 16u * 1024u * 1024u, .cpu_profile = X86_CORE_CPU_PROFILE_80386 };
    x86_core_machine *machine = LIB_NULL;
    x86_core_cpu_external_cycle_provider provider;
    void *context;
    lib_i32 failed = 0;

    config.transaction_contract.external_access_wait_windows[0] =
        (x86_core_machine_external_access_wait_window) {
            X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x03c0u, 0x03cfu, 1u};
    failed |= x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
    provider = x86_core_machine_cpu_external_cycle_trace;
    context = machine;
    failed |= provider == LIB_NULL;
    if (!failed) {
        provider(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN,
            X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x03c0u, 1u, LIB_TRUE,
            X86_CORE_CPU_MEMORY_ACCESS_DATA);
        provider(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT,
            X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x03c0u, 1u, LIB_TRUE,
            X86_CORE_CPU_MEMORY_ACCESS_DATA);
        failed |= machine->external_cycle_round_ticks != 1u ||
            machine->external_cycle_pending_valid;
        provider(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN,
            X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x03d0u, 1u, LIB_FALSE,
            X86_CORE_CPU_MEMORY_ACCESS_DATA);
        provider(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT,
            X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x03d0u, 1u, LIB_FALSE,
            X86_CORE_CPU_MEMORY_ACCESS_DATA);
        failed |= machine->external_cycle_round_ticks != 1u;
        provider(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN,
            X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x03c1u, 1u, LIB_FALSE,
            X86_CORE_CPU_MEMORY_ACCESS_DATA);
        provider(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL,
            X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x03c1u, 1u, LIB_FALSE,
            X86_CORE_CPU_MEMORY_ACCESS_DATA);
        failed |= machine->external_cycle_round_ticks != 1u ||
            machine->external_cycle_pending_valid;
        failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK ||
            machine->external_cycle_round_ticks != 0u ||
            machine->external_cycle_pending_valid;
    }
    x86_core_machine_destroy(machine);
    return !failed;
}
typedef struct invalidation_fixture {
    x86_core_machine *machine;
    lib_bool pending;
    lib_u8 mode;
    lib_status status;
} invalidation_fixture;

static lib_u64 invalidation_dma_ticks(void *owner, lib_u64 source_ticks)
{
    return ((invalidation_fixture *)owner)->pending ? source_ticks : 0u;
}

static lib_bool invalidation_dma_request(void *owner)
{
    return ((invalidation_fixture *)owner)->pending;
}

static void invalidation_dma_advance(void *owner, lib_u64 ticks)
{
    invalidation_fixture *fixture = owner;
    x86_core_machine *machine = fixture->machine;
    lib_u16 value = 0x1234u;

    if (ticks == 0u || !fixture->pending) return;
    fixture->pending = LIB_FALSE;
    if (fixture->mode < 2u) {
        fixture->status = x86_core_machine_dma_memory_cycle(machine, 0x8000u, 2u, 1u,
            fixture->mode == 0u ? X86_CORE_MACHINE_MEMORY_ACCESS_WRITE :
                X86_CORE_MACHINE_MEMORY_ACCESS_READ,
            &value, LIB_NULL, LIB_NULL, LIB_NULL);
        return;
    }
    fixture->status = x86_core_machine_transaction_begin(&machine->transaction,
        X86_CORE_MACHINE_TRANSACTION_OWNER_DMA, X86_CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE,
        0x8000u, 2u, 1u);
    if (fixture->status != LIB_STATUS_OK) return;
    /* Inject cancellation after an accepted prefix, or before any write. */
    if (fixture->mode == 2u)
        fixture->status = x86_core_machine_memory_write_physical(&machine->executor_memory,
            0x8000u, (lib_uptr)&value, 1u);
    x86_core_machine_transaction_cancel(&machine->transaction);
}

static lib_i32 invalidation_cycle_contract(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 1024u * 1024u, .cpu_profile = X86_CORE_CPU_PROFILE_80386};
    const x86_core_machine_memory_alias_config aliases[] = {
        {0xfffffff0u, 0xffff0u, 16u}, {0x8000u, 0x2000u, 16u}, {0x9000u, 0x2000u, 16u}};
    invalidation_fixture fixture = {0};
    const x86_core_machine_attachment attachment = {
        .dma_ticks = invalidation_dma_ticks, .dma_request = invalidation_dma_request,
        .dma_advance = invalidation_dma_advance, .finalize_devices = prefetch_finalize,
        .context = &fixture};
    x86_core_machine *machine = LIB_NULL;
    lib_i32 failed = 0;

    failed |= x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
    if (failed) return 0;
    fixture.machine = machine;
    failed |= x86_core_machine_install_memory_aliases(machine, aliases,
        sizeof(aliases) / sizeof(aliases[0]), LIB_FALSE) != LIB_STATUS_OK;
    failed |= x86_core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK;
    failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
    if (!failed) {
        const x86_core_cpu_instruction_timing instruction_timing = {.base_ticks = 1u};
        machine->cpu_profile = X86_CORE_CPU_PROFILE_80486SX;
        machine->maximum_instruction_ticks = 0u;
        x86_core_cpu_execution_context_bind_profiles(machine->executor_cpu_execution,
            X86_CORE_CPU_PROFILE_80486SX, X86_FPU_PROFILE_NONE, LIB_FALSE, &instruction_timing);
        prefetch_observed_cpu = machine->executor_cpu_execution;
        for (lib_u8 mode = 0u; mode < 4u; ++mode) {
            x86_core_machine_run_result result;
            const lib_u64 expected = mode == 0u || mode == 2u ? 1u : 0u;

            fixture.mode = mode;
            fixture.pending = LIB_TRUE;
            bus_invalidation_cycles = 0u;
            failed |= x86_core_machine_advance_time(machine, 1u) != LIB_STATUS_OK ||
                fixture.pending || fixture.status != LIB_STATUS_OK ||
                machine->bus_invalidation_cycles != expected || bus_invalidation_cycles != 0u;
            failed |= x86_core_machine_memory_write(machine, 0xffff0u,
                (const lib_u8[]){0x90u,0x90u,0x90u,0x90u}, 4u) != LIB_STATUS_OK;
            fixture.pending = LIB_TRUE;
            bus_invalidation_cycles = 0u;
            failed |= x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u},
                &result) != LIB_STATUS_OK || result.executed != 1u ||
                fixture.pending || fixture.status != LIB_STATUS_OK ||
                machine->bus_invalidation_cycles != expected || bus_invalidation_cycles != expected;
        }
        /* Host writes outside a DMA transaction do not create external cycles. */
        const lib_u64 previous = machine->bus_invalidation_cycles;
        failed |= x86_core_machine_memory_write(machine, 0x8000u,
            (const lib_u8[]){0xa5u}, 1u) != LIB_STATUS_OK ||
            machine->bus_invalidation_cycles != previous;
        failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK ||
            machine->bus_invalidation_cycles != 0u || machine->bus_invalidation_seen;
    }
    prefetch_observed_cpu = LIB_NULL;
    x86_core_machine_destroy(machine);
    return !failed;
}

static lib_i32 prefetch_grant_contract(x86_core_cpu_profile profile)
{
    static const x86_core_machine_external_cycle_timing timing = {2048u, 2u, 0u,
        X86_CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED, 0u, 0u};
    x86_core_machine_executor_config config = { .memory_bytes = 16u * 1024u * 1024u };
    x86_core_machine *machine = LIB_NULL;
    lib_bool refresh_pending = LIB_FALSE;
    const x86_core_machine_attachment attachment = {
        .refresh_request = prefetch_refresh_request,
        .refresh_complete = prefetch_refresh_complete,
        .finalize_devices = prefetch_finalize,
        .context = &refresh_pending
    };
    lib_i32 failed = 0;

    config.cpu_profile = X86_CORE_CPU_PROFILE_80386;
    config.transaction_contract.external_cycle_timing = timing;
    config.transaction_contract.cpu_prefetch_reservation_enabled = LIB_TRUE;
    config.transaction_contract.cpu_cycle_bus_ready_gate_enabled = LIB_TRUE;
    failed |= x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
    failed |= !failed && x86_core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK;
    failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
    if (!failed) {
        const x86_core_cpu_instruction_timing instruction_timing = {.base_ticks = 1u};
        machine->cpu_profile = profile;
        machine->maximum_instruction_ticks = x86_core_cpu_timing_maximum_ticks(profile,
            &instruction_timing);
        x86_core_cpu_execution_context_bind_profiles(machine->executor_cpu_execution,
            profile, X86_FPU_PROFILE_NONE, LIB_FALSE, &instruction_timing);
        prefetch_observed_cpu = machine->executor_cpu_execution;
        prefetch_grants = 0u;
        bus_granted_ticks = bus_blocked_ticks = 0u;
        failed |= x86_core_machine_advance_time(machine, 1u) != LIB_STATUS_OK;
        failed |= prefetch_grants != 1u ||
            bus_granted_ticks != 1u || bus_blocked_ticks != 0u ||
            machine->external_cycle_overlap_valid ||
            machine->external_cycle_round_ticks != 0u;
        failed |= x86_core_machine_transaction_hold_request(&machine->transaction,
            X86_CORE_MACHINE_TRANSACTION_OWNER_DMA, 0u) != LIB_STATUS_OK;
        failed |= x86_core_machine_transaction_hold_acknowledge(&machine->transaction,
            X86_CORE_MACHINE_TRANSACTION_OWNER_DMA) != LIB_STATUS_OK;
        failed |= x86_core_machine_advance_time(machine, 1u) != LIB_STATUS_OK ||
            prefetch_grants != 1u || bus_granted_ticks != 1u || bus_blocked_ticks != 1u;
        x86_core_machine_transaction_hold_release(&machine->transaction,
            X86_CORE_MACHINE_TRANSACTION_OWNER_DMA);
        refresh_pending = LIB_TRUE;
        failed |= x86_core_machine_advance_time(machine, 1u) != LIB_STATUS_OK ||
            prefetch_grants != 1u || bus_granted_ticks != 1u || bus_blocked_ticks != 2u;
        failed |= x86_core_machine_advance_time(machine, 1u) != LIB_STATUS_OK ||
            prefetch_grants != 2u || bus_granted_ticks != 2u || bus_blocked_ticks != 2u;
        failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
        prefetch_grants = 0u;
        failed |= x86_core_machine_advance_time(machine, 1u) != LIB_STATUS_OK ||
            prefetch_grants != 1u || bus_granted_ticks != 3u || bus_blocked_ticks != 2u;
        failed |= x86_core_machine_set_cpu_bus_ready(machine, LIB_FALSE) != LIB_STATUS_OK ||
            x86_core_machine_advance_time(machine, 1u) != LIB_STATUS_OK ||
            bus_granted_ticks != 3u || bus_blocked_ticks != 3u;
        failed |= x86_core_machine_set_cpu_bus_ready(machine, LIB_TRUE) != LIB_STATUS_OK ||
            x86_core_machine_advance_time(machine, 1u) != LIB_STATUS_OK ||
            bus_granted_ticks != 4u || bus_blocked_ticks != 3u;
    }
    prefetch_observed_cpu = LIB_NULL;
    x86_core_machine_destroy(machine);
    return !failed;
}
lib_i32 main(void)
{
    static const x86_core_machine_external_cycle_timing disabled = {0u, 0u, 0u,
        X86_CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED, 0u, 0u};
    static const x86_core_machine_external_cycle_timing timing = {2048u, 2u, 1u,
        X86_CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED, 0u, 0u};
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
    failed |= !retirement_wait_contract(X86_CORE_CPU_PROFILE_80386);
    failed |= !retirement_wait_contract(X86_CORE_CPU_PROFILE_80486SX);
    failed |= !faulted_external_wait_contract(X86_CORE_CPU_PROFILE_80386);
    failed |= !faulted_external_wait_contract(X86_CORE_CPU_PROFILE_80486SX);
    failed |= !prefetch_grant_contract(X86_CORE_CPU_PROFILE_80386);
    failed |= !prefetch_grant_contract(X86_CORE_CPU_PROFILE_80486SX);
    failed |= !invalidation_cycle_contract();
    failed |= !cecg_port_wait_contract();
    failed |= !d4_cecg_memory_class_contract();
    failed |= !cecg_aperture_wait_contract();
    if (failed != 0) return 1;
    lib_c_printf("EXTERNAL-READ-LOCALITY:OK\n");
    lib_c_printf("EXTERNAL-WRITE-BRIDGE:OK\n");
    lib_c_printf("DATA-READ-LOCALITY:OK\n");
    lib_c_printf("PAGE-WALK-LOCALITY:OK\n");
    lib_c_printf("DMA-HOLD-LOCALITY:OK\n");
    lib_c_printf("REFRESH-LOCALITY:OK\n");
    lib_c_printf("INSTRUCTION-BOUNDARY-LOCALITY:OK\n");
    lib_c_printf("EXTERNAL-CYCLE-OVERLAP:OK\n");
    lib_c_printf("CPU-BOARD-TRANSACTION:OK\n");
    lib_c_printf("GENERIC-PREFETCH-PRODUCER:OK\n");
    lib_c_printf("CECG-8BIT-BUS-WAIT:OK\n");
    lib_c_printf("D4-CECG-MEMORY-CLASS:OK\n");
    lib_c_printf("CECG-APERTURE-WAIT:OK\n");
    return 0;
}
