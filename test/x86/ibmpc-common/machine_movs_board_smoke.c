#include "cpu_board_limit_fixture.h"
#include "x86/ibmpc-common/machine_board_interface.h"
#include "x86/ibmpc-common/machine_board_state.h"
#include "pic_fixture.h"
#include <stdio.h>

static lib_i32 movs_irq_case(lib_bool repeated)
{
    static const lib_u8 hlt = 0xf4u;
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const lib_u8 code[] = {repeated ? 0xf3u : 0xa4u,
        repeated ? 0xa4u : 0x90u, 0x90u};
    const lib_u8 source[] = {0x51u, 0x62u, 0x73u};
    lib_u8 source_after[] = {0u, 0u, 0u};
    lib_u8 destination[] = {0xa5u, 0xa5u, 0xa5u};
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_pic_irq_source *irq = LIB_NULL;
    core_machine_debug_register_patch patch = {0};
    core_machine_debug_cpu_snapshot after = {0};
    core_machine_run_result result = {0};
    lib_u16 offset = 0x100u, segment = 0u, frame_ip = 0xffffu;
    lib_u8 bytes = repeated ? 3u : 2u;
    lib_u8 count = repeated ? 3u : 1u;
    lib_i32 failed = core_machine_create(&config, &machine, &board) != LIB_STATUS_OK;

    if (!failed) {
        patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESI) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDI);
        patch.values[CORE_MACHINE_DEBUG_DS] = 0x1000u;
        patch.values[CORE_MACHINE_DEBUG_ES] = 0x2000u;
        patch.values[CORE_MACHINE_DEBUG_ESP] = 0x8000u;
        patch.values[CORE_MACHINE_DEBUG_EFLAGS] = CORE_MACHINE_DEBUG_EFLAGS_IF;
        patch.values[CORE_MACHINE_DEBUG_ECX] = 0x11220000u | count;
        patch.values[CORE_MACHINE_DEBUG_ESI] = 0x10u;
        patch.values[CORE_MACHINE_DEBUG_EDI] = 0x20u;
        failed = core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            core_machine_reset(machine) != LIB_STATUS_OK ||
            core_machine_debug_patch_registers(machine, &patch) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x10010u, source, count) !=
                LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x20020u, destination, count) !=
                LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0u, code, bytes) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x80u, &offset,
                sizeof(offset)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x82u, &segment,
                sizeof(segment)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x100u, &hlt,
                sizeof(hlt)) != LIB_STATUS_OK;
    }
    if (!failed) {
        test_pic_program_vector(board->shared_pic_master, 0x20u);
        test_pic_bind_source(&irq, board->shared_pic_master,
            board->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(irq);
        core_machine_pic_irq_source_deassert(irq);
        failed = core_machine_run(machine,
                (core_machine_run_budget){count + 1u, 0u},
                &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            core_machine_debug_read_memory(machine,
                after.ss.base + (lib_u16)after.esp, &frame_ip,
                sizeof(frame_ip)) != LIB_STATUS_OK ||
            core_machine_debug_read_memory(machine, 0x20020u, destination,
                count) != LIB_STATUS_OK ||
            core_machine_debug_read_memory(machine, 0x10010u, source_after,
                count) != LIB_STATUS_OK;
        if (!failed)
            failed = after.eip != 0x101u || frame_ip != (repeated ? 0u : 1u) ||
                after.esi != 0x11u || after.edi != 0x21u ||
                after.ecx != (repeated ? 0x11220002u : 0x11220001u) ||
                lib_memory_compare(source_after, source, count) != 0 ||
                destination[0] != source[0] ||
                (repeated && (destination[1] != 0xa5u ||
                    destination[2] != 0xa5u)) ||
                !(test_pic_read(board->shared_pic_master, 0x0bu) &
                    VPIC_ISR_IRQ(0u)) ||
                (test_pic_read(board->shared_pic_master, 0x0au) &
                    VPIC_IRR_IRQ(0u));
    }
    core_machine_destroy(machine);
    return !failed;
}

static lib_i32 movs_protected_case(lib_u8 form)
{
    static const lib_u8 codes[][2] = {{0xa4u, 0u}, {0x66u, 0xa5u}};
    core_machine *machine = LIB_NULL;
    core_machine_debug_register_patch patch = {0};
    core_machine_debug_cpu_snapshot after = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_run_result result = {0};
    lib_u32 source = 0x11223344u;
    lib_u32 destination = 0xa5a5a5a5u;
    lib_u32 source_address = form == 0u ? 0x3010u : 0x4010u;
    lib_u32 destination_address = form == 0u ? 0x3020u : 0x3010u;
    lib_u8 width = form == 0u ? 1u : 4u;
    lib_i32 failed = !test_cpu_board_limit_prepare_exact(&machine,
        codes[form], form == 0u ? 1u : 2u, LIB_TRUE, 0x0fu);

    if (!failed) {
        patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESI) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDI) |
            (form == 1u ? CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) : 0u);
        patch.values[CORE_MACHINE_DEBUG_EAX] = 0xaabb3344u;
        patch.values[CORE_MACHINE_DEBUG_ESI] = 0x10u;
        patch.values[CORE_MACHINE_DEBUG_EDI] = 0x20u;
        patch.values[CORE_MACHINE_DEBUG_DS] = 0x18u;
        failed = core_machine_debug_patch_registers(machine, &patch) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, source_address, &source,
                width) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, destination_address,
                &destination, width) != LIB_STATUS_OK;
    }
    if (!failed) {
        failed = core_machine_run(machine, (core_machine_run_budget){1u, 0u},
                &result) != LIB_STATUS_INTERNAL_ERROR ||
            result.reason != CORE_MACHINE_STOP_FAULT ||
            core_machine_get_cpu_diagnostic(machine, &diagnostic) != LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
        if (!failed)
            failed = !diagnostic.first_fault.valid ||
                !(diagnostic.first_fault.exception_mask & VCPUINS_EXCEPT_DF) ||
                after.eip != 0u || after.eax != 0xaabb3344u ||
                after.esi != 0x10u || after.edi != 0x20u;
    }
    core_machine_destroy(machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!movs_irq_case(LIB_FALSE) || !movs_irq_case(LIB_TRUE)) {
        printf("MOVS board stage=irq\n");
        return 1;
    }
    for (lib_u8 form = 0u; form != 2u; ++form)
        if (!movs_protected_case(form)) {
            printf("MOVS board stage=protected form=%u\n", form);
            return 1;
        }
    printf("M5:T539:S37:MOVS-BOARD:OK\n");
    return 0;
}
