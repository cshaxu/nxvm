#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "x86/core/device_support_interface.h"

#include "x86/core/machine_interface.h"
#include "x86/core/debug_interface.h"
#include "exception_fixture.h"

typedef struct cpu_profile_machine {
    core_machine *machine;
} cpu_profile_machine;

/* T337_REAL_UD_TERMINAL_IVT_REJECT: fault cases make the IVT route
 * unavailable, without changing CPU IDTR. */
static lib_i32 prepare_machine(core_machine_cpu_profile profile,
    cpu_profile_machine *state, lib_bool terminal_ud)
{
    const core_machine_executor_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP)
    };
    if (state == LIB_NULL) return 1;
    lib_memory_set(state, 0, sizeof(*state));
    if (core_machine_neutral_create(&config, &state->machine) != LIB_STATUS_OK) return 1;
    if ((terminal_ud && test_core_exception_block_vector(state->machine, 6u, state) != LIB_STATUS_OK) ||
        core_machine_freeze_execution_providers(state->machine) != LIB_STATUS_OK ||
        core_machine_reset(state->machine) != LIB_STATUS_OK ||
        core_machine_debug_patch_registers(state->machine, &entry) != LIB_STATUS_OK) {
        core_machine_destroy(state->machine);
        state->machine = LIB_NULL;
        return 1;
    }
    return 0;
}

static lib_i32 run_case(core_machine_cpu_profile profile, const lib_u8 *program,
    lib_size program_size, lib_i32 expect_ud)
{
    cpu_profile_machine state;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    lib_status run_status = LIB_STATUS_INVALID_STATE;
    lib_i32 failed = prepare_machine(profile, &state, expect_ud != 0);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0u, program,
            program_size) != LIB_STATUS_OK;
        run_status = core_machine_run(state.machine, budget, &result);
        failed |= run_status != (expect_ud ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK);
        failed |= expect_ud && result.reason != CORE_MACHINE_STOP_FAULT;
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != LIB_STATUS_OK;
        if (expect_ud) {
            failed |= !diagnostic.first_fault.valid ||
                diagnostic.first_fault.exception_mask !=
                    (profile < CORE_MACHINE_CPU_PROFILE_80186 ?
                        VCPUINS_EXCEPT_UD : VCPUINS_EXCEPT_CE) ||
                (profile >= CORE_MACHINE_CPU_PROFILE_80186 &&
                    diagnostic.first_fault.exception_code != 6u * 4u) ||
                diagnostic.first_fault.point.bytes[0] != program[0];
        } else {
            failed |= diagnostic.first_fault.valid;
        }
    }
    if (failed) lib_c_printf("Profile case failed: profile=%u opcode=%02x status=%u reason=%u fault=%u mask=%x\n",
        (lib_u32)profile, program[0], (lib_u32)run_status,
        (lib_u32)result.reason, diagnostic.first_fault.valid,
        diagnostic.first_fault.exception_mask);
    core_machine_destroy(state.machine);
    return failed;
}

static lib_i32 run_pop_cs_8086(void)
{
    static const lib_u8 program[] = { 0xbcu, 0x00u, 0x01u, 0x0fu };
    lib_u8 selector[2] = { 0u, 0u };
    cpu_profile_machine state;
    core_machine_cpu_state cpu;
    core_machine_run_budget budget = { 2u, 0u };
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    lib_i32 failed = prepare_machine(CORE_MACHINE_CPU_PROFILE_8086, &state, LIB_FALSE);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0u, program,
            sizeof(program)) != LIB_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x0100u, selector,
            sizeof(selector)) != LIB_STATUS_OK;
        failed |= core_machine_run(state.machine, budget, &result) != LIB_STATUS_OK;
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != LIB_STATUS_OK;
        failed |= core_machine_get_cpu_state(state.machine, &cpu) != LIB_STATUS_OK;
        failed |= diagnostic.first_fault.valid || cpu.cs != 0u ||
            cpu.eip != 4u;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static lib_i32 run_mov_rm8_imm8_8086(void)
{
    static const lib_u8 program[] = { 0xc6u, 0x06u, 0xbeu, 0x1fu, 0x01u };
    lib_u8 value = 0u;
    cpu_profile_machine state;
    core_machine_cpu_state cpu;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    lib_i32 failed = prepare_machine(CORE_MACHINE_CPU_PROFILE_8086, &state, LIB_FALSE);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0u, program,
            sizeof(program)) != LIB_STATUS_OK;
        failed |= core_machine_run(state.machine, budget, &result) != LIB_STATUS_OK;
        failed |= core_machine_memory_read(state.machine, 0x1fbeu,
            &value, sizeof(value)) != LIB_STATUS_OK;
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            LIB_STATUS_OK;
        failed |= core_machine_get_cpu_state(state.machine, &cpu) != LIB_STATUS_OK;
        failed |= diagnostic.first_fault.valid || value != 1u ||
            cpu.eip != sizeof(program);
    }
    core_machine_destroy(state.machine);
    return failed;
}

lib_i32 main(void)
{
    static const lib_u8 nop[] = { 0x90u };
    static const lib_u8 pusha[] = { 0x60u };
    static const lib_u8 arpl[] = { 0x63u, 0xc0u };
    static const lib_u8 fs_prefix[] = { 0x64u, 0x90u };
    static const lib_u8 jcc_near[] = { 0x0fu, 0x80u, 0u, 0u };
    static const lib_u8 shift_rm16_imm8[] = { 0xc1u, 0xeau, 0x04u };
    lib_i32 failed = 0;

    failed |= run_case(CORE_MACHINE_CPU_PROFILE_8086, nop, sizeof(nop), 0);
    failed |= run_case(CORE_MACHINE_CPU_PROFILE_8088, nop, sizeof(nop), 0);
    failed |= run_pop_cs_8086();
    failed |= run_mov_rm8_imm8_8086();
    failed |= run_case(CORE_MACHINE_CPU_PROFILE_8086, pusha, sizeof(pusha), 1);
    failed |= run_case(CORE_MACHINE_CPU_PROFILE_8086, shift_rm16_imm8,
        sizeof(shift_rm16_imm8), 1);
    failed |= run_case(CORE_MACHINE_CPU_PROFILE_80186, shift_rm16_imm8,
        sizeof(shift_rm16_imm8), 0);
    failed |= run_case(CORE_MACHINE_CPU_PROFILE_80186, arpl, sizeof(arpl), 1);
    failed |= run_case(CORE_MACHINE_CPU_PROFILE_80286, fs_prefix, sizeof(fs_prefix), 1);
    failed |= run_case(CORE_MACHINE_CPU_PROFILE_80286, jcc_near, sizeof(jcc_near), 1);
    if (failed) return 1;
    lib_c_printf("M5:T155:S1:CPU-PROFILE-GATE:OK\n");
    return 0;
}
