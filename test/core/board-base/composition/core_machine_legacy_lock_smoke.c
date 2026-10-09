#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "core/board-base/machine_board_interface.h"
#include "core/x86/debug_interface.h"

#define LEGACY_LOCK_IOPL 0x3000u

typedef struct legacy_lock_machine {
    core_machine *machine;
    lib_u32 writes;
    lib_u16 last_port;
    lib_u32 last_value;
} legacy_lock_machine;

static lib_status legacy_lock_port_read(void *owner,
    lib_u16 port, lib_u64 tick,
    lib_u32 *value)
{
    (void)tick;
    (void)owner;
    (void)port;
    if (value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *value = 0u;
    return LIB_STATUS_OK;
}

static lib_status legacy_lock_port_write(void *owner,
    lib_u16 port, lib_u32 value)
{
    legacy_lock_machine *state = (legacy_lock_machine *)owner;

    if (state == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    ++state->writes;
    state->last_port = port;
    state->last_value = value;
    return LIB_STATUS_OK;
}

static const core_machine_port_provider legacy_lock_port_provider = {
    legacy_lock_port_read, legacy_lock_port_write
};

static lib_i32 legacy_lock_prepare(core_machine_cpu_profile profile,
    legacy_lock_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP)
    };

    lib_memory_set(state, 0, sizeof(*state));
    return core_machine_create(&config, &state->machine, LIB_NULL) == LIB_STATUS_OK &&
        core_machine_install_port_provider(state->machine, 0x005au, 0x005au,
            &legacy_lock_port_provider, state) == LIB_STATUS_OK &&
        core_machine_freeze_execution_providers(state->machine) == LIB_STATUS_OK &&
        core_machine_reset(state->machine) == LIB_STATUS_OK &&
        core_machine_debug_patch_registers(state->machine, &entry) == LIB_STATUS_OK;
}

static lib_i32 legacy_lock_test_port_output(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const lib_u8 code[] = { 0xf0u, 0xe6u, 0x5au };

    for (lib_size profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        legacy_lock_machine state = {0};
        core_machine_cpu_diagnostic diagnostic = {0};
        core_machine_run_result result = {0};
        core_machine_debug_cpu_snapshot before = {0};
        core_machine_debug_cpu_snapshot after = {0};
        const core_machine_debug_register_patch eax = {
            .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX),
            .values = { [CORE_MACHINE_DEBUG_EAX] = 0xaabbcc44u }
        };
        lib_i32 failed = !legacy_lock_prepare(profiles[profile], &state);

        if (!failed)
            failed = core_machine_debug_patch_registers(state.machine, &eax) !=
                LIB_STATUS_OK ||
                core_machine_debug_capture_cpu_snapshot(state.machine,
                    CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0u, code,
                    sizeof(code)) != LIB_STATUS_OK ||
                core_machine_run(state.machine,
                    (core_machine_run_budget){1u, 0u}, &result) != LIB_STATUS_OK ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                    LIB_STATUS_OK ||
                core_machine_debug_capture_cpu_snapshot(state.machine,
                    CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET ||
                diagnostic.first_fault.valid ||
                after.eip != sizeof(code) || after.eax != before.eax ||
                after.ecx != before.ecx || after.edx != before.edx ||
                after.ebx != before.ebx || after.esp != before.esp ||
                after.ebp != before.ebp || after.esi != before.esi ||
                after.edi != before.edi || after.eflags != before.eflags ||
                lib_memory_compare(&after.es, &before.es, sizeof(after.es)) ||
                lib_memory_compare(&after.cs, &before.cs, sizeof(after.cs)) ||
                lib_memory_compare(&after.ss, &before.ss, sizeof(after.ss)) ||
                lib_memory_compare(&after.ds, &before.ds, sizeof(after.ds)) ||
                lib_memory_compare(&after.fs, &before.fs, sizeof(after.fs)) ||
                lib_memory_compare(&after.gs, &before.gs, sizeof(after.gs)) ||
                state.writes != 1u || state.last_port != 0x005au ||
                state.last_value != 0x44u;
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static lib_i32 legacy_lock_prepare_80286_protected(
    legacy_lock_machine *state, lib_u8 cpl, lib_u32 eflags)
{
    static const lib_u8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0x00u,0x05u,
        0x0fu,0x01u,0x1eu,0x10u,0x05u,
        0xb8u,0x01u,0x00u,
        0x0fu,0x01u,0xf0u
    };
    static const lib_u8 gdtr[] = {0x17u,0u,0u,0x03u,0u,0u};
    static const lib_u8 idtr[] = {0x6fu,0u,0u,0x04u,0u,0u};
    static const lib_u8 handler[] = {0xf4u};
    lib_u8 gdt[] = {
        0u,0u,0u,0u,0u,0u,0u,0u,
        0xffu,0xffu,0u,0x20u,0u,0xfau,0u,0u,
        0xffu,0xffu,0u,0u,0u,0xf2u,0u,0u
    };
    lib_u8 gate[8] = {0u};
    core_machine_run_result result = {0};
    core_machine_debug_register_patch entry = {0};

    if (!legacy_lock_prepare(CORE_MACHINE_CPU_PROFILE_80286, state))
        return 0;
    gdt[13u] = cpl ? 0xfau : 0x9au;
    gdt[21u] = cpl ? 0xf2u : 0x92u;
    gate[1u] = 0x01u;
    gate[2u] = cpl ? 0x0bu : 0x08u;
    gate[5u] = 0x86u;
    if (core_machine_memory_write(state->machine, 0x0500u, gdtr,
            sizeof(gdtr)) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, 0x0510u, idtr,
            sizeof(idtr)) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, 0x0300u, gdt,
            sizeof(gdt)) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, 0x0468u, gate,
            sizeof(gate)) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, 0x2100u, handler,
            sizeof(handler)) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, 0u, bootstrap,
            sizeof(bootstrap)) != LIB_STATUS_OK ||
        core_machine_run(state->machine,
            (core_machine_run_budget){4u, 0u}, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 4u)
        return 0;

    entry.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS);
    entry.values[CORE_MACHINE_DEBUG_CS] = cpl ? 0x0bu : 0x08u;
    entry.values[CORE_MACHINE_DEBUG_SS] = cpl ? 0x13u : 0x10u;
    entry.values[CORE_MACHINE_DEBUG_DS] = cpl ? 0x13u : 0x10u;
    entry.values[CORE_MACHINE_DEBUG_EIP] = 0u;
    entry.values[CORE_MACHINE_DEBUG_EAX] = 0xaabb0080u;
    entry.values[CORE_MACHINE_DEBUG_ESP] = 0x8000u;
    entry.values[CORE_MACHINE_DEBUG_EFLAGS] = eflags;
    return core_machine_debug_patch_registers(state->machine, &entry) ==
        LIB_STATUS_OK;
}

static lib_i32 legacy_lock_test_80286_iopl(void)
{
    static const lib_u8 code[] = {0xf0u, 0x98u};

    for (lib_u8 form = 0u; form != 3u; ++form) {
        legacy_lock_machine state = {0};
        core_machine_cpu_diagnostic diagnostic = {0};
        core_machine_run_result result = {0};
        core_machine_debug_cpu_snapshot after = {0};
        lib_u16 frame[4] = {0u};
        const lib_u8 cpl = form == 0u ? 0u : 3u;
        const lib_u32 flags = form == 0u ? 0u :
            (form == 1u ? LEGACY_LOCK_IOPL : 0u) |
            CORE_MACHINE_DEBUG_EFLAGS_CF;
        lib_i32 failed = !legacy_lock_prepare_80286_protected(&state,
            cpl, flags);

        if (!failed)
            failed = core_machine_memory_write(state.machine, 0x2000u, code,
                sizeof(code)) != LIB_STATUS_OK ||
                core_machine_run(state.machine,
                    (core_machine_run_budget){1u, 0u}, &result) != LIB_STATUS_OK ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                    LIB_STATUS_OK ||
                core_machine_debug_capture_cpu_snapshot(state.machine,
                    CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET ||
                diagnostic.first_fault.valid ||
                after.eax != (form == 2u ? 0xaabb0080u : 0xaabbff80u) ||
                after.eip != (form == 2u ? 0x0100u : sizeof(code)) ||
                after.eflags != (form == 0u ? 0u : flags) ||
                (form == 2u &&
                    (!diagnostic.last_delivered_exception.valid ||
                    !(diagnostic.last_delivered_exception.exception_mask &
                        VCPUINS_EXCEPT_GP) ||
                    diagnostic.last_delivered_exception.exception_code != 0u ||
                    after.cs.selector != 0x000bu ||
                    core_machine_memory_read(state.machine,
                        after.ss.base + (lib_u16)after.esp, frame,
                        sizeof(frame)) != LIB_STATUS_OK ||
                    frame[0] != 0u || frame[1] != 0u ||
                    frame[2] != 0x000bu ||
                    frame[3] != CORE_MACHINE_DEBUG_EFLAGS_CF));
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

int main(void)
{
    if (!legacy_lock_test_port_output() ||
        !legacy_lock_test_80286_iopl()) {
        lib_c_fprintf(lib_c_stderr, "%s", "BOARD-LEGACY-LOCK:FAIL\n");
        return 1;
    }
    lib_c_printf("%s\n", "BOARD-LEGACY-LOCK:OK");
    return 0;
}
