#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "pic_fixture.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/board-base/machine_board_state.h"
#include "board_board_fixture.h"
#include "x86-nxvm/board-base/pic_bus_interface.h"
typedef struct enter_leave_machine
{
    x86_core_machine *machine;
    x86_nxvm_board_state *board;
} enter_leave_machine;

static lib_i32 enter_leave_prepare(x86_core_cpu_profile profile, enter_leave_machine *state)
{
    const x86_nxvm_board_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = profile, .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ES) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP)
    };
    lib_memory_set(state, 0, sizeof(*state));
    return x86_nxvm_board_create(&config, &state->machine, &state->board) == LIB_STATUS_OK &&
        x86_core_machine_freeze_execution_providers(state->machine) == LIB_STATUS_OK &&
        x86_core_machine_reset(state->machine) == LIB_STATUS_OK &&
        x86_core_machine_debug_patch_registers(state->machine, &entry) == LIB_STATUS_OK;
}

static lib_status enter_leave_seed(enter_leave_machine *state, lib_u32 esp, lib_u32 ebp)
{
    const x86_core_cpu_debug_register_patch registers = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EAX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ECX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EDX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EBX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EBP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESI) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EDI) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS),
        .values = {
            [X86_CORE_CPU_DEBUG_EAX] = 0xa1a23344u, [X86_CORE_CPU_DEBUG_ECX] = 0xb1b25566u,
            [X86_CORE_CPU_DEBUG_EDX] = 0xc1c27788u, [X86_CORE_CPU_DEBUG_EBX] = 0xd1d299aau,
            [X86_CORE_CPU_DEBUG_ESP] = esp, [X86_CORE_CPU_DEBUG_EBP] = ebp,
            [X86_CORE_CPU_DEBUG_ESI] = 0xf1f2ddefu, [X86_CORE_CPU_DEBUG_EDI] = 0x1122a5a5u,
            [X86_CORE_CPU_DEBUG_EFLAGS] = 0x245u
        }
    };
    return x86_core_machine_debug_patch_registers(state->machine, &registers);
}

static lib_i32 enter_leave_boot_protected(enter_leave_machine *state, lib_u16 limit, lib_bool expdown)
{
    static const lib_u8 pointer[] = {0x1fu, 0u, 0u, 0x03u, 0u, 0u};
    lib_u8 gdt[] = {
        0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
        0xffu, 0xffu, 0u, 0x20u, 0u, 0x9au, 0u, 0u,
        0xffu, 0xffu, 0u, 0x30u, 0u, 0x92u, 0u, 0u,
        0xffu, 0xffu, 0u, 0x40u, 0u, 0x92u, 0u, 0u
    };
    static const lib_u8 bootstrap[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u, 0xb8u, 0x01u, 0x00u,
        0x0fu, 0x01u, 0xf0u, 0xb8u, 0x10u, 0x00u, 0x8eu, 0xd8u,
        0x8eu, 0xc0u, 0xb8u, 0x18u, 0x00u, 0x8eu, 0xd0u, 0xbcu,
        0x00u, 0x80u, 0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    x86_core_machine_run_result result;

    lib_i32 ready = x86_core_machine_memory_write(state->machine, 0x0100u, pointer,
        sizeof(pointer)) == LIB_STATUS_OK && x86_core_machine_memory_write(
        state->machine, 0x0300u, gdt, sizeof(gdt)) == LIB_STATUS_OK &&
        x86_core_machine_memory_write(state->machine, 0u, bootstrap,
        sizeof(bootstrap)) == LIB_STATUS_OK &&
        x86_core_machine_run(state->machine, (x86_core_machine_run_budget){10u, 0u},
        &result) == LIB_STATUS_OK && result.reason ==
        X86_CORE_MACHINE_STOP_BUDGET && result.executed == 10u;

    const x86_core_cpu_debug_register_patch segment = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS),
        .values = {
            [X86_CORE_CPU_DEBUG_SS] = 0x18u
        }
    };

    /* Prepare the original invalid-stack precondition at the paused boundary.
     * Executing MOV SS with that ESP would fault before the tested instruction. */
    if (!ready) return 0;
    gdt[24u] = (lib_u8)limit;
    gdt[25u] = (lib_u8)(limit >> 8);
    if (expdown) gdt[29u] = 0x96u;
    return x86_core_machine_memory_write(state->machine, 0x0300u, gdt,
        sizeof(gdt)) == LIB_STATUS_OK &&
        x86_core_machine_debug_patch_registers(state->machine, &segment) == LIB_STATUS_OK;
}

static lib_i32 enter_leave_sregs_same(const x86_core_cpu_debug_cpu_snapshot *before, const x86_core_cpu_debug_cpu_snapshot *after)
{
    return lib_memory_compare(&before->es, &after->es,
        sizeof(before->es)) == 0 && lib_memory_compare(&before->cs,
        &after->cs, sizeof(before->cs)) == 0 && lib_memory_compare(
        &before->ss, &after->ss, sizeof(before->ss)) == 0 &&
        lib_memory_compare(&before->ds, &after->ds,
        sizeof(before->ds)) == 0 && lib_memory_compare(&before->fs,
        &after->fs, sizeof(before->fs)) == 0 && lib_memory_compare(
        &before->gs, &after->gs, sizeof(before->gs)) == 0;
}

static lib_i32 enter_leave_cpu_same(const x86_core_cpu_debug_cpu_snapshot *before, const x86_core_cpu_debug_cpu_snapshot *after)
{
    return before->eax == after->eax &&
        before->ecx == after->ecx &&
        before->edx == after->edx &&
        before->ebx == after->ebx &&
        before->esp == after->esp &&
        before->ebp == after->ebp &&
        before->esi == after->esi &&
        before->edi == after->edi &&
        before->eip == after->eip &&
        before->eflags == after->eflags &&
        enter_leave_sregs_same(before, after);
}

static lib_i32 enter_leave_read(enter_leave_machine *state, lib_u32 address,
    lib_u8 width, lib_u32 *value)
{
    *value = 0u;
    return x86_core_machine_memory_inspect(state->machine,
        address, (void *)lib_pointer_to_uptr(&(*value)), width) == LIB_STATUS_OK;
}

static lib_i32 enter_leave_expect_image(enter_leave_machine *state,
    lib_u32 address, lib_u8 width, lib_u32 expected)
{
    lib_u32 observed;

    return enter_leave_read(state, address, width, &observed) && observed ==
        (width == 2u ? (expected & 0xffffu) : expected);
}

static lib_i32 enter_leave_test_protected_faults(void)
{
    static const lib_u8 enter[] = {0xc8u, 0x00u, 0x00u, 0x03u};
    static const lib_u8 leave[] = {0xc9u};
    enter_leave_machine state;
    x86_core_cpu_debug_cpu_snapshot before;
    x86_core_cpu_debug_cpu_snapshot after;
    x86_core_machine_run_result result;
    lib_status status;
    lib_u16 stack_image[] = {0xaaaau, 0xbbbbu, 0xccccu, 0xddddu,
        0xeeeeu};
    lib_u16 observed_frame[5];
    lib_u32 value;
    lib_i32 failed = !enter_leave_prepare(X86_CORE_CPU_PROFILE_80386,
        &state);

    if (!failed)
        failed |= !enter_leave_boot_protected(&state, 0x18u, LIB_TRUE);
    if (!failed)
    {
        failed |= enter_leave_seed(&state, 0x12340020u, 0xe1e29000u) != LIB_STATUS_OK;
        stack_image[0] = 0x1111u;
        stack_image[1] = 0x2222u;
        failed |= x86_core_machine_memory_write(state.machine, 0xcffeu,
            &stack_image[0], sizeof(lib_u16)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(state.machine, 0xcffcu, &stack_image[1],
            sizeof(lib_u16)) != LIB_STATUS_OK || x86_core_machine_memory_write(
            state.machine, 0x4018u, stack_image, sizeof(stack_image)) !=
            LIB_STATUS_OK || x86_core_machine_memory_write(state.machine, 0x2000u,
            enter, sizeof(enter)) != LIB_STATUS_OK;
        failed |= x86_core_machine_debug_capture_cpu_snapshot(state.machine,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
        status = x86_core_machine_run(state.machine,
            (x86_core_machine_run_budget){1u, 0u}, &result);
        failed |= !test_x86_nxvm_machine_fixture_shutdown_wait(status, &result);
        failed |= x86_core_machine_debug_capture_cpu_snapshot(state.machine,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
        if (!failed)
            failed |= !enter_leave_cpu_same(&before, &after) ||
            x86_core_machine_memory_inspect(state.machine, 0x4018u, observed_frame,
                sizeof(observed_frame)) != LIB_STATUS_OK ||
            lib_memory_compare(observed_frame, stack_image, sizeof(stack_image)) != 0;
    }
    x86_core_machine_destroy(state.machine);

    if (failed)
        return 0;
    failed = !enter_leave_prepare(X86_CORE_CPU_PROFILE_80386, &state);
    if (!failed)
        failed |= !enter_leave_boot_protected(&state, 0x1fu, LIB_FALSE);
    if (!failed)
    {
        failed |= enter_leave_seed(&state, 0x12348000u, 0xe1e20020u) != LIB_STATUS_OK;
        failed |= x86_core_machine_memory_write(state.machine, 0x4020u, stack_image,
            sizeof(lib_u16)) != LIB_STATUS_OK || x86_core_machine_memory_write(
            state.machine, 0x2000u, leave, sizeof(leave)) != LIB_STATUS_OK;
        failed |= x86_core_machine_debug_capture_cpu_snapshot(state.machine,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
        status = x86_core_machine_run(state.machine, (x86_core_machine_run_budget){1u, 0u},
            &result);
        failed |= !test_x86_nxvm_machine_fixture_shutdown_wait(status, &result);
        failed |= x86_core_machine_debug_capture_cpu_snapshot(state.machine,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
        if (!failed)
            failed |= !enter_leave_cpu_same(&before, &after) || !enter_leave_read(&state,
            0x4020u, 2u, &value) || value != stack_image[0];
    }
    x86_core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 enter_leave_test_irq_no_shadow(void)
{
    static const lib_u8 codes[][5] = {
        {0xc8u, 0x04u, 0x00u, 0x00u, 0x90u},
        {0xc9u, 0x90u, 0u, 0u, 0u}
    };
    static const lib_u8 bytes[] = {5u, 2u};
    static const lib_u8 halt = 0xf4u;
    lib_u8 form;

    for (form = 0u; form != 2u; ++form)
    {
        enter_leave_machine state;
        x86_nxvm_board_pic_irq_source *source = LIB_NULL;
        x86_core_machine_run_result result;
        x86_core_cpu_debug_cpu_snapshot before;
        x86_core_cpu_debug_cpu_snapshot after;
        lib_u16 offset = 0x100u;
        lib_u16 segment = 0u;
        lib_u16 frame_ip = 0u;
        lib_u16 old_bp = 0x4567u;
        lib_i32 failed = !enter_leave_prepare(X86_CORE_CPU_PROFILE_80386,
            &state);

        if (!failed)
        {
            failed |= enter_leave_seed(&state, 0x12348000u,
                form != 0u ? 0xe1e28020u : 0xe1e29000u) != LIB_STATUS_OK;
            if (form != 0u)
            {
                failed |= x86_core_machine_memory_write(state.machine, 0x8020u,
                    &old_bp, sizeof(old_bp)) != LIB_STATUS_OK;
            }
            failed |= x86_core_machine_memory_write(state.machine, 0u, codes[form],
                bytes[form]) != LIB_STATUS_OK || x86_core_machine_memory_write(
                state.machine, 0x80u, &offset, sizeof(offset)) != LIB_STATUS_OK ||
                x86_core_machine_memory_write(state.machine, 0x82u, &segment,
                sizeof(segment)) != LIB_STATUS_OK || x86_core_machine_memory_write(
                state.machine, 0x100u, &halt, sizeof(halt)) != LIB_STATUS_OK;
        }
        if (!failed)
        {
            lib_memory_set(&source, 0, sizeof(source));
            test_pic_program_vector(state.board->shared_pic_master, 0x20u);
            test_pic_bind_source(&source,
                state.board->shared_pic_master, state.board->shared_pic_slave,
                0u);
            x86_nxvm_board_pic_irq_source_assert(source);
            failed |= x86_core_machine_debug_capture_cpu_snapshot(state.machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
            failed |= x86_core_machine_run(state.machine,
                (x86_core_machine_run_budget){2u, 0u}, &result) != LIB_STATUS_OK ||
                result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            x86_nxvm_board_pic_irq_source_deassert(source);
            failed |= x86_core_machine_debug_capture_cpu_snapshot(state.machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
            if (!failed)
                failed |= x86_core_machine_memory_inspect(state.machine,
                after.ss.base + (lib_u16)after.esp,
                (void *)lib_pointer_to_uptr(&(frame_ip)), sizeof(frame_ip)) != LIB_STATUS_OK ||
                after.eip != 0x101u || frame_ip != (form == 0u ? 4u : 1u) ||
                !(((test_pic_read(state.board->shared_pic_master, 0x0bu)) & (VPIC_ISR_IRQ(0u))) != 0u) || (((test_pic_read(state.board->shared_pic_master, 0x0au)) & (VPIC_IRR_IRQ(0u))) != 0u);
            if (!failed && form == 0u)
            {
                failed |= (lib_u16)after.ebp != 0x7ffeu || !enter_leave_expect_image(
                    &state, 0x7ffeu, 2u, (lib_u16)before.ebp);
            }
            else if (!failed)
            {
                failed |= (lib_u16)after.ebp != old_bp || after.esp !=
                    0x1234801cu;
            }
        }
        x86_core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!enter_leave_test_protected_faults())
    {
        lib_c_printf("ENTER-LEAVE stage=protected-faults\n");
        return 1;
    }
    if (!enter_leave_test_irq_no_shadow())
    {
        lib_c_printf("ENTER-LEAVE stage=irq\n");
        return 1;
    }
    lib_c_printf("ENTER-LEAVE:OK\n");
    lib_c_printf("ENTER-LEAVE-PROFILES:OK\n");
    return 0;
}
