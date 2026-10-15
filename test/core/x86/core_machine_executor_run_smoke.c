#include "lib/types/types_interface.h"
#include "lib/types/file.h"



#include "core/x86/machine_interface.h"
#include "core/x86/machine.h"
#include "core/x86/device_support_interface.h"
#include "core/x86/debug_interface.h"
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

    if (!executor_sx_fpu_interface()) return 1;
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
