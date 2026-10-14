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

static lib_i32 legacy_lock_test_register_sources(core_machine_cpu_profile profile)
{
    lib_u8 legal[6];
    static const lib_u8 opcodes[] = {0x00u, 0x08u, 0x10u, 0x18u,
        0x20u, 0x28u, 0x30u, 0x86u};
    static const lib_u32 results[] = {3u, 3u, 3u, LIB_UINT32_MAX,
        0u, LIB_UINT32_MAX, 3u, 2u};
    static const lib_u8 invalid[] = { 0xf0u, 0x01u, 0xc0u };
    cpu_instruction_fixture state;
    t_cpu after;

    for (lib_u8 operation = 0u; operation != sizeof(opcodes); ++operation) {
        for (lib_u8 size = 0u; size != 3u; ++size) {
            for (lib_u8 reg = 0u; reg != 8u; ++reg) {
                lib_u32 image = 1u;
                const lib_u8 width = (lib_u8)(1u << size);
                lib_u8 length = 0u;

                cpu_instruction_prepare(&state, profile);
                state.cpu.data.eax = state.cpu.data.ecx = state.cpu.data.edx =
                    state.cpu.data.ebx = state.cpu.data.esp = state.cpu.data.ebp =
                    state.cpu.data.esi = state.cpu.data.edi = size ? 2u : 0x0202u;
                state.cpu.data.eflags = 2u;
                legal[length++] = 0xf0u;
                if (size == 2u) legal[length++] = 0x66u;
                legal[length++] = (lib_u8)(opcodes[operation] + (size != 0u));
                legal[length++] = (lib_u8)(0x06u | (reg << 3u));
                legal[length++] = 0u;
                legal[length++] = 1u;
                if (cpu_instruction_write(&state, 0x100u, &image, width,
                        CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
                    !legacy_lock_run(&state, legal, length, 1u, &after) ||
                    after.data.eip != length ||
                    cpu_instruction_read(&state, 0x100u, &image, width,
                        CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                        LIB_STATUS_OK ||
                    image != (results[operation] & (size == 2u ? LIB_UINT32_MAX :
                        size == 1u ? 0xffffu : 0xffu))) return 0;
            }
        }
    }

    cpu_instruction_prepare(&state, profile);
    return cpu_instruction_expect_real_fault(&state, invalid, sizeof(invalid),
        6u);
}

static lib_bool legacy_lock_test_groups(void)
{
    lib_bool passed = LIB_TRUE;
    static const struct {
        lib_u8 opcode;
        lib_bool extended;
        lib_u8 legal;
        lib_u8 immediate;
        lib_bool byte;
    } forms[] = {
        {0x80u, LIB_FALSE, 0x7fu, 1u, LIB_TRUE},
        {0x81u, LIB_FALSE, 0x7fu, 2u, LIB_FALSE},
        {0x83u, LIB_FALSE, 0x7fu, 1u, LIB_FALSE},
        {0xfeu, LIB_FALSE, 0x03u, 0u, LIB_TRUE},
        {0xffu, LIB_FALSE, 0x03u, 0u, LIB_FALSE},
        {0xf6u, LIB_FALSE, 0x0cu, 0u, LIB_TRUE},
        {0xf7u, LIB_FALSE, 0x0cu, 0u, LIB_FALSE},
        {0xbau, LIB_TRUE, 0xe0u, 1u, LIB_FALSE},
        {0xabu, LIB_TRUE, 0xffu, 0u, LIB_FALSE},
        {0xb3u, LIB_TRUE, 0xffu, 0u, LIB_FALSE},
        {0xbbu, LIB_TRUE, 0xffu, 0u, LIB_FALSE},
        {0xa3u, LIB_TRUE, 0x00u, 0u, LIB_FALSE}
    };

    for (core_machine_cpu_profile profile = CORE_MACHINE_CPU_PROFILE_80386;
         profile <= CORE_MACHINE_CPU_PROFILE_80486SX; ++profile)
    for (lib_size form = 0u; form < sizeof(forms) / sizeof(forms[0]); ++form)
    for (lib_u8 wide = 0u; wide < (forms[form].byte ? 1u : 2u); ++wide)
    for (lib_u8 reg = 0u; reg < 8u; ++reg)
    for (lib_u8 direct = 0u; direct < 2u; ++direct) {
        cpu_instruction_fixture state;
        t_cpu locked, ordinary;
        lib_u8 code[10] = {0xf0u}, length = 1u;
        const lib_bool legal = !direct && (forms[form].legal & (1u << reg));
        const lib_u8 width = forms[form].byte ? 1u : wide ? 4u : 2u;
        lib_u32 locked_value = 0u, ordinary_value = 0u;

        if (wide) code[length++] = 0x66u;
        if (forms[form].extended) code[length++] = 0x0fu;
        code[length++] = forms[form].opcode;
        code[length++] = (lib_u8)((direct ? 0xc0u : 0x06u) | (reg << 3u));
        if (!direct) {
            code[length++] = 0u;
            code[length++] = 1u;
        }
        if (forms[form].immediate) {
            code[length++] = 1u;
            for (lib_u8 i = 1u; i < (forms[form].immediate == 2u ?
                    width : 1u); ++i) code[length++] = 0u;
        }
        cpu_instruction_prepare(&state, profile);
        if (!legal) {
            if (!cpu_instruction_expect_real_fault(&state, code, length, 6u)) {
                lib_c_fprintf(lib_c_stderr, "LOCK reject: profile=%u form=%u reg=%u direct=%u\n",
                    profile, (lib_u32)form, reg, direct);
                passed = LIB_FALSE;
            }
            continue;
        }
        state.cpu.data.eax = state.cpu.data.ecx = state.cpu.data.edx =
            state.cpu.data.ebx = state.cpu.data.esp = state.cpu.data.ebp =
            state.cpu.data.esi = state.cpu.data.edi = 1u;
        state.memory[0x100u] = 3u;
        if (!legacy_lock_run(&state, code, length, 1u, &locked) ||
            state.delivered_exception.valid) return LIB_FALSE;
        lib_memory_copy(&locked_value, state.memory + 0x100u, width);
        cpu_instruction_prepare(&state, profile);
        state.cpu.data.eax = state.cpu.data.ecx = state.cpu.data.edx =
            state.cpu.data.ebx = state.cpu.data.esp = state.cpu.data.ebp =
            state.cpu.data.esi = state.cpu.data.edi = 1u;
        state.memory[0x100u] = 3u;
        code[0] = 0x3eu; /* Default DS override preserves the instruction length. */
        if (!legacy_lock_run(&state, code, length, 1u, &ordinary) ||
            state.delivered_exception.valid) return LIB_FALSE;
        lib_memory_copy(&ordinary_value, state.memory + 0x100u, width);
        if (locked_value != ordinary_value ||
            lib_memory_compare(&locked, &ordinary, sizeof(locked)) != 0)
            return LIB_FALSE;
    }
    return passed;
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
    if (!legacy_lock_test_register_sources(CORE_MACHINE_CPU_PROFILE_80386) ||
        !legacy_lock_test_register_sources(CORE_MACHINE_CPU_PROFILE_80486SX) ||
        !legacy_lock_test_groups()) {
        lib_c_fprintf(lib_c_stderr, "%s", "CPU-LEGACY-LOCK:386-486:FAIL\n");
        return 1;
    }
    lib_c_printf("%s\n", "CPU-LEGACY-LOCK:OK");
    return 0;
}
