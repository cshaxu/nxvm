#include "support/cpu_instruction_fixture.h"
#include <stdio.h>

#define OAS_GDT_POINTER 0x0100u
#define OAS_GDT_ADDRESS 0x0300u
#define OAS_CODE_ADDRESS 0x2000u
#define OAS_DATA_ADDRESS 0x3000u
#define OAS_STACK_ADDRESS 0x4000u

typedef struct oas_machine {
    cpu_instruction_fixture chip;
} oas_machine;

static lib_i32 oas_write(oas_machine *state, lib_u32 address,
    const void *bytes, lib_size byte_count)
{
    return byte_count <= 255u && cpu_instruction_write(&state->chip,
        address, bytes, (lib_u8)byte_count,
        CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) == LIB_STATUS_OK;
}

static lib_status oas_read(oas_machine *state, lib_u32 address,
    void *bytes, lib_size byte_count)
{
    if (byte_count > 255u) return LIB_STATUS_INVALID_ARGUMENT;
    return cpu_instruction_read(&state->chip, address, bytes,
        (lib_u8)byte_count, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA,
        LIB_FALSE, LIB_FALSE);
}

static lib_status oas_execute(oas_machine *state, lib_u32 budget)
{
    lib_u32 step;

    for (step = 0u; step != budget; ++step) {
        if (state->chip.cpu.data.flagHalt || state->chip.execution.stop_requested)
            break;
        core_machine_cpu_execution_refresh(&state->chip.execution);
    }
    return state->chip.execution.stop_requested ?
        LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK;
}

static void oas_resume(oas_machine *state, lib_u32 eip)
{
    state->chip.cpu.data.eip = eip;
    state->chip.cpu.data.flagHalt = LIB_FALSE;
}

static lib_i32 oas_prepare(oas_machine *state, core_machine_cpu_profile profile,
    lib_i32 code32)
{
    const lib_u8 gdt_pointer[] = { 0x1fu,0,0,0x03u,0,0 };
    lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0xcfu,0,
        0xffu,0xffu,0,0x40u,0,0x92u,0x40u,0
    };
    const lib_u8 real_code[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,
        0xb8u,0x18u,0x00u,0x8eu,0xd0u,
        0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    const lib_u8 halt[] = { 0xf4u };

    if (state == LIB_NULL) return 0;
    cpu_instruction_prepare(&state->chip, profile);
    gdt[14] = code32 ? 0x40u : 0u;
    return oas_write(state, OAS_GDT_POINTER, gdt_pointer,
            sizeof(gdt_pointer)) &&
        oas_write(state, OAS_GDT_ADDRESS, gdt, sizeof(gdt)) &&
        oas_write(state, 0u, real_code, sizeof(real_code)) &&
        oas_write(state, OAS_CODE_ADDRESS, halt, sizeof(halt)) &&
        oas_execute(state, 96u) == LIB_STATUS_OK &&
        state->chip.cpu.data.flagHalt;
}

static lib_i32 oas_run_halt(oas_machine *state, const lib_u8 *code,
    lib_size code_size, t_cpu *out_cpu)
{
    if (!oas_write(state, OAS_CODE_ADDRESS, code, code_size)) return 0;
    oas_resume(state, 0u);
    if (oas_execute(state, 48u) != LIB_STATUS_OK ||
        !state->chip.cpu.data.flagHalt) return 0;
    *out_cpu = state->chip.cpu;
    return 1;
}

static lib_i32 oas_run_gp(oas_machine *state, const lib_u8 *code,
    lib_size code_size, t_cpu *out_cpu)
{
    if (!oas_write(state, OAS_CODE_ADDRESS, code, code_size)) return 0;
    oas_resume(state, 0u);
    if (oas_execute(state, 16u) != LIB_STATUS_INTERNAL_ERROR ||
        !state->chip.fault.valid ||
        !X86_CPU_BIT_IS_SET(state->chip.fault.exception_mask,
            X86_CPU_BIT_IS_SET(state->chip.cpu.data.cr0, VCPU_CR0_PE) ?
                VCPUINS_EXCEPT_DF : VCPUINS_EXCEPT_GP)) return 0;
    *out_cpu = state->chip.cpu;
    return 1;
}

static lib_i32 oas_test_prefix_and_ea(void)
{
    static const lib_u8 repeat_prefix[] = {
        0x66u,0x66u,0xb8u,0x34u,0x12u,0xb9u,0x78u,0x56u,0x34u,0x12u,0xf4u
    };
    static const lib_u8 address16[] = { 0x67u,0x8bu,0x00u,0xf4u };
    static const lib_u8 operand16[] = { 0x66u,0x8bu,0x00u,0xf4u };
    static const lib_u8 moffs16[] = { 0x67u,0xa1u,0x60u,0x00u,0xf4u };
    static const lib_u8 sib_scaled[] = { 0x8bu,0x44u,0x88u,0xfcu,0xf4u };
    static const lib_u8 sib_absolute[] = {
        0x8bu,0x04u,0x25u,0x20u,0x01u,0x00u,0x00u,0xf4u
    };
    static const lib_u8 ss_default[] = { 0x8bu,0x45u,0x00u,0xf4u };
    static const lib_u8 ds_override[] = { 0x3eu,0x8bu,0x45u,0x00u,0xf4u };
    const lib_u32 word_source = 0x1234beefu;
    const lib_u32 sib_source = 0x87654321u;
    const lib_u32 absolute_source = 0x0badf00du;
    const lib_u32 ds_source = 0x13579bdfu;
    const lib_u32 ss_source = 0x2468ace0u;
    oas_machine state;
    t_cpu cpu;
    lib_i32 failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);

    if (!failed) failed |= !oas_run_halt(&state, repeat_prefix,
        sizeof(repeat_prefix), &cpu) || cpu.data.eax != 0x00001234u ||
        cpu.data.ecx != 0x12345678u;

    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.chip.cpu.data.ebx = 0x0040u;
        state.chip.cpu.data.esi = 0x0010u;
        failed |= !oas_write(&state, OAS_DATA_ADDRESS + 0x0050u, &word_source,
                sizeof(word_source)) || !oas_run_halt(&state, address16,
                sizeof(address16), &cpu) || cpu.data.eax != word_source;
    }

    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.chip.cpu.data.eax = 0x0070u;
        failed |= !oas_write(&state, OAS_DATA_ADDRESS + 0x0070u, &word_source,
                sizeof(word_source)) || !oas_run_halt(&state, operand16,
                sizeof(operand16), &cpu) || cpu.data.eax != 0x0000beefu;
    }

    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        failed |= !oas_write(&state, OAS_DATA_ADDRESS + 0x0060u, &word_source,
                sizeof(word_source)) || !oas_run_halt(&state, moffs16,
                sizeof(moffs16), &cpu) || cpu.data.eax != word_source;
    }

    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.chip.cpu.data.eax = 0x0100u;
        state.chip.cpu.data.ecx = 1u;
        failed |= !oas_write(&state, OAS_DATA_ADDRESS + 0x0100u, &sib_source,
                sizeof(sib_source)) || !oas_run_halt(&state, sib_scaled,
                sizeof(sib_scaled), &cpu) || cpu.data.eax != sib_source;
    }

    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) failed |= !oas_write(&state, OAS_DATA_ADDRESS + 0x0120u,
            &absolute_source, sizeof(absolute_source)) || !oas_run_halt(&state,
            sib_absolute, sizeof(sib_absolute), &cpu) ||
            cpu.data.eax != absolute_source;

    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.chip.cpu.data.ebp = 0x0080u;
        failed |= !oas_write(&state, OAS_DATA_ADDRESS + 0x0080u, &ds_source,
                sizeof(ds_source)) || !oas_write(&state, OAS_STACK_ADDRESS + 0x0080u,
                &ss_source, sizeof(ss_source)) || !oas_run_halt(&state,
                ss_default, sizeof(ss_default), &cpu) || cpu.data.eax != ss_source;
    }

    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.chip.cpu.data.ebp = 0x0080u;
        failed |= !oas_write(&state, OAS_DATA_ADDRESS + 0x0080u, &ds_source,
                sizeof(ds_source)) || !oas_write(&state, OAS_STACK_ADDRESS + 0x0080u,
                &ss_source, sizeof(ss_source)) || !oas_run_halt(&state,
                ds_override, sizeof(ds_override), &cpu) || cpu.data.eax != ds_source;
    }

    return !failed;
}

static lib_i32 oas_test_16bit_code_and_faults(void)
{
    static const lib_u8 defaults16[] = {
        0x66u,0xb8u,0x78u,0x56u,0x34u,0x12u,0xb9u,0x34u,0x12u,0xf4u
    };
    static const lib_u8 address32[] = { 0x67u,0x8bu,0x00u,0xf4u };
    static const lib_u8 invalid_data[] = {
        0x8bu,0x05u,0x20u,0x00u,0x00u,0x00u
    };
    static const lib_u8 invalid_expand_down[] = {
        0x8bu,0x05u,0x10u,0x00u,0x00u,0x00u
    };
    static const lib_u8 nop[] = { 0x90u };
    static const lib_u8 halt[] = { 0xf4u };
    oas_machine state;
    t_cpu before;
    t_cpu after;
    const lib_u32 source = 0xcafebabeu;
    lib_i32 failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 0);

    if (!failed) {
        state.chip.cpu.data.ecx = 0xdead0000u;
        failed |= !oas_run_halt(&state, defaults16, sizeof(defaults16), &after) ||
            after.data.eax != 0x12345678u || after.data.ecx != 0xdead1234u;
    }

    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.chip.cpu.data.ds.limit = 0x10u;
        state.chip.cpu.data.eax = 0xaabbccddu;
        before = state.chip.cpu;
        failed |= !oas_run_gp(&state, invalid_data, sizeof(invalid_data), &after) ||
            after.data.eax != before.data.eax;
    }

    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.chip.cpu.data.ds.limit = 0x10u;
        state.chip.cpu.data.ds.seg.data.expdown = LIB_TRUE;
        state.chip.cpu.data.ds.seg.data.big = LIB_FALSE;
        state.chip.cpu.data.eax = 0x11223344u;
        before = state.chip.cpu;
        failed |= !oas_run_gp(&state, invalid_expand_down,
            sizeof(invalid_expand_down), &after) ||
            after.data.eax != before.data.eax;
    }

    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 0);
    if (!failed) {
        state.chip.cpu.data.eax = 0x00010070u;
        failed |= !oas_write(&state, OAS_DATA_ADDRESS + 0x00010070u, &source,
                sizeof(source)) || !oas_run_halt(&state, address32,
                sizeof(address32), &after) || after.data.eax != 0x0001babeu;
    }

    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 0);
    if (!failed) {
        failed |= !oas_write(&state, OAS_CODE_ADDRESS + 0xffffu, nop,
                sizeof(nop)) || !oas_write(&state, OAS_CODE_ADDRESS, halt,
                sizeof(halt));
        oas_resume(&state, 0xffffu);
        failed |= oas_execute(&state, 8u) != LIB_STATUS_OK ||
            !state.chip.cpu.data.flagHalt;
        after = state.chip.cpu;
        failed |= after.data.eip != 1u;
    }

    return !failed;
}

static void oas_set_stack32(oas_machine *state, lib_u32 esp)
{
    state->chip.cpu.data.ss.seg.data.big = LIB_TRUE;
    state->chip.cpu.data.ss.limit = 0xffffffffu;
    state->chip.cpu.data.esp = esp;
}

static lib_i32 oas_test_stack_forms(void)
{
    static const lib_u8 push_pop32[] = { 0x50u,0x59u,0xf4u };
    static const lib_u8 push_pop16[] = { 0x66u,0x50u,0x66u,0x5bu,0xf4u };
    static const lib_u8 pusha[] = { 0x60u,0xf4u };
    static const lib_u8 popa[] = { 0x61u,0xf4u };
    static const lib_u8 pushf[] = { 0x9cu,0xf4u };
    static const lib_u8 popf[] = { 0x9du,0xf4u };
    static const lib_u8 enter_leave[] = {
        0xc8u,0x10u,0x00u,0x02u,0xc9u,0xf4u
    };
    static const lib_u8 enter_leave16[] = {
        0x66u,0xc8u,0x08u,0x00u,0x01u,0x66u,0xc9u,0xf4u
    };
    const lib_u32 ignored_slot = 0xfeedfaceu;
    const lib_u32 flags = VCPU_EFLAGS_IF | 0x00000002u;
    oas_machine state;
    t_cpu before;
    t_cpu after;
    lib_i32 failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);

    if (!failed) {
        state.chip.cpu.data.esp = 0x0100u;
        state.chip.cpu.data.eax = 0x11223344u;
        failed |= !oas_run_halt(&state, push_pop32, sizeof(push_pop32), &after) ||
            after.data.esp != 0x0100u || after.data.ecx != 0x11223344u;
    }

    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.chip.cpu.data.esp = 0x0100u;
        state.chip.cpu.data.eax = 0x11223344u;
        failed |= !oas_run_halt(&state, push_pop16, sizeof(push_pop16), &after) ||
            after.data.esp != 0x0100u || after.data.ebx != 0x00003344u;
    }

    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        oas_set_stack32(&state, 0x00010100u);
        state.chip.cpu.data.eax = 0x55667788u;
        failed |= !oas_run_halt(&state, push_pop16, sizeof(push_pop16), &after) ||
            after.data.esp != 0x00010100u || after.data.ebx != 0x00007788u;
    }

    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        oas_set_stack32(&state, 0x00000100u);
        state.chip.cpu.data.eax = 0x11111111u;
        state.chip.cpu.data.ecx = 0x22222222u;
        state.chip.cpu.data.edx = 0x33333333u;
        state.chip.cpu.data.ebx = 0x44444444u;
        state.chip.cpu.data.ebp = 0x55555555u;
        state.chip.cpu.data.esi = 0x66666666u;
        state.chip.cpu.data.edi = 0x77777777u;
        before = state.chip.cpu;
        failed |= !oas_run_halt(&state, pusha, sizeof(pusha), &after) ||
            after.data.esp != 0x000000e0u ||
            !oas_write(&state, OAS_STACK_ADDRESS + 0x000000ecu, &ignored_slot,
                sizeof(ignored_slot)) || !oas_run_halt(&state, popa,
                sizeof(popa), &after) || after.data.esp != before.data.esp ||
            after.data.eax != before.data.eax || after.data.ecx != before.data.ecx ||
            after.data.edx != before.data.edx || after.data.ebx != before.data.ebx ||
            after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi;
    }

    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        oas_set_stack32(&state, 0x00000100u);
        state.chip.cpu.data.eflags = 0x00000002u;
        failed |= !oas_run_halt(&state, pushf, sizeof(pushf), &after) ||
            !oas_write(&state, OAS_STACK_ADDRESS + after.data.esp, &flags,
                sizeof(flags)) || !oas_run_halt(&state, popf, sizeof(popf),
                &after) || after.data.esp != 0x00000100u ||
            (after.data.eflags & VCPU_EFLAGS_IF) == 0u;
    }

    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        oas_set_stack32(&state, 0x00000100u);
        state.chip.cpu.data.ebp = 0x00000080u;
        before = state.chip.cpu;
        failed |= !oas_run_halt(&state, enter_leave, sizeof(enter_leave), &after) ||
            after.data.esp != before.data.esp || after.data.ebp != before.data.ebp;
    }

    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        oas_set_stack32(&state, 0x00000100u);
        state.chip.cpu.data.ebp = 0x00000080u;
        before = state.chip.cpu;
        failed |= !oas_run_halt(&state, enter_leave16, sizeof(enter_leave16),
            &after) || after.data.esp != before.data.esp ||
            after.data.ebp != before.data.ebp;
    }

    return !failed;
}

static lib_i32 oas_test_memory_strings(void)
{
    static const lib_u8 movs[] = { 0xbeu,0,1,0,0,0xbfu,0,2,0,0,
        0xb9u,2,0,0,0,0xf3u,0xa4u,0xf4u };
    static const lib_u8 stos_lods[] = { 0xb8u,0x5au,0,0,0,0xbfu,0,3,0,0,
        0xaau,0xbeu,0,3,0,0,0xacu,0xf4u };
    static const lib_u8 df_movs[] = { 0xbeu,1,1,0,0,0xbfu,1,2,0,0,
        0xfdu,0xa4u,0xf4u };
    static const lib_u8 wrap_movs[] = { 0x66u,0xbeu,0xffu,0xffu,
        0x66u,0xbfu,0xffu,0xffu,0x66u,0xb9u,1,0,0xf3u,0x67u,0xa4u,0xf4u };
    static const lib_u8 limited_movs[] = { 0xbeu,0,1,0,0,0xbfu,0,2,0,0,
        0xb9u,1,0,0,0,0xf3u,0xa4u };
    static const lib_u8 source[] = { 0x31u,0x42u };
    lib_u8 destination[2] = {0};
    oas_machine state;
    t_cpu after;
    lib_i32 failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);

    if (!failed) failed |= !oas_write(&state, OAS_DATA_ADDRESS + 0x0100u,
        source, sizeof(source)) || !oas_run_halt(&state, movs, sizeof(movs),
        &after) || after.data.esi != 0x0102u || after.data.edi != 0x0202u ||
        after.data.ecx != 0u || oas_read(&state, OAS_DATA_ADDRESS + 0x0200u, destination, sizeof(destination)) !=
            LIB_STATUS_OK || lib_memory_compare(source, destination, sizeof(source));

    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.chip.cpu.data.es.base = OAS_STACK_ADDRESS;
        failed |= !oas_write(&state, OAS_DATA_ADDRESS + 0x0100u, source, 1u) ||
            !oas_run_halt(&state, movs, sizeof(movs), &after) ||
            oas_read(&state, OAS_STACK_ADDRESS + 0x0200u,
                destination, 1u) != LIB_STATUS_OK || destination[0] != source[0];
    }

    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.chip.cpu.data.ds.limit = 0x00ffu;
        failed |= !oas_run_gp(&state, limited_movs, sizeof(limited_movs), &after) ||
            after.data.esi != 0x0100u || after.data.edi != 0x0200u ||
            after.data.ecx != 1u;
    }

    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) failed |= !oas_write(&state, OAS_DATA_ADDRESS + 0x0101u,
        source, 1u) || !oas_run_halt(&state, df_movs, sizeof(df_movs), &after) ||
        after.data.esi != 0x0100u || after.data.edi != 0x0200u;

    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) failed |= !oas_write(&state, OAS_DATA_ADDRESS + 0xffffu,
        source, 1u) || !oas_run_halt(&state, wrap_movs, sizeof(wrap_movs),
        &after) || after.data.esi != 0u || after.data.edi != 0u;

    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) failed |= !oas_run_halt(&state, stos_lods, sizeof(stos_lods),
        &after) || after.data.al != 0x5au || after.data.esi != 0x0301u ||
        after.data.edi != 0x0301u;

    return !failed;
}

lib_i32 main(void)
{
    if (!oas_test_prefix_and_ea()) return 1;
    if (!oas_test_16bit_code_and_faults()) return 1;
    if (!oas_test_stack_forms()) return 1;
    if (!oas_test_memory_strings()) return 1;
    printf("M5:T539:S29:CPU-OPERAND-ADDRESS:OK\n");
    return 0;
}
