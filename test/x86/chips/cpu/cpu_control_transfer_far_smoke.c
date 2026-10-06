#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "support/cpu_instruction_fixture.h"

static lib_i32 far_step(cpu_instruction_fixture *state, const lib_u8 *code,
    lib_u8 bytes, t_cpu *after)
{
    state->cpu.data.eip = 0u;
    lib_memory_copy(state->memory + state->cpu.data.cs.base, code, bytes);
    core_machine_cpu_execution_refresh(&state->execution);
    *after = state->cpu;
    return !state->execution.stop_requested && !state->fault.valid;
}

static lib_i32 far_steps(cpu_instruction_fixture *state, const lib_u8 *code,
    lib_u8 bytes, lib_u8 steps, t_cpu *after)
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

static void far_prepare_protected(cpu_instruction_fixture *state)
{
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0x40u,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0xcfu,0,
        0xffu,0xffu,0,0x40u,0,0x92u,0x40u,0,
        0xffu,0xffu,0,0x20u,0,0xbau,0,0,
        0xffu,0xffu,0,0x20u,0,0x1au,0,0
    };
    t_cpu_data_sreg *const cs = &state->cpu.data.cs;
    t_cpu_data_sreg *const ds = &state->cpu.data.ds;
    t_cpu_data_sreg *const ss = &state->cpu.data.ss;

    cpu_instruction_prepare(state, CORE_MACHINE_CPU_PROFILE_80386);
    lib_memory_copy(state->memory + 0x0300u, gdt, sizeof(gdt));
    state->cpu.data.gdtr.base = 0x0300u;
    state->cpu.data.gdtr.limit = (lib_u16)(sizeof(gdt) - 1u);
    state->cpu.data.gdtr.flagValid = LIB_TRUE;
    state->cpu.data.gdtr.sregtype = SREG_GDTR;
    state->cpu.data.cr0 |= VCPU_CR0_PE;
    cs->selector = 0x0008u;
    cs->dpl = 0u;
    cs->base = 0x2000u;
    cs->limit = 0xffffu;
    cs->flagValid = LIB_TRUE;
    cs->sregtype = SREG_CODE;
    cs->seg.executable = LIB_TRUE;
    cs->seg.exec.defsize = LIB_TRUE;
    cs->seg.exec.readable = LIB_TRUE;
    ds->selector = 0x0010u;
    ds->dpl = 0u;
    ds->base = 0x3000u;
    ds->limit = 0xffffffffu;
    ds->flagValid = LIB_TRUE;
    ds->sregtype = SREG_DATA;
    ds->seg.executable = LIB_FALSE;
    ds->seg.data.writable = LIB_TRUE;
    *ss = *ds;
    ss->selector = 0x0018u;
    ss->base = 0x4000u;
    ss->sregtype = SREG_STACK;
    ss->limit = 0xffffu;
    ss->seg.data.big = LIB_TRUE;
    ss->seg.data.expdown = LIB_FALSE;
    state->cpu.data.esp = 0x8000u;
}

static lib_i32 far_expect_protected_fault(cpu_instruction_fixture *state,
    const lib_u8 *code, lib_u8 bytes, lib_u32 exception, const t_cpu *before)
{
    t_cpu after;

    state->cpu.data.idtr.limit = 0x17u;
    state->cpu.data.eip = 0u;
    lib_memory_copy(state->memory + state->cpu.data.cs.base, code, bytes);
    core_machine_cpu_execution_refresh(&state->execution);
    after = state->cpu;
    return state->execution.stop_requested && state->fault.valid &&
        (state->fault.exception_mask & exception) != 0u &&
        after.data.eip == before->data.eip && after.data.esp == before->data.esp &&
        after.data.eflags == before->data.eflags &&
        after.data.cs.selector == before->data.cs.selector &&
        after.data.cs.base == before->data.cs.base &&
        after.data.cs.limit == before->data.cs.limit &&
        after.data.cs.seg.executable == before->data.cs.seg.executable &&
        after.data.cs.seg.exec.conform == before->data.cs.seg.exec.conform &&
        after.data.cs.seg.exec.defsize == before->data.cs.seg.exec.defsize;
}

static lib_i32 far_test_protected_forms(void)
{
    static const lib_u8 jmp32[] = {0xeau,7u,0u,0u,0u,8u,0u,0xf4u};
    static const lib_u8 jmp16[] = {0x66u,0xeau,6u,0u,8u,0u,0xf4u};
    static const lib_u8 call32[] = {
        0x9au,10u,0u,0u,0u,8u,0u,0xb0u,0xa5u,0xf4u,0xcbu
    };
    static const lib_u8 call16[] = {
        0x66u,0x9au,9u,0u,8u,0u,0xb0u,0xa5u,0xf4u,0x66u,0xcbu
    };
    static const lib_u8 indirect_jmp[] = {0xffu,0x2du,0u,1u,0u,0u,0xf4u};
    static const lib_u8 indirect_call[] = {
        0xffu,0x1du,0u,1u,0u,0u,0xb0u,0xa5u,0xf4u,0xcbu
    };
    const lib_u8 *const jumps[] = {jmp32, jmp16};
    const lib_u8 jump_sizes[] = {sizeof(jmp32), sizeof(jmp16)};
    const lib_u8 *const calls[] = {call32, call16};
    const lib_u8 call_sizes[] = {sizeof(call32), sizeof(call16)};
    cpu_instruction_fixture state;
    t_cpu before;
    t_cpu after;

    for (lib_size index = 0u; index < 2u; ++index) {
        far_prepare_protected(&state);
        if (!far_step(&state, jumps[index], jump_sizes[index], &after) ||
            after.data.cs.selector != 8u || after.data.eip != 7u - index) {
            lib_c_fprintf(lib_c_stderr, "protected jump %u cs=%u ip=%u fault=%u\n", (unsigned)index,
                (unsigned)after.data.cs.selector, (unsigned)after.data.eip,
                (unsigned)state.fault.exception_mask);
            return 0;
        }
    }
    for (lib_size index = 0u; index < 2u; ++index) {
        far_prepare_protected(&state);
        before = state.cpu;
        if (!far_step(&state, calls[index], call_sizes[index], &after) ||
            after.data.eip != 10u - index || after.data.esp >= before.data.esp) {
            lib_c_fprintf(lib_c_stderr, "protected call %u ip=%u sp=%u fault=%u\n", (unsigned)index,
                (unsigned)after.data.eip, (unsigned)after.data.esp,
                (unsigned)state.fault.exception_mask);
            return 0;
        }
        core_machine_cpu_execution_refresh(&state.execution);
        after = state.cpu;
        if (state.execution.stop_requested || state.fault.valid ||
            after.data.eip != 7u - index || after.data.esp != before.data.esp) {
            lib_c_fprintf(lib_c_stderr, "protected return %u ip=%u sp=%u fault=%u\n", (unsigned)index,
                (unsigned)after.data.eip, (unsigned)after.data.esp,
                (unsigned)state.fault.exception_mask);
            return 0;
        }
        core_machine_cpu_execution_refresh(&state.execution);
        after = state.cpu;
        if (state.execution.stop_requested || state.fault.valid ||
            after.data.eax != 0xa5u) return 0;
    }
    far_prepare_protected(&state);
    lib_memory_copy(state.memory + state.cpu.data.ds.base + 0x0100u,
        (const lib_u8[]){6u,0u,0u,0u,8u,0u}, 6u);
    if (!far_step(&state, indirect_jmp, sizeof(indirect_jmp), &after) ||
        after.data.cs.selector != 8u || after.data.eip != 6u) {
        lib_c_fprintf(lib_c_stderr, "%s", "protected indirect jump\n");
        return 0;
    }
    far_prepare_protected(&state);
    lib_memory_copy(state.memory + state.cpu.data.ds.base + 0x0100u,
        (const lib_u8[]){9u,0u,0u,0u,8u,0u}, 6u);
    before = state.cpu;
    if (!far_step(&state, indirect_call, sizeof(indirect_call), &after) ||
        after.data.eip != 9u || after.data.esp >= before.data.esp) {
        lib_c_fprintf(lib_c_stderr, "%s", "protected indirect call\n");
        return 0;
    }
    core_machine_cpu_execution_refresh(&state.execution);
    after = state.cpu;
    if (state.execution.stop_requested || state.fault.valid || after.data.eip != 6u ||
        after.data.esp != before.data.esp) {
        lib_c_fprintf(lib_c_stderr, "%s", "protected indirect return\n");
        return 0;
    }
    core_machine_cpu_execution_refresh(&state.execution);
    after = state.cpu;
    if (state.execution.stop_requested || state.fault.valid || after.data.eax != 0xa5u) return 0;
    far_prepare_protected(&state);
    lib_memory_copy(state.memory + state.cpu.data.ss.base + state.cpu.data.esp,
        (const lib_u8[]){0u,0u,0u,0u,0x20u,0u,0u,0u}, 8u);
    before = state.cpu;
    if (!far_expect_protected_fault(&state, (const lib_u8[]){0xcbu}, 1u,
            VCPUINS_EXCEPT_DF, &before)) {
        lib_c_fprintf(lib_c_stderr, "%s", "protected dpl fault\n");
        return 0;
    }
    if (state.memory[0x0325u] != 0xbau) return 0;
    far_prepare_protected(&state);
    lib_memory_copy(state.memory + state.cpu.data.ss.base + state.cpu.data.esp,
        (const lib_u8[]){0u,0u,0u,0u,0x28u,0u,0u,0u}, 8u);
    before = state.cpu;
    if (!far_expect_protected_fault(&state, (const lib_u8[]){0xcbu}, 1u,
            VCPUINS_EXCEPT_DF, &before)) {
        lib_c_fprintf(lib_c_stderr, "%s", "protected np fault\n");
        return 0;
    }
    if (state.memory[0x032du] != 0x1au) return 0;
    far_prepare_protected(&state);
    before = state.cpu;
    if (!far_expect_protected_fault(&state, (const lib_u8[]){0xffu,0xd8u}, 2u,
            VCPUINS_EXCEPT_UD, &before)) {
        lib_c_fprintf(lib_c_stderr, "%s", "protected reserved d8\n");
        return 0;
    }
    far_prepare_protected(&state);
    before = state.cpu;
    if (!far_expect_protected_fault(&state, (const lib_u8[]){0xffu,0xe8u}, 2u,
            VCPUINS_EXCEPT_UD, &before)) {
        lib_c_fprintf(lib_c_stderr, "%s", "protected reserved e8\n");
        return 0;
    }
    return 1;
}

static lib_i32 far_test_real_near_control(core_machine_cpu_profile profile)
{
    static const lib_u8 jump_near[] = {0xe9u,2u,0u,0xb0u,0u,0xf4u};
    static const lib_u8 jump_short[] = {0xebu,2u,0xb0u,0u,0xf4u};
    static const lib_u8 call_near[] = {0xe8u,3u,0u,0xb0u,0xa5u,0xf4u,0xc3u};
    static const lib_u8 call_indirect[] = {
        0xb8u,8u,0u,0xffu,0xd0u,0xb0u,0xa5u,0xf4u,0xc3u
    };
    static const lib_u8 jump_indirect[] = {
        0xb8u,5u,0u,0xffu,0xe0u,0xb0u,0xa5u,0xf4u
    };
    static const lib_u8 call_indirect_memory[] = {
        0xffu,0x16u,0u,1u,0xb0u,0xa5u,0xf4u,0xc3u
    };
    static const lib_u8 jump_indirect_memory[] = {
        0xffu,0x26u,0u,1u,0xb0u,0xa5u,0xf4u
    };
    static const lib_u8 return_immediate[] = {0xc2u,2u,0u,0xf4u};
    cpu_instruction_fixture state;
    t_cpu before;
    t_cpu after;

    cpu_instruction_prepare(&state, profile);
    before = state.cpu;
    if (!far_steps(&state, jump_near, sizeof(jump_near), 2u, &after) ||
        after.data.ip != sizeof(jump_near) || after.data.ax != before.data.ax ||
        after.data.sp != before.data.sp || after.data.eflags != before.data.eflags) return 0;
    cpu_instruction_prepare(&state, profile);
    before = state.cpu;
    if (!far_steps(&state, jump_short, sizeof(jump_short), 2u, &after) ||
        after.data.ip != sizeof(jump_short) || after.data.ax != before.data.ax ||
        after.data.sp != before.data.sp || after.data.eflags != before.data.eflags) return 0;
    cpu_instruction_prepare(&state, profile);
    before = state.cpu;
    if (!far_steps(&state, call_near, sizeof(call_near), 3u, &after) ||
        after.data.al != 0xa5u || after.data.sp != before.data.sp ||
        after.data.eflags != before.data.eflags) return 0;
    cpu_instruction_prepare(&state, profile);
    before = state.cpu;
    if (!far_steps(&state, call_indirect, sizeof(call_indirect), 4u, &after) ||
        after.data.al != 0xa5u || after.data.sp != before.data.sp ||
        after.data.eflags != before.data.eflags) return 0;
    cpu_instruction_prepare(&state, profile);
    before = state.cpu;
    if (!far_steps(&state, jump_indirect, sizeof(jump_indirect), 3u, &after) ||
        after.data.al != 0xa5u || after.data.sp != before.data.sp ||
        after.data.eflags != before.data.eflags) return 0;
    cpu_instruction_prepare(&state, profile);
    lib_memory_copy(state.memory + 0x0100u, (const lib_u8[]){7u,0u}, 2u);
    before = state.cpu;
    if (!far_steps(&state, call_indirect_memory, sizeof(call_indirect_memory), 3u,
            &after) || after.data.al != 0xa5u || after.data.sp != before.data.sp ||
        after.data.eflags != before.data.eflags) return 0;
    cpu_instruction_prepare(&state, profile);
    lib_memory_copy(state.memory + 0x0100u, (const lib_u8[]){4u,0u}, 2u);
    before = state.cpu;
    if (!far_steps(&state, jump_indirect_memory, sizeof(jump_indirect_memory), 2u,
            &after) || after.data.al != 0xa5u || after.data.sp != before.data.sp ||
        after.data.eflags != before.data.eflags) return 0;
    cpu_instruction_prepare(&state, profile);
    state.cpu.data.sp = 0x8000u;
    lib_memory_copy(state.memory + 0x8000u, (const lib_u8[]){3u,0u}, 2u);
    before = state.cpu;
    return far_step(&state, return_immediate, sizeof(return_immediate), &after) &&
        after.data.ip == 3u && after.data.sp == before.data.sp + 4u &&
        after.data.eflags == before.data.eflags;
}

static lib_i32 far_test_real_mode(core_machine_cpu_profile profile)
{
    static const lib_u8 jump[] = {0xeau,0u,0u,0u,1u};
    static const lib_u8 call[] = {0x9au,0u,0u,0u,1u};
    static const lib_u8 retf[] = {0xcbu};
    static const lib_u8 retf_immediate[] = {0xcau,2u,0u};
    static const lib_u8 indirect_jump[] = {0xffu,0x2eu,0u,1u};
    static const lib_u8 indirect_jump_boundary[] = {0xffu,0x2eu,0xfeu,0xffu};
    static const lib_u8 indirect_call[] = {0xffu,0x1eu,0u,1u};
    static const lib_u8 reserved[][2] = {
        {0xffu,0xd8u}, {0xffu,0xe8u}, {0xffu,0xf8u}
    };
    cpu_instruction_fixture state;
    t_cpu after;
    lib_size index;

    cpu_instruction_prepare(&state, profile);
    if (!far_step(&state, jump, sizeof(jump), &after) ||
        after.data.cs.selector != 0x100u || after.data.cs.base != 0x1000u) return 0;
    cpu_instruction_prepare(&state, profile);
    state.cpu.data.esp = 0x8000u;
    if (!far_step(&state, call, sizeof(call), &after) ||
        after.data.cs.selector != 0x100u || after.data.sp != 0x7ffcu) return 0;
    state.memory[0x1000u] = retf[0u];
    core_machine_cpu_execution_refresh(&state.execution);
    after = state.cpu;
    if (state.execution.stop_requested || state.fault.valid ||
        after.data.cs.selector != 0u || after.data.sp != 0x8000u) return 0;
    cpu_instruction_prepare(&state, profile);
    state.cpu.data.esp = 0x8000u;
    if (!far_step(&state, call, sizeof(call), &after) ||
        after.data.cs.selector != 0x100u || after.data.sp != 0x7ffcu) return 0;
    lib_memory_copy(state.memory + 0x1000u, retf_immediate, sizeof(retf_immediate));
    core_machine_cpu_execution_refresh(&state.execution);
    after = state.cpu;
    if (state.execution.stop_requested || state.fault.valid ||
        after.data.cs.selector != 0u || after.data.sp != 0x8002u) return 0;
    cpu_instruction_prepare(&state, profile);
    lib_memory_copy(state.memory + 0x0100u, (const lib_u8[]){0u,0u,0u,1u}, 4u);
    if (!far_step(&state, indirect_jump, sizeof(indirect_jump), &after) ||
        after.data.cs.selector != 0x100u || after.data.cs.base != 0x1000u) return 0;
    cpu_instruction_prepare(&state, profile);
    /* Separate DS from CS so wrapped pointer bytes cannot alias the opcode.
     * Early CPUs wrap the second word; 286/386 reject the full four-byte span. */
    state.cpu.data.ds.base = 0x10000u;
    lib_memory_copy(state.memory + 0x1fffeu, (const lib_u8[]){0u,0u}, 2u);
    lib_memory_copy(state.memory + 0x10000u, (const lib_u8[]){0u,2u}, 2u);
    if (profile < CORE_MACHINE_CPU_PROFILE_80286) {
        if (!far_step(&state, indirect_jump_boundary, sizeof(indirect_jump_boundary), &after) ||
            after.data.cs.selector != 0x200u || after.data.cs.base != 0x2000u) return 0;
    } else {
        state.cpu.data.idtr.limit = 0x17u;
        if (cpu_instruction_run(&state, indirect_jump_boundary,
                sizeof(indirect_jump_boundary), &after) != LIB_STATUS_INTERNAL_ERROR ||
            !state.fault.valid ||
            (state.fault.exception_mask & VCPUINS_EXCEPT_GP) == 0u ||
            after.data.cs.selector != 0u || after.data.eip != 0u) return 0;
    }
    cpu_instruction_prepare(&state, profile);
    state.cpu.data.esp = 0x8000u;
    lib_memory_copy(state.memory + 0x0100u, (const lib_u8[]){0u,0u,0u,1u}, 4u);
    if (!far_step(&state, indirect_call, sizeof(indirect_call), &after) ||
        after.data.cs.selector != 0x100u || after.data.sp != 0x7ffcu) return 0;
    state.memory[0x1000u] = retf[0u];
    core_machine_cpu_execution_refresh(&state.execution);
    after = state.cpu;
    if (state.execution.stop_requested || state.fault.valid ||
        after.data.cs.selector != 0u || after.data.sp != 0x8000u) return 0;
    cpu_instruction_prepare(&state, profile);
    /* T337_REAL_UD_TERMINAL_CPU_OWNER: no IVT is installed in this CPU fixture. */
    state.cpu.data.idtr.limit = 0x17u;
    for (index = 0u; index < sizeof(reserved) / sizeof(reserved[0]); ++index) {
        const t_cpu before = state.cpu;
        if (cpu_instruction_run(&state, reserved[index], sizeof(reserved[index]), &after) !=
                LIB_STATUS_INTERNAL_ERROR || !state.fault.valid ||
            (state.fault.exception_mask & VCPUINS_EXCEPT_UD) == 0u ||
            lib_memory_compare(&before.data, &after.data, sizeof(before.data)) != 0) return 0;
        cpu_instruction_prepare(&state, profile);
        state.cpu.data.idtr.limit = 0x17u;
    }
    return 1;
}

int main(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    if (!far_test_protected_forms()) {
        lib_c_fprintf(lib_c_stderr, "%s", "protected\n");
        return 1;
    }
    for (lib_size index = 0u; index < sizeof(profiles) / sizeof(profiles[0]);
        ++index) {
        if (!far_test_real_mode(profiles[index]) ||
            !far_test_real_near_control(profiles[index])) {
            lib_c_fprintf(lib_c_stderr, "real profile %u\n", (unsigned)profiles[index]);
            return 1;
        }
    }
    lib_c_printf("%s\n", "M5:T303:CONTROL-TRANSFER:OK");
    lib_c_printf("%s\n", "M5:T401:S23:FAR-RETURN-PROFILES:OK");
    lib_c_printf("%s\n", "M5:T539:S51:CPU-CONTROL-TRANSFER-FAR:OK");
    return 0;
}
