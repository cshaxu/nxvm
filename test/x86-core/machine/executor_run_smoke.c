#include "lib/types/types_interface.h"
#include "lib/types/file.h"



#include "x86-core/machine/machine_interface.h"
#include "x86-core/machine/machine.h"
#include "x86-core/machine/debug_interface.h"
#include "x86-core/machine/attachment_interface.h"
#include "support/memory_alias_fixture.h"

static lib_bool executor_sx_fpu_interface(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = X86_CORE_CPU_PROFILE_80486SX,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const lib_u16 vector[] = {0x0100u,0u};
    const lib_u8 hlt = 0xf4u;

    for (lib_u8 controls = 0u; controls < 16u; ++controls)
    for (lib_u8 operation = 0u; operation < 9u; ++operation) {
        x86_core_machine *machine = LIB_NULL;
        x86_core_machine_run_result result;
        x86_core_cpu_diagnostic diagnostic;
        x86_core_cpu_debug_cpu_snapshot before, after;
        const lib_u32 cr0 = ((controls & 1u) ? VCPU_CR0_EM : 0u) |
            ((controls & 2u) ? VCPU_CR0_TS : 0u) |
            ((controls & 4u) ? VCPU_CR0_MP : 0u) |
            ((controls & 8u) ? VCPU_CR0_NE : 0u);
        const lib_bool nm = operation < 8u || (controls & 6u) == 6u;
        const lib_u8 code[] = {operation < 8u ? (lib_u8)(0xd8u + operation) : 0x9bu,
            operation < 8u ? 0xc0u : 0x90u};
        const x86_core_cpu_debug_register_patch entry = {
            .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
                X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS) |
                X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ES) |
                X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
                X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
                X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESP) |
                X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS) |
                X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CR0),
            .values = {[X86_CORE_CPU_DEBUG_ESP] = 0x8000u,
                [X86_CORE_CPU_DEBUG_EFLAGS] = 2u, [X86_CORE_CPU_DEBUG_CR0] = cr0}
        };
        lib_u16 frame[3];
        lib_bool failed = x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

        if (!failed) {
            failed = x86_core_machine_bind_execution_provider(machine, LIB_NULL, LIB_NULL) != LIB_STATUS_OK ||
                x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
                x86_core_machine_reset(machine) != LIB_STATUS_OK;
        }
        if (!failed) {
            failed = x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
                x86_core_machine_memory_write(machine, 0u, code, sizeof(code)) != LIB_STATUS_OK ||
                x86_core_machine_memory_write(machine, 0x1cu, vector, sizeof(vector)) != LIB_STATUS_OK ||
                x86_core_machine_memory_write(machine, vector[0], &hlt, sizeof(hlt)) != LIB_STATUS_OK ||
                x86_core_machine_debug_capture_cpu_snapshot(machine, X86_CORE_CPU_SNAPSHOT_CURRENT,
                    &before) != LIB_STATUS_OK ||
                x86_core_machine_run(machine, (x86_core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
                result.reason != X86_CORE_MACHINE_STOP_BUDGET ||
                x86_core_machine_get_cpu_diagnostic(machine, &diagnostic) != LIB_STATUS_OK ||
                x86_core_machine_debug_capture_cpu_snapshot(machine, X86_CORE_CPU_SNAPSHOT_CURRENT,
                    &after) != LIB_STATUS_OK;
        }
        if (!failed) {
            failed = diagnostic.first_fault.valid || diagnostic.last_delivered_exception.valid != nm ||
                after.eip != (nm ? vector[0] : 1u) || after.esp != (nm ? 0x7ffau : 0x8000u) ||
                after.eax != before.eax || after.ebx != before.ebx || after.ecx != before.ecx ||
                after.edx != before.edx || after.ebp != before.ebp || after.esi != before.esi ||
                after.edi != before.edi || after.eflags != before.eflags;
            if (!failed && nm) {
                failed = diagnostic.last_delivered_exception.exception_mask != VCPUINS_EXCEPT_NM ||
                    x86_core_machine_memory_read(machine, after.ss.base + (lib_u16)after.esp,
                        frame, sizeof(frame)) != LIB_STATUS_OK ||
                    frame[0] != 0u || frame[1] != before.cs.selector || frame[2] != before.eflags;
            }
        }
        if (!failed) {
            failed = x86_core_machine_run(machine, (x86_core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
                result.reason != (nm ? X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT : X86_CORE_MACHINE_STOP_BUDGET) ||
                x86_core_machine_debug_capture_cpu_snapshot(machine, X86_CORE_CPU_SNAPSHOT_CURRENT,
                    &after) != LIB_STATUS_OK || after.eip != (nm ? vector[0] + 1u : 2u);
        }
        x86_core_machine_destroy(machine);
        if (failed) {
            lib_c_fprintf(lib_c_stderr, "Core SX FPU delivery: controls=%u operation=%u\n",
                (lib_u32)controls, (lib_u32)operation);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool executor_event_time(void)
{
    x86_core_machine_executor_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS),
        .values = {[X86_CORE_CPU_DEBUG_ESP] = 0x8000u, [X86_CORE_CPU_DEBUG_EFLAGS] = 2u}
    };
    const lib_u16 vector[] = {0x100u, 0u};
    const lib_u8 nop = 0x90u;

    for (x86_core_cpu_profile profile = X86_CORE_CPU_PROFILE_80386;
         profile <= X86_CORE_CPU_PROFILE_80486SX; ++profile)
    for (lib_u8 route = 0u; route < 3u; ++route) {
        x86_core_machine *machine = LIB_NULL;
        x86_core_machine_run_result result = {0};
        const lib_bool nmi = route == 1u;
        const lib_bool overflow = route == 2u;
        const lib_u8 code[] = {nmi ? 0x90u : 0x0fu, 0x0bu};
        /* Real SX reset disables caching: cold NOP 5 + NMI 31,
         * or the original uncached fault entry 33. */
        const lib_u64 clocks = profile == X86_CORE_CPU_PROFILE_80486SX ?
            (nmi ? 36u : 33u) : nmi ? 3u : 0u;
        const lib_u64 before = overflow ? LIB_UINT64_MAX - 25u : 0u;
        lib_status status = LIB_STATUS_INTERNAL_ERROR;
        lib_bool failed;

        if (overflow && profile != X86_CORE_CPU_PROFILE_80486SX) continue;
        config.cpu_profile = profile;
        failed = x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
        if (!failed) {
            failed = x86_core_machine_bind_execution_provider(machine, LIB_NULL, LIB_NULL) != LIB_STATUS_OK ||
                x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
                x86_core_machine_reset(machine) != LIB_STATUS_OK ||
                x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
                x86_core_machine_memory_write(machine, 0u, code, sizeof(code)) != LIB_STATUS_OK ||
                x86_core_machine_memory_write(machine, nmi ? 8u : 24u, vector, sizeof(vector)) != LIB_STATUS_OK ||
                x86_core_machine_memory_write(machine, vector[0], &nop, sizeof(nop)) != LIB_STATUS_OK;
        }
        if (!failed) {
            machine->elapsed_ticks = before;
            failed = nmi && !x86_core_cpu_request_nmi(machine->executor_cpu_execution);
            status = x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u}, &result);
            failed = failed || status != (overflow ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK) ||
                result.executed != (nmi ? 1u : 0u) ||
                result.reason != (overflow ? X86_CORE_MACHINE_STOP_FAULT : X86_CORE_MACHINE_STOP_BUDGET) ||
                result.ticks != (overflow ? 0u : clocks) ||
                result.elapsed_ticks != (overflow ? before : clocks) ||
                machine->elapsed_ticks != result.elapsed_ticks;
        }
        if (!failed && !overflow) {
            failed = x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u}, &result) != LIB_STATUS_OK ||
                result.executed != 1u || result.ticks != (profile == X86_CORE_CPU_PROFILE_80486SX ? 5u : 3u) ||
                result.elapsed_ticks != clocks + result.ticks;
        }
        if (failed)
            lib_c_printf("Executor event: profile=%u route=%u status=%u count=%llu ticks=%llu elapsed=%llu source=%llu event=%llu\n",
                (lib_u32)profile, route, (lib_u32)status, result.executed, result.ticks, result.elapsed_ticks,
                x86_core_cpu_capture_timing(machine->executor_cpu_execution).ticks,
                x86_core_cpu_capture_timing(machine->executor_cpu_execution).event_ticks);
        x86_core_machine_destroy(machine);
        if (failed) return LIB_FALSE;
    }
    return LIB_TRUE;
}

typedef struct executor_entry_fault_probe {
    lib_u32 fetches, rejected, finalized;
} executor_entry_fault_probe;

static void executor_entry_fault_finalize(void *opaque)
{
    ++((executor_entry_fault_probe *)opaque)->finalized;
}

static lib_status executor_entry_fault_memory(void *opaque, lib_u32 address,
    lib_u8 bytes, lib_bool write, lib_u8 cache_control)
{
    executor_entry_fault_probe *probe = opaque;
    (void)bytes;
    (void)cache_control;
    if (!write && address == 8u) {
        ++probe->rejected;
        return LIB_STATUS_IO_ERROR;
    }
    if (!write && address >= 0x100u && address <= 0x10fu) ++probe->fetches;
    return LIB_STATUS_OK;
}

static lib_bool executor_sx_bounded_retirement(void)
{
    const lib_u8 code[] = {0x90u, 0x90u};
    const x86_core_machine_retirement_eligibility_key key = {
        .cpu_profile = X86_CORE_CPU_PROFILE_80486SX,
        .timing_origin = X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_80486_MODEL,
        .source_timing_form_id = 0x90u, .opcode = 0x90u,
        .escape_opcode = X86_CORE_MACHINE_RETIREMENT_CONTEXT_UNAVAILABLE,
        .modrm_extension = X86_CORE_MACHINE_RETIREMENT_CONTEXT_UNAVAILABLE,
        .next_lexeme_components = X86_CORE_MACHINE_RETIREMENT_CONTEXT_UNAVAILABLE
    };
    const x86_core_machine_retirement_qualification_descriptor qualification = {&key, 1u};
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CR0),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x100u,
            [X86_CORE_CPU_DEBUG_ESP] = 0x8000u, [X86_CORE_CPU_DEBUG_EFLAGS] = 2u,
            [X86_CORE_CPU_DEBUG_CR0] = 0x60000010u}
    };

    for (lib_u8 wait = 0u; wait <= 6u; wait += 3u)
    for (lib_u8 quantum = 1u; quantum <= 3u; ++quantum)
    for (lib_u8 mode = 0u; mode < 7u; ++mode) {
        x86_core_machine_executor_config config = {.memory_bytes = 2u * 1024u * 1024u,
            .cpu_profile = X86_CORE_CPU_PROFILE_80486SX, .fpu_profile = X86_FPU_PROFILE_NONE};
        x86_core_machine *machine = LIB_NULL;
        executor_entry_fault_probe probe = {0};
        const x86_core_machine_attachment attachment = {.context = &probe,
            .finalize_devices = executor_entry_fault_finalize,
            .memory_cache_control = executor_entry_fault_memory};
        x86_core_machine_run_result result = {0};
        const lib_u64 expected = 5u + wait;
        const lib_u64 initial = mode == 3u ? LIB_UINT64_MAX - expected + 1u : 0u;
        lib_u64 elapsed = 0u, retired = 0u;
        lib_status status = LIB_STATUS_OK;
        lib_bool failed;

        if (mode == 1u || mode == 2u) {
            config.time_axis = (x86_core_machine_time_axis){
                X86_CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL, 25000000u};
            config.retirement_time_contract = X86_CORE_MACHINE_RETIREMENT_TIME_PHYSICAL;
            if (mode == 1u) config.retirement_qualification = &qualification;
        }
        config.transaction_contract.external_access_wait_windows[0] =
            (x86_core_machine_external_access_wait_window){X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY,
                0x100u, 0x10fu, wait};
        failed = x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
        if (!failed) failed = x86_core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK ||
            x86_core_machine_bind_execution_provider(machine, LIB_NULL, LIB_NULL) != LIB_STATUS_OK ||
            x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            x86_core_machine_reset(machine) != LIB_STATUS_OK;
        if (!failed) {
            failed = machine->maximum_instruction_ticks != 0u ||
                x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
                x86_core_machine_memory_write(machine, 0x100u, code, sizeof(code)) != LIB_STATUS_OK;
            machine->elapsed_ticks = initial;
        }
        for (lib_u32 calls = 0u; !failed && retired == 0u && status == LIB_STATUS_OK &&
                calls < 32u; ++calls) {
            status = x86_core_machine_run(machine,
                (x86_core_machine_run_budget){1u, quantum}, &result);
            elapsed += result.ticks;
            retired += result.executed;
            failed = result.ticks > quantum || result.elapsed_ticks != initial + elapsed ||
                probe.fetches != 1u || probe.rejected != 0u;
            if (mode == 2u || mode == 3u) {
                failed = failed || status != LIB_STATUS_INTERNAL_ERROR || elapsed != 0u ||
                    retired != 0u || machine->lifecycle != X86_CORE_MACHINE_FAULTED;
                break;
            }
            failed = failed || status != LIB_STATUS_OK || result.ticks == 0u ||
                result.reason != X86_CORE_MACHINE_STOP_BUDGET || elapsed > expected ||
                (elapsed < expected && retired != 0u);
            if (mode == 6u && retired == 0u) {
                lib_u8 output = 0xa5u;
                x86_core_cpu_debug_cpu_snapshot snapshot;

                failed = failed || x86_core_machine_debug_capture_cpu_snapshot(machine,
                        X86_CORE_CPU_SNAPSHOT_CURRENT, &snapshot) != LIB_STATUS_OK ||
                    snapshot.eip != 0x101u ||
                    x86_core_machine_debug_patch_registers(machine, &entry) !=
                        LIB_STATUS_INVALID_STATE ||
                    x86_core_machine_debug_read_linear(machine, 0x100u, &output, 1u) !=
                        LIB_STATUS_INVALID_STATE || output != 0xa5u ||
                    x86_core_machine_debug_write_linear(machine, 0x100u, code, 1u) !=
                        LIB_STATUS_INVALID_STATE ||
                    x86_core_machine_debug_set_watchpoint(machine,
                        X86_CORE_CPU_DEBUG_WATCH_EXECUTE, 0x101u) != LIB_STATUS_INVALID_STATE;
            }
            if (mode == 4u || mode == 5u) {
                failed = failed || retired != 0u || !machine->cpu_retirement_wait_pending ||
                    (mode == 4u ? x86_core_machine_reset(machine) :
                        x86_core_machine_request_stop(machine)) != LIB_STATUS_OK;
                if (!failed && mode == 5u)
                    failed = x86_core_machine_run(machine, (x86_core_machine_run_budget){1u,1u},
                            &result) != LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_REQUESTED ||
                        result.ticks != 0u || result.executed != 0u;
                failed = failed || machine->cpu_retirement_wait_pending || machine->elapsed_ticks != 0u;
                break;
            }
        }
        if (mode < 2u || mode == 6u) failed = failed || elapsed != expected || retired != 1u ||
            machine->cpu_retirement_wait_pending || x86_core_machine_linear_pc(machine) != 0x101u;
        if (!failed && mode == 6u) {
            lib_u8 output = 0u;

            failed = x86_core_machine_debug_read_linear(machine, 0x100u, &output, 1u) !=
                LIB_STATUS_OK || output != code[0];
        }
        if (failed) lib_c_fprintf(lib_c_stderr,
            "Core SX bounded: wait=%u quantum=%u mode=%u status=%u ticks=%llu retired=%llu fetches=%u\n",
            (lib_u32)wait, (lib_u32)quantum, (lib_u32)mode, (lib_u32)status,
            elapsed, retired, probe.fetches);
        x86_core_machine_destroy(machine);
        if (failed || probe.finalized != 1u) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool executor_retired_before_entry_fault(void)
{
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CR0),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x100u, [X86_CORE_CPU_DEBUG_ESP] = 0x8000u,
            [X86_CORE_CPU_DEBUG_EFLAGS] = 2u, [X86_CORE_CPU_DEBUG_CR0] = 0x60000010u}
    };
    const lib_u8 code[] = {0x90u, 0x90u};

    for (lib_u8 wait = 0u; wait <= 3u; wait += 3u)
    for (lib_u8 bounded = 0u; bounded < 2u; ++bounded) {
        x86_core_machine_executor_config config = {.memory_bytes = 2u * 1024u * 1024u,
            .cpu_profile = X86_CORE_CPU_PROFILE_80486SX, .fpu_profile = X86_FPU_PROFILE_NONE};
        x86_core_machine *machine = LIB_NULL;
        executor_entry_fault_probe probe = {0};
        const x86_core_machine_attachment attachment = {.context = &probe,
            .finalize_devices = executor_entry_fault_finalize,
            .memory_cache_control = executor_entry_fault_memory};
        x86_core_machine_run_result result = {0};
        x86_core_cpu_diagnostic diagnostic;
        lib_u64 ticks = 0u, retired = 0u;
        lib_status status = LIB_STATUS_OK;
        lib_bool failed;

        config.transaction_contract.external_access_wait_windows[0] =
            (x86_core_machine_external_access_wait_window){X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY,
                0x100u, 0x10fu, wait};
        failed = x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
        if (!failed) {
            failed = x86_core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK ||
                x86_core_machine_bind_execution_provider(machine, LIB_NULL, LIB_NULL) != LIB_STATUS_OK ||
                x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
                x86_core_machine_reset(machine) != LIB_STATUS_OK;
        }
        if (!failed) {
            failed = x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
                x86_core_machine_memory_write(machine, 0x100u, code, sizeof(code)) != LIB_STATUS_OK ||
                !x86_core_cpu_request_nmi(machine->executor_cpu_execution);
        }
        for (lib_u32 calls = 0u; !failed && status == LIB_STATUS_OK && calls < 128u; ++calls) {
            status = x86_core_machine_run(machine,
                (x86_core_machine_run_budget){1u, bounded ? 1u : 0u}, &result);
            ticks += result.ticks;
            retired += result.executed;
            failed = result.elapsed_ticks != ticks || result.ticks > (bounded ? 1u : LIB_UINT64_MAX) ||
                (status == LIB_STATUS_OK && (result.reason != X86_CORE_MACHINE_STOP_BUDGET ||
                    machine->lifecycle != X86_CORE_MACHINE_PAUSED));
        }
        if (!failed) {
            /* Cold NOP (5), accepted entry writes (6), failed-entry setup (3).
             * The one original fetch owns its accepted wait; entry does not
             * speculatively fetch the interrupted next instruction. */
            const lib_u64 expected = 14u + wait;
            failed = status != LIB_STATUS_INTERNAL_ERROR || result.reason != X86_CORE_MACHINE_STOP_FAULT ||
                retired != 1u || ticks != expected || probe.fetches != 1u || probe.rejected != 1u ||
                machine->lifecycle != X86_CORE_MACHINE_FAULTED ||
                x86_core_machine_get_cpu_diagnostic(machine, &diagnostic) != LIB_STATUS_OK ||
                !diagnostic.first_fault.valid || diagnostic.first_fault.point.eip != 0x101u ||
                diagnostic.first_fault.exception_mask != VCPUINS_EXCEPT_CE ||
                diagnostic.first_fault.exception_code != 8u ||
                x86_core_machine_run(machine, (x86_core_machine_run_budget){1u,1u}, &result) !=
                    LIB_STATUS_INTERNAL_ERROR || result.executed != 0u || result.ticks != 0u ||
                result.elapsed_ticks != ticks || probe.rejected != 1u;
        }
        if (failed) lib_c_printf("Retired before entry fault: wait=%u bounded=%u status=%u ticks=%llu retired=%llu fetches=%u rejected=%u\n",
            (lib_u32)wait, (lib_u32)bounded, (lib_u32)status, ticks, retired,
            probe.fetches, probe.rejected);
        x86_core_machine_destroy(machine);
        if (failed || probe.finalized != 1u) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static void executor_reset_event(void *context, lib_u64 due_tick)
{
    *(lib_u64 *)context = due_tick;
}

static lib_bool executor_smm_return_at(x86_core_machine *machine, lib_u32 base)
{
    const lib_u8 rsm[] = {0x0fu, 0xaau};
    x86_core_machine_run_result result;

    return x86_core_machine_memory_write(machine, base + 0x8000u, rsm,
            sizeof(rsm)) == LIB_STATUS_OK &&
        x86_core_machine_set_smi(machine, LIB_FALSE) == LIB_STATUS_OK &&
        x86_core_machine_set_smi(machine, LIB_TRUE) == LIB_STATUS_OK &&
        x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u},
            &result) == LIB_STATUS_OK && result.executed == 0u &&
        result.linear_pc == base + 0x8000u &&
        x86_core_machine_set_smi(machine, LIB_FALSE) == LIB_STATUS_OK &&
        x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u},
            &result) == LIB_STATUS_OK && result.executed == 1u;
}

static lib_bool executor_sx_soft_reset(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = X86_CORE_CPU_PROFILE_80486SX,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const lib_u8 marker = 0x5au;
    /* Relocate through the architectural SMM save area, not CPU-private state. */
    const lib_u8 relocate[] = {
        0x26u, 0x66u, 0x67u, 0xc7u, 0x05u, 0xf8u, 0xfeu, 0x03u, 0u,
        0u, 0u, 0x18u, 0u, 0x0fu, 0xaau
    };
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_run_result result;
    lib_u8 observed = 0u;
    lib_u64 event_tick = 0u;
    lib_u64 due_tick;
    x86_core_machine_timeline_token token;
    lib_u64 ticks;
    lib_bool failed = x86_core_machine_signal_soft_processor_reset(LIB_NULL) !=
        LIB_STATUS_INVALID_ARGUMENT || x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

    if (!failed) {
        failed = x86_core_machine_bind_execution_provider(machine, LIB_NULL, LIB_NULL) != LIB_STATUS_OK ||
            x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            x86_core_machine_reset(machine) != LIB_STATUS_OK;
    }
    if (!failed) {
        failed = x86_core_machine_debug_write_register(machine, X86_CORE_CPU_DEBUG_CR0,
                VCPU_CR0_ET) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x38000u, relocate,
                sizeof(relocate)) != LIB_STATUS_OK ||
            x86_core_machine_set_smi(machine, LIB_TRUE) != LIB_STATUS_OK ||
            x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u},
                &result) != LIB_STATUS_OK || result.executed != 0u ||
            result.linear_pc != 0x38000u ||
            x86_core_machine_set_smi(machine, LIB_FALSE) != LIB_STATUS_OK ||
            x86_core_machine_run(machine, (x86_core_machine_run_budget){2u, 0u},
                &result) != LIB_STATUS_OK || result.executed != 2u ||
            !executor_smm_return_at(machine, 0x180000u);
    }
    if (!failed) {
        ticks = machine->elapsed_ticks;
        failed = x86_core_machine_memory_write(machine, 0x100u, &marker, sizeof(marker)) != LIB_STATUS_OK ||
            x86_core_machine_timeline_schedule(&machine->timeline, ticks + 10u,
                executor_reset_event, &event_tick, &token) != LIB_STATUS_OK ||
            x86_core_machine_signal_soft_processor_reset(machine) != LIB_STATUS_OK ||
            x86_core_machine_timeline_pending_count(&machine->timeline) != 1u ||
            x86_core_machine_timeline_next_due(&machine->timeline, &due_tick) != LIB_STATUS_OK ||
            due_tick != ticks + 10u || event_tick != 0u ||
            x86_core_machine_run(machine, (x86_core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
            result.reason != X86_CORE_MACHINE_STOP_RESET_REQUESTED || result.executed != 0u ||
            result.ticks != 0u || result.linear_pc != 0xfffffff0u ||
            machine->elapsed_ticks != ticks || machine->lifecycle != X86_CORE_MACHINE_STOPPED ||
            x86_core_machine_memory_read(machine, 0x100u, &observed, sizeof(observed)) != LIB_STATUS_OK ||
            observed != marker || !executor_smm_return_at(machine, 0x180000u);
    }
    if (!failed) {
        /* SMI entry now advances guest time: the earlier deadline fires
         * during the round trip. Re-arm one to prove hard-reset preservation. */
        failed = event_tick != ticks + 10u ||
            x86_core_machine_timeline_pending_count(&machine->timeline) != 0u;
        ticks = machine->elapsed_ticks;
        event_tick = 0u;
        failed = failed || x86_core_machine_timeline_schedule(&machine->timeline, ticks + 10u,
            executor_reset_event, &event_tick, &token) != LIB_STATUS_OK;
    }
    if (!failed) {
        x86_core_machine_signal_processor_reset(machine);
        failed = x86_core_machine_signal_soft_processor_reset(machine) != LIB_STATUS_OK ||
            x86_core_machine_run(machine, (x86_core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
            result.reason != X86_CORE_MACHINE_STOP_RESET_REQUESTED || result.executed != 0u ||
            x86_core_machine_timeline_pending_count(&machine->timeline) != 1u ||
            x86_core_machine_timeline_advance(&machine->timeline, ticks + 10u) != LIB_STATUS_OK ||
            event_tick != ticks + 10u ||
            x86_core_machine_memory_read(machine, 0x100u, &observed, sizeof(observed)) != LIB_STATUS_OK ||
            observed != marker || !executor_smm_return_at(machine, 0x30000u);
    }
    x86_core_machine_destroy(machine);
    return !failed;
}

typedef struct executor_smm_probe {
    lib_u32 assertions;
    lib_u32 releases;
    lib_u32 finalized;
    lib_bool active;
    lib_bool bad_order;
    lib_u32 transfers;
    lib_u32 fail_transfer;
    lib_bool fail_restore;
} executor_smm_probe;

static void executor_smm_active(void *context, lib_bool active)
{
    executor_smm_probe *probe = context;

    if (probe->active == active || probe->finalized) probe->bad_order = LIB_TRUE;
    probe->active = active;
    if (active) ++probe->assertions;
    else ++probe->releases;
}

static void executor_smm_finalize(void *context)
{
    executor_smm_probe *probe = context;

    if (probe->active) probe->bad_order = LIB_TRUE;
    ++probe->finalized;
}

static lib_status executor_smm_memory_control(void *context, lib_u32 address,
    lib_u8 bytes, lib_bool write, lib_u8 cache_control)
{
    executor_smm_probe *probe = context;
    (void)cache_control;

    if (probe->fail_transfer != 0u && address >= 0x3fe00u && address < 0x40000u &&
        write != probe->fail_restore) {
        ++probe->transfers;
        if (!probe->active || bytes != 4u || address != 0x40000u - probe->transfers * 4u)
            probe->bad_order = LIB_TRUE;
        if (probe->transfers == probe->fail_transfer) return LIB_STATUS_IO_ERROR;
    }
    return LIB_STATUS_OK;
}

static lib_bool executor_sx_smm(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = X86_CORE_CPU_PROFILE_80486SX,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const lib_u8 handler[] = {
        0x26u,0x66u,0x67u,0xc7u,0x05u,0xd0u,0xffu,0x03u,0u,0x78u,0x56u,0x34u,0x12u,
        0x0fu,0xaau
    };
    const lib_u8 nop = 0x90u;
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ES) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CR0),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x100u, [X86_CORE_CPU_DEBUG_ESP] = 0x8000u,
            [X86_CORE_CPU_DEBUG_EFLAGS] = 2u,
            [X86_CORE_CPU_DEBUG_CR0] = VCPU_CR0_ET | VCPU_CR0_CD | VCPU_CR0_NW}
    };
    executor_smm_probe probe = {0};
    const x86_core_machine_attachment attachment = {.context = &probe,
        .finalize_devices = executor_smm_finalize,
        .system_management_active = executor_smm_active};
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_run_result result = {0};
    x86_core_cpu_debug_cpu_snapshot after;
    lib_u8 observed = 0u;
    lib_u64 event_tick = 0u;
    x86_core_machine_timeline_token token;
    lib_bool failed = x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

    if (!failed) {
        failed = x86_core_machine_set_smi(LIB_NULL, LIB_TRUE) != LIB_STATUS_INVALID_ARGUMENT ||
            x86_core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK ||
            x86_core_machine_bind_execution_provider(machine, LIB_NULL, LIB_NULL) != LIB_STATUS_OK ||
            x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            x86_core_machine_reset(machine) != LIB_STATUS_OK;
    }
    if (!failed) {
        failed = x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x38000u, handler, sizeof(handler)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x100u, &nop, sizeof(nop)) != LIB_STATUS_OK ||
            x86_core_machine_timeline_schedule(&machine->timeline, 150u,
                executor_reset_event, &event_tick, &token) != LIB_STATUS_OK ||
            x86_core_machine_set_smi(machine, LIB_TRUE) != LIB_STATUS_OK ||
            x86_core_machine_run(machine, (x86_core_machine_run_budget){1u,100u}, &result) != LIB_STATUS_OK ||
            result.reason != X86_CORE_MACHINE_STOP_BUDGET || result.executed != 0u || result.ticks != 100u ||
            !machine->cpu_retirement_wait_pending || machine->cpu_retirement_completion_ticks != 198u ||
            event_tick != 0u || result.linear_pc != 0x38000u || !probe.active || probe.assertions != 1u ||
            x86_core_machine_memory_read(machine, 0x100u, &observed, sizeof(observed)) != LIB_STATUS_OK ||
            observed != nop;
    }
    for (lib_u8 slice = 0u; !failed && slice < 2u; ++slice) {
        const lib_u64 ticks = slice == 0u ? 100u : 98u;

        failed = x86_core_machine_run(machine, (x86_core_machine_run_budget){1u,ticks}, &result) != LIB_STATUS_OK ||
            result.executed != 0u || result.reason != X86_CORE_MACHINE_STOP_BUDGET || result.ticks != ticks ||
            result.elapsed_ticks != (slice == 0u ? 200u : 298u) || event_tick != 150u ||
            machine->cpu_retirement_wait_pending != (slice == 0u) ||
            machine->cpu_retirement_completion_ticks != (slice == 0u ? 98u : 0u) ||
            probe.assertions != 1u || result.linear_pc != 0x38000u;
    }
    for (lib_u8 instruction = 0u; !failed && instruction < 2u; ++instruction) {
        failed = x86_core_machine_run(machine, (x86_core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
            result.executed != 1u || result.reason != X86_CORE_MACHINE_STOP_BUDGET;
    }
    if (!failed) {
        failed = x86_core_machine_debug_capture_cpu_snapshot(machine, X86_CORE_CPU_SNAPSHOT_CURRENT,
            &after) != LIB_STATUS_OK || after.eip != 0x100u || after.cs.base != 0u ||
            after.eax != 0x12345678u || probe.active || probe.releases != 1u ||
            x86_core_machine_set_smi(machine, LIB_FALSE) != LIB_STATUS_OK ||
            x86_core_machine_set_smi(machine, LIB_TRUE) != LIB_STATUS_OK ||
            x86_core_machine_run(machine, (x86_core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
            result.executed != 0u || !probe.active;
    }
    if (failed && machine != LIB_NULL)
        lib_c_fprintf(lib_c_stderr,
            "Core SMM time: count=%llu ticks=%llu elapsed=%llu pc=%08x pending=%u "
            "remaining=%llu deadline=%llu active=%u assertions=%u\n",
            result.executed, result.ticks, result.elapsed_ticks, result.linear_pc,
            (lib_u32)machine->cpu_retirement_wait_pending, machine->cpu_retirement_completion_ticks,
            event_tick, (lib_u32)probe.active, probe.assertions);
    x86_core_machine_destroy(machine);
    return !failed && probe.finalized == 1u && probe.assertions == 2u &&
        probe.releases == 2u && !probe.bad_order;
}

static lib_bool executor_sx_smm_fault(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = X86_CORE_CPU_PROFILE_80486SX,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const lib_u8 rsm[] = {0x0fu,0xaau};
    const lib_u32 positions[] = {2u,127u};
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CR0),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x100u,
            [X86_CORE_CPU_DEBUG_CR0] = VCPU_CR0_ET | VCPU_CR0_CD | VCPU_CR0_NW}
    };

    for (lib_u8 bounded = 0u; bounded < 3u; ++bounded)
    for (lib_u8 restore = 0u; restore < 2u; ++restore)
    for (lib_size index = 0u; index < sizeof(positions) / sizeof(positions[0]); ++index) {
        executor_smm_probe probe = {.fail_restore = restore != 0u};
        const x86_core_machine_attachment attachment = {.context = &probe,
            .finalize_devices = executor_smm_finalize,
            .system_management_active = executor_smm_active,
            .memory_cache_control = executor_smm_memory_control};
        x86_core_machine *machine = LIB_NULL;
        x86_core_machine_run_result result = {0};
        x86_core_cpu_diagnostic diagnostic = {0};
        x86_core_cpu_debug_cpu_snapshot before = {0}, after = {0};
        lib_status run_status = LIB_STATUS_OK;
        lib_u8 image[512], observed[4];
        lib_u64 cancelled = 0u;
        lib_u64 before_ticks = 0u;
        lib_u64 event_tick = 0u;
        x86_core_machine_timeline_token token;
        const lib_u64 prefix_ticks = (restore ? 5u : 42u) + 2u * (positions[index] - 1u);
        const lib_bool reset_prefix = bounded == 2u;
        lib_bool failed = x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

        lib_memory_set(image, 0xa5u, sizeof(image));
        if (!failed) {
            failed = x86_core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK ||
                x86_core_machine_bind_execution_provider(machine, LIB_NULL, LIB_NULL) != LIB_STATUS_OK ||
                x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
                x86_core_machine_reset(machine) != LIB_STATUS_OK;
        }
        if (!failed) {
            failed = x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
                x86_core_machine_memory_write(machine, 0x38000u, rsm, sizeof(rsm)) != LIB_STATUS_OK ||
                x86_core_machine_memory_write(machine, 0x3fe00u, image, sizeof(image)) != LIB_STATUS_OK ||
                x86_core_machine_set_smi(machine, LIB_TRUE) != LIB_STATUS_OK;
        }
        if (!failed && restore != 0u) {
            failed = x86_core_machine_run(machine, (x86_core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
                result.reason != X86_CORE_MACHINE_STOP_BUDGET || result.executed != 0u || !probe.active;
        }
        if (!failed) {
            probe.fail_transfer = positions[index];
            cancelled = machine->transaction.cancelled_count;
            before_ticks = machine->elapsed_ticks;
            failed = x86_core_machine_debug_capture_cpu_snapshot(machine,
                    X86_CORE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK ||
                x86_core_machine_timeline_schedule(&machine->timeline, before_ticks + prefix_ticks / 2u,
                    executor_reset_event, &event_tick, &token) != LIB_STATUS_OK;
            const lib_u64 calls = reset_prefix ? 1u : bounded ? prefix_ticks : 1u;

            for (lib_u64 step = 0u; !failed && step < calls; ++step) {
                const lib_bool final = !reset_prefix && step + 1u == calls;

                run_status = x86_core_machine_run(machine,
                    (x86_core_machine_run_budget){1u,bounded ? 1u : 0u}, &result);
                failed = run_status != (final ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK) ||
                    result.reason != (final ? X86_CORE_MACHINE_STOP_FAULT : X86_CORE_MACHINE_STOP_BUDGET) ||
                    result.executed != 0u || result.ticks != (bounded ? 1u : prefix_ticks) ||
                    result.elapsed_ticks != before_ticks + (bounded ? step + 1u : prefix_ticks) ||
                    machine->lifecycle != (final ? X86_CORE_MACHINE_FAULTED : X86_CORE_MACHINE_PAUSED);
            }
            failed = failed || event_tick != (reset_prefix ? 0u : before_ticks + prefix_ticks / 2u) ||
                probe.transfers != positions[index] || probe.bad_order || !probe.active ||
                x86_core_machine_get_cpu_diagnostic(machine, &diagnostic) != LIB_STATUS_OK ||
                !diagnostic.first_fault.valid ||
                diagnostic.first_fault.exception_mask != VCPUINS_EXCEPT_CE ||
                diagnostic.first_fault.exception_code != 0x40000u - positions[index] * 4u ||
                x86_core_machine_debug_capture_cpu_snapshot(machine,
                    X86_CORE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
                lib_memory_compare(&before, &after, sizeof(before)) ||
                machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE ||
                machine->transaction.cancelled_count != cancelled + 1u;
        }
        if (!failed && reset_prefix) {
            const lib_u8 nop = 0x90u;

            if (restore) failed = x86_core_machine_signal_soft_processor_reset(machine) != LIB_STATUS_OK;
            else x86_core_machine_signal_processor_reset(machine);
            failed = failed || x86_core_machine_run(machine, (x86_core_machine_run_budget){1u,1u},
                    &result) != LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_RESET_REQUESTED ||
                result.executed != 0u || result.ticks != 0u || machine->fault_detail != 0u ||
                machine->cpu_retirement_wait_pending || result.elapsed_ticks != before_ticks + 1u ||
                x86_core_machine_timeline_pending_count(&machine->timeline) != 1u ||
                x86_core_machine_get_cpu_diagnostic(machine, &diagnostic) != LIB_STATUS_OK ||
                !diagnostic.first_fault.valid ||
                x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
                x86_core_machine_memory_write(machine, 0x100u, &nop, sizeof(nop)) != LIB_STATUS_OK ||
                x86_core_machine_set_smi(machine, LIB_FALSE) != LIB_STATUS_OK ||
                x86_core_machine_run(machine, (x86_core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
                result.executed != 1u || machine->fault_detail != 0u ||
                probe.transfers != positions[index] || probe.active;
        }
        if (!failed && !reset_prefix) {
            x86_core_machine_run_result repeated = {0};

            failed = x86_core_machine_run(machine, (x86_core_machine_run_budget){1u,1u},
                    &repeated) != LIB_STATUS_INTERNAL_ERROR || repeated.executed != 0u || repeated.ticks != 0u ||
                repeated.elapsed_ticks != before_ticks + prefix_ticks ||
                probe.transfers != positions[index] || machine->transaction.cancelled_count != cancelled + 1u;
        }
        if (!failed && restore == 0u) {
            const lib_u8 cr0[] = {0x10u,0u,0u,0x60u};

            /* The first save write is committed even though a later cycle
             * fails; the rejected write retains the original RAM bytes. */
            failed = x86_core_machine_memory_inspect(machine, 0x3fffcu, observed, sizeof(observed)) !=
                    LIB_STATUS_OK || lib_memory_compare(observed, cr0, sizeof(cr0)) ||
                x86_core_machine_memory_inspect(machine, 0x40000u - positions[index] * 4u,
                    observed, sizeof(observed)) != LIB_STATUS_OK ||
                lib_memory_compare(observed, image, sizeof(observed));
        }
        if (failed && machine != LIB_NULL) {
            (void)x86_core_machine_get_cpu_diagnostic(machine, &diagnostic);
            lib_c_fprintf(lib_c_stderr,
                "Core SMM fault detail: status=%u reason=%u retired=%llu ticks=%llu expected=%llu active=%u bad=%u "
                "mask=%08x code=%08x cancelled=%llu/%llu\n",
                (lib_u32)run_status, (lib_u32)result.reason,
                (unsigned long long)result.executed, result.ticks, prefix_ticks, (lib_u32)probe.active,
                (lib_u32)probe.bad_order, diagnostic.first_fault.exception_mask,
                diagnostic.first_fault.exception_code,
                (unsigned long long)machine->transaction.cancelled_count,
                (unsigned long long)cancelled);
        }
        x86_core_machine_destroy(machine);
        if (failed || probe.active || probe.assertions != 1u || probe.releases != 1u ||
            probe.finalized != 1u || probe.bad_order) {
            lib_c_fprintf(lib_c_stderr, "Core SMM fault: restore=%u cycle=%u transfers=%u\n",
                (lib_u32)restore, positions[index], probe.transfers);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

typedef struct executor_dma_probe {
    x86_core_machine *machine;
    lib_u64 granted_ticks;
    lib_bool locked;
    lib_bool bad_grant;
} executor_dma_probe;

static lib_u64 executor_dma_ticks(void *context, lib_u64 source_ticks)
{
    (void)context;
    return source_ticks;
}

static lib_bool executor_dma_request(void *context)
{
    (void)context;
    return LIB_TRUE;
}

static void executor_dma_advance(void *context, lib_u64 ticks)
{
    executor_dma_probe *probe = context;
    const x86_core_machine_transaction_state *transaction = &probe->machine->transaction;

    if (probe->locked) {
        if (transaction->owner != X86_CORE_MACHINE_TRANSACTION_OWNER_CPU ||
            transaction->kind != X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_LOCK ||
            transaction->hold_acknowledged) probe->bad_grant = LIB_TRUE;
        return;
    }
    if (transaction->hold_owner != X86_CORE_MACHINE_TRANSACTION_OWNER_DMA ||
        !transaction->hold_acknowledged ||
        transaction->owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE) {
        probe->bad_grant = LIB_TRUE;
        return;
    }
    probe->granted_ticks += ticks;
}

static void executor_dma_finalize(void *context)
{
    executor_dma_probe *probe = context;

    if (probe->machine->transaction.hold_owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE)
        probe->bad_grant = LIB_TRUE;
}

static lib_bool executor_dma_hold(void)
{
    const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_80286, X86_CORE_CPU_PROFILE_80386,
        X86_CORE_CPU_PROFILE_80486SX
    };

    for (lib_size profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (lib_u32 wait = 0u; wait < 2u; ++wait) {
        const x86_core_machine_executor_config config = {
            .memory_bytes = (2u * 1024u * 1024u),
            .cpu_profile = profiles[profile],
            .fpu_profile = X86_FPU_PROFILE_NONE,
            .transaction_contract = {.dma_cycle_wait_quanta = wait,
                .dma_cycle_bus_ready_gate_enabled = wait != 0u}
        };
        executor_dma_probe probe = {0};
        const x86_core_machine_attachment attachment = {.context = &probe,
            .dma_ticks = executor_dma_ticks, .dma_request = executor_dma_request,
            .dma_advance = executor_dma_advance, .finalize_devices = executor_dma_finalize};
        x86_core_machine *machine = LIB_NULL;
        lib_u64 before = 0u;
        const char *stage = "create";
        lib_bool failed = x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

        probe.machine = machine;
        if (!failed) {
            stage = "bind-reset";
            failed = x86_core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK ||
                x86_core_machine_bind_execution_provider(machine, LIB_NULL, LIB_NULL) != LIB_STATUS_OK ||
                x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
                x86_core_machine_reset(machine) != LIB_STATUS_OK;
        }
        if (!failed) {
            stage = "grant";

            failed = x86_core_machine_advance_time(machine, 4u) != LIB_STATUS_OK ||
                probe.bad_grant || probe.granted_ticks != (wait != 0u ? 2u : 4u) ||
                machine->transaction.hold_owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE;
        }
        if (!failed && wait != 0u) {
            stage = "ready";
            before = probe.granted_ticks;
            failed = x86_core_machine_set_dma_bus_ready(machine, LIB_FALSE) != LIB_STATUS_OK ||
                x86_core_machine_advance_time(machine, 4u) != LIB_STATUS_OK ||
                probe.granted_ticks != before || machine->dma_cycle_wait_remaining != 0u ||
                x86_core_machine_set_dma_bus_ready(machine, LIB_TRUE) != LIB_STATUS_OK ||
                x86_core_machine_advance_time(machine, 1u) != LIB_STATUS_OK ||
                probe.granted_ticks != before || machine->dma_cycle_wait_remaining != 1u ||
                x86_core_machine_advance_time(machine, 1u) != LIB_STATUS_OK ||
                probe.granted_ticks != before + 1u || machine->dma_cycle_wait_remaining != 0u;
        }
        if (!failed) {
            stage = "lock";
            before = probe.granted_ticks;
            probe.locked = LIB_TRUE;
            failed = x86_core_machine_transaction_begin(&machine->transaction,
                X86_CORE_MACHINE_TRANSACTION_OWNER_CPU, X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_LOCK,
                0x100u, 0u, 0u) != LIB_STATUS_OK ||
                x86_core_machine_advance_time(machine, 4u) != LIB_STATUS_OK ||
                probe.bad_grant || probe.granted_ticks != before ||
                machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_CPU;
            x86_core_machine_transaction_commit(&machine->transaction);
            probe.locked = LIB_FALSE;
            failed = failed || x86_core_machine_advance_time(machine, 4u) != LIB_STATUS_OK ||
                probe.bad_grant || probe.granted_ticks != before + (wait != 0u ? 2u : 4u) ||
                machine->transaction.hold_owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE;
        }
        if (!failed && wait != 0u) {
            stage = "reset-wait";
            failed = x86_core_machine_advance_time(machine, 1u) != LIB_STATUS_OK ||
                machine->dma_cycle_wait_remaining != 1u ||
                x86_core_machine_reset(machine) != LIB_STATUS_OK ||
                machine->dma_cycle_wait_remaining != 0u;
        }
        x86_core_machine_destroy(machine);
        failed = failed || probe.bad_grant;
        if (failed) {
            lib_c_fprintf(lib_c_stderr,
                "Core DMA HOLD: profile=%u wait=%u stage=%s granted=%llu bad=%u\n",
                (lib_u32)profiles[profile], wait, stage,
                (unsigned long long)probe.granted_ticks, (lib_u32)probe.bad_grant);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

lib_i32 main(void)
{
    x86_core_machine_executor_config config = { .cpu_profile = X86_CORE_CPU_PROFILE_80386, .memory_bytes = 16u * 1024u * 1024u };
    x86_core_machine_run_budget budget = {1u, 0u};
    const x86_core_machine_run_budget alu_budget = {2u, 0u};
    x86_core_machine_run_result result;
    x86_core_machine_observation observation;
    lib_status status;
    x86_core_machine *machine = LIB_NULL;
    const x86_core_cpu_execution_context *cpu;
    const lib_u8 program[] = {0x90u, 0xf4u};
    const lib_u8 register_or_program[] = {
        0xbau, 0x00u, 0x03u, /* mov dx, 0300h */
        0x0au, 0xdfu,       /* or bh, bl */
        0xf4u
    };

    static const struct {
        const char *name;
        lib_bool (*run)(void);
    } cases[] = {
        {"event-time",executor_event_time}, {"sx-fpu-interface",executor_sx_fpu_interface},
        {"sx-bounded-retirement",executor_sx_bounded_retirement},
        {"retired-before-entry-fault",executor_retired_before_entry_fault},
        {"sx-soft-reset",executor_sx_soft_reset}, {"sx-smm",executor_sx_smm},
        {"sx-smm-fault",executor_sx_smm_fault}, {"dma-hold",executor_dma_hold}
    };
    lib_bool failed = LIB_FALSE;

    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        if (!cases[index].run()) {
            lib_c_fprintf(lib_c_stderr, "Core executor case failed: %s\n", cases[index].name);
            failed = LIB_TRUE;
        }
    }
    if (failed) return 1;
    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK) {
        x86_core_machine_destroy(machine);
        return 1;
    }
    cpu = machine->executor_cpu_execution;
    if (cpu == LIB_NULL) {
        x86_core_machine_destroy(machine);
        return 1;
    }
    if (test_x86_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
            0x000ffff0u, 16u) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        machine->executor_cpu_execution != cpu) {
        x86_core_machine_destroy(machine);
        return 1;
    }
    if (x86_core_machine_memory_write(machine, 0xfffffff0u, program, sizeof(program)) !=
        LIB_STATUS_OK) {
        x86_core_machine_destroy(machine);
        return 1;
    }
    status = x86_core_machine_run(machine, budget, &result);
    if (status != LIB_STATUS_OK || result.executed != 1u ||
        result.reason != X86_CORE_MACHINE_STOP_BUDGET) {
        lib_c_fprintf(lib_c_stderr,
            "CORE-EXECUTOR-RUN:FAIL status=%d executed=%llu reason=%d\n",
            (lib_i32)status,
            (unsigned long long)result.executed, (lib_i32)result.reason);
        x86_core_machine_destroy(machine);
        return 1;
    }
    if (x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        machine->executor_cpu_execution != cpu ||
        x86_core_machine_memory_write(machine, 0xfffffff0u, register_or_program,
            sizeof(register_or_program)) != LIB_STATUS_OK ||
        x86_core_machine_run(machine, alu_budget, &result) != LIB_STATUS_OK ||
        result.executed != 2u || result.reason != X86_CORE_MACHINE_STOP_BUDGET ||
        x86_core_machine_capture_observation(machine, &observation) != LIB_STATUS_OK ||
        observation.cpu.eip != 0x0000fff5u) {
        x86_core_machine_destroy(machine);
        return 1;
    }
    status = x86_core_machine_run(machine, budget, &result);
    if (status != LIB_STATUS_OK ||
        result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
        lib_c_fprintf(lib_c_stderr,
            "CORE-EXECUTOR-WAIT:FAIL status=%d executed=%llu reason=%d\n",
            (lib_i32)status, (unsigned long long)result.executed,
            (lib_i32)result.reason);
        x86_core_machine_destroy(machine);
        return 1;
    }
    x86_core_machine_destroy(machine);
    lib_c_printf("%s\n", "CORE-EXECUTOR-RUN:OK");
    return 0;
}
