#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "support/cpu_instruction_fixture.h"

typedef struct direct_flags_case {
    lib_u8 opcode;
    lib_u32 initial_bits;
} direct_flags_case;

static void direct_flags_seed(cpu_instruction_fixture *state)
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
    cpu->data.eflags = VCPU_EFLAGS_IF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF |
        VCPU_EFLAGS_OF;
}

static lib_i32 direct_flags_gprs_same(const t_cpu *before, const t_cpu *after)
{
    return after->data.eax == before->data.eax &&
        after->data.ecx == before->data.ecx &&
        after->data.edx == before->data.edx &&
        after->data.ebx == before->data.ebx &&
        after->data.esp == before->data.esp &&
        after->data.ebp == before->data.ebp &&
        after->data.esi == before->data.esi &&
        after->data.edi == before->data.edi;
}

static lib_u32 direct_flags_expected(lib_u8 opcode, lib_u32 flags)
{
    switch (opcode) {
    case 0xf5u: return flags ^ VCPU_EFLAGS_CF;
    case 0xf8u: return flags & ~VCPU_EFLAGS_CF;
    case 0xf9u: return flags | VCPU_EFLAGS_CF;
    case 0xfcu: return flags & ~VCPU_EFLAGS_DF;
    case 0xfdu: return flags | VCPU_EFLAGS_DF;
    default: return flags;
    }
}

static lib_i32 direct_flags_run(cpu_instruction_fixture *state,
    const lib_u8 *code, lib_u8 bytes, t_cpu *after)
{
    state->cpu.data.eip = 0u;
    return cpu_instruction_run(state, code, bytes, after) == LIB_STATUS_OK &&
        !state->fault.valid;
}

static lib_i32 direct_flags_test_default(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const direct_flags_case cases[] = {
        {0xf5u,0u}, {0xf5u,VCPU_EFLAGS_CF}, {0xf8u,VCPU_EFLAGS_CF},
        {0xf9u,0u}, {0xfcu,VCPU_EFLAGS_DF}, {0xfdu,0u}
    };

    for (lib_size profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        for (lib_size form = 0u; form < sizeof(cases) / sizeof(cases[0]);
            ++form) {
            cpu_instruction_fixture state;
            t_cpu before;
            t_cpu after = {0};
            lib_u32 flags = VCPU_EFLAGS_IF | VCPU_EFLAGS_PF |
                VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF |
                VCPU_EFLAGS_OF | cases[form].initial_bits;

            cpu_instruction_prepare(&state, profiles[profile]);
            direct_flags_seed(&state);
            state.cpu.data.eflags = flags;
            before = state.cpu;
            if (!direct_flags_run(&state, &cases[form].opcode, 1u, &after) ||
                after.data.eip != 1u ||
                !direct_flags_gprs_same(&before, &after) ||
                after.data.eflags != direct_flags_expected(
                    cases[form].opcode, before.data.eflags)) return 0;
        }
    }
    return 1;
}

static lib_i32 direct_flags_expect_ud(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    cpu_instruction_fixture state;
    t_cpu before;
    t_cpu after = {0};

    cpu_instruction_prepare(&state, profile);
    direct_flags_seed(&state);
    state.cpu.data.idtr.limit = 0x17u;
    before = state.cpu;
    if (cpu_instruction_run(&state, code, bytes, &after) !=
            LIB_STATUS_INTERNAL_ERROR ||
        !state.fault.valid ||
        !(state.fault.exception_mask & VCPUINS_EXCEPT_UD) ||
        after.data.eip != 0u ||
        lib_memory_compare(&before.data, &after.data,
            sizeof(before.data)) != 0) return 0;
    return 1;
}

static lib_i32 direct_flags_test_attributes(void)
{
    static const lib_u8 opcodes[] = {0xf5u,0xf8u,0xf9u,0xfcu,0xfdu};
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };

    for (lib_size profile = 0u; profile < sizeof(legacy) / sizeof(legacy[0]);
        ++profile) {
        for (lib_size opcode = 0u; opcode < sizeof(opcodes); ++opcode) {
            for (lib_u8 prefix = 0u; prefix != 3u; ++prefix) {
                lib_u8 code[] = {0x66u,opcodes[opcode],0u};
                lib_u8 bytes = prefix == 2u ? 3u : 2u;

                if (prefix == 1u) code[0] = 0x67u;
                if (prefix == 2u) {
                    code[1] = 0x67u;
                    code[2] = opcodes[opcode];
                }
                if (!direct_flags_expect_ud(legacy[profile], code, bytes))
                    return 0;
            }
        }
    }
    for (lib_size opcode = 0u; opcode < sizeof(opcodes); ++opcode) {
        lib_u8 code[] = {0xf0u,opcodes[opcode]};
        if (!direct_flags_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, code,
            sizeof(code))) return 0;
    }
    return 1;
}

static lib_i32 direct_flags_test_386_attributes(void)
{
    static const lib_u8 opcodes[] = {0xf5u,0xf8u,0xf9u,0xfcu,0xfdu};

    for (lib_size opcode = 0u; opcode < sizeof(opcodes); ++opcode) {
        for (lib_u8 prefix = 0u; prefix != 3u; ++prefix) {
            cpu_instruction_fixture state;
            t_cpu before;
            t_cpu after = {0};
            lib_u8 code[] = {0x66u,opcodes[opcode],0u};
            lib_u8 bytes = prefix == 2u ? 3u : 2u;

            if (prefix == 1u) code[0] = 0x67u;
            if (prefix == 2u) {
                code[1] = 0x67u;
                code[2] = opcodes[opcode];
            }
            cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
            direct_flags_seed(&state);
            before = state.cpu;
            if (!direct_flags_run(&state, code, bytes, &after) ||
                after.data.eip != bytes ||
                !direct_flags_gprs_same(&before, &after) ||
                after.data.eflags != direct_flags_expected(opcodes[opcode],
                    before.data.eflags)) return 0;
        }
    }
    return 1;
}

static lib_i32 direct_flags_test_vm86(void)
{
    static const lib_u8 opcodes[] = {0xf5u,0xf8u,0xf9u,0xfcu,0xfdu};
    const lib_u32 flags = VCPU_EFLAGS_VM | VCPU_EFLAGS_IF |
        VCPU_EFLAGS_CF | VCPU_EFLAGS_DF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF |
        VCPU_EFLAGS_OF | VCPU_EFLAGS_IOPL;

    for (lib_size opcode = 0u; opcode < sizeof(opcodes); ++opcode) {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after = {0};

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        direct_flags_seed(&state);
        state.cpu.data.cr0 |= VCPU_CR0_PE;
        state.cpu.data.eflags = flags;
        state.cpu.data.cs.dpl = 3u;
        state.cpu.data.ss.dpl = 3u;
        before = state.cpu;
        if (!direct_flags_run(&state, &opcodes[opcode], 1u, &after) ||
            after.data.eip != 1u ||
            !direct_flags_gprs_same(&before, &after) ||
            after.data.eflags != direct_flags_expected(opcodes[opcode],
                before.data.eflags)) return 0;
    }
    return 1;
}

static lib_i32 direct_flags_test_real_identity(void)
{
    static const lib_u8 code[] = {
        0xb8u,0x00u,0xf0u,0x50u,0x9du,0x9cu,0x58u,0xf4u
    };
    static const struct {
        core_machine_cpu_profile profile;
        lib_u16 known_mask;
        lib_u16 expected_image;
    } cases[] = {
        { CORE_MACHINE_CPU_PROFILE_8086, 0x0fd7u, 0x0002u },
        { CORE_MACHINE_CPU_PROFILE_8088, 0x0fd7u, 0x0002u },
        { CORE_MACHINE_CPU_PROFILE_80186, 0x0fd7u, 0x0002u },
        { CORE_MACHINE_CPU_PROFILE_80286, 0x7fd7u, 0x0002u },
        { CORE_MACHINE_CPU_PROFILE_80386, 0xffd7u, 0x7002u }
    };

    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]);
        ++index) {
        cpu_instruction_fixture state;
        lib_i32 failed = 0;

        cpu_instruction_prepare(&state, cases[index].profile);
        direct_flags_seed(&state);
        lib_memory_copy(state.memory, code, sizeof(code));
        for (lib_u8 step = 0u; step != 7u && !state.cpu.data.flagHalt;
            ++step) {
            core_machine_cpu_execution_refresh(&state.execution);
            failed |= state.execution.stop_requested;
        }
        if (failed || !state.cpu.data.flagHalt ||
            (state.cpu.data.eax & cases[index].known_mask) !=
                cases[index].expected_image ||
            (state.cpu.data.eflags & cases[index].known_mask) !=
                cases[index].expected_image) return 0;
    }
    return 1;
}

static lib_bool direct_flags_test_interrupt_privilege(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0,0,0x9au,0,0,
        0xffu,0xffu,0,0,0,0x92u,0,0
    };
    static const lib_u8 gate[] = {0u,1u,8u,0u,0u,0x86u,0u,0u};
    lib_size profile;
    lib_u8 mode, cpl, iopl, initial_if, set;
    lib_u32 failures = 0u, count = 0u;

    for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (mode = 0u; mode <= (profiles[profile] >= CORE_MACHINE_CPU_PROFILE_80286);
        ++mode)
    for (cpl = 0u; cpl < (mode ? 4u : 1u); ++cpl)
    for (iopl = 0u; iopl < (mode ? 4u : 1u); ++iopl)
    for (initial_if = 0u; initial_if < 2u; ++initial_if)
    for (set = 0u; set < 2u; ++set) {
        cpu_instruction_fixture fixture;
        const lib_u16 tss[] = {0u,0x9000u,0x10u};
        const lib_u8 opcode = set ? 0xfbu : 0xfau;
        const lib_u32 flags = 0x43u | ((lib_u32)iopl << 12u) |
            (initial_if ? VCPU_EFLAGS_IF : 0u);
        const lib_bool rejected = mode && cpl > iopl;
        lib_u16 saved_ip = 0xffffu, saved_flags = 0u;

        cpu_instruction_prepare(&fixture, profiles[profile]);
        fixture.cpu.data.sp = 0x8000u;
        fixture.cpu.data.eflags = flags;
        fixture.cpu.data.cs.base = 0x2000u;
        if (mode) {
            fixture.cpu.data.cr0 |= VCPU_CR0_PE;
            fixture.cpu.data.cs.dpl = cpl;
            fixture.cpu.data.cs.selector = (lib_u16)(8u | cpl);
            fixture.cpu.data.ss.dpl = cpl;
            fixture.cpu.data.gdtr.base = 0x300u;
            fixture.cpu.data.gdtr.limit = sizeof(gdt) - 1u;
            fixture.cpu.data.idtr.base = 0x400u;
            fixture.cpu.data.idtr.limit = 0x6fu;
            fixture.cpu.data.tr.flagValid = LIB_TRUE;
            fixture.cpu.data.tr.selector = 0x28u;
            fixture.cpu.data.tr.sregtype = SREG_TR;
            fixture.cpu.data.tr.base = 0x500u;
            fixture.cpu.data.tr.limit = 0x2bu;
            fixture.cpu.data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_16_BUSY;
            lib_memory_copy(fixture.memory + 0x300u, gdt, sizeof(gdt));
            lib_memory_copy(fixture.memory + 0x468u, gate, sizeof(gate));
            lib_memory_copy(fixture.memory + 0x500u, tss, sizeof(tss));
        }
        fixture.memory[0x2000u] = opcode;
        core_machine_cpu_execution_refresh(&fixture.execution);
        ++count;
        if (rejected) {
            lib_memory_copy(&saved_ip, fixture.memory + fixture.cpu.data.sp + 2u,
                sizeof(saved_ip));
            lib_memory_copy(&saved_flags, fixture.memory + fixture.cpu.data.sp + 6u,
                sizeof(saved_flags));
            if (fixture.execution.stop_requested || fixture.fault.valid ||
                !fixture.delivered_exception.valid ||
                fixture.delivered_exception.exception_mask != VCPUINS_EXCEPT_GP ||
                fixture.delivered_exception.exception_code != 0u ||
                fixture.cpu.data.eip != 0x100u || saved_ip != 0u ||
                saved_flags != flags || fixture.execution.interrupt_shadow !=
                    CPU_INTERRUPT_SHADOW_NONE) {
                if (failures < 4u) lib_c_printf("CLI/STI reject profile=%u cpl=%u iopl=%u stop=%u terminal=%x delivered=%u/%x/%x ip=%x sp=%x saved=%x/%x shadow=%u\n",
                    (unsigned)profiles[profile], cpl, iopl,
                    fixture.execution.stop_requested, fixture.fault.exception_mask,
                    fixture.delivered_exception.valid,
                    fixture.delivered_exception.exception_mask,
                    fixture.delivered_exception.exception_code,
                    fixture.cpu.data.eip, fixture.cpu.data.sp, saved_ip,
                    saved_flags, fixture.execution.interrupt_shadow);
                ++failures;
            }
        } else if (fixture.execution.stop_requested || fixture.fault.valid ||
            fixture.delivered_exception.valid || fixture.cpu.data.eip != 1u ||
            fixture.cpu.data.sp != 0x8000u || fixture.cpu.data.eflags !=
                (set ? flags | VCPU_EFLAGS_IF : flags & ~VCPU_EFLAGS_IF) ||
            fixture.execution.interrupt_shadow != (set ? CPU_INTERRUPT_SHADOW_INTR :
                CPU_INTERRUPT_SHADOW_NONE))
            ++failures;
    }
    lib_c_printf("CLI/STI privilege cases=%u failures=%u\n",
        (unsigned)count, (unsigned)failures);
    return failures == 0u;
}

int main(void)
{
    if (!direct_flags_test_interrupt_privilege()) return 1;
    if (!direct_flags_test_default()) {
        lib_c_fprintf(lib_c_stderr, "%s", "M5:T539:S36:CPU-DIRECT-FLAGS:DEFAULT:FAIL\n");
        return 1;
    }
    if (!direct_flags_test_attributes()) {
        lib_c_fprintf(lib_c_stderr, "%s", "M5:T539:S36:CPU-DIRECT-FLAGS:ATTRIBUTES:FAIL\n");
        return 1;
    }
    if (!direct_flags_test_386_attributes()) {
        lib_c_fprintf(lib_c_stderr, "%s", "M5:T539:S36:CPU-DIRECT-FLAGS:386-ATTRIBUTES:FAIL\n");
        return 1;
    }
    if (!direct_flags_test_vm86()) {
        lib_c_fprintf(lib_c_stderr, "%s", "M5:T539:S36:CPU-DIRECT-FLAGS:VM86:FAIL\n");
        return 1;
    }
    if (!direct_flags_test_real_identity()) {
        lib_c_fprintf(lib_c_stderr, "%s", "M5:T539:S36:CPU-DIRECT-FLAGS:REAL-IDENTITY:FAIL\n");
        return 1;
    }
    lib_c_printf("%s\n", "M5:T316:S40:DIRECT-FLAGS:OK");
    lib_c_printf("%s\n", "M5:T401:S42:DIRECT-FLAGS-PROFILES:OK");
    lib_c_printf("%s\n", "M5:T539:S36:CPU-DIRECT-FLAGS:OK");
    return 0;
}
