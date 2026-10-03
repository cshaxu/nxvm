#include "support/cpu_board_limit_fixture.h"
#include "app-nxvm/devices/machine_board_interface.h"
#include "app-nxvm/devices/machine_board_state.h"
#include "support/pic_fixture.h"
#include "app-nxvm/devices/debug_interface.h"
#include "app-nxvm/devices/pic_bus.h"
#include <stdio.h>

static void imul_seed_patch(core_machine_debug_register_patch *patch)
{
    patch->mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDX) |
        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBX) |
        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBP) |
        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESI) |
        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDI) |
        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS);
    patch->values[CORE_MACHINE_DEBUG_EAX] = 0xa1a10000u;
    patch->values[CORE_MACHINE_DEBUG_ECX] = 0xb2b2fffeu;
    patch->values[CORE_MACHINE_DEBUG_EDX] = 0xc3c30000u;
    patch->values[CORE_MACHINE_DEBUG_EBX] = 0xd4d40000u;
    patch->values[CORE_MACHINE_DEBUG_ESP] = 0x00008000u;
    patch->values[CORE_MACHINE_DEBUG_EBP] = 0xe5e50000u;
    patch->values[CORE_MACHINE_DEBUG_ESI] = 0x00004000u;
    patch->values[CORE_MACHINE_DEBUG_EDI] = 0xf6f60000u;
    patch->values[CORE_MACHINE_DEBUG_EFLAGS] =
        VCPU_EFLAGS_IF | VCPU_EFLAGS_DF | VCPU_EFLAGS_CF;
}

static lib_i32 imul_sregs_same(const core_machine_debug_cpu_snapshot *before,
    const core_machine_debug_cpu_snapshot *after)
{
    return lib_memory_compare(&before->es, &after->es,
            sizeof(before->es)) == 0 &&
        lib_memory_compare(&before->cs, &after->cs,
            sizeof(before->cs)) == 0 &&
        lib_memory_compare(&before->ss, &after->ss,
            sizeof(before->ss)) == 0 &&
        lib_memory_compare(&before->ds, &after->ds,
            sizeof(before->ds)) == 0 &&
        lib_memory_compare(&before->fs, &after->fs,
            sizeof(before->fs)) == 0 &&
        lib_memory_compare(&before->gs, &after->gs,
            sizeof(before->gs)) == 0;
}

static lib_i32 imul_nonparticipants_same(
    const core_machine_debug_cpu_snapshot *before,
    const core_machine_debug_cpu_snapshot *after)
{
    return before->ecx == after->ecx && before->edx == after->edx &&
        before->ebx == after->ebx && before->esp == after->esp &&
        before->ebp == after->ebp && before->esi == after->esi &&
        before->edi == after->edi;
}

static lib_i32 imul_protected_source_limits(void)
{
    static const lib_u8 ds_code[] = {
        0x69u, 0x06u, 0x10u, 0u, 0xfeu, 0xffu
    };
    core_machine *machine = LIB_NULL;
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_debug_cpu_snapshot before = {0}, after = {0};
    core_machine_debug_register_patch patch = {0};
    lib_i32 failed = !test_cpu_board_limit_prepare(&machine, ds_code,
        sizeof(ds_code), LIB_TRUE, LIB_TRUE);

    if (!failed) {
        imul_seed_patch(&patch);
        failed = core_machine_debug_patch_registers(machine, &patch) !=
                LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) !=
                LIB_STATUS_OK ||
            core_machine_run(machine, (core_machine_run_budget){1u, 0u},
                &result) != LIB_STATUS_INTERNAL_ERROR ||
            result.reason != CORE_MACHINE_STOP_FAULT ||
            core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
                LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) !=
                LIB_STATUS_OK ||
            !diagnostic.first_fault.valid ||
            !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_DF) || after.eip != 0u ||
            after.eax != before.eax ||
            !imul_nonparticipants_same(&before, &after) ||
            after.eflags != before.eflags ||
            !imul_sregs_same(&before, &after);
    }
    core_machine_destroy(machine);
    return !failed;
}

static lib_i32 imul_irq_no_shadow(void)
{
    static const lib_u8 register_code[] = {
        0x69u, 0xc1u, 0xfeu, 0xffu, 0x90u
    };
    static const lib_u8 memory_code[] = {
        0x69u, 0x06u, 0x00u, 0x40u, 0xfeu, 0xffu, 0x90u
    };
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const lib_u16 vector_offset = 0x0100u, vector_segment = 0u;
    const lib_u16 source = 0xfffeu;
    const lib_u8 halt = 0xf4u;
    lib_u8 form;

    for (form = 0u; form < 2u; ++form) {
        const lib_u8 *code = form ? memory_code : register_code;
        const lib_u8 instruction_bytes = form ? 6u : 4u;
        core_machine *machine = LIB_NULL;
        core_machine_board_state *board = LIB_NULL;
        core_machine_pic_irq_source source_irq = {0};
        core_machine_run_result result = {0};
        core_machine_debug_cpu_snapshot before = {0}, after = {0};
        core_machine_debug_register_patch patch = {0};
        lib_u16 frame_ip = 0u, frame_flags = 0u;
        lib_i32 failed = core_machine_create(&config, &machine, &board) !=
                LIB_STATUS_OK ||
            core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            core_machine_reset(machine) != LIB_STATUS_OK;

        if (!failed) {
            imul_seed_patch(&patch);
            patch.mask |= CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP);
            failed |= core_machine_debug_patch_registers(machine, &patch) !=
                    LIB_STATUS_OK ||
                core_machine_debug_capture_cpu_snapshot(machine,
                    CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) !=
                    LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0u, code,
                    form ? sizeof(memory_code) : sizeof(register_code)) !=
                    LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x20u * 4u,
                    &vector_offset, sizeof(vector_offset)) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x20u * 4u + 2u,
                    &vector_segment, sizeof(vector_segment)) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x0100u, &halt,
                    sizeof(halt)) != LIB_STATUS_OK;
            if (form)
                failed |= core_machine_memory_write(machine, 0x4000u,
                    &source, sizeof(source)) != LIB_STATUS_OK;
        }
        if (!failed) {
            test_pic_program_vector(&board->shared_pic_master, 0x20u);
            core_machine_pic_irq_source_bind(&source_irq,
                &board->shared_pic_master, &board->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(&source_irq);
            core_machine_pic_irq_source_deassert(&source_irq);
            failed = core_machine_run(machine,
                    (core_machine_run_budget){2u, 0u}, &result) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
                core_machine_debug_capture_cpu_snapshot(machine,
                    CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
                core_machine_memory_read(machine,
                    after.ss.base + (lib_u16)after.esp,
                    &frame_ip, sizeof(frame_ip)) != LIB_STATUS_OK ||
                core_machine_memory_read(machine,
                    after.ss.base + (lib_u16)after.esp + 4u,
                    &frame_flags, sizeof(frame_flags)) != LIB_STATUS_OK ||
                after.eip != 0x0101u ||
                after.eax != ((before.eax & 0xffff0000u) | 4u) ||
                after.ecx != before.ecx || after.edx != before.edx ||
                after.ebx != before.ebx || after.ebp != before.ebp ||
                after.esi != before.esi || after.edi != before.edi ||
                !imul_sregs_same(&before, &after) ||
                frame_ip != instruction_bytes ||
                (frame_flags & ~(VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
                    VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF |
                    VCPU_EFLAGS_PF)) !=
                    ((before.eflags & ~(VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
                    VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF |
                    VCPU_EFLAGS_PF | VCPU_EFLAGS_RESERVED)) | 0x02u) ||
                CORE_MACHINE_BIT_IS_SET(frame_flags, VCPU_EFLAGS_CF) ||
                CORE_MACHINE_BIT_IS_SET(frame_flags, VCPU_EFLAGS_OF) ||
                !CORE_MACHINE_BIT_IS_SET(
                    test_pic_read(&board->shared_pic_master, 0x0bu),
                    VPIC_ISR_IRQ(0u)) ||
                CORE_MACHINE_BIT_IS_SET(
                    test_pic_read(&board->shared_pic_master, 0x0au),
                    VPIC_IRR_IRQ(0u));
        }
        core_machine_destroy(machine);
        if (failed) return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!imul_protected_source_limits()) {
        fprintf(stderr, "IMUL protected source limit failed\n");
        return 1;
    }
    if (!imul_irq_no_shadow()) {
        fprintf(stderr, "IMUL IRQ/no-shadow failed\n");
        return 1;
    }
    printf("M5:T316:S56:IMUL-IMM:OK\n");
    printf("M5:T401:S30:IMUL-IMMEDIATE-PROFILES:OK\n");
    return 0;
}
