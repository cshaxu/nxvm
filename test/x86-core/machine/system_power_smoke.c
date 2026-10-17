#include "lib/types/file.h"
#include "x86-core/machine/machine.h"
#include "x86-core/machine/attachment_interface.h"
#include "x86-core/machine/debug_interface.h"

typedef struct power_probe {
    x86_core_machine *machine;
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
    const x86_core_machine_transaction_state *transaction = &probe->machine->transaction;

    if (probe->finalized || cycle != probe->cycle ||
        transaction->owner != X86_CORE_MACHINE_TRANSACTION_OWNER_CPU ||
        transaction->kind != X86_CORE_MACHINE_TRANSACTION_CPU_SPECIAL_CYCLE ||
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
        probe->machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_CPU ||
        probe->machine->transaction.kind != X86_CORE_MACHINE_TRANSACTION_CPU_SPECIAL_CYCLE)
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
        probe->machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE)
        probe->bad_order = LIB_TRUE;
    probe->finalized = LIB_TRUE;
}

static lib_bool power_run(x86_core_cpu_special_cycle cycle, lib_u8 finish)
{
    const x86_core_machine_executor_config config = {
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
    const x86_core_machine_attachment attachment = {.context = &probe,
        .special_cycle = power_cycle, .complete_special_cycle = power_complete,
        .after_internal_advance = power_ticks, .finalize_devices = power_finalize};
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_run_result result = {0};
    x86_core_cpu_debug_cpu_snapshot before = {0}, after = {0};
    lib_u64 ticks = 0u;
    lib_bool failed = x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

    probe.machine = machine;
    if (!failed) {
        x86_core_machine_attachment incomplete = attachment;

        incomplete.complete_special_cycle = LIB_NULL;
        failed = x86_core_machine_bind_attachment(machine, &incomplete) != LIB_STATUS_INVALID_ARGUMENT ||
            x86_core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK ||
            x86_core_machine_bind_execution_provider(machine, LIB_NULL, LIB_NULL) != LIB_STATUS_OK ||
            x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            x86_core_machine_reset(machine) != LIB_STATUS_OK;
    }
    if (!failed) {
        /* The public constructor remains closed to 486; this is internal
         * qualification of the real CPU/Core connection. */
        machine->cpu_profile = X86_CORE_CPU_PROFILE_80486SX;
        x86_core_cpu_execution_context_bind_profiles(machine->executor_cpu_execution,
            X86_CORE_CPU_PROFILE_80486SX, X86_FPU_PROFILE_NONE, LIB_FALSE, &timing);
        failed = x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x100u, code, sizeof(code)) != LIB_STATUS_OK;
        if (cycle == X86_CORE_CPU_SPECIAL_STOP_GRANT)
            failed = failed || x86_core_machine_set_stop_clock(machine, LIB_TRUE) != LIB_STATUS_OK;
        else if (cycle == X86_CORE_CPU_SPECIAL_SHUTDOWN)
            x86_core_cpu_execution_request_shutdown(machine->executor_cpu_execution);
    }
    if (!failed && cycle == X86_CORE_CPU_SPECIAL_STOP_GRANT) {
        failed = x86_core_machine_transaction_hold_request(&machine->transaction,
                X86_CORE_MACHINE_TRANSACTION_OWNER_DMA, 0u) != LIB_STATUS_OK ||
            x86_core_machine_transaction_hold_acknowledge(&machine->transaction,
                X86_CORE_MACHINE_TRANSACTION_OWNER_DMA) != LIB_STATUS_OK ||
            x86_core_machine_run(machine, (x86_core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
            result.executed || result.ticks || probe.polls ||
            x86_core_cpu_set_clock_running(machine->executor_cpu_execution,
                LIB_FALSE) != LIB_STATUS_INVALID_STATE ||
            machine->transaction.hold_owner != X86_CORE_MACHINE_TRANSACTION_OWNER_DMA;
        x86_core_machine_transaction_hold_release(&machine->transaction, X86_CORE_MACHINE_TRANSACTION_OWNER_DMA);
    }
    if (!failed) {
        failed = x86_core_machine_run(machine, (x86_core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
            result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            result.executed != (cycle == X86_CORE_CPU_SPECIAL_HALT ? 1u : 0u) ||
            !x86_core_cpu_is_system_waiting(machine->executor_cpu_execution) ||
            x86_core_cpu_set_clock_running(machine->executor_cpu_execution,
                LIB_FALSE) != LIB_STATUS_INVALID_STATE || probe.polls != 1u || probe.commits ||
            probe.cancels || probe.bad_order ||
            x86_core_machine_debug_capture_cpu_snapshot(machine, X86_CORE_CPU_SNAPSHOT_CURRENT,
                &before) != LIB_STATUS_OK;
        ticks = machine->elapsed_ticks;
    }
    if (!failed) {
        failed = x86_core_machine_advance_time(machine, 3u) != LIB_STATUS_OK ||
            machine->elapsed_ticks != ticks + 3u || probe.ticks != machine->elapsed_ticks ||
            probe.polls != 1u || probe.commits;
    }
    if (!failed && finish == 0u) {
        probe.ready = LIB_TRUE;
        failed = x86_core_machine_set_cpu_bus_ready(machine, LIB_FALSE) != LIB_STATUS_OK ||
            x86_core_machine_run(machine, (x86_core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
            result.executed || result.ticks || probe.commits || probe.polls != 2u ||
            x86_core_machine_set_cpu_bus_ready(machine, LIB_TRUE) != LIB_STATUS_OK ||
            x86_core_machine_run(machine, (x86_core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
            result.executed || result.ticks != (cycle == X86_CORE_CPU_SPECIAL_HALT ? 0u : 2u) ||
            probe.commits != 1u || probe.cancels ||
            machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE ||
            x86_core_machine_debug_capture_cpu_snapshot(machine, X86_CORE_CPU_SNAPSHOT_CURRENT,
                &after) != LIB_STATUS_OK || lib_memory_compare(&before, &after, sizeof(before));
        if (!failed && cycle == X86_CORE_CPU_SPECIAL_STOP_GRANT) {
            failed = x86_core_machine_set_clock_running(machine, LIB_FALSE) != LIB_STATUS_OK ||
                x86_core_machine_set_stop_clock(machine, LIB_FALSE) != LIB_STATUS_INVALID_STATE ||
                x86_core_machine_set_nmi(machine, LIB_TRUE) != LIB_STATUS_OK ||
                x86_core_machine_set_nmi(machine, LIB_FALSE) != LIB_STATUS_OK ||
                x86_core_machine_set_clock_running(machine, LIB_TRUE) != LIB_STATUS_OK ||
                x86_core_machine_set_flush(machine, LIB_TRUE) != LIB_STATUS_OK ||
                x86_core_machine_set_flush(machine, LIB_FALSE) != LIB_STATUS_OK ||
                x86_core_machine_set_stop_clock(machine, LIB_FALSE) != LIB_STATUS_OK ||
                x86_core_machine_run(machine, (x86_core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
                result.executed != 1u || result.reason != X86_CORE_MACHINE_STOP_BUDGET ||
                result.linear_pc != 0x101u || probe.commits != 1u;
        }
    } else if (!failed && finish < 3u) {
        if (finish == 1u) x86_core_machine_signal_processor_reset(machine);
        else failed = x86_core_machine_signal_soft_processor_reset(machine) != LIB_STATUS_OK;
        failed = failed || x86_core_machine_run(machine, (x86_core_machine_run_budget){1u,0u}, &result) !=
                LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_RESET_REQUESTED ||
            result.executed || probe.commits || probe.cancels != 1u ||
            x86_core_cpu_is_system_waiting(machine->executor_cpu_execution) ||
            machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE;
    } else if (!failed && finish == 4u) {
        probe.fail = LIB_TRUE;
        failed = x86_core_machine_run(machine, (x86_core_machine_run_budget){1u,0u}, &result) !=
                LIB_STATUS_INTERNAL_ERROR || result.reason != X86_CORE_MACHINE_STOP_FAULT ||
            result.executed || result.ticks || probe.commits || probe.cancels != 1u ||
            machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE;
    }
    x86_core_machine_destroy(machine);
    if (failed || !probe.finalized || probe.bad_order ||
        probe.commits != (finish == 0u ? 1u : 0u) || probe.cancels != (finish != 0u ? 1u : 0u)) {
        lib_c_fprintf(lib_c_stderr, "Core system power: cycle=%u finish=%u polls=%u commit=%u cancel=%u\n",
            (lib_u32)cycle, (lib_u32)finish, probe.polls, probe.commits, probe.cancels);
        return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool power_buffer_service(void)
{
    static const lib_u8 waits[] = {0u, 9u, 30u};
    const x86_core_cpu_instruction_timing timing = {.base_ticks = 1u};
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CR0),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x100u, [X86_CORE_CPU_DEBUG_EFLAGS] = 2u,
            [X86_CORE_CPU_DEBUG_CR0] = 0x10u}
    };
    const lib_u8 code[] = {0xa3u, 0u, 2u, 0x90u};

    for (lib_size index = 0u; index < sizeof(waits) / sizeof(waits[0]); ++index)
    for (lib_u8 outcome = 0u; outcome < 3u; ++outcome)
    for (lib_u8 bounded = 0u; bounded < 2u; ++bounded) {
        x86_core_machine_executor_config config = {
            .memory_bytes = 2u * 1024u * 1024u, .cpu_profile = X86_CORE_CPU_PROFILE_80386
        };
        const lib_u64 service = 1u + (waits[index] < 5u ? waits[index] : 5u);
        power_probe probe = {.cycle = X86_CORE_CPU_SPECIAL_STOP_GRANT,
            .ready = outcome == 0u, .fail = outcome == 2u};
        const x86_core_machine_attachment attachment = {.context = &probe,
            .special_cycle = power_cycle, .complete_special_cycle = power_complete,
            .after_internal_advance = power_ticks, .finalize_devices = power_finalize};
        x86_core_machine *machine = LIB_NULL;
        x86_core_machine_run_result result = {0};
        lib_u64 event_ticks = 0u, before_ticks = 0u;
        lib_status status = LIB_STATUS_OK;
        lib_bool failed;

        config.transaction_contract.external_access_wait_windows[0] =
            (x86_core_machine_external_access_wait_window){X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY,
                0x200u, 0x201u, waits[index]};
        failed = x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

        probe.machine = machine;
        if (!failed) {
            failed = x86_core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK ||
                x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
                x86_core_machine_reset(machine) != LIB_STATUS_OK;
        }
        if (!failed) {
            /* Internal SX transport proof; production construction stays closed. */
            machine->cpu_profile = X86_CORE_CPU_PROFILE_80486SX;
            machine->maximum_instruction_ticks = 1u;
            x86_core_cpu_execution_context_bind_profiles(machine->executor_cpu_execution,
                X86_CORE_CPU_PROFILE_80486SX, X86_FPU_PROFILE_NONE, LIB_FALSE, &timing);
            failed = x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
                x86_core_machine_memory_write(machine, 0x100u, code, sizeof(code)) != LIB_STATUS_OK ||
                x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u},
                    &result) != LIB_STATUS_OK || result.executed != 1u ||
                result.ticks != 1u + waits[index] ||
                probe.polls != 0u ||
                x86_core_machine_set_stop_clock(machine, LIB_TRUE) != LIB_STATUS_OK;
            before_ticks = machine->elapsed_ticks;
        }
        for (lib_u8 calls = 0u; !failed && calls < 8u &&
            (calls == 0u || machine->cpu_retirement_wait_pending); ++calls) {
            status = x86_core_machine_run(machine,
                (x86_core_machine_run_budget){1u, bounded ? 1u : 0u}, &result);
            event_ticks += result.ticks;
            failed = result.executed != 0u || result.ticks > (bounded ? 1u : service + 2u) ||
                result.elapsed_ticks != before_ticks + event_ticks ||
                probe.ticks != machine->elapsed_ticks;
        }
        if (!failed) {
            /* Two-clock write: one issue clock is already serviced. The store's
             * 1+wait elapsed clocks first reconcile five fetch plus one issue
             * credit; only excess clocks can service the remaining write. */
            failed = event_ticks != service + (outcome == 0u ? 2u : 0u) ||
                machine->cpu_retirement_wait_pending ||
                probe.polls != 1u || probe.commits != (lib_u32)(outcome == 0u) ||
                probe.cancels != (lib_u32)(outcome == 2u) || probe.bad_order ||
                status != (outcome == 2u ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK) ||
                result.reason != (outcome == 2u ? X86_CORE_MACHINE_STOP_FAULT :
                    X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT);
        }
        if (!failed && outcome == 1u) {
            probe.ready = LIB_TRUE;
            failed = x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u},
                    &result) != LIB_STATUS_OK || result.executed || result.ticks != 2u ||
                probe.polls != 2u || probe.commits != 1u || probe.cancels ||
                machine->elapsed_ticks != before_ticks + service + 2u;
        }
        if (!failed && outcome != 2u)
            failed = x86_core_machine_set_stop_clock(machine, LIB_FALSE) != LIB_STATUS_OK ||
                x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u},
                    &result) != LIB_STATUS_OK || result.executed != 1u || result.ticks != 21u ||
                result.linear_pc != 0x104u ||
                machine->elapsed_ticks != before_ticks + service + 23u ||
                probe.ticks != machine->elapsed_ticks;
        x86_core_machine_destroy(machine);
        if (failed || !probe.finalized || probe.bad_order) {
            lib_c_fprintf(lib_c_stderr, "Core buffered power: wait=%u outcome=%u bounded=%u ticks=%llu/%llu status=%u\n",
                (lib_u32)waits[index], (lib_u32)outcome, (lib_u32)bounded,
                event_ticks, service, (lib_u32)status);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool power_flush_timing(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 65536u, .cpu_profile = X86_CORE_CPU_PROFILE_80386
    };
    const x86_core_cpu_instruction_timing timing = {.base_ticks = 1u};
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x100u}
    };
    const lib_u8 code[] = {0x90u, 0x90u};

    for (lib_u8 bounded = 0u; bounded < 2u; ++bounded) {
        power_probe probe = {0};
        const x86_core_machine_attachment attachment = {.context = &probe,
            .after_internal_advance = power_ticks, .finalize_devices = power_finalize};
        x86_core_machine *machine = LIB_NULL;
        x86_core_machine_run_result result = {0};
        lib_u64 pulse_ticks = 0u, retired = 0u;
        lib_bool failed = x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

        probe.machine = machine;
        if (!failed) {
            failed = x86_core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK ||
                x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
                x86_core_machine_reset(machine) != LIB_STATUS_OK;
        }
        if (!failed) {
            /* Internal qualified FLUSH path, not public SX production admission. */
            machine->cpu_profile = X86_CORE_CPU_PROFILE_80486SX;
            machine->maximum_instruction_ticks = 1u;
            x86_core_cpu_execution_context_bind_profiles(machine->executor_cpu_execution,
                X86_CORE_CPU_PROFILE_80486SX, X86_FPU_PROFILE_NONE, LIB_FALSE, &timing);
            failed = x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
                x86_core_machine_memory_write(machine, 0x100u, code, sizeof(code)) != LIB_STATUS_OK ||
                x86_core_machine_set_flush(machine, LIB_TRUE) != LIB_STATUS_OK;
        }
        for (lib_u8 round = 0u; !failed && round < 2u; ++round)
            failed = x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u},
                    &result) != LIB_STATUS_OK || result.ticks != 1u || result.executed ||
                result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
                result.linear_pc != 0x100u;
        if (!failed)
            failed = x86_core_machine_set_flush(machine, LIB_FALSE) != LIB_STATUS_OK ||
                x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u},
                    &result) != LIB_STATUS_OK || result.executed != 1u || result.ticks != 1u ||
                result.linear_pc != 0x101u ||
                x86_core_machine_set_flush(machine, LIB_TRUE) != LIB_STATUS_OK ||
                x86_core_machine_set_flush(machine, LIB_FALSE) != LIB_STATUS_OK;
        for (lib_u8 calls = 0u; !failed && calls < 2u &&
            (calls == 0u || machine->cpu_retirement_wait_pending); ++calls) {
            failed = x86_core_machine_run(machine,
                (x86_core_machine_run_budget){1u, bounded ? 1u : 0u}, &result) != LIB_STATUS_OK ||
                result.ticks > (bounded ? 1u : 2u) || result.reason != X86_CORE_MACHINE_STOP_BUDGET;
            pulse_ticks += result.ticks;
            retired += result.executed;
        }
        failed = failed || pulse_ticks != 2u || retired != 1u ||
            (machine != LIB_NULL && (machine->elapsed_ticks != 5u ||
                probe.ticks != 5u || machine->cpu_retirement_wait_pending));
        x86_core_machine_destroy(machine);
        if (failed || !probe.finalized || probe.bad_order) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool power_shutdown_reset(void *context)
{
    (void)context;
    return LIB_TRUE;
}

static lib_bool power_halted_shutdown_reset(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 65536u, .cpu_profile = X86_CORE_CPU_PROFILE_80386
    };
    power_probe probe = {0};
    const x86_core_machine_attachment attachment = {
        .context = &probe, .shutdown_reset = power_shutdown_reset,
        .finalize_devices = power_finalize
    };
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP)
    };
    const lib_u8 halt = 0xf4u;
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_run_result result = {0};
    x86_core_cpu_state cpu = {0};
    lib_u64 ticks = 0u;
    lib_bool failed = x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

    probe.machine = machine;
    if (!failed) {
        failed = x86_core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK ||
            x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            x86_core_machine_reset(machine) != LIB_STATUS_OK ||
            x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0u, &halt, 1u) != LIB_STATUS_OK ||
            x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u},
                &result) != LIB_STATUS_OK ||
            result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            !x86_core_cpu_is_halted(machine->executor_cpu_execution);
    }
    if (!failed) {
        ticks = machine->elapsed_ticks;
        x86_core_cpu_execution_request_shutdown(machine->executor_cpu_execution);
        failed = x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u},
                &result) != LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_RESET_REQUESTED ||
            result.executed != 0u || machine->elapsed_ticks != ticks ||
            x86_core_machine_get_cpu_state(machine, &cpu) != LIB_STATUS_OK ||
            cpu.eip != 0xfff0u || cpu.halted;
    }
    x86_core_machine_destroy(machine);
    return !failed && probe.finalized && !probe.bad_order;
}

lib_i32 main(void)
{
    if (!power_buffer_service()) return 1;
    if (!power_flush_timing()) return 1;
    if (!power_halted_shutdown_reset()) return 1;
    for (x86_core_cpu_special_cycle cycle = X86_CORE_CPU_SPECIAL_HALT;
        cycle <= X86_CORE_CPU_SPECIAL_STOP_GRANT; ++cycle)
    for (lib_u8 finish = 0u; finish < 5u; ++finish)
        if (!power_run(cycle, finish)) return 1;
    lib_c_printf("CORE-SYSTEM-POWER:OK\n");
    return 0;
}
