#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/cpu_instructions.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define UD_S1_GDT_BASE 0x0300u
#define UD_S1_IDT_BASE 0x0400u
#define UD_S1_CODE_BASE 0x2000u
#define UD_S1_STACK_BASE 0x3000u
#define UD_S1_HANDLER_OFFSET 0x0100u

typedef struct ud_s1_machine {
    core_machine *machine;
} ud_s1_machine;

static C_VOID ud_s1_reset(C_VOID *opaque)
{
    ud_s1_machine *state = (ud_s1_machine *)opaque;

    if (state != STD_NULL) {
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
    }
}

static const core_machine_execution_provider ud_s1_provider = {
    ud_s1_reset, STD_NULL
};

static C_INT ud_s1_prepare(ud_s1_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    if (state == STD_NULL) {
        return 0;
    }
    STD_MEMSET(state, 0, sizeof(*state));
    return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &ud_s1_provider, state, &state->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u);
}

static C_INT ud_s1_gprs_same(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi;
}

static C_INT ud_s1_data_sregs_same(const t_cpu *before, const t_cpu *after)
{
    return STD_MEMCMP(&before->data.es, &after->data.es,
            sizeof(before->data.es)) == 0 &&
        STD_MEMCMP(&before->data.ss, &after->data.ss,
            sizeof(before->data.ss)) == 0 &&
        STD_MEMCMP(&before->data.ds, &after->data.ds,
            sizeof(before->data.ds)) == 0 &&
        STD_MEMCMP(&before->data.fs, &after->data.fs,
            sizeof(before->data.fs)) == 0 &&
        STD_MEMCMP(&before->data.gs, &after->data.gs,
            sizeof(before->data.gs)) == 0;
}

static C_INT ud_s1_delivered(const core_machine_cpu_diagnostic *diagnostic)
{
    return !diagnostic->first_fault.valid &&
        diagnostic->last_delivered_exception.valid && TYPE_GET_BIT(
            diagnostic->last_delivered_exception.exception_mask,
            VCPUINS_EXCEPT_UD) &&
        diagnostic->last_delivered_exception.exception_code == 0u;
}

static C_INT ud_s1_boot_protected(ud_s1_machine *state,
    const type_unsigned_8 *code, STD_SIZE_T bytes, type_bool valid_gate)
{
    static const type_unsigned_8 gdt[] = {
        0u,0u,0u,0u,0u,0u,0u,0u,
        0xffu,0xffu,0u,0x20u,0u,0x9au,0u,0u,
        0xffu,0xffu,0u,0x30u,0u,0x92u,0u,0u
    };
    type_unsigned_8 idt[6u * 8u + 8u] = { 0u };
    t_cpu *cpu;

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
            sizeof(gdt)) != TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, UD_S1_IDT_BASE, idt,
            sizeof(idt)) != TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, UD_S1_CODE_BASE, code,
            bytes) != TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine,
            UD_S1_CODE_BASE + UD_S1_HANDLER_OFFSET,
            (const type_unsigned_8[]){ 0xf4u }, 1u) != TYPE_STATUS_OK) {
        return 0;
    }
    cpu = &state->machine->executor_cpu;
    cpu->data.cr0 = VCPU_CR0_PE;
    cpu->data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
    cpu->data.gdtr.flagValid = TYPE_TRUE;
    cpu->data.gdtr.sregtype = SREG_GDTR;
    cpu->data.gdtr.base = UD_S1_GDT_BASE;
    cpu->data.gdtr.limit = sizeof(gdt) - 1u;
    cpu->data.idtr.flagValid = TYPE_TRUE;
    cpu->data.idtr.sregtype = SREG_IDTR;
    cpu->data.idtr.base = UD_S1_IDT_BASE;
    cpu->data.idtr.limit = sizeof(idt) - 1u;
    cpu->data.cs.selector = 0x0008u;
    cpu->data.cs.base = UD_S1_CODE_BASE;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.flagValid = TYPE_TRUE;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.dpl = 0u;
    cpu->data.cs.seg.executable = TYPE_TRUE;
    cpu->data.ss.selector = 0x0010u;
    cpu->data.ss.base = UD_S1_STACK_BASE;
    cpu->data.ss.limit = 0xffffu;
    cpu->data.ss.flagValid = TYPE_TRUE;
    cpu->data.ss.sregtype = SREG_STACK;
    cpu->data.ss.dpl = 0u;
    cpu->data.ss.seg.data.writable = TYPE_TRUE;
    cpu->data.ds = cpu->data.ss;
    cpu->data.ds.sregtype = SREG_DATA;
    cpu->data.es = cpu->data.ds;
    cpu->data.fs = cpu->data.ds;
    cpu->data.gs = cpu->data.ds;
    cpu->data.esp = 0x00008000u;
    cpu->data.eip = 0u;
    return 1;
}

static C_INT ud_s1_protected_delivery(const type_unsigned_8 *code,
    STD_SIZE_T bytes)
{
    type_unsigned_32 frame[3u] = { 0u, 0u, 0u };
    ud_s1_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    C_INT failed = !ud_s1_boot_protected(&state, code, bytes, TYPE_TRUE);

    if (!failed) {
        before = state.machine->executor_cpu;
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){ 1u, 0u }, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !ud_s1_delivered(&diagnostic) ||
            after.data.eip != UD_S1_HANDLER_OFFSET ||
            after.data.esp != before.data.esp - 12u ||
            after.data.eflags != (before.data.eflags & ~VCPU_EFLAGS_IF) ||
            !ud_s1_gprs_same(&before, &after) ||
            !ud_s1_data_sregs_same(&before, &after) ||
            !test_core_machine_fixture_read_linear(state.machine,
                after.data.ss.base + after.data.esp, TYPE_REFERENCE_OF(frame),
                sizeof(frame)) || frame[0] != 0u ||
            frame[1] != before.data.cs.selector ||
            frame[2] != before.data.eflags;
    }
    if (!failed) {
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){ 1u, 0u }, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            state.machine->executor_cpu.data.eip != UD_S1_HANDLER_OFFSET + 1u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT ud_s1_metadata_and_lexeme(C_VOID)
{
    static const type_unsigned_8 reserved[] = { 0x0fu, 0x25u, 0xc0u };
    static const type_unsigned_8 adjacent[][3] = {
        { 0x0fu, 0x20u, 0xc0u }, { 0x0fu, 0x21u, 0xc0u },
        { 0x0fu, 0x22u, 0xc0u }, { 0x0fu, 0x23u, 0xc0u },
        { 0x0fu, 0x24u, 0xf0u }, { 0x0fu, 0x26u, 0xf0u }
    };
    core_machine_cpu_instruction_lexeme lexeme;
    STD_SIZE_T index;

    if (core_machine_cpu_instruction_metadata_get(
            CORE_MACHINE_CPU_INSTRUCTION_0F, 0x25u, 0xc0u).valid ||
        core_machine_cpu_instruction_lexeme_scan(reserved, sizeof(reserved),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE, &lexeme)) return 0;
    for (index = 0u; index != sizeof(adjacent) / sizeof(adjacent[0]); ++index) {
        if (!core_machine_cpu_instruction_metadata_get(
                CORE_MACHINE_CPU_INSTRUCTION_0F, adjacent[index][1],
                adjacent[index][2]).valid ||
            !core_machine_cpu_instruction_lexeme_scan(adjacent[index],
                sizeof(adjacent[index]), CORE_MACHINE_CPU_PROFILE_80386,
                TYPE_TRUE, &lexeme) || !lexeme.available) return 0;
    }
    return 1;
}
static C_INT ud_s1_lexeme_memory_form_rejection(C_VOID)
{
    static const type_unsigned_8 invalid[][3] = {
        { 0x62u, 0xc0u, 0u }, { 0x8du, 0xc0u, 0u },
        { 0xc4u, 0xc0u, 0u }, { 0xc5u, 0xc0u, 0u },
        { 0xffu, 0xd8u, 0u }, { 0xffu, 0xe8u, 0u },
        { 0x0fu, 0x01u, 0xc0u }, { 0x0fu, 0xb2u, 0xc0u },
        { 0x0fu, 0xb4u, 0xc0u }, { 0x0fu, 0xb5u, 0xc0u }
    };
    static const type_unsigned_8 valid[][3] = {
        { 0x62u, 0x00u, 0u }, { 0x8du, 0x00u, 0u },
        { 0xc4u, 0x00u, 0u }, { 0xc5u, 0x00u, 0u },
        { 0xffu, 0x18u, 0u }, { 0xffu, 0x28u, 0u },
        { 0x0fu, 0x01u, 0x00u }, { 0x0fu, 0xb2u, 0x00u },
        { 0x0fu, 0xb4u, 0x00u }, { 0x0fu, 0xb5u, 0x00u }
    };
    core_machine_cpu_instruction_lexeme lexeme;
    STD_SIZE_T index;

    for (index = 0u; index != sizeof(invalid) / sizeof(invalid[0]); ++index) {
        if (core_machine_cpu_instruction_lexeme_scan(invalid[index],
                sizeof(invalid[index]), CORE_MACHINE_CPU_PROFILE_80386,
                TYPE_TRUE, &lexeme)) return 0;
    }
    for (index = 0u; index != sizeof(valid) / sizeof(valid[0]); ++index) {
        if (!core_machine_cpu_instruction_lexeme_scan(valid[index],
                sizeof(valid[index]), CORE_MACHINE_CPU_PROFILE_80386,
                TYPE_TRUE, &lexeme) || !lexeme.available) return 0;
    }
    return 1;
}
static C_INT ud_s1_lexeme_primary_group_rejection(C_VOID)
{
    static const type_unsigned_8 invalid[][2] = {
        { 0x8fu, 0xc8u }, { 0xc6u, 0xc8u }, { 0xc7u, 0xc8u },
        { 0xf6u, 0xc8u }, { 0xf7u, 0xc8u }, { 0xfeu, 0xd0u },
        { 0xffu, 0xf8u }
    };
    static const type_unsigned_8 valid[][2] = {
        { 0x8fu, 0xc0u }, { 0xf6u, 0xd0u }, { 0xf7u, 0xd0u },
        { 0xfeu, 0xc0u }, { 0xffu, 0xf0u }
    };
    core_machine_cpu_instruction_lexeme lexeme;
    STD_SIZE_T index;

    for (index = 0u; index != sizeof(invalid) / sizeof(invalid[0]); ++index) {
        if (core_machine_cpu_instruction_lexeme_scan(invalid[index],
                sizeof(invalid[index]), CORE_MACHINE_CPU_PROFILE_80386,
                TYPE_TRUE, &lexeme)) return 0;
    }
    for (index = 0u; index != sizeof(valid) / sizeof(valid[0]); ++index) {
        if (!core_machine_cpu_instruction_lexeme_scan(valid[index],
                sizeof(valid[index]), CORE_MACHINE_CPU_PROFILE_80386,
                TYPE_TRUE, &lexeme) || !lexeme.available) return 0;
    }
    if (!core_machine_cpu_instruction_lexeme_scan(
            (const type_unsigned_8[]){ 0xc6u, 0xc0u, 0x12u }, 3u,
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE, &lexeme) ||
        !core_machine_cpu_instruction_lexeme_scan(
            (const type_unsigned_8[]){ 0xc7u, 0xc0u, 0x78u, 0x56u, 0x34u, 0x12u },
            6u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE, &lexeme)) return 0;
    return 1;
}
static C_INT ud_s1_lexeme_8086_pop_cs(C_VOID)
{
    static const type_unsigned_8 pop_cs[] = { 0x0fu };
    core_machine_cpu_instruction_lexeme lexeme;

    return core_machine_cpu_instruction_lexeme_scan(pop_cs, sizeof(pop_cs),
        CORE_MACHINE_CPU_PROFILE_8086, TYPE_FALSE, &lexeme) && lexeme.available &&
        lexeme.byte_count == 1u && lexeme.component_count == 1u &&
        !core_machine_cpu_instruction_lexeme_scan(pop_cs, sizeof(pop_cs),
            CORE_MACHINE_CPU_PROFILE_80186, TYPE_FALSE, &lexeme);
}
static C_INT ud_s1_primary_metadata_and_lexeme(C_VOID)
{
    static const type_unsigned_8 reserved[] = { 0xf1u };
    core_machine_cpu_instruction_lexeme lexeme;
    core_machine_cpu_instruction_metadata metadata =
        core_machine_cpu_instruction_metadata_get(
            CORE_MACHINE_CPU_INSTRUCTION_PRIMARY, 0xf1u, 0u);

    return !metadata.valid && !core_machine_cpu_instruction_lexeme_scan(
        reserved, sizeof(reserved), CORE_MACHINE_CPU_PROFILE_80386,
        TYPE_TRUE, &lexeme);
}
static core_machine_cpu_profile ud_s1_primary_expected_minimum(type_unsigned_8 opcode)
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

static C_INT ud_s1_primary_metadata_matrix(C_VOID)
{
    type_unsigned_16 value;

    for (value = 0u; value != 0x100u; ++value) {
        type_unsigned_8 opcode = (type_unsigned_8)value;
        core_machine_cpu_instruction_metadata metadata =
            core_machine_cpu_instruction_metadata_get(
                CORE_MACHINE_CPU_INSTRUCTION_PRIMARY, opcode, 0u);
        type_bool reserved = opcode == 0xd6u || opcode == 0xf1u;

        if (metadata.valid == reserved || (!reserved &&
            metadata.minimum_cpu != ud_s1_primary_expected_minimum(opcode))) return 0;
    }
    return 1;
}
static core_machine_cpu_profile ud_s1_0f_expected_minimum(type_unsigned_8 opcode)
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

static C_INT ud_s1_0f_metadata_matrix(C_VOID)
{
    type_unsigned_16 value;

    for (value = 0u; value != 0x100u; ++value) {
        type_unsigned_8 opcode = (type_unsigned_8)value;
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
static C_INT ud_s1_protected_invalid_gate(C_VOID)
{
    static const type_unsigned_8 code[] = { 0x0fu, 0x01u, 0xf8u };
    ud_s1_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    C_INT failed = !ud_s1_boot_protected(&state, code, sizeof(code), TYPE_FALSE);

    if (!failed) {
        before = state.machine->executor_cpu;
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){ 1u, 0u }, &result) != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.eip != before.data.eip || after.data.esp != before.data.esp ||
            after.data.eflags != before.data.eflags ||
            !ud_s1_gprs_same(&before, &after) ||
            !ud_s1_data_sregs_same(&before, &after);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

C_INT main(C_VOID)
{
    static const type_unsigned_8 invalid_primary[] = { 0xf1u };
    static const type_unsigned_8 reserved_0f[] = { 0x0fu, 0x01u, 0xf8u };
    static const type_unsigned_8 reserved_0f25[] = { 0x0fu, 0x25u, 0xc0u };
    static const type_unsigned_8 invalid_operand[] = { 0x62u, 0xc0u };
    static const type_unsigned_8 invalid_lock[] = { 0xf0u, 0x90u };
    const type_unsigned_8 *forms[] = {
        invalid_primary, reserved_0f, reserved_0f25, invalid_operand, invalid_lock
    };
    const STD_SIZE_T sizes[] = {
        sizeof(invalid_primary), sizeof(reserved_0f), sizeof(reserved_0f25), sizeof(invalid_operand),
        sizeof(invalid_lock)
    };
    STD_SIZE_T index;

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
    STD_PRINTF("M5:T326:S1:PROTECTED-UD-DELIVERY:OK\n");
    STD_PRINTF("M5:T401:S2:0F25-METADATA:OK\n");
    STD_PRINTF("M5:T401:S3:0F-METADATA-MATRIX:OK\n");
    STD_PRINTF("M5:T401:S4:F1-METADATA:OK\n");
    STD_PRINTF("M5:T401:S4:PRIMARY-METADATA-MATRIX:OK\n");
    STD_PRINTF("M5:T401:S5:LEXEME-8086-POP-CS:OK\n");
    STD_PRINTF("M5:T401:S5:LEXEME-PRIMARY-GROUPS:OK\n");
    STD_PRINTF("M5:T401:S5:LEXEME-MEMORY-FORMS:OK\n");
    return 0;
}
