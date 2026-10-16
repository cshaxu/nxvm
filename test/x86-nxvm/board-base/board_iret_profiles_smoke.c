#include "lib/types/test.h"
#include "lib/types/file.h"
#include "pic_fixture.h"
#include "x86-nxvm/board-base/machine_board_state.h"
#include "lib/types/types_interface.h"

#define main cli_sti_main
#include "machine_cli_sti_interrupt_smoke.c"
#undef main

static lib_i32 iret_sregs_same(const x86_core_cpu_debug_cpu_snapshot *before, const x86_core_cpu_debug_cpu_snapshot *after)
{
    return lib_memory_compare(&before->cs, &after->cs,
            sizeof(before->cs)) == 0 &&
        lib_memory_compare(&before->es, &after->es,
            sizeof(before->es)) == 0 &&
        lib_memory_compare(&before->ss, &after->ss,
            sizeof(before->ss)) == 0 &&
        lib_memory_compare(&before->ds, &after->ds,
            sizeof(before->ds)) == 0 &&
        lib_memory_compare(&before->fs, &after->fs,
            sizeof(before->fs)) == 0 &&
        lib_memory_compare(&before->gs, &after->gs,
            sizeof(before->gs)) == 0;
}

static void iret_seed(cli_sti_machine *state, lib_u32 flags)
{
    const x86_core_cpu_debug_register_patch seed = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EAX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ECX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EDX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EBX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EBP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESI) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EDI) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS),
        .values = {
            [X86_CORE_CPU_DEBUG_EAX] = 0xaabbccddu,
            [X86_CORE_CPU_DEBUG_ECX] = 0x11223344u,
            [X86_CORE_CPU_DEBUG_EDX] = 0x55667788u,
            [X86_CORE_CPU_DEBUG_EBX] = 0x99aabbccu,
            [X86_CORE_CPU_DEBUG_ESP] = 0x18000u,
            [X86_CORE_CPU_DEBUG_EBP] = 0x120u,
            [X86_CORE_CPU_DEBUG_ESI] = 0x10u,
            [X86_CORE_CPU_DEBUG_EDI] = 0x20u,
            [X86_CORE_CPU_DEBUG_EFLAGS] = flags
        }
    };
    if (x86_core_machine_debug_patch_registers(state->machine, &seed) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
}

static lib_u32 iret_real_flags_load(
    x86_core_cpu_profile profile, lib_u32 flags)
{
    const lib_u16 known_mask = profile < X86_CORE_CPU_PROFILE_80286 ?
        0x0fd5u : 0x7fd5u;

    return (flags & known_mask) | 0x02u;
}

static lib_i32 iret_real_case(x86_core_cpu_profile profile,
    const lib_u8 *prefix, lib_u8 prefix_bytes, lib_u8 wrap_stack)
{
    static const lib_u8 hlt = 0xf4u;
    const lib_u32 flags = X86_CORE_CPU_DEBUG_EFLAGS_CF | X86_CORE_CPU_DEBUG_EFLAGS_PF |
        X86_CORE_CPU_DEBUG_EFLAGS_IF | X86_CORE_CPU_DEBUG_EFLAGS_DF | 0x3000u | X86_CORE_CPU_DEBUG_EFLAGS_NT |
        0x8002u;
    const lib_u32 expected_flags = iret_real_flags_load(profile, flags);
    const lib_i32 wide = prefix_bytes != 0u && prefix[0] == 0x66u;
    const lib_u16 return_ip = wrap_stack ? 0x0200u : 0x0100u;
    const lib_u32 code_offset = wrap_stack ? 0x0100u : 0u;
    cli_sti_machine state;
    x86_core_machine_run_result result;
    x86_core_cpu_diagnostic diagnostic;
    x86_core_cpu_debug_cpu_snapshot before;
    x86_core_cpu_debug_cpu_snapshot after;
    lib_u8 code[3] = { 0u, 0u, 0u };
    lib_u16 frame16[] = { return_ip, 0x0000u, (lib_u16)flags };
    lib_u32 frame32[] = { return_ip, 0x00000000u, flags };
    lib_u8 unselected_before[12] = {
        0xd1u, 0xd2u, 0xd3u, 0xd4u, 0xd5u, 0xd6u,
        0xd7u, 0xd8u, 0xd9u, 0xdau, 0xdbu, 0xdcu
    };
    lib_u8 unselected_after[12] = { 0u };
    lib_i32 failed = !cli_sti_prepare(profile, &state, LIB_FALSE);

    if (!failed) {
        lib_memory_copy(code, prefix, prefix_bytes);
        code[prefix_bytes] = 0xcfu;
        failed = !cli_sti_real_entry(
            state.machine, code_offset);
        failed |= x86_core_machine_memory_write(state.machine, code_offset, code,
            prefix_bytes + 1u) != LIB_STATUS_OK;
        failed |= x86_core_machine_memory_write(state.machine, return_ip, &hlt,
            sizeof(hlt)) != LIB_STATUS_OK;
        if (wrap_stack) {
            const lib_u32 stack_base = cli_sti_capture(state.machine).ss.base;

            failed |= wide || x86_core_machine_memory_write(state.machine,
                stack_base + 0xfffeu, frame16, sizeof(frame16[0u])) != LIB_STATUS_OK;
            failed |= x86_core_machine_memory_write(state.machine, stack_base,
                frame16 + 1u, sizeof(frame16[0u])) != LIB_STATUS_OK;
            failed |= x86_core_machine_memory_write(state.machine, stack_base + 2u,
                frame16 + 2u, sizeof(frame16[0u])) != LIB_STATUS_OK;
        } else {
            failed |= x86_core_machine_memory_write(state.machine, 0x8000u,
                wide ? (const void *)frame32 : (const void *)frame16,
                wide ? sizeof(frame32) : sizeof(frame16)) != LIB_STATUS_OK;
        }
        failed |= x86_core_machine_memory_write(state.machine, 0x18000u,
            unselected_before, sizeof(unselected_before)) != LIB_STATUS_OK;
    }
    if (!failed) {
        iret_seed(&state, flags);
        if (wrap_stack) test_x86_nxvm_machine_fixture_write_word(state.machine, X86_CORE_CPU_DEBUG_ESP, 0xfffeu);
        before = cli_sti_capture(state.machine);
        failed |= x86_core_machine_run(state.machine,
            (x86_core_machine_run_budget){ 2u, 0u }, &result) != LIB_STATUS_OK;
        after = cli_sti_capture(state.machine);
        failed |= x86_core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            LIB_STATUS_OK;
        if (!failed) failed |= result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        if (!failed) failed |= diagnostic.first_fault.valid;
        failed |= after.eip != return_ip + 1u;
        x86_core_cpu_state cpu_state;
        failed |= x86_core_machine_get_cpu_state(state.machine, &cpu_state) != LIB_STATUS_OK ||
            !cpu_state.halted;
        failed |= wrap_stack ? (lib_u16)after.esp != 0x0004u :
            after.esp != before.esp + (wide ? 12u : 6u);
        failed |= after.cs.selector != 0u || after.cs.base != 0u;
        failed |= after.cs.limit != before.cs.limit;
        failed |= (after.eflags & iret_real_flags_load(profile,
            0xffffu)) != expected_flags;
        failed |= after.eax != before.eax;
        failed |= after.ecx != before.ecx;
        failed |= after.edx != before.edx;
        failed |= after.ebx != before.ebx;
        failed |= after.ebp != before.ebp;
        failed |= after.esi != before.esi;
        failed |= after.edi != before.edi;
        failed |= !iret_sregs_same(&before, &after);
        failed |= x86_core_machine_memory_read(state.machine, 0x18000u,
            (void *)unselected_after,
            sizeof(unselected_after)) != LIB_STATUS_OK;
        failed |= lib_memory_compare(unselected_before, unselected_after,
            sizeof(unselected_before)) != 0;
    }
    x86_core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 iret_test_real(void)
{
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_8086,
        X86_CORE_CPU_PROFILE_8088,
        X86_CORE_CPU_PROFILE_80186,
        X86_CORE_CPU_PROFILE_80286,
        X86_CORE_CPU_PROFILE_80386
    };
    static const lib_u8 prefixes[][2] = {
        { 0x66u, 0u },
        { 0x67u, 0u },
        { 0x66u, 0x67u }
    };
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        if (!iret_real_case(profiles[profile], (const lib_u8[]){ 0u },
                0u, LIB_FALSE))
            return 0;
    }
    for (profile = 0u; profile != 4u; ++profile) {
        if (!iret_real_case(profiles[profile], (const lib_u8[]){ 0u },
                0u, LIB_TRUE))
            return 0;
    }
    for (profile = 0u; profile != sizeof(prefixes) / sizeof(prefixes[0]);
        ++profile) {
        lib_u8 bytes = profile == 2u ? 2u : 1u;

        if (!iret_real_case(X86_CORE_CPU_PROFILE_80386,
                prefixes[profile], bytes, LIB_FALSE))
            return 0;
    }
    return 1;
}

static lib_i32 iret_test_pic(void)
{
    static const lib_u8 code[] = { 0xcfu, 0x90u };
    static const lib_u8 hlt = 0xf4u;
    const lib_u16 offset = 0x0100u;
    const lib_u16 segment = 0u;
    const lib_u32 vector = 0x20u;
    lib_u8 restore_if;

    for (restore_if = 0u; restore_if != 2u; ++restore_if) {
        cli_sti_machine state;
        x86_nxvm_board_pic_irq_source *source = LIB_NULL;
        x86_core_machine_run_result result;
        x86_core_cpu_debug_cpu_snapshot after;
        lib_u16 frame[] = { 0x0001u, 0x0000u,
            (lib_u16)(restore_if ? X86_CORE_CPU_DEBUG_EFLAGS_IF : 0u) };
        lib_u16 frame_ip = 0u;
        lib_i32 failed = !cli_sti_prepare(X86_CORE_CPU_PROFILE_80386,
            &state, LIB_FALSE);

        if (!failed) {
            failed = !cli_sti_real_entry(
                state.machine, 0u);
            failed |= x86_core_machine_memory_write(state.machine, 0u, code,
                sizeof(code)) != LIB_STATUS_OK;
            failed |= x86_core_machine_memory_write(state.machine, 0x8000u, frame,
                sizeof(frame)) != LIB_STATUS_OK;
            failed |= x86_core_machine_memory_write(state.machine, vector * 4u,
                &offset, sizeof(offset)) != LIB_STATUS_OK;
            failed |= x86_core_machine_memory_write(state.machine, vector * 4u + 2u,
                &segment, sizeof(segment)) != LIB_STATUS_OK;
            failed |= x86_core_machine_memory_write(state.machine, offset, &hlt,
                sizeof(hlt)) != LIB_STATUS_OK;
        }
        if (!failed) {
            iret_seed(&state, X86_CORE_CPU_DEBUG_EFLAGS_CF);
            lib_memory_set(&source, 0, sizeof(source));
            test_pic_program_vector(state.board->shared_pic_master, (lib_u8)vector);
            test_pic_bind_source(&source,
                state.board->shared_pic_master,
                state.board->shared_pic_slave, 0u);
            x86_nxvm_board_pic_irq_source_assert(source);
            x86_nxvm_board_pic_irq_source_deassert(source);
            failed |= x86_core_machine_run(state.machine,
                (x86_core_machine_run_budget){ 2u, 0u }, &result) != LIB_STATUS_OK;
            after = cli_sti_capture(state.machine);
            if (restore_if) {
                if (!failed) failed |= result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
                failed |= after.eip != offset + 1u;
                x86_core_cpu_state cpu_state;
                failed |= x86_core_machine_get_cpu_state(state.machine, &cpu_state) != LIB_STATUS_OK ||
                    !cpu_state.halted;
                failed |= !(((test_pic_read(state.board->shared_pic_master, 0x0bu)) & (VPIC_ISR_IRQ(0u))) != 0u);
                failed |= (((test_pic_read(state.board->shared_pic_master, 0x0au)) & (VPIC_IRR_IRQ(0u))) != 0u);
                failed |= x86_core_machine_memory_read(state.machine,
                    after.ss.base + (lib_u16)after.esp,
                    (void *)lib_pointer_to_uptr(&(frame_ip)), sizeof(frame_ip)) != LIB_STATUS_OK;
                failed |= frame_ip != 1u;
            } else {
                if (!failed) failed |= result.reason != X86_CORE_MACHINE_STOP_BUDGET;
                failed |= after.eip != 2u;
                failed |= (((test_pic_read(state.board->shared_pic_master, 0x0bu)) & (VPIC_ISR_IRQ(0u))) != 0u);
                failed |= !(((test_pic_read(state.board->shared_pic_master, 0x0au)) & (VPIC_IRR_IRQ(0u))) != 0u);
            }
        }
        if (failed) {
            x86_core_machine_destroy(state.machine);
            return 0;
        }
        x86_core_machine_destroy(state.machine);
    }
    return 1;
}

lib_i32 main(void)
{
    if (!iret_test_real() ||
        !iret_test_pic())
        return 1;
    lib_c_printf("IRET:OK\n");
    lib_c_printf("IRET-PROFILES:OK\n");
    return 0;
}
