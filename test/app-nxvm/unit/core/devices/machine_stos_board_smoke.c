#include "support/cpu_board_limit_fixture.h"
#include "app-nxvm/devices/machine_board_interface.h"
#include "app-nxvm/devices/machine_board_state.h"
#include "support/pic_fixture.h"
#include <stdio.h>

static lib_i32 stos_protected_case(lib_u8 form)
{
    static const lib_u8 codes[][2] = {
        {0xaau, 0u}, {0x66u, 0xabu}, {0xf3u, 0xaau}
    };
    core_machine *machine = LIB_NULL;
    core_machine_debug_register_patch patch = {0};
    core_machine_debug_cpu_snapshot after = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_run_result result = {0};
    lib_u32 image = 0xa5a5a5a5u;
    lib_u8 width = form == 1u ? 4u : 1u;
    lib_u8 bytes = form == 0u ? 1u : 2u;
    lib_i32 failed = !test_cpu_board_limit_prepare_exact(&machine,
        codes[form], bytes, LIB_TRUE, form == 2u ? 0x10u : 0x0fu);

    if (!failed) {
        patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDI);
        patch.values[CORE_MACHINE_DEBUG_EAX] = 0xaabb3344u;
        patch.values[CORE_MACHINE_DEBUG_ECX] = form == 2u ? 0x11220003u :
            0x11225566u;
        patch.values[CORE_MACHINE_DEBUG_EDI] = 0x10u;
        failed = core_machine_debug_patch_registers(machine, &patch) !=
            LIB_STATUS_OK || core_machine_memory_write(machine, 0x3010u,
            &image, width) != LIB_STATUS_OK;
    }
    if (!failed)
        failed = core_machine_run(machine,
            (core_machine_run_budget){form == 2u ? 2u : 1u, 0u},
            &result) != LIB_STATUS_INTERNAL_ERROR ||
            result.reason != CORE_MACHINE_STOP_FAULT ||
            core_machine_get_cpu_diagnostic(machine, &diagnostic) != LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
    if (!failed)
        failed = !diagnostic.first_fault.valid ||
            !(diagnostic.first_fault.exception_mask & VCPUINS_EXCEPT_DF) ||
            after.eip != 0u || after.eax != 0xaabb3344u ||
            after.ecx != (form == 2u ? 0x11220002u : 0x11225566u) ||
            after.edi != (form == 2u ? 0x11u : 0x10u);
    core_machine_destroy(machine);
    return !failed;
}

static lib_i32 stos_irq_case(lib_bool repeated, lib_u8 width)
{
    static const lib_u8 halt = 0xf4u;
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const lib_u8 code[] = {
        repeated ? 0xf3u : width == 1u ? 0xaau : 0xabu,
        repeated ? 0xaau : 0x90u, 0x90u
    };
    core_machine *machine = LIB_NULL;
    core_machine_pic_irq_source irq = {0};
    core_machine_debug_register_patch patch = {0};
    core_machine_debug_cpu_snapshot after = {0};
    core_machine_run_result result = {0};
    lib_u16 offset = 0x100u, segment = 0u, frame_ip = 0xffffu;
    lib_u8 image[] = {0xa5u, 0xa5u, 0xa5u};
    lib_u8 bytes = repeated ? 3u : 2u;
    lib_u8 image_bytes = repeated ? 3u : width;
    lib_i32 failed = core_machine_create(&config, &machine) != LIB_STATUS_OK;

    if (!failed) {
        patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDI);
        patch.values[CORE_MACHINE_DEBUG_ES] = 0x2000u;
        patch.values[CORE_MACHINE_DEBUG_ESP] = 0x8000u;
        patch.values[CORE_MACHINE_DEBUG_EFLAGS] = VCPU_EFLAGS_IF;
        patch.values[CORE_MACHINE_DEBUG_EAX] = 0xaabb3344u;
        patch.values[CORE_MACHINE_DEBUG_ECX] = repeated ? 0x11220003u :
            0x11225566u;
        patch.values[CORE_MACHINE_DEBUG_EDI] = 0x20u;
        failed = core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            core_machine_reset(machine) != LIB_STATUS_OK ||
            core_machine_debug_patch_registers(machine, &patch) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x20020u, image,
                image_bytes) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0u, code, bytes) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x80u, &offset,
                sizeof(offset)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x82u, &segment,
                sizeof(segment)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x100u, &halt,
                sizeof(halt)) != LIB_STATUS_OK;
    }
    if (!failed) {
        test_pic_program_vector(&machine->board->shared_pic_master, 0x20u);
        core_machine_pic_irq_source_bind(&irq, &machine->board->shared_pic_master,
            &machine->board->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&irq);
        core_machine_pic_irq_source_deassert(&irq);
        failed = core_machine_run(machine,
            (core_machine_run_budget){repeated ? 4u : 2u, 0u},
            &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            core_machine_debug_read_memory(machine,
                after.ss.base + (lib_u16)after.esp, &frame_ip,
                sizeof(frame_ip)) != LIB_STATUS_OK ||
            core_machine_debug_read_memory(machine, 0x20020u, image,
                image_bytes) != LIB_STATUS_OK;
        if (!failed)
            failed = after.eip != 0x101u || frame_ip != (repeated ? 0u : 1u) ||
                after.edi != 0x20u + (repeated ? 1u : width) ||
                (repeated && after.ecx != 0x11220002u) ||
                image[0] != 0x44u ||
                (!repeated && width == 2u && image[1] != 0x33u) ||
                (repeated && (image[1] != 0xa5u || image[2] != 0xa5u)) ||
                !(test_pic_read(&machine->board->shared_pic_master, 0x0bu) &
                    VPIC_ISR_IRQ(0u)) ||
                (test_pic_read(&machine->board->shared_pic_master, 0x0au) &
                    VPIC_IRR_IRQ(0u));
    }
    core_machine_destroy(machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!stos_protected_case(0u) || !stos_protected_case(1u) ||
        !stos_protected_case(2u) ||
        !stos_irq_case(LIB_FALSE, 1u) ||
        !stos_irq_case(LIB_FALSE, 2u) ||
        !stos_irq_case(LIB_TRUE, 1u)) {
        printf("STOS board fault/IRQ failed\n");
        return 1;
    }
    printf("M5:T539:S38:STOS-BOARD:OK\n");
    return 0;
}
