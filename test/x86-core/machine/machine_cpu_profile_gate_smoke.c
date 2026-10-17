#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "x86-core/machine/machine_interface.h"
#include "x86-core/machine/debug_interface.h"
#include "support/exception_fixture.h"

typedef struct cpu_profile_machine {
    x86_core_machine *machine;
} cpu_profile_machine;

/* REAL_UD_TERMINAL_IVT_REJECT: fault cases make the IVT route
 * unavailable, without changing CPU IDTR. */
static lib_i32 prepare_machine(x86_core_cpu_profile profile,
    cpu_profile_machine *state, lib_bool terminal_ud)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = profile,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ES) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP)
    };
    if (state == LIB_NULL) return 1;
    lib_memory_set(state, 0, sizeof(*state));
    if (x86_core_machine_neutral_create(&config, &state->machine) != LIB_STATUS_OK) return 1;
    if ((terminal_ud && test_x86_core_exception_block_vector(state->machine, 6u, state) != LIB_STATUS_OK) ||
        x86_core_machine_freeze_execution_providers(state->machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(state->machine) != LIB_STATUS_OK ||
        (profile != X86_CORE_CPU_PROFILE_80486SX &&
            (x86_core_machine_signal_soft_processor_reset(state->machine) != LIB_STATUS_UNSUPPORTED ||
                x86_core_machine_set_smi(state->machine, LIB_TRUE) != LIB_STATUS_UNSUPPORTED)) ||
        x86_core_machine_debug_patch_registers(state->machine, &entry) != LIB_STATUS_OK) {
        x86_core_machine_destroy(state->machine);
        state->machine = LIB_NULL;
        return 1;
    }
    return 0;
}

static lib_i32 run_case(x86_core_cpu_profile profile, const lib_u8 *program,
    lib_size program_size, lib_i32 expect_ud)
{
    cpu_profile_machine state;
    x86_core_machine_run_budget budget = { 1u, 0u };
    x86_core_machine_run_result result = {0};
    x86_core_cpu_diagnostic diagnostic = {0};
    lib_status run_status = LIB_STATUS_INVALID_STATE;
    lib_i32 failed = prepare_machine(profile, &state, expect_ud != 0);

    if (!failed) {
        failed |= x86_core_machine_memory_write(state.machine, 0u, program,
            program_size) != LIB_STATUS_OK;
        run_status = x86_core_machine_run(state.machine, budget, &result);
        failed |= run_status != (expect_ud ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK);
        failed |= expect_ud && result.reason != X86_CORE_MACHINE_STOP_FAULT;
        failed |= x86_core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != LIB_STATUS_OK;
        if (expect_ud) {
            failed |= !diagnostic.first_fault.valid ||
                diagnostic.first_fault.exception_mask !=
                    (profile < X86_CORE_CPU_PROFILE_80186 ?
                        VCPUINS_EXCEPT_UD : VCPUINS_EXCEPT_CE) ||
                (profile >= X86_CORE_CPU_PROFILE_80186 &&
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
    x86_core_machine_destroy(state.machine);
    return failed;
}

static lib_i32 run_pop_cs_8086(void)
{
    static const lib_u8 program[] = { 0xbcu, 0x00u, 0x01u, 0x0fu };
    lib_u8 selector[2] = { 0u, 0u };
    cpu_profile_machine state;
    x86_core_cpu_state cpu;
    x86_core_machine_run_budget budget = { 2u, 0u };
    x86_core_machine_run_result result;
    x86_core_cpu_diagnostic diagnostic;
    lib_i32 failed = prepare_machine(X86_CORE_CPU_PROFILE_8086, &state, LIB_FALSE);

    if (!failed) {
        failed |= x86_core_machine_memory_write(state.machine, 0u, program,
            sizeof(program)) != LIB_STATUS_OK;
        failed |= x86_core_machine_memory_write(state.machine, 0x0100u, selector,
            sizeof(selector)) != LIB_STATUS_OK;
        failed |= x86_core_machine_run(state.machine, budget, &result) != LIB_STATUS_OK;
        failed |= x86_core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != LIB_STATUS_OK;
        failed |= x86_core_machine_get_cpu_state(state.machine, &cpu) != LIB_STATUS_OK;
        failed |= diagnostic.first_fault.valid || cpu.cs != 0u ||
            cpu.eip != 4u;
    }
    x86_core_machine_destroy(state.machine);
    return failed;
}

static lib_i32 run_mov_rm8_imm8_8086(void)
{
    static const lib_u8 program[] = { 0xc6u, 0x06u, 0xbeu, 0x1fu, 0x01u };
    lib_u8 value = 0u;
    cpu_profile_machine state;
    x86_core_cpu_state cpu;
    x86_core_machine_run_budget budget = { 1u, 0u };
    x86_core_machine_run_result result;
    x86_core_cpu_diagnostic diagnostic;
    lib_i32 failed = prepare_machine(X86_CORE_CPU_PROFILE_8086, &state, LIB_FALSE);

    if (!failed) {
        failed |= x86_core_machine_memory_write(state.machine, 0u, program,
            sizeof(program)) != LIB_STATUS_OK;
        failed |= x86_core_machine_run(state.machine, budget, &result) != LIB_STATUS_OK;
        failed |= x86_core_machine_memory_read(state.machine, 0x1fbeu,
            &value, sizeof(value)) != LIB_STATUS_OK;
        failed |= x86_core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            LIB_STATUS_OK;
        failed |= x86_core_machine_get_cpu_state(state.machine, &cpu) != LIB_STATUS_OK;
        failed |= diagnostic.first_fault.valid || value != 1u ||
            cpu.eip != sizeof(program);
    }
    x86_core_machine_destroy(state.machine);
    return failed;
}

static lib_i32 run_selected_sx(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 2u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80486SX,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS),
        .values = {[X86_CORE_CPU_DEBUG_EFLAGS] = 2u}
    };
    const lib_u8 program[] = {
        0x66u,0xb8u,1u,0u,0u,0u,           /* mov eax,1 */
        0x0fu,0xa2u,                       /* cpuid */
        0x66u,0xa3u,0u,4u,                 /* mov [0400h],eax */
        0x66u,0x89u,0x16u,4u,4u,           /* mov [0404h],edx */
        0x0fu,0x21u,0xf3u,                 /* mov ebx,dr6 */
        0x66u,0xb8u,0x12u,0x34u,0x56u,0x78u,
        0x66u,0x0fu,0xc8u                  /* bswap eax */
    };
    x86_core_machine *machine = LIB_NULL;
    x86_core_cpu_debug_cpu_snapshot reset, after;
    x86_core_machine_run_result result = {0};
    x86_core_cpu_profile profile = X86_CORE_CPU_PROFILE_DEFAULT;
    lib_u32 identity[2] = {0};
    lib_i32 failed = !x86_core_machine_neutral_config_is_valid(&config) ||
        x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

    if (!failed) failed = x86_core_machine_get_cpu_profile(machine, &profile) != LIB_STATUS_OK ||
        profile != config.cpu_profile ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_debug_capture_cpu_snapshot(machine, X86_CORE_CPU_SNAPSHOT_CURRENT,
            &reset) != LIB_STATUS_OK || reset.edx != 0x042bu || reset.eip != 0xfff0u ||
        reset.cs.selector != 0xf000u || reset.cs.base != 0xffff0000u ||
        reset.cr0 != 0x60000010u ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0u, program, sizeof(program)) != LIB_STATUS_OK ||
        x86_core_machine_run(machine, (x86_core_machine_run_budget){7u,0u}, &result) != LIB_STATUS_OK ||
        result.executed != 7u || result.reason != X86_CORE_MACHINE_STOP_BUDGET ||
        x86_core_machine_debug_capture_cpu_snapshot(machine, X86_CORE_CPU_SNAPSHOT_CURRENT,
            &after) != LIB_STATUS_OK || after.eax != 0x12345678u || after.ebx != 0xffff0ff0u ||
        after.eip != sizeof(program) ||
        x86_core_machine_debug_read_memory(machine, 0x400u, identity, sizeof(identity)) != LIB_STATUS_OK ||
        identity[0] != 0x042bu || (identity[1] & 3u) != 2u;
    if (failed) lib_c_fprintf(lib_c_stderr, "Selected SX production construction/reset/run failed\n");
    x86_core_machine_destroy(machine);
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

    failed |= run_selected_sx();

    failed |= run_case(X86_CORE_CPU_PROFILE_8086, nop, sizeof(nop), 0);
    failed |= run_case(X86_CORE_CPU_PROFILE_8088, nop, sizeof(nop), 0);
    failed |= run_pop_cs_8086();
    failed |= run_mov_rm8_imm8_8086();
    failed |= run_case(X86_CORE_CPU_PROFILE_8086, pusha, sizeof(pusha), 1);
    failed |= run_case(X86_CORE_CPU_PROFILE_8086, shift_rm16_imm8,
        sizeof(shift_rm16_imm8), 1);
    failed |= run_case(X86_CORE_CPU_PROFILE_80186, shift_rm16_imm8,
        sizeof(shift_rm16_imm8), 0);
    failed |= run_case(X86_CORE_CPU_PROFILE_80186, arpl, sizeof(arpl), 1);
    failed |= run_case(X86_CORE_CPU_PROFILE_80286, fs_prefix, sizeof(fs_prefix), 1);
    failed |= run_case(X86_CORE_CPU_PROFILE_80286, jcc_near, sizeof(jcc_near), 1);
    if (failed) return 1;
    lib_c_printf("CPU-PROFILE-GATE:OK\n");
    return 0;
}
