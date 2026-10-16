#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "pic_fixture.h"
#include "core/board-base/machine_board_interface.h"
#include "core/board-base/machine_board_state.h"
#include "core_machine_board_fixture.h"
#include "core/board-base/pic_bus_interface.h"
typedef struct les_lds_machine { x86_core_machine *machine;
    core_machine_board_state *board; } les_lds_machine;

static lib_i32 les_lds_prepare(x86_core_cpu_profile profile, les_lds_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = profile, .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ES) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP)
    };
    lib_memory_set(state, 0, sizeof(*state));
    return core_machine_create(&config, &state->machine, &state->board) == LIB_STATUS_OK &&
        x86_core_machine_freeze_execution_providers(state->machine) == LIB_STATUS_OK &&
        x86_core_machine_reset(state->machine) == LIB_STATUS_OK &&
        x86_core_machine_debug_patch_registers(state->machine, &entry) == LIB_STATUS_OK;
}

static lib_i32 les_lds_seed(les_lds_machine *state)
{
    return x86_core_machine_debug_patch_registers(state->machine,
                &(x86_core_cpu_debug_register_patch){
                    .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EAX) |
                        X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ECX) |
                        X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EDX) |
                        X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EBX) |
                        X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESP) |
                        X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EBP) |
                        X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESI) |
                        X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EDI) |
                        X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS),
                    .values = { [X86_CORE_CPU_DEBUG_EAX] = 0xaabbccddU,
                        [X86_CORE_CPU_DEBUG_ECX] = 0x11223344u,
                        [X86_CORE_CPU_DEBUG_EDX] = 0x55667788u,
                        [X86_CORE_CPU_DEBUG_EBX] = 0x99aabbccU,
                        [X86_CORE_CPU_DEBUG_ESP] = 0x8000u,
                        [X86_CORE_CPU_DEBUG_EBP] = 0x120u,
                        [X86_CORE_CPU_DEBUG_ESI] = 0x10u,
                        [X86_CORE_CPU_DEBUG_EDI] = 0x20u,
                        [X86_CORE_CPU_DEBUG_EFLAGS] = 0x8d5u }
                }) == LIB_STATUS_OK;
}

static lib_i32 les_lds_gprs_same_except_eax(const x86_core_cpu_debug_cpu_snapshot *before,
    const x86_core_cpu_debug_cpu_snapshot *after)
{
    return after->ecx == before->ecx &&
        after->edx == before->edx &&
        after->ebx == before->ebx &&
        after->esp == before->esp &&
        after->ebp == before->ebp &&
        after->esi == before->esi &&
        after->edi == before->edi;
}

static lib_i32 les_lds_irq_gprs_same_except_eax(const x86_core_cpu_debug_cpu_snapshot *before,
    const x86_core_cpu_debug_cpu_snapshot *after)
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
    return x86_core_machine_memory_inspect(state->machine,
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
    x86_core_machine_run_result result;

    return x86_core_machine_memory_write(state->machine, 0x100u, pointer, sizeof(pointer)) == LIB_STATUS_OK && x86_core_machine_memory_write(state->machine, 0x300u, gdt, sizeof(gdt)) == LIB_STATUS_OK &&
        x86_core_machine_memory_write(state->machine, 0u, boot, sizeof(boot)) ==
        LIB_STATUS_OK && x86_core_machine_run(
        state->machine, (x86_core_machine_run_budget){11u,0u}, &result) ==
        LIB_STATUS_OK && result.reason == X86_CORE_MACHINE_STOP_BUDGET && result.executed == 11u;
}

static lib_i32 les_lds_protected_case(lib_u8 opcode, lib_u16 selector)
{
    les_lds_machine state;
    x86_core_cpu_debug_cpu_snapshot before;
    x86_core_cpu_debug_cpu_snapshot after;
    x86_core_machine_run_result result;
    lib_u8 pointer[] = {0x44u,0x33u,0,0};
    lib_u8 source[4] = {0x44u,0x33u,0,0};
    lib_u8 observed[4] = {0};
    lib_u8 program[4] = {opcode,0x06u,0x10u,0u};
    lib_i32 failed = !les_lds_prepare(X86_CORE_CPU_PROFILE_80386,
        &state);

    pointer[2] = (lib_u8)selector;
    pointer[3] = (lib_u8)(selector >> 8u);
    source[2] = pointer[2];
    source[3] = pointer[3];
    if (!failed)
        failed |= !les_lds_boot_protected(&state);
    if (!failed) {

        failed |= !les_lds_seed(&state);
        failed |= x86_core_machine_debug_patch_registers(state.machine,
                &(x86_core_cpu_debug_register_patch){
                    .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ES),
                    .values = { [X86_CORE_CPU_DEBUG_ES] = opcode == 0xc4u ? 0x18u : 0x10u }
                }) != LIB_STATUS_OK;
        failed |= x86_core_machine_memory_write(state.machine, 0x3010u, pointer,
            sizeof(pointer)) != LIB_STATUS_OK || x86_core_machine_memory_write(
            state.machine, 0x2000u, program, sizeof(program)) != LIB_STATUS_OK;

        failed |= x86_core_machine_debug_capture_cpu_snapshot(state.machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
        failed |= !test_core_machine_fixture_shutdown_wait(x86_core_machine_run(
            state.machine, (x86_core_machine_run_budget){1u,0u}, &result), &result);
        failed |= x86_core_machine_debug_capture_cpu_snapshot(state.machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
        failed |= !les_lds_read(&state, 0x3010u, observed,
            sizeof(observed)) || lib_memory_compare(source, observed,
            sizeof(source)) != 0;
        if (!failed) failed |= after.eip != before.eip ||
            after.eax != before.eax ||
            !les_lds_gprs_same_except_eax(&before, &after) ||
            after.eflags != before.eflags ||
            lib_memory_compare(opcode == 0xc4u ? &before.es : &before.ds,
            opcode == 0xc4u ? &after.es : &after.ds,
            sizeof(x86_core_cpu_debug_segment_snapshot)) != 0 || lib_memory_compare(&before.cs,
            &after.cs, sizeof(before.cs)) != 0 ||
            lib_memory_compare(&before.ss, &after.ss, sizeof(before.ss)) != 0 ||
            lib_memory_compare(opcode == 0xc4u ? &before.ds : &before.es,
            opcode == 0xc4u ? &after.ds : &after.es,
            sizeof(x86_core_cpu_debug_segment_snapshot)) != 0;
    }
    x86_core_machine_destroy(state.machine);
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
        x86_core_cpu_debug_cpu_snapshot before;
        x86_core_cpu_debug_cpu_snapshot after;
        x86_core_machine_run_result result;
        lib_u8 code[] = {opcodes[opcode],0x06u,0x10u,0u};
        lib_i32 failed = !les_lds_prepare(X86_CORE_CPU_PROFILE_80386,
            &state);

        if (!failed)
            failed |= !les_lds_boot_protected(&state);
        if (!failed) {
            failed |= !les_lds_seed(&state);
            const lib_u8 limit[] = {0x11u,0u};
            failed |= x86_core_machine_memory_write(state.machine, 0x310u,
                limit, sizeof(limit)) != LIB_STATUS_OK;
            failed |= x86_core_machine_debug_patch_registers(state.machine,
                &(x86_core_cpu_debug_register_patch){
                    .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS),
                    .values = { [X86_CORE_CPU_DEBUG_DS] = 0x10u }
                }) != LIB_STATUS_OK;
            failed |= x86_core_machine_memory_write(state.machine, 0x2000u, code,
                sizeof(code)) != LIB_STATUS_OK;

            failed |= x86_core_machine_debug_capture_cpu_snapshot(state.machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
            failed |= !test_core_machine_fixture_shutdown_wait(x86_core_machine_run(
                state.machine, (x86_core_machine_run_budget){1u,0u}, &result),
                &result);
            failed |= x86_core_machine_debug_capture_cpu_snapshot(state.machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
            if (!failed) failed |= after.eip != before.eip ||
                after.eax != before.eax ||
                !les_lds_gprs_same_except_eax(&before, &after) ||
                after.eflags != before.eflags ||
                lib_memory_compare(opcodes[opcode] == 0xc4u ? &before.es :
                &before.ds, opcodes[opcode] == 0xc4u ? &after.es :
                &after.ds, sizeof(x86_core_cpu_debug_segment_snapshot)) != 0;
        }
        x86_core_machine_destroy(state.machine);
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
        x86_core_machine_run_result result;
        x86_core_cpu_debug_cpu_snapshot before;
        x86_core_cpu_debug_cpu_snapshot after;
        lib_u16 offset = 0x100u;
        lib_u16 segment = 0u;
        lib_u16 frame_ip = 0u;
        lib_u8 code[] = {opcodes[opcode],0x06u,0,0x10u,0x90u};
        lib_i32 failed = !les_lds_prepare(X86_CORE_CPU_PROFILE_80386,
            &state);

        if (!failed) {
            failed |= x86_core_machine_memory_write(state.machine, 0x1000u, pointer,
                sizeof(pointer)) != LIB_STATUS_OK || x86_core_machine_memory_write(
                state.machine, 0u, code, sizeof(code)) != LIB_STATUS_OK ||
                x86_core_machine_memory_write(state.machine, 0x80u, &offset,
                sizeof(offset)) != LIB_STATUS_OK || x86_core_machine_memory_write(
                state.machine, 0x82u, &segment, sizeof(segment)) != LIB_STATUS_OK ||
                x86_core_machine_memory_write(state.machine, 0x100u, &hlt,
                sizeof(hlt)) != LIB_STATUS_OK;
        }
        if (!failed) {
            failed |= !les_lds_seed(&state);
            failed |= x86_core_machine_debug_patch_registers(state.machine,
                &(x86_core_cpu_debug_register_patch){
                    .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS),
                    .values = { [X86_CORE_CPU_DEBUG_EFLAGS] = 0x200u }
                }) != LIB_STATUS_OK;
            failed |= x86_core_machine_debug_capture_cpu_snapshot(state.machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
            lib_memory_set(&irq, 0, sizeof(irq));
            test_pic_program_vector(state.board->shared_pic_master, 0x20u);
            test_pic_bind_source(&irq, state.board->shared_pic_master,
                state.board->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(irq);
            core_machine_pic_irq_source_deassert(irq);
            failed |= x86_core_machine_run(state.machine,
                (x86_core_machine_run_budget){2u,0u}, &result) != LIB_STATUS_OK ||
                result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            failed |= x86_core_machine_debug_capture_cpu_snapshot(state.machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
            if (!failed) failed |= x86_core_machine_memory_read(state.machine,
                after.ss.base + (lib_u16)after.esp,
                (void *)lib_pointer_to_uptr(&(frame_ip)), sizeof(frame_ip)) != LIB_STATUS_OK ||
                after.eip != 0x101u || frame_ip != 4u ||
                after.eax != 0xaabb3344u ||
                !les_lds_irq_gprs_same_except_eax(&before, &after) ||
                after.eflags != 0u || !(((test_pic_read(state.board->shared_pic_master, 0x0bu)) & (VPIC_ISR_IRQ(0u))) != 0u) ||
                (((test_pic_read(state.board->shared_pic_master, 0x0au)) & (VPIC_IRR_IRQ(0u))) != 0u);
        }
        x86_core_machine_destroy(state.machine);
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
