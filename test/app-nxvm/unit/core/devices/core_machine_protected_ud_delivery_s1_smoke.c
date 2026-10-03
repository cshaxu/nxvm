#include "lib/types/types_interface.h"
#include <stdio.h>
#include "x86/core/device_support_interface.h"

#include "x86/core/debug_interface.h"
#include "app-nxvm/devices/machine_board_interface.h"

#define UD_S1_GDT_BASE 0x0300u
#define UD_S1_IDT_BASE 0x0400u
#define UD_S1_CODE_BASE 0x2000u
#define UD_S1_STACK_BASE 0x3000u
#define UD_S1_HANDLER_OFFSET 0x0100u

typedef struct ud_s1_machine {
    core_machine *machine;
} ud_s1_machine;

static lib_i32 ud_s1_prepare(ud_s1_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };

    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP)
    };

    if (state == LIB_NULL) return 0;
    lib_memory_set(state, 0, sizeof(*state));
    return core_machine_create(&config, &state->machine, LIB_NULL) == LIB_STATUS_OK &&
        core_machine_freeze_execution_providers(state->machine) == LIB_STATUS_OK &&
        core_machine_reset(state->machine) == LIB_STATUS_OK &&
        core_machine_debug_patch_registers(state->machine, &entry) == LIB_STATUS_OK;
}

static lib_i32 ud_s1_gprs_same(const core_machine_debug_cpu_snapshot *before, const core_machine_debug_cpu_snapshot *after)
{
    return before->eax == after->eax &&
        before->ecx == after->ecx &&
        before->edx == after->edx &&
        before->ebx == after->ebx &&
        before->ebp == after->ebp &&
        before->esi == after->esi &&
        before->edi == after->edi;
}

static lib_i32 ud_s1_data_sregs_same(const core_machine_debug_cpu_snapshot *before, const core_machine_debug_cpu_snapshot *after)
{
    return lib_memory_compare(&before->es, &after->es,
            sizeof(before->es)) == 0 &&
        lib_memory_compare(&before->ss, &after->ss,
            sizeof(before->ss)) == 0 &&
        lib_memory_compare(&before->ds, &after->ds,
            sizeof(before->ds)) == 0 &&
        lib_memory_compare(&before->fs, &after->fs,
            sizeof(before->fs)) == 0 &&
        lib_memory_compare(&before->gs, &after->gs,
            sizeof(before->gs)) == 0;
}

static lib_i32 ud_s1_delivered(const core_machine_cpu_diagnostic *diagnostic)
{
    return !diagnostic->first_fault.valid &&
        diagnostic->last_delivered_exception.valid && CORE_MACHINE_BIT_IS_SET(
            diagnostic->last_delivered_exception.exception_mask,
            VCPUINS_EXCEPT_UD) &&
        diagnostic->last_delivered_exception.exception_code == 0u;
}

static lib_i32 ud_s1_boot_protected(ud_s1_machine *state,
    const lib_u8 *code, lib_size bytes, lib_u8 valid_gate)
{
    static const lib_u8 gdt[] = {
        0u,0u,0u,0u,0u,0u,0u,0u,
        0xffu,0xffu,0u,0x20u,0u,0x9au,0u,0u,
        0xffu,0xffu,0u,0x30u,0u,0x92u,0u,0u
    };
    lib_u8 idt[6u * 8u + 8u] = { 0u };
    /* Load architectural table registers through the guest instruction path. */
    static const lib_u8 setup[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x05u, /* LGDT [0500] */
        0x0fu, 0x01u, 0x1eu, 0x06u, 0x05u  /* LIDT [0506] */
    };
    const lib_u8 tables[] = {
        sizeof(gdt) - 1u, 0u, 0u, 3u, 0u, 0u,
        sizeof(idt) - 1u, 0u, 0u, 4u, 0u, 0u
    };
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_FS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_GS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
        .values = { [CORE_MACHINE_DEBUG_CS] = 8u,
            [CORE_MACHINE_DEBUG_SS] = 0x10u, [CORE_MACHINE_DEBUG_DS] = 0x10u,
            [CORE_MACHINE_DEBUG_ES] = 0x10u, [CORE_MACHINE_DEBUG_FS] = 0x10u,
            [CORE_MACHINE_DEBUG_GS] = 0x10u, [CORE_MACHINE_DEBUG_ESP] = 0x8000u,
            [CORE_MACHINE_DEBUG_EFLAGS] = CORE_MACHINE_DEBUG_EFLAGS_CF |
                CORE_MACHINE_DEBUG_EFLAGS_IF | CORE_MACHINE_DEBUG_EFLAGS_DF }
    };
    core_machine_run_result result;

    if (!ud_s1_prepare(state)) {
        return 0;
    }
    if (valid_gate) {
        idt[6u * 8u] = 0u;
        idt[6u * 8u + 1u] = 0x01u;
        idt[6u * 8u + 2u] = 0x08u;
        idt[6u * 8u + 5u] = 0x8eu;
    }
    if (core_machine_memory_write(state->machine, UD_S1_GDT_BASE, gdt,
            sizeof(gdt)) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, UD_S1_IDT_BASE, idt,
            sizeof(idt)) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, UD_S1_CODE_BASE, code,
            bytes) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine,
            UD_S1_CODE_BASE + UD_S1_HANDLER_OFFSET,
            (const lib_u8[]){ 0xf4u }, 1u) != LIB_STATUS_OK) {
        return 0;
    }
    return core_machine_memory_write(state->machine, 0u, setup,
            sizeof(setup)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x0500u, tables,
            sizeof(tables)) == LIB_STATUS_OK &&
        core_machine_run(state->machine, (core_machine_run_budget){2u, 0u},
            &result) == LIB_STATUS_OK && result.executed == 2u &&
        result.reason == CORE_MACHINE_STOP_BUDGET &&
        core_machine_debug_write_register(state->machine, CORE_MACHINE_DEBUG_CR0,
            VCPU_CR0_PE) == LIB_STATUS_OK &&
        core_machine_debug_patch_registers(state->machine, &entry) == LIB_STATUS_OK;
}

static lib_i32 ud_s1_protected_delivery(const lib_u8 *code,
    lib_size bytes)
{
    lib_u32 frame[3u] = { 0u, 0u, 0u };
    ud_s1_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_debug_cpu_snapshot before = {0};
    core_machine_debug_cpu_snapshot after = {0};
    lib_i32 failed = !ud_s1_boot_protected(&state, code, bytes, LIB_TRUE);

    if (!failed) {
        failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){ 1u, 0u }, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                LIB_STATUS_OK;
        failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
        failed |= !ud_s1_delivered(&diagnostic) ||
            after.eip != UD_S1_HANDLER_OFFSET ||
            after.esp != before.esp - 12u ||
            after.eflags != (before.eflags & ~CORE_MACHINE_DEBUG_EFLAGS_IF) ||
            !ud_s1_gprs_same(&before, &after) ||
            !ud_s1_data_sregs_same(&before, &after) ||
            core_machine_debug_read_linear(state.machine,
                after.ss.base + after.esp, frame, sizeof(frame)) != LIB_STATUS_OK || frame[0] != 0u ||
            frame[1] != before.cs.selector ||
            frame[2] != before.eflags;
    }
    if (!failed) {
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){ 1u, 0u }, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            after.eip != UD_S1_HANDLER_OFFSET + 1u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 ud_s1_metadata_and_lexeme(void)
{
    static const lib_u8 reserved[] = { 0x0fu, 0x25u, 0xc0u };
    static const lib_u8 adjacent[][3] = {
        { 0x0fu, 0x20u, 0xc0u }, { 0x0fu, 0x21u, 0xc0u },
        { 0x0fu, 0x22u, 0xc0u }, { 0x0fu, 0x23u, 0xc0u },
        { 0x0fu, 0x24u, 0xf0u }, { 0x0fu, 0x26u, 0xf0u }
    };
    core_machine_cpu_instruction_lexeme lexeme;
    lib_size index;

    if (core_machine_cpu_instruction_metadata_get(
            CORE_MACHINE_CPU_INSTRUCTION_0F, 0x25u, 0xc0u).valid ||
        core_machine_cpu_instruction_lexeme_scan(reserved, sizeof(reserved),
            CORE_MACHINE_CPU_PROFILE_80386, LIB_TRUE, &lexeme)) return 0;
    for (index = 0u; index != sizeof(adjacent) / sizeof(adjacent[0]); ++index) {
        if (!core_machine_cpu_instruction_metadata_get(
                CORE_MACHINE_CPU_INSTRUCTION_0F, adjacent[index][1],
                adjacent[index][2]).valid ||
            !core_machine_cpu_instruction_lexeme_scan(adjacent[index],
                sizeof(adjacent[index]), CORE_MACHINE_CPU_PROFILE_80386,
                LIB_TRUE, &lexeme) || !lexeme.available) return 0;
    }
    return 1;
}
static lib_i32 ud_s1_lexeme_memory_form_rejection(void)
{
    static const lib_u8 invalid[][3] = {
        { 0x62u, 0xc0u, 0u }, { 0x8du, 0xc0u, 0u },
        { 0xc4u, 0xc0u, 0u }, { 0xc5u, 0xc0u, 0u },
        { 0xffu, 0xd8u, 0u }, { 0xffu, 0xe8u, 0u },
        { 0x0fu, 0x01u, 0xc0u }, { 0x0fu, 0xb2u, 0xc0u },
        { 0x0fu, 0xb4u, 0xc0u }, { 0x0fu, 0xb5u, 0xc0u }
    };
    static const lib_u8 valid[][3] = {
        { 0x62u, 0x00u, 0u }, { 0x8du, 0x00u, 0u },
        { 0xc4u, 0x00u, 0u }, { 0xc5u, 0x00u, 0u },
        { 0xffu, 0x18u, 0u }, { 0xffu, 0x28u, 0u },
        { 0x0fu, 0x01u, 0x00u }, { 0x0fu, 0xb2u, 0x00u },
        { 0x0fu, 0xb4u, 0x00u }, { 0x0fu, 0xb5u, 0x00u }
    };
    core_machine_cpu_instruction_lexeme lexeme;
    lib_size index;

    for (index = 0u; index != sizeof(invalid) / sizeof(invalid[0]); ++index) {
        if (core_machine_cpu_instruction_lexeme_scan(invalid[index],
                sizeof(invalid[index]), CORE_MACHINE_CPU_PROFILE_80386,
                LIB_TRUE, &lexeme)) return 0;
    }
    for (index = 0u; index != sizeof(valid) / sizeof(valid[0]); ++index) {
        if (!core_machine_cpu_instruction_lexeme_scan(valid[index],
                sizeof(valid[index]), CORE_MACHINE_CPU_PROFILE_80386,
                LIB_TRUE, &lexeme) || !lexeme.available) return 0;
    }
    return 1;
}
static lib_i32 ud_s1_lexeme_primary_group_rejection(void)
{
    static const lib_u8 invalid[][2] = {
        { 0x8fu, 0xc8u }, { 0xc6u, 0xc8u }, { 0xc7u, 0xc8u },
        { 0xf6u, 0xc8u }, { 0xf7u, 0xc8u }, { 0xfeu, 0xd0u },
        { 0xffu, 0xf8u }
    };
    static const lib_u8 valid[][2] = {
        { 0x8fu, 0xc0u }, { 0xf6u, 0xd0u }, { 0xf7u, 0xd0u },
        { 0xfeu, 0xc0u }, { 0xffu, 0xf0u }
    };
    core_machine_cpu_instruction_lexeme lexeme;
    lib_size index;

    for (index = 0u; index != sizeof(invalid) / sizeof(invalid[0]); ++index) {
        if (core_machine_cpu_instruction_lexeme_scan(invalid[index],
                sizeof(invalid[index]), CORE_MACHINE_CPU_PROFILE_80386,
                LIB_TRUE, &lexeme)) return 0;
    }
    for (index = 0u; index != sizeof(valid) / sizeof(valid[0]); ++index) {
        if (!core_machine_cpu_instruction_lexeme_scan(valid[index],
                sizeof(valid[index]), CORE_MACHINE_CPU_PROFILE_80386,
                LIB_TRUE, &lexeme) || !lexeme.available) return 0;
    }
    if (!core_machine_cpu_instruction_lexeme_scan(
            (const lib_u8[]){ 0xc6u, 0xc0u, 0x12u }, 3u,
            CORE_MACHINE_CPU_PROFILE_80386, LIB_TRUE, &lexeme) ||
        !core_machine_cpu_instruction_lexeme_scan(
            (const lib_u8[]){ 0xc7u, 0xc0u, 0x78u, 0x56u, 0x34u, 0x12u },
            6u, CORE_MACHINE_CPU_PROFILE_80386, LIB_TRUE, &lexeme)) return 0;
    return 1;
}
static lib_i32 ud_s1_lexeme_8086_pop_cs(void)
{
    static const lib_u8 pop_cs[] = { 0x0fu };
    core_machine_cpu_instruction_lexeme lexeme;

    return core_machine_cpu_instruction_lexeme_scan(pop_cs, sizeof(pop_cs),
        CORE_MACHINE_CPU_PROFILE_8086, LIB_FALSE, &lexeme) && lexeme.available &&
        lexeme.byte_count == 1u && lexeme.component_count == 1u &&
        !core_machine_cpu_instruction_lexeme_scan(pop_cs, sizeof(pop_cs),
            CORE_MACHINE_CPU_PROFILE_80186, LIB_FALSE, &lexeme);
}
static lib_i32 ud_s1_primary_metadata_and_lexeme(void)
{
    static const lib_u8 reserved[] = { 0xf1u };
    core_machine_cpu_instruction_lexeme lexeme;
    core_machine_cpu_instruction_metadata metadata =
        core_machine_cpu_instruction_metadata_get(
            CORE_MACHINE_CPU_INSTRUCTION_PRIMARY, 0xf1u, 0u);

    return !metadata.valid && !core_machine_cpu_instruction_lexeme_scan(
        reserved, sizeof(reserved), CORE_MACHINE_CPU_PROFILE_80386,
        LIB_TRUE, &lexeme);
}
static core_machine_cpu_profile ud_s1_primary_expected_minimum(lib_u8 opcode)
{
    if ((opcode >= 0x60u && opcode <= 0x62u) || opcode == 0x68u ||
        opcode == 0x69u || opcode == 0x6au || opcode == 0x6bu ||
        (opcode >= 0x6cu && opcode <= 0x6fu) || opcode == 0xc0u ||
        opcode == 0xc1u || opcode == 0xc8u || opcode == 0xc9u)
        return CORE_MACHINE_CPU_PROFILE_80186;
    if (opcode == 0x63u) return CORE_MACHINE_CPU_PROFILE_80286;
    if (opcode >= 0x64u && opcode <= 0x67u)
        return CORE_MACHINE_CPU_PROFILE_80386;
    return CORE_MACHINE_CPU_PROFILE_8086;
}

static lib_i32 ud_s1_primary_metadata_matrix(void)
{
    lib_u16 value;

    for (value = 0u; value != 0x100u; ++value) {
        lib_u8 opcode = (lib_u8)value;
        core_machine_cpu_instruction_metadata metadata =
            core_machine_cpu_instruction_metadata_get(
                CORE_MACHINE_CPU_INSTRUCTION_PRIMARY, opcode, 0u);
        lib_u8 reserved = opcode == 0xd6u || opcode == 0xf1u;

        if (metadata.valid == reserved || (!reserved &&
            metadata.minimum_cpu != ud_s1_primary_expected_minimum(opcode))) return 0;
    }
    return 1;
}
static core_machine_cpu_profile ud_s1_0f_expected_minimum(lib_u8 opcode)
{
    if (opcode == 0x00u || opcode == 0x01u || opcode == 0x02u ||
        opcode == 0x03u || opcode == 0x06u) return CORE_MACHINE_CPU_PROFILE_80286;
    if ((opcode >= 0x20u && opcode <= 0x24u) || opcode == 0x26u ||
        (opcode >= 0x80u && opcode <= 0x8fu) ||
        (opcode >= 0x90u && opcode <= 0x9fu) || opcode == 0xa0u ||
        opcode == 0xa1u || opcode == 0xa3u || opcode == 0xa4u ||
        opcode == 0xa5u || opcode == 0xa8u || opcode == 0xa9u ||
        opcode == 0xabu || opcode == 0xacu || opcode == 0xadu ||
        opcode == 0xafu || (opcode >= 0xb2u && opcode <= 0xb7u) ||
        (opcode >= 0xbbu && opcode <= 0xbfu))
        return CORE_MACHINE_CPU_PROFILE_80386;
    return (core_machine_cpu_profile)0xffu;
}

static lib_i32 ud_s1_0f_metadata_matrix(void)
{
    lib_u16 value;

    for (value = 0u; value != 0x100u; ++value) {
        lib_u8 opcode = (lib_u8)value;
        core_machine_cpu_instruction_metadata metadata =
            core_machine_cpu_instruction_metadata_get(
                CORE_MACHINE_CPU_INSTRUCTION_0F, opcode, 0xc0u);
        core_machine_cpu_profile expected = ud_s1_0f_expected_minimum(opcode);

        if ((expected == (core_machine_cpu_profile)0xffu) != (!metadata.valid) ||
            (metadata.valid && metadata.minimum_cpu != expected)) return 0;
    }
    if (core_machine_cpu_instruction_metadata_get(
            CORE_MACHINE_CPU_INSTRUCTION_0F, 0xbau, 0xc0u).valid ||
        !core_machine_cpu_instruction_metadata_get(
            CORE_MACHINE_CPU_INSTRUCTION_0F, 0xbau, 0xe0u).valid) return 0;
    return 1;
}
static lib_i32 ud_s1_protected_invalid_gate(void)
{
    static const lib_u8 code[] = { 0x0fu, 0x01u, 0xf8u };
    ud_s1_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_debug_cpu_snapshot before = {0};
    core_machine_debug_cpu_snapshot after = {0};
    lib_i32 failed = !ud_s1_boot_protected(&state, code, sizeof(code), LIB_FALSE);

    if (!failed) {
        failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){ 1u, 0u }, &result) != LIB_STATUS_INTERNAL_ERROR ||
            result.reason != CORE_MACHINE_STOP_FAULT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                LIB_STATUS_OK;
        failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
        failed |= !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.eip != before.eip || after.esp != before.esp ||
            after.eflags != before.eflags ||
            !ud_s1_gprs_same(&before, &after) ||
            !ud_s1_data_sregs_same(&before, &after);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

lib_i32 main(void)
{
    static const lib_u8 invalid_primary[] = { 0xf1u };
    static const lib_u8 reserved_0f[] = { 0x0fu, 0x01u, 0xf8u };
    static const lib_u8 reserved_0f25[] = { 0x0fu, 0x25u, 0xc0u };
    static const lib_u8 invalid_operand[] = { 0x62u, 0xc0u };
    static const lib_u8 invalid_lock[] = { 0xf0u, 0x90u };
    const lib_u8 *forms[] = {
        invalid_primary, reserved_0f, reserved_0f25, invalid_operand, invalid_lock
    };
    const lib_size sizes[] = {
        sizeof(invalid_primary), sizeof(reserved_0f), sizeof(reserved_0f25), sizeof(invalid_operand),
        sizeof(invalid_lock)
    };
    lib_size index;

    for (index = 0u; index < sizeof(forms) / sizeof(forms[0]); ++index) {
        if (!ud_s1_protected_delivery(forms[index], sizes[index])) {
            return 1;
        }
    }
    if (!ud_s1_metadata_and_lexeme() || !ud_s1_lexeme_memory_form_rejection() ||
        !ud_s1_lexeme_primary_group_rejection() ||
        !ud_s1_lexeme_8086_pop_cs() || !ud_s1_primary_metadata_and_lexeme() ||
        !ud_s1_primary_metadata_matrix() || !ud_s1_0f_metadata_matrix() ||
        !ud_s1_protected_invalid_gate()) {
        return 1;
    }
    printf("M5:T326:S1:PROTECTED-UD-DELIVERY:OK\n");
    printf("M5:T401:S2:0F25-METADATA:OK\n");
    printf("M5:T401:S3:0F-METADATA-MATRIX:OK\n");
    printf("M5:T401:S4:F1-METADATA:OK\n");
    printf("M5:T401:S4:PRIMARY-METADATA-MATRIX:OK\n");
    printf("M5:T401:S5:LEXEME-8086-POP-CS:OK\n");
    printf("M5:T401:S5:LEXEME-PRIMARY-GROUPS:OK\n");
    printf("M5:T401:S5:LEXEME-MEMORY-FORMS:OK\n");
    return 0;
}
