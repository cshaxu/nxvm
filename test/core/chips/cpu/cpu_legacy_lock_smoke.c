#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "support/cpu_instruction_fixture.h"
/* REAL_UD_TERMINAL_CPU_OWNER: invalid LOCK forms stay CPU-owned. */

static lib_i32 legacy_lock_sregs_same(const t_cpu *before,
    const t_cpu *after)
{
    return lib_memory_compare(&before->data.es, &after->data.es,
        sizeof(before->data.es)) == 0 && lib_memory_compare(&before->data.cs,
        &after->data.cs, sizeof(before->data.cs)) == 0 &&
        lib_memory_compare(&before->data.ss, &after->data.ss,
        sizeof(before->data.ss)) == 0 && lib_memory_compare(&before->data.ds,
        &after->data.ds, sizeof(before->data.ds)) == 0 &&
        lib_memory_compare(&before->data.fs, &after->data.fs,
        sizeof(before->data.fs)) == 0 && lib_memory_compare(&before->data.gs,
        &after->data.gs, sizeof(before->data.gs)) == 0;
}

static lib_i32 legacy_lock_run(cpu_instruction_fixture *state,
    const lib_u8 *code, lib_u8 bytes, lib_u8 steps,
    t_cpu *after)
{
    lib_status status;

    status = cpu_instruction_run(state, code, bytes, after);
    for (lib_u8 step = 1u; step < steps && status == LIB_STATUS_OK; ++step) {
        core_machine_cpu_execution_refresh(&state->execution);
        *after = state->cpu;
        if (state->execution.stop_requested) status = LIB_STATUS_INTERNAL_ERROR;
    }
    return status == LIB_STATUS_OK && !state->fault.valid;
}

static lib_i32 legacy_lock_test_transparent_real(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const lib_u8 cbw[] = { 0xf0u, 0x98u };
    static const lib_u8 add_memory[] = {
        0xf0u, 0x01u, 0x06u, 0x00u, 0x01u
    };
    static const lib_u8 rep_movs[] = { 0xf0u, 0xf3u, 0xa4u };

    for (lib_size profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;
        lib_u16 image = 3u;
        lib_u8 source[] = { 0x31u, 0x42u };
        lib_u8 target[] = { 0u, 0u };
        lib_i32 failed;

        cpu_instruction_prepare(&state, profiles[profile]);
        state.cpu.data.eax = 0xaabb0080u;
        before = state.cpu;
        failed = !legacy_lock_run(&state, cbw, sizeof(cbw), 1u,
            &after) || after.data.eip != sizeof(cbw) ||
            after.data.eax != 0xaabbff80u ||
            after.data.eflags != before.data.eflags ||
            after.data.ecx != before.data.ecx ||
            after.data.edx != before.data.edx ||
            after.data.ebx != before.data.ebx ||
            after.data.esp != before.data.esp ||
            after.data.ebp != before.data.ebp ||
            after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi ||
            !legacy_lock_sregs_same(&before, &after);
        if (failed) {
            lib_c_fprintf(lib_c_stderr, "LOCK CBW profile=%u\n", (unsigned)profile);
            return 0;
        }

        cpu_instruction_prepare(&state, profiles[profile]);
        state.cpu.data.eax = 0xaabb0002u;
        failed = cpu_instruction_write(&state, 0x100u, &image, sizeof(image),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            !legacy_lock_run(&state, add_memory, sizeof(add_memory),
            1u, &after) || after.data.eip != sizeof(add_memory) ||
            cpu_instruction_read(&state, 0x100u, &image, sizeof(image),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                LIB_STATUS_OK || image != 5u;
        if (failed) {
            lib_c_fprintf(lib_c_stderr, "LOCK ADD profile=%u\n", (unsigned)profile);
            return 0;
        }

        cpu_instruction_prepare(&state, profiles[profile]);
        state.cpu.data.ecx = 0x11220002u;
        state.cpu.data.esi = 0x0100u;
        state.cpu.data.edi = 0x0200u;
        failed = cpu_instruction_write(&state, 0x100u, source, sizeof(source),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            cpu_instruction_write(&state, 0x200u, target, sizeof(target),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            !legacy_lock_run(&state, rep_movs, sizeof(rep_movs), 2u,
                &after) || after.data.eip != sizeof(rep_movs) ||
            after.data.ecx != 0x11220000u || after.data.esi != 0x0102u ||
            after.data.edi != 0x0202u ||
            cpu_instruction_read(&state, 0x200u, target, sizeof(target),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                LIB_STATUS_OK || lib_memory_compare(source, target,
                sizeof(source)) != 0;
        if (failed) {
            lib_c_fprintf(lib_c_stderr, "LOCK REP MOVS profile=%u\n", (unsigned)profile);
            return 0;
        }
    }
    return 1;
}

static lib_i32 legacy_lock_test_legacy_ud(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286
    };
    static const lib_u8 code[] = { 0xf0u, 0xf1u };

    for (lib_size profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        cpu_instruction_fixture state;

        cpu_instruction_prepare(&state, profiles[profile]);
        if (!cpu_instruction_expect_real_fault(&state, code, sizeof(code), 6u))
            return 0;
    }
    return 1;
}

static lib_i32 legacy_lock_test_80386_regression(void)
{
    static const lib_u8 legal[] = { 0xf0u, 0x01u, 0x06u, 0x00u, 0x01u };
    static const lib_u8 invalid[] = { 0xf0u, 0x01u, 0xc0u };
    cpu_instruction_fixture state;
    t_cpu after;
    lib_u16 image = 1u;

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.cpu.data.eax = 2u;
    if (cpu_instruction_write(&state, 0x100u, &image, sizeof(image),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
        !legacy_lock_run(&state, legal, sizeof(legal), 1u, &after) ||
        after.data.eip != sizeof(legal) ||
        cpu_instruction_read(&state, 0x100u, &image, sizeof(image),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
            LIB_STATUS_OK || image != 3u) return 0;

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    return cpu_instruction_expect_real_fault(&state, invalid, sizeof(invalid),
        6u);
}

int main(void)
{
    if (!legacy_lock_test_transparent_real()) {
        lib_c_fprintf(lib_c_stderr, "%s", "CPU-LEGACY-LOCK:TRANSPARENT:FAIL\n");
        return 1;
    }
    if (!legacy_lock_test_legacy_ud()) {
        lib_c_fprintf(lib_c_stderr, "%s", "CPU-LEGACY-LOCK:UD:FAIL\n");
        return 1;
    }
    if (!legacy_lock_test_80386_regression()) {
        lib_c_fprintf(lib_c_stderr, "%s", "CPU-LEGACY-LOCK:80386:FAIL\n");
        return 1;
    }
    lib_c_printf("%s\n", "CPU-LEGACY-LOCK:OK");
    return 0;
}
