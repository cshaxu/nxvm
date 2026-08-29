#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "core/machine/pic.h"
#include "../support/core_machine_cpu_fixture.h"

#define LGDT_LIDT_SOURCE 0x0200u
#define LGDT_LIDT_SS_BASE 0x0040u
#define LGDT_LIDT_ES_BASE 0x0080u
#define LGDT_LIDT_GDT_BASE 0x0300u
#define LGDT_LIDT_IDT_BASE 0x0400u

typedef struct lgdt_lidt_machine {
    core_machine *machine;
} lgdt_lidt_machine;

static C_VOID lgdt_lidt_reset(C_VOID *opaque)
{
    lgdt_lidt_machine *state = (lgdt_lidt_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider lgdt_lidt_provider = {
    lgdt_lidt_reset, STD_NULL
};

static C_INT lgdt_lidt_prepare(lgdt_lidt_machine *state,
    core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    STD_MEMSET(state, 0, sizeof(*state));
    return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &lgdt_lidt_provider, state, &state->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u);
}

static C_VOID lgdt_lidt_enter_protected(lgdt_lidt_machine *state,
    type_unsigned_8 cpl)
{
    t_cpu *cpu = &state->machine->executor_cpu;

    TYPE_SET_BIT(cpu->data.cr0, VCPU_CR0_PE);
    cpu->data.cs.selector = (type_unsigned_16)(0x0008u | cpl);
    cpu->data.cs.base = 0u;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.dpl = cpl;
    cpu->data.cs.flagValid = TYPE_TRUE;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.seg.executable = TYPE_TRUE;
    cpu->data.ds.selector = (type_unsigned_16)(0x0010u | cpl);
    cpu->data.ds.base = 0u;
    cpu->data.ds.limit = 0xffffu;
    cpu->data.ds.dpl = cpl;
    cpu->data.ds.flagValid = TYPE_TRUE;
    cpu->data.ds.sregtype = SREG_DATA;
    cpu->data.ds.seg.data.writable = TYPE_TRUE;
    cpu->data.ss = cpu->data.ds;
    cpu->data.ss.sregtype = SREG_STACK;
}

static C_VOID lgdt_lidt_seed(t_cpu *cpu)
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

static C_INT lgdt_lidt_cpu_same(const t_cpu *before, const t_cpu *after)
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

static C_VOID lgdt_lidt_write_image(type_unsigned_8 *image,
    type_unsigned_16 limit, type_unsigned_32 base)
{
    image[0] = TYPE_MASK_UNSIGNED_8(limit);
    image[1] = TYPE_MASK_UNSIGNED_8(limit >> 8u);
    image[2] = TYPE_MASK_UNSIGNED_8(base);
    image[3] = TYPE_MASK_UNSIGNED_8(base >> 8u);
    image[4] = TYPE_MASK_UNSIGNED_8(base >> 16u);
    image[5] = TYPE_MASK_UNSIGNED_8(base >> 24u);
}

static C_INT lgdt_lidt_run(lgdt_lidt_machine *state,
    const type_unsigned_8 *code, STD_SIZE_T bytes, type_unsigned_32 budget,
    type_status *status, core_machine_run_result *result,
    core_machine_cpu_diagnostic *diagnostic)
{
    return core_machine_memory_write(state->machine, 0u, code, bytes) ==
        TYPE_STATUS_OK && ((*status = core_machine_run(state->machine,
        (core_machine_run_budget){budget, 0u}, result)) == TYPE_STATUS_OK ||
        *status == TYPE_STATUS_FAULT) && core_machine_get_cpu_diagnostic(
        state->machine, diagnostic) == TYPE_STATUS_OK;
}

static C_INT lgdt_lidt_test_success(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 prefixes[] = {0u, 0x66u, 0x67u, 0x66u};
    type_unsigned_8 profile, opcode, mode, form;

    for (profile = 0u; profile != 2u; ++profile)
        for (opcode = 2u; opcode != 4u; ++opcode)
            for (mode = 0u; mode != 2u; ++mode)
                for (form = 0u; form != (profiles[profile] ==
                    CORE_MACHINE_CPU_PROFILE_80386 ? 4u : 1u); ++form) {
                    lgdt_lidt_machine state;
                    t_cpu before, after;
                    core_machine_run_result result;
                    core_machine_cpu_diagnostic diagnostic;
                    type_status status = TYPE_STATUS_INVALID_STATE;
                    type_unsigned_8 code[9] = {0u};
                    type_unsigned_8 image[6];
                    type_unsigned_32 expected_base = opcode == 2u ?
                        0x00123456u : 0x00abcdefu;
                    type_unsigned_16 expected_limit = opcode == 2u ? 0x2468u : 0x1357u;
                    STD_SIZE_T bytes;
                    C_INT failed = !lgdt_lidt_prepare(&state, profiles[profile]);

                    if (!failed && mode) lgdt_lidt_enter_protected(&state, 0u);
                    if (!failed) {
                        lgdt_lidt_seed(&state.machine->executor_cpu);
                        lgdt_lidt_write_image(image, expected_limit, expected_base);
                        if (form == 0u) {
                            code[0] = 0x0fu; code[1] = 0x01u;
                            code[2] = (type_unsigned_8)(0x06u | (opcode << 3u));
                            code[3] = 0x00u; code[4] = 0x02u; bytes = 5u;
                        } else if (form == 1u) {
                            code[0] = prefixes[form]; code[1] = 0x0fu; code[2] = 0x01u;
                            code[3] = (type_unsigned_8)(0x06u | (opcode << 3u));
                            code[4] = 0x00u; code[5] = 0x02u; bytes = 6u;
                        } else if (form == 2u) {
                            code[0] = prefixes[form]; code[1] = 0x0fu; code[2] = 0x01u;
                            code[3] = (type_unsigned_8)(0x05u | (opcode << 3u));
                            code[4] = 0x00u; code[5] = 0x02u; bytes = 8u;
                        } else {
                            code[0] = 0x66u; code[1] = 0x67u; code[2] = 0x0fu;
                            code[3] = 0x01u; code[4] = (type_unsigned_8)(0x05u | (opcode << 3u));
                            code[5] = 0x00u; code[6] = 0x02u; bytes = 9u;
                        }
                        failed |= core_machine_memory_write(state.machine, LGDT_LIDT_SOURCE,
                            image, sizeof(image)) != TYPE_STATUS_OK;
                        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                        failed |= !lgdt_lidt_run(&state, code, bytes, 1u, &status,
                            &result, &diagnostic);
                        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                        failed |= status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
                            after.data.eip != bytes || !lgdt_lidt_cpu_same(&before, &after) ||
                            (opcode == 2u && (after.data.gdtr.limit != expected_limit ||
                            after.data.gdtr.base != (form == 1u || form == 3u ? expected_base :
                            TYPE_MASK_UNSIGNED_24(expected_base)))) ||
                            (opcode == 3u && (after.data.idtr.limit != expected_limit ||
                            after.data.idtr.base != (form == 1u || form == 3u ? expected_base :
                            TYPE_MASK_UNSIGNED_24(expected_base))));
                    }
                    core_machine_destroy(state.machine);
                    if (failed) return 0;
                }
    return 1;
}

static C_INT lgdt_lidt_expect_ud(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, STD_SIZE_T bytes)
{
    lgdt_lidt_machine state;
    t_cpu before, after;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    type_status status = TYPE_STATUS_INVALID_STATE;
    C_INT failed = !lgdt_lidt_prepare(&state, profile);

    if (!failed) {
        lgdt_lidt_seed(&state.machine->executor_cpu);
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !lgdt_lidt_run(&state, code, bytes, 1u, &status, &result, &diagnostic);
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= status != TYPE_STATUS_FAULT || !diagnostic.first_fault.valid ||
            !TYPE_GET_BIT(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            STD_MEMCMP(&before.data, &after.data, sizeof(before.data)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT lgdt_lidt_test_rejections(C_VOID)
{
    static const type_unsigned_8 low[] = {0x0fu,0x01u,0x16u,0x00u,0x02u};
    static const type_unsigned_8 register_form[] = {0x0fu,0x01u,0xd0u};
    static const type_unsigned_8 lock[] = {0xf0u,0x0fu,0x01u,0x16u,0x00u,0x02u};
    static const type_unsigned_8 attributes[][7] = {
        {0x66u,0x0fu,0x01u,0x16u,0x00u,0x02u,0u},
        {0x67u,0x0fu,0x01u,0x16u,0x00u,0x02u,0u},
        {0x66u,0x67u,0x0fu,0x01u,0x16u,0x00u,0x02u}
    };
    type_unsigned_8 opcode, attr;

    for (opcode = 2u; opcode != 4u; ++opcode) {
        type_unsigned_8 code[sizeof(low)], direct[sizeof(register_form)], locked[sizeof(lock)];
        STD_MEMCPY(code, low, sizeof(code));
        STD_MEMCPY(direct, register_form, sizeof(direct));
        STD_MEMCPY(locked, lock, sizeof(locked));
        code[2] = (type_unsigned_8)(0x06u | (opcode << 3u));
        direct[2] = (type_unsigned_8)(0xc0u | (opcode << 3u));
        locked[3] = (type_unsigned_8)(0x06u | (opcode << 3u));
        if (!lgdt_lidt_expect_ud(CORE_MACHINE_CPU_PROFILE_80186, code, sizeof(code)) ||
            !lgdt_lidt_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, direct, sizeof(direct)) ||
            !lgdt_lidt_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, locked, sizeof(locked)))
            return 0;
        for (attr = 0u; attr != 3u; ++attr) {
            type_unsigned_8 attr_code[sizeof(attributes[0])];
            STD_MEMCPY(attr_code, attributes[attr], sizeof(attr_code));
            attr_code[attr == 2u ? 4u : 3u] = (type_unsigned_8)(0x06u | (opcode << 3u));
            if (!lgdt_lidt_expect_ud(CORE_MACHINE_CPU_PROFILE_80286, attr_code,
                attr == 2u ? 7u : 6u)) return 0;
        }
    }
    return 1;
}

static C_INT lgdt_lidt_test_segments(C_VOID)
{
    static const type_unsigned_8 code[][6] = {
        {0x0fu,0x01u,0x56u,0x10u,0u,0u},
        {0x26u,0x0fu,0x01u,0x16u,0x00u,0x03u}
    };
    type_unsigned_8 form, opcode;

    for (opcode = 2u; opcode != 4u; ++opcode) for (form = 0u; form != 2u; ++form) {
        lgdt_lidt_machine state;
        type_unsigned_8 image[6];
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        type_status status = TYPE_STATUS_INVALID_STATE;
        type_unsigned_32 address = form == 0u ? LGDT_LIDT_SS_BASE * 16u + 0x0030u :
            LGDT_LIDT_ES_BASE * 16u + 0x0300u;
        C_INT failed = !lgdt_lidt_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        if (!failed) {
            if (form == 0u)
                failed |= core_machine_cpu_execution_load_segment(&state.machine->executor_cpu_execution,
                    &state.machine->executor_cpu.data.ss, LGDT_LIDT_SS_BASE) != 0;
            else
                failed |= core_machine_cpu_execution_load_segment(&state.machine->executor_cpu_execution,
                    &state.machine->executor_cpu.data.es, LGDT_LIDT_ES_BASE) != 0;
            lgdt_lidt_seed(&state.machine->executor_cpu);
            state.machine->executor_cpu.data.ebp = 0x0020u;
            lgdt_lidt_write_image(image, 0x1357u, 0x00abcdefu);
            failed |= core_machine_memory_write(state.machine, address, image, sizeof(image)) != TYPE_STATUS_OK;
            {
                type_unsigned_8 local[sizeof(code[0])];
                STD_MEMCPY(local, code[form], sizeof(local));
                local[form == 0u ? 2u : 3u] |= opcode << 3u;
                failed |= !lgdt_lidt_run(&state, local, form == 0u ? 4u : 6u, 1u,
                    &status, &result, &diagnostic) || status != TYPE_STATUS_OK ||
                    diagnostic.first_fault.valid;
            }
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT lgdt_lidt_test_source_limit(C_VOID)
{
    type_unsigned_8 opcode;

    for (opcode = 2u; opcode != 4u; ++opcode) {
        lgdt_lidt_machine state;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before, after;
        type_unsigned_8 code[] = {0x0fu,0x01u,0x16u,0x00u,0x02u};
        type_unsigned_8 source[6];
        type_status status = TYPE_STATUS_INVALID_STATE;
        C_INT failed = !lgdt_lidt_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            lgdt_lidt_enter_protected(&state, 0u);
            lgdt_lidt_seed(&state.machine->executor_cpu);
            state.machine->executor_cpu.data.ds.limit = 0x0203u;
            code[2] = (type_unsigned_8)(0x06u | (opcode << 3u));
            lgdt_lidt_write_image(source, 0x1357u, 0x00abcdefu);
            state.machine->executor_cpu.data.gdtr.base = 0x11111111u;
            state.machine->executor_cpu.data.gdtr.limit = 0x1111u;
            state.machine->executor_cpu.data.idtr.base = 0u;
            state.machine->executor_cpu.data.idtr.limit = 0u;
            failed |= core_machine_memory_write(state.machine, LGDT_LIDT_SOURCE,
                source, sizeof(source)) != TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !lgdt_lidt_run(&state, code, sizeof(code), 1u, &status,
                &result, &diagnostic);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= status != TYPE_STATUS_FAULT || !diagnostic.first_fault.valid ||
                !TYPE_GET_BIT(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                after.data.eip != 0u || !lgdt_lidt_cpu_same(&before, &after) ||
                after.data.gdtr.base != before.data.gdtr.base ||
                after.data.gdtr.limit != before.data.gdtr.limit ||
                after.data.idtr.base != before.data.idtr.base ||
                after.data.idtr.limit != before.data.idtr.limit;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT lgdt_lidt_test_gdtr_consumer(C_VOID)
{
    static const type_unsigned_8 code[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x02u, 0x8eu, 0xd8u
    };
    static const type_unsigned_8 data_descriptor[] = {
        0xffu, 0x0fu, 0x00u, 0x40u, 0x00u, 0x92u, 0x00u, 0x00u
    };
    lgdt_lidt_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before, after;
    type_unsigned_8 image[6];
    type_status status = TYPE_STATUS_INVALID_STATE;
    C_INT failed = !lgdt_lidt_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        lgdt_lidt_enter_protected(&state, 0u);
        lgdt_lidt_seed(&state.machine->executor_cpu);
        state.machine->executor_cpu.data.eax = 0x11220008u;
        lgdt_lidt_write_image(image, 0x000fu, 0x00000500u);
        failed |= core_machine_memory_write(state.machine, LGDT_LIDT_SOURCE,
            image, sizeof(image)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x0508u, data_descriptor,
            sizeof(data_descriptor)) != TYPE_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !lgdt_lidt_run(&state, code, sizeof(code), 2u, &status,
            &result, &diagnostic);
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 2u ||
            after.data.eip != sizeof(code) || after.data.gdtr.limit != 0x000fu ||
            after.data.gdtr.base != 0x00000500u || after.data.ds.selector != 0x0008u ||
            after.data.ds.base != 0x00004000u || after.data.ds.limit != 0x00000fffu ||
            !after.data.ds.flagValid || after.data.ds.sregtype != SREG_DATA ||
            !after.data.ds.seg.data.writable || after.data.eax != before.data.eax ||
            after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
            after.data.ebx != before.data.ebx || after.data.esp != before.data.esp ||
            after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi || after.data.eflags != before.data.eflags ||
            STD_MEMCMP(&before.data.cs, &after.data.cs, sizeof(before.data.cs)) != 0 ||
            STD_MEMCMP(&before.data.es, &after.data.es, sizeof(before.data.es)) != 0 ||
            STD_MEMCMP(&before.data.ss, &after.data.ss, sizeof(before.data.ss)) != 0 ||
            STD_MEMCMP(&before.data.fs, &after.data.fs, sizeof(before.data.fs)) != 0 ||
            STD_MEMCMP(&before.data.gs, &after.data.gs, sizeof(before.data.gs)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT lgdt_lidt_test_pending_pic(C_VOID)
{
    static const type_unsigned_8 hlt = 0xf4u;
    type_unsigned_8 opcode;

    for (opcode = 2u; opcode != 4u; ++opcode) {
        lgdt_lidt_machine state;
        core_machine_pic_irq_source irq;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before, after;
        type_unsigned_8 code[] = {0x0fu,0x01u,0x16u,0x00u,0x02u,0x90u};
        type_unsigned_8 image[6];
        type_unsigned_16 vector_offset = 0x0100u, vector_segment = 0u, frame = 0u;
        type_status status = TYPE_STATUS_INVALID_STATE;
        C_INT failed = !lgdt_lidt_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            code[2] = (type_unsigned_8)(0x06u | (opcode << 3u));
            lgdt_lidt_seed(&state.machine->executor_cpu);
            lgdt_lidt_write_image(image, opcode == 2u ? 0x2468u : 0x03ffu,
                opcode == 2u ? 0x00123456u : 0u);
            failed |= core_machine_memory_write(state.machine, LGDT_LIDT_SOURCE,
                image, sizeof(image)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x0080u, &vector_offset,
                sizeof(vector_offset)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x0082u, &vector_segment,
                sizeof(vector_segment)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x0100u, &hlt,
                sizeof(hlt)) != TYPE_STATUS_OK || core_machine_memory_write(
                state.machine, 0u, code, sizeof(code)) != TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            STD_MEMSET(&irq, 0, sizeof(irq));
            state.machine->shared_pic_master.data.icw2 = 0x20u;
            core_machine_pic_irq_source_bind(&irq, &state.machine->shared_pic_master,
                &state.machine->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(&irq);
            core_machine_pic_irq_source_deassert(&irq);
            status = core_machine_run(state.machine, (core_machine_run_budget){2u,0u},
                &result);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK || status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
                after.data.eip != 0x0101u || after.data.eflags !=
                (before.data.eflags & ~VCPU_EFLAGS_IF) ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                after.data.ss.base + (type_unsigned_16)after.data.esp,
                TYPE_REFERENCE_OF(frame), sizeof(frame)) != TYPE_STATUS_OK || frame != 5u ||
                !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u)) || TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,
                VPIC_IRR_IRQ(0u));
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}
static C_INT lgdt_lidt_boot_protected(lgdt_lidt_machine *state,
    core_machine_cpu_profile profile)
{
    static const type_unsigned_8 gdt_pointer[] = {
        0x3fu,0x00u,0x00u,0x03u,0x00u,0x00u
    };
    static const type_unsigned_8 idt_pointer[] = {
        0x07u,0x01u,0x00u,0x04u,0x00u,0x00u
    };
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x40u,0,0x92u,0,0,
        0xffu,0xffu,0,0x50u,0,0x92u,0,0,
        0xffu,0xffu,0,0x60u,0,0x92u,0,0,
        0xffu,0xffu,0,0x70u,0,0x92u,0,0,
        0xffu,0xffu,0,0x20u,0,0xfau,0,0
    };
    static const type_unsigned_8 real_code_286[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,0x0fu,0x01u,0x1eu,0x10u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd0u,
        0xb8u,0x18u,0x00u,0x8eu,0xd8u,
        0xb8u,0x20u,0x00u,0x8eu,0xc0u,
        0xeau,0x00u,0x00u,0x08u,0x00u
    };
    static const type_unsigned_8 real_code_386[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,0x0fu,0x01u,0x1eu,0x10u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd0u,
        0xb8u,0x18u,0x00u,0x8eu,0xd8u,
        0xb8u,0x20u,0x00u,0x8eu,0xc0u,
        0xb8u,0x28u,0x00u,0x8eu,0xe0u,
        0xb8u,0x30u,0x00u,0x8eu,0xe8u,
        0xeau,0x00u,0x00u,0x08u,0x00u
    };
    type_unsigned_8 idt[0x108u] = {0u};
    core_machine_run_result result;
    const type_unsigned_8 *real_code = profile == CORE_MACHINE_CPU_PROFILE_80286 ?
        real_code_286 : real_code_386;
    STD_SIZE_T real_code_bytes = profile == CORE_MACHINE_CPU_PROFILE_80286 ?
        sizeof(real_code_286) : sizeof(real_code_386);

    idt[13u * 8u + 1u] = 0x01u;
    idt[13u * 8u + 2u] = 0x38u;
    idt[13u * 8u + 5u] = profile == CORE_MACHINE_CPU_PROFILE_80286 ?
        0x86u : 0xeeu;
    return lgdt_lidt_prepare(state, profile) &&
        core_machine_memory_write(state->machine, 0x0100u, gdt_pointer,
            sizeof(gdt_pointer)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, LGDT_LIDT_GDT_BASE, gdt,
            sizeof(gdt)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x0110u, idt_pointer,
            sizeof(idt_pointer)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, LGDT_LIDT_IDT_BASE, idt,
            sizeof(idt)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, real_code,
            real_code_bytes) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x2100u,
            (const type_unsigned_8[]){0x90u}, 1u) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x2000u,
            (const type_unsigned_8[]){0xf4u}, 1u) == TYPE_STATUS_OK &&
        core_machine_run(state->machine, (core_machine_run_budget){64u,0u},
            &result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static C_INT lgdt_lidt_test_protected_cpl_reject(C_VOID)
{
    static const type_unsigned_8 forms[][5] = {
        {0x0fu,0x01u,0x16u,0x00u,0x04u},
        {0x0fu,0x01u,0x1eu,0x00u,0x04u}
    };
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 opcode, profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) for (opcode = 0u; opcode != 2u; ++opcode) {
        lgdt_lidt_machine state;
        core_machine_run_result result, handler_result;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before, after;
        type_unsigned_8 source[6] = {0x5au,0x5au,0x5au,0x5au,0x5au,0x5au};
        type_unsigned_8 observed[6];
        C_INT failed = !lgdt_lidt_boot_protected(&state, profiles[profile]);

        if (!failed) {
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            state.machine->executor_cpu.data.cs.selector = 0x003bu;
            state.machine->executor_cpu.data.cs.dpl = 3u;
            state.machine->executor_cpu.data.cs.base = 0x2000u;
            state.machine->executor_cpu.data.cs.limit = 0xffffu;
            state.machine->executor_cpu.data.cs.flagValid = TYPE_TRUE;
            state.machine->executor_cpu.data.cs.sregtype = SREG_CODE;
            state.machine->executor_cpu.data.cs.seg.executable = TYPE_TRUE;
            lgdt_lidt_seed(&state.machine->executor_cpu);
            failed |= core_machine_memory_write(state.machine, 0x2000u, forms[opcode],
                sizeof(forms[opcode])) != TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_write(state.machine, 0x3400u, source,
                sizeof(source)) != TYPE_STATUS_OK;
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){1u,0u}, &result) != TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= result.reason != CORE_MACHINE_STOP_BUDGET || after.data.eip != 0x0100u;
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){1u,0u}, &handler_result) != TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= handler_result.reason != CORE_MACHINE_STOP_BUDGET;
            failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                !diagnostic.last_delivered_exception.valid ||
                !TYPE_GET_BIT(diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_GP) || diagnostic.last_delivered_exception.point.eip != 0u ||
                after.data.eip != 0x0101u || after.data.gdtr.base != before.data.gdtr.base ||
                after.data.gdtr.limit != before.data.gdtr.limit ||
                after.data.idtr.base != before.data.idtr.base ||
                after.data.idtr.limit != before.data.idtr.limit ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                0x3400u, (type_virtual_address)observed, sizeof(observed)) != TYPE_STATUS_OK ||
                STD_MEMCMP(source, observed, sizeof(source)) != 0;
        }
        if (failed) {
            core_machine_destroy(state.machine);
            return 0;
        }
        core_machine_destroy(state.machine);
    }
    return 1;
}
C_INT main(C_VOID)
{
    if (!lgdt_lidt_test_success()) {
        STD_PRINTF("LGDT/LIDT stage=success\n"); return 1;
    }
    if (!lgdt_lidt_test_rejections()) {
        STD_PRINTF("LGDT/LIDT stage=rejections\n"); return 1;
    }
    if (!lgdt_lidt_test_segments()) {
        STD_PRINTF("LGDT/LIDT stage=segments\n"); return 1;
    }
    if (!lgdt_lidt_test_pending_pic()) {
        STD_PRINTF("LGDT/LIDT stage=pending-pic\n"); return 1;
    }
    if (!lgdt_lidt_test_source_limit()) {
        STD_PRINTF("LGDT/LIDT stage=source-limit\n"); return 1;
    }
    if (!lgdt_lidt_test_gdtr_consumer()) {
        STD_PRINTF("LGDT/LIDT stage=gdtr-consumer\n"); return 1;
    }
    if (!lgdt_lidt_test_protected_cpl_reject()) {
        STD_PRINTF("LGDT/LIDT stage=protected-cpl-reject\n"); return 1;
    }
    STD_PRINTF("M5:T319:S1:LGDT-LIDT:OK\n");
    return 0;
}
