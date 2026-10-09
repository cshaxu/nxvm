#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "pic_fixture.h"
#include "core/board-base/machine_board_interface.h"
#include "core/board-base/machine_board_state.h"
#include "core_machine_board_fixture.h"
#include "core/x86/device_support_interface.h"
#include "core/board-base/pic_bus_interface.h"
typedef struct push_immediate_machine {
    core_machine *machine;
    core_machine_board_state *board;
} push_immediate_machine;

static lib_i32 push_immediate_prepare(core_machine_cpu_profile profile, push_immediate_machine *state)
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

static lib_status push_immediate_seed(push_immediate_machine *state, lib_u32 esp, lib_u32 eax)
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

static lib_i32 push_immediate_boot_protected(push_immediate_machine *state, lib_u8 limit_segment,
    lib_u16 limit, lib_bool expdown)
{
    static const lib_u8 pointer[] = {0x1fu,0u,0u,0x03u,0u,0u};
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

    lib_i32 ready = core_machine_memory_write(state->machine, 0x100u, pointer,
        sizeof(pointer)) == LIB_STATUS_OK && core_machine_memory_write(
        state->machine, 0x300u, gdt, sizeof(gdt)) == LIB_STATUS_OK &&
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

static lib_i32 push_immediate_sregs_same(const core_machine_debug_cpu_snapshot *before, const core_machine_debug_cpu_snapshot *after)
{
    return lib_memory_compare(&before->es, &after->es, sizeof(before->es)) == 0 &&
        lib_memory_compare(&before->cs, &after->cs, sizeof(before->cs)) == 0 &&
        lib_memory_compare(&before->ss, &after->ss, sizeof(before->ss)) == 0 &&
        lib_memory_compare(&before->ds, &after->ds, sizeof(before->ds)) == 0 &&
        lib_memory_compare(&before->fs, &after->fs, sizeof(before->fs)) == 0 &&
        lib_memory_compare(&before->gs, &after->gs, sizeof(before->gs)) == 0;
}

static lib_i32 push_immediate_test_protected(void)
{
    static const lib_u8 codes[][6] = {{0x68u,0x34u,0x12u},
        {0x66u,0x6au,0x80u}};
    static const lib_u8 bytes[] = {3u,3u};
    lib_u8 form;

    for (form = 0u; form != 2u; ++form)
    {
        push_immediate_machine state;
        core_machine_run_result result;
        core_machine_debug_cpu_snapshot before;
        core_machine_debug_cpu_snapshot after;
        lib_u32 sentinel = 0xdeadbeefu;
        lib_u32 observed = 0u;
        lib_i32 failed = !push_immediate_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed)
            failed |= !push_immediate_boot_protected(&state, 0u, 0xffffu, LIB_TRUE);
        if (!failed)
        {
            lib_u32 stack = form == 0u ? 0xbffeu : 0xbffcu;
            lib_u8 width = form == 0u ? 2u : 4u;

            failed |= push_immediate_seed(&state, 0x12348000u, 0xa1a23344u) != LIB_STATUS_OK;
            failed |= core_machine_memory_write(state.machine, stack, &sentinel,
                width) != LIB_STATUS_OK || core_machine_memory_write(
                state.machine, 0x2000u, codes[form], bytes[form]) != LIB_STATUS_OK;
            failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
            failed |= !test_core_machine_fixture_shutdown_wait(core_machine_run(
                state.machine, (core_machine_run_budget){1u,0u}, &result),
                &result);
            failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
            if (!failed)
                failed |= after.eip != 0u || after.eax != before.eax ||
                after.ecx != before.ecx || after.edx != before.edx ||
                after.ebx != before.ebx || after.esp != before.esp ||
                after.ebp != before.ebp || after.esi != before.esi ||
                after.edi != before.edi || after.eflags !=
            before.eflags || !push_immediate_sregs_same(&before, &after) ||
            core_machine_memory_inspect(state.machine,
            stack, (void *)CORE_MACHINE_REFERENCE_OF(observed), width) != LIB_STATUS_OK ||
            observed != (width == 2u ? (sentinel & 0xffffu) : sentinel);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 push_immediate_test_irq(void)
{
    static const lib_u8 codes[][5] = {{0x68u,0x34u,0x12u,0x90u},
        {0x6au,0x80u,0x90u}};
    static const lib_u8 length[] = {3u,2u};
    static const lib_u8 halt = 0xf4u;
    lib_u8 form;

    for (form = 0u; form != 2u; ++form)
    {
        push_immediate_machine state;
        core_machine_pic_irq_source *source = LIB_NULL;
        core_machine_run_result result;
        core_machine_debug_cpu_snapshot after;
        lib_u16 offset = 0x100u;
        lib_u16 segment = 0u;
        lib_u16 frame_ip = 0u;
        lib_u16 value = 0u;
        lib_i32 failed = !push_immediate_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed)
        {
            failed |= push_immediate_seed(&state, 0x12348000u, 0xa1a23344u) != LIB_STATUS_OK;
            failed |= core_machine_memory_write(state.machine, 0u, codes[form],
                length[form] + 1u) != LIB_STATUS_OK || core_machine_memory_write(
                state.machine, 0x80u, &offset, 2u) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x82u, &segment, 2u) !=
                LIB_STATUS_OK || core_machine_memory_write(state.machine, 0x100u,
                &halt, 1u) != LIB_STATUS_OK;
        }
        if (!failed)
        {
            lib_memory_set(&source, 0, sizeof(source));
            test_pic_program_vector(state.board->shared_pic_master, 0x20u);
            test_pic_bind_source(&source, state.board->shared_pic_master,
                state.board->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(source);
            core_machine_pic_irq_source_deassert(source);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){2u,0u}, &result) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
            if (!failed)
                failed |= core_machine_memory_inspect(state.machine,
                after.ss.base + (lib_u16)after.esp,
                (void *)CORE_MACHINE_REFERENCE_OF(frame_ip), 2u) != LIB_STATUS_OK ||
                after.eip != 0x101u || frame_ip != length[form] ||
                !CORE_MACHINE_BIT_IS_SET(test_pic_read(state.board->shared_pic_master, 0x0bu),
                VPIC_ISR_IRQ(0u)) || CORE_MACHINE_BIT_IS_SET(test_pic_read(state.board->shared_pic_master, 0x0au),
                VPIC_IRR_IRQ(0u)) || after.esp != 0x12347ff8u ||
                core_machine_memory_inspect(state.machine,
                0x7ffeu, (void *)CORE_MACHINE_REFERENCE_OF(value), 2u) != LIB_STATUS_OK ||
                value != (form == 0u ? 0x1234u : 0xff80u);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!push_immediate_test_protected())
    {
        lib_c_printf("PUSH-IMMEDIATE stage=protected\n");
        return 1;
    }
    if (!push_immediate_test_irq())
    {
        lib_c_printf("PUSH-IMMEDIATE stage=irq\n");
        return 1;
    }
    lib_c_printf("PUSH-IMMEDIATE:OK\n");
    return 0;
}
