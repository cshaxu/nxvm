#include "lib/types/file.h"
#include "lib/types/types_interface.h"

#include "x86-nxvm/board-base/machine_board_interface.h"
#include "../board_board_fixture.h"

#define MOVX_SOURCE_MEMORY 0x5000u

typedef struct movx_provider {
    lib_u32 reads;
    lib_u8 value[2];
    lib_status read_status;
} movx_provider;

typedef struct movx_machine {
    x86_core_machine *machine;
} movx_machine;

static lib_status movx_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    movx_provider *provider = (movx_provider *)owner;

    /* The no-handler fixture rejects the IVT read, not private CPU state. */
    if (physical == 0x18u) return LIB_STATUS_IO_ERROR;
    if (provider == LIB_NULL || physical != MOVX_SOURCE_MEMORY ||
        (bytes != 1u && bytes != 2u)) return LIB_STATUS_INVALID_ARGUMENT;
    if (!observe_only) ++provider->reads;
    if (provider->read_status != LIB_STATUS_OK) return provider->read_status;
    lib_memory_copy((void *)destination, provider->value, bytes);
    return LIB_STATUS_OK;
}

static lib_status movx_write(void *owner, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    (void)owner;
    (void)physical;
    (void)source;
    (void)bytes;
    return LIB_STATUS_UNSUPPORTED;
}

static lib_status movx_query(void *owner, lib_u32 physical,
    lib_uptr bytes, x86_core_machine_memory_access access)
{
    (void)owner;
    if (physical >= 0x18u && physical < 0x1cu &&
        bytes <= 0x1cu - physical && access == X86_CORE_MACHINE_MEMORY_ACCESS_READ)
        return LIB_STATUS_OK;
    return physical == MOVX_SOURCE_MEMORY && (bytes == 1u || bytes == 2u) &&
        (access == X86_CORE_MACHINE_MEMORY_ACCESS_READ ||
         access == X86_CORE_MACHINE_MEMORY_ACCESS_WRITE) ? LIB_STATUS_OK :
        LIB_STATUS_UNSUPPORTED;
}

static lib_i32 movx_prepare(x86_core_cpu_profile profile,
    movx_provider *provider, movx_machine *state)
{
    const x86_nxvm_board_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = profile,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const x86_core_machine_memory_device_route routes[] = {
        {.physical_start = MOVX_SOURCE_MEMORY, .bytes = 2u,
            .callbacks = {movx_read, movx_write, movx_query}, .failure = LIB_NULL},
        {.physical_start = 0x18u, .bytes = 4u,
            .callbacks = {movx_read, movx_write, movx_query}, .failure = LIB_NULL}
    };

    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ES) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP)
    };

    if (state == LIB_NULL) return 0;
    lib_memory_set(state, 0, sizeof(*state));
    if (x86_nxvm_board_create(&config, &state->machine, LIB_NULL) != LIB_STATUS_OK ||
        (provider != LIB_NULL && x86_core_machine_install_memory_device_routes(
            state->machine, routes, sizeof(routes) / sizeof(routes[0]),
            LIB_NULL, LIB_NULL, provider) != LIB_STATUS_OK) ||
        x86_core_machine_freeze_execution_providers(state->machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(state->machine) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(state->machine, &entry) != LIB_STATUS_OK) {
        x86_core_machine_destroy(state->machine);
        state->machine = LIB_NULL;
        return 0;
    }
    return 1;
}

static lib_i32 movx_run(movx_machine *state, const lib_u8 *code,
    lib_size code_size, x86_core_cpu_debug_cpu_snapshot *out_cpu,
    x86_core_cpu_diagnostic *out_diagnostic)
{
    const x86_core_machine_run_budget budget = {1u, 0u};
    x86_core_machine_run_result result;
    lib_status status;

    if (state == LIB_NULL || state->machine == LIB_NULL || code == LIB_NULL ||
        out_cpu == LIB_NULL || out_diagnostic == LIB_NULL ||
        x86_core_machine_memory_write(state->machine, 0u, code, code_size) !=
            LIB_STATUS_OK) return 0;
    /* REAL_UD_TERMINAL_IVT_REJECT: vector-6 bus read fails. */
    status = x86_core_machine_run(state->machine, budget, &result);
    if (status != LIB_STATUS_INTERNAL_ERROR ||
        result.reason != X86_CORE_MACHINE_STOP_FAULT ||
        x86_core_machine_get_cpu_diagnostic(state->machine, out_diagnostic) !=
            LIB_STATUS_OK) return 0;
    return x86_core_machine_debug_capture_cpu_snapshot(state->machine,
        X86_CORE_CPU_SNAPSHOT_CURRENT, out_cpu) == LIB_STATUS_OK;
}

static lib_i32 movx_prepare_protected_limit(movx_machine *state)
{
    static const lib_u8 gdt_pointer[] = {0x1fu,0,0,0x03u,0,0};
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0x0fu,0,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x40u,0,0x92u,0x40u,0
    };
    static const lib_u8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,
        0xb8u,0x18u,0x00u,0x8eu,0xd0u,
        0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    const x86_core_machine_run_budget budget = {10u, 0u};
    x86_core_machine_run_result result;

    return movx_prepare(X86_CORE_CPU_PROFILE_80386, LIB_NULL, state) &&
        x86_core_machine_memory_write(state->machine, 0x0100u, gdt_pointer,
            sizeof(gdt_pointer)) == LIB_STATUS_OK &&
        x86_core_machine_memory_write(state->machine, 0x0300u, gdt, sizeof(gdt)) ==
            LIB_STATUS_OK && x86_core_machine_memory_write(state->machine, 0u,
            bootstrap, sizeof(bootstrap)) == LIB_STATUS_OK &&
        x86_core_machine_run(state->machine, budget, &result) == LIB_STATUS_OK &&
        result.reason == X86_CORE_MACHINE_STOP_BUDGET && result.executed == 10u;
}

static lib_i32 movx_test_read_boundaries(void)
{
    static const lib_u8 code[] = {0x0fu,0xb6u,0x0eu,0x00u,0x50u};
    const lib_u32 flags = 0x41u; /* CF | ZF */
    const x86_core_cpu_debug_register_patch registers = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ECX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS),
        .values = {
            [X86_CORE_CPU_DEBUG_ECX] = 0xaabbccddu,
            [X86_CORE_CPU_DEBUG_EFLAGS] = 0x41u
        }
    };
    x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_80186, X86_CORE_CPU_PROFILE_80286
    };
    lib_u8 opcode;
    lib_u8 profile_index;
    lib_u8 changed_byte;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
         ++profile_index) {
        /* Byte 1 covers each opcode; byte 2 retains the original ModR/M cases. */
        for (changed_byte = 1u; changed_byte <= 2u; ++changed_byte) {
            for (opcode = 0xb6u; opcode <= 0xbfu; ++opcode) {
                if (opcode == 0xb8u || opcode == 0xb9u || opcode == 0xbau ||
                    opcode == 0xbbu || opcode == 0xbcu || opcode == 0xbdu) continue;
                movx_provider provider = {0u, {0x80u,0x01u}, LIB_STATUS_OK};
                movx_machine state;
                x86_core_cpu_debug_cpu_snapshot after;
                x86_core_cpu_diagnostic diagnostic;
                lib_i32 failed = !movx_prepare(profiles[profile_index], &provider,
                    &state);
                lib_u8 form_code[sizeof(code)];

                lib_memory_copy(form_code, code, sizeof(code));
                form_code[changed_byte] = opcode;
                if (!failed) {
                    failed |= x86_core_machine_debug_patch_registers(state.machine,
                        &registers) != LIB_STATUS_OK ||
                        !movx_run(&state, form_code, sizeof(form_code),
                        &after, &diagnostic) || !diagnostic.first_fault.valid ||
                        !(((diagnostic.first_fault.exception_mask) & (VCPUINS_EXCEPT_CE)) != 0u) ||
                        provider.reads != 0u ||
                        after.ecx != 0xaabbccddu || after.eflags != flags ||
                        after.eip != 0u;
                }
                x86_core_machine_destroy(state.machine);
                if (failed) return 0;
            }
        }
    }

    {
        static const lib_u8 limit_code[] = {0x0fu,0xbfu,0x0eu,0x10u,0x00u};
        const x86_core_machine_run_budget budget = {1u, 0u};
        movx_machine state;
        x86_core_cpu_debug_cpu_snapshot after;
        x86_core_machine_run_result result;
        lib_i32 failed = !movx_prepare_protected_limit(&state);

        if (!failed) {
            failed |= x86_core_machine_debug_patch_registers(state.machine,
                &registers) != LIB_STATUS_OK ||
                x86_core_machine_memory_write(state.machine, 0x2000u,
                    limit_code, sizeof(limit_code)) != LIB_STATUS_OK ||
                !test_x86_nxvm_machine_fixture_shutdown_wait(x86_core_machine_run(state.machine,
                    budget, &result), &result) ||
                x86_core_machine_debug_capture_cpu_snapshot(state.machine,
                    X86_CORE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
                after.ecx != 0xaabbccddu || after.eflags != flags ||
                after.eip != 0u;
        }
        x86_core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!movx_test_read_boundaries()) return 1;
    lib_c_printf("MOVX:OK\n");
    lib_c_printf("MOVX-PROFILES:OK\n");
    return 0;
}
