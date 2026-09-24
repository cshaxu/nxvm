#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"
#include "app-nxvm/devices/cpu.h"
#include "app-nxvm/devices/pic.h"
#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_cpu_fixture.h"

typedef struct les_lds_s41_machine {
    core_machine *machine;
} les_lds_s41_machine;

static void les_lds_s41_reset(void *opaque)
{
    les_lds_s41_machine *state = (les_lds_s41_machine *)opaque;

    if (state != LIB_NULL)
        (void)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider les_lds_s41_provider = {
    les_lds_s41_reset, LIB_NULL
};

static lib_i32 les_lds_s41_prepare(core_machine_cpu_profile profile,
    les_lds_s41_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    lib_memory_set(state, 0, sizeof(*state));
return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &les_lds_s41_provider, state, &state->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u);
}

static void les_lds_s41_seed(les_lds_s41_machine *state)
{
    t_cpu *cpu = &state->machine->executor_cpu;

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

static lib_i32 les_lds_s41_irq_gprs_same_except_eax(const t_cpu *before,
    const t_cpu *after)
{
    return after->data.ecx == before->data.ecx &&
        after->data.edx == before->data.edx &&
        after->data.ebx == before->data.ebx &&
        after->data.ebp == before->data.ebp &&
        after->data.esi == before->data.esi &&
        after->data.edi == before->data.edi;
}

static lib_i32 les_lds_s41_run(les_lds_s41_machine *state, const lib_u8 *code,
    lib_u8 bytes, t_cpu *after, core_machine_cpu_diagnostic *diagnostic,
    lib_status *status, core_machine_run_result *result)
{
    if (core_machine_memory_write(state->machine, 0u, code, bytes) !=
        LIB_STATUS_OK)
        return 0;
    *status = core_machine_run(state->machine,
        (core_machine_run_budget){1u,0u}, result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) ==
        LIB_STATUS_OK;
}

static lib_i32 les_lds_s41_read(les_lds_s41_machine *state, lib_u32 physical,
    void *data, lib_u8 bytes)
{
    return core_machine_memory_read_physical(&state->machine->executor_memory,
        physical, (lib_uptr)data, bytes) == LIB_STATUS_OK;
}

static lib_i32 les_lds_s41_real_case(core_machine_cpu_profile profile,
    lib_u8 opcode, lib_u8 prefix, lib_u8 segment_prefix)
{
    static const lib_u8 pointer16[] = {0x44u,0x33u,0x34u,0x12u};
    static const lib_u8 pointer32[] = {0x44u,0x33u,0x22u,0x11u,0x34u,0x12u};
    les_lds_s41_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
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
    lib_i32 failed = !les_lds_s41_prepare(profile, &state);

    if (!failed) {
        les_lds_s41_seed(&state);
        state.machine->executor_cpu.data.ds.base = source_base;
        state.machine->executor_cpu.data.cs.base = 0u;
        state.machine->executor_cpu.data.ss.base = source_base + 0x2000u;
        state.machine->executor_cpu.data.es.base = source_base + 0x3000u;
        state.machine->executor_cpu.data.fs.base = source_base + 0x4000u;
        state.machine->executor_cpu.data.gs.base = source_base + 0x5000u;
        if (segment_prefix == 0x2eu)
            source_base = state.machine->executor_cpu.data.cs.base;
        else if (segment_prefix == 0x36u)
            source_base = state.machine->executor_cpu.data.ss.base;
        else if (segment_prefix == 0x26u)
            source_base = state.machine->executor_cpu.data.es.base;
        else if (segment_prefix == 0x64u)
            source_base = state.machine->executor_cpu.data.fs.base;
        else if (segment_prefix == 0x65u)
            source_base = state.machine->executor_cpu.data.gs.base;
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
        failed |= core_machine_memory_write(state.machine, physical, source,
            operand32 ? sizeof(pointer32) : sizeof(pointer16)) != LIB_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        expected_eax = operand32 ? 0x11223344u :
            ((before.data.eax & 0xffff0000u) | 0x3344u);
        expected_eip = count;
        failed |= !les_lds_s41_run(&state, code, count, &after, &diagnostic,
            &status, &result) || status != LIB_STATUS_OK ||
            diagnostic.first_fault.valid || after.data.eip != expected_eip ||
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
    core_machine_destroy(state.machine);
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
    les_lds_s41_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    lib_status status;
    lib_i32 failed = !les_lds_s41_prepare(profile, &state);

    if (!failed) {
        les_lds_s41_seed(&state);
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !les_lds_s41_run(&state, code, bytes, &after, &diagnostic,
            &status, &result) || status != LIB_STATUS_INTERNAL_ERROR ||
            !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.eip != 0u || lib_memory_compare(&before.data, &after.data,
            sizeof(before.data)) != 0;
    }
    core_machine_destroy(state.machine);
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

static lib_i32 les_lds_s41_boot_protected(les_lds_s41_machine *state)
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
    static const lib_u8 halt = 0xf4u;
    core_machine_run_result result;

    return core_machine_memory_write(state->machine, 0x100u, pointer,
        sizeof(pointer)) == LIB_STATUS_OK && core_machine_memory_write(
        state->machine, 0x300u, gdt, sizeof(gdt)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, boot, sizeof(boot)) ==
        LIB_STATUS_OK && core_machine_memory_write(state->machine, 0x2000u,
        &halt, sizeof(halt)) == LIB_STATUS_OK && core_machine_run(
        state->machine, (core_machine_run_budget){96u,0u}, &result) ==
        LIB_STATUS_OK && result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static lib_i32 les_lds_s41_protected_case(lib_u8 opcode, lib_u16 selector,
    lib_i32 expect_fault, lib_i32 null_selector)
{
    les_lds_s41_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    lib_u8 pointer[] = {0x44u,0x33u,0,0};
    lib_u8 source[4] = {0x44u,0x33u,0,0};
    lib_u8 observed[4] = {0};
    lib_u8 program[4] = {opcode,0x06u,0x10u,0u};
    lib_u8 access = 0u;
    lib_i32 failed = !les_lds_s41_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);

    pointer[2] = (lib_u8)selector;
    pointer[3] = (lib_u8)(selector >> 8u);
    source[2] = pointer[2];
    source[3] = pointer[3];
    if (!failed)
        failed |= !les_lds_s41_boot_protected(&state);
    if (!failed) {
        const t_cpu_data_sreg original = opcode == 0xc4u ?
            state.machine->executor_cpu.data.es : state.machine->executor_cpu.data.ds;

        les_lds_s41_seed(&state);
        state.machine->executor_cpu.data.cs.base = 0x2000u;
        state.machine->executor_cpu.data.ds.base = 0x3000u;
        state.machine->executor_cpu.data.ds.limit = 0xffffu;
        state.machine->executor_cpu.data.ss.base = 0x3000u;
        state.machine->executor_cpu.data.es = original;
        failed |= core_machine_memory_write(state.machine, 0x3010u, pointer,
            sizeof(pointer)) != LIB_STATUS_OK || core_machine_memory_write(
            state.machine, 0x2000u, program, sizeof(program)) != LIB_STATUS_OK;
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){1u,0u}, &result) !=
            (expect_fault ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK) ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            LIB_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !les_lds_s41_read(&state, 0x3010u, observed,
            sizeof(observed)) || lib_memory_compare(source, observed,
            sizeof(source)) != 0;
        if (expect_fault) {
            failed |= result.reason != CORE_MACHINE_STOP_FAULT ||
                !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
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
            failed |= result.reason != CORE_MACHINE_STOP_BUDGET ||
                diagnostic.first_fault.valid || after.data.eip != 4u ||
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
            failed |= result.reason != CORE_MACHINE_STOP_BUDGET ||
                diagnostic.first_fault.valid || after.data.eip != 4u ||
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
    core_machine_destroy(state.machine);
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
        les_lds_s41_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        lib_u8 code[] = {opcodes[opcode],0x06u,0x10u,0u};
        lib_i32 failed = !les_lds_s41_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed)
            failed |= !les_lds_s41_boot_protected(&state);
        if (!failed) {
            les_lds_s41_seed(&state);
            state.machine->executor_cpu.data.cs.base = 0x2000u;
            state.machine->executor_cpu.data.ds.base = 0x3000u;
            state.machine->executor_cpu.data.ds.limit = 0x11u;
            failed |= core_machine_memory_write(state.machine, 0x2000u, code,
                sizeof(code)) != LIB_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_INTERNAL_ERROR ||
                result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                LIB_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                after.data.eip != before.data.eip ||
                after.data.eax != before.data.eax ||
                !les_lds_s41_gprs_same_except_eax(&before, &after) ||
                after.data.eflags != before.data.eflags ||
                lib_memory_compare(opcodes[opcode] == 0xc4u ? &before.data.es :
                &before.data.ds, opcodes[opcode] == 0xc4u ? &after.data.es :
                &after.data.ds, sizeof(t_cpu_data_sreg)) != 0;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 les_lds_s41_test_irq(void)
{
    static const lib_u8 opcodes[] = {0xc4u,0xc5u};
    static const lib_u8 pointer[] = {0x44u,0x33u,0,0};
    static const lib_u8 hlt = 0xf4u;
    lib_u8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        les_lds_s41_machine state;
        core_machine_pic_irq_source irq;
        core_machine_run_result result;
        t_cpu before;
        t_cpu after;
        lib_u16 offset = 0x100u;
        lib_u16 segment = 0u;
        lib_u16 frame_ip = 0u;
        lib_u8 code[] = {opcodes[opcode],0x06u,0,0x10u,0x90u};
        lib_i32 failed = !les_lds_s41_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed) {
            failed |= core_machine_memory_write(state.machine, 0x1000u, pointer,
                sizeof(pointer)) != LIB_STATUS_OK || core_machine_memory_write(
                state.machine, 0u, code, sizeof(code)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x80u, &offset,
                sizeof(offset)) != LIB_STATUS_OK || core_machine_memory_write(
                state.machine, 0x82u, &segment, sizeof(segment)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x100u, &hlt,
                sizeof(hlt)) != LIB_STATUS_OK;
        }
        if (!failed) {
            les_lds_s41_seed(&state);
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_IF;
            before = test_core_machine_fixture_capture_cpu_after_run(
                state.machine);
            lib_memory_set(&irq, 0, sizeof(irq));
            state.machine->shared_pic_master.data.icw2 = 0x20u;
            core_machine_pic_irq_source_bind(&irq, &state.machine->shared_pic_master,
                &state.machine->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(&irq);
            core_machine_pic_irq_source_deassert(&irq);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){2u,0u}, &result) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                after.data.ss.base + (lib_u16)after.data.esp,
                CORE_MACHINE_REFERENCE_OF(frame_ip), sizeof(frame_ip)) != LIB_STATUS_OK ||
                after.data.eip != 0x101u || frame_ip != 4u ||
                after.data.eax != 0xaabb3344u ||
                !les_lds_s41_irq_gprs_same_except_eax(&before, &after) ||
                after.data.eflags != 0u || !CORE_MACHINE_BIT_IS_SET(
                state.machine->shared_pic_master.data.isr, VPIC_ISR_IRQ(0u)) ||
                CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.irr,
                VPIC_IRR_IRQ(0u));
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!les_lds_s41_test_real()) {
        printf("LES-LDS-S41 stage=real\n");
        return 1;
    }
    if (!les_lds_s41_test_rejections()) {
        printf("LES-LDS-S41 stage=rejections\n");
        return 1;
    }
    if (!les_lds_s41_test_protected()) {
        printf("LES-LDS-S41 stage=protected\n");
        return 1;
    }
    if (!les_lds_s41_test_limit()) {
        printf("LES-LDS-S41 stage=limit\n");
        return 1;
    }
    if (!les_lds_s41_test_irq()) {
        printf("LES-LDS-S41 stage=irq\n");
        return 1;
    }
    printf("M5:T316:S41:LES-LDS:OK\n");
    printf("M5:T401:S33:LES-LDS-PROFILES:OK\n");
    return 0;
}
