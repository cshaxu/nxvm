#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"

#include "app-nxvm/devices/cpu.h"
#include "app-nxvm/devices/cpu_instructions.h"
#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_cpu_fixture.h"

#define ARPL_GDT_POINTER_ADDRESS 0x0100u
#define ARPL_GDT_ADDRESS 0x0300u
#define ARPL_CODE_ADDRESS 0x2000u
#define ARPL_DATA_ADDRESS 0x3000u

typedef struct arpl_machine {
    core_machine *machine;
} arpl_machine;

static void arpl_reset(void *opaque)
{
    arpl_machine *state = (arpl_machine *)opaque;

    if (state != LIB_NULL) (void)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static const core_machine_execution_provider arpl_execution_provider = {
    arpl_reset,
    LIB_NULL
};

static lib_i32 arpl_prepare(arpl_machine *state,
    core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    if (state == LIB_NULL) return 0;
    lib_memory_set(state, 0, sizeof(*state));
    if (!test_core_machine_fixture_create_bind_freeze_reset(&config,
            &arpl_execution_provider, state, &state->machine)) {
        core_machine_destroy(state->machine);
        state->machine = LIB_NULL;
        return 0;
    }
    return 1;
}

static lib_i32 arpl_install_gdt(core_machine *machine)
{
    static const lib_u8 gdt_pointer[] = {
        0x17u, 0x00u, 0x00u, 0x03u, 0x00u, 0x00u
    };
    static const lib_u8 gdt[] = {
        0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
        0xffu, 0xffu, 0x00u, 0x20u, 0x00u, 0x9au, 0x00u, 0x00u,
        0xffu, 0xffu, 0x00u, 0x30u, 0x00u, 0x92u, 0x00u, 0x00u
    };

    return core_machine_memory_write(machine, ARPL_GDT_POINTER_ADDRESS,
        gdt_pointer, sizeof(gdt_pointer)) == LIB_STATUS_OK &&
        core_machine_memory_write(machine, ARPL_GDT_ADDRESS, gdt,
            sizeof(gdt)) == LIB_STATUS_OK;
}

static lib_i32 arpl_run_protected(arpl_machine *state,
    const lib_u8 *protected_code, lib_size protected_code_size,
    t_cpu *out_cpu)
{
    static const lib_u8 real_code[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0xb8u, 0x01u, 0x00u,
        0x0fu, 0x01u, 0xf0u,
        0xb8u, 0x10u, 0x00u,
        0x8eu, 0xd8u,
        0x8eu, 0xc0u,
        0x8eu, 0xd0u,
        0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    const core_machine_run_budget budget = { 64u, 0u };
    core_machine_run_result result;
    lib_status run_status;

    if (state == LIB_NULL || state->machine == LIB_NULL ||
        protected_code == LIB_NULL || out_cpu == LIB_NULL ||
        !arpl_install_gdt(state->machine) ||
        core_machine_memory_write(state->machine, 0u, real_code,
            sizeof(real_code)) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, ARPL_CODE_ADDRESS,
            protected_code, protected_code_size) != LIB_STATUS_OK) return 0;
    run_status = core_machine_run(state->machine, budget, &result);
    if (run_status != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
        fprintf(stderr,
            "M5:T263:S3:ARPL run status=%d reason=%d executed=%llu\n",
            run_status, result.reason,
            (unsigned long long)result.executed);
        return 0;
    }
    *out_cpu = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return 1;
}

static lib_i32 arpl_test_register_forms(void)
{
    static const lib_u8 adjust_code[] = {
        0xb8u, 0x01u, 0x00u,
        0xb9u, 0x03u, 0x00u,
        0x63u, 0xc8u,
        0xf4u
    };
    static const lib_u8 retain_code[] = {
        0xb8u, 0x03u, 0x00u,
        0xb9u, 0x01u, 0x00u,
        0x63u, 0xc8u,
        0xf4u
    };
    arpl_machine state;
    t_cpu cpu;
    lib_i32 failed = !arpl_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286);

    if (!failed) {
        lib_i32 ran = arpl_run_protected(&state, adjust_code, sizeof(adjust_code),
            &cpu);
        failed |= !ran;
        failed |= (cpu.data.eax & 0xffffu) != 0x0003u ||
            !CORE_MACHINE_BIT_IS_SET(cpu.data.eflags, VCPU_EFLAGS_ZF);
        if (!ran) {
            fprintf(stderr,
                "M5:T263:S3:ARPL register-adjust stopped ip=%08x ax=%04x flags=%08x\n",
                cpu.data.eip, cpu.data.eax & 0xffffu, cpu.data.eflags);
        }
    }
    core_machine_destroy(state.machine);
    state.machine = LIB_NULL;
    if (!failed) failed = !arpl_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286);
    if (!failed) {
        lib_i32 ran = arpl_run_protected(&state, retain_code, sizeof(retain_code),
            &cpu);
        failed |= !ran;
        failed |= (cpu.data.eax & 0xffffu) != 0x0003u ||
            CORE_MACHINE_BIT_IS_SET(cpu.data.eflags, VCPU_EFLAGS_ZF);
        if (!ran) {
            fprintf(stderr,
                "M5:T263:S3:ARPL register-retain stopped ip=%08x ax=%04x flags=%08x\n",
                cpu.data.eip, cpu.data.eax & 0xffffu, cpu.data.eflags);
        }
    }
    core_machine_destroy(state.machine);
    return failed;
}

static lib_i32 arpl_test_memory_prefix_form(void)
{
    static const lib_u8 code[] = {
        0xb9u, 0x03u, 0x00u,
        0x26u, 0x63u, 0x0eu, 0x00u, 0x04u,
        0xf4u
    };
    lib_u16 selector = 0x0001u;
    t_cpu cpu;
    arpl_machine state;
    lib_i32 failed = !arpl_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286);

    if (!failed) {
        lib_i32 ran;
        failed |= core_machine_memory_write(state.machine,
            ARPL_DATA_ADDRESS + 0x0400u, &selector, sizeof(selector)) !=
            LIB_STATUS_OK;
        ran = arpl_run_protected(&state, code, sizeof(code), &cpu);
        failed |= !ran;
        failed |= core_machine_memory_read(state.machine,
            ARPL_DATA_ADDRESS + 0x0400u, &selector, sizeof(selector)) !=
            LIB_STATUS_OK;
        failed |= selector != 0x0003u ||
            !CORE_MACHINE_BIT_IS_SET(cpu.data.eflags, VCPU_EFLAGS_ZF);
        if (!ran) {
            fprintf(stderr,
                "M5:T263:S3:ARPL memory-prefix stopped ip=%08x value=%04x flags=%08x\n",
                cpu.data.eip, selector, cpu.data.eflags);
        }
    }
    core_machine_destroy(state.machine);
    return failed;
}

static lib_i32 arpl_test_rejected_forms(void)
{
    static const lib_u8 program[] = {
        0xb8u, 0x01u, 0x00u,
        0xb9u, 0x03u, 0x00u,
        0x63u, 0xc8u
    };
    const core_machine_run_budget budget = { 3u, 0u };
    const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    lib_size index;
    lib_i32 failed = 0;

    for (index = 0u; index < sizeof(profiles) / sizeof(profiles[0]); ++index) {
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu cpu;
        arpl_machine state;

        if (!arpl_prepare(&state, profiles[index])) {
            failed = 1;
            continue;
        }
        failed |= core_machine_memory_write(state.machine, 0u, program,
            sizeof(program)) != LIB_STATUS_OK;
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        failed |= core_machine_run(state.machine, budget, &result) !=
            LIB_STATUS_INTERNAL_ERROR || result.reason != CORE_MACHINE_STOP_FAULT;
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            LIB_STATUS_OK || !diagnostic.first_fault.valid ||
            !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_UD) || diagnostic.first_fault.point.bytes[0] !=
            0x63u;
        cpu = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= (cpu.data.eax & 0xffffu) != 0x0001u;
        core_machine_destroy(state.machine);
    }
    return failed;
}

static lib_i32 arpl_test_metadata(void)
{
    core_machine_cpu_instruction_metadata metadata =
        core_machine_cpu_instruction_metadata_get(
            CORE_MACHINE_CPU_INSTRUCTION_PRIMARY, 0x63u, 0xc8u);

    return !metadata.valid || metadata.minimum_cpu !=
        CORE_MACHINE_CPU_PROFILE_80286;
}

lib_i32 main(void)
{
    lib_i32 metadata = arpl_test_metadata();
    lib_i32 registers = arpl_test_register_forms();
    lib_i32 memory_prefix = arpl_test_memory_prefix_form();
    lib_i32 rejected = arpl_test_rejected_forms();

    if (metadata || registers || memory_prefix || rejected) {
        fprintf(stderr,
            "M5:T263:S3:ARPL:FAIL metadata=%d registers=%d memory_prefix=%d rejected=%d\n",
            metadata, registers, memory_prefix, rejected);
        return 1;
    }
    printf("M5:T263:S2:ARPL:OK\n");
    printf("M5:T263:S3:ARPL:CORPUS:OK\n");
    return 0;
}
