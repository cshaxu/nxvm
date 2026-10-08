#include "support/cpu_outer_return_fixture.h"
#include "lib/types/file.h"

static lib_bool cpu_outer_user_code(const t_cpu_data_sreg *segment)
{
    return segment->flagValid && segment->selector == 0x001bu &&
        segment->sregtype == SREG_CODE && segment->base == CPU_OUTER_USER_CODE_BASE &&
        segment->limit == 0xffffu && segment->dpl == 3u &&
        segment->seg.accessed && segment->seg.executable &&
        !segment->seg.exec.defsize && !segment->seg.exec.conform &&
        segment->seg.exec.readable;
}

static lib_bool cpu_outer_user_stack(const t_cpu_data_sreg *segment)
{
    return segment->flagValid && segment->selector == 0x0023u &&
        segment->sregtype == SREG_STACK && segment->base == CPU_OUTER_USER_STACK_BASE &&
        segment->limit == 0xffffu && segment->dpl == 3u &&
        segment->seg.accessed && !segment->seg.executable &&
        segment->seg.data.writable && !segment->seg.data.expdown;
}

static lib_bool cpu_outer_conforming_user_code(const t_cpu_data_sreg *segment,
    lib_u8 dpl)
{
    return segment->flagValid && segment->selector == 0x001bu &&
        segment->sregtype == SREG_CODE && segment->base == CPU_OUTER_USER_CODE_BASE &&
        segment->limit == 0xffffu && segment->dpl == dpl &&
        segment->seg.accessed && segment->seg.executable &&
        !segment->seg.exec.defsize && segment->seg.exec.conform &&
        segment->seg.exec.readable;
}

static void cpu_outer_prepare_return_segments(cpu_instruction_fixture *state,
    core_machine_cpu_profile profile)
{
    state->cpu.data.ds = state->cpu.data.ss;
    state->cpu.data.ds.sregtype = SREG_DATA;
    state->cpu.data.es = state->cpu.data.cs;
    state->cpu.data.es.sregtype = SREG_DATA;
    state->cpu.data.es.selector = 0x001bu;
    state->cpu.data.es.dpl = 0u;
    state->cpu.data.es.seg.exec.conform = LIB_TRUE;
    if (profile >= CORE_MACHINE_CPU_PROFILE_80386) {
        state->cpu.data.fs = state->cpu.data.ds;
        state->cpu.data.gs = state->cpu.data.es;
    }
}

static lib_bool cpu_outer_conforming_returns(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 conforming_access[] = {0x9eu,0xfeu};
    static const lib_u8 programs[][1] = {{0xcbu},{0xcfu}};
    static const lib_u32 frames32[][5] = {
        {0x10u,0x1bu,0x1000u,0x23u,0u},
        {0x10u,0x1bu,0x203u,0x1000u,0x23u}
    };
    static const lib_u16 frames16[][5] = {
        {0x10u,0x1bu,0x1000u,0x23u,0u},
        {0x10u,0x1bu,0x203u,0x1000u,0x23u}
    };
    lib_size profile;
    lib_size form;
    lib_size dpl;
    lib_u8 wide;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        for (form = 0u; form != sizeof(programs) / sizeof(programs[0]); ++form) {
            for (dpl = 0u; dpl != sizeof(conforming_access) / sizeof(conforming_access[0]);
                    ++dpl) {
                for (wide = 0u; wide <= (profiles[profile] >= CORE_MACHINE_CPU_PROFILE_80386);
                        ++wide) {
                    cpu_instruction_fixture state;
                    t_cpu after;
                    const lib_size words = form ? 5u : 4u;
                    const lib_size frame_bytes = words * (wide ? 4u : 2u);

                    cpu_outer_return_prepare(&state, profiles[profile]);
                    state.cpu.data.cs.seg.exec.defsize = wide;
                    state.cpu.data.esp = 0x12348000u;
                    state.memory[CPU_OUTER_GDT_BASE + 29u] = conforming_access[dpl];
                    cpu_outer_prepare_return_segments(&state, profiles[profile]);
                    lib_memory_copy(state.memory + CPU_OUTER_KERNEL_STACK_BASE + 0x8000u,
                        wide ? (const void *)frames32[form] : (const void *)frames16[form],
                        frame_bytes);
                    if (!cpu_outer_return_step(&state, programs[form], 1u, &after) ||
                        state.delivered_exception.valid || after.data.eip != 0x10u ||
                        after.data.esp != 0x12341000u ||
                        !cpu_outer_conforming_user_code(&after.data.cs, dpl ? 3u : 0u) ||
                        !cpu_outer_user_stack(&after.data.ss) ||
                        after.data.ds.flagValid || !after.data.es.flagValid ||
                        after.data.es.selector != 0x001bu ||
                        (profiles[profile] >= CORE_MACHINE_CPU_PROFILE_80386 &&
                            (after.data.fs.flagValid || !after.data.gs.flagValid ||
                                after.data.gs.selector != 0x001bu))) return LIB_FALSE;
                }
            }
        }
    }
    return LIB_TRUE;
}

static lib_bool cpu_outer_nonconforming_dpl_rejection(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 programs[][1] = {{0xcbu},{0xcfu}};
    static const lib_u16 frames[][5] = {
        {0x10u,0x1bu,0x1000u,0x23u,0u},
        {0x10u,0x1bu,0x203u,0x1000u,0x23u}
    };
    lib_size profile;
    lib_size form;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        for (form = 0u; form != sizeof(programs) / sizeof(programs[0]); ++form) {
            cpu_instruction_fixture state;
            t_cpu before;
            t_cpu after;
            const lib_size words = form ? 5u : 4u;

            cpu_outer_return_prepare(&state, profiles[profile]);
            state.memory[CPU_OUTER_GDT_BASE + 29u] = 0x9au;
            lib_memory_copy(state.memory + CPU_OUTER_KERNEL_STACK_BASE + 0x8000u,
                frames[form], words * sizeof(frames[form][0]));
            before = state.cpu;
            if (!cpu_outer_return_step(&state, programs[form], 1u, &after)) return LIB_FALSE;
            core_machine_cpu_execution_refresh(&state.execution);
            after = state.cpu;
            if (state.fault.valid || !state.delivered_exception.valid ||
                state.delivered_exception.exception_mask != VCPUINS_EXCEPT_GP ||
                state.delivered_exception.exception_code != 0x0018u ||
                after.data.cs.selector != before.data.cs.selector ||
                after.data.ss.selector != before.data.ss.selector ||
                state.memory[CPU_OUTER_GDT_BASE + 29u] != 0x9au) return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool cpu_outer_iret_success(void)
{
    static const lib_u8 programs[][3] = {
        {0xcfu,0,0}, {0x67u,0xcfu,0}, {0x66u,0xcfu,0}, {0x66u,0x67u,0xcfu}
    };
    static const lib_u8 bytes[] = {1u,2u,2u,3u};
    static const lib_bool operand16[] = {LIB_FALSE,LIB_FALSE,LIB_TRUE,LIB_TRUE};
    static const lib_bool wide_stack[] = {LIB_FALSE,LIB_TRUE,LIB_FALSE,LIB_TRUE};
    static const lib_u32 frame32[] = {
        0x00000010u, 0x0000001bu, 0x00000203u, 0x00001000u, 0x00000023u
    };
    static const lib_u16 frame16[] = {0x0010u,0x001bu,0x0203u,0x1000u,0x0023u};
    const lib_u8 stack_big = 0x40u;
    lib_size form;

    for (form = 0u; form != sizeof(programs) / sizeof(programs[0]); ++form) {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;
        const lib_size frame_bytes = operand16[form] ? sizeof(frame16) : sizeof(frame32);
        lib_u8 frame_before[sizeof(frame32)] = {0};
        lib_u8 frame_after[sizeof(frame32)] = {0};

        cpu_outer_return_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.cs.seg.exec.defsize = LIB_TRUE;
        state.cpu.data.esp = 0x12348000u;
        if (wide_stack[form]) state.memory[CPU_OUTER_GDT_BASE + 38u] = stack_big;
        lib_memory_copy(state.memory + CPU_OUTER_KERNEL_STACK_BASE + 0x8000u,
            operand16[form] ? (const void *)frame16 : (const void *)frame32, frame_bytes);
        lib_memory_copy(frame_before,
            state.memory + CPU_OUTER_KERNEL_STACK_BASE + 0x8000u, frame_bytes);
        before = state.cpu;
        if (!cpu_outer_return_step(&state, programs[form], bytes[form], &after)) return LIB_FALSE;
        lib_memory_copy(frame_after,
            state.memory + CPU_OUTER_KERNEL_STACK_BASE + 0x8000u, frame_bytes);
        if (after.data.eip != 0x10u || after.data.eflags != 0x203u ||
            after.data.esp != (wide_stack[form] ? 0x1000u : 0x12341000u) ||
            !cpu_outer_user_code(&after.data.cs) || !cpu_outer_user_stack(&after.data.ss) ||
            after.data.ss.seg.data.big != wide_stack[form] ||
            after.data.eax != before.data.eax || after.data.ecx != before.data.ecx ||
            after.data.edx != before.data.edx || after.data.ebx != before.data.ebx ||
            after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi ||
            lib_memory_compare(frame_before, frame_after, frame_bytes) != 0) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cpu_outer_retf_success(void)
{
    static const lib_u8 retf[] = {0xcbu};
    static const lib_u32 frame[] = {
        0x00000010u, 0x0000001bu, 0x00001000u, 0x00000023u
    };
    cpu_instruction_fixture state;
    t_cpu before;
    t_cpu after;
    lib_u8 frame_before[sizeof(frame)] = {0};
    lib_u8 frame_after[sizeof(frame)] = {0};

    cpu_outer_return_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.cpu.data.cs.seg.exec.defsize = LIB_TRUE;
    state.cpu.data.esp = 0x12348000u;
    lib_memory_copy(state.memory + CPU_OUTER_KERNEL_STACK_BASE + 0x8000u,
        frame, sizeof(frame));
    lib_memory_copy(frame_before,
        state.memory + CPU_OUTER_KERNEL_STACK_BASE + 0x8000u, sizeof(frame_before));
    before = state.cpu;
    if (!cpu_outer_return_step(&state, retf, sizeof(retf), &after)) return LIB_FALSE;
    lib_memory_copy(frame_after,
        state.memory + CPU_OUTER_KERNEL_STACK_BASE + 0x8000u, sizeof(frame_after));
    return after.data.eip == 0x10u && after.data.eflags == before.data.eflags &&
        after.data.esp == 0x12341000u && cpu_outer_user_code(&after.data.cs) &&
        cpu_outer_user_stack(&after.data.ss) && after.data.eax == before.data.eax &&
        after.data.ecx == before.data.ecx && after.data.edx == before.data.edx &&
        after.data.ebx == before.data.ebx && after.data.ebp == before.data.ebp &&
        after.data.esi == before.data.esi && after.data.edi == before.data.edi &&
        lib_memory_compare(frame_before, frame_after, sizeof(frame)) == 0;
}

static lib_bool cpu_outer_retf_immediate(void)
{
    static const lib_u8 retf[] = {0xcau,0x04u,0x00u};
    static const lib_u32 frame[] = {
        0x00000010u,0x0000001bu,0u,0x00001000u,0x00000023u
    };
    cpu_instruction_fixture state;
    t_cpu after;

    cpu_outer_return_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.cpu.data.cs.seg.exec.defsize = LIB_TRUE;
    state.cpu.data.esp = 0x12348000u;
    lib_memory_copy(state.memory + CPU_OUTER_KERNEL_STACK_BASE + 0x8000u,
        frame, sizeof(frame));
    if (!cpu_outer_return_step(&state, retf, sizeof(retf), &after)) return LIB_FALSE;
    return after.data.eip == 0x10u && after.data.esp == 0x12341004u &&
        cpu_outer_user_code(&after.data.cs) && cpu_outer_user_stack(&after.data.ss);
}

static lib_bool cpu_outer_iret_nonpresent_stack(void)
{
    static const lib_u8 iret[] = {0xcfu};
    static const lib_u32 frame[] = {
        0x00000010u, 0x0000001bu, 0x00000203u, 0x00001000u, 0x00000033u
    };
    cpu_instruction_fixture state;
    t_cpu before;
    t_cpu after;
    lib_u8 frame_before[sizeof(frame)] = {0};
    lib_u8 frame_after[sizeof(frame)] = {0};

    cpu_outer_return_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.cpu.data.cs.seg.exec.defsize = LIB_TRUE;
    state.cpu.data.esp = 0x12348000u;
    lib_memory_copy(state.memory + CPU_OUTER_KERNEL_STACK_BASE + 0x8000u,
        frame, sizeof(frame));
    lib_memory_copy(frame_before,
        state.memory + CPU_OUTER_KERNEL_STACK_BASE + 0x8000u, sizeof(frame_before));
    before = state.cpu;
    const lib_bool completed = cpu_outer_return_step(&state, iret, sizeof(iret), &after);
    if (completed) {
        core_machine_cpu_execution_refresh(&state.execution);
        after = state.cpu;
    }
    lib_memory_copy(frame_after,
        state.memory + CPU_OUTER_KERNEL_STACK_BASE + 0x8000u, sizeof(frame_after));
    if (!(!state.fault.valid &&
        state.delivered_exception.valid &&
        (state.delivered_exception.exception_mask & VCPUINS_EXCEPT_SS) != 0u &&
        after.data.eip == 0x121u && after.data.esp == before.data.esp - 8u &&
        after.data.cs.selector == before.data.cs.selector &&
        after.data.ss.selector == before.data.ss.selector &&
        lib_memory_compare(frame_before, frame_after, sizeof(frame)) == 0)) {
        return LIB_FALSE;
    }
    return LIB_TRUE;
}

typedef struct cpu_outer_error_case {
    lib_u16 cs;
    lib_u16 ss;
    lib_u16 error_code;
    lib_u32 exception_mask;
    lib_u16 handler_eip;
} cpu_outer_error_case;

static lib_bool cpu_outer_error_delivery(void)
{
    static const cpu_outer_error_case cases[] = {
        {0x002bu,0x0023u,0x0028u,VCPUINS_EXCEPT_NP,0x0111u},
        {0x001bu,0x0033u,0x0030u,VCPUINS_EXCEPT_SS,0x0121u},
        {0x003bu,0x0023u,0x0038u,VCPUINS_EXCEPT_GP,0x0101u}
    };
    static const lib_u8 programs[][1] = {{0xcbu},{0xcfu}};
    lib_size form;
    lib_size index;

    for (form = 0u; form != sizeof(programs) / sizeof(programs[0]); ++form) {
        for (index = 0u; index != sizeof(cases) / sizeof(cases[0]); ++index) {
            const cpu_outer_error_case *const test = &cases[index];
            cpu_instruction_fixture state;
            t_cpu before;
            t_cpu after;
            lib_u16 retf_frame[] = {0x0010u,test->cs,0x1000u,test->ss};
            lib_u16 iret_frame[] = {0x0010u,test->cs,0x0202u,0x1000u,test->ss};
            const lib_size frame_bytes = form == 0u ? sizeof(retf_frame) :
                sizeof(iret_frame);
            const void *const frame = form == 0u ? (const void *)retf_frame :
                (const void *)iret_frame;
            lib_u8 frame_before[sizeof(iret_frame)] = {0};
            lib_u8 frame_after[sizeof(iret_frame)] = {0};

            cpu_outer_return_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286);
            lib_memory_copy(state.memory + CPU_OUTER_KERNEL_STACK_BASE + 0x8000u,
                frame, frame_bytes);
            lib_memory_copy(frame_before,
                state.memory + CPU_OUTER_KERNEL_STACK_BASE + 0x8000u, frame_bytes);
            before = state.cpu;
            if (!cpu_outer_return_step(&state, programs[form], sizeof(programs[form]),
                    &after)) return LIB_FALSE;
            core_machine_cpu_execution_refresh(&state.execution);
            after = state.cpu;
            lib_memory_copy(frame_after,
                state.memory + CPU_OUTER_KERNEL_STACK_BASE + 0x8000u, frame_bytes);
            if (state.fault.valid || !state.delivered_exception.valid ||
                state.delivered_exception.exception_mask != test->exception_mask ||
                state.delivered_exception.exception_code != test->error_code ||
                after.data.eip != test->handler_eip ||
                after.data.cs.selector != before.data.cs.selector ||
                after.data.ss.selector != before.data.ss.selector ||
                lib_memory_compare(frame_before, frame_after, frame_bytes) != 0)
                return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool cpu_outer_iret_rejections(void)
{
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const lib_u8 attributes[][3] = {
        {0x66u,0xcfu,0u}, {0x67u,0xcfu,0u}, {0x66u,0x67u,0xcfu}
    };
    static const lib_u8 lock_forms[][4] = {
        {0xf0u,0xcfu,0u,0u}, {0xf0u,0x66u,0xcfu,0u},
        {0xf0u,0x67u,0xcfu,0u}, {0xf0u,0x66u,0x67u,0xcfu}
    };
    lib_size index;
    lib_size form;

    for (index = 0u; index != sizeof(legacy) / sizeof(legacy[0]); ++index) {
        for (form = 0u; form != sizeof(attributes) / sizeof(attributes[0]); ++form) {
            cpu_instruction_fixture state;
            const lib_u8 bytes = form == 2u ? 3u : 2u;

            cpu_instruction_prepare(&state, legacy[index]);
            if (!cpu_instruction_expect_real_fault(&state, attributes[form],
                    bytes, 6u))
                return LIB_FALSE;
        }
    }
    for (form = 0u; form != sizeof(lock_forms) / sizeof(lock_forms[0]); ++form) {
        cpu_instruction_fixture state;
        const lib_u8 bytes = form == 0u ? 2u : form == 3u ? 4u : 3u;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        if (!cpu_instruction_expect_real_fault(&state, lock_forms[form],
                bytes, 6u))
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

int main(void)
{
    if (!cpu_outer_iret_success()) return 1;
    if (!cpu_outer_retf_success()) return 2;
    if (!cpu_outer_retf_immediate()) return 3;
    if (!cpu_outer_iret_nonpresent_stack()) return 4;
    if (!cpu_outer_error_delivery()) return 5;
    if (!cpu_outer_iret_rejections()) return 6;
    if (!cpu_outer_conforming_returns()) return 7;
    if (!cpu_outer_nonconforming_dpl_rejection()) return 8;
    lib_c_printf("%s\n", "M5:T539:S54:CPU-OUTER-RETURN:OK");
    return 0;
}
