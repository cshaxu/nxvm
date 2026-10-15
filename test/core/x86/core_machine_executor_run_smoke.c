#include "lib/types/types_interface.h"
#include "lib/types/file.h"



#include "core/x86/machine_interface.h"
#include "core/x86/machine.h"
#include "core/x86/device_support_interface.h"
#include "core/x86/debug_interface.h"
#include "core/x86/attachment_interface.h"
#include "core/chips/cpu/cpu_instructions.h"
#include "../support/memory_alias_fixture.h"

static lib_bool executor_sx_fpu_interface(void)
{
    const core_machine_executor_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const core_machine_instruction_timing timing = {.base_ticks = 1u};
    const lib_u16 vector[] = {0x0100u,0u};
    const lib_u8 hlt = 0xf4u;

    for (lib_u8 controls = 0u; controls < 16u; ++controls)
    for (lib_u8 operation = 0u; operation < 9u; ++operation) {
        core_machine *machine = LIB_NULL;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_debug_cpu_snapshot before, after;
        const lib_u32 cr0 = ((controls & 1u) ? VCPU_CR0_EM : 0u) |
            ((controls & 2u) ? VCPU_CR0_TS : 0u) |
            ((controls & 4u) ? VCPU_CR0_MP : 0u) |
            ((controls & 8u) ? VCPU_CR0_NE : 0u);
        const lib_bool nm = operation < 8u || (controls & 6u) == 6u;
        const lib_u8 code[] = {operation < 8u ? (lib_u8)(0xd8u + operation) : 0x9bu,
            operation < 8u ? 0xc0u : 0x90u};
        const core_machine_debug_register_patch entry = {
            .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CR0),
            .values = {[CORE_MACHINE_DEBUG_ESP] = 0x8000u,
                [CORE_MACHINE_DEBUG_EFLAGS] = 2u, [CORE_MACHINE_DEBUG_CR0] = cr0}
        };
        lib_u16 frame[3];
        lib_bool failed = core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

        if (!failed) {
            failed = core_machine_bind_execution_provider(machine, LIB_NULL, LIB_NULL) != LIB_STATUS_OK ||
                core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
                core_machine_reset(machine) != LIB_STATUS_OK;
        }
        if (!failed) {
            /* Internal Executor qualification; the production admission gate stays closed. */
            core_machine_cpu_execution_context_bind_profiles(machine->executor_cpu_execution,
                CORE_MACHINE_CPU_PROFILE_80486SX, X86_FPU_PROFILE_NONE, LIB_FALSE, &timing);
            failed = core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0u, code, sizeof(code)) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x1cu, vector, sizeof(vector)) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, vector[0], &hlt, sizeof(hlt)) != LIB_STATUS_OK ||
                core_machine_debug_capture_cpu_snapshot(machine, CORE_MACHINE_CPU_SNAPSHOT_CURRENT,
                    &before) != LIB_STATUS_OK ||
                core_machine_run(machine, (core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET ||
                core_machine_get_cpu_diagnostic(machine, &diagnostic) != LIB_STATUS_OK ||
                core_machine_debug_capture_cpu_snapshot(machine, CORE_MACHINE_CPU_SNAPSHOT_CURRENT,
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
                    core_machine_memory_read(machine, after.ss.base + (lib_u16)after.esp,
                        frame, sizeof(frame)) != LIB_STATUS_OK ||
                    frame[0] != 0u || frame[1] != before.cs.selector || frame[2] != before.eflags;
            }
        }
        if (!failed) {
            failed = core_machine_run(machine, (core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
                result.reason != (nm ? CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT : CORE_MACHINE_STOP_BUDGET) ||
                core_machine_debug_capture_cpu_snapshot(machine, CORE_MACHINE_CPU_SNAPSHOT_CURRENT,
                    &after) != LIB_STATUS_OK || after.eip != (nm ? vector[0] + 1u : 2u);
        }
        core_machine_destroy(machine);
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
    const core_machine_executor_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const core_machine_instruction_timing timing = {.base_ticks = 1u};
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
        .values = {[CORE_MACHINE_DEBUG_ESP] = 0x8000u, [CORE_MACHINE_DEBUG_EFLAGS] = 2u}
    };
    const lib_u16 vector[] = {0x100u, 0u};
    const lib_u8 nop = 0x90u;

    for (core_machine_cpu_profile profile = CORE_MACHINE_CPU_PROFILE_80386;
         profile <= CORE_MACHINE_CPU_PROFILE_80486SX; ++profile)
    for (lib_u8 route = 0u; route < 3u; ++route) {
        core_machine *machine = LIB_NULL;
        core_machine_run_result result = {0};
        const lib_bool nmi = route == 1u;
        const lib_bool overflow = route == 2u;
        const lib_u8 code[] = {nmi ? 0x90u : 0x0fu, 0x0bu};
        const lib_u64 clocks = profile == CORE_MACHINE_CPU_PROFILE_80486SX ?
            (nmi ? 30u : 26u) : nmi ? 3u : 0u;
        const lib_u64 before = overflow ? LIB_UINT64_MAX - 25u : 0u;
        lib_status status = LIB_STATUS_INTERNAL_ERROR;
        lib_bool failed;

        if (overflow && profile != CORE_MACHINE_CPU_PROFILE_80486SX) continue;
        failed = core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
        if (!failed) {
            failed = core_machine_bind_execution_provider(machine, LIB_NULL, LIB_NULL) != LIB_STATUS_OK ||
                core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
                core_machine_reset(machine) != LIB_STATUS_OK ||
                core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0u, code, sizeof(code)) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, nmi ? 8u : 24u, vector, sizeof(vector)) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, vector[0], &nop, sizeof(nop)) != LIB_STATUS_OK;
        }
        if (!failed) {
            /* Qualify the real Executor seam without opening production SX admission. */
            core_machine_cpu_execution_context_bind_profiles(machine->executor_cpu_execution,
                profile, X86_FPU_PROFILE_NONE, LIB_FALSE, &timing);
            machine->elapsed_ticks = before;
            failed = nmi && !core_machine_cpu_request_nmi(machine->executor_cpu_execution);
            status = core_machine_run(machine, (core_machine_run_budget){1u, 0u}, &result);
            failed = failed || status != (overflow ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK) ||
                result.executed != (nmi ? 1u : 0u) ||
                result.reason != (overflow ? CORE_MACHINE_STOP_FAULT : CORE_MACHINE_STOP_BUDGET) ||
                result.ticks != (overflow ? 0u : clocks) ||
                result.elapsed_ticks != (overflow ? before : clocks) ||
                machine->elapsed_ticks != result.elapsed_ticks;
        }
        if (!failed && !overflow) {
            failed = core_machine_run(machine, (core_machine_run_budget){1u, 0u}, &result) != LIB_STATUS_OK ||
                result.executed != 1u || result.ticks != (profile == CORE_MACHINE_CPU_PROFILE_80486SX ? 1u : 3u) ||
                result.elapsed_ticks != clocks + result.ticks;
        }
        if (failed)
            lib_c_printf("Executor event: profile=%u route=%u status=%u count=%llu ticks=%llu elapsed=%llu\n",
                (lib_u32)profile, route, (lib_u32)status, result.executed, result.ticks, result.elapsed_ticks);
        core_machine_destroy(machine);
        if (failed) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static void executor_reset_event(void *context, lib_u64 due_tick)
{
    *(lib_u64 *)context = due_tick;
}

static lib_bool executor_sx_soft_reset(void)
{
    const core_machine_executor_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const core_machine_instruction_timing timing = {.base_ticks = 1u};
    const lib_u8 marker = 0x5au;
    core_machine *machine = LIB_NULL;
    core_machine_run_result result;
    lib_u8 observed = 0u;
    lib_u64 event_tick = 0u;
    lib_u64 due_tick;
    core_machine_timeline_token token;
    lib_u64 ticks;
    lib_bool failed = core_machine_signal_soft_processor_reset(LIB_NULL) !=
        LIB_STATUS_INVALID_ARGUMENT || core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

    if (!failed) {
        failed = core_machine_signal_soft_processor_reset(machine) != LIB_STATUS_UNSUPPORTED ||
            core_machine_bind_execution_provider(machine, LIB_NULL, LIB_NULL) != LIB_STATUS_OK ||
            core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            core_machine_reset(machine) != LIB_STATUS_OK;
    }
    if (!failed) {
        /* This internal owner qualifies the chip before production SX admission. */
        core_machine_cpu_execution_context_bind_profiles(machine->executor_cpu_execution,
            CORE_MACHINE_CPU_PROFILE_80486SX, X86_FPU_PROFILE_NONE, LIB_FALSE, &timing);
        machine->executor_cpu_execution->smbase = 0x00180000u;
        ticks = machine->elapsed_ticks;
        failed = core_machine_memory_write(machine, 0x100u, &marker, sizeof(marker)) != LIB_STATUS_OK ||
            core_machine_timeline_schedule(&machine->timeline, ticks + 10u,
                executor_reset_event, &event_tick, &token) != LIB_STATUS_OK ||
            core_machine_signal_soft_processor_reset(machine) != LIB_STATUS_OK ||
            machine->executor_cpu_execution->smbase != 0x00180000u ||
            core_machine_timeline_pending_count(&machine->timeline) != 1u ||
            core_machine_timeline_next_due(&machine->timeline, &due_tick) != LIB_STATUS_OK ||
            due_tick != ticks + 10u || event_tick != 0u ||
            core_machine_run(machine, (core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_RESET_REQUESTED || result.executed != 0u ||
            result.ticks != 0u || result.linear_pc != 0xfffffff0u ||
            machine->elapsed_ticks != ticks || machine->lifecycle != CORE_MACHINE_STOPPED ||
            machine->executor_cpu_execution->smbase != 0x00180000u ||
            core_machine_memory_read(machine, 0x100u, &observed, sizeof(observed)) != LIB_STATUS_OK ||
            observed != marker;
    }
    if (!failed) {
        core_machine_signal_processor_reset(machine);
        failed = core_machine_signal_soft_processor_reset(machine) != LIB_STATUS_OK ||
            core_machine_run(machine, (core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_RESET_REQUESTED || result.executed != 0u ||
            machine->executor_cpu_execution->smbase != X86_CPU_DEFAULT_SMBASE ||
            core_machine_timeline_pending_count(&machine->timeline) != 1u ||
            core_machine_timeline_advance(&machine->timeline, ticks + 10u) != LIB_STATUS_OK ||
            event_tick != ticks + 10u ||
            core_machine_memory_read(machine, 0x100u, &observed, sizeof(observed)) != LIB_STATUS_OK ||
            observed != marker;
    }
    core_machine_destroy(machine);
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
    const core_machine_executor_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const core_machine_instruction_timing timing = {.base_ticks = 1u};
    const lib_u8 handler[] = {
        0x26u,0x66u,0x67u,0xc7u,0x05u,0xd0u,0xffu,0x03u,0u,0x78u,0x56u,0x34u,0x12u,
        0x0fu,0xaau
    };
    const lib_u8 nop = 0x90u;
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CR0),
        .values = {[CORE_MACHINE_DEBUG_EIP] = 0x100u, [CORE_MACHINE_DEBUG_ESP] = 0x8000u,
            [CORE_MACHINE_DEBUG_EFLAGS] = 2u,
            [CORE_MACHINE_DEBUG_CR0] = VCPU_CR0_ET | VCPU_CR0_CD | VCPU_CR0_NW}
    };
    executor_smm_probe probe = {0};
    const core_machine_attachment attachment = {.context = &probe,
        .finalize_devices = executor_smm_finalize,
        .system_management_active = executor_smm_active};
    core_machine *machine = LIB_NULL;
    core_machine_run_result result;
    core_machine_debug_cpu_snapshot after;
    lib_u8 observed = 0u;
    lib_bool failed = core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

    if (!failed) {
        failed = core_machine_set_smi(LIB_NULL, LIB_TRUE) != LIB_STATUS_INVALID_ARGUMENT ||
            core_machine_set_smi(machine, LIB_TRUE) != LIB_STATUS_UNSUPPORTED ||
            core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK ||
            core_machine_bind_execution_provider(machine, LIB_NULL, LIB_NULL) != LIB_STATUS_OK ||
            core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            core_machine_reset(machine) != LIB_STATUS_OK;
    }
    if (!failed) {
        /* Internal CPU/Core qualification only; the public admission gate stays closed. */
        core_machine_cpu_execution_context_bind_profiles(machine->executor_cpu_execution,
            CORE_MACHINE_CPU_PROFILE_80486SX, X86_FPU_PROFILE_NONE, LIB_FALSE, &timing);
        failed = core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x38000u, handler, sizeof(handler)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x100u, &nop, sizeof(nop)) != LIB_STATUS_OK ||
            core_machine_set_smi(machine, LIB_TRUE) != LIB_STATUS_OK ||
            core_machine_run(machine, (core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 0u || result.ticks != 0u ||
            result.linear_pc != 0x38000u || !probe.active || probe.assertions != 1u ||
            core_machine_memory_read(machine, 0x100u, &observed, sizeof(observed)) != LIB_STATUS_OK ||
            observed != nop;
    }
    for (lib_u8 instruction = 0u; !failed && instruction < 2u; ++instruction) {
        failed = core_machine_run(machine, (core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
            result.executed != 1u || result.reason != CORE_MACHINE_STOP_BUDGET;
    }
    if (!failed) {
        failed = core_machine_debug_capture_cpu_snapshot(machine, CORE_MACHINE_CPU_SNAPSHOT_CURRENT,
            &after) != LIB_STATUS_OK || after.eip != 0x100u || after.cs.base != 0u ||
            after.eax != 0x12345678u || probe.active || probe.releases != 1u ||
            core_machine_set_smi(machine, LIB_FALSE) != LIB_STATUS_OK ||
            core_machine_set_smi(machine, LIB_TRUE) != LIB_STATUS_OK ||
            core_machine_run(machine, (core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
            result.executed != 0u || !probe.active;
    }
    core_machine_destroy(machine);
    return !failed && probe.finalized == 1u && probe.assertions == 2u &&
        probe.releases == 2u && !probe.bad_order;
}

static lib_bool executor_sx_smm_fault(void)
{
    const core_machine_executor_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const core_machine_instruction_timing timing = {.base_ticks = 1u};
    const lib_u8 rsm[] = {0x0fu,0xaau};
    const lib_u32 positions[] = {2u,127u};
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CR0),
        .values = {[CORE_MACHINE_DEBUG_EIP] = 0x100u,
            [CORE_MACHINE_DEBUG_CR0] = VCPU_CR0_ET | VCPU_CR0_CD | VCPU_CR0_NW}
    };

    for (lib_u8 restore = 0u; restore < 2u; ++restore)
    for (lib_size index = 0u; index < sizeof(positions) / sizeof(positions[0]); ++index) {
        executor_smm_probe probe = {.fail_restore = restore != 0u};
        const core_machine_attachment attachment = {.context = &probe,
            .finalize_devices = executor_smm_finalize,
            .system_management_active = executor_smm_active,
            .memory_cache_control = executor_smm_memory_control};
        core_machine *machine = LIB_NULL;
        core_machine_run_result result = {0};
        core_machine_cpu_diagnostic diagnostic = {0};
        core_machine_debug_cpu_snapshot before = {0}, after = {0};
        lib_status run_status = LIB_STATUS_OK;
        lib_u8 image[512], observed[4];
        lib_u64 cancelled = 0u;
        lib_bool failed = core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

        lib_memory_set(image, 0xa5u, sizeof(image));
        if (!failed) {
            failed = core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK ||
                core_machine_bind_execution_provider(machine, LIB_NULL, LIB_NULL) != LIB_STATUS_OK ||
                core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
                core_machine_reset(machine) != LIB_STATUS_OK;
        }
        if (!failed) {
            core_machine_cpu_execution_context_bind_profiles(machine->executor_cpu_execution,
                CORE_MACHINE_CPU_PROFILE_80486SX, X86_FPU_PROFILE_NONE, LIB_FALSE, &timing);
            failed = core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x38000u, rsm, sizeof(rsm)) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x3fe00u, image, sizeof(image)) != LIB_STATUS_OK ||
                core_machine_set_smi(machine, LIB_TRUE) != LIB_STATUS_OK;
        }
        if (!failed && restore != 0u) {
            failed = core_machine_run(machine, (core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 0u || !probe.active;
        }
        if (!failed) {
            probe.fail_transfer = positions[index];
            cancelled = machine->transaction.cancelled_count;
            failed = core_machine_debug_capture_cpu_snapshot(machine,
                    CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK ||
                (run_status = core_machine_run(machine,
                    (core_machine_run_budget){1u,0u}, &result)) !=
                    LIB_STATUS_INTERNAL_ERROR || result.reason != CORE_MACHINE_STOP_FAULT ||
                result.executed != 0u || result.ticks != 0u ||
                probe.transfers != positions[index] || probe.bad_order || !probe.active ||
                core_machine_get_cpu_diagnostic(machine, &diagnostic) != LIB_STATUS_OK ||
                !diagnostic.first_fault.valid ||
                diagnostic.first_fault.exception_mask != VCPUINS_EXCEPT_CE ||
                diagnostic.first_fault.exception_code != 0x40000u - positions[index] * 4u ||
                core_machine_debug_capture_cpu_snapshot(machine,
                    CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
                lib_memory_compare(&before, &after, sizeof(before)) ||
                machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE ||
                machine->transaction.cancelled_count != cancelled + 1u;
        }
        if (!failed && restore == 0u) {
            const lib_u8 cr0[] = {0x10u,0u,0u,0x60u};

            /* The first save write is committed even though a later cycle
             * fails; the rejected write retains the original RAM bytes. */
            failed = core_machine_memory_inspect(machine, 0x3fffcu, observed, sizeof(observed)) !=
                    LIB_STATUS_OK || lib_memory_compare(observed, cr0, sizeof(cr0)) ||
                core_machine_memory_inspect(machine, 0x40000u - positions[index] * 4u,
                    observed, sizeof(observed)) != LIB_STATUS_OK ||
                lib_memory_compare(observed, image, sizeof(observed));
        }
        if (failed && machine != LIB_NULL) {
            (void)core_machine_get_cpu_diagnostic(machine, &diagnostic);
            lib_c_fprintf(lib_c_stderr,
                "Core SMM fault detail: status=%u reason=%u retired=%llu active=%u bad=%u "
                "mask=%08x code=%08x cancelled=%llu/%llu\n",
                (lib_u32)run_status, (lib_u32)result.reason,
                (unsigned long long)result.executed, (lib_u32)probe.active,
                (lib_u32)probe.bad_order, diagnostic.first_fault.exception_mask,
                diagnostic.first_fault.exception_code,
                (unsigned long long)machine->transaction.cancelled_count,
                (unsigned long long)cancelled);
        }
        core_machine_destroy(machine);
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
    core_machine *machine;
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
    const core_machine_transaction_state *transaction = &probe->machine->transaction;

    if (probe->locked) {
        if (transaction->owner != CORE_MACHINE_TRANSACTION_OWNER_CPU ||
            transaction->kind != CORE_MACHINE_TRANSACTION_CPU_MEMORY_LOCK ||
            transaction->hold_acknowledged) probe->bad_grant = LIB_TRUE;
        return;
    }
    if (transaction->hold_owner != CORE_MACHINE_TRANSACTION_OWNER_DMA ||
        !transaction->hold_acknowledged ||
        transaction->owner != CORE_MACHINE_TRANSACTION_OWNER_NONE) {
        probe->bad_grant = LIB_TRUE;
        return;
    }
    probe->granted_ticks += ticks;
}

static void executor_dma_finalize(void *context)
{
    executor_dma_probe *probe = context;

    if (probe->machine->transaction.hold_owner != CORE_MACHINE_TRANSACTION_OWNER_NONE)
        probe->bad_grant = LIB_TRUE;
}

static lib_bool executor_dma_hold(void)
{
    const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386,
        CORE_MACHINE_CPU_PROFILE_80486SX
    };
    const core_machine_instruction_timing timing = {.base_ticks = 1u};

    for (lib_size profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (lib_u32 wait = 0u; wait < 2u; ++wait) {
        const core_machine_executor_config config = {
            .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
            .cpu_profile = profiles[profile] == CORE_MACHINE_CPU_PROFILE_80486SX ?
                CORE_MACHINE_CPU_PROFILE_80386 : profiles[profile],
            .fpu_profile = X86_FPU_PROFILE_NONE,
            .transaction_contract = {.dma_cycle_wait_quanta = wait,
                .dma_cycle_bus_ready_gate_enabled = wait != 0u}
        };
        executor_dma_probe probe = {0};
        const core_machine_attachment attachment = {.context = &probe,
            .dma_ticks = executor_dma_ticks, .dma_request = executor_dma_request,
            .dma_advance = executor_dma_advance, .finalize_devices = executor_dma_finalize};
        core_machine *machine = LIB_NULL;
        lib_u64 before = 0u;
        const char *stage = "create";
        lib_bool failed = core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

        probe.machine = machine;
        if (!failed) {
            stage = "bind-reset";
            failed = core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK ||
                core_machine_bind_execution_provider(machine, LIB_NULL, LIB_NULL) != LIB_STATUS_OK ||
                core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
                core_machine_reset(machine) != LIB_STATUS_OK;
        }
        if (!failed) {
            stage = "grant";
            /* Internal qualification of the scheduler and CPU together, not
             * production admission of a 486 machine. */
            machine->cpu_profile = profiles[profile];
            core_machine_cpu_execution_context_bind_profiles(machine->executor_cpu_execution,
                profiles[profile], X86_FPU_PROFILE_NONE, LIB_FALSE, &timing);
            failed = core_machine_advance_time(machine, 4u) != LIB_STATUS_OK ||
                probe.bad_grant || probe.granted_ticks != (wait != 0u ? 2u : 4u) ||
                machine->transaction.hold_owner != CORE_MACHINE_TRANSACTION_OWNER_NONE;
        }
        if (!failed && wait != 0u) {
            stage = "ready";
            before = probe.granted_ticks;
            failed = core_machine_set_dma_bus_ready(machine, LIB_FALSE) != LIB_STATUS_OK ||
                core_machine_advance_time(machine, 4u) != LIB_STATUS_OK ||
                probe.granted_ticks != before ||
                core_machine_set_dma_bus_ready(machine, LIB_TRUE) != LIB_STATUS_OK;
        }
        if (!failed) {
            stage = "lock";
            before = probe.granted_ticks;
            probe.locked = LIB_TRUE;
            failed = core_machine_transaction_begin(&machine->transaction,
                CORE_MACHINE_TRANSACTION_OWNER_CPU, CORE_MACHINE_TRANSACTION_CPU_MEMORY_LOCK,
                0x100u, 0u, 0u) != LIB_STATUS_OK ||
                core_machine_advance_time(machine, 4u) != LIB_STATUS_OK ||
                probe.bad_grant || probe.granted_ticks != before ||
                machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_CPU;
            core_machine_transaction_commit(&machine->transaction);
            probe.locked = LIB_FALSE;
            failed = failed || core_machine_advance_time(machine, 4u) != LIB_STATUS_OK ||
                probe.bad_grant || probe.granted_ticks != before + (wait != 0u ? 2u : 4u) ||
                machine->transaction.hold_owner != CORE_MACHINE_TRANSACTION_OWNER_NONE;
        }
        core_machine_destroy(machine);
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
    core_machine_executor_config config = { .memory_bytes = 0u };
    core_machine_run_budget budget = {1u, 0u};
    const core_machine_run_budget alu_budget = {2u, 0u};
    core_machine_run_result result;
    core_machine_observation observation;
    lib_status status;
    core_machine *machine = LIB_NULL;
    const core_machine_cpu_execution_context *cpu;
    const lib_u8 program[] = {0x90u, 0xf4u};
    const lib_u8 register_or_program[] = {
        0xbau, 0x00u, 0x03u, /* mov dx, 0300h */
        0x0au, 0xdfu,       /* or bh, bl */
        0xf4u
    };

    if (!executor_event_time() || !executor_sx_fpu_interface() || !executor_sx_soft_reset() || !executor_sx_smm() ||
        !executor_sx_smm_fault() || !executor_dma_hold()) return 1;
    if (core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK) {
        core_machine_destroy(machine);
        return 1;
    }
    cpu = machine->executor_cpu_execution;
    if (cpu == LIB_NULL) {
        core_machine_destroy(machine);
        return 1;
    }
    if (test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
            0x000ffff0u, 16u) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        machine->executor_cpu_execution != cpu) {
        core_machine_destroy(machine);
        return 1;
    }
    if (core_machine_memory_write(machine, 0xfffffff0u, program, sizeof(program)) !=
        LIB_STATUS_OK) {
        core_machine_destroy(machine);
        return 1;
    }
    status = core_machine_run(machine, budget, &result);
    if (status != LIB_STATUS_OK || result.executed != 1u ||
        result.reason != CORE_MACHINE_STOP_BUDGET) {
        lib_c_fprintf(lib_c_stderr,
            "CORE-EXECUTOR-RUN:FAIL status=%d executed=%llu reason=%d\n",
            (lib_i32)status,
            (unsigned long long)result.executed, (lib_i32)result.reason);
        core_machine_destroy(machine);
        return 1;
    }
    if (core_machine_reset(machine) != LIB_STATUS_OK ||
        machine->executor_cpu_execution != cpu ||
        core_machine_memory_write(machine, 0xfffffff0u, register_or_program,
            sizeof(register_or_program)) != LIB_STATUS_OK ||
        core_machine_run(machine, alu_budget, &result) != LIB_STATUS_OK ||
        result.executed != 2u || result.reason != CORE_MACHINE_STOP_BUDGET ||
        core_machine_capture_observation(machine, &observation) != LIB_STATUS_OK ||
        observation.cpu.eip != 0x0000fff5u) {
        core_machine_destroy(machine);
        return 1;
    }
    status = core_machine_run(machine, budget, &result);
    if (status != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
        lib_c_fprintf(lib_c_stderr,
            "CORE-EXECUTOR-WAIT:FAIL status=%d executed=%llu reason=%d\n",
            (lib_i32)status, (unsigned long long)result.executed,
            (lib_i32)result.reason);
        core_machine_destroy(machine);
        return 1;
    }
    core_machine_destroy(machine);
    lib_c_printf("%s\n", "CORE-EXECUTOR-RUN:OK");
    return 0;
}
