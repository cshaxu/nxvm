#include "lib/types/test.h"
#include "lib/types/file.h"
#include "pic_fixture.h"
#include "core/board-base/machine_board_state.h"
#include "lib/types/types_interface.h"
#include "core/x86/device_support_interface.h"
#include "core/chips/cpu/cpu_interface.h"
#include "core/board-base/pic_bus_interface.h"
#include "core/board-base/machine_board_interface.h"
#include "composition/debug_fixture.h"
#include "composition/exception_fixture.h"

typedef struct cli_sti_machine {
    core_machine *machine;
    core_machine_board_state *board;
} cli_sti_machine;

static core_machine_debug_cpu_snapshot cli_sti_capture(core_machine *machine)
{
    core_machine_debug_cpu_snapshot snapshot = {0};
    if (core_machine_debug_capture_cpu_snapshot(machine, CORE_MACHINE_CPU_SNAPSHOT_CURRENT,
            &snapshot) != LIB_STATUS_OK) lib_test_assert(LIB_FALSE);
    return snapshot;
}

static lib_i32 cli_sti_real_entry(core_machine *machine, lib_u32 eip)
{
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP),
        .values = {[CORE_MACHINE_DEBUG_EIP] = eip}
    };
    return core_machine_debug_patch_registers(machine, &entry) == LIB_STATUS_OK;
}

static lib_i32 cli_sti_prepare(core_machine_cpu_profile profile,
    cli_sti_machine *state, lib_bool reject_ud)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = X86_FPU_PROFILE_NONE,
        .shared_pit_personality = X86_PIT_PERSONALITY_8253,
        .clock_plan = { .pit = {1u, 4u, 0u} }
    };
    lib_memory_set(state, 0, sizeof(*state));
    return core_machine_create(&config, &state->machine, &state->board) == LIB_STATUS_OK &&
        (!reject_ud || test_core_exception_block_vector(state->machine, 6u,
            state) == LIB_STATUS_OK) &&
        core_machine_freeze_execution_providers(state->machine) == LIB_STATUS_OK &&
        core_machine_reset(state->machine) == LIB_STATUS_OK &&
        cli_sti_real_entry(state->machine, 0u);
}

static lib_i32 cli_sti_run(cli_sti_machine *state, const lib_u8 *code,
    lib_u32 count, lib_u32 budget, core_machine_debug_cpu_snapshot *after)
{
    core_machine_run_result result;

    if (!cli_sti_real_entry(state->machine,
            0u) || core_machine_memory_write(state->machine, 0u, code,
            count) != LIB_STATUS_OK || core_machine_run(state->machine,
            (core_machine_run_budget){ budget, 0u }, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET)
        return 0;
    *after = cli_sti_capture(state->machine);
    return 1;
}

static lib_i32 cli_sti_test_real_forms(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8088, CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 opcodes[] = { 0xfau, 0xfbu };
    const lib_u32 preserved = CORE_MACHINE_DEBUG_EFLAGS_CF | CORE_MACHINE_DEBUG_EFLAGS_PF |
        CORE_MACHINE_DEBUG_EFLAGS_AF | CORE_MACHINE_DEBUG_EFLAGS_ZF | CORE_MACHINE_DEBUG_EFLAGS_SF |
        CORE_MACHINE_DEBUG_EFLAGS_DF | CORE_MACHINE_DEBUG_EFLAGS_OF;
    lib_u8 profile;
    lib_u8 opcode;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
            cli_sti_machine state;
            core_machine_debug_cpu_snapshot after = {0};
            lib_u32 initial = preserved | (opcodes[opcode] == 0xfau ?
                CORE_MACHINE_DEBUG_EFLAGS_IF : 0u);
            lib_u32 expected = opcodes[opcode] == 0xfau ? preserved :
                preserved | CORE_MACHINE_DEBUG_EFLAGS_IF;
            lib_i32 failed = !cli_sti_prepare(profiles[profile], &state, LIB_FALSE);

            if (!failed) {
                test_core_machine_fixture_write_register(state.machine, CORE_MACHINE_DEBUG_EFLAGS, initial);
                failed |= !cli_sti_run(&state, &opcodes[opcode], 1u, 1u, &after) ||
                    after.eip != 1u || after.eflags != expected;
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    return 1;
}

static lib_i32 cli_sti_test_irq_shadow(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8088, CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 sti_nop[] = { 0xfbu, 0x90u };
    static const lib_u8 cli_nop[] = { 0xfau, 0x90u };
    static const lib_u8 hlt = 0xf4u;
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        cli_sti_machine state;
        core_machine_pic_irq_source *source = LIB_NULL;
        core_machine_run_result result;
        core_machine_debug_cpu_snapshot after;
        lib_u32 vector = 0x20u;
        lib_u16 offset = 0x0100u;
        lib_u16 segment = 0u;
        lib_u16 frame_ip = 0u;
        lib_status frame_status;
        lib_i32 failed = !cli_sti_prepare(profiles[profile], &state, LIB_FALSE);

        if (!failed) {
            failed |= !cli_sti_real_entry(
                    state.machine, 0u) ||
                core_machine_memory_write(state.machine, vector * 4u, &offset,
                    sizeof(offset)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, vector * 4u + 2u,
                    &segment, sizeof(segment)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x0100u, &hlt,
                    sizeof(hlt)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0u, sti_nop,
                    sizeof(sti_nop)) != LIB_STATUS_OK;
        }
        if (!failed) {
            lib_memory_set(&source, 0, sizeof(source));
            test_pic_program_vector(state.board->shared_pic_master, (lib_u8)vector);
            test_pic_bind_source(&source,
                state.board->shared_pic_master, state.board->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(source);
            core_machine_pic_irq_source_deassert(source);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){ 2u, 0u }, &result) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET;
            after = cli_sti_capture(state.machine);
            frame_status = core_machine_memory_read(state.machine,
                after.ss.base + (lib_u16)after.esp,
                (void *)&frame_ip, sizeof(frame_ip));
            failed |=
                after.eip != offset ||
                !CORE_MACHINE_BIT_IS_SET(test_pic_read(state.board->shared_pic_master, 0x0bu), VPIC_ISR_IRQ(0u)) ||
                CORE_MACHINE_BIT_IS_SET(test_pic_read(state.board->shared_pic_master, 0x0au), VPIC_IRR_IRQ(0u)) ||
                frame_status != LIB_STATUS_OK || frame_ip != 2u;
        }
        if (failed) {
            core_machine_destroy(state.machine);
            return 0;
        }
        core_machine_destroy(state.machine);
    }
    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        cli_sti_machine state;
        core_machine_pic_irq_source *source = LIB_NULL;
        core_machine_run_result result;
        lib_i32 failed = !cli_sti_prepare(profiles[profile], &state, LIB_FALSE);

        if (!failed) {
            failed |= !cli_sti_real_entry(
                    state.machine, 0u) ||
                core_machine_memory_write(state.machine, 0u, cli_nop,
                    sizeof(cli_nop)) != LIB_STATUS_OK;
        }
        if (!failed) {
            lib_memory_set(&source, 0, sizeof(source));
            test_pic_bind_source(&source,
                state.board->shared_pic_master, state.board->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(source);
            core_machine_pic_irq_source_deassert(source);
            failed |= core_machine_run(state.machine,
                    (core_machine_run_budget){ 2u, 0u }, &result) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET ||
                !CORE_MACHINE_BIT_IS_SET(test_pic_read(state.board->shared_pic_master, 0x0au), VPIC_IRR_IRQ(0u)) ||
                CORE_MACHINE_BIT_IS_SET(test_pic_read(state.board->shared_pic_master, 0x0bu), VPIC_ISR_IRQ(0u));
        }
        core_machine_destroy(state.machine);
        if (failed) {
            return 0;
        }
    }
    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        cli_sti_machine state;
        core_machine_pic_irq_source *source = LIB_NULL;
        core_machine_run_result result;
        core_machine_debug_cpu_snapshot after;
        lib_i32 failed = !cli_sti_prepare(profiles[profile], &state, LIB_FALSE);

        if (!failed) {
            failed |= !cli_sti_real_entry(
                    state.machine, 0u) ||
                core_machine_memory_write(state.machine, 0u, sti_nop,
                    sizeof(sti_nop)) != LIB_STATUS_OK;
        }
        if (!failed) {
            lib_memory_set(&source, 0, sizeof(source));
            test_pic_program_vector(state.board->shared_pic_master, 0u); core_machine_pic_write_register(state.board->shared_pic_master, 1u, 0xffu);
            test_pic_bind_source(&source,
                state.board->shared_pic_master, state.board->shared_pic_slave, 1u);
            core_machine_pic_irq_source_assert(source);
            core_machine_pic_irq_source_deassert(source);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){2u, 0u}, &result) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET;
            after = cli_sti_capture(state.machine);
            failed |= after.eip != 2u ||
                !CORE_MACHINE_BIT_IS_SET(test_pic_read(state.board->shared_pic_master, 0x0au), VPIC_IRR_IRQ(1u)) ||
                CORE_MACHINE_BIT_IS_SET(test_pic_read(state.board->shared_pic_master, 0x0bu), VPIC_ISR_IRQ(1u));
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static lib_i32 cli_sti_test_8088_pic_mask_round_trip(void)
{
    static const lib_u8 code[] = {
        0xb0u, 0x11u, 0xe6u, 0x20u, 0xb0u, 0x08u, 0xe6u, 0x21u,
        0xb0u, 0x04u, 0xe6u, 0x21u, 0xb0u, 0x01u, 0xe6u, 0x21u,
        0xb0u, 0xffu, 0xe6u, 0x21u, 0xe4u, 0x21u, 0xfeu, 0xc0u, 0x75u,
        0x07u, 0xc6u, 0x06u, 0x00u, 0x01u, 0x00u, 0xebu, 0x05u,
        0xc6u, 0x06u, 0x00u, 0x01u, 0x01u, 0xf4u
    };
    cli_sti_machine state;
    core_machine_run_result result;
    lib_u8 marker = 0xffu;
    lib_i32 failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_8088, &state, LIB_FALSE);

    if (!failed) {
        failed |= !cli_sti_real_entry(
            state.machine, 0u);
        failed |= !failed && core_machine_memory_write(state.machine, 0u, code,
            sizeof(code)) != LIB_STATUS_OK;
        failed |= !failed && core_machine_run(state.machine,
            (core_machine_run_budget){32u, 0u}, &result) != LIB_STATUS_OK;
        failed |= !failed && core_machine_memory_read(state.machine, 0x0100u,
            &marker, sizeof(marker)) != LIB_STATUS_OK;
        failed |= !failed && marker != 0u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 cli_sti_test_8088_keyboard_compare(void)
{
    static const lib_u8 code[] = {
        0xb0u, 0xaau, 0x8au, 0xd8u, 0x80u, 0xfbu, 0xaau, 0x75u, 0x07u,
        0xc6u, 0x06u, 0x00u, 0x01u, 0x00u, 0xebu, 0x05u,
        0xc6u, 0x06u, 0x00u, 0x01u, 0x01u, 0xf4u
    };
    cli_sti_machine state;
    core_machine_run_result result;
    lib_u8 marker = 0xffu;
    lib_i32 failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_8088, &state, LIB_FALSE);

    if (!failed) {
        failed |= !cli_sti_real_entry(
            state.machine, 0u);
        failed |= !failed && core_machine_memory_write(state.machine, 0u, code,
            sizeof(code)) != LIB_STATUS_OK;
        failed |= !failed && core_machine_run(state.machine,
            (core_machine_run_budget){16u, 0u}, &result) != LIB_STATUS_OK;
        failed |= !failed && core_machine_memory_read(state.machine, 0x0100u,
            &marker, sizeof(marker)) != LIB_STATUS_OK;
        failed |= !failed && marker != 0u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 cli_sti_test_8088_pit_irq_round_trip(void)
{
    static const lib_u8 code[] = {
        0xfau, 0xb0u, 0x11u, 0xe6u, 0x20u, 0xb0u, 0x10u, 0xe6u, 0x21u,
        0xb0u, 0x04u, 0xe6u, 0x21u, 0xb0u, 0x01u, 0xe6u, 0x21u,
        0xb0u, 0xfeu, 0xe6u, 0x21u, 0xb0u, 0x10u, 0xe6u, 0x43u,
        0xb9u, 0x16u, 0x00u, 0x8au, 0xc1u, 0xe6u, 0x40u,
        0xfbu, 0x2bu, 0xc9u, 0x80u, 0x3eu, 0x00u, 0x02u, 0x00u,
        0x75u, 0x02u, 0xe2u, 0xf7u, 0xf4u
    };
    static const lib_u8 handler[] = {
        0xc6u, 0x06u, 0x00u, 0x02u, 0x01u, 0xb0u, 0x20u, 0xe6u, 0x20u, 0xcfu
    };
    static const lib_u8 vector[] = { 0x00u, 0x01u, 0x00u, 0x00u };
    cli_sti_machine state;
    core_machine_run_result result;
    lib_u8 marker = 0u;
    lib_i32 failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_8088, &state, LIB_FALSE);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0x40u, vector, sizeof(vector)) !=
            LIB_STATUS_OK;
        failed |= !failed && core_machine_memory_write(state.machine, 0x0100u, handler,
            sizeof(handler)) != LIB_STATUS_OK;
        failed |= !failed && core_machine_memory_write(state.machine, 0u, code,
            sizeof(code)) != LIB_STATUS_OK;
        failed |= !failed && core_machine_run(state.machine,
            (core_machine_run_budget){256u, 0u}, &result) != LIB_STATUS_OK;
        failed |= !failed && core_machine_memory_read(state.machine, 0x0200u,
            &marker, sizeof(marker)) != LIB_STATUS_OK;
        failed |= marker != 1u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 cli_sti_test_8088_ram_post_store(void)
{
    static const lib_u8 code[] = {
        0xb8u, 0x00u, 0x04u, 0x8eu, 0xd8u, 0x8eu, 0xc0u,
        0xfcu, 0x2bu, 0xffu, 0xb8u, 0xaau, 0xaau, 0xb9u, 0x10u, 0x00u,
        0xf3u, 0xabu, 0xf4u
    };
    cli_sti_machine state;
    core_machine_run_result result;
    lib_u8 contents[32] = {0};
    lib_size index;
    lib_i32 failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_8088, &state, LIB_FALSE);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0u, code, sizeof(code)) !=
            LIB_STATUS_OK;
        failed |= !failed && core_machine_run(state.machine,
            (core_machine_run_budget){64u, 0u}, &result) != LIB_STATUS_OK;
        failed |= !failed && core_machine_memory_read(state.machine, 0x4000u, contents,
            sizeof(contents)) != LIB_STATUS_OK;
        for (index = 0u; !failed && index < sizeof(contents); ++index)
            failed |= contents[index] != 0xaau;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

/* Real guest mode entry: valid ring-3 selectors and a loaded busy TSS. */
static lib_i32 cli_sti_prepare_mode(cli_sti_machine *state, lib_u8 mode,
    lib_u32 flags)
{
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0x40u,0,
        0xffu,0xffu,0,0,0,0x92u,0xcfu,0,
        0xffu,0xffu,0,0x20u,0,0xfau,0,0,
        0xffu,0xffu,0,0,0,0xf2u,0xcfu,0,
        0x67u,0,0,0x05u,0,0x89u,0,0
    };
    static const lib_u8 pointers[] = {
        0x2fu,0,0,0x03u,0,0, 0x6fu,0,0,0x04u,0,0
    };
    static const lib_u8 setup[] = {
        0x0fu,0x01u,0x16u,0,0x01u, 0x0fu,0x01u,0x1eu,0x06u,0x01u,
        0xb8u,1u,0,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0,0x8eu,0xd8u,0x8eu,0xc0u,0x8eu,0xd0u,
        0xbcu,0,0x80u,0xeau,0,0x04u,8u,0
    };
    lib_u8 transfer[] = {0x66u,0xb8u,0x28u,0,0x0fu,0,0xd8u,0xcfu};
    static const lib_u8 jump[] = {0xeau,0,0,0,0,8u,0};
    const lib_u32 frame[] = {
        0u, mode == 2u ? 0u : 0x1bu, flags,
        0x8000u, mode == 2u ? 0u : 0x23u, 0u,0u,0u,0u
    };
    const lib_u32 esp0 = 0x9000u;
    const lib_u16 ss0 = 0x10u;
    lib_u8 gate[8u] = {0,1u,8u,0,0,0x8eu,0,0};
    core_machine_run_result result;
    core_machine_debug_cpu_snapshot snapshot;
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP),
        .values = {[CORE_MACHINE_DEBUG_EIP] = 0x600u}
    };

    if (!cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80386, state, LIB_FALSE) ||
        core_machine_memory_write(state->machine, 0x300u, gdt, sizeof(gdt)) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, 0x100u, pointers, sizeof(pointers)) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, 0x600u, setup, sizeof(setup)) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, 0x500u + 4u, &esp0, sizeof(esp0)) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, 0x500u + 8u, &ss0, sizeof(ss0)) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, 0x400u + 13u * 8u, gate, sizeof(gate)) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, 0x2100u, (const lib_u8[]){0xf4u}, 1u) != LIB_STATUS_OK ||
        core_machine_debug_patch_registers(state->machine, &entry) != LIB_STATUS_OK ||
        core_machine_run(state->machine, (core_machine_run_budget){10u,0u}, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 10u) return 0;
    if (mode == 0u) transfer[7u] = 0x90u;
    if (core_machine_memory_write(state->machine, 0x2400u, transfer, sizeof(transfer)) != LIB_STATUS_OK ||
        (mode == 0u && core_machine_memory_write(state->machine, 0x2407u, jump, sizeof(jump)) != LIB_STATUS_OK) ||
        (mode != 0u && core_machine_memory_write(state->machine, 0x8000u, frame, sizeof(frame)) != LIB_STATUS_OK) ||
        core_machine_run(state->machine, (core_machine_run_budget){3u,0u}, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 3u) return 0;
    if (mode == 0u) test_core_machine_fixture_write_register(state->machine, CORE_MACHINE_DEBUG_EFLAGS, flags);
    snapshot = cli_sti_capture(state->machine);
    return snapshot.eip == 0u && snapshot.cs.base == (mode == 2u ? 0u : 0x2000u) &&
        snapshot.cs.selector == (mode == 0u ? 8u : mode == 1u ? 0x1bu : 0u) &&
        snapshot.cs.dpl == (mode == 0u ? 0u : 3u) &&
        snapshot.ss.selector == (mode == 0u ? 0x10u : mode == 1u ? 0x23u : 0u) &&
        snapshot.tr.selector == 0x28u && snapshot.tr.type == 0x0bu &&
        snapshot.tr.base == 0x500u && snapshot.tr.limit == 0x67u &&
        ((snapshot.eflags & CORE_MACHINE_DEBUG_EFLAGS_VM) != 0u) == (mode == 2u);
}

static lib_i32 cli_sti_run_vm86(cli_sti_machine *state, const lib_u8 *code,
    lib_u8 bytes, lib_u32 eflags, lib_i32 fault,
    core_machine_debug_cpu_snapshot *after, core_machine_cpu_diagnostic *diagnostic)
{
    core_machine_run_result result;
    lib_status status;

    if (!cli_sti_prepare_mode(state, 2u, eflags) ||
        core_machine_memory_write(state->machine, 0u, code, bytes) != LIB_STATUS_OK)
        return 0;
    status = core_machine_run(state->machine, (core_machine_run_budget){1u,0u}, &result);
    *after = cli_sti_capture(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) == LIB_STATUS_OK &&
        status == (fault ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK) &&
        result.reason == (fault ? CORE_MACHINE_STOP_FAULT : CORE_MACHINE_STOP_BUDGET);
}

static lib_i32 cli_sti_test_protected_success(void)
{
    static const lib_u8 opcodes[] = { 0xfau, 0xfbu };
    lib_u8 pass;

    for (pass = 0u; pass != 2u; ++pass) {
        lib_u8 opcode;

        for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
            cli_sti_machine state;
            core_machine_run_result result;
            core_machine_debug_cpu_snapshot after;
            lib_u32 iopl = pass ? 0x3000u : 0u;
            lib_u32 initial = CORE_MACHINE_DEBUG_EFLAGS_CF | iopl |
                (opcodes[opcode] == 0xfau ? CORE_MACHINE_DEBUG_EFLAGS_IF : 0u);
            lib_u32 expected = CORE_MACHINE_DEBUG_EFLAGS_CF | iopl |
                (opcodes[opcode] == 0xfbu ? CORE_MACHINE_DEBUG_EFLAGS_IF : 0u);
            lib_i32 failed = !cli_sti_prepare_mode(&state, pass ? 1u : 0u, initial);

            if (!failed) {
                failed |= core_machine_memory_write(state.machine, 0x2000u,
                        &opcodes[opcode], 1u) != LIB_STATUS_OK;
                failed |= core_machine_run(state.machine,
                        (core_machine_run_budget){ 1u, 0u }, &result) != LIB_STATUS_OK ||
                    result.reason != CORE_MACHINE_STOP_BUDGET;
                after = cli_sti_capture(state.machine);
                failed |= after.eip != 1u ||
                    (after.eflags & (CORE_MACHINE_DEBUG_EFLAGS_CF | CORE_MACHINE_DEBUG_EFLAGS_IF |
                        0x3000u)) != expected;
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    return 1;
}

static lib_i32 cli_sti_test_protected_reject(void)
{
    static const lib_u8 opcodes[] = { 0xfau, 0xfbu };
    lib_u8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        cli_sti_machine state;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_debug_cpu_snapshot after;
        lib_status status;
        lib_u32 frame[4] = { 0u, 0u, 0u, 0u };
        const lib_u32 flags = CORE_MACHINE_DEBUG_EFLAGS_CF | 2u;
        lib_i32 failed = !cli_sti_prepare_mode(&state, 1u, flags);
        if (!failed) {
            failed |= core_machine_memory_write(state.machine, 0x2000u,
                    &opcodes[opcode], 1u) != LIB_STATUS_OK;
            status = core_machine_run(state.machine,
                (core_machine_run_budget){ 1u, 0u }, &result);
            failed |= status != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                LIB_STATUS_OK;
            after = cli_sti_capture(state.machine);
            failed |= diagnostic.first_fault.valid ||
                !diagnostic.last_delivered_exception.valid ||
                diagnostic.delivered_exception_count != 1u || !CORE_MACHINE_BIT_IS_SET(
                    diagnostic.last_delivered_exception.exception_mask,
                    VCPUINS_EXCEPT_GP) ||
                diagnostic.last_delivered_exception.exception_code != 0u ||
                after.cs.selector != 0x0008u || after.eip != 0x00000100u ||
                core_machine_memory_read(state.machine,
                    after.ss.base + after.esp, (void *)frame,
                    sizeof(frame)) != LIB_STATUS_OK || frame[0] != 0u ||
                frame[1] != 0u || frame[2] != 0x001bu ||
                frame[3] != (flags | 0x00010000u);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 cli_sti_test_vm86(void)
{
    static const lib_u8 opcodes[] = { 0xfau, 0xfbu };
    lib_u8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        lib_u8 pass;

        for (pass = 0u; pass != 2u; ++pass) {
            cli_sti_machine state;
            core_machine_cpu_diagnostic diagnostic;
            core_machine_debug_cpu_snapshot after = {0};
            const lib_u32 flags = CORE_MACHINE_DEBUG_EFLAGS_VM | CORE_MACHINE_DEBUG_EFLAGS_CF | 2u |
                (pass ? 0u : 0x3000u);
            const lib_u32 expected = pass ? flags :
                (opcodes[opcode] == 0xfau ? flags & ~CORE_MACHINE_DEBUG_EFLAGS_IF :
                    flags | CORE_MACHINE_DEBUG_EFLAGS_IF);
            lib_i32 failed = 0;

            if (!failed) {
                failed |= !cli_sti_run_vm86(&state, &opcodes[opcode], 1u,
                    flags, 0, &after, &diagnostic);
                if (pass) {
                    failed |= diagnostic.first_fault.valid ||
                        !diagnostic.last_delivered_exception.valid ||
                        !CORE_MACHINE_BIT_IS_SET(diagnostic.last_delivered_exception.exception_mask,
                            VCPUINS_EXCEPT_GP) ||
                        diagnostic.last_delivered_exception.exception_code != 0u ||
                        after.cs.selector != 0x0008u ||
                        after.ss.selector != 0x0010u || after.eip != 0x00000100u ||
                        CORE_MACHINE_BIT_IS_SET(after.eflags, CORE_MACHINE_DEBUG_EFLAGS_VM) ||
                        CORE_MACHINE_BIT_IS_SET(after.eflags, CORE_MACHINE_DEBUG_EFLAGS_IF);
                } else {
                    failed |= diagnostic.first_fault.valid || after.eip != 1u ||
                        after.eflags != expected;
                }
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    return 1;
}

lib_i32 main(void)
{
    if (!cli_sti_test_real_forms()) {
        lib_c_printf("CLI-STI stage=real failed\n");
        return 1;
    }
    if (!cli_sti_test_irq_shadow()) {
        lib_c_printf("CLI-STI stage=pic failed\n");
        return 1;
    }
    if (!cli_sti_test_8088_pic_mask_round_trip()) {
        lib_c_printf("CLI-STI stage=8088-pic-mask failed\n");
        return 1;
    }
    if (!cli_sti_test_8088_keyboard_compare()) {
        lib_c_printf("CLI-STI stage=8088-keyboard-compare failed\n");
        return 1;
    }
    if (!cli_sti_test_8088_pit_irq_round_trip()) {
        lib_c_printf("CLI-STI stage=8088-pit-irq failed\n");
        return 1;
    }
    if (!cli_sti_test_8088_ram_post_store()) {
        lib_c_printf("CLI-STI stage=8088-ram-post-store failed\n");
        return 1;
    }
    if (!cli_sti_test_protected_success())
        return 1;
    if (!cli_sti_test_protected_reject())
        return 1;
    if (!cli_sti_test_vm86())
        return 1;
    lib_c_printf("CLI-STI:OK\n");
    lib_c_printf("CLI-STI-INTERRUPT:OK\n");
    return 0;
}
