#include "../../../../x86/ibmpc-common/pic_fixture.h"
#include "app-nxvm/devices/machine_board_interface.h"
#include "app-nxvm/devices/machine_board_state.h"
#include "support/core_machine_board_fixture.h"
#include "x86/core/device_support_interface.h"
#include "x86/ibmpc-common/pic_bus_interface.h"
#include "x86/core/debug_interface.h"
#include <stdio.h>

static lib_i32 sign_extend_test_irq(void)
{
    static const lib_u8 opcodes[] = {0x98u, 0x99u};
    static const lib_u8 hlt = 0xf4u;
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    lib_u8 opcode;

    for (opcode = 0u; opcode < sizeof(opcodes); ++opcode) {
        core_machine *machine = LIB_NULL;
        core_machine_board_state *board = LIB_NULL;
        core_machine_pic_irq_source *source = LIB_NULL;
        core_machine_run_result result = {0};
        core_machine_debug_cpu_snapshot after = {0};
        core_machine_debug_register_patch patch = {0};
        const lib_u8 code[] = {opcodes[opcode], 0x90u};
        lib_u16 offset = 0x0100u, segment = 0u, frame = 0u;
        lib_i32 failed = core_machine_create(&config, &machine, &board) != LIB_STATUS_OK ||
            core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            core_machine_reset(machine) != LIB_STATUS_OK;

        if (!failed) {
            patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDX) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBX) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBP) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESI) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDI) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS);
            patch.values[CORE_MACHINE_DEBUG_EAX] = opcodes[opcode] == 0x98u ?
                0xaabb0080u : 0xaabb8000u;
            patch.values[CORE_MACHINE_DEBUG_ECX] = 0x11223344u;
            patch.values[CORE_MACHINE_DEBUG_EDX] = 0x55660000u;
            patch.values[CORE_MACHINE_DEBUG_EBX] = 0x778899aau;
            patch.values[CORE_MACHINE_DEBUG_ESP] = 0xbbbb8000u;
            patch.values[CORE_MACHINE_DEBUG_EBP] = 0xccccddddu;
            patch.values[CORE_MACHINE_DEBUG_ESI] = 0xeeeeffffu;
            patch.values[CORE_MACHINE_DEBUG_EDI] = 0x10203040u;
            patch.values[CORE_MACHINE_DEBUG_EFLAGS] = VCPU_EFLAGS_IF;
            failed |= core_machine_debug_patch_registers(machine, &patch) !=
                    LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0u, code, sizeof(code)) !=
                    LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x80u, &offset,
                    sizeof(offset)) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x82u, &segment,
                    sizeof(segment)) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x100u, &hlt,
                    sizeof(hlt)) != LIB_STATUS_OK;
        }
        if (!failed) {
            test_pic_program_vector(board->shared_pic_master, 0x20u);
            core_machine_pic_irq_source_bind(&source,
                board->shared_pic_master, board->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(source);
            core_machine_pic_irq_source_deassert(source);
            failed |= core_machine_run(machine,
                    (core_machine_run_budget){2u, 0u}, &result) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
                core_machine_debug_capture_cpu_snapshot(machine,
                    CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
                core_machine_memory_read(machine,
                    after.ss.base + (lib_u16)after.esp, &frame,
                    sizeof(frame)) != LIB_STATUS_OK ||
                after.eip != 0x101u || frame != 1u ||
                after.eax != (opcodes[opcode] == 0x98u ?
                    0xaabbff80u : 0xaabb8000u) ||
                after.edx != (opcodes[opcode] == 0x99u ?
                    0x5566ffffu : 0x55660000u) ||
                !CORE_MACHINE_BIT_IS_SET(
                    test_pic_read(board->shared_pic_master, 0x0bu),
                    VPIC_ISR_IRQ(0u));
        }
        core_machine_destroy(machine);
        if (failed) return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!sign_extend_test_irq()) return 1;
    printf("M5:T316:S29:SIGN-EXTEND:OK\n");
    printf("M5:T401:S45:SIGN-EXTEND-PROFILES:OK\n");
    return 0;
}
