#include "support/pic_fixture.h"
#include "app-nxvm/devices/machine_board_interface.h"
#include "app-nxvm/devices/machine_board_state.h"
#include "support/core_machine_board_fixture.h"
#include "x86/core/device_support_interface.h"
#include "app-nxvm/devices/pic_bus.h"
#include <stdio.h>

typedef struct lld_machine { core_machine *machine;
    core_machine_board_state *board; } lld_machine;

static lib_i32 lld_prepare(core_machine_cpu_profile profile, lld_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile, .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP)
    };
    lib_memory_set(state, 0, sizeof(*state));
    return core_machine_create(&config, &state->machine, &state->board) == LIB_STATUS_OK &&
        core_machine_freeze_execution_providers(state->machine) == LIB_STATUS_OK &&
        core_machine_reset(state->machine) == LIB_STATUS_OK &&
        core_machine_debug_patch_registers(state->machine, &entry) == LIB_STATUS_OK;
}

static lib_i32 lld_prepare_protected(lld_machine *state)
{
    static const lib_u8 pointer[] = { 0x1fu, 0, 0, 0x03u, 0, 0 };
    static const lib_u8 gdt[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0xffu, 0xffu, 0, 0x20u, 0, 0x9au, 0, 0,
        0xffu, 0xffu, 0, 0, 0, 0x92u, 0, 0, 0xffu, 0xffu, 0, 0x40u, 0, 0x92u,
        0, 0
    };
    static const lib_u8 bootstrap[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u, 0xb8u, 0x01u, 0x00u, 0x0fu, 0x01u,
        0xf0u, 0xb8u, 0x10u, 0x00u, 0x8eu, 0xd8u, 0x8eu, 0xc0u, 0xb8u, 0x18u,
        0x00u, 0x8eu, 0xd0u, 0xbcu, 0x00u, 0x80u, 0xeau, 0x00u, 0x00u, 0x08u,
        0x00u
    };
    core_machine_run_result result;

    return lld_prepare(CORE_MACHINE_CPU_PROFILE_80386, state) &&
        core_machine_memory_write(state->machine, 0x0100u, pointer, sizeof(pointer)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x0300u, gdt, sizeof(gdt)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, bootstrap, sizeof(bootstrap)) == LIB_STATUS_OK &&
        core_machine_run(state->machine, (core_machine_run_budget){10u,0u},
            &result) == LIB_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET && result.executed == 10u;
}

static lib_i32 lld_test_source_fault_atomicity(void)
{
    static const lib_u8 opcodes[] = { 0xc4u, 0xc5u };
    lib_u8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        lld_machine state;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_debug_cpu_snapshot before;
        core_machine_debug_cpu_snapshot after;
        lib_u8 code[] = { opcodes[opcode], 0x06u, 0x00u, 0x10u };
        lib_i32 failed = !lld_prepare_protected(&state);

        if (!failed) {
            const lib_u8 limit[] = {0x01u,0x10u};
            failed |= core_machine_memory_write(state.machine, 0x310u,
                limit, sizeof(limit)) != LIB_STATUS_OK;
            failed |= core_machine_debug_patch_registers(state.machine,
                &(core_machine_debug_register_patch){
                    .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
                        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
                        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
                        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
                    .values = { [CORE_MACHINE_DEBUG_DS] = 0x10u,
                        [CORE_MACHINE_DEBUG_EAX] = 0x55557777u,
                        [CORE_MACHINE_DEBUG_ES] = 0x10u,
                        [CORE_MACHINE_DEBUG_EFLAGS] = 0x41u }
                }) != LIB_STATUS_OK;
            failed |= core_machine_memory_write(state.machine, 0x2000u, code,
                    sizeof(code)) != LIB_STATUS_OK;

            failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
            failed |= core_machine_run(state.machine,
                    (core_machine_run_budget){ 1u, 0u }, &result) !=
                        LIB_STATUS_INTERNAL_ERROR || result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                    LIB_STATUS_OK;
            failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
            failed |= !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
                    diagnostic.first_fault.exception_mask, (1u << 8)) ||
                after.eip != before.eip ||
                after.eax != before.eax ||
                after.eflags != before.eflags ||
                (opcode == 0u ? after.es.selector : after.ds.selector) !=
                    (opcode == 0u ? before.es.selector :
                        before.ds.selector);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 lld_test_irq_no_shadow(void)
{
    static const lib_u8 opcodes[] = { 0xc4u, 0xc5u };
    static const lib_u8 pointer[] = { 0x44u, 0x33u, 0x00u, 0x00u };
    static const lib_u8 hlt = 0xf4u;
    lib_u8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        lld_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result;
        core_machine_debug_cpu_snapshot after;
        lib_u16 vector_offset = 0x0100u;
        lib_u16 vector_segment = 0u;
        lib_u16 frame_ip = 0u;
        lib_u8 code[] = { opcodes[opcode], 0x06u, 0x00u, 0x10u, 0x90u };
        lib_i32 failed = !lld_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            failed |= core_machine_memory_write(state.machine, 0x1000u, pointer,
                    sizeof(pointer)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0u, code,
                    sizeof(code)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x20u * 4u,
                    &vector_offset, sizeof(vector_offset)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x20u * 4u + 2u,
                    &vector_segment, sizeof(vector_segment)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x0100u, &hlt,
                    sizeof(hlt)) != LIB_STATUS_OK;
        }
        if (!failed) {
            failed |= core_machine_debug_patch_registers(state.machine,
                &(core_machine_debug_register_patch){
                    .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
                    .values = { [CORE_MACHINE_DEBUG_EFLAGS] = 0x200u }
                }) != LIB_STATUS_OK;
            lib_memory_set(&source, 0, sizeof(source));
            test_pic_program_vector(&state.board->shared_pic_master, 0x20u);
            core_machine_pic_irq_source_bind(&source,
                &state.board->shared_pic_master, &state.board->shared_pic_slave,
                0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
            failed |= core_machine_run(state.machine,
                    (core_machine_run_budget){ 2u, 0u }, &result) !=
                        LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                    after.ss.base + (lib_u16)after.esp,
                    (lib_uptr)&frame_ip, sizeof(frame_ip)) !=
                        LIB_STATUS_OK || after.eip != 0x0101u ||
                !CORE_MACHINE_BIT_IS_SET(test_pic_read(&state.board->shared_pic_master, 0x0bu),
                    VPIC_ISR_IRQ(0u)) || CORE_MACHINE_BIT_IS_SET(
                    test_pic_read(&state.board->shared_pic_master, 0x0au), VPIC_IRR_IRQ(0u)) ||
                frame_ip != 4u;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!lld_test_source_fault_atomicity() ||
        !lld_test_irq_no_shadow()) return 1;
    printf("M5:T539:S27:LES_LDS-BOARD:OK\n");
    return 0;
}
