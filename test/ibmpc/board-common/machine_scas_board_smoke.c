#include "lib/types/file.h"
#include "cpu_board_limit_fixture.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include "ibmpc/board-common/machine_board_state.h"
#include "pic_fixture.h"
static lib_i32 scas_protected_case(lib_bool repeated)
{
    static const lib_u8 single[] = {0xaeu};
    static const lib_u8 rep[] = {0xf3u, 0xaeu};
    const lib_u8 *code = repeated ? rep : single;
    lib_u8 bytes = repeated ? sizeof(rep) : sizeof(single);
    core_machine *machine = LIB_NULL;
    core_machine_debug_register_patch patch = {0};
    core_machine_debug_cpu_snapshot after = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_run_result result = {0};
    lib_u8 image[] = {repeated ? 0x10u : 1u, 1u};
    lib_i32 failed = !test_cpu_board_limit_prepare_exact(&machine,
        code, bytes, LIB_TRUE, repeated ? 0x10u : 0x0fu);

    if (!failed) {
        patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDI) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS);
        patch.values[CORE_MACHINE_DEBUG_EAX] = 0xaabb0010u;
        patch.values[CORE_MACHINE_DEBUG_ECX] = 0x11220003u;
        patch.values[CORE_MACHINE_DEBUG_EDI] = 0x10u;
        patch.values[CORE_MACHINE_DEBUG_EFLAGS] = CORE_MACHINE_DEBUG_EFLAGS_IF;
        failed = core_machine_debug_patch_registers(machine, &patch) !=
            LIB_STATUS_OK || core_machine_memory_write(machine,
            0x3010u, image, sizeof(image)) != LIB_STATUS_OK;
    }
    if (!failed)
        failed = core_machine_run(machine,
            (core_machine_run_budget){repeated ? 2u : 1u, 0u},
            &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            result.detail != VCPUINS_EXCEPT_SHUTDOWN ||
            core_machine_get_cpu_diagnostic(machine, &diagnostic) != LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
    if (!failed)
        failed = diagnostic.first_fault.valid || !diagnostic.last_delivered_exception.valid ||
            !(diagnostic.last_delivered_exception.exception_mask & VCPUINS_EXCEPT_SHUTDOWN) ||
            after.eip != 0u || after.eax != 0xaabb0010u ||
            after.ecx != (repeated ? 0x11220002u : 0x11220003u) ||
            after.edi != (repeated ? 0x11u : 0x10u) ||
            after.eflags != (repeated ? CORE_MACHINE_DEBUG_EFLAGS_IF | CORE_MACHINE_DEBUG_EFLAGS_PF |
                CORE_MACHINE_DEBUG_EFLAGS_ZF : CORE_MACHINE_DEBUG_EFLAGS_IF);
    core_machine_destroy(machine);
    return !failed;
}

static lib_i32 scas_irq_case(lib_bool repeated)
{
    static const lib_u8 single[] = {0xaeu, 0x90u};
    static const lib_u8 rep[] = {0xf3u, 0xaeu, 0x90u};
    static const lib_u8 halt = 0xf4u;
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const lib_u8 *code = repeated ? rep : single;
    lib_u8 bytes = repeated ? sizeof(rep) : sizeof(single);
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_pic_irq_source *irq = LIB_NULL;
    core_machine_debug_register_patch patch = {0};
    core_machine_debug_cpu_snapshot after = {0};
    core_machine_run_result result = {0};
    lib_u16 offset = 0x100u, segment = 0u, frame_ip = 0xffffu;
    lib_u8 image[] = {0x10u, 1u, 1u};
    lib_i32 failed = core_machine_create(&config, &machine, &board) != LIB_STATUS_OK;

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
        patch.values[CORE_MACHINE_DEBUG_EFLAGS] = CORE_MACHINE_DEBUG_EFLAGS_IF;
        patch.values[CORE_MACHINE_DEBUG_EAX] = 0xaabb0010u;
        patch.values[CORE_MACHINE_DEBUG_ECX] = 0x11220003u;
        patch.values[CORE_MACHINE_DEBUG_EDI] = 0x20u;
        failed = core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            core_machine_reset(machine) != LIB_STATUS_OK ||
            core_machine_debug_patch_registers(machine, &patch) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x20020u, image,
                sizeof(image)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0u, code, bytes) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x80u, &offset,
                sizeof(offset)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x82u, &segment,
                sizeof(segment)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x100u, &halt,
                sizeof(halt)) != LIB_STATUS_OK;
    }
    if (!failed) {
        test_pic_program_vector(board->shared_pic_master, 0x20u);
        test_pic_bind_source(&irq, board->shared_pic_master,
            board->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(irq);
        core_machine_pic_irq_source_deassert(irq);
        failed = core_machine_run(machine,
            (core_machine_run_budget){repeated ? 4u : 2u, 0u},
            &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            core_machine_debug_read_memory(machine,
                after.ss.base + (lib_u16)after.esp, &frame_ip,
                sizeof(frame_ip)) != LIB_STATUS_OK;
        if (!failed)
            failed = after.eip != 0x101u ||
                frame_ip != (repeated ? 0u : 1u) ||
                after.edi != 0x21u ||
                (repeated && after.ecx != 0x11220002u) ||
                !(test_pic_read(board->shared_pic_master, 0x0bu) &
                    VPIC_ISR_IRQ(0u)) ||
                (test_pic_read(board->shared_pic_master, 0x0au) &
                    VPIC_IRR_IRQ(0u));
    }
    core_machine_destroy(machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!scas_protected_case(LIB_FALSE) ||
        !scas_protected_case(LIB_TRUE) ||
        !scas_irq_case(LIB_FALSE) || !scas_irq_case(LIB_TRUE)) {
        lib_c_printf("SCAS board fault/IRQ failed\n");
        return 1;
    }
    lib_c_printf("M5:T539:S38:SCAS-BOARD:OK\n");
    return 0;
}
