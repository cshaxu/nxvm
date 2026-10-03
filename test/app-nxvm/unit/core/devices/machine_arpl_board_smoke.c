#include "support/core_machine_board_fixture.h"
#include "app-nxvm/devices/machine_board_state.h"
#include "support/pic_fixture.h"
#include "x86/core/device_support_interface.h"
#include "app-nxvm/devices/machine_board_interface.h"
#include <stdio.h>

static lib_i32 arpl_board_prepare(core_machine **out_machine,
    core_machine_cpu_profile profile, const lib_u8 *code, lib_size bytes,
    lib_u16 ds_limit,
    core_machine_board_state **out_board)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile, .fpu_profile = X86_FPU_PROFILE_NONE
    };
    static const lib_u8 gdt_pointer[] = {0x37u,0,0,0x03u,0,0};
    static const lib_u8 idt_pointer[] = {0x07u,0x01u,0,0x04u,0,0};
    lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x40u,0,0x92u,0,0,
        0xffu,0xffu,0,0x50u,0,0x92u,0,0,
        0xffu,0xffu,0,0x60u,0,0x92u,0,0,
        0xffu,0xffu,0,0x70u,0,0x92u,0,0
    };
    static const lib_u8 bootstrap286[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0x0fu,0x01u,0x1eu,0x10u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd0u,
        0xb8u,0x18u,0x00u,0x8eu,0xd8u,
        0xb8u,0x20u,0x00u,0x8eu,0xc0u,
        0xeau,0x00u,0x00u,0x08u,0x00u
    };
    static const lib_u8 bootstrap386[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0x0fu,0x01u,0x1eu,0x10u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd0u,
        0xb8u,0x18u,0x00u,0x8eu,0xd8u,
        0xb8u,0x20u,0x00u,0x8eu,0xc0u,
        0xb8u,0x28u,0x00u,0x8eu,0xe0u,
        0xb8u,0x30u,0x00u,0x8eu,0xe8u,
        0xeau,0x00u,0x00u,0x08u,0x00u
    };
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP)
    };
    const lib_bool is386 = profile == CORE_MACHINE_CPU_PROFILE_80386;
    const lib_u8 *bootstrap = is386 ? bootstrap386 : bootstrap286;
    const lib_size bootstrap_bytes = is386 ? sizeof(bootstrap386) :
        sizeof(bootstrap286);
    lib_u8 idt[0x108u] = {0};
    core_machine *machine = LIB_NULL;
    core_machine_run_result result = {0};

    *out_machine = LIB_NULL;
    gdt[24u] = (lib_u8)ds_limit;
    gdt[25u] = (lib_u8)(ds_limit >> 8u);
    idt[13u * 8u] = 0x00u;
    idt[13u * 8u + 1u] = 0x01u;
    idt[13u * 8u + 2u] = 0x08u;
    idt[13u * 8u + 5u] = 0x86u;
    if (core_machine_create(&config, &machine, out_board) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x0100u, gdt_pointer,
            sizeof(gdt_pointer)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x0110u, idt_pointer,
            sizeof(idt_pointer)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x0300u, gdt,
            sizeof(gdt)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x0400u, idt,
            sizeof(idt)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0u, bootstrap,
            bootstrap_bytes) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x2000u, code, bytes) !=
            LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x2100u,
            (const lib_u8[]){0xf4u}, 1u) != LIB_STATUS_OK ||
        core_machine_run(machine, (core_machine_run_budget){
            is386 ? 15u : 11u, 0u}, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET ||
        result.executed != (is386 ? 15u : 11u)) {
        core_machine_destroy(machine);
        if (out_board != LIB_NULL) *out_board = LIB_NULL;
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static lib_i32 arpl_board_memory_prefix(void)
{
    static const lib_u8 code[] = {
        0xb9u,0x03u,0x00u,0x26u,0x63u,0x0eu,0x00u,0x04u,0xf4u
    };
    core_machine *machine = LIB_NULL;
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot after = {0};
    lib_u16 value = 1u;
    lib_i32 failed = !arpl_board_prepare(&machine,
        CORE_MACHINE_CPU_PROFILE_80286, code, sizeof(code), 0xffffu, LIB_NULL);

    if (!failed)
        failed = core_machine_memory_write(machine, 0x5400u, &value,
            sizeof(value)) != LIB_STATUS_OK ||
            core_machine_run(machine, (core_machine_run_budget){64u,0u},
                &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            core_machine_debug_read_memory(machine, 0x5400u, &value,
                sizeof(value)) != LIB_STATUS_OK;
    if (!failed)
        failed = value != 3u || !(after.eflags & VCPU_EFLAGS_ZF);
    core_machine_destroy(machine);
    return !failed;
}

static lib_i32 arpl_board_limit(void)
{
    static const lib_u8 code[] = {0xb9u,0x03u,0x00u,
        0x63u,0x0eu,0x00u,0x04u};
    const lib_u16 image = 0x5a01u;
    const lib_u16 adjacent = 0x7e7eu;
    core_machine *machine = LIB_NULL;
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_debug_cpu_snapshot before = {0}, after = {0};
    lib_u16 observed = 0u, observed_adjacent = 0u;
    lib_u16 frame[4] = {0};
    lib_i32 failed = !arpl_board_prepare(&machine,
        CORE_MACHINE_CPU_PROFILE_80386, code, sizeof(code), 0x000fu, LIB_NULL);

    if (!failed)
        failed = core_machine_memory_write(machine, 0x4400u, &image,
            sizeof(image)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x4410u, &adjacent,
                sizeof(adjacent)) != LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK ||
            test_core_machine_fixture_run_after_delivery(machine,
                (core_machine_run_budget){64u,0u},
                &result) != LIB_STATUS_OK ||
            core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
                LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            core_machine_debug_read_memory(machine, 0x4400u, &observed,
                sizeof(observed)) != LIB_STATUS_OK ||
            core_machine_debug_read_memory(machine, 0x4410u,
                &observed_adjacent, sizeof(observed_adjacent)) != LIB_STATUS_OK ||
            core_machine_debug_read_memory(machine,
                after.ss.base + (lib_u16)after.esp, frame,
                sizeof(frame)) != LIB_STATUS_OK;
    if (!failed)
        failed = diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid ||
            !(diagnostic.last_delivered_exception.exception_mask &
                VCPUINS_EXCEPT_GP) ||
            after.eip != 0x101u || observed != image ||
            observed_adjacent != adjacent || after.ecx != 3u ||
            frame[0] != 0u || frame[1] != 3u ||
            frame[2] != 0x0008u || frame[3] != (lib_u16)before.eflags ||
            lib_memory_compare(&before.es, &after.es,
                sizeof(before.es)) != 0 ||
            lib_memory_compare(&before.cs, &after.cs,
                sizeof(before.cs)) != 0 ||
            lib_memory_compare(&before.ss, &after.ss,
                sizeof(before.ss)) != 0 ||
            lib_memory_compare(&before.ds, &after.ds,
                sizeof(before.ds)) != 0 ||
            lib_memory_compare(&before.fs, &after.fs,
                sizeof(before.fs)) != 0 ||
            lib_memory_compare(&before.gs, &after.gs,
                sizeof(before.gs)) != 0;
    core_machine_destroy(machine);
    return !failed;
}

static lib_i32 arpl_board_irq(void)
{
    static const lib_u8 code[] = {0xfbu,0x63u,0xc8u,0x90u};
    lib_u8 change;

    for (change = 0u; change != 2u; ++change) {
        core_machine *machine = LIB_NULL;
        core_machine_board_state *board = LIB_NULL;
        core_machine_pic_irq_source source = {0};
        core_machine_run_result result = {0};
        core_machine_debug_cpu_snapshot before = {0}, after = {0};
        core_machine_cpu_diagnostic diagnostic = {0};
        core_machine_debug_register_patch patch = {0};
        lib_u8 gate[8] = {0};
        lib_u32 frame[3] = {0};
        lib_i32 failed = !arpl_board_prepare(&machine,
            CORE_MACHINE_CPU_PROFILE_80386, code, sizeof(code), 0xffffu, &board);

        gate[1u] = 0x01u;
        gate[2u] = 0x08u;
        gate[5u] = 0x8eu;
        patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS);
        patch.values[CORE_MACHINE_DEBUG_EAX] = 0xa1a10000u |
            (change ? 1u : 3u);
        patch.values[CORE_MACHINE_DEBUG_ECX] = 0xb2b20000u |
            (change ? 3u : 1u);
        patch.values[CORE_MACHINE_DEBUG_EFLAGS] = VCPU_EFLAGS_IF;
        if (!failed)
            failed = core_machine_memory_write(machine, 0x0500u, gate,
                sizeof(gate)) != LIB_STATUS_OK ||
                core_machine_debug_patch_registers(machine, &patch) !=
                    LIB_STATUS_OK ||
                core_machine_debug_capture_cpu_snapshot(machine,
                    CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
        if (!failed) {
            test_pic_program_vector(&board->shared_pic_master, 0x20u);
            core_machine_pic_irq_source_bind(&source,
                &board->shared_pic_master, &board->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
            failed = core_machine_run(machine,
                (core_machine_run_budget){2u,0u}, &result) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET ||
                core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
                    LIB_STATUS_OK ||
                core_machine_debug_capture_cpu_snapshot(machine,
                    CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
                core_machine_debug_read_memory(machine,
                    after.ss.base + (lib_u16)after.esp, frame,
                    sizeof(frame)) != LIB_STATUS_OK;
        }
        if (!failed)
            failed = diagnostic.first_fault.valid || after.eip != 0x100u ||
                (after.eax & 0xffffu) != 3u ||
                (after.eax & 0xffff0000u) !=
                    (before.eax & 0xffff0000u) ||
                after.ecx != before.ecx || after.edx != before.edx ||
                after.ebx != before.ebx || after.ebp != before.ebp ||
                after.esi != before.esi || after.edi != before.edi ||
                !!(after.eflags & VCPU_EFLAGS_ZF) != change ||
                (after.eflags & ~(VCPU_EFLAGS_ZF | VCPU_EFLAGS_IF)) !=
                    (before.eflags & ~(VCPU_EFLAGS_ZF | VCPU_EFLAGS_IF)) ||
                lib_memory_compare(&before.es, &after.es,
                    sizeof(before.es)) != 0 ||
                lib_memory_compare(&before.cs, &after.cs,
                    sizeof(before.cs)) != 0 ||
                lib_memory_compare(&before.ss, &after.ss,
                    sizeof(before.ss)) != 0 ||
                lib_memory_compare(&before.ds, &after.ds,
                    sizeof(before.ds)) != 0 ||
                lib_memory_compare(&before.fs, &after.fs,
                    sizeof(before.fs)) != 0 ||
                lib_memory_compare(&before.gs, &after.gs,
                    sizeof(before.gs)) != 0 ||
                !(test_pic_read(&board->shared_pic_master, 0x0bu) &
                    VPIC_ISR_IRQ(0u)) ||
                (test_pic_read(&board->shared_pic_master, 0x0au) &
                    VPIC_IRR_IRQ(0u)) ||
                frame[0u] != 3u ||
                !!(frame[2u] & VCPU_EFLAGS_ZF) != change ||
                (frame[2u] & ~VCPU_EFLAGS_ZF) !=
                    (before.eflags & ~VCPU_EFLAGS_ZF);
        core_machine_destroy(machine);
        if (failed) return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    lib_i32 memory = arpl_board_memory_prefix();
    lib_i32 limit = arpl_board_limit();
    lib_i32 irq = arpl_board_irq();

    if (!memory || !limit || !irq) {
        fprintf(stderr, "M5:T539:S40:ARPL board failed memory=%d limit=%d irq=%d\n",
            memory, limit, irq);
        return 1;
    }
    printf("M5:T539:S40:ARPL-BOARD:OK\n");
    return 0;
}
