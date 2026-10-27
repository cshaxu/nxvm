#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "pic_fixture.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/board-base/machine_board_state.h"
#include "board_board_fixture.h"
#include "x86-nxvm/board-base/pic_bus_interface.h"
typedef struct xchg_machine {
    x86_core_machine *machine;
    x86_nxvm_board_state *board;
} xchg_machine;

static lib_i32 xchg_prepare(x86_core_cpu_profile profile, xchg_machine *state)
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

static lib_status xchg_seed(xchg_machine *state, lib_u32 flags)
{
    const x86_core_cpu_debug_register_patch registers = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EAX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ECX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS),
        .values = {
            [X86_CORE_CPU_DEBUG_EAX] = 0xaabb3344u,
            [X86_CORE_CPU_DEBUG_ECX] = 0x55667788u,
            [X86_CORE_CPU_DEBUG_EFLAGS] = flags
        }
    };
    return x86_core_machine_debug_patch_registers(state->machine, &registers);
}

static lib_i32 xchg_prepare_protected(xchg_machine *state, lib_u16 limit, lib_bool writable)
{
    static const lib_u8 pointer[] = { 0x1fu, 0, 0, 0x03u, 0, 0 };
    lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0,0,0x92u,0,0, 0xffu,0xffu,0,0x40u,0,0x92u,0,0
    };
    static const lib_u8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0,0x01u,0xb8u,1,0,0x0fu,0x01u,0xf0u,0xb8u,
        0x10u,0,0x8eu,0xd8u,0x8eu,0xc0u,0xb8u,0x18u,0,0x8eu,0xd0u,
        0xbcu,0,0x80u,0xeau,0,0,0x08u,0
    };
    x86_core_machine_run_result result;

    gdt[16] = (lib_u8)limit;
    gdt[17] = (lib_u8)(limit >> 8);
    gdt[21] = writable ? 0x92u : 0x90u;
    return xchg_prepare(X86_CORE_CPU_PROFILE_80386, state) &&
        x86_core_machine_memory_write(state->machine,0x0100u,pointer,sizeof(pointer))==LIB_STATUS_OK &&
        x86_core_machine_memory_write(state->machine,0x0300u,gdt,sizeof(gdt))==LIB_STATUS_OK &&
        x86_core_machine_memory_write(state->machine,0u,bootstrap,sizeof(bootstrap))==LIB_STATUS_OK &&
        x86_core_machine_run(state->machine,(x86_core_machine_run_budget){10u,0u},&result)==LIB_STATUS_OK &&
        result.reason==X86_CORE_MACHINE_STOP_BUDGET && result.executed==10u;
}

static lib_i32 xchg_test_write_fault_atomicity(void)
{
    static const lib_u8 code[] = { 0x87u,0x06u,0x00u,0x10u };
    xchg_machine state;
    x86_core_machine_run_result result;
    x86_core_cpu_debug_cpu_snapshot before;
    x86_core_cpu_debug_cpu_snapshot after;
    lib_u16 memory_before = 0x7788u;
    lib_u16 memory_after = 0u;
    lib_i32 failed = !xchg_prepare_protected(&state, 0xffffu, LIB_FALSE);

    if (!failed)
    {
        failed |= xchg_seed(&state, 0x41u) != LIB_STATUS_OK;
        failed |= x86_core_machine_memory_write(state.machine,0x1000u,&memory_before,
            sizeof(memory_before)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(state.machine,0x2000u,code,sizeof(code)) != LIB_STATUS_OK;
        failed |= x86_core_machine_debug_capture_cpu_snapshot(state.machine,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
        failed |= !test_x86_nxvm_machine_fixture_shutdown_wait(x86_core_machine_run(
            state.machine, (x86_core_machine_run_budget){1u,0u}, &result), &result);
        failed |= x86_core_machine_debug_capture_cpu_snapshot(state.machine,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
        failed |= x86_core_machine_memory_inspect(state.machine,0x1000u,
            (void *)&memory_after,sizeof(memory_after)) != LIB_STATUS_OK;
        if (!failed) failed |= after.eax != before.eax || after.eflags != before.eflags ||
            after.eip != before.eip || memory_after != memory_before;
    }
    x86_core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 xchg_test_read_fault_atomicity(void)
{
    static const lib_u8 codes[][5] = { {0x86u,0x06u,0x02u,0x10u,0u},
        {0x66u,0x87u,0x06u,0x02u,0x10u} };
    lib_u8 form;
    for (form = 0u; form != 2u; ++form)
    {
        xchg_machine state;
        x86_core_machine_run_result result;
        x86_core_cpu_debug_cpu_snapshot before;
        x86_core_cpu_debug_cpu_snapshot after;
        lib_u32 memory_before = 0x11223344u;
        lib_u32 memory_after = 0u;
        lib_i32 failed = !xchg_prepare_protected(&state, 0x1001u, LIB_TRUE);
        if (!failed)
        {
            failed |= xchg_seed(&state, 0x41u) != LIB_STATUS_OK;
            failed |= x86_core_machine_memory_write(state.machine,0x1002u,&memory_before,4u)!=LIB_STATUS_OK ||
                x86_core_machine_memory_write(state.machine,0x2000u,codes[form],form?5u:4u)!=LIB_STATUS_OK;
            failed |= x86_core_machine_debug_capture_cpu_snapshot(state.machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
            failed |= !test_x86_nxvm_machine_fixture_shutdown_wait(x86_core_machine_run(
                state.machine, (x86_core_machine_run_budget){1u,0u}, &result), &result);
            failed |= x86_core_machine_debug_capture_cpu_snapshot(state.machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
            if (!failed) failed |= x86_core_machine_memory_inspect(state.machine,0x1002u,
                (void *)&memory_after,4u)!=LIB_STATUS_OK || after.eax!=before.eax ||
                after.eip!=before.eip || after.eflags!=before.eflags || memory_after!=memory_before;
        }
        x86_core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 xchg_test_irq_no_shadow(void)
{
    static const lib_u8 code[] = { 0x87u, 0x06u, 0x00u, 0x10u, 0x90u };
    static const lib_u8 hlt = 0xf4u;
    xchg_machine state;
    x86_nxvm_board_pic_irq_source *source = LIB_NULL;
    x86_core_machine_run_result result;
    x86_core_cpu_debug_cpu_snapshot after;
    lib_u16 offset = 0x0100u;
    lib_u16 segment = 0u;
    lib_u16 frame = 0u;
    lib_u16 memory = 0x7788u;
    lib_i32 failed = !xchg_prepare(X86_CORE_CPU_PROFILE_80386,&state);
    if (!failed)
    {
        failed |= x86_core_machine_memory_write(state.machine,0x1000u,&memory,2u)!=LIB_STATUS_OK ||
            x86_core_machine_memory_write(state.machine,0u,code,sizeof(code))!=LIB_STATUS_OK ||
            x86_core_machine_memory_write(state.machine,0x80u,&offset,2u)!=LIB_STATUS_OK ||
            x86_core_machine_memory_write(state.machine,0x82u,&segment,2u)!=LIB_STATUS_OK ||
            x86_core_machine_memory_write(state.machine,0x100u,&hlt,1u)!=LIB_STATUS_OK;
        failed |= xchg_seed(&state, 0x200u) != LIB_STATUS_OK;
        lib_memory_set(&source,0,sizeof(source));
        test_pic_program_vector(state.board->shared_pic_master, 0x20u);
        test_pic_bind_source(&source,state.board->shared_pic_master,state.board->shared_pic_slave,0u);
        x86_nxvm_board_pic_irq_source_assert(source);
        failed |= x86_core_machine_run(state.machine,(x86_core_machine_run_budget){2u,0u},&result)!=LIB_STATUS_OK ||
            result.reason!=X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        x86_nxvm_board_pic_irq_source_deassert(source);
        failed |= x86_core_machine_debug_capture_cpu_snapshot(state.machine,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
        if (!failed) failed |= x86_core_machine_memory_inspect(state.machine, after.ss.base + (lib_u16)after.esp, &frame, 2u)!=LIB_STATUS_OK || after.eip!=0x101u || frame!=4u ||
            !(((test_pic_read(state.board->shared_pic_master, 0x0bu)) & (VPIC_ISR_IRQ(0u))) != 0u);
    }
    x86_core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 xchg_test_accumulator_irq(void)
{
    static const lib_u8 code[] = { 0x91u, 0x90u };
    static const lib_u8 hlt = 0xf4u;
    xchg_machine state;
    x86_nxvm_board_pic_irq_source *source = LIB_NULL;
    x86_core_machine_run_result result;
    x86_core_cpu_debug_cpu_snapshot after;
    lib_u16 offset = 0x0100u;
    lib_u16 segment = 0u;
    lib_u16 frame = 0u;
    lib_i32 failed;

    lib_memory_set(&state, 0, sizeof(state));
    lib_memory_set(&source, 0, sizeof(source));
    lib_memory_set(&result, 0, sizeof(result));
    lib_memory_set(&after, 0, sizeof(after));
    failed = !xchg_prepare(X86_CORE_CPU_PROFILE_80386, &state);

    if (!failed)
    {
        failed |= x86_core_machine_memory_write(state.machine, 0u, code,
                sizeof(code)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(state.machine, 0x80u, &offset,
                2u) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(state.machine, 0x82u, &segment,
                2u) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(state.machine, 0x100u, &hlt,
                1u) != LIB_STATUS_OK;
        failed |= xchg_seed(&state, 0x200u) != LIB_STATUS_OK;
        test_pic_program_vector(state.board->shared_pic_master, 0x20u);
        test_pic_bind_source(&source,
            state.board->shared_pic_master,
            state.board->shared_pic_slave, 0u);
        x86_nxvm_board_pic_irq_source_assert(source);
        failed |= x86_core_machine_run(state.machine,
            (x86_core_machine_run_budget){ 2u, 0u }, &result) != LIB_STATUS_OK ||
            result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        x86_nxvm_board_pic_irq_source_deassert(source);
        failed |= x86_core_machine_debug_capture_cpu_snapshot(state.machine,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
        if (!failed) failed |= x86_core_machine_memory_inspect(state.machine, after.ss.base + (lib_u16)after.esp, &frame, 2u) != LIB_STATUS_OK ||
            after.eip != 0x101u ||
            frame != 1u ||
            after.eax != 0xaabb7788u ||
            after.ecx != 0x55663344u ||
            !(((test_pic_read(state.board->shared_pic_master, 0x0bu)) & (VPIC_ISR_IRQ(0u))) != 0u);
    }
    if (failed)
    {
        lib_c_printf(
            "XCHG acc irq reason=%d eip=%08x frame=%04x eax=%08x "
            "ecx=%08x irr=%02x isr=%02x\n",
            result.reason,
            after.eip,
            frame,
            after.eax,
            after.ecx,
            state.board != LIB_NULL ? test_pic_read(state.board->shared_pic_master, 0x0au) : 0u,
            state.board != LIB_NULL ? test_pic_read(state.board->shared_pic_master, 0x0bu) : 0u);
    }
    x86_core_machine_destroy(state.machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!xchg_test_write_fault_atomicity())
    {
        lib_c_printf("XCHG stage=write-fault\n");
        return 1;
    }
    if (!xchg_test_read_fault_atomicity())
    {
        lib_c_printf("XCHG stage=read-fault\n");
        return 1;
    }
    if (!xchg_test_irq_no_shadow())
    {
        lib_c_printf("XCHG stage=irq\n");
        return 1;
    }
    if (!xchg_test_accumulator_irq())
    {
        lib_c_printf("XCHG acc stage=irq\n");
        return 1;
    }
    lib_c_printf("XCHG:OK\n");
    lib_c_printf("XCHG-ACC:OK\n");
    lib_c_printf("ACCUMULATOR-XCHG-PROFILES:OK\n");
    lib_c_printf("XCHG-MODRM-PROFILES:OK\n");
    return 0;
}
