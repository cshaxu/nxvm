#include "lib/types/file.h"
#include "core/x86/machine.h"
#include "core/x86/attachment_interface.h"
#include "core/x86/debug_interface.h"
#include "x86-core/cpu/cpu_instructions.h"

typedef struct power_probe {
    core_machine *machine;
    x86_core_cpu_special_cycle cycle;
    lib_u32 polls;
    lib_u32 commits;
    lib_u32 cancels;
    lib_u64 ticks;
    lib_bool ready;
    lib_bool fail;
    lib_bool bad_order;
    lib_bool finalized;
} power_probe;

static lib_status power_cycle(void *opaque, x86_core_cpu_special_cycle cycle,
    lib_bool *ready)
{
    power_probe *probe = opaque;
    const core_machine_transaction_state *transaction = &probe->machine->transaction;

    if (probe->finalized || cycle != probe->cycle ||
        transaction->owner != CORE_MACHINE_TRANSACTION_OWNER_CPU ||
        transaction->kind != CORE_MACHINE_TRANSACTION_CPU_SPECIAL_CYCLE ||
        transaction->value != (lib_u32)cycle ||
        transaction->address != (cycle == X86_CORE_CPU_SPECIAL_STOP_GRANT ? 0x10u : 0u) ||
        transaction->detail != (cycle == X86_CORE_CPU_SPECIAL_SHUTDOWN ? 0x0eu : 0x0bu))
        probe->bad_order = LIB_TRUE;
    ++probe->polls;
    *ready = probe->ready;
    return probe->fail ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
}

static void power_complete(void *opaque, x86_core_cpu_special_cycle cycle, lib_bool commit)
{
    power_probe *probe = opaque;

    if (probe->finalized || cycle != probe->cycle ||
        probe->machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_CPU ||
        probe->machine->transaction.kind != CORE_MACHINE_TRANSACTION_CPU_SPECIAL_CYCLE)
        probe->bad_order = LIB_TRUE;
    if (commit) ++probe->commits;
    else ++probe->cancels;
}

static void power_ticks(void *opaque, lib_u64 ticks, lib_u64 due_tick)
{
    (void)due_tick;
    ((power_probe *)opaque)->ticks += ticks;
}

static void power_finalize(void *opaque)
{
    power_probe *probe = opaque;

    if (probe->finalized ||
        probe->machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE)
        probe->bad_order = LIB_TRUE;
    probe->finalized = LIB_TRUE;
}

static lib_bool power_run(x86_core_cpu_special_cycle cycle, lib_u8 finish)
{
    const core_machine_executor_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE,
        .transaction_contract = {.cpu_cycle_bus_ready_gate_enabled = LIB_TRUE}
    };
    const x86_core_cpu_instruction_timing timing = {.base_ticks = 1u};
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x100u}
    };
    const lib_u8 code[] = {cycle == X86_CORE_CPU_SPECIAL_HALT ? 0xf4u : 0x90u, 0x90u};
    power_probe probe = {.cycle = cycle};
    const core_machine_attachment attachment = {.context = &probe,
        .special_cycle = power_cycle, .complete_special_cycle = power_complete,
        .after_internal_advance = power_ticks, .finalize_devices = power_finalize};
    core_machine *machine = LIB_NULL;
    core_machine_run_result result = {0};
    x86_core_cpu_debug_cpu_snapshot before = {0}, after = {0};
    lib_u64 ticks = 0u;
    lib_bool failed = core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

    probe.machine = machine;
    if (!failed) {
        core_machine_attachment incomplete = attachment;

        incomplete.complete_special_cycle = LIB_NULL;
        failed = core_machine_bind_attachment(machine, &incomplete) != LIB_STATUS_INVALID_ARGUMENT ||
            core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK ||
            core_machine_bind_execution_provider(machine, LIB_NULL, LIB_NULL) != LIB_STATUS_OK ||
            core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            core_machine_reset(machine) != LIB_STATUS_OK;
    }
    if (!failed) {
        /* The public constructor remains closed to 486; this is internal
         * qualification of the real CPU/Core connection. */
        machine->cpu_profile = X86_CORE_CPU_PROFILE_80486SX;
        x86_core_cpu_execution_context_bind_profiles(machine->executor_cpu_execution,
            X86_CORE_CPU_PROFILE_80486SX, X86_FPU_PROFILE_NONE, LIB_FALSE, &timing);
        failed = core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x100u, code, sizeof(code)) != LIB_STATUS_OK;
        if (cycle == X86_CORE_CPU_SPECIAL_STOP_GRANT)
            failed = failed || core_machine_set_stop_clock(machine, LIB_TRUE) != LIB_STATUS_OK;
        else if (cycle == X86_CORE_CPU_SPECIAL_SHUTDOWN)
            x86_core_cpu_execution_request_shutdown(machine->executor_cpu_execution);
    }
    if (!failed && cycle == X86_CORE_CPU_SPECIAL_STOP_GRANT) {
        failed = core_machine_transaction_hold_request(&machine->transaction,
                CORE_MACHINE_TRANSACTION_OWNER_DMA, 0u) != LIB_STATUS_OK ||
            core_machine_transaction_hold_acknowledge(&machine->transaction,
                CORE_MACHINE_TRANSACTION_OWNER_DMA) != LIB_STATUS_OK ||
            core_machine_run(machine, (core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
            result.executed || result.ticks || probe.polls ||
            machine->executor_cpu_execution->stop_granted ||
            machine->transaction.hold_owner != CORE_MACHINE_TRANSACTION_OWNER_DMA;
        core_machine_transaction_hold_release(&machine->transaction, CORE_MACHINE_TRANSACTION_OWNER_DMA);
    }
    if (!failed) {
        failed = core_machine_run(machine, (core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            result.executed != (cycle == X86_CORE_CPU_SPECIAL_HALT ? 1u : 0u) ||
            !x86_core_cpu_is_system_waiting(machine->executor_cpu_execution) ||
            machine->executor_cpu_execution->stop_granted || probe.polls != 1u || probe.commits ||
            probe.cancels || probe.bad_order ||
            core_machine_debug_capture_cpu_snapshot(machine, X86_CORE_CPU_SNAPSHOT_CURRENT,
                &before) != LIB_STATUS_OK;
        ticks = machine->elapsed_ticks;
    }
    if (!failed) {
        failed = core_machine_advance_time(machine, 3u) != LIB_STATUS_OK ||
            machine->elapsed_ticks != ticks + 3u || probe.ticks != machine->elapsed_ticks ||
            probe.polls != 1u || probe.commits;
    }
    if (!failed && finish == 0u) {
        probe.ready = LIB_TRUE;
        failed = core_machine_set_cpu_bus_ready(machine, LIB_FALSE) != LIB_STATUS_OK ||
            core_machine_run(machine, (core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
            result.executed || result.ticks || probe.commits || probe.polls != 2u ||
            core_machine_set_cpu_bus_ready(machine, LIB_TRUE) != LIB_STATUS_OK ||
            core_machine_run(machine, (core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
            result.executed || result.ticks || probe.commits != 1u || probe.cancels ||
            machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE ||
            core_machine_debug_capture_cpu_snapshot(machine, X86_CORE_CPU_SNAPSHOT_CURRENT,
                &after) != LIB_STATUS_OK || lib_memory_compare(&before, &after, sizeof(before));
        if (!failed && cycle == X86_CORE_CPU_SPECIAL_STOP_GRANT) {
            failed = core_machine_set_clock_running(machine, LIB_FALSE) != LIB_STATUS_OK ||
                core_machine_set_stop_clock(machine, LIB_FALSE) != LIB_STATUS_INVALID_STATE ||
                core_machine_set_nmi(machine, LIB_TRUE) != LIB_STATUS_OK ||
                core_machine_set_nmi(machine, LIB_FALSE) != LIB_STATUS_OK ||
                core_machine_set_clock_running(machine, LIB_TRUE) != LIB_STATUS_OK ||
                core_machine_set_flush(machine, LIB_TRUE) != LIB_STATUS_OK ||
                core_machine_set_flush(machine, LIB_FALSE) != LIB_STATUS_OK ||
                core_machine_set_stop_clock(machine, LIB_FALSE) != LIB_STATUS_OK ||
                core_machine_run(machine, (core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
                result.executed != 1u || result.reason != CORE_MACHINE_STOP_BUDGET ||
                result.linear_pc != 0x101u || probe.commits != 1u;
        }
    } else if (!failed && finish < 3u) {
        if (finish == 1u) core_machine_signal_processor_reset(machine);
        else failed = core_machine_signal_soft_processor_reset(machine) != LIB_STATUS_OK;
        failed = failed || core_machine_run(machine, (core_machine_run_budget){1u,0u}, &result) !=
                LIB_STATUS_OK || result.reason != CORE_MACHINE_STOP_RESET_REQUESTED ||
            result.executed || probe.commits || probe.cancels != 1u ||
            x86_core_cpu_is_system_waiting(machine->executor_cpu_execution) ||
            machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE;
    } else if (!failed && finish == 4u) {
        probe.fail = LIB_TRUE;
        failed = core_machine_run(machine, (core_machine_run_budget){1u,0u}, &result) !=
                LIB_STATUS_INTERNAL_ERROR || result.reason != CORE_MACHINE_STOP_FAULT ||
            result.executed || result.ticks || probe.commits || probe.cancels != 1u ||
            machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE;
    }
    core_machine_destroy(machine);
    if (failed || !probe.finalized || probe.bad_order ||
        probe.commits != (finish == 0u ? 1u : 0u) || probe.cancels != (finish != 0u ? 1u : 0u)) {
        lib_c_fprintf(lib_c_stderr, "Core system power: cycle=%u finish=%u polls=%u commit=%u cancel=%u\n",
            (lib_u32)cycle, (lib_u32)finish, probe.polls, probe.commits, probe.cancels);
        return LIB_FALSE;
    }
    return LIB_TRUE;
}

lib_i32 main(void)
{
    for (x86_core_cpu_special_cycle cycle = X86_CORE_CPU_SPECIAL_HALT;
        cycle <= X86_CORE_CPU_SPECIAL_STOP_GRANT; ++cycle)
    for (lib_u8 finish = 0u; finish < 5u; ++finish)
        if (!power_run(cycle, finish)) return 1;
    lib_c_printf("CORE-SYSTEM-POWER:OK\n");
    return 0;
}
