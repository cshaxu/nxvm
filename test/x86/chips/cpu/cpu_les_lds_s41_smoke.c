#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

/* T337_REAL_UD_TERMINAL_CPU_OWNER: rejection and shutdown remain distinct. */
static void les_lds_s41_seed(cpu_instruction_fixture *state)
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
    cpu->data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF |
        VCPU_EFLAGS_OF;
}

static lib_i32 les_lds_s41_gprs_same_except_eax(const t_cpu *before,
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

static lib_i32 les_lds_s41_read(cpu_instruction_fixture *state, lib_u32 physical,
    void *data, lib_u8 bytes)
{
    return cpu_instruction_read(state, physical, data, bytes,
        CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) == LIB_STATUS_OK;
}

static lib_i32 les_lds_s41_real_case(core_machine_cpu_profile profile,
    lib_u8 opcode, lib_u8 prefix, lib_u8 segment_prefix)
{
    static const lib_u8 pointer16[] = {0x44u,0x33u,0x34u,0x12u};
    static const lib_u8 pointer32[] = {0x44u,0x33u,0x22u,0x11u,0x34u,0x12u};
    cpu_instruction_fixture state;
    t_cpu before;
    t_cpu after;
    lib_status status;
    lib_u8 code[9] = {0};
    lib_u8 source[6] = {0};
    lib_u8 observed[6] = {0};
    lib_u8 count = 0u;
    lib_u8 operand32 = prefix == 0x66u || prefix == 0xc6u;
    lib_u8 address32 = prefix == 0x67u || prefix == 0xc6u;
    lib_u32 source_base = 0x10000u;
    lib_u32 physical;
    lib_u32 expected_eax;
    lib_u32 expected_eip;
    lib_i32 failed = 0;
    cpu_instruction_prepare(&state, profile);

    if (!failed) {
        les_lds_s41_seed(&state);
        state.cpu.data.ds.base = source_base;
        state.cpu.data.cs.base = 0u;
        state.cpu.data.ss.base = source_base + 0x2000u;
        state.cpu.data.es.base = source_base + 0x3000u;
        state.cpu.data.fs.base = source_base + 0x4000u;
        state.cpu.data.gs.base = source_base + 0x5000u;
        if (segment_prefix == 0x2eu)
            source_base = state.cpu.data.cs.base;
        else if (segment_prefix == 0x36u)
            source_base = state.cpu.data.ss.base;
        else if (segment_prefix == 0x26u)
            source_base = state.cpu.data.es.base;
        else if (segment_prefix == 0x64u)
            source_base = state.cpu.data.fs.base;
        else if (segment_prefix == 0x65u)
            source_base = state.cpu.data.gs.base;
        if (segment_prefix != 0u)
            code[count++] = segment_prefix;
        if (operand32)
            code[count++] = 0x66u;
        if (address32)
            code[count++] = 0x67u;
        code[count++] = opcode;
        code[count++] = address32 ? 0x05u : 0x06u;
        code[count++] = 0x00u;
        code[count++] = 0x10u;
        if (address32) {
            code[count++] = 0x00u;
            code[count++] = 0x00u;
        }
        lib_memory_copy(source, operand32 ? pointer32 : pointer16,
            operand32 ? sizeof(pointer32) : sizeof(pointer16));
        physical = source_base + 0x1000u;
        failed |= cpu_instruction_write(&state, physical, source, operand32 ? sizeof(pointer32) : sizeof(pointer16), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
        before = state.cpu;
        expected_eax = operand32 ? 0x11223344u :
            ((before.data.eax & 0xffff0000u) | 0x3344u);
        expected_eip = count;
        failed |= (status = cpu_instruction_run(&state, code, count, &after)) != LIB_STATUS_OK ||
            state.fault.valid || after.data.eip != expected_eip ||
            after.data.eax != expected_eax ||
            !les_lds_s41_gprs_same_except_eax(&before, &after) ||
            after.data.eflags != before.data.eflags ||
            (opcode == 0xc4u ? after.data.es.selector : after.data.ds.selector) !=
            0x1234u || (opcode == 0xc4u ? after.data.es.base : after.data.ds.base) !=
            0x12340u || (opcode == 0xc4u ? after.data.es.limit : after.data.ds.limit) !=
            0xffffu || !(opcode == 0xc4u ? after.data.es.flagValid :
            after.data.ds.flagValid) || (opcode == 0xc4u ? after.data.es.seg.executable :
            after.data.ds.seg.executable) || !(opcode == 0xc4u ?
            after.data.es.seg.data.writable : after.data.ds.seg.data.writable) ||
            lib_memory_compare(&before.data.cs, &after.data.cs, sizeof(before.data.cs)) != 0 ||
            lib_memory_compare(&before.data.ss, &after.data.ss, sizeof(before.data.ss)) != 0 ||
            lib_memory_compare(opcode == 0xc4u ? &before.data.ds : &before.data.es,
            opcode == 0xc4u ? &after.data.ds : &after.data.es,
            sizeof(t_cpu_data_sreg)) != 0 || !les_lds_s41_read(&state, physical, observed,
            operand32 ? sizeof(pointer32) : sizeof(pointer16)) ||
            lib_memory_compare(source, observed, operand32 ? sizeof(pointer32) :
            sizeof(pointer16)) != 0;
    }

    return !failed;
}

static lib_i32 les_lds_s41_test_real(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 opcodes[] = {0xc4u,0xc5u};
    lib_u8 profile;
    lib_u8 opcode;
    lib_u8 segment;
    static const lib_u8 prefixes[] = {0u,0x2eu,0x36u,0x26u,0x64u,0x65u};

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        for (opcode = 0u; opcode != sizeof(opcodes); ++opcode)
            if (!les_lds_s41_real_case(profiles[profile], opcodes[opcode], 0u,
                0u))
                return 0;
    }
    for (segment = 0u; segment != sizeof(prefixes); ++segment)
        if (!les_lds_s41_real_case(CORE_MACHINE_CPU_PROFILE_80386, 0xc4u, 0u,
            prefixes[segment]) || !les_lds_s41_real_case(
            CORE_MACHINE_CPU_PROFILE_80386, 0xc5u, 0u, prefixes[segment]))
            return 0;
    return les_lds_s41_real_case(CORE_MACHINE_CPU_PROFILE_80386, 0xc4u,
        0x66u, 0u) && les_lds_s41_real_case(CORE_MACHINE_CPU_PROFILE_80386,
        0xc4u, 0x67u, 0x36u) && les_lds_s41_real_case(
        CORE_MACHINE_CPU_PROFILE_80386, 0xc4u, 0xc6u, 0u) &&
        les_lds_s41_real_case(CORE_MACHINE_CPU_PROFILE_80386, 0xc5u,
        0x66u, 0u) && les_lds_s41_real_case(CORE_MACHINE_CPU_PROFILE_80386,
        0xc5u, 0x67u, 0x36u) && les_lds_s41_real_case(
        CORE_MACHINE_CPU_PROFILE_80386, 0xc5u, 0xc6u, 0u);
}

static lib_i32 les_lds_s41_expect_ud(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    cpu_instruction_fixture state;
    lib_i32 failed = 0;
    cpu_instruction_prepare(&state, profile);

    if (!failed) {
        les_lds_s41_seed(&state);
        failed |= !cpu_instruction_expect_real_fault(&state, code, bytes, 6u);
    }

    return !failed;
}

static lib_i32 les_lds_s41_test_rejections(void)
{
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 opcodes[] = {0xc4u,0xc5u};
    lib_u8 profile;
    lib_u8 opcode;

    for (profile = 0u; profile != sizeof(legacy) / sizeof(legacy[0]);
        ++profile) {
        for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
            lib_u8 p66[] = {0x66u,opcodes[opcode],0x06u,0,0x10u};
            lib_u8 p67[] = {0x67u,opcodes[opcode],0x05u,0,0x10u,0,0};
            lib_u8 both[] = {0x66u,0x67u,opcodes[opcode],0x05u,0,0x10u,0,0};
            if (!les_lds_s41_expect_ud(legacy[profile], p66, sizeof(p66)) ||
                !les_lds_s41_expect_ud(legacy[profile], p67, sizeof(p67)) ||
                !les_lds_s41_expect_ud(legacy[profile], both, sizeof(both)))
                return 0;
        }
    }
    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
            lib_u8 reg[] = {opcodes[opcode],0xc0u};

            if (!les_lds_s41_expect_ud(profiles[profile], reg, sizeof(reg)))
                return 0;
        }
    }
    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        lib_u8 lock[] = {0xf0u,opcodes[opcode],0x06u,0,0x10u};

        if (!les_lds_s41_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, lock,
            sizeof(lock)))
            return 0;
    }
    return 1;
}

static lib_i32 les_lds_s41_boot_protected(cpu_instruction_fixture *state)
{
    static const lib_u8 pointer[] = {0x3fu,0u,0u,0x03u,0u,0u};
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0, 0xffu,0xffu,0,0x40u,0,0x92u,0,0,
        0xffu,0xffu,0,0x50u,0,0x12u,0,0, 0xffu,0xffu,0,0x60u,0,0x98u,0,0,
        0xffu,0xffu,0,0x70u,0,0x92u,0,0, 0xffu,0xffu,0,0x80u,0,0x92u,0,0
    };
    static const lib_u8 boot[] = {
        0x0fu,0x01u,0x16u,0,1u, 0xb8u,1u,0,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0,0x8eu,0xd8u, 0xb8u,0x18u,0,0x8eu,0xc0u,
        0xb8u,0x10u,0,0x8eu,0xd0u,0xbcu,0,0x80u, 0xeau,0,0,8u,0
    };

    lib_memory_copy(state->memory + 0x100u, pointer, sizeof(pointer));
    lib_memory_copy(state->memory + 0x300u, gdt, sizeof(gdt));
    lib_memory_copy(state->memory, boot, sizeof(boot));
    for (lib_u8 step = 0u; step != 11u; ++step) {
        core_machine_cpu_execution_refresh(&state->execution);
        if (state->execution.stop_requested || state->fault.valid) return 0;
    }
    return state->cpu.data.cs.selector == 8u && state->cpu.data.eip == 0u;
}

static lib_i32 les_lds_s41_protected_case(lib_u8 opcode, lib_u16 selector,
    lib_i32 expect_fault, lib_i32 null_selector)
{
    cpu_instruction_fixture state;
    t_cpu before;
    t_cpu after;
    lib_u8 pointer[] = {0x44u,0x33u,0,0};
    lib_u8 source[4] = {0x44u,0x33u,0,0};
    lib_u8 observed[4] = {0};
    lib_u8 program[4] = {opcode,0x06u,0x10u,0u};
    lib_u8 access = 0u;
    lib_i32 failed = 0;
    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    pointer[2] = (lib_u8)selector;
    pointer[3] = (lib_u8)(selector >> 8u);
    source[2] = pointer[2];
    source[3] = pointer[3];
    if (!failed)
        failed |= !les_lds_s41_boot_protected(&state);
    if (!failed) {
        const t_cpu_data_sreg original = opcode == 0xc4u ?
            state.cpu.data.es : state.cpu.data.ds;

        les_lds_s41_seed(&state);
        state.cpu.data.cs.base = 0x2000u;
        state.cpu.data.ds.base = 0x3000u;
        state.cpu.data.ds.limit = 0xffffu;
        state.cpu.data.ss.base = 0x3000u;
        state.cpu.data.es = original;
        failed |= cpu_instruction_write(&state, 0x3010u, pointer, sizeof(pointer), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK || cpu_instruction_write(&state, 0x2000u, program, sizeof(program), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
        state.cpu.data.eip = 0u;
        before = state.cpu;
        core_machine_cpu_execution_refresh(&state.execution);
        failed |= state.execution.stop_requested;
        after = state.cpu;
        failed |= !les_lds_s41_read(&state, 0x3010u, observed,
            sizeof(observed)) || lib_memory_compare(source, observed,
            sizeof(source)) != 0;
        if (expect_fault) {
            failed |= !core_machine_cpu_is_shutdown(&state.execution) || state.fault.valid ||
                !state.delivered_exception.valid ||
                state.delivered_exception.exception_mask != VCPUINS_EXCEPT_SHUTDOWN ||
                after.data.eip != before.data.eip ||
                after.data.eax != before.data.eax ||
                !les_lds_s41_gprs_same_except_eax(&before, &after) ||
                after.data.eflags != before.data.eflags ||
                lib_memory_compare(opcode == 0xc4u ? &before.data.es : &before.data.ds,
                opcode == 0xc4u ? &after.data.es : &after.data.ds,
                sizeof(t_cpu_data_sreg)) != 0 || lib_memory_compare(&before.data.cs,
                &after.data.cs, sizeof(before.data.cs)) != 0 ||
                lib_memory_compare(&before.data.ss, &after.data.ss, sizeof(before.data.ss)) != 0 ||
                lib_memory_compare(opcode == 0xc4u ? &before.data.ds : &before.data.es,
                opcode == 0xc4u ? &after.data.ds : &after.data.es,
                sizeof(t_cpu_data_sreg)) != 0;
        } else if (null_selector) {
            failed |= state.execution.stop_requested ||
                state.fault.valid || after.data.eip != 4u ||
                after.data.eax != 0xaabb3344u ||
                !les_lds_s41_gprs_same_except_eax(&before, &after) ||
                after.data.eflags != before.data.eflags ||
                (opcode == 0xc4u ? after.data.es.selector : after.data.ds.selector) !=
                0u || (opcode == 0xc4u ? after.data.es.flagValid :
                after.data.ds.flagValid) || lib_memory_compare(&before.data.cs,
                &after.data.cs, sizeof(before.data.cs)) != 0 ||
                lib_memory_compare(&before.data.ss, &after.data.ss, sizeof(before.data.ss)) != 0 ||
                lib_memory_compare(opcode == 0xc4u ? &before.data.ds : &before.data.es,
                opcode == 0xc4u ? &after.data.ds : &after.data.es,
                sizeof(t_cpu_data_sreg)) != 0;
        } else {
            failed |= state.execution.stop_requested ||
                state.fault.valid || after.data.eip != 4u ||
                after.data.eax != 0xaabb3344u ||
                !les_lds_s41_gprs_same_except_eax(&before, &after) ||
                after.data.eflags != before.data.eflags ||
                (opcode == 0xc4u ? after.data.es.selector : after.data.ds.selector) !=
                0x18u || !(opcode == 0xc4u ? after.data.es.flagValid :
                after.data.ds.flagValid) || (opcode == 0xc4u ? after.data.es.base :
                after.data.ds.base) != 0x4000u || (opcode == 0xc4u ?
                after.data.es.limit : after.data.ds.limit) != 0xffffu ||
                (opcode == 0xc4u ? after.data.es.dpl : after.data.ds.dpl) != 0u ||
                (opcode == 0xc4u ? after.data.es.seg.executable :
                after.data.ds.seg.executable) || !(opcode == 0xc4u ?
                after.data.es.seg.data.writable : after.data.ds.seg.data.writable) ||
                lib_memory_compare(&before.data.cs, &after.data.cs, sizeof(before.data.cs)) != 0 ||
                lib_memory_compare(&before.data.ss, &after.data.ss, sizeof(before.data.ss)) != 0 ||
                lib_memory_compare(opcode == 0xc4u ? &before.data.ds : &before.data.es,
                opcode == 0xc4u ? &after.data.ds : &after.data.es,
                sizeof(t_cpu_data_sreg)) != 0 || !les_lds_s41_read(&state, 0x31du,
                &access, sizeof(access)) || access != 0x93u;
        }
    }

    return !failed;
}

static lib_i32 les_lds_s41_test_protected(void)
{
    static const lib_u8 opcodes[] = {0xc4u,0xc5u};
    lib_u8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        if (!les_lds_s41_protected_case(opcodes[opcode], 0x18u, 0, 0) ||
            !les_lds_s41_protected_case(opcodes[opcode], 0u, 0, 1) ||
            !les_lds_s41_protected_case(opcodes[opcode], 0x20u, 1, 0) ||
            !les_lds_s41_protected_case(opcodes[opcode], 0x28u, 1, 0) ||
            !les_lds_s41_protected_case(opcodes[opcode], 0x33u, 1, 0))
            return 0;
    }
    return 1;
}

static lib_i32 les_lds_s41_test_limit(void)
{
    static const lib_u8 opcodes[] = {0xc4u,0xc5u};
    lib_u8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;
        lib_u8 code[] = {opcodes[opcode],0x06u,0x10u,0u};
        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed)
            failed |= !les_lds_s41_boot_protected(&state);
        if (!failed) {
            les_lds_s41_seed(&state);
            state.cpu.data.cs.base = 0x2000u;
            state.cpu.data.ds.base = 0x3000u;
            state.cpu.data.ds.limit = 0x11u;
            failed |= cpu_instruction_write(&state, 0x2000u, code, sizeof(code), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            state.cpu.data.eip = 0u;
            before = state.cpu;
            core_machine_cpu_execution_refresh(&state.execution);
            failed |= !core_machine_cpu_is_shutdown(&state.execution) || state.execution.stop_requested;
            after = state.cpu;
            failed |= state.fault.valid || !state.delivered_exception.valid ||
                state.delivered_exception.exception_mask != VCPUINS_EXCEPT_SHUTDOWN ||
                after.data.eip != before.data.eip ||
                after.data.eax != before.data.eax ||
                !les_lds_s41_gprs_same_except_eax(&before, &after) ||
                after.data.eflags != before.data.eflags ||
                lib_memory_compare(opcodes[opcode] == 0xc4u ? &before.data.es :
                &before.data.ds, opcodes[opcode] == 0xc4u ? &after.data.es :
                &after.data.ds, sizeof(t_cpu_data_sreg)) != 0;
        }

        if (failed)
            return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!les_lds_s41_test_real()) {
        lib_c_printf("LES-LDS-S41 stage=real\n");
        return 1;
    }
    if (!les_lds_s41_test_rejections()) {
        lib_c_printf("LES-LDS-S41 stage=rejections\n");
        return 1;
    }
    if (!les_lds_s41_test_protected()) {
        lib_c_printf("LES-LDS-S41 stage=protected\n");
        return 1;
    }
    if (!les_lds_s41_test_limit()) {
        lib_c_printf("LES-LDS-S41 stage=limit\n");
        return 1;
    }
    lib_c_printf("M5:T316:S41:LES-LDS:OK\n");
    lib_c_printf("M5:T401:S33:LES-LDS-PROFILES:OK\n");
    return 0;
}
