#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "pic_fixture.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include "ibmpc/board-common/machine_board_state.h"
#include "core_machine_board_fixture.h"
#include "x86/core/device_support_interface.h"
typedef struct sreg_mov_machine { core_machine *machine;
    core_machine_board_state *board; } sreg_mov_machine;

static lib_i32 sreg_mov_prepare(sreg_mov_machine *state,
    core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = X86_FPU_PROFILE_NONE
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

static lib_i32 sreg_mov_patch(sreg_mov_machine *state,
    core_machine_debug_register register_id, lib_u32 value)
{
    core_machine_debug_register_patch patch = {0};
    patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(register_id);
    patch.values[register_id] = value;
    return core_machine_debug_patch_registers(state->machine, &patch) == LIB_STATUS_OK;
}

static lib_i32 sreg_mov_capture(sreg_mov_machine *state,
    core_machine_debug_cpu_snapshot *snapshot)
{
    return core_machine_debug_capture_cpu_snapshot(state->machine,
        CORE_MACHINE_CPU_SNAPSHOT_CURRENT, snapshot) == LIB_STATUS_OK;
}

static const core_machine_debug_segment_snapshot *sreg_mov_sreg(
    const core_machine_debug_cpu_snapshot *cpu, lib_u8 index)
{
    switch (index) {
    case 0u: return &cpu->es;
    case 2u: return &cpu->ss;
    case 3u: return &cpu->ds;
    case 4u: return &cpu->fs;
    default: return &cpu->gs;
    }
}

static lib_i32 sreg_mov_boot_protected(sreg_mov_machine *state)
{
    static const lib_u8 pointer[] = {0x3fu, 0, 0, 0x03u, 0, 0};
    static const lib_u8 gdt[] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0xffu, 0xffu, 0, 0x20u, 0, 0x9au, 0, 0,
        0xffu, 0xffu, 0, 0x30u, 0, 0x92u, 0, 0,
        0xffu, 0xffu, 0, 0x30u, 0, 0x12u, 0, 0,
        0xffu, 0xffu, 0, 0x30u, 0, 0x98u, 0, 0,
        0xffu, 0xffu, 0, 0x50u, 0, 0x92u, 0, 0,
        0xffu, 0xffu, 0, 0x50u, 0, 0x92u, 0, 0,
        0x0fu, 0, 0, 0x50u, 0, 0x92u, 0, 0
    };
    static const lib_u8 boot[] = {
        0x0fu, 0x01u, 0x16u, 0, 1u,
        0xb8u, 1u, 0, 0x0fu, 0x01u, 0xf0u,
        0xb8u, 0x10u, 0, 0x8eu, 0xd8u, 0x8eu, 0xc0u,
        0x8eu, 0xd0u, 0xbcu, 0, 0x80u,
        0xeau, 0, 0, 8u, 0
    };
    core_machine_run_result result;

    return core_machine_memory_write(state->machine, 0x100u, pointer,
            sizeof(pointer)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x300u, gdt,
            sizeof(gdt)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine, 0, boot,
            sizeof(boot)) == LIB_STATUS_OK &&
        core_machine_run(state->machine, (core_machine_run_budget){9u, 0u},
            &result) == LIB_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET && result.executed == 9u;
}

static lib_i32 sreg_mov_expect_fault(sreg_mov_machine *state,
    const lib_u8 *code, lib_size size)
{
    core_machine_debug_cpu_snapshot before = {0};
    core_machine_debug_cpu_snapshot after = {0};
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;

    if (!sreg_mov_capture(state, &before) ||
        !sreg_mov_patch(state, CORE_MACHINE_DEBUG_EIP, 0u) ||
        core_machine_memory_write(state->machine, 0x2000u, code, size) !=
            LIB_STATUS_OK ||
        core_machine_run(state->machine, (core_machine_run_budget){1u,0u},
            &result) != LIB_STATUS_INTERNAL_ERROR ||
        result.reason != CORE_MACHINE_STOP_FAULT ||
        core_machine_get_cpu_diagnostic(state->machine, &diagnostic) !=
            LIB_STATUS_OK ||
        !sreg_mov_capture(state, &after)) return 0;

    return diagnostic.first_fault.valid &&
        CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
            VCPUINS_EXCEPT_DF) &&
        after.eip == 0u && before.esp == after.esp &&
        before.eflags == after.eflags &&
        lib_memory_compare(&before.es, &after.es, sizeof(before.es)) == 0 &&
        lib_memory_compare(&before.ds, &after.ds, sizeof(before.ds)) == 0 &&
        lib_memory_compare(&before.ss, &after.ss, sizeof(before.ss)) == 0 &&
        lib_memory_compare(&before.fs, &after.fs, sizeof(before.fs)) == 0 &&
        lib_memory_compare(&before.gs, &after.gs, sizeof(before.gs)) == 0;
}

static lib_i32 sreg_mov_test_protected_faults(void)
{
    static const lib_u16 selectors[] = {0u, 0x18u, 0x20u, 0x2bu};
    static const lib_u8 load_limit[] = {0x8eu, 0x1eu, 0x10u, 0};
    static const lib_u8 store_limit[] = {0x8cu, 0x06u, 0x10u, 0};
    lib_u8 form;

    for (form = 0u; form != 6u; ++form) {
        sreg_mov_machine state;
        lib_u16 image = 0xbe5au;
        core_machine_run_result result;
        lib_i32 passed = sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386) &&
            sreg_mov_boot_protected(&state);
        if (passed && form < 4u) {
            const lib_u8 code[] = {0x8eu, form == 0u ? 0xd0u : 0xd8u};
            passed = sreg_mov_patch(&state, CORE_MACHINE_DEBUG_EAX,
                0xaabb0000u | selectors[form]) &&
                core_machine_memory_write(state.machine, 0x3010u, &image,
                    sizeof(image)) == LIB_STATUS_OK &&
                sreg_mov_expect_fault(&state, code, sizeof(code));
        } else if (passed) {
            const lib_u8 *code = form == 4u ? load_limit : store_limit;
            const core_machine_debug_register_patch registers = {
                .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
                    CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP),
                .values = {[CORE_MACHINE_DEBUG_EAX] = 0xaabb0038u}
            };
            passed = core_machine_debug_patch_registers(state.machine,
                &registers) == LIB_STATUS_OK &&
                core_machine_memory_write(state.machine, 0x2000u,
                    (const lib_u8[]){0x8eu,0xd8u}, 2u) == LIB_STATUS_OK &&
                core_machine_run(state.machine,
                    (core_machine_run_budget){1u,0u}, &result) == LIB_STATUS_OK &&
                result.reason == CORE_MACHINE_STOP_BUDGET &&
                core_machine_memory_write(state.machine, 0x5010u, &image,
                    sizeof(image)) == LIB_STATUS_OK &&
                sreg_mov_expect_fault(&state, code, 4u);
        }
        core_machine_destroy(state.machine);
        if (!passed) {
            lib_c_fprintf(lib_c_stderr, "M5:T539:S28:SREG-MOV protected fault form=%u\n",
                (unsigned)form);
            return 0;
        }
    }
    return 1;
}

static lib_i32 sreg_mov_test_irq_shadow(void)
{
    static const lib_u8 modrms[] = {0xd0u, 0xd8u, 0xe0u};
    static const lib_u8 hlt = 0xf4u;
    lib_u8 form;

    for (form = 0u; form != sizeof(modrms); ++form) {
        sreg_mov_machine state;
        core_machine_pic_irq_source *source = LIB_NULL;
        core_machine_run_result result;
        core_machine_debug_cpu_snapshot after = {0};
        lib_u16 offset = 0x0100u;
        lib_u16 segment = 0u;
        lib_u16 frame_ip = 0u;
        lib_u8 code[] = {0x8eu, modrms[form], 0x90u};
        lib_u16 expected_ip = form == 0u ? 3u : 2u;
        lib_u8 target = form == 0u ? 2u : form == 1u ? 3u : 4u;
        lib_i32 failed = !sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            failed |= core_machine_memory_write(state.machine,
                0u, code, sizeof(code)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x80u, &offset,
                    sizeof(offset)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x82u, &segment,
                    sizeof(segment)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x100u, &hlt,
                    sizeof(hlt)) != LIB_STATUS_OK;
        }
        if (!failed) {
            const core_machine_debug_register_patch registers = {
                .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
                    CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
                    CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDX) |
                    CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBX) |
                    CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
                    CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBP) |
                    CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESI) |
                    CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDI) |
                    CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS) |
                    CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
                    CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
                    CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
                    CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_FS) |
                    CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_GS),
                .values = {
                    [CORE_MACHINE_DEBUG_EAX] = 0xaabb2000u,
                    [CORE_MACHINE_DEBUG_ECX] = 0x11225566u,
                    [CORE_MACHINE_DEBUG_EDX] = 0x778899aau,
                    [CORE_MACHINE_DEBUG_EBX] = 0xbbccddeeu,
                    [CORE_MACHINE_DEBUG_ESP] = 0x8000u,
                    [CORE_MACHINE_DEBUG_EBP] = 0x120u,
                    [CORE_MACHINE_DEBUG_ESI] = 0x10u,
                    [CORE_MACHINE_DEBUG_EDI] = 0x20u,
                    [CORE_MACHINE_DEBUG_EFLAGS] = 0x241u,
                    [CORE_MACHINE_DEBUG_ES] = 0x1111u,
                    [CORE_MACHINE_DEBUG_SS] = 0x2222u,
                    [CORE_MACHINE_DEBUG_DS] = 0x3333u,
                    [CORE_MACHINE_DEBUG_FS] = 0x4444u,
                    [CORE_MACHINE_DEBUG_GS] = 0x5555u
                }
            };
            failed |= core_machine_debug_patch_registers(state.machine,
                &registers) != LIB_STATUS_OK;
        }
        if (!failed) {
            lib_memory_set(&source, 0, sizeof(source));
            test_pic_program_vector(state.board->shared_pic_master, 0x20u);
            test_pic_bind_source(&source,
                state.board->shared_pic_master,
                state.board->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(source);
            core_machine_pic_irq_source_deassert(source);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){form == 0u ? 3u : 2u, 0u},
                &result) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
                !sreg_mov_capture(&state, &after) ||
                core_machine_memory_read(state.machine,
                after.ss.base + (lib_u16)after.esp, &frame_ip,
                sizeof(frame_ip)) != LIB_STATUS_OK ||
                after.eip != 0x101u || frame_ip != expected_ip ||
                !CORE_MACHINE_BIT_IS_SET(
                    test_pic_read(state.board->shared_pic_master, 0x0bu),
                    VPIC_ISR_IRQ(0u)) ||
                CORE_MACHINE_BIT_IS_SET(
                    test_pic_read(state.board->shared_pic_master, 0x0au),
                    VPIC_IRR_IRQ(0u)) ||
                sreg_mov_sreg(&after, target)->selector != 0x2000u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!sreg_mov_test_protected_faults()) return 1;
    if (!sreg_mov_test_irq_shadow()) return 1;
    lib_c_printf("M5:T539:S28:SREG-MOV-BOARD:OK\n");
    return 0;
}
