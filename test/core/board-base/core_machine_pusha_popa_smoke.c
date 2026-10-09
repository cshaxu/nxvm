#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "pic_fixture.h"
#include "core/board-base/machine_board_interface.h"
#include "core/board-base/machine_board_state.h"
#include "core_machine_board_fixture.h"
#include "core/x86/device_support_interface.h"
#include "core/board-base/pic_bus_interface.h"
typedef struct pusha_popa_machine {
    core_machine *machine;
    core_machine_board_state *board;
} pusha_popa_machine;

static lib_i32 pusha_popa_prepare(core_machine_cpu_profile profile, pusha_popa_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile, .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP)
    };
    lib_memory_set(state, 0, sizeof(*state));
    return core_machine_create(&config, &state->machine, &state->board) == LIB_STATUS_OK &&
        core_machine_freeze_execution_providers(state->machine) == LIB_STATUS_OK &&
        core_machine_reset(state->machine) == LIB_STATUS_OK &&
        core_machine_debug_patch_registers(state->machine, &entry) == LIB_STATUS_OK;
}

static lib_status pusha_popa_seed(pusha_popa_machine *state, lib_u32 esp, lib_u32 eax)
{
    const core_machine_debug_register_patch registers = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESI) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDI) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
        .values = {
            [CORE_MACHINE_DEBUG_EAX] = eax, [CORE_MACHINE_DEBUG_ECX] = 0xb1b25566u,
            [CORE_MACHINE_DEBUG_EDX] = 0xc1c27788u, [CORE_MACHINE_DEBUG_EBX] = 0xd1d299aau,
            [CORE_MACHINE_DEBUG_ESP] = esp, [CORE_MACHINE_DEBUG_EBP] = 0xe1e2bbcdu,
            [CORE_MACHINE_DEBUG_ESI] = 0xf1f2ddefu, [CORE_MACHINE_DEBUG_EDI] = 0x1122a5a5u,
            [CORE_MACHINE_DEBUG_EFLAGS] = 0x245u
        }
    };
    return core_machine_debug_patch_registers(state->machine, &registers);
}

static lib_i32 pusha_popa_boot_protected(pusha_popa_machine *state, lib_u8 limit_segment,
    lib_u16 limit, lib_bool expdown)
{
    static const lib_u8 pointer[] = {0x1fu, 0u, 0u, 0x03u, 0u, 0u};
    lib_u8 gdt[] = {
        0u,0u,0u,0u,0u,0u,0u,0u,
        0xffu,0xffu,0u,0x20u,0u,0x9au,0u,0u,
        0xffu,0xffu,0u,0x30u,0u,0x92u,0u,0u,
        0xffu,0xffu,0u,0x40u,0u,0x92u,0u,0u
    };
    static const lib_u8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,0xb8u,0x18u,0x00u,0x8eu,
        0xd0u,0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    core_machine_run_result result;

    lib_i32 ready = core_machine_memory_write(state->machine, 0x0100u, pointer,
        sizeof(pointer)) == LIB_STATUS_OK && core_machine_memory_write(
        state->machine, 0x0300u, gdt, sizeof(gdt)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, bootstrap,
        sizeof(bootstrap)) == LIB_STATUS_OK &&
        core_machine_run(state->machine, (core_machine_run_budget){10u, 0u},
        &result) == LIB_STATUS_OK && result.reason ==
        CORE_MACHINE_STOP_BUDGET && result.executed == 10u;

    const core_machine_debug_register reg = limit_segment == 0u ?
        CORE_MACHINE_DEBUG_SS : CORE_MACHINE_DEBUG_DS;
    const core_machine_debug_register_patch segment = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(reg),
        .values = {
            [CORE_MACHINE_DEBUG_SS] = 0x18u,
            [CORE_MACHINE_DEBUG_DS] = 0x10u
        }
    };
    lib_u8 descriptor = limit_segment == 0u ? 24u : 16u;

    /* Prepare the original invalid-stack precondition at the paused boundary.
     * Executing MOV SS with that ESP would fault before the tested instruction. */
    if (!ready) return 0;
    gdt[descriptor] = (lib_u8)limit;
    gdt[descriptor + 1u] = (lib_u8)(limit >> 8);
    if (limit_segment == 0u && expdown) gdt[descriptor + 5u] = 0x96u;
    return core_machine_memory_write(state->machine, 0x0300u, gdt,
        sizeof(gdt)) == LIB_STATUS_OK &&
        core_machine_debug_patch_registers(state->machine, &segment) == LIB_STATUS_OK;
}

static lib_i32 pusha_popa_sregs_same(const core_machine_debug_cpu_snapshot *before, const core_machine_debug_cpu_snapshot *after)
{
    return lib_memory_compare(&before->es, &after->es,
        sizeof(before->es)) == 0 && lib_memory_compare(&before->cs,
        &after->cs, sizeof(before->cs)) == 0 && lib_memory_compare(
        &before->ss, &after->ss, sizeof(before->ss)) == 0 &&
        lib_memory_compare(&before->ds, &after->ds, sizeof(before->ds)) ==
        0 && lib_memory_compare(&before->fs, &after->fs,
        sizeof(before->fs)) == 0 && lib_memory_compare(&before->gs,
        &after->gs, sizeof(before->gs)) == 0;
}

static lib_i32 pusha_popa_read_image(pusha_popa_machine *state, lib_u32 address,
    lib_u8 width, lib_u32 *value)
{
    *value = 0u;
    return core_machine_memory_inspect(state->machine,
        address, (void *)CORE_MACHINE_REFERENCE_OF(*value), width) == LIB_STATUS_OK;
}

static lib_i32 pusha_popa_expect_push_image(pusha_popa_machine *state,
    const core_machine_debug_cpu_snapshot *before, lib_u32 stack, lib_u8 width)
{
    const lib_u32 expected[] = {
        before->edi, before->esi, before->ebp,
        width == 2u ? (lib_u16)before->esp : before->esp,
        before->ebx, before->edx, before->ecx, before->eax
    };
    lib_u8 slot;

    for (slot = 0u; slot != sizeof(expected) / sizeof(expected[0]); ++slot)
    {
        lib_u32 image;

        if (!pusha_popa_read_image(state, stack + slot * width, width, &image) ||
            image != (width == 2u ? (expected[slot] & 0xffffu) : expected[slot]))
            return 0;
    }
    return 1;
}

static lib_i32 pusha_popa_test_protected_pusha_limit(void)
{
    static const lib_u8 code[] = {0x60u};
    pusha_popa_machine state;
    core_machine_debug_cpu_snapshot before;
    core_machine_debug_cpu_snapshot after;
    core_machine_run_result result;
    lib_u16 expected[] = {0xa001u, 0xa002u, 0xa003u, 0xa004u, 0xa005u};
    lib_u8 slot;
    lib_i32 failed = !pusha_popa_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed)
        failed |= !pusha_popa_boot_protected(&state, 0u, 0x18u, LIB_TRUE);
    if (!failed)
    {
        failed |= pusha_popa_seed(&state, 0x12340022u, 0xa1a23344u) != LIB_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x4018u, expected,
            sizeof(expected)) != LIB_STATUS_OK || core_machine_memory_write(
            state.machine, 0x2000u, code, sizeof(code)) != LIB_STATUS_OK;
        failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
        failed |= !test_core_machine_fixture_shutdown_wait(core_machine_run(
            state.machine, (core_machine_run_budget){1u, 0u}, &result), &result);
        failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
        if (!failed)
            failed |= after.eip != 0u || after.eax != before.eax ||
            after.ecx != before.ecx || after.edx != before.edx ||
            after.ebx != before.ebx || after.ebp != before.ebp ||
            after.esi != before.esi || after.edi != before.edi ||
            after.eflags != before.eflags || after.esp != before.esp ||
            !pusha_popa_sregs_same(&before, &after);
        for (slot = 0u; !failed && slot != sizeof(expected) / sizeof(expected[0]); ++slot)
        {
            lib_u32 value;

            failed |= !pusha_popa_read_image(&state, 0x4018u + slot * 2u, 2u,
                &value) || value != expected[slot];
        }
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 pusha_popa_test_protected_popa_limit(void)
{
    static const lib_u8 code[] = {0x61u};
    static const lib_u16 image[] = {0x1111u, 0x2222u, 0x3333u, 0x4444u,
        0x5555u, 0x6666u, 0x7777u, 0x8888u};
    pusha_popa_machine state;
    core_machine_debug_cpu_snapshot before;
    core_machine_debug_cpu_snapshot after;
    core_machine_run_result result;
    lib_u8 slot;
    lib_i32 failed = !pusha_popa_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed)
        failed |= !pusha_popa_boot_protected(&state, 0u, 0x1fu, LIB_FALSE);
    if (!failed)
    {
        failed |= pusha_popa_seed(&state, 0x12340018u, 0xa1a23344u) != LIB_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x4018u, image,
            sizeof(image)) != LIB_STATUS_OK || core_machine_memory_write(
            state.machine, 0x2000u, code, sizeof(code)) != LIB_STATUS_OK;
        failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
        failed |= !test_core_machine_fixture_shutdown_wait(core_machine_run(
            state.machine, (core_machine_run_budget){1u, 0u}, &result), &result);
        failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
        if (!failed)
            failed |= after.eip != 0u || after.eax != before.eax ||
            after.ecx != before.ecx || after.edx != before.edx ||
            after.ebx != before.ebx || after.esp != before.esp ||
            after.ebp != before.ebp || after.esi != before.esi ||
            after.edi != before.edi ||
            after.eflags != before.eflags || !pusha_popa_sregs_same(
            &before, &after);
        for (slot = 0u; !failed && slot != sizeof(image) / sizeof(image[0]); ++slot)
        {
            lib_u32 value;

            failed |= !pusha_popa_read_image(&state, 0x4018u + slot * 2u, 2u,
                &value) || value != image[slot];
        }
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 pusha_popa_test_irq_no_shadow(void)
{
    static const lib_u8 codes[][2] = {{0x60u, 0x90u}, {0x61u, 0x90u}};
    static const lib_u8 halt = 0xf4u;
    lib_u8 form;

    for (form = 0u; form != 2u; ++form)
    {
        pusha_popa_machine state;
        core_machine_pic_irq_source *source = LIB_NULL;
        core_machine_run_result result;
        core_machine_debug_cpu_snapshot before;
        core_machine_debug_cpu_snapshot after;
        lib_u16 offset = 0x100u;
        lib_u16 segment = 0u;
        lib_u16 frame_ip = 0u;
        lib_u16 image[] = {0x1111u, 0x2222u, 0x3333u, 0x4444u,
            0x5555u, 0x6666u, 0x7777u, 0x8888u};
        lib_i32 failed = !pusha_popa_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed)
        {
            failed |= pusha_popa_seed(&state, 0x12348000u, 0xa1a23344u) != LIB_STATUS_OK;
            if (form != 0u)
                failed |= core_machine_memory_write(state.machine, 0x8000u, image,
                    sizeof(image)) != LIB_STATUS_OK;
            failed |= core_machine_memory_write(state.machine, 0u, codes[form],
                sizeof(codes[form])) != LIB_STATUS_OK || core_machine_memory_write(
                state.machine, 0x80u, &offset, sizeof(offset)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x82u, &segment,
                sizeof(segment)) != LIB_STATUS_OK || core_machine_memory_write(
                state.machine, 0x100u, &halt, sizeof(halt)) != LIB_STATUS_OK;
        }
        if (!failed)
        {
            lib_memory_set(&source, 0, sizeof(source));
            test_pic_program_vector(state.board->shared_pic_master, 0x20u);
            test_pic_bind_source(&source,
                state.board->shared_pic_master, state.board->shared_pic_slave,
                0u);
            core_machine_pic_irq_source_assert(source);
            core_machine_pic_irq_source_deassert(source);
            failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){2u, 0u}, &result) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
            if (!failed)
                failed |= core_machine_memory_inspect(state.machine,
                after.ss.base + (lib_u16)after.esp,
                (void *)CORE_MACHINE_REFERENCE_OF(frame_ip), sizeof(frame_ip)) != LIB_STATUS_OK ||
                after.eip != 0x101u || frame_ip != 1u || !CORE_MACHINE_BIT_IS_SET(
                test_pic_read(state.board->shared_pic_master, 0x0bu), VPIC_ISR_IRQ(0u)) ||
                CORE_MACHINE_BIT_IS_SET(test_pic_read(state.board->shared_pic_master, 0x0au),
                VPIC_IRR_IRQ(0u));
            if (!failed && form == 0u)
            {
                failed |= after.esp != 0x12347feau || !pusha_popa_expect_push_image(
                    &state, &before, 0x7ff0u, 2u);
            }
            else if (!failed)
            {
                failed |= after.eax != 0xa1a28888u || after.ecx !=
                    0xb1b27777u || after.edx != 0xc1c26666u || after.ebx !=
                    0xd1d25555u || after.ebp != 0xe1e23333u || after.esi !=
                    0xf1f22222u || after.edi != 0x11221111u || after.esp !=
                    0x1234800au;
            }
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!pusha_popa_test_protected_pusha_limit())
    {
        lib_c_printf("PUSHA-POPA stage=protected-pusha\n");
        return 1;
    }
    if (!pusha_popa_test_protected_popa_limit())
    {
        lib_c_printf("PUSHA-POPA stage=protected-popa\n");
        return 1;
    }
    if (!pusha_popa_test_irq_no_shadow())
    {
        lib_c_printf("PUSHA-POPA stage=irq\n");
        return 1;
    }
    lib_c_printf("PUSHA-POPA:OK\n");
    lib_c_printf("PUSHA-POPA-PROFILES:OK\n");
    return 0;
}
