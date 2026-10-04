#include "../ibmpc-at/controller_fixture.h"
#include "core_machine_board_fixture.h"
#include "pic_fixture.h"
#include "x86/ibmpc-common/machine_board_state.h"

typedef struct core_machine_kbc_cpu_fixture {
    core_machine *machine;
    core_machine_board_state *board;
    lib_status reset_status;
} core_machine_kbc_cpu_fixture;

static void core_machine_kbc_cpu_reset(void *opaque)
{
    core_machine_kbc_cpu_fixture *fixture = opaque;
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP)
    };

    if (fixture != LIB_NULL)
        fixture->reset_status = test_kbc_core_reset_entry(fixture->machine, &entry);
}

static const core_machine_execution_provider core_machine_kbc_cpu_provider = {
    core_machine_kbc_cpu_reset, LIB_NULL
};

/* Keep the POST-relevant path owner-local: a real CPU issues FFh, reads its
 * synchronous FAh, then receives the queued AAh through IRQ1 after STI's
 * architectural interrupt shadow. */
lib_i32 test_kbc_board_cpu_reset_irq1(void)
{
    static const lib_u8 code[] = {
        0xb0u, 0xffu, 0xe6u, 0x60u, 0xe4u, 0x60u, 0xfbu, 0x90u
    };
    static const lib_u8 handler = 0xf4u;
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80286,
        .fpu_profile = X86_FPU_PROFILE_NONE,
        .shared_pit_personality = X86_PIT_PERSONALITY_8253
    };
    core_machine_kbc_cpu_fixture fixture = {0};
    core_machine_run_result result;
    core_machine_cpu_state cpu;
    lib_u16 offset = 0x0100u;
    lib_u16 segment = 0u;
    lib_i32 failed = !test_core_machine_fixture_create_bind_freeze_reset(&config,
        &core_machine_kbc_cpu_provider, &fixture, &fixture.machine, &fixture.board) ||
        fixture.reset_status != LIB_STATUS_OK;

    if (!failed) {
        test_kbc_initialize_pic(fixture.machine);
        failed |= core_machine_bus_write(fixture.machine, 0x0021u, 0xfdu) != LIB_STATUS_OK;
        failed |= core_machine_memory_write(fixture.machine, 0u, code, sizeof(code)) !=
                LIB_STATUS_OK ||
            core_machine_memory_write(fixture.machine, 0x0024u, &offset,
                sizeof(offset)) != LIB_STATUS_OK ||
            core_machine_memory_write(fixture.machine, 0x0026u, &segment,
                sizeof(segment)) != LIB_STATUS_OK ||
            core_machine_memory_write(fixture.machine, offset, &handler,
                sizeof(handler)) != LIB_STATUS_OK;
    }
    if (!failed) {
        failed |= core_machine_run(fixture.machine, (core_machine_run_budget){3u, 0u},
                &result) != LIB_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_state(fixture.machine, &cpu) != LIB_STATUS_OK ||
            cpu.eip != 6u ||
            test_kbc_bat_ready(fixture.board->shared_kbc) ||
            (test_core_machine_fixture_read_port(fixture.machine, 0x64u) & 0x01u) == 0u ||
            !core_machine_pic_irq_source_is_asserted(fixture.board->keyboard_irq1_source);
    }
    if (!failed) {
        failed |= core_machine_run(fixture.machine, (core_machine_run_budget){2u, 0u},
                &result) != LIB_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_state(fixture.machine, &cpu) != LIB_STATUS_OK ||
            cpu.eip != offset ||
            !test_pic_bit_is_set(test_pic_read(fixture.board->shared_pic_master, 0x0bu),
                VPIC_ISR_IRQ(1u)) ||
            test_core_machine_fixture_read_port(fixture.machine, 0x60u) != 0xaau ||
            (test_core_machine_fixture_read_port(fixture.machine, 0x64u) & 0x01u) != 0u;
    }
    core_machine_destroy(fixture.machine);
    return failed;
}
