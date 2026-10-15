#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "support/cpu_instruction_fixture.h"

#define LAHF_SAHF_MASK (VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF | \
    VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF)

static void lahf_sahf_seed(cpu_instruction_fixture *state)
{
    t_cpu *cpu = &state->cpu;

    cpu->data.eax = 0xaabbccddU;
    cpu->data.ecx = 0x11223344u;
    cpu->data.edx = 0x55667788u;
    cpu->data.ebx = 0x99aabbccU;
    cpu->data.esp = 0x8000u;
    cpu->data.ebp = 0x120u;
    cpu->data.esi = 0x10u;
    cpu->data.edi = 0x20u;
    cpu->data.eflags = VCPU_EFLAGS_IF | VCPU_EFLAGS_DF | VCPU_EFLAGS_OF;
}

static lib_i32 lahf_sahf_nonparticipants_same(const t_cpu *before,
    const t_cpu *after)
{
    return after->data.ecx == before->data.ecx &&
        after->data.edx == before->data.edx &&
        after->data.ebx == before->data.ebx &&
        after->data.esp == before->data.esp &&
        after->data.ebp == before->data.ebp &&
        after->data.esi == before->data.esi &&
        after->data.edi == before->data.edi;
}

static lib_i32 lahf_sahf_run(cpu_instruction_fixture *state,
    const lib_u8 *code, lib_u8 bytes, t_cpu *after)
{
    state->cpu.data.eip = 0u;
    return cpu_instruction_run(state, code, bytes, after) == LIB_STATUS_OK &&
        !state->fault.valid;
}

static lib_i32 lahf_sahf_test_default(void)
{
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_8086, X86_CORE_CPU_PROFILE_80186,
        X86_CORE_CPU_PROFILE_80286, X86_CORE_CPU_PROFILE_80386,
        X86_CORE_CPU_PROFILE_80486SX
    };
    static const lib_u8 lahf = 0x9fu;
    static const lib_u8 sahf = 0x9eu;

    for (lib_size profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        for (lib_u32 image = 0u; image < 256u; ++image) {
            cpu_instruction_fixture state;
            t_cpu before;
            t_cpu after = {0};
            const lib_u32 transfer = image & LAHF_SAHF_MASK;
            lib_u32 flags = VCPU_EFLAGS_IF | VCPU_EFLAGS_DF |
                VCPU_EFLAGS_OF | transfer;

            cpu_instruction_prepare(&state, profiles[profile]);
            lahf_sahf_seed(&state);
            state.cpu.data.eflags = flags;
            before = state.cpu;
            if (!lahf_sahf_run(&state, &lahf, sizeof(lahf), &after) ||
                after.data.eip != 1u ||
                !lahf_sahf_nonparticipants_same(&before, &after) ||
                after.data.eax != ((before.data.eax & 0xffff00ffu) |
                    ((transfer | 0x02u) << 8u)) ||
                after.data.eflags != flags) return 0;

            cpu_instruction_prepare(&state, profiles[profile]);
            lahf_sahf_seed(&state);
            state.cpu.data.eax = 0xaabb00ddU | (image << 8u);
            before = state.cpu;
            if (!lahf_sahf_run(&state, &sahf, sizeof(sahf), &after) ||
                after.data.eip != 1u ||
                !lahf_sahf_nonparticipants_same(&before, &after) ||
                after.data.eax != before.data.eax ||
                (after.data.eflags & LAHF_SAHF_MASK) !=
                    transfer ||
                (after.data.eflags & ~LAHF_SAHF_MASK) !=
                    (before.data.eflags & ~LAHF_SAHF_MASK)) return 0;
        }
    }
    return 1;
}

static lib_i32 lahf_sahf_expect_ud(x86_core_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    cpu_instruction_fixture state;

    cpu_instruction_prepare(&state, profile);
    lahf_sahf_seed(&state);
    return cpu_instruction_expect_real_fault(&state, code, bytes, 6u);
}

static lib_i32 lahf_sahf_test_attributes(void)
{
    static const lib_u8 codes[][3] = {
        {0x66u,0x9fu,0u}, {0x67u,0x9fu,0u}, {0x66u,0x67u,0x9fu},
        {0x66u,0x9eu,0u}, {0x67u,0x9eu,0u}, {0x66u,0x67u,0x9eu}
    };
    static const x86_core_cpu_profile legacy[] = {
        X86_CORE_CPU_PROFILE_8086, X86_CORE_CPU_PROFILE_80186,
        X86_CORE_CPU_PROFILE_80286
    };
    static const lib_u8 lock_lahf[] = {0xf0u,0x9fu};
    static const lib_u8 lock_sahf[] = {0xf0u,0x9eu};
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_80386, X86_CORE_CPU_PROFILE_80486SX
    };

    for (lib_size index = 0u; index < sizeof(legacy) / sizeof(legacy[0]);
        ++index) {
        for (lib_size form = 0u; form < sizeof(codes) / sizeof(codes[0]);
            ++form) {
            if (!lahf_sahf_expect_ud(legacy[index], codes[form],
                form == 2u || form == 5u ? 3u : 2u)) return 0;
        }
    }
    for (lib_size index = 0u; index < sizeof(profiles) / sizeof(profiles[0]);
        ++index) {
        if (!lahf_sahf_expect_ud(profiles[index], lock_lahf, sizeof(lock_lahf)) ||
            !lahf_sahf_expect_ud(profiles[index], lock_sahf, sizeof(lock_sahf)))
            return 0;
    }
    return 1;
}

static lib_i32 lahf_sahf_test_386_attributes(x86_core_cpu_profile profile)
{
    static const lib_u8 codes[][3] = {
        {0x66u,0x9fu,0u}, {0x67u,0x9fu,0u}, {0x66u,0x67u,0x9fu},
        {0x66u,0x9eu,0u}, {0x67u,0x9eu,0u}, {0x66u,0x67u,0x9eu}
    };

    for (lib_size form = 0u; form < sizeof(codes) / sizeof(codes[0]);
        ++form) {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after = {0};
        lib_u8 bytes = form == 2u || form == 5u ? 3u : 2u;

        cpu_instruction_prepare(&state, profile);
        lahf_sahf_seed(&state);
        if (form < 3u) state.cpu.data.eflags |= LAHF_SAHF_MASK;
        else state.cpu.data.eax = 0xaabb00ddU;
        before = state.cpu;
        if (!lahf_sahf_run(&state, codes[form], bytes, &after) ||
            after.data.eip != bytes ||
            !lahf_sahf_nonparticipants_same(&before, &after) ||
            (form >= 3u ? after.data.eax != before.data.eax ||
                (after.data.eflags & LAHF_SAHF_MASK) != 0u ||
                (after.data.eflags & ~LAHF_SAHF_MASK) !=
                    (before.data.eflags & ~LAHF_SAHF_MASK) :
                after.data.eax != ((before.data.eax & 0xffff00ffu) |
                    (((before.data.eflags & LAHF_SAHF_MASK) | 0x02u) << 8u)) ||
                after.data.eflags != before.data.eflags)) return 0;
    }
    return 1;
}

static lib_i32 lahf_sahf_test_vm86(x86_core_cpu_profile profile)
{
    static const lib_u8 opcodes[] = {0x9fu,0x9eu};
    const lib_u32 flags = VCPU_EFLAGS_VM | VCPU_EFLAGS_IF |
        VCPU_EFLAGS_DF | VCPU_EFLAGS_OF | VCPU_EFLAGS_IOPL |
        LAHF_SAHF_MASK;

    for (lib_size opcode = 0u; opcode < sizeof(opcodes); ++opcode) {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after = {0};

        cpu_instruction_prepare(&state, profile);
        lahf_sahf_seed(&state);
        state.cpu.data.cr0 |= VCPU_CR0_PE;
        state.cpu.data.eflags = flags;
        state.cpu.data.cs.dpl = 3u;
        state.cpu.data.ss.dpl = 3u;
        if (opcodes[opcode] == 0x9eu) state.cpu.data.eax = 0xaabb00ddU;
        before = state.cpu;
        if (!lahf_sahf_run(&state, &opcodes[opcode], 1u, &after) ||
            after.data.eip != 1u ||
            !lahf_sahf_nonparticipants_same(&before, &after) ||
            (opcodes[opcode] == 0x9eu ?
                after.data.eax != before.data.eax ||
                (after.data.eflags & LAHF_SAHF_MASK) != 0u ||
                (after.data.eflags & ~LAHF_SAHF_MASK) !=
                    (before.data.eflags & ~LAHF_SAHF_MASK) :
                after.data.eax != ((before.data.eax & 0xffff00ffu) |
                    ((LAHF_SAHF_MASK | 0x02u) << 8u)) ||
                after.data.eflags != before.data.eflags)) return 0;
    }
    return 1;
}

static lib_i32 lahf_sahf_test_386_checksum_sequence(x86_core_cpu_profile profile)
{
    static const lib_u8 code[] = {0x9eu,0x66u,0xd1u,0xd3u,0x9fu};
    cpu_instruction_fixture state;

    cpu_instruction_prepare(&state, profile);
    lahf_sahf_seed(&state);
    state.cpu.data.eax = 0x00000300u;
    state.cpu.data.ebx = 0x80000000u;
    state.cpu.data.eflags = VCPU_EFLAGS_IF;
    lib_memory_copy(state.memory, code, sizeof(code));
    for (lib_u8 step = 0u; step != 3u; ++step) {
        x86_core_cpu_execution_refresh(&state.execution);
        if (state.execution.stop_requested || state.fault.valid) return 0;
    }
    return state.cpu.data.eip == sizeof(code) &&
        state.cpu.data.eax == 0x00000300u &&
        state.cpu.data.ebx == 1u &&
        state.cpu.data.eflags ==
            (VCPU_EFLAGS_IF | VCPU_EFLAGS_CF | VCPU_EFLAGS_OF);
}

int main(void)
{
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_80386, X86_CORE_CPU_PROFILE_80486SX
    };
    lib_bool passed = lahf_sahf_test_default() && lahf_sahf_test_attributes();

    for (lib_size index = 0u; index < sizeof(profiles) / sizeof(profiles[0]);
        ++index) {
        passed &= lahf_sahf_test_386_attributes(profiles[index]) &&
            lahf_sahf_test_vm86(profiles[index]) &&
            lahf_sahf_test_386_checksum_sequence(profiles[index]);
    }
    if (!passed) {
        lib_c_fprintf(lib_c_stderr, "%s", "CPU-LAHF-SAHF:FAIL\n");
        return 1;
    }
    lib_c_printf("%s\n", "LAHF-SAHF:OK");
    lib_c_printf("%s\n", "LAHF-SAHF-PROFILES:OK");
    lib_c_printf("%s\n", "CPU-LAHF-SAHF:OK");
    return 0;
}
