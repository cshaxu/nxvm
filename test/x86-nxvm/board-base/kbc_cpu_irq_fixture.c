#include "../board-at/controller_fixture.h"
#include "kbc_controller_fixture.h"
#include "composition/bus_fixture.h"
#include "pic_fixture.h"
#include "x86-nxvm/board-base/machine_board_state.h"

typedef struct x86_nxvm_test_kbc_cpu_fixture {
    x86_core_machine *machine;
    x86_nxvm_board_state *board;
    lib_status reset_status;
} x86_nxvm_test_kbc_cpu_fixture;

static void x86_nxvm_test_kbc_cpu_reset(void *opaque)
{
    x86_nxvm_test_kbc_cpu_fixture *fixture = opaque;
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ES) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP)
    };

    if (fixture != LIB_NULL)
        fixture->reset_status = x86_core_machine_debug_patch_registers(
            fixture->machine, &entry);
}

/* Keep the POST-relevant path owner-local: a real CPU issues FFh, reads its
 * synchronous FAh, then receives the queued AAh through IRQ1 after STI's
 * architectural interrupt shadow. */
lib_i32 test_kbc_board_cpu_reset_irq1(void)
{
    static const lib_u8 code[] = {
        0xb0u, 0xffu, 0xe6u, 0x60u, 0xe4u, 0x60u, 0xfbu, 0x90u
    };
    static const lib_u8 handler = 0xf4u;
    const x86_nxvm_board_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = X86_CORE_CPU_PROFILE_80286,
        .fpu_profile = X86_FPU_PROFILE_NONE,
        .shared_pit_personality = X86_PIT_PERSONALITY_8253
    };
    x86_nxvm_test_kbc_cpu_fixture fixture = {0};
    x86_core_machine_run_result result;
    x86_core_cpu_state cpu;
    lib_u16 offset = 0x0100u;
    lib_u16 segment = 0u;
    lib_i32 failed = x86_nxvm_board_create(&config, &fixture.machine,
        &fixture.board) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(fixture.machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(fixture.machine) != LIB_STATUS_OK;

    if (!failed) {
        x86_nxvm_test_kbc_cpu_reset(&fixture);
        failed |= fixture.reset_status != LIB_STATUS_OK;
    }

    if (!failed) {
        test_kbc_initialize_pic(fixture.machine);
        failed |= x86_core_machine_bus_write(fixture.machine, 0x0021u, 0xfdu) != LIB_STATUS_OK;
        failed |= x86_core_machine_memory_write(fixture.machine, 0u, code, sizeof(code)) !=
                LIB_STATUS_OK ||
            x86_core_machine_memory_write(fixture.machine, 0x0024u, &offset,
                sizeof(offset)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(fixture.machine, 0x0026u, &segment,
                sizeof(segment)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(fixture.machine, offset, &handler,
                sizeof(handler)) != LIB_STATUS_OK;
    }
    if (!failed) {
        failed |= x86_core_machine_run(fixture.machine, (x86_core_machine_run_budget){3u, 0u},
                &result) != LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_BUDGET ||
            x86_core_machine_get_cpu_state(fixture.machine, &cpu) != LIB_STATUS_OK ||
            cpu.eip != 6u ||
            test_kbc_bat_ready(fixture.board->shared_kbc) ||
            (test_x86_nxvm_machine_fixture_read_port(fixture.machine, 0x64u) & 0x01u) == 0u ||
            !x86_nxvm_board_pic_irq_source_is_asserted(fixture.board->keyboard_irq1_source);
    }
    if (!failed) {
        failed |= x86_core_machine_run(fixture.machine, (x86_core_machine_run_budget){2u, 0u},
                &result) != LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_BUDGET ||
            x86_core_machine_get_cpu_state(fixture.machine, &cpu) != LIB_STATUS_OK ||
            cpu.eip != offset ||
            !test_pic_bit_is_set(test_pic_read(fixture.board->shared_pic_master, 0x0bu),
                VPIC_ISR_IRQ(1u)) ||
            test_x86_nxvm_machine_fixture_read_port(fixture.machine, 0x60u) != 0xaau ||
            (test_x86_nxvm_machine_fixture_read_port(fixture.machine, 0x64u) & 0x01u) != 0u;
    }
    x86_core_machine_destroy(fixture.machine);
    return failed;
}
