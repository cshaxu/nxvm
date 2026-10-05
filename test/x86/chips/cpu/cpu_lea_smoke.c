#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

/* T337_REAL_UD_TERMINAL_CPU_OWNER: CPU-local IDTR excludes vector 6,
 * preserving the original no-handler terminal-fault cases. */
static void lea_run_prepared(cpu_instruction_fixture *state, const lib_u8 *code,
    lib_u8 bytes, t_cpu *after)
{
    lib_memory_copy(state->memory, code, bytes);
    core_machine_cpu_execution_refresh(&state->execution);
    *after = state->cpu;
}

static void lea_set_registers(cpu_instruction_fixture *state)
{
    state->cpu.data.eax = 0xaabb0000u;
    state->cpu.data.ebx = 0x11112000u;
    state->cpu.data.esi = 0x12345000u;
    state->cpu.data.ebp = 0x56780000u;
    state->cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
}

static lib_i32 lea_test_real_forms(void)
{
    static const lib_u8 code16[] = { 0x8du, 0x40u, 0x10u };
    static const lib_u8 code66[] = { 0x66u, 0x8du, 0x40u, 0x10u };
    static const lib_u8 code67[] = { 0x67u, 0x8du, 0x46u, 0x10u };
    static const lib_u8 code6766[] = {
        0x67u, 0x66u, 0x8du, 0x46u, 0x10u
    };
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    const lib_u8 *codes[] = { code16, code66, code67, code6766 };
    const lib_u8 code_bytes[] = { 3u, 4u, 4u, 5u };
    const lib_u32 expected_eax[] = {
        0xaabb7010u, 0x00007010u, 0xaabb5010u, 0x12345010u
    };
    lib_u8 profile;
    lib_u8 form;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
            ++profile) {
        for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form) {
            cpu_instruction_fixture state;
            t_cpu before = {0};
            t_cpu after = {0};
            lib_i32 failed = 0;

            cpu_instruction_prepare(&state, profiles[profile]);

            lea_set_registers(&state);
            if (profile != 3u && form != 0u)
                state.cpu.data.idtr.limit = 0x17u;
            before = state.cpu;
            lea_run_prepared(&state, codes[form], code_bytes[form], &after);
            if (profile == 3u || form == 0u) {
                failed |= state.execution.stop_requested ||
                    state.fault.valid ||
                    after.data.eip != code_bytes[form] ||
                    after.data.eax != expected_eax[form] ||
                    after.data.ebx != before.data.ebx ||
                    after.data.esi != before.data.esi ||
                    after.data.eflags != before.data.eflags;
            }
            else {
                failed |= !state.execution.stop_requested ||
                    !state.fault.valid ||
                    !(state.fault.exception_mask & VCPUINS_EXCEPT_UD) ||
                    after.data.eip != before.data.eip ||
                    after.data.eax != before.data.eax ||
                    after.data.eflags != before.data.eflags;
            }
            if (failed)
                return 0;
        }
    }
    return 1;
}

static lib_i32 lea_test_register_direct(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 code[] = { 0x8du, 0xc0u };
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
            ++profile) {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;
        lib_i32 failed = 0;

        cpu_instruction_prepare(&state, profiles[profile]);
        lea_set_registers(&state);
        state.cpu.data.idtr.limit = 0x17u;
        before = state.cpu;
        lea_run_prepared(&state, code, sizeof(code), &after);
        failed |= !state.execution.stop_requested ||
            !state.fault.valid || !(state.fault.exception_mask & VCPUINS_EXCEPT_UD) ||
            after.data.eip != before.data.eip ||
            after.data.eax != before.data.eax ||
            after.data.eflags != before.data.eflags;
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 lea_test_lock_ud(void)
{
    static const lib_u8 code[] = { 0xf0u, 0x8du, 0x40u, 0x10u };
    cpu_instruction_fixture state;
    t_cpu before;
    t_cpu after;
    lib_i32 failed = 0;

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    lea_set_registers(&state);
    state.cpu.data.idtr.limit = 0x17u;
    before = state.cpu;
    lea_run_prepared(&state, code, sizeof(code), &after);
    failed |= !state.execution.stop_requested ||
        !state.fault.valid || !(state.fault.exception_mask & VCPUINS_EXCEPT_UD) ||
        after.data.eip != before.data.eip || after.data.eax != before.data.eax ||
        after.data.eflags != before.data.eflags;
    return !failed;
}

static lib_i32 lea_prepare_protected(cpu_instruction_fixture *state)
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
    static const lib_u8 hlt[] = { 0xf4u };
    cpu_instruction_prepare(state, CORE_MACHINE_CPU_PROFILE_80386);
    lib_memory_copy(state->memory + 0x0100u, pointer, sizeof(pointer));
    lib_memory_copy(state->memory + 0x0300u, gdt, sizeof(gdt));
    lib_memory_copy(state->memory, bootstrap, sizeof(bootstrap));
    lib_memory_copy(state->memory + 0x2000u, hlt, sizeof(hlt));
    for (lib_u32 step = 0u; step < 96u; ++step) {
        core_machine_cpu_execution_refresh(&state->execution);
        if (state->execution.stop_requested) return 0;
        if (state->cpu.data.flagHalt) return 1;
    }
    return 0;
}

static lib_i32 lea_test_protected(void)
{
    static const lib_u8 code16[] = { 0x8du, 0x40u, 0x10u };
    static const lib_u8 code66[] = { 0x66u, 0x8du, 0x40u, 0x10u };
    static const lib_u8 code67[] = { 0x67u, 0x8du, 0x46u, 0x10u };
    static const lib_u8 code6766[] = {
        0x67u, 0x66u, 0x8du, 0x46u, 0x10u
    };
    const lib_u8 *codes[] = { code16, code66, code67, code6766 };
    const lib_u8 code_bytes[] = { 3u, 4u, 4u, 5u };
    const lib_u32 expected_eax[] = {
        0xaabb7010u, 0x00007010u, 0xaabb5010u, 0x12345010u
    };
    lib_u8 form;

    for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form) {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;
        lib_i32 failed = !lea_prepare_protected(&state);

        if (!failed) {
            lea_set_registers(&state);
            lib_memory_copy(state.memory + 0x2000u, codes[form], code_bytes[form]);
            state.cpu.data.eip = 0u;
            state.cpu.data.flagHalt = LIB_FALSE;
            core_machine_cpu_execution_invalidate_prefetch(&state.execution);
            before = state.cpu;
            core_machine_cpu_execution_refresh(&state.execution);
            failed |= state.execution.stop_requested || state.instructions.data.except != 0u;
            after = state.cpu;
            failed |= after.data.eip != code_bytes[form] ||
                after.data.eax != expected_eax[form] ||
                after.data.ebx != before.data.ebx ||
                after.data.esi != before.data.esi ||
                after.data.eflags != before.data.eflags;
        }
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 lea_test_null_ds_no_read(void)
{
    static const lib_u8 code[] = { 0x8du, 0x40u, 0x10u };
    cpu_instruction_fixture state;
    t_cpu before;
    t_cpu after;
    lib_i32 failed = !lea_prepare_protected(&state);

    if (!failed) {
        lea_set_registers(&state);
        state.cpu.data.ds.selector = 0u;
        state.cpu.data.ds.flagValid = LIB_FALSE;
        lib_memory_copy(state.memory + 0x2000u, code, sizeof(code));
        state.cpu.data.eip = 0u;
        state.cpu.data.flagHalt = LIB_FALSE;
        core_machine_cpu_execution_invalidate_prefetch(&state.execution);
        before = state.cpu;
        core_machine_cpu_execution_refresh(&state.execution);
        failed |= state.execution.stop_requested || state.instructions.data.except != 0u;
        after = state.cpu;
        failed |= after.data.eip != sizeof(code) ||
            after.data.eax != 0xaabb7010u ||
            after.data.ebx != before.data.ebx ||
            after.data.esi != before.data.esi ||
            after.data.eflags != before.data.eflags ||
            after.data.ds.selector != 0u || after.data.ds.flagValid;
    }
    return !failed;
}

lib_i32 main(void)
{
    if (!lea_test_real_forms() || !lea_test_register_direct() ||
        !lea_test_lock_ud() || !lea_test_protected() ||
        !lea_test_null_ds_no_read()) return 1;
    lib_c_printf("M5:T539:S21:LEA-CPU:OK\n");
    return 0;
}
