#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "pic_fixture.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include "ibmpc/board-common/machine_board_state.h"
#include "core_machine_board_fixture.h"
#include "x86/core/device_support_interface.h"
#include "ibmpc/board-common/pic_bus_interface.h"
typedef struct les_lds_machine { core_machine *machine;
    core_machine_board_state *board; } les_lds_machine;

static lib_i32 les_lds_prepare(core_machine_cpu_profile profile, les_lds_machine *state)
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

static lib_i32 les_lds_seed(les_lds_machine *state)
{
    return core_machine_debug_patch_registers(state->machine,
                &(core_machine_debug_register_patch){
                    .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
                        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
                        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDX) |
                        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBX) |
                        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
                        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBP) |
                        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESI) |
                        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDI) |
                        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
                    .values = { [CORE_MACHINE_DEBUG_EAX] = 0xaabbccddU,
                        [CORE_MACHINE_DEBUG_ECX] = 0x11223344u,
                        [CORE_MACHINE_DEBUG_EDX] = 0x55667788u,
                        [CORE_MACHINE_DEBUG_EBX] = 0x99aabbccU,
                        [CORE_MACHINE_DEBUG_ESP] = 0x8000u,
                        [CORE_MACHINE_DEBUG_EBP] = 0x120u,
                        [CORE_MACHINE_DEBUG_ESI] = 0x10u,
                        [CORE_MACHINE_DEBUG_EDI] = 0x20u,
                        [CORE_MACHINE_DEBUG_EFLAGS] = 0x8d5u }
                }) == LIB_STATUS_OK;
}

static lib_i32 les_lds_gprs_same_except_eax(const core_machine_debug_cpu_snapshot *before,
    const core_machine_debug_cpu_snapshot *after)
{
    return after->ecx == before->ecx &&
        after->edx == before->edx &&
        after->ebx == before->ebx &&
        after->esp == before->esp &&
        after->ebp == before->ebp &&
        after->esi == before->esi &&
        after->edi == before->edi;
}

static lib_i32 les_lds_irq_gprs_same_except_eax(const core_machine_debug_cpu_snapshot *before,
    const core_machine_debug_cpu_snapshot *after)
{
    return after->ecx == before->ecx &&
        after->edx == before->edx &&
        after->ebx == before->ebx &&
        after->ebp == before->ebp &&
        after->esi == before->esi &&
        after->edi == before->edi;
}

static lib_i32 les_lds_read(les_lds_machine *state, lib_u32 physical,
    void *data, lib_u8 bytes)
{
    return core_machine_memory_inspect(state->machine,
        physical, (void *)data, bytes) == LIB_STATUS_OK;
}

static lib_i32 les_lds_boot_protected(les_lds_machine *state)
{
    static const lib_u8 pointer[] = {0x3fu,0u,0u,0x03u,0u,0u};
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0, 0xffu,0xffu,0,0x40u,0,0x92u,0,0,
        0xffu,0xffu,0,0x50u,0,0x12u,0,0, 0xffu,0xffu,0,0x60u,0,0x98u,0,0,
        0xffu,0xffu,0,0x70u,0,0x92u,0,0, 0xffu,0xffu,0,0x80u,0,0x92u,0,0
    };
    static const lib_u8 boot[] = {
        0x0fu,0x01u,0x16u,0,1u, 0xb8u,1u,0,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0,0x8eu,0xd8u, 0xb8u,0x18u,0,0x8eu,0xc0u,
        0xb8u,0x10u,0,0x8eu,0xd0u,0xbcu,0,0x80u, 0xeau,0,0,8u,0
    };
    core_machine_run_result result;

    return core_machine_memory_write(state->machine, 0x100u, pointer, sizeof(pointer)) == LIB_STATUS_OK && core_machine_memory_write(state->machine, 0x300u, gdt, sizeof(gdt)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, boot, sizeof(boot)) ==
        LIB_STATUS_OK && core_machine_run(
        state->machine, (core_machine_run_budget){11u,0u}, &result) ==
        LIB_STATUS_OK && result.reason == CORE_MACHINE_STOP_BUDGET && result.executed == 11u;
}

static lib_i32 les_lds_protected_case(lib_u8 opcode, lib_u16 selector)
{
    les_lds_machine state;
    core_machine_debug_cpu_snapshot before;
    core_machine_debug_cpu_snapshot after;
    core_machine_run_result result;
    lib_u8 pointer[] = {0x44u,0x33u,0,0};
    lib_u8 source[4] = {0x44u,0x33u,0,0};
    lib_u8 observed[4] = {0};
    lib_u8 program[4] = {opcode,0x06u,0x10u,0u};
    lib_i32 failed = !les_lds_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);

    pointer[2] = (lib_u8)selector;
    pointer[3] = (lib_u8)(selector >> 8u);
    source[2] = pointer[2];
    source[3] = pointer[3];
    if (!failed)
        failed |= !les_lds_boot_protected(&state);
    if (!failed) {

        failed |= !les_lds_seed(&state);
        failed |= core_machine_debug_patch_registers(state.machine,
                &(core_machine_debug_register_patch){
                    .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES),
                    .values = { [CORE_MACHINE_DEBUG_ES] = opcode == 0xc4u ? 0x18u : 0x10u }
                }) != LIB_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x3010u, pointer,
            sizeof(pointer)) != LIB_STATUS_OK || core_machine_memory_write(
            state.machine, 0x2000u, program, sizeof(program)) != LIB_STATUS_OK;

        failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
        failed |= !test_core_machine_fixture_shutdown_wait(core_machine_run(
            state.machine, (core_machine_run_budget){1u,0u}, &result), &result);
        failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
        failed |= !les_lds_read(&state, 0x3010u, observed,
            sizeof(observed)) || lib_memory_compare(source, observed,
            sizeof(source)) != 0;
        if (!failed) failed |= after.eip != before.eip ||
            after.eax != before.eax ||
            !les_lds_gprs_same_except_eax(&before, &after) ||
            after.eflags != before.eflags ||
            lib_memory_compare(opcode == 0xc4u ? &before.es : &before.ds,
            opcode == 0xc4u ? &after.es : &after.ds,
            sizeof(core_machine_debug_segment_snapshot)) != 0 || lib_memory_compare(&before.cs,
            &after.cs, sizeof(before.cs)) != 0 ||
            lib_memory_compare(&before.ss, &after.ss, sizeof(before.ss)) != 0 ||
            lib_memory_compare(opcode == 0xc4u ? &before.ds : &before.es,
            opcode == 0xc4u ? &after.ds : &after.es,
            sizeof(core_machine_debug_segment_snapshot)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 les_lds_test_protected(void)
{
    static const lib_u8 opcodes[] = {0xc4u,0xc5u};
    lib_u8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        if (!les_lds_protected_case(opcodes[opcode], 0x20u) ||
            !les_lds_protected_case(opcodes[opcode], 0x28u) ||
            !les_lds_protected_case(opcodes[opcode], 0x33u))
            return 0;
    }
    return 1;
}

static lib_i32 les_lds_test_limit(void)
{
    static const lib_u8 opcodes[] = {0xc4u,0xc5u};
    lib_u8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        les_lds_machine state;
        core_machine_debug_cpu_snapshot before;
        core_machine_debug_cpu_snapshot after;
        core_machine_run_result result;
        lib_u8 code[] = {opcodes[opcode],0x06u,0x10u,0u};
        lib_i32 failed = !les_lds_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed)
            failed |= !les_lds_boot_protected(&state);
        if (!failed) {
            failed |= !les_lds_seed(&state);
            const lib_u8 limit[] = {0x11u,0u};
            failed |= core_machine_memory_write(state.machine, 0x310u,
                limit, sizeof(limit)) != LIB_STATUS_OK;
            failed |= core_machine_debug_patch_registers(state.machine,
                &(core_machine_debug_register_patch){
                    .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS),
                    .values = { [CORE_MACHINE_DEBUG_DS] = 0x10u }
                }) != LIB_STATUS_OK;
            failed |= core_machine_memory_write(state.machine, 0x2000u, code,
                sizeof(code)) != LIB_STATUS_OK;

            failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
            failed |= !test_core_machine_fixture_shutdown_wait(core_machine_run(
                state.machine, (core_machine_run_budget){1u,0u}, &result),
                &result);
            failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
            if (!failed) failed |= after.eip != before.eip ||
                after.eax != before.eax ||
                !les_lds_gprs_same_except_eax(&before, &after) ||
                after.eflags != before.eflags ||
                lib_memory_compare(opcodes[opcode] == 0xc4u ? &before.es :
                &before.ds, opcodes[opcode] == 0xc4u ? &after.es :
                &after.ds, sizeof(core_machine_debug_segment_snapshot)) != 0;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 les_lds_test_irq(void)
{
    static const lib_u8 opcodes[] = {0xc4u,0xc5u};
    static const lib_u8 pointer[] = {0x44u,0x33u,0,0};
    static const lib_u8 hlt = 0xf4u;
    lib_u8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        les_lds_machine state;
        core_machine_pic_irq_source *irq = LIB_NULL;
        core_machine_run_result result;
        core_machine_debug_cpu_snapshot before;
        core_machine_debug_cpu_snapshot after;
        lib_u16 offset = 0x100u;
        lib_u16 segment = 0u;
        lib_u16 frame_ip = 0u;
        lib_u8 code[] = {opcodes[opcode],0x06u,0,0x10u,0x90u};
        lib_i32 failed = !les_lds_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed) {
            failed |= core_machine_memory_write(state.machine, 0x1000u, pointer,
                sizeof(pointer)) != LIB_STATUS_OK || core_machine_memory_write(
                state.machine, 0u, code, sizeof(code)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x80u, &offset,
                sizeof(offset)) != LIB_STATUS_OK || core_machine_memory_write(
                state.machine, 0x82u, &segment, sizeof(segment)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x100u, &hlt,
                sizeof(hlt)) != LIB_STATUS_OK;
        }
        if (!failed) {
            failed |= !les_lds_seed(&state);
            failed |= core_machine_debug_patch_registers(state.machine,
                &(core_machine_debug_register_patch){
                    .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
                    .values = { [CORE_MACHINE_DEBUG_EFLAGS] = 0x200u }
                }) != LIB_STATUS_OK;
            failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
            lib_memory_set(&irq, 0, sizeof(irq));
            test_pic_program_vector(state.board->shared_pic_master, 0x20u);
            test_pic_bind_source(&irq, state.board->shared_pic_master,
                state.board->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(irq);
            core_machine_pic_irq_source_deassert(irq);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){2u,0u}, &result) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
            if (!failed) failed |= core_machine_memory_read(state.machine,
                after.ss.base + (lib_u16)after.esp,
                (void *)CORE_MACHINE_REFERENCE_OF(frame_ip), sizeof(frame_ip)) != LIB_STATUS_OK ||
                after.eip != 0x101u || frame_ip != 4u ||
                after.eax != 0xaabb3344u ||
                !les_lds_irq_gprs_same_except_eax(&before, &after) ||
                after.eflags != 0u || !CORE_MACHINE_BIT_IS_SET(
                test_pic_read(state.board->shared_pic_master, 0x0bu), VPIC_ISR_IRQ(0u)) ||
                CORE_MACHINE_BIT_IS_SET(test_pic_read(state.board->shared_pic_master, 0x0au),
                VPIC_IRR_IRQ(0u));
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!les_lds_test_protected()) {
        lib_c_printf("%s\n", "LES-LDS-PROFILE stage=protected"); return 1;
    }
    if (!les_lds_test_limit()) {
        lib_c_printf("%s\n", "LES-LDS-PROFILE stage=limit"); return 1;
    }
    if (!les_lds_test_irq()) {
        lib_c_printf("%s\n", "LES-LDS-PROFILE stage=irq"); return 1;
    }
    lib_c_printf("LES_LDS_BOARD-BOARD:OK\n");
    return 0;
}
