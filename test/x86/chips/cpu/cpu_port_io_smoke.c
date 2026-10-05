#include "support/cpu_port_instruction_fixture.h"
#include "lib/types/file.h"
/* T337_REAL_UD_TERMINAL_CPU_OWNER: scalar port prefixes are CPU-owned. */

static void port_io_seed(t_cpu *cpu)
{
    cpu->data.eax = 0xa1a1b2b2u;
    cpu->data.ecx = 0xc3c3d4d4u;
    cpu->data.edx = 0xe5e500e0u;
    cpu->data.ebx = 0xf6f60707u;
    cpu->data.esp = 0x8000u;
    cpu->data.ebp = 0x08080909u;
    cpu->data.esi = 0x10101111u;
    cpu->data.edi = 0x12121313u;
    cpu->data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
}

static lib_i32 port_io_other_registers_same(const t_cpu *before,
    const t_cpu *after)
{
    return before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi &&
        lib_memory_compare(&before->data.es, &after->data.es,
            sizeof(before->data.es)) == 0 &&
        lib_memory_compare(&before->data.cs, &after->data.cs,
            sizeof(before->data.cs)) == 0 &&
        lib_memory_compare(&before->data.ss, &after->data.ss,
            sizeof(before->data.ss)) == 0 &&
        lib_memory_compare(&before->data.ds, &after->data.ds,
            sizeof(before->data.ds)) == 0 &&
        lib_memory_compare(&before->data.fs, &after->data.fs,
            sizeof(before->data.fs)) == 0 &&
        lib_memory_compare(&before->data.gs, &after->data.gs,
            sizeof(before->data.gs)) == 0;
}

static lib_i32 port_io_success(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes, lib_bool input, lib_u8 width,
    lib_u16 port)
{
    cpu_port_instruction_fixture state;
    t_cpu before, after;
    lib_u32 expected;

    cpu_port_prepare(&state, profile);
    port_io_seed(&state.instruction.cpu);
    state.input_value = 0x11223344u;
    before = state.instruction.cpu;
    if (cpu_instruction_run(&state.instruction, code, bytes, &after) !=
        LIB_STATUS_OK || state.instruction.fault.valid ||
        after.data.eip != bytes ||
        !port_io_other_registers_same(&before, &after) ||
        after.data.eflags != before.data.eflags ||
        state.transfer_count != 1u || state.complete_count != 1u ||
        state.transfers[0].port != port ||
        state.transfers[0].bytes != width ||
        state.transfers[0].write == input) return 0;
    if (input) {
        expected = width == 1u ? (before.data.eax & 0xffffff00u) |
            0x44u : width == 2u ? (before.data.eax & 0xffff0000u) |
            0x3344u : 0x11223344u;
        return after.data.eax == expected;
    }
    expected = width == 1u ? before.data.eax & 0xffu :
        width == 2u ? before.data.eax & 0xffffu : before.data.eax;
    return after.data.eax == before.data.eax &&
        state.transfers[0].value == expected;
}

static lib_i32 port_io_test_default_forms(void)
{
    static const lib_u8 codes[][2] = {
        {0xe4u,0x5au}, {0xe5u,0x5au}, {0xe6u,0x5au},
        {0xe7u,0x5au}, {0xecu,0u}, {0xedu,0u},
        {0xeeu,0u}, {0xefu,0u}
    };
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_u8 profile, form;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile)
        for (form = 0u; form != 8u; ++form)
            if (!port_io_success(profiles[profile], codes[form],
                form < 4u ? 2u : 1u, form < 2u || (form >= 4u && form < 6u),
                form % 2u == 0u ? 1u : 2u,
                form < 4u ? 0x005au : 0x00e0u)) return 0;
    return 1;
}

static lib_i32 port_io_test_386_attributes(void)
{
    static const lib_u8 opcodes[] = {
        0xe4u,0xe5u,0xe6u,0xe7u,0xecu,0xedu,0xeeu,0xefu
    };
    lib_u8 attribute, form;

    for (attribute = 0u; attribute != 3u; ++attribute)
        for (form = 0u; form != sizeof(opcodes); ++form) {
            lib_u8 code[4];
            lib_u8 prefix_bytes = attribute == 2u ? 2u : 1u;
            lib_u8 bytes = prefix_bytes + 1u + (form < 4u ? 1u : 0u);
            lib_u8 width = form % 2u == 0u ? 1u :
                attribute == 1u ? 2u : 4u;

            code[0] = attribute == 1u ? 0x67u : 0x66u;
            if (attribute == 2u) code[1] = 0x67u;
            code[prefix_bytes] = opcodes[form];
            if (form < 4u) code[prefix_bytes + 1u] = 0x5au;
            if (!port_io_success(CORE_MACHINE_CPU_PROFILE_80386, code,
                bytes, form < 2u || (form >= 4u && form < 6u), width,
                form < 4u ? 0x005au : 0x00e0u)) return 0;
        }
    return 1;
}

static lib_i32 port_io_test_provider_failure(void)
{
    static const lib_u8 in_code[] = {0xe4u,0x5au};
    static const lib_u8 out_code[] = {0xe7u,0x5au};
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_u8 profile, form;

    for (profile = 0u; profile != 2u; ++profile)
        for (form = 0u; form != 2u; ++form) {
            cpu_port_instruction_fixture state;
            t_cpu before, after;
            const lib_u8 *code = form == 0u ? in_code : out_code;

            cpu_port_prepare(&state, profiles[profile]);
            port_io_seed(&state.instruction.cpu);
            state.fail_status = LIB_STATUS_INVALID_ARGUMENT;
            before = state.instruction.cpu;
            (void)cpu_instruction_run(&state.instruction, code, 2u, &after);
            if (!state.instruction.fault.valid ||
                !(state.instruction.fault.exception_mask & VCPUINS_EXCEPT_CE) ||
                after.data.eip != before.data.eip ||
                after.data.eax != before.data.eax ||
                after.data.eflags != before.data.eflags ||
                !port_io_other_registers_same(&before, &after) ||
                state.transfer_count != 0u || state.complete_count != 0u)
                return 0;
        }
    return 1;
}

static lib_i32 port_io_test_vm86(void)
{
    static const lib_u8 code[] = {0xe4u,0x5au};
    cpu_port_instruction_fixture state;
    t_cpu before, after;

    cpu_port_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    port_io_seed(&state.instruction.cpu);
    state.instruction.cpu.data.cr0 |= VCPU_CR0_PE;
    state.instruction.cpu.data.eflags = VCPU_EFLAGS_VM |
        VCPU_EFLAGS_IOPL | VCPU_EFLAGS_IF;
    state.instruction.cpu.data.cs.dpl = 3u;
    state.instruction.cpu.data.ss.dpl = 3u;
    before = state.instruction.cpu;
    (void)cpu_instruction_run(&state.instruction, code, sizeof(code), &after);
    return state.instruction.fault.valid &&
        (state.instruction.fault.exception_mask & VCPUINS_EXCEPT_DF) &&
        after.data.eip == before.data.eip &&
        after.data.eax == before.data.eax &&
        after.data.eflags == before.data.eflags &&
        port_io_other_registers_same(&before, &after) &&
        state.transfer_count == 0u && state.complete_count == 0u;
}

static lib_i32 port_io_test_tss_iomap(void)
{
    static const lib_u8 code[] = {0xe4u,0xe0u};
    const lib_u16 iomap_base = 0x0080u;
    lib_u8 denied;

    for (denied = 0u; denied != 3u; ++denied) {
        cpu_port_instruction_fixture state;
        t_cpu before, after;
        lib_u8 bitmap = denied == 1u ? 0x01u : 0u;

        cpu_port_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        port_io_seed(&state.instruction.cpu);
        state.instruction.cpu.data.cr0 |= VCPU_CR0_PE;
        state.instruction.cpu.data.eflags = denied == 2u ?
            VCPU_EFLAGS_IF | VCPU_EFLAGS_IOPL : VCPU_EFLAGS_IF;
        state.instruction.cpu.data.cs.dpl = 3u;
        state.instruction.cpu.data.ss.dpl = 3u;
        state.instruction.cpu.data.tr.flagValid = LIB_TRUE;
        state.instruction.cpu.data.tr.selector = 0x0028u;
        state.instruction.cpu.data.tr.base = 0x0600u;
        state.instruction.cpu.data.tr.limit = 0x00ffu;
        state.instruction.cpu.data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
        state.input_value = 0x11223344u;
        lib_memory_copy(state.instruction.memory + 0x0666u,
            &iomap_base, sizeof(iomap_base));
        state.instruction.memory[0x069cu] = bitmap;
        before = state.instruction.cpu;
        (void)cpu_instruction_run(&state.instruction, code, sizeof(code),
            &after);
        if (denied == 1u) {
            if (!state.instruction.fault.valid ||
                !(state.instruction.fault.exception_mask & VCPUINS_EXCEPT_DF) ||
                after.data.eip != before.data.eip ||
                after.data.eax != before.data.eax ||
                state.transfer_count != 0u) return 0;
        }
        else if (state.instruction.fault.valid ||
            after.data.eip != sizeof(code) ||
            after.data.eax != ((before.data.eax & 0xffffff00u) | 0x44u) ||
            state.transfer_count != 1u || state.complete_count != 1u ||
            state.transfers[0].port != 0x00e0u) return 0;
        if (!port_io_other_registers_same(&before, &after) ||
            after.data.eflags != before.data.eflags) return 0;
    }
    return 1;
}

static lib_i32 port_io_expect_ud(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    cpu_port_instruction_fixture state;
    t_cpu before, after;

    cpu_port_prepare(&state, profile);
    port_io_seed(&state.instruction.cpu);
    state.instruction.cpu.data.idtr.limit = 0x17u;
    before = state.instruction.cpu;
    (void)cpu_instruction_run(&state.instruction, code, bytes, &after);
    return state.instruction.fault.valid &&
        (state.instruction.fault.exception_mask & VCPUINS_EXCEPT_UD) &&
        after.data.eip == before.data.eip &&
        after.data.eax == before.data.eax &&
        after.data.eflags == before.data.eflags &&
        port_io_other_registers_same(&before, &after) &&
        state.transfer_count == 0u && state.complete_count == 0u;
}

static lib_i32 port_io_test_rejections(void)
{
    static const lib_u8 opcodes[] = {
        0xe4u,0xe5u,0xe6u,0xe7u,0xecu,0xedu,0xeeu,0xefu
    };
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    lib_u8 profile, form;

    for (profile = 0u; profile != 3u; ++profile)
        for (form = 0u; form != 8u; ++form) {
            lib_u8 attr66[] = {0x66u,opcodes[form],0x5au};
            lib_u8 attr67[] = {0x67u,opcodes[form],0x5au};
            lib_u8 both[] = {0x66u,0x67u,opcodes[form],0x5au};
            lib_u8 bytes = form < 4u ? 3u : 2u;

            if (!port_io_expect_ud(legacy[profile], attr66, bytes) ||
                !port_io_expect_ud(legacy[profile], attr67, bytes) ||
                !port_io_expect_ud(legacy[profile], both,
                    (lib_u8)(bytes + 1u))) return 0;
        }
    for (form = 0u; form != 8u; ++form) {
        lib_u8 lock[] = {0xf0u,opcodes[form],0x5au};
        lib_u8 lock66[] = {0xf0u,0x66u,opcodes[form],0x5au};
        lib_u8 lock67[] = {0xf0u,0x67u,opcodes[form],0x5au};
        lib_u8 both[] = {0xf0u,0x66u,0x67u,opcodes[form],0x5au};
        lib_u8 bytes = form < 4u ? 3u : 2u;

        if (!port_io_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, lock, bytes) ||
            !port_io_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, lock66,
                (lib_u8)(bytes + 1u)) ||
            !port_io_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, lock67,
                (lib_u8)(bytes + 1u)) ||
            !port_io_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, both,
                (lib_u8)(bytes + 2u))) return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!port_io_test_default_forms() || !port_io_test_386_attributes() ||
        !port_io_test_provider_failure() || !port_io_test_vm86() ||
        !port_io_test_tss_iomap() || !port_io_test_rejections()) {
        lib_c_printf("Scalar port I/O CPU semantics failed\n");
        return 1;
    }
    lib_c_printf("M5:T316:S55:PORT-IO:OK\n");
    return 0;
}
