#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "core/machine/pic.h"
#include "../support/core_machine_cpu_fixture.h"

#define SGDT_SIDT_IMAGE_BYTES 6u
#define SGDT_SIDT_DS_ADDRESS 0x0200u
#define SGDT_SIDT_SS_BASE 0x0040u
#define SGDT_SIDT_SS_OFFSET 0x0030u
#define SGDT_SIDT_ES_BASE 0x0080u

typedef struct sgdt_sidt_machine {
    core_machine *machine;
} sgdt_sidt_machine;

static C_VOID sgdt_sidt_reset(C_VOID *opaque)
{
    sgdt_sidt_machine *state = (sgdt_sidt_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider sgdt_sidt_provider = {
    sgdt_sidt_reset, STD_NULL
};

static C_INT sgdt_sidt_prepare(sgdt_sidt_machine *state,
    core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    STD_MEMSET(state, 0, sizeof(*state));
    return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &sgdt_sidt_provider, state, &state->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u);
}

static C_VOID sgdt_sidt_enter_protected(sgdt_sidt_machine *state, C_INT vm86)
{
    t_cpu *cpu = &state->machine->executor_cpu;

    TYPE_SET_BIT(cpu->data.cr0, VCPU_CR0_PE);
    cpu->data.cs.selector = 0x0008u;
    cpu->data.cs.base = 0u;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.dpl = 0u;
    cpu->data.cs.flagValid = TYPE_TRUE;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.seg.executable = TYPE_TRUE;
    cpu->data.ds.selector = 0x0010u;
    cpu->data.ds.base = 0u;
    cpu->data.ds.limit = 0xffffu;
    cpu->data.ds.dpl = 0u;
    cpu->data.ds.flagValid = TYPE_TRUE;
    cpu->data.ds.sregtype = SREG_DATA;
    cpu->data.ds.seg.data.writable = TYPE_TRUE;
    if (vm86)
        TYPE_SET_BIT(cpu->data.eflags, VCPU_EFLAGS_VM);
}

static C_VOID sgdt_sidt_seed(t_cpu *cpu)
{
    cpu->data.eax = 0x11223344u;
    cpu->data.ecx = 0x55667788u;
    cpu->data.edx = 0x99aabbccu;
    cpu->data.ebx = 0xddeeff00u;
    cpu->data.esp = 0x00007000u;
    cpu->data.ebp = 0x00000020u;
    cpu->data.esi = 0x01020304u;
    cpu->data.edi = 0x05060708u;
    cpu->data.eflags = VCPU_EFLAGS_IF | VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
}

static C_INT sgdt_sidt_gprs_flags_sregs_same(const t_cpu *before,
    const t_cpu *after)
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
        STD_MEMCMP(&before->data.cs, &after->data.cs, sizeof(before->data.cs)) == 0 &&
        STD_MEMCMP(&before->data.ds, &after->data.ds, sizeof(before->data.ds)) == 0 &&
        STD_MEMCMP(&before->data.es, &after->data.es, sizeof(before->data.es)) == 0 &&
        STD_MEMCMP(&before->data.ss, &after->data.ss, sizeof(before->data.ss)) == 0 &&
        STD_MEMCMP(&before->data.fs, &after->data.fs, sizeof(before->data.fs)) == 0 &&
        STD_MEMCMP(&before->data.gs, &after->data.gs, sizeof(before->data.gs)) == 0;
}

static C_INT sgdt_sidt_run(sgdt_sidt_machine *state,
    const type_unsigned_8 *code, STD_SIZE_T bytes, type_unsigned_32 budget,
    type_status *out_status, core_machine_run_result *out_result,
    core_machine_cpu_diagnostic *out_diagnostic)
{
    return core_machine_memory_write(state->machine, 0u, code, bytes) ==
        TYPE_STATUS_OK && ((*out_status = core_machine_run(state->machine,
        (core_machine_run_budget){budget, 0u}, out_result)) == TYPE_STATUS_OK ||
        *out_status == TYPE_STATUS_FAULT) && core_machine_get_cpu_diagnostic(
        state->machine, out_diagnostic) == TYPE_STATUS_OK;
}

static C_VOID sgdt_sidt_expected_image(type_unsigned_8 *image,
    type_unsigned_16 limit, type_unsigned_32 base, C_INT operand32,
    core_machine_cpu_profile profile)
{
    image[0] = TYPE_MASK_UNSIGNED_8(limit);
    image[1] = TYPE_MASK_UNSIGNED_8(limit >> 8u);
    image[2] = TYPE_MASK_UNSIGNED_8(base);
    image[3] = TYPE_MASK_UNSIGNED_8(base >> 8u);
    image[4] = TYPE_MASK_UNSIGNED_8(base >> 16u);
    image[5] = profile == CORE_MACHINE_CPU_PROFILE_80286 ? 0xffu :
        (operand32 ? TYPE_MASK_UNSIGNED_8(base >> 24u) : 0u);
}

static C_INT sgdt_sidt_test_profiles_modes_and_attributes(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 forms[][9] = {
        {0x0fu,0x01u,0x06u,0x00u,0x02u,0u,0u,0u,0u},
        {0x66u,0x0fu,0x01u,0x06u,0x00u,0x02u,0u,0u,0u},
        {0x67u,0x0fu,0x01u,0x05u,0x00u,0x02u,0x00u,0x00u,0u},
        {0x66u,0x67u,0x0fu,0x01u,0x05u,0x00u,0x02u,0x00u,0x00u}
    };
    static const type_unsigned_8 lengths[] = {5u,6u,8u,9u};
    type_unsigned_8 profile, protected_mode, table, form;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
        for (protected_mode = 0u; protected_mode != 2u; ++protected_mode)
            for (table = 0u; table != 2u; ++table)
                for (form = 0u; form != (profiles[profile] ==
                    CORE_MACHINE_CPU_PROFILE_80386 ? 4u : 1u); ++form) {
                    sgdt_sidt_machine state;
                    t_cpu before, after;
                    core_machine_run_result result;
                    core_machine_cpu_diagnostic diagnostic;
                    type_unsigned_8 code[9], image[SGDT_SIDT_IMAGE_BYTES], expected[SGDT_SIDT_IMAGE_BYTES];
                    type_status status = TYPE_STATUS_INVALID_STATE;
                    type_unsigned_32 base = table ? 0x89abcdefu : 0x12345678u;
                    type_unsigned_16 limit = table ? 0x1357u : 0x2468u;
                    C_INT failed = !sgdt_sidt_prepare(&state, profiles[profile]);

                    STD_MEMCPY(code, forms[form], lengths[form]);
                    code[form == 0u ? 2u : form == 1u ? 3u : form == 2u ? 3u : 4u] |= table << 3u;
                    if (!failed && protected_mode)
                        sgdt_sidt_enter_protected(&state, 0);
                    if (!failed) {
                        sgdt_sidt_seed(&state.machine->executor_cpu);
                        state.machine->executor_cpu.data.gdtr.base = 0x12345678u;
                        state.machine->executor_cpu.data.gdtr.limit = 0x2468u;
                        state.machine->executor_cpu.data.idtr.base = 0x89abcdefu;
                        state.machine->executor_cpu.data.idtr.limit = 0x1357u;
                        STD_MEMSET(image, 0xa5, sizeof(image));
                        failed |= core_machine_memory_write(state.machine,
                            SGDT_SIDT_DS_ADDRESS, image, sizeof(image)) != TYPE_STATUS_OK;
                        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                        failed |= !sgdt_sidt_run(&state, code, lengths[form], 1u,
                            &status, &result, &diagnostic);
                        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                        sgdt_sidt_expected_image(expected, limit, base,
                            form == 1u || form == 3u, profiles[profile]);
                        failed |= status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
                            after.data.eip != lengths[form] || !sgdt_sidt_gprs_flags_sregs_same(
                            &before, &after) || core_machine_memory_read_physical(
                            &state.machine->executor_memory, SGDT_SIDT_DS_ADDRESS,
                            (type_virtual_address)image, sizeof(image)) != TYPE_STATUS_OK ||
                            STD_MEMCMP(image, expected, sizeof(image)) != 0;
                    }
                    core_machine_destroy(state.machine);
                    if (failed) return 0;
                }
    return 1;
}

static C_INT sgdt_sidt_test_address_segments_and_vm86(C_VOID)
{
    const type_unsigned_8 lengths[] = {5u,4u,6u};
    const type_unsigned_32 addresses[] = {0x0200u,
        SGDT_SIDT_SS_BASE * 16u + SGDT_SIDT_SS_OFFSET, SGDT_SIDT_ES_BASE * 16u + 0x0300u};
    type_unsigned_8 form, table;

    for (table = 0u; table != 2u; ++table) for (form = 0u; form != 3u; ++form) {
        sgdt_sidt_machine state;
        t_cpu before, after;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_8 code[6] = {0};
        type_unsigned_8 image[SGDT_SIDT_IMAGE_BYTES], expected[SGDT_SIDT_IMAGE_BYTES];
        type_status status = TYPE_STATUS_INVALID_STATE;
        C_INT failed = !sgdt_sidt_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed && form == 1u)
            failed |= core_machine_cpu_execution_load_segment(&state.machine->executor_cpu_execution,
                &state.machine->executor_cpu.data.ss, SGDT_SIDT_SS_BASE) != 0;
        if (!failed && form == 2u)
            failed |= core_machine_cpu_execution_load_segment(&state.machine->executor_cpu_execution,
                &state.machine->executor_cpu.data.es, SGDT_SIDT_ES_BASE) != 0;
        if (!failed) {
            sgdt_sidt_seed(&state.machine->executor_cpu);
            state.machine->executor_cpu.data.ebp = 0x0020u;
            state.machine->executor_cpu.data.gdtr.limit = 0x2468u;
            state.machine->executor_cpu.data.gdtr.base = 0x12345678u;
            state.machine->executor_cpu.data.idtr.limit = 0xabceu;
            state.machine->executor_cpu.data.idtr.base = 0x00c0ffeeu;
            if (form == 0u) {
                code[0] = 0x0fu; code[1] = 0x01u;
                code[2] = (type_unsigned_8)(0x06u | (table << 3u));
                code[3] = 0x00u; code[4] = 0x02u;
            } else if (form == 1u) {
                code[0] = 0x0fu; code[1] = 0x01u;
                code[2] = (type_unsigned_8)(0x46u | (table << 3u));
                code[3] = 0x10u;
            } else {
                code[0] = 0x26u; code[1] = 0x0fu; code[2] = 0x01u;
                code[3] = (type_unsigned_8)(0x06u | (table << 3u));
                code[4] = 0x00u; code[5] = 0x03u;
            }
            STD_MEMSET(image, 0x5a, sizeof(image));
            failed |= core_machine_memory_write(state.machine, addresses[form], image,
                sizeof(image)) != TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !sgdt_sidt_run(&state, code, lengths[form], 1u, &status,
                &result, &diagnostic);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            sgdt_sidt_expected_image(expected, table ? 0xabceu : 0x2468u,
                table ? 0x00c0ffeeu : 0x12345678u, 0,
                CORE_MACHINE_CPU_PROFILE_80386);
            failed |= status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != lengths[form] || !sgdt_sidt_gprs_flags_sregs_same(&before,
                &after) || core_machine_memory_read_physical(&state.machine->executor_memory,
                addresses[form], (type_virtual_address)image, sizeof(image)) != TYPE_STATUS_OK ||
                STD_MEMCMP(image, expected, sizeof(image)) != 0;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    for (table = 0u; table != 2u; ++table) {
        sgdt_sidt_machine state;
        t_cpu before, after;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        type_status status = TYPE_STATUS_INVALID_STATE;
        type_unsigned_8 code[] = {0x0fu,0x01u,0x06u,0x00u,0x02u};
        type_unsigned_8 image[SGDT_SIDT_IMAGE_BYTES], expected[SGDT_SIDT_IMAGE_BYTES];
        C_INT failed = !sgdt_sidt_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            sgdt_sidt_seed(&state.machine->executor_cpu);
            sgdt_sidt_enter_protected(&state, 1);
            state.machine->executor_cpu.data.gdtr.limit = 0x9876u;
            state.machine->executor_cpu.data.gdtr.base = 0x12345678u;
            state.machine->executor_cpu.data.idtr.limit = 0xabceu;
            state.machine->executor_cpu.data.idtr.base = 0x00c0ffeeu;
            code[2] |= table << 3u;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !sgdt_sidt_run(&state, code, sizeof(code), 1u, &status,
                &result, &diagnostic) || core_machine_memory_read_physical(
                &state.machine->executor_memory, SGDT_SIDT_DS_ADDRESS,
                (type_virtual_address)image, sizeof(image)) != TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            sgdt_sidt_expected_image(expected, table ? 0xabceu : 0x9876u,
                table ? 0x00c0ffeeu : 0x12345678u, 0,
                CORE_MACHINE_CPU_PROFILE_80386);
            failed |= status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != sizeof(code) || !sgdt_sidt_gprs_flags_sregs_same(
                &before, &after) || STD_MEMCMP(image, expected, sizeof(image)) != 0;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT sgdt_sidt_expect_ud(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, STD_SIZE_T bytes)
{
    sgdt_sidt_machine state;
    t_cpu before, after;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    type_status status = TYPE_STATUS_INVALID_STATE;
    C_INT failed = !sgdt_sidt_prepare(&state, profile);

    if (!failed) {
        sgdt_sidt_seed(&state.machine->executor_cpu);
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !sgdt_sidt_run(&state, code, bytes, 1u, &status, &result, &diagnostic);
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= status != TYPE_STATUS_FAULT || !diagnostic.first_fault.valid ||
            !TYPE_GET_BIT(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            STD_MEMCMP(&before.data, &after.data, sizeof(before.data)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT sgdt_sidt_test_rejections_and_atomicity(C_VOID)
{
    static const type_unsigned_8 register_form[] = {0x0fu,0x01u,0xc0u};
    static const type_unsigned_8 reserved_form[] = {0x0fu,0x01u,0x2eu,0x00u,0x02u};
    static const type_unsigned_8 lock_form[] = {0xf0u,0x0fu,0x01u,0x06u,0x00u,0x02u};
    static const type_unsigned_8 sgdt[] = {0x0fu,0x01u,0x06u,0x0eu,0x00u};
    type_unsigned_8 table;
    for (table = 0u; table != 2u; ++table) {
        type_unsigned_8 low_profile_code[sizeof(sgdt)];

        STD_MEMCPY(low_profile_code, sgdt, sizeof(low_profile_code));
        low_profile_code[2] |= table << 3u;
        if (!sgdt_sidt_expect_ud(CORE_MACHINE_CPU_PROFILE_80186,
            low_profile_code, sizeof(low_profile_code))) return 0;
    }
    if (!sgdt_sidt_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, reserved_form,
        sizeof(reserved_form))) return 0;
    for (table = 0u; table != 2u; ++table) {
        type_unsigned_8 register_code[sizeof(register_form)];
        type_unsigned_8 lock_code[sizeof(lock_form)];

        STD_MEMCPY(register_code, register_form, sizeof(register_code));
        STD_MEMCPY(lock_code, lock_form, sizeof(lock_code));
        register_code[2] |= table << 3u;
        lock_code[3] |= table << 3u;
        if (!sgdt_sidt_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, register_code,
            sizeof(register_code)) || !sgdt_sidt_expect_ud(
            CORE_MACHINE_CPU_PROFILE_80386, lock_code, sizeof(lock_code))) return 0;
    }
    for (table = 0u; table != 2u; ++table) {
        sgdt_sidt_machine state;
        t_cpu before, after;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_8 code[sizeof(sgdt)];
        type_unsigned_8 image[SGDT_SIDT_IMAGE_BYTES];
        type_status status = TYPE_STATUS_INVALID_STATE;
        C_INT failed = !sgdt_sidt_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            sgdt_sidt_enter_protected(&state, 0);
            state.machine->executor_cpu.data.ds.limit = 0x0011u;
            sgdt_sidt_seed(&state.machine->executor_cpu);
            state.machine->executor_cpu.data.gdtr.limit = 0x2468u;
            state.machine->executor_cpu.data.gdtr.base = 0x12345678u;
            state.machine->executor_cpu.data.idtr.limit = 0xabceu;
            state.machine->executor_cpu.data.idtr.base = 0x00c0ffeeu;
            STD_MEMCPY(code, sgdt, sizeof(code));
            code[2] |= table << 3u;
            STD_MEMSET(image, 0x3c, sizeof(image));
            failed |= core_machine_memory_write(state.machine, 0x000eu, image,
                sizeof(image)) != TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !sgdt_sidt_run(&state, code, sizeof(code), 1u, &status,
                &result, &diagnostic);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= status != TYPE_STATUS_FAULT || !diagnostic.first_fault.valid ||
                after.data.eip != 0u || !sgdt_sidt_gprs_flags_sregs_same(&before,
                &after) || core_machine_memory_read_physical(&state.machine->executor_memory,
                0x000eu, (type_virtual_address)image, sizeof(image)) != TYPE_STATUS_OK ||
                image[0] != 0x3cu || image[1] != 0x3cu || image[2] != 0x3cu ||
                image[3] != 0x3cu || image[4] != 0x3cu || image[5] != 0x3cu;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT sgdt_sidt_test_pending_pic(C_VOID)
{
    static const type_unsigned_8 hlt = 0xf4u;
    type_unsigned_8 table;

    for (table = 0u; table != 2u; ++table) {
        sgdt_sidt_machine state;
        core_machine_pic_irq_source irq;
        core_machine_run_result result;
        t_cpu before, after;
        type_unsigned_8 code[] = {0x0fu,0x01u,0x06u,0x00u,0x02u,0x90u};
        type_unsigned_8 image[SGDT_SIDT_IMAGE_BYTES], expected[SGDT_SIDT_IMAGE_BYTES];
        type_unsigned_16 vector_offset = 0x0100u, vector_segment = 0u, frame = 0u;
        type_status status = TYPE_STATUS_INVALID_STATE;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !sgdt_sidt_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        sgdt_sidt_seed(&state.machine->executor_cpu);
        state.machine->executor_cpu.data.gdtr.limit = 0x2468u;
        state.machine->executor_cpu.data.gdtr.base = 0x12345678u;
        state.machine->executor_cpu.data.idtr.limit = 0x03ffu;
        state.machine->executor_cpu.data.idtr.base = 0u;
        code[2] |= table << 3u;
        failed |= core_machine_memory_write(state.machine, 0x0080u, &vector_offset,
            sizeof(vector_offset)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x0082u, &vector_segment, sizeof(vector_segment)) !=
            TYPE_STATUS_OK || core_machine_memory_write(state.machine, 0x0100u, &hlt,
            sizeof(hlt)) != TYPE_STATUS_OK || core_machine_memory_write(state.machine,
            0u, code, sizeof(code)) != TYPE_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        STD_MEMSET(&irq, 0, sizeof(irq));
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&irq, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&irq);
        core_machine_pic_irq_source_deassert(&irq);
        status = core_machine_run(state.machine, (core_machine_run_budget){2u,0u}, &result);
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
            SGDT_SIDT_DS_ADDRESS, (type_virtual_address)image, sizeof(image)) != TYPE_STATUS_OK;
        sgdt_sidt_expected_image(expected, table ? 0x03ffu : 0x2468u,
            table ? 0u : 0x12345678u, 0,
            CORE_MACHINE_CPU_PROFILE_80386);
        failed |= status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT || after.data.eip != 0x0101u ||
            after.data.eflags != (before.data.eflags & ~VCPU_EFLAGS_IF) ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
            after.data.ss.base + (type_unsigned_16)after.data.esp,
            TYPE_REFERENCE_OF(frame), sizeof(frame)) != TYPE_STATUS_OK || frame != 5u ||
            !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr, VPIC_ISR_IRQ(0u)) ||
            TYPE_GET_BIT(state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u)) ||
            STD_MEMCMP(image, expected, sizeof(image)) != 0;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    }
    return 1;
}

/* MS-DOS uses this observable 80286-versus-80386 discriminator: the 80286
 * writes FF to SGDT's sixth byte, whereas the 80386 does not.  Keep the full
 * caller sequence here so the boot diagnosis is not inferred from a helper. */
static C_INT sgdt_sidt_test_dos_cpu_discriminator(C_VOID)
{
    static const type_unsigned_8 code[] = {
        0x9cu, 0x58u,             /* pushf; pop ax */
        0x25u, 0x00u, 0xf0u,      /* and ax,f000h */
        0x3du, 0x00u, 0xf0u,      /* cmp ax,f000h */
        0x75u, 0x00u,             /* jnz discriminator */
        0xc8u, 0x06u, 0x00u, 0x00u, /* enter 6,0 */
        0x0fu, 0x01u, 0x46u, 0xfau, /* sgdt [bp-6] */
        0x80u, 0x7eu, 0xffu, 0xffu, /* cmp byte [bp-1],ffh */
        0xc9u,                    /* leave */
        0xbau, 0x86u, 0x03u,      /* mov dx,0386h */
        0x75u, 0x03u,             /* jnz not-286 */
        0xbau, 0x86u, 0x02u,      /* mov dx,0286h */
        0xf4u                     /* hlt */
    };
    static const struct {
        core_machine_cpu_profile profile;
        type_unsigned_16 expected_dx;
    } cases[] = {
        { CORE_MACHINE_CPU_PROFILE_80286, 0x0286u },
        { CORE_MACHINE_CPU_PROFILE_80386, 0x0386u }
    };
    type_unsigned_8 index;

    for (index = 0u; index != sizeof(cases) / sizeof(cases[0]); ++index) {
        sgdt_sidt_machine state;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        type_status status = TYPE_STATUS_INVALID_STATE;
        C_INT failed = !sgdt_sidt_prepare(&state, cases[index].profile);

        if (!failed) {
            state.machine->executor_cpu.data.esp = 0x7000u;
            state.machine->executor_cpu.data.ebp = 0x0200u;
            failed |= !sgdt_sidt_run(&state, code, sizeof(code), 32u, &status,
                &result, &diagnostic) || status != TYPE_STATUS_OK ||
                diagnostic.first_fault.valid || result.reason !=
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
                state.machine->executor_cpu.data.dx != cases[index].expected_dx;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

C_INT main(C_VOID)
{
    if (!sgdt_sidt_test_profiles_modes_and_attributes()) {
        STD_PRINTF("SGDT/SIDT stage=profiles-modes-attributes\n"); return 1;
    }
    if (!sgdt_sidt_test_address_segments_and_vm86()) {
        STD_PRINTF("SGDT/SIDT stage=address-segments-vm86\n"); return 1;
    }
    if (!sgdt_sidt_test_rejections_and_atomicity()) {
        STD_PRINTF("SGDT/SIDT stage=rejections-atomicity\n"); return 1;
    }
    if (!sgdt_sidt_test_pending_pic()) {
        STD_PRINTF("SGDT/SIDT stage=pending-pic\n"); return 1;
    }
    if (!sgdt_sidt_test_dos_cpu_discriminator()) {
        STD_PRINTF("SGDT/SIDT stage=dos-cpu-discriminator\n"); return 1;
    }
    STD_PRINTF("M5:T318:S1:SGDT-SIDT:OK\n");
    return 0;
}
