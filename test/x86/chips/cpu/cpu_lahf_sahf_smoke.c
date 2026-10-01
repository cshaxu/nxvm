#include "lib/types/types_interface.h"
#include <stdio.h>
#include "support/cpu_instruction_fixture.h"

#define LAHF_SAHF_MASK (VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF | \
    VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF)
#define LAHF_SAHF_IGNORED_AH_BITS 0x2au

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
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 lahf = 0x9fu;
    static const lib_u8 sahf = 0x9eu;
    static const lib_u32 transfer_values[] = {0u, LAHF_SAHF_MASK};

    for (lib_size profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        for (lib_size transfer = 0u; transfer != sizeof(transfer_values) /
            sizeof(transfer_values[0]); ++transfer) {
            cpu_instruction_fixture state;
            t_cpu before;
            t_cpu after = {0};
            lib_u32 flags = VCPU_EFLAGS_IF | VCPU_EFLAGS_DF |
                VCPU_EFLAGS_OF | transfer_values[transfer];

            cpu_instruction_prepare(&state, profiles[profile]);
            lahf_sahf_seed(&state);
            state.cpu.data.eflags = flags;
            before = state.cpu;
            if (!lahf_sahf_run(&state, &lahf, sizeof(lahf), &after) ||
                after.data.eip != 1u ||
                !lahf_sahf_nonparticipants_same(&before, &after) ||
                after.data.eax != ((before.data.eax & 0xffff00ffu) |
                    ((transfer_values[transfer] | 0x02u) << 8u)) ||
                after.data.eflags != flags) return 0;

            cpu_instruction_prepare(&state, profiles[profile]);
            lahf_sahf_seed(&state);
            state.cpu.data.eax = 0xaabb00ddU |
                ((transfer_values[transfer] |
                    LAHF_SAHF_IGNORED_AH_BITS) << 8u);
            before = state.cpu;
            if (!lahf_sahf_run(&state, &sahf, sizeof(sahf), &after) ||
                after.data.eip != 1u ||
                !lahf_sahf_nonparticipants_same(&before, &after) ||
                after.data.eax != before.data.eax ||
                (after.data.eflags & LAHF_SAHF_MASK) !=
                    transfer_values[transfer] ||
                (after.data.eflags & ~LAHF_SAHF_MASK) !=
                    (before.data.eflags & ~LAHF_SAHF_MASK)) return 0;
        }
    }
    return 1;
}

static lib_i32 lahf_sahf_expect_ud(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    cpu_instruction_fixture state;
    t_cpu before;
    t_cpu after = {0};

    cpu_instruction_prepare(&state, profile);
    lahf_sahf_seed(&state);
    state.cpu.data.idtr.limit = 0x17u;
    before = state.cpu;
    return cpu_instruction_run(&state, code, bytes, &after) ==
            LIB_STATUS_INTERNAL_ERROR &&
        state.fault.valid &&
        (state.fault.exception_mask & VCPUINS_EXCEPT_UD) != 0u &&
        after.data.eip == 0u &&
        lib_memory_compare(&before.data, &after.data,
            sizeof(before.data)) == 0;
}

static lib_i32 lahf_sahf_test_attributes(void)
{
    static const lib_u8 codes[][3] = {
        {0x66u,0x9fu,0u}, {0x67u,0x9fu,0u}, {0x66u,0x67u,0x9fu},
        {0x66u,0x9eu,0u}, {0x67u,0x9eu,0u}, {0x66u,0x67u,0x9eu}
    };
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const lib_u8 lock_lahf[] = {0xf0u,0x9fu};
    static const lib_u8 lock_sahf[] = {0xf0u,0x9eu};

    for (lib_size profile = 0u; profile < sizeof(legacy) / sizeof(legacy[0]);
        ++profile) {
        for (lib_size form = 0u; form < sizeof(codes) / sizeof(codes[0]);
            ++form) {
            if (!lahf_sahf_expect_ud(legacy[profile], codes[form],
                form == 2u || form == 5u ? 3u : 2u)) return 0;
        }
    }
    return lahf_sahf_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, lock_lahf,
        sizeof(lock_lahf)) &&
        lahf_sahf_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, lock_sahf,
            sizeof(lock_sahf));
}

static lib_i32 lahf_sahf_test_386_attributes(void)
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

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
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

static lib_i32 lahf_sahf_test_vm86(void)
{
    static const lib_u8 opcodes[] = {0x9fu,0x9eu};
    const lib_u32 flags = VCPU_EFLAGS_VM | VCPU_EFLAGS_IF |
        VCPU_EFLAGS_DF | VCPU_EFLAGS_OF | VCPU_EFLAGS_IOPL |
        LAHF_SAHF_MASK;

    for (lib_size opcode = 0u; opcode < sizeof(opcodes); ++opcode) {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after = {0};

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
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

static lib_i32 lahf_sahf_test_386_checksum_sequence(void)
{
    static const lib_u8 code[] = {0x9eu,0x66u,0xd1u,0xd3u,0x9fu};
    cpu_instruction_fixture state;

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    lahf_sahf_seed(&state);
    state.cpu.data.eax = 0x00000300u;
    state.cpu.data.ebx = 0x80000000u;
    state.cpu.data.eflags = VCPU_EFLAGS_IF;
    lib_memory_copy(state.memory, code, sizeof(code));
    for (lib_u8 step = 0u; step != 3u; ++step) {
        core_machine_cpu_execution_refresh(&state.execution);
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
    if (!lahf_sahf_test_default() ||
        !lahf_sahf_test_attributes() ||
        !lahf_sahf_test_386_attributes() ||
        !lahf_sahf_test_vm86() ||
        !lahf_sahf_test_386_checksum_sequence()) {
        fputs("M5:T539:S36:CPU-LAHF-SAHF:FAIL\n", stderr);
        return 1;
    }
    puts("M5:T316:S39:LAHF-SAHF:OK");
    puts("M5:T401:S36:LAHF-SAHF-PROFILES:OK");
    puts("M5:T539:S36:CPU-LAHF-SAHF:OK");
    return 0;
}
