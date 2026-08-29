#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define PFT_GDT_POINTER 0x0100u
#define PFT_GDT_ADDRESS 0x0300u
#define PFT_CODE_ADDRESS 0x2000u
#define PFT_DATA_ADDRESS 0x3000u
#define PFT_TARGET_ADDRESS 0x4000u

typedef struct pft_machine {
    core_machine *machine;
} pft_machine;

static C_VOID pft_reset(C_VOID *opaque)
{
    pft_machine *state = (pft_machine *)opaque;

    if (state != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static const core_machine_execution_provider pft_provider = {
    pft_reset, STD_NULL
};

static C_INT pft_write(pft_machine *state, type_unsigned_32 address,
    const C_VOID *bytes, STD_SIZE_T byte_count)
{
    return state != STD_NULL && state->machine != STD_NULL &&
        core_machine_memory_write(state->machine, address, bytes, byte_count) ==
            TYPE_STATUS_OK;
}

static C_INT pft_read_private(pft_machine *state, type_unsigned_32 address,
    C_VOID *bytes, STD_SIZE_T byte_count)
{
    return state != STD_NULL && state->machine != STD_NULL &&
        core_machine_memory_read_physical(&state->machine->executor_memory,
            address, (type_virtual_address)bytes, byte_count) == TYPE_STATUS_OK;
}

static C_INT pft_prepare(pft_machine *state, core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    static const type_unsigned_8 gdt_pointer[] = {
        0x3fu,0u,0u,0x03u,0u,0u
    };
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x40u,0,0x9au,0,0,
        0xffu,0xffu,0,0x50u,0,0x9eu,0,0,
        0xffu,0xffu,0,0x60u,0,0xfau,0,0,
        0xffu,0xffu,0,0x70u,0,0x1au,0,0,
        0xffu,0xffu,0,0x80u,0,0x92u,0,0
    };
    static const type_unsigned_8 real_code[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,
        0x8eu,0xd0u,0xbcu,0x00u,0x80u,
        0xeau,0x00u,0x00u,0x08u,0x00u
    };
    static const type_unsigned_8 halt[] = {0xf4u};
    const core_machine_run_budget budget = {96u,0u};
    core_machine_run_result result;

    if (state == STD_NULL) return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(state->machine,
            &pft_provider, state) ||
        !pft_write(state, PFT_GDT_POINTER, gdt_pointer, sizeof(gdt_pointer)) ||
        !pft_write(state, PFT_GDT_ADDRESS, gdt, sizeof(gdt)) ||
        !pft_write(state, 0u, real_code, sizeof(real_code)) ||
        !pft_write(state, PFT_CODE_ADDRESS, halt, sizeof(halt)) ||
        core_machine_run(state->machine, budget, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT pft_run(pft_machine *state, const type_unsigned_8 *code,
    STD_SIZE_T code_size, core_machine_stop_reason expected_reason, t_cpu *out_cpu)
{
    const core_machine_run_budget budget = {64u,0u};
    core_machine_run_result result;
    type_status status;

    if (!pft_write(state, PFT_CODE_ADDRESS, code, code_size)) return 0;
    test_core_machine_fixture_resume_after_halt_at(state->machine, 0u);
    status = core_machine_run(state->machine, budget, &result);
    if ((expected_reason == CORE_MACHINE_STOP_FAULT && status != TYPE_STATUS_FAULT) ||
        (expected_reason != CORE_MACHINE_STOP_FAULT && status != TYPE_STATUS_OK) ||
        result.reason != expected_reason) return 0;
    *out_cpu = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return 1;
}

static C_INT pft_cpu_unchanged(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi &&
        before->data.eip == after->data.eip &&
        before->data.eflags == after->data.eflags &&
        STD_MEMCMP(&before->data.es, &after->data.es, sizeof(before->data.es)) == 0 &&
        STD_MEMCMP(&before->data.cs, &after->data.cs, sizeof(before->data.cs)) == 0 &&
        STD_MEMCMP(&before->data.ss, &after->data.ss, sizeof(before->data.ss)) == 0 &&
        STD_MEMCMP(&before->data.ds, &after->data.ds, sizeof(before->data.ds)) == 0 &&
        STD_MEMCMP(&before->data.fs, &after->data.fs, sizeof(before->data.fs)) == 0 &&
        STD_MEMCMP(&before->data.gs, &after->data.gs, sizeof(before->data.gs)) == 0;
}

static C_INT pft_expect_fault(pft_machine *state, const type_unsigned_8 *code,
    STD_SIZE_T code_size, type_unsigned_32 expected_mask, t_cpu *out_cpu)
{
    core_machine_cpu_diagnostic diagnostic;

    return pft_run(state, code, code_size, CORE_MACHINE_STOP_FAULT, out_cpu) &&
        core_machine_get_cpu_diagnostic(state->machine, &diagnostic) == TYPE_STATUS_OK &&
        diagnostic.first_fault.valid &&
        diagnostic.first_fault.exception_mask == expected_mask;
}

static C_INT pft_test_success(core_machine_cpu_profile profile)
{
    static const type_unsigned_8 halt[] = {0xf4u};
    static const type_unsigned_8 jmp16[] = {0xeau,0,0,0x18u,0};
    static const type_unsigned_8 call16[] = {0x9au,0,0,0x18u,0};
    static const type_unsigned_8 jmp_indirect[] = {0xffu,0x2eu,0,1};
    static const type_unsigned_8 call_indirect[] = {0xffu,0x1eu,0,1};
    static const type_unsigned_8 pointer[] = {0,0,0x18u,0};
    pft_machine state;
    t_cpu before;
    t_cpu after;
    type_unsigned_16 frame[2] = {0u,0u};
    C_INT failed = !pft_prepare(&state, profile);

    if (!failed) {
        failed |= !pft_write(&state, PFT_TARGET_ADDRESS, halt, sizeof(halt));
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !pft_run(&state, jmp16, sizeof(jmp16),
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) ||
            after.data.cs.selector != 0x18u || after.data.cs.base != PFT_TARGET_ADDRESS ||
            after.data.eip != 1u || after.data.esp != before.data.esp ||
            after.data.eflags != before.data.eflags;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    failed = !pft_prepare(&state, profile);
    if (!failed) {
        failed |= !pft_write(&state, PFT_TARGET_ADDRESS, halt, sizeof(halt));
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !pft_run(&state, call16, sizeof(call16),
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) ||
            after.data.cs.selector != 0x18u || after.data.eip != 1u ||
            after.data.esp != (before.data.esp & 0xffff0000u) + 0x7ffcu ||
            !pft_read_private(&state, PFT_DATA_ADDRESS + 0x7ffcu, frame,
                sizeof(frame)) || frame[0] != sizeof(call16) || frame[1] != 0x08u;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    failed = !pft_prepare(&state, profile);
    if (!failed) {
        failed |= !pft_write(&state, PFT_TARGET_ADDRESS, halt, sizeof(halt)) ||
            !pft_write(&state, PFT_DATA_ADDRESS + 0x100u, pointer, sizeof(pointer)) ||
            !pft_run(&state, jmp_indirect, sizeof(jmp_indirect),
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) ||
            after.data.cs.selector != 0x18u || after.data.eip != 1u;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    failed = !pft_prepare(&state, profile);
    if (!failed) {
        failed |= !pft_write(&state, PFT_TARGET_ADDRESS, halt, sizeof(halt)) ||
            !pft_write(&state, PFT_DATA_ADDRESS + 0x100u, pointer, sizeof(pointer)) ||
            !pft_run(&state, call_indirect, sizeof(call_indirect),
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) ||
            after.data.cs.selector != 0x18u || after.data.eip != 1u ||
            !pft_read_private(&state, PFT_DATA_ADDRESS + 0x7ffcu, frame,
                sizeof(frame)) || frame[0] != sizeof(call_indirect) ||
            frame[1] != 0x08u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT pft_test_386_attributes(C_VOID)
{
    static const type_unsigned_8 halt[] = {0xf4u};
    static const type_unsigned_8 jmp32[] = {0x66u,0xeau,0x00u,0x01u,0,0,0x18u,0};
    static const type_unsigned_8 call32[] = {0x66u,0x9au,0x00u,0x01u,0,0,0x18u,0};
    static const type_unsigned_8 pointer16[] = {0x00u,0x01u,0x18u,0};
    static const type_unsigned_8 pointer32[] = {0x00u,0x01u,0,0,0x18u,0};
    static const type_unsigned_8 jmp_indirect67[] = {
        0x67u,0xffu,0x2du,0x00u,0x01u,0,0
    };
    static const type_unsigned_8 jmp_indirect32[] = {
        0x66u,0x67u,0xffu,0x2du,0x00u,0x01u,0,0
    };
    static const type_unsigned_8 call_indirect32[] = {
        0x66u,0x67u,0xffu,0x1du,0x00u,0x01u,0,0
    };
    static const type_unsigned_8 lock_jmp[] = {0xf0u,0xeau,0,0,0x18u,0};
    static const type_unsigned_8 lock_call[] = {0xf0u,0x9au,0,0,0x18u,0};
    static const type_unsigned_8 pre386[] = {0x66u,0xeau,0,0,0,0,0x18u,0};
    static const type_unsigned_8 pre386_call[] = {0x66u,0x9au,0,0,0,0,0x18u,0};
    pft_machine state;
    t_cpu before;
    t_cpu after;
    C_INT failed = !pft_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed |= !pft_write(&state, PFT_TARGET_ADDRESS + 0x100u, halt,
            sizeof(halt)) || !pft_run(&state, jmp32, sizeof(jmp32),
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) ||
            after.data.cs.selector != 0x18u || after.data.eip != 0x101u;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    failed = !pft_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) {
        failed |= !pft_write(&state, PFT_TARGET_ADDRESS + 0x100u, halt,
            sizeof(halt)) || !pft_run(&state, call32, sizeof(call32),
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) ||
            after.data.cs.selector != 0x18u || after.data.eip != 0x101u ||
            after.data.esp != 0x00007ff8u;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    failed = !pft_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) {
        failed |= !pft_write(&state, PFT_TARGET_ADDRESS + 0x100u, halt,
            sizeof(halt)) || !pft_write(&state, PFT_DATA_ADDRESS + 0x100u,
            pointer16, sizeof(pointer16)) || !pft_run(&state, jmp_indirect67,
            sizeof(jmp_indirect67), CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) ||
            after.data.cs.selector != 0x18u || after.data.eip != 0x101u;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    failed = !pft_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) {
        failed |= !pft_write(&state, PFT_TARGET_ADDRESS + 0x100u, halt,
            sizeof(halt)) || !pft_write(&state, PFT_DATA_ADDRESS + 0x100u,
            pointer32, sizeof(pointer32)) || !pft_run(&state, call_indirect32,
            sizeof(call_indirect32), CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) ||
            after.data.cs.selector != 0x18u || after.data.eip != 0x101u ||
            after.data.esp != 0x00007ff8u;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    failed = !pft_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) {
        failed |= !pft_write(&state, PFT_TARGET_ADDRESS + 0x100u, halt,
            sizeof(halt)) || !pft_write(&state, PFT_DATA_ADDRESS + 0x100u,
            pointer32, sizeof(pointer32)) || !pft_run(&state, jmp_indirect32,
            sizeof(jmp_indirect32), CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) ||
            after.data.cs.selector != 0x18u || after.data.eip != 0x101u;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    failed = !pft_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        before.data.eip = 0u;
        failed |= !pft_expect_fault(&state, lock_jmp, sizeof(lock_jmp),
            VCPUINS_EXCEPT_UD, &after) ||
            !pft_cpu_unchanged(&before, &after);
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    failed = !pft_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286);
    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        before.data.eip = 0u;
        failed |= !pft_expect_fault(&state, pre386, sizeof(pre386),
            VCPUINS_EXCEPT_UD, &after) ||
            !pft_cpu_unchanged(&before, &after);
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    failed = !pft_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        before.data.eip = 0u;
        failed |= !pft_expect_fault(&state, lock_call, sizeof(lock_call),
            VCPUINS_EXCEPT_UD, &after) || !pft_cpu_unchanged(&before, &after);
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    failed = !pft_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286);
    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        before.data.eip = 0u;
        failed |= !pft_expect_fault(&state, pre386_call, sizeof(pre386_call),
            VCPUINS_EXCEPT_UD, &after) || !pft_cpu_unchanged(&before, &after);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT pft_test_descriptor_rejections(C_VOID)
{
    static const type_unsigned_8 call_dpl[] = {0x9au,0,0,0x28u,0};
    static const type_unsigned_8 jmp_rpl[] = {0xeau,0,0,0x1bu,0};
    static const type_unsigned_8 call_nonpresent[] = {0x9au,0,0,0x30u,0};
    static const type_unsigned_8 jmp_data[] = {0xeau,0,0,0x38u,0};
    static const type_unsigned_8 jmp_conforming[] = {0xeau,0,0,0x20u,0};
    static const type_unsigned_8 call_conforming[] = {0x9au,0,0,0x20u,0};
    static const type_unsigned_8 halt[] = {0xf4u};
    const type_unsigned_8 *const rejected[] = {
        call_dpl,jmp_rpl,call_nonpresent,jmp_data
    };
    const STD_SIZE_T sizes[] = {
        sizeof(call_dpl),sizeof(jmp_rpl),sizeof(call_nonpresent),sizeof(jmp_data)
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(rejected) / sizeof(rejected[0]); ++index) {
        pft_machine state;
        t_cpu before;
        t_cpu after;
        C_INT failed = !pft_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            before.data.eip = 0u;
            failed = !pft_expect_fault(&state, rejected[index], sizes[index],
                VCPUINS_EXCEPT_DF, &after) ||
                !pft_cpu_unchanged(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        pft_machine state;
        t_cpu after;
        C_INT failed = !pft_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            failed |= !pft_write(&state, PFT_TARGET_ADDRESS + 0x1000u, halt,
                sizeof(halt)) ||
                !pft_run(&state, jmp_conforming, sizeof(jmp_conforming),
                    CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) ||
                after.data.cs.selector != 0x20u || after.data.cs.seg.exec.conform != TYPE_TRUE;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        pft_machine state;
        t_cpu after;
        C_INT failed = !pft_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            failed |= !pft_write(&state, PFT_TARGET_ADDRESS + 0x1000u, halt,
                sizeof(halt)) || !pft_run(&state, call_conforming,
                sizeof(call_conforming), CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT,
                &after) || after.data.cs.selector != 0x20u ||
                after.data.cs.seg.exec.conform != TYPE_TRUE ||
                after.data.esp != 0x00007ffcu;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT pft_test_preflight_faults(C_VOID)
{
    static const type_unsigned_8 jmp_limit[] = {0xeau,1,0,0x18u,0};
    static const type_unsigned_8 call_stack[] = {0x9au,0,0,0x18u,0};
    type_unsigned_8 zero_limit[] = {0u,0u};
    type_unsigned_32 sentinel = 0x11223344u;
    pft_machine state;
    t_cpu before;
    t_cpu after;
    C_INT failed = !pft_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed |= !pft_write(&state, PFT_GDT_ADDRESS + 0x18u, zero_limit,
            sizeof(zero_limit));
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        before.data.eip = 0u;
        failed |= !pft_expect_fault(&state, jmp_limit, sizeof(jmp_limit),
            VCPUINS_EXCEPT_DF, &after) || !pft_cpu_unchanged(&before, &after);
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    failed = !pft_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) {
        state.machine->executor_cpu.data.ss.limit = 1u;
        failed |= !pft_write(&state, PFT_DATA_ADDRESS + 0x7ffcu, &sentinel,
            sizeof(sentinel));
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        before.data.eip = 0u;
        failed |= !pft_expect_fault(&state, call_stack, sizeof(call_stack),
            VCPUINS_EXCEPT_DF, &after) || !pft_cpu_unchanged(&before, &after) ||
            !pft_read_private(&state, PFT_DATA_ADDRESS + 0x7ffcu, &sentinel,
                sizeof(sentinel)) || sentinel != 0x11223344u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT pft_test_irq_no_shadow(C_VOID)
{
    static const type_unsigned_8 jmp[] = {0xeau,0,0,0x18u,0,0x90u};
    static const type_unsigned_8 target[] = {0x90u,0xf4u};
    static const type_unsigned_8 handler[] = {0xf4u};
    static const type_unsigned_8 gate[] = {0,1,8,0,0,0x86u,0,0};
    pft_machine state;
    core_machine_pic_irq_source source;
    core_machine_run_result result;
    t_cpu after;
    type_unsigned_16 frame_ip = 0u;
    C_INT failed = !pft_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed |= !pft_write(&state, PFT_TARGET_ADDRESS, target, sizeof(target)) ||
            !pft_write(&state, PFT_CODE_ADDRESS + 0x100u, handler,
                sizeof(handler)) || !pft_write(&state, 0x0600u + 0x100u, gate,
                sizeof(gate)) || !pft_write(&state, PFT_CODE_ADDRESS, jmp,
                sizeof(jmp));
        state.machine->executor_cpu.data.idtr.flagValid = TYPE_TRUE;
        state.machine->executor_cpu.data.idtr.sregtype = SREG_IDTR;
        state.machine->executor_cpu.data.idtr.base = 0x0600u;
        state.machine->executor_cpu.data.idtr.limit = 0x0107u;
        state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        STD_MEMSET(&source, 0, sizeof(source));
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&source, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&source);
        core_machine_pic_irq_source_deassert(&source);
        failed |= core_machine_run(state.machine, (core_machine_run_budget){2u,0u},
            &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !pft_read_private(&state, PFT_DATA_ADDRESS +
            (type_unsigned_16)after.data.esp, &frame_ip, sizeof(frame_ip)) ||
            after.data.cs.selector != 0x08u || after.data.eip != 0x101u ||
            frame_ip != 0u || !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u)) || TYPE_GET_BIT(
                state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

C_INT main(C_VOID)
{
    if (!pft_test_success(CORE_MACHINE_CPU_PROFILE_80286)) {
        STD_PRINTF("PFT stage=80286\n");
        return 1;
    }
    if (!pft_test_success(CORE_MACHINE_CPU_PROFILE_80386)) {
        STD_PRINTF("PFT stage=80386\n");
        return 1;
    }
    if (!pft_test_386_attributes()) {
        STD_PRINTF("PFT stage=attributes\n");
        return 1;
    }
    if (!pft_test_descriptor_rejections()) {
        STD_PRINTF("PFT stage=descriptors\n");
        return 1;
    }
    if (!pft_test_preflight_faults()) {
        STD_PRINTF("PFT stage=preflight\n");
        return 1;
    }
    if (!pft_test_irq_no_shadow()) {
        STD_PRINTF("PFT stage=irq\n");
        return 1;
    }
    STD_PRINTF("M5:T323:S1:PROTECTED-FAR:OK\n");
    return 0;
}
