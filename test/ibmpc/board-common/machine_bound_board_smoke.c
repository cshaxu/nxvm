#include "core_machine_board_fixture.h"
#include "ibmpc/board-common/machine_board_state.h"
#include "pic_fixture.h"
#include "x86/core/device_support_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include <stdio.h>

static lib_i32 bound_board_create(core_machine **out_machine,
    core_machine_cpu_profile profile,
    core_machine_board_state **out_board)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile, .fpu_profile = X86_FPU_PROFILE_NONE
    };
    core_machine *machine = LIB_NULL;

    *out_machine = LIB_NULL;
    if (core_machine_create(&config, &machine, out_board) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK) {
        core_machine_destroy(machine);
        if (out_board != LIB_NULL) *out_board = LIB_NULL;
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static lib_i32 bound_board_entry(core_machine *machine, lib_u32 eax)
{
    const core_machine_debug_register_patch patch = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX),
        .values = {
            [CORE_MACHINE_DEBUG_ESP] = 0x8000u,
            [CORE_MACHINE_DEBUG_EAX] = eax
        }
    };

    return core_machine_debug_patch_registers(machine, &patch) == LIB_STATUS_OK;
}

static lib_i32 bound_board_real_br(core_machine_cpu_profile profile,
    lib_u16 ax)
{
    static const lib_u8 code[] = {0x62u,0x06u,0x00u,0x04u};
    const lib_i16 pair[] = {-2,2};
    const lib_u16 vector[] = {0x0100u,0u};
    const lib_u8 handler = 0xf4u;
    core_machine *machine = LIB_NULL;
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_debug_cpu_snapshot before = {0}, after = {0};
    lib_u16 frame[3] = {0};
    lib_i32 failed = !bound_board_create(&machine, profile, LIB_NULL);

    if (!failed)
        failed = !bound_board_entry(machine, 0xa1a10000u | ax) ||
            core_machine_memory_write(machine, 0u, code,
                sizeof(code)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x0400u, pair,
                sizeof(pair)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 5u * 4u, vector,
                sizeof(vector)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x0100u, &handler,
                sizeof(handler)) != LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK ||
            core_machine_run(machine, (core_machine_run_budget){1u,0u},
                &result) != LIB_STATUS_OK ||
            core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
                LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            core_machine_debug_read_memory(machine,
                after.ss.base + (lib_u16)after.esp, frame,
                sizeof(frame)) != LIB_STATUS_OK;
    if (!failed)
        failed = diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid ||
            !(diagnostic.last_delivered_exception.exception_mask &
                VCPUINS_EXCEPT_BR) ||
            after.eip != 0x0100u || after.eax != before.eax ||
            frame[0] != 0u || frame[1] != 0u ||
            frame[2] != (lib_u16)before.eflags;
    if (!failed)
        failed = core_machine_run(machine,
            (core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            after.eip != 0x0101u;
    core_machine_destroy(machine);
    return !failed;
}

static lib_i32 bound_board_boot_protected(core_machine **out_machine,
    const lib_u8 *code, lib_size bytes, lib_u16 ds_limit,
    lib_u16 ss_limit)
{
    static const lib_u8 gdtr[] = {0x1fu,0,0,0x03u,0,0};
    static const lib_u8 idtr[] = {0xffu,0,0,0x04u,0,0};
    lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x40u,0,0x92u,0,0
    };
    static const lib_u8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0x0fu,0x01u,0x1eu,0x10u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,
        0xb8u,0x18u,0x00u,0x8eu,0xd0u,
        0xeau,0x00u,0x00u,0x08u,0x00u
    };
    lib_u8 idt[0x100u] = {0};
    core_machine *machine = LIB_NULL;
    core_machine_run_result result = {0};
    const core_machine_debug_register_patch bootstrap_sp = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP),
        .values = {[CORE_MACHINE_DEBUG_ESP] = 0x0100u}
    };
    const core_machine_debug_register_patch fault_sp = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP),
        .values = {[CORE_MACHINE_DEBUG_ESP] = 0x8000u}
    };

    *out_machine = LIB_NULL;
    gdt[16u] = (lib_u8)ds_limit;
    gdt[17u] = (lib_u8)(ds_limit >> 8u);
    gdt[24u] = (lib_u8)ss_limit;
    gdt[25u] = (lib_u8)(ss_limit >> 8u);
    idt[5u * 8u + 1u] = 0x01u;
    idt[5u * 8u + 2u] = 0x08u;
    idt[5u * 8u + 5u] = 0x86u;
    idt[13u * 8u + 1u] = 0x01u;
    idt[13u * 8u + 2u] = 0x08u;
    idt[13u * 8u + 5u] = 0x86u;
    if (!bound_board_create(&machine, CORE_MACHINE_CPU_PROFILE_80386, LIB_NULL) ||
        !bound_board_entry(machine, 1u) ||
        (ss_limit != 0xffffu && core_machine_debug_patch_registers(machine,
            &bootstrap_sp) != LIB_STATUS_OK) ||
        core_machine_memory_write(machine, 0x0100u, gdtr,
            sizeof(gdtr)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x0110u, idtr,
            sizeof(idtr)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x0300u, gdt,
            sizeof(gdt)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x0400u, idt,
            sizeof(idt)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0u, bootstrap,
            sizeof(bootstrap)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x2000u, code,
            bytes) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x2100u,
            (const lib_u8[]){0xf4u}, 1u) != LIB_STATUS_OK ||
        core_machine_run(machine, (core_machine_run_budget){9u,0u},
            &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET ||
        result.executed != 9u ||
        (ss_limit != 0xffffu && core_machine_debug_patch_registers(machine,
            &fault_sp) != LIB_STATUS_OK)) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static lib_i32 bound_board_protected_fault(lib_u8 fault,
    lib_u16 ds_limit, lib_u16 ss_limit)
{
    static const lib_u8 code[] = {0x62u,0x06u,0x00u,0x04u};
    static const lib_u8 ss_code[] = {0x36u,0x62u,0x06u,0x00u,0x04u};
    const lib_i16 pair[] = {-2,2};
    core_machine *machine = LIB_NULL;
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_debug_cpu_snapshot before = {0}, after = {0};
    const lib_u8 *program = fault == 2u ? ss_code : code;
    const lib_size bytes = fault == 2u ? sizeof(ss_code) : sizeof(code);
    const lib_u32 pair_address = fault == 2u ? 0x4400u : 0x3400u;
    lib_u16 frame[4] = {0};
    lib_u16 observed[2] = {0};
    lib_status run_status = LIB_STATUS_OK;
    lib_i32 failed = !bound_board_boot_protected(&machine, program,
        bytes, ds_limit, ss_limit);
    const core_machine_debug_register_patch patch = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX),
        .values = {[CORE_MACHINE_DEBUG_EAX] =
            0xa1a10000u | (fault == 0u ? 3u : 1u)}
    };

    if (!failed)
        failed = core_machine_debug_patch_registers(machine, &patch) !=
                LIB_STATUS_OK ||
            core_machine_memory_write(machine, pair_address, pair,
                sizeof(pair)) != LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
    if (!failed) {
        run_status = core_machine_run(machine,
            (core_machine_run_budget){64u,0u}, &result);
        failed = run_status != LIB_STATUS_OK &&
            (fault != 2u || run_status != LIB_STATUS_INTERNAL_ERROR);
    }
    if (!failed)
        failed = core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
                LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            (fault != 2u && core_machine_debug_read_memory(machine,
                pair_address, observed, sizeof(observed)) != LIB_STATUS_OK);
    if (!failed && fault != 2u)
        failed = diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid ||
            !(diagnostic.last_delivered_exception.exception_mask &
                (fault == 0u ? VCPUINS_EXCEPT_BR : VCPUINS_EXCEPT_GP)) ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            result.executed != 0u || result.ticks != 0u ||
            after.eip != 0x0100u || after.eax != before.eax ||
            observed[0] != (lib_u16)pair[0] ||
            observed[1] != (lib_u16)pair[1];
    if (!failed && fault != 2u) {
        failed = core_machine_debug_read_memory(machine,
            after.ss.base + (lib_u16)after.esp, frame,
            sizeof(frame)) != LIB_STATUS_OK;
        if (!failed && fault == 0u)
            failed = frame[0] != 0u || frame[1] != 0x0008u ||
                frame[2] != (lib_u16)before.eflags;
        else if (!failed)
            failed = frame[0] != 0u || frame[1] != 0u ||
                frame[2] != 0x0008u ||
                frame[3] != (lib_u16)before.eflags;
    }
    if (!failed && fault == 2u)
        failed = !diagnostic.first_fault.valid ||
            !(diagnostic.first_fault.exception_mask & VCPUINS_EXCEPT_DF) ||
            result.reason != CORE_MACHINE_STOP_FAULT ||
            after.eip != 0u || after.eax != before.eax;
    if (!failed && fault != 2u)
        failed = core_machine_run(machine,
                (core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            after.eip != 0x0101u;
    core_machine_destroy(machine);
    return !failed;
}

static lib_i32 bound_board_irq(void)
{
    static const lib_u8 code[] = {0x62u,0x06u,0x00u,0x04u,0x90u};
    const lib_i16 pair[] = {-2,2};
    const lib_u16 vector[] = {0x0100u,0u};
    const lib_u8 handler = 0xf4u;
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_pic_irq_source *source = LIB_NULL;
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot before = {0}, after = {0};
    lib_u16 frame[3] = {0};
    lib_i32 failed = !bound_board_create(&machine,
        CORE_MACHINE_CPU_PROFILE_80386, &board);
    const core_machine_debug_register_patch flags = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
        .values = {[CORE_MACHINE_DEBUG_EFLAGS] = CORE_MACHINE_DEBUG_EFLAGS_IF}
    };

    if (!failed)
        failed = !bound_board_entry(machine, 0xa1a10001u) ||
            core_machine_debug_patch_registers(machine, &flags) !=
                LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0u, code,
                sizeof(code)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x0400u, pair,
                sizeof(pair)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x20u * 4u, vector,
                sizeof(vector)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x0100u, &handler,
                sizeof(handler)) != LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
    if (!failed) {
        test_pic_program_vector(board->shared_pic_master, 0x20u);
        test_pic_bind_source(&source, board->shared_pic_master,
            board->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(source);
        core_machine_pic_irq_source_deassert(source);
        failed = core_machine_run(machine,
                (core_machine_run_budget){2u,0u}, &result) != LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            core_machine_debug_read_memory(machine,
                after.ss.base + (lib_u16)after.esp, frame,
                sizeof(frame)) != LIB_STATUS_OK;
    }
    if (!failed)
        failed = result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            after.eip != 0x0101u || after.eax != before.eax ||
            after.ecx != before.ecx || after.edx != before.edx ||
            after.ebx != before.ebx || after.ebp != before.ebp ||
            after.esi != before.esi || after.edi != before.edi ||
            frame[0] != 4u || frame[1] != 0u ||
            frame[2] != ((lib_u16)before.eflags | 0x0002u) ||
            !(test_pic_read(board->shared_pic_master, 0x0bu) &
                VPIC_ISR_IRQ(0u)) ||
            (test_pic_read(board->shared_pic_master, 0x0au) &
                VPIC_IRR_IRQ(0u));
    core_machine_destroy(machine);
    return !failed;
}

lib_i32 main(void)
{
    lib_i32 real186 = bound_board_real_br(CORE_MACHINE_CPU_PROFILE_80186, 3u);
    lib_i32 real286 = bound_board_real_br(CORE_MACHINE_CPU_PROFILE_80286, 3u);
    lib_i32 real386 = bound_board_real_br(CORE_MACHINE_CPU_PROFILE_80386, 3u);
    lib_i32 lower = bound_board_real_br(CORE_MACHINE_CPU_PROFILE_80386, 0xfffdu);
    lib_i32 br = bound_board_protected_fault(0u, 0xffffu, 0xffffu);
    lib_i32 gp = bound_board_protected_fault(1u, 0x0401u, 0xffffu);
    lib_i32 ss = bound_board_protected_fault(2u, 0xffffu, 0x0401u);
    lib_i32 irq = bound_board_irq();

    if (!real186 || !real286 || !real386 || !lower || !br || !gp || !ss || !irq) {
        fprintf(stderr, "M5:T539:S41:BOUND board failed real186=%d real286=%d real386=%d lower=%d br=%d gp=%d ss=%d irq=%d\n",
            real186, real286, real386, lower, br, gp, ss, irq);
        return 1;
    }
    puts("M5:T539:S41:BOUND-BOARD:OK");
    return 0;
}
