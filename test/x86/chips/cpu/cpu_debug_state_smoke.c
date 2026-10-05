#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

static void debug_state_enter_protected(cpu_instruction_fixture *fixture,
    lib_u8 cpl, lib_bool vm86)
{
    t_cpu *cpu = &fixture->cpu;

    cpu->data.cr0 |= VCPU_CR0_PE;
    cpu->data.cs.selector = (lib_u16)(0x0008u | cpl);
    cpu->data.cs.base = 0u;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.dpl = cpl;
    cpu->data.cs.flagValid = LIB_TRUE;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.seg.executable = LIB_TRUE;
    if (vm86) {
        cpu->data.eflags |= VCPU_EFLAGS_VM;
        cpu->data.cs.dpl = 3u;
    }
}

static lib_i32 debug_state_same(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi &&
        before->data.eflags == after->data.eflags &&
        before->data.dr0 == after->data.dr0 &&
        before->data.dr1 == after->data.dr1 &&
        before->data.dr2 == after->data.dr2 &&
        before->data.dr3 == after->data.dr3 &&
        before->data.dr6 == after->data.dr6 &&
        before->data.dr7 == after->data.dr7;
}

static lib_u32 *debug_state_dr(t_cpu *cpu, lib_u8 index)
{
    switch (index) {
    case 0: return &cpu->data.dr0;
    case 1: return &cpu->data.dr1;
    case 2: return &cpu->data.dr2;
    case 3: return &cpu->data.dr3;
    case 6: return &cpu->data.dr6;
    case 7: return &cpu->data.dr7;
    default: return LIB_NULL;
    }
}

static lib_i32 debug_state_run(cpu_instruction_fixture *fixture,
    const lib_u8 *code, lib_u8 bytes, t_cpu *after)
{
    lib_u8 steps;

    lib_memory_copy(fixture->memory, code, bytes);
    for (steps = 0u; steps != 16u && fixture->cpu.data.eip < bytes &&
        !fixture->execution.stop_requested; ++steps)
        core_machine_cpu_execution_refresh(&fixture->execution);
    *after = fixture->cpu;
    return fixture->cpu.data.eip >= bytes && !fixture->execution.stop_requested &&
        !fixture->fault.valid;
}

static lib_i32 debug_state_fault(cpu_instruction_fixture *fixture,
    const lib_u8 *code, lib_u8 bytes, lib_u32 exception, t_cpu *after)
{
    (void)cpu_instruction_run(fixture, code, bytes, after);
    if (fixture->execution.cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386 &&
        (fixture->cpu.data.cr0 & VCPU_CR0_PE) != 0u &&
        exception == VCPUINS_EXCEPT_GP) exception = VCPUINS_EXCEPT_DF;
    return (fixture->execution.stop_requested && fixture->fault.valid &&
        (fixture->fault.exception_mask & exception) != 0u) ||
        (fixture->instructions.data.except & exception) != 0u;
}

static lib_i32 debug_state_test_mov_dr(void)
{
    static const lib_u8 indices[] = {0u,1u,2u,3u,6u,7u};
    static const lib_u8 rejected[][3] = {
        {0x0fu,0x21u,0xe0u}, {0x0fu,0x23u,0xe8u},
        {0x0fu,0x21u,0x00u}, {0x0fu,0x23u,0x00u}
    };
    static const lib_u8 prefixes[][2] = {
        {0x66u,0u}, {0x67u,0u}, {0x66u,0x67u}
    };
    static const lib_u8 lock[][4] = {
        {0xf0u,0x0fu,0x21u,0xc0u}, {0xf0u,0x0fu,0x23u,0xc1u}
    };
    lib_size index;

    for (index = 0u; index != 6u; ++index) {
        cpu_instruction_fixture fixture;
        lib_u8 code[] = {0x0fu,0x23u,(lib_u8)(0xc1u | (indices[index] << 3u)),
            0x0fu,0x21u,(lib_u8)(0xc0u | (indices[index] << 3u))};
        t_cpu before, after;
        const lib_u32 value = 0x10203040u + (lib_u32)indices[index];

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        fixture.cpu.data.ecx = value;
        fixture.cpu.data.eax = 0xaabbccddu;
        before = fixture.cpu;
        if (!debug_state_run(&fixture, code, sizeof(code), &after) ||
            after.data.eip != sizeof(code) || after.data.eax != value ||
            *debug_state_dr(&after, indices[index]) != value ||
            after.data.ecx != before.data.ecx || after.data.eflags !=
            before.data.eflags) return 0;
    }
    for (index = 0u; index != 4u; ++index) {
        cpu_instruction_fixture fixture;
        t_cpu before, after;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        debug_state_enter_protected(&fixture, 0u, LIB_FALSE);
        fixture.cpu.data.eax = 0xaabbccddu;
        fixture.cpu.data.ecx = 0x11223344u;
        fixture.cpu.data.dr0 = 0x55667788u;
        fixture.cpu.data.dr7 = 0x99aabbccu;
        before = fixture.cpu;
        if (!debug_state_fault(&fixture, rejected[index], sizeof(rejected[index]),
                VCPUINS_EXCEPT_UD, &after) || !debug_state_same(&before,
                &after)) return 0;
    }
    for (index = 0u; index != 3u; ++index) {
        cpu_instruction_fixture fixture;
        lib_u8 code[8u] = {0u};
        const lib_u8 prefix_bytes = prefixes[index][1] == 0u ? 1u : 2u;
        t_cpu before, after;

        code[0] = prefixes[index][0];
        if (prefix_bytes == 2u) code[1] = prefixes[index][1];
        code[prefix_bytes] = 0x0fu;
        code[prefix_bytes + 1u] = 0x23u;
        code[prefix_bytes + 2u] = 0xc1u;
        code[prefix_bytes + 3u] = 0x0fu;
        code[prefix_bytes + 4u] = 0x21u;
        code[prefix_bytes + 5u] = 0xc0u;
        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        fixture.cpu.data.ecx = 0x5a5aa5a5u;
        before = fixture.cpu;
        if (!debug_state_run(&fixture, code, prefix_bytes + 6u, &after) ||
            after.data.eax != before.data.ecx || after.data.dr0 !=
            before.data.ecx || after.data.eflags != before.data.eflags) return 0;
    }
    for (index = 0u; index != 2u; ++index) {
        cpu_instruction_fixture fixture;
        static const lib_u8 mov_dr[] = {0x0fu,0x21u,0xc0u};
        t_cpu before, after;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        debug_state_enter_protected(&fixture, 3u, index != 0u);
        fixture.cpu.data.eax = 0xaabbccddu;
        fixture.cpu.data.dr0 = 0x55667788u;
        before = fixture.cpu;
        if (!debug_state_fault(&fixture, mov_dr, sizeof(mov_dr),
                VCPUINS_EXCEPT_GP, &after) || !debug_state_same(&before,
                &after)) return 0;
    }
    for (index = 0u; index != 2u; ++index) {
        cpu_instruction_fixture fixture;
        t_cpu before, after;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        debug_state_enter_protected(&fixture, 0u, LIB_FALSE);
        fixture.cpu.data.eax = 0xaabbccddu;
        fixture.cpu.data.ecx = 0x11223344u;
        fixture.cpu.data.dr0 = 0x55667788u;
        before = fixture.cpu;
        if (!debug_state_fault(&fixture, lock[index], sizeof(lock[index]),
                VCPUINS_EXCEPT_UD, &after) || !debug_state_same(&before,
                &after)) return 0;
    }
    return 1;
}

static lib_i32 debug_state_test_debug_exceptions(void)
{
    static const lib_u8 nop[] = {0x90u};
    {
        cpu_instruction_fixture fixture;
        t_cpu after;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        fixture.cpu.data.dr0 = 0u;
        fixture.cpu.data.dr7 = 0x00000001u;
        if (cpu_instruction_run(&fixture, nop, sizeof(nop), &after) !=
                LIB_STATUS_OK || fixture.fault.valid ||
            (after.data.dr6 & 1u) == 0u)
            return 0;
    }
    {
        cpu_instruction_fixture fixture;
        t_cpu after;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        fixture.cpu.data.eflags = VCPU_EFLAGS_RF | VCPU_EFLAGS_IF;
        fixture.cpu.data.dr0 = 0u;
        fixture.cpu.data.dr7 = 0x00000001u;
        if (cpu_instruction_run(&fixture, nop, sizeof(nop), &after) !=
                LIB_STATUS_OK || fixture.fault.valid ||
            (after.data.eflags & VCPU_EFLAGS_RF) != 0u || after.data.dr6 != 0u)
            return 0;
    }
    return 1;
}

static lib_i32 debug_state_test_data_breakpoints(void)
{
    static const lib_u8 read[] = {0xa1u,0x00u,0x10u};
    static const lib_u8 write[] = {0xc6u,0x06u,0x00u,0x10u,0x5au};
    static const lib_u8 source[] = {0x5au,0x34u,0x56u,0x78u};
    cpu_instruction_fixture fixture;
    t_cpu after;

    cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
    lib_memory_copy(fixture.memory + 0x1000u, source, sizeof(source));
    fixture.cpu.data.dr0 = 0x1001u;
    fixture.cpu.data.dr1 = 0x1001u;
    fixture.cpu.data.dr7 = 0x00ff0008u;
    if (cpu_instruction_run(&fixture, read, sizeof(read), &after) !=
            LIB_STATUS_OK || fixture.fault.valid ||
        (after.data.dr6 & 3u) != 2u || (after.data.eax & 0xffffu) != 0x345au)
        return 0;

    cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
    fixture.cpu.data.eflags = VCPU_EFLAGS_TF | VCPU_EFLAGS_IF |
        VCPU_EFLAGS_CF;
    fixture.cpu.data.dr0 = 0x1000u;
    fixture.cpu.data.dr7 = 0x00010101u;
    if (cpu_instruction_run(&fixture, write, sizeof(write), &after) !=
            LIB_STATUS_OK || fixture.fault.valid ||
        (after.data.dr6 & (1u | 0x00004000u)) != (1u | 0x00004000u) ||
        fixture.memory[0x1000u] != 0x5au) return 0;
    return 1;
}

int main(void)
{
    if (!debug_state_test_mov_dr()) {
        lib_c_printf("debug-state stage=mov-dr\n");
        return 1;
    }
    if (!debug_state_test_debug_exceptions()) {
        lib_c_printf("debug-state stage=exceptions\n");
        return 1;
    }
    if (!debug_state_test_data_breakpoints()) {
        lib_c_printf("debug-state stage=data-breakpoints\n");
        return 1;
    }
    lib_c_printf("M5:T539:S46:DEBUG-STATE:OK\n");
    return 0;
}
