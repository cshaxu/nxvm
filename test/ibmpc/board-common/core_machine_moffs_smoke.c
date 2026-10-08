#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "pic_fixture.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include "ibmpc/board-common/machine_board_state.h"
#include "core_machine_board_fixture.h"
#include "x86/core/device_support_interface.h"
#include "ibmpc/board-common/pic_bus_interface.h"
typedef struct moffs_machine { core_machine *machine;
    core_machine_board_state *board; } moffs_machine;

static lib_i32 moffs_prepare(core_machine_cpu_profile profile, moffs_machine *state)
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

static lib_status moffs_set_registers(moffs_machine *state)
{
    const core_machine_debug_register_patch registers = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESI) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDI) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
        .values = {
            [CORE_MACHINE_DEBUG_EAX] = 0xaabb3344u,
            [CORE_MACHINE_DEBUG_ECX] = 0x11223344u,
            [CORE_MACHINE_DEBUG_EDX] = 0x55667788u,
            [CORE_MACHINE_DEBUG_EBX] = 0x99aabbccu,
            [CORE_MACHINE_DEBUG_ESI] = 0xddeeff00u,
            [CORE_MACHINE_DEBUG_EDI] = 0x10203040u,
            [CORE_MACHINE_DEBUG_EBP] = 0x50607080u,
            [CORE_MACHINE_DEBUG_ESP] = 0x00007777u,
            [CORE_MACHINE_DEBUG_EFLAGS] = 0x41u /* CF | ZF */
        }
    };
    return core_machine_debug_patch_registers(state->machine, &registers);
}

static lib_i32 moffs_test_protected_read_limit(void)
{
    static const lib_u8 gdt_pointer[] = { 0x1fu, 0, 0, 0x03u, 0, 0 };
    static const lib_u8 gdt[] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0xffu, 0xffu, 0, 0x20u, 0, 0x9au, 0, 0,
        0x0fu, 0, 0, 0x30u, 0, 0x92u, 0, 0,
        0xffu, 0xffu, 0, 0x40u, 0, 0x92u, 0, 0
    };
    static const lib_u8 bootstrap[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0xb8u, 0x01u, 0x00u, 0x0fu, 0x01u, 0xf0u,
        0xb8u, 0x10u, 0x00u, 0x8eu, 0xd8u, 0x8eu, 0xc0u,
        0xb8u, 0x18u, 0x00u, 0x8eu, 0xd0u,
        0xbcu, 0x00u, 0x80u, 0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    static const lib_u8 read_code[] = { 0xa0u, 0x10u, 0x00u };
    static const lib_u8 write_code[] = { 0x66u, 0xa3u, 0x10u, 0x00u };
    const lib_u8 *codes[] = { read_code, write_code };
    const lib_u8 bytes[] = { sizeof(read_code), sizeof(write_code) };
    lib_u8 form;

    for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form)
    {
        moffs_machine state;
        core_machine_debug_cpu_snapshot before;
        core_machine_debug_cpu_snapshot after;
        core_machine_run_result result;
        lib_u32 image = 0x11223344u;
        lib_i32 failed = !moffs_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed)
        {
            failed |= core_machine_memory_write(state.machine, 0x0100u, gdt_pointer, sizeof(gdt_pointer)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x0300u, gdt, sizeof(gdt)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0u, bootstrap, sizeof(bootstrap)) != LIB_STATUS_OK ||
                core_machine_run(state.machine, (core_machine_run_budget){10u, 0u},
                    &result) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 10u;
        }
        if (!failed)
        {
            failed |= moffs_set_registers(&state) != LIB_STATUS_OK;
            failed |= core_machine_memory_write(state.machine, 0x3010u, &image, sizeof(image)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x2000u, codes[form], bytes[form]) != LIB_STATUS_OK;
            failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
            failed |= !test_core_machine_fixture_shutdown_wait(core_machine_run(
                state.machine, (core_machine_run_budget){ 1u, 0u }, &result),
                &result);
            failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
            if (!failed) failed |= after.eip != 0u || after.eax != before.eax ||
                after.ecx != before.ecx || after.edx != before.edx ||
                after.ebx != before.ebx || after.esi != before.esi ||
                after.edi != before.edi || after.ebp != before.ebp ||
                after.esp != before.esp ||
                after.eflags != before.eflags ||
                core_machine_memory_inspect(state.machine,
                    0x3010u, (void *)&image, sizeof(image)) !=
                    LIB_STATUS_OK || image != 0x11223344u;
        }
        core_machine_destroy(state.machine);
        if (failed)
        {
            lib_c_printf("MOFFS protected-limit form=%u\n", form);
            return 0;
        }
    }
    return 1;
}

static lib_i32 moffs_test_irq_no_shadow(void)
{
    static const lib_u8 codes[][4] = {
        { 0xa0u, 0x00u, 0x10u, 0x90u },
        { 0xa2u, 0x00u, 0x10u, 0x90u }
    };
    static const lib_u8 hlt = 0xf4u;
    lib_u8 form;

    for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form)
    {
        moffs_machine state;
        core_machine_pic_irq_source *source = LIB_NULL;
        core_machine_run_result result;
        core_machine_debug_cpu_snapshot after;
        lib_u16 vector_offset = 0x0100u;
        lib_u16 vector_segment = 0u;
        lib_u16 frame_ip = 0u;
        lib_u8 image = form == 0u ? 0x5au : 0u;
        lib_i32 failed = !moffs_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed)
        {
            failed |= core_machine_memory_write(state.machine, 0x1000u, &image, sizeof(image)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0u, codes[form], sizeof(codes[form])) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x20u * 4u, &vector_offset, sizeof(vector_offset)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x20u * 4u + 2u, &vector_segment, sizeof(vector_segment)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x0100u, &hlt, sizeof(hlt)) != LIB_STATUS_OK;
        }
        if (!failed)
        {
            failed |= moffs_set_registers(&state) != LIB_STATUS_OK;
            {
                const core_machine_debug_register_patch flags = {
                    .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
                    .values = {[CORE_MACHINE_DEBUG_EFLAGS] = 0x241u}
                };
                failed |= core_machine_debug_patch_registers(state.machine, &flags) != LIB_STATUS_OK;
            }
            lib_memory_set(&source, 0, sizeof(source));
            test_pic_program_vector(state.board->shared_pic_master, 0x20u);
            test_pic_bind_source(&source,
                state.board->shared_pic_master, state.board->shared_pic_slave,
                0u);
            core_machine_pic_irq_source_assert(source);
            core_machine_pic_irq_source_deassert(source);
            failed |= core_machine_run(state.machine,
                    (core_machine_run_budget){ 2u, 0u }, &result) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
            if (!failed) failed |= core_machine_memory_inspect(state.machine, after.ss.base + (lib_u16)after.esp, &frame_ip, sizeof(frame_ip)) !=
                    LIB_STATUS_OK || after.eip != 0x0101u || frame_ip != 3u ||
                !CORE_MACHINE_BIT_IS_SET(test_pic_read(state.board->shared_pic_master, 0x0bu),
                    VPIC_ISR_IRQ(0u)) || CORE_MACHINE_BIT_IS_SET(
                    test_pic_read(state.board->shared_pic_master, 0x0au), VPIC_IRR_IRQ(0u)) ||
                (form == 0u && after.eax != 0xaabb335au) ||
                (form == 1u && (core_machine_memory_inspect(state.machine, 0x1000u, &image, sizeof(image)) != LIB_STATUS_OK ||
                    image != 0x44u));
        }
        core_machine_destroy(state.machine);
        if (failed)
        {
            lib_c_printf("MOFFS irq form=%u\n", form);
            return 0;
        }
    }
    return 1;
}

lib_i32 main(void)
{
    if (!moffs_test_protected_read_limit() ||
        !moffs_test_irq_no_shadow()) return 1;
    lib_c_printf("MOFFS:OK\n");
    lib_c_printf("MOFFS-MOV-PROFILES:OK\n");
    return 0;
}
