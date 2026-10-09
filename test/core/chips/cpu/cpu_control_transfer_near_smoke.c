#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "support/cpu_instruction_fixture.h"

static lib_i32 near_run(cpu_instruction_fixture *state, const lib_u8 *code,
    lib_u8 bytes, t_cpu *after)
{
    state->cpu.data.eip = 0u;
    return cpu_instruction_run(state, code, bytes, after) == LIB_STATUS_OK &&
        !state->fault.valid;
}

static lib_i32 near_run_steps(cpu_instruction_fixture *state,
    const lib_u8 *code, lib_u8 bytes, lib_u8 steps, t_cpu *after)
{
    lib_memory_copy(state->memory, code, bytes);
    state->cpu.data.eip = 0u;
    for (lib_u8 step = 0u; step < steps; ++step) {
        core_machine_cpu_execution_refresh(&state->execution);
        if (state->execution.stop_requested || state->fault.valid) return 0;
    }
    *after = state->cpu;
    return 1;
}

static lib_i32 near_test_call_and_return_forms(void)
{
    static const lib_u8 code32_call32[] = {0xe8u,3u,0,0,0,0xb0u,0xa5u,0xf4u,0xc3u};
    static const lib_u8 code32_call16[] = {0x66u,0xe8u,3u,0,0xb0u,0xa5u,0xf4u,0x66u,0xc3u};
    static const lib_u8 code16_call16[] = {0xe8u,3u,0,0xb0u,0xa5u,0xf4u,0xc3u};
    static const lib_u8 code16_call32[] = {0x66u,0xe8u,3u,0,0,0,0xb0u,0xa5u,0xf4u,0x66u,0xc3u};
    const lib_u8 *const programs[] = { code32_call32, code32_call16,
        code16_call16, code16_call32 };
    const lib_u8 sizes[] = { sizeof(code32_call32), sizeof(code32_call16),
        sizeof(code16_call16), sizeof(code16_call32) };

    for (lib_size index = 0u; index < sizeof(programs) / sizeof(programs[0]);
        ++index) {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.cs.seg.data.big = index < 2u;
        state.cpu.data.esp = 0x8000u;
        before = state.cpu;
        if (!near_run_steps(&state, programs[index], sizes[index], 3u, &after) ||
            after.data.eax != 0x000000a5u || after.data.esp != before.data.esp ||
            after.data.eflags != before.data.eflags) return 0;
    }
    return 1;
}

static lib_i32 near_test_return_immediate(void)
{
    static const lib_u8 ret16[] = {0x66u,0xc2u,4u,0,0xf4u};
    static const lib_u8 ret32[] = {0xc2u,4u,0,0xf4u};
    static const lib_u8 target16[] = {4u,0u};
    static const lib_u8 target32[] = {3u,0u,0u,0u};
    const lib_u8 *const programs[] = { ret16, ret32 };
    const lib_u8 *const targets[] = { target16, target32 };
    const lib_u8 target_sizes[] = { sizeof(target16), sizeof(target32) };
    const lib_u8 sizes[] = { sizeof(ret16), sizeof(ret32) };
    const lib_u32 expected[] = { 6u, 8u };

    for (lib_size index = 0u; index != 2u; ++index) {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.cs.seg.data.big = LIB_TRUE;
        state.cpu.data.ss.seg.data.big = LIB_TRUE;
        state.cpu.data.ss.limit = 0xffffffffu;
        state.cpu.data.esp = 0x8000u;
        lib_memory_copy(state.memory + state.cpu.data.esp, targets[index],
            target_sizes[index]);
        before = state.cpu;
        if (!near_run(&state, programs[index], sizes[index], &after) ||
            after.data.esp != before.data.esp + expected[index]) return 0;
    }
    return 1;
}

static lib_i32 near_test_indirect_and_fault_boundaries(void)
{
    static const lib_u8 call32[] = {0xb8u,10u,0,0,0,0xffu,0xd0u,0xb0u,0xa5u,0xf4u,0xc3u};
    static const lib_u8 call16[] = {0xb8u,8u,0,0xffu,0xd0u,0xb0u,0xa5u,0xf4u,0xc3u};
    static const lib_u8 jmp32[] = {0xffu,0x25u,0,1u,0,0,0xb0u,0,0xf4u};
    static const lib_u8 jmp16[] = {0xffu,0x26u,0,1u,0xb0u,0,0xf4u};
    static const lib_u8 target32[] = {8u,0,0,0};
    static const lib_u8 target16[] = {6u,0};
    static const lib_u8 call_fault[] = {0xe8u,0x7bu,0,0,0};
    static const lib_u8 jmp_fault[] = {0xffu,0xe0u};
    cpu_instruction_fixture state;
    t_cpu before;
    t_cpu after;

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.cpu.data.cs.seg.data.big = LIB_TRUE;
    state.cpu.data.esp = 0x8000u;
    before = state.cpu;
    if (!near_run_steps(&state, call32, sizeof(call32), 4u, &after) ||
        after.data.eax != 0xa5u || after.data.esp != before.data.esp) return 0;
    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.cpu.data.esp = 0x8000u;
    before = state.cpu;
    if (!near_run_steps(&state, call16, sizeof(call16), 4u, &after) ||
        after.data.eax != 0xa5u || after.data.esp != before.data.esp) return 0;
    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.cpu.data.cs.seg.data.big = LIB_TRUE;
    lib_memory_copy(state.memory + 0x100u, target32, sizeof(target32));
    if (!near_run(&state, jmp32, sizeof(jmp32), &after) || after.data.eip != 8u) return 0;
    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    lib_memory_copy(state.memory + 0x100u, target16, sizeof(target16));
    if (!near_run(&state, jmp16, sizeof(jmp16), &after) || after.data.eip != 6u) return 0;
    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.cpu.data.cs.seg.data.big = LIB_TRUE;
    state.cpu.data.cs.limit = 0x7fu;
    if (!cpu_instruction_expect_real_fault(&state, call_fault, sizeof(call_fault), 13u)) return 0;
    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.cpu.data.cs.seg.data.big = LIB_TRUE;
    state.cpu.data.cs.limit = 0x7fu;
    state.cpu.data.eax = 0x80u;
    before = state.cpu;
    return cpu_instruction_expect_real_fault(&state, jmp_fault, sizeof(jmp_fault), 13u) &&
        state.cpu.data.eax == before.data.eax;
}

int main(void)
{
    if (!near_test_call_and_return_forms()) goto fail_call;
    if (!near_test_return_immediate()) goto fail_return;
    if (!near_test_indirect_and_fault_boundaries()) goto fail_indirect;
    lib_c_printf("%s\n", "NEAR-RETURN-PROFILES:OK");
    lib_c_printf("%s\n", "CPU-CONTROL-TRANSFER-NEAR:OK");
    return 0;
fail_call:
    lib_c_fprintf(lib_c_stderr, "%s", "call: ");
    goto fail;
fail_return:
    lib_c_fprintf(lib_c_stderr, "%s", "return: ");
    goto fail;
fail_indirect:
    lib_c_fprintf(lib_c_stderr, "%s", "indirect: ");
fail:
    lib_c_fprintf(lib_c_stderr, "%s", "CPU-CONTROL-TRANSFER-NEAR:FAIL\n");
    return 1;
}
