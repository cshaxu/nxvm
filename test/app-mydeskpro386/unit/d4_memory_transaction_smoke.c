#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "x86-core/machine/machine_interface.h"

/* Same-owner white-box parity input; do not expose a public fault injector
 * or borrow the neutral Machine's private RAM/registry representation. */
#include "app-mydeskpro386/d4_memory.c"

#define D4_TEST_REGISTRATION_CAPACITY 512u

typedef struct iochk_probe {
    lib_u32 assertions;
    lib_u32 clears;
} iochk_probe;

static void observe_iochk(void *context, lib_bool asserted)
{
    iochk_probe *probe = context;

    if (asserted) ++probe->assertions;
    else ++probe->clears;
}

typedef struct d4_memory_case {
    mydeskpro386_d4_memory memory;
    mydeskpro386_d4_memory_config config;
    iochk_probe probe;
    lib_u32 mode;
    lib_u32 attempts;
    lib_i32 failed;
} d4_memory_case;

static lib_status configure_d4_memory(x86_core_machine *machine, void *context)
{
    d4_memory_case *state = context;
    lib_status status;

    if (state->attempts++ == (state->mode == 4u ? 1u : 0u)) {
        const x86_core_machine_memory_alias_config alias = {
            0x000e0000u, 0x00100000u, 0x00020000u
        };

        status = x86_core_machine_install_memory_aliases(machine, &alias, 1u, LIB_FALSE);
        if (status != LIB_STATUS_OK) return status;
    }
    status = mydeskpro386_d4_memory_configure(machine, &state->memory,
        &state->config, observe_iochk, &state->probe);

    if (status == LIB_STATUS_OK) state->failed |= !state->memory.configured;
    else state->failed |= state->memory.configured ||
        state->memory.core != LIB_NULL || state->memory.control != 0u ||
        state->memory.iochk_output != LIB_NULL ||
        state->probe.assertions != 0u || state->probe.clears != 0u;
    return status;
}

static lib_i32 exercise_d4_memory(x86_core_machine *machine, void *context)
{
    d4_memory_case *state = context;
    lib_u8 value = 0u;
    lib_i32 failed = 0;

    failed |= x86_core_machine_memory_read(machine, 0x80c00000u,
        &value, 1u) != LIB_STATUS_OK || value != 0xf7u;
    /* Exercise the installed owner's electrical boundary, not a board lookup. */
    mydeskpro386_d4_parity_fault(&state->memory, 3u);
    failed |= state->memory.parity_fault_mask != 0x08u ||
        state->probe.assertions != 1u || state->probe.clears != 0u;
    failed |= x86_core_machine_memory_write(machine, 0x100u,
        &value, 1u) != LIB_STATUS_OK || state->probe.clears != 1u;
    state->memory.parity_fault_mask = 1u;
    state->memory.ram_setup = 1u;
    mydeskpro386_d4_memory_reset(&state->memory);
    failed |= state->memory.parity_fault_mask != 0u ||
        state->memory.ram_setup != state->config.ram_setup ||
        state->memory.control != 0xffu;
    return failed;
}

static lib_status filler_read(void *owner, lib_u32 address, lib_uptr destination,
    lib_uptr bytes, lib_bool observe_only)
{
    (void)owner; (void)address; (void)destination; (void)bytes; (void)observe_only;
    return LIB_STATUS_UNSUPPORTED;
}

static lib_status filler_write(void *owner, lib_u32 address, lib_uptr source,
    lib_uptr bytes)
{
    (void)owner; (void)address; (void)source; (void)bytes;
    return LIB_STATUS_UNSUPPORTED;
}

static lib_status filler_query(void *owner, lib_u32 address, lib_uptr bytes,
    x86_core_machine_memory_access access)
{
    (void)owner; (void)address; (void)bytes; (void)access;
    return LIB_STATUS_UNSUPPORTED;
}

static void filler_observer(void *owner, lib_u32 address, lib_uptr bytes)
{
    (void)owner; (void)address; (void)bytes;
}

static void filler_fault(void *owner, lib_u32 address)
{
    (void)owner; (void)address;
}

static lib_i32 run_case(lib_u32 mode)
{
    const x86_core_machine_executor_config configuration = {
        .memory_bytes = 2u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386
    };
    const x86_core_machine_memory_parity_config parity = {1024u, filler_fault};
    d4_memory_case state = {
        .config = {LIB_TRUE, 0xf7u, 0x80u, 0x0002u}, .mode = mode
    };
    x86_core_machine *machine = LIB_NULL;
    lib_u8 owners[D4_TEST_REGISTRATION_CAPACITY] = {0};
    lib_size count = 0u;
    lib_status status = LIB_STATUS_OK;
    const lib_status expected = mode == 2u || mode == 4u ?
        LIB_STATUS_INVALID_ARGUMENT : LIB_STATUS_NO_MEMORY;
    x86_core_machine_memory_route observed_route;
    lib_i32 failed = 1;

    if (x86_core_machine_neutral_create(&configuration, &machine) != LIB_STATUS_OK)
        goto done;
    if (mode <= 1u || mode == 3u) {
        for (; count < D4_TEST_REGISTRATION_CAPACITY; ++count) {
            const x86_core_machine_memory_device_route route = {
                .physical_start = 0x1000u + (lib_u32)count * 0x100u, .bytes = 1u,
                .callbacks = {filler_read, filler_write, filler_query},
                .failure = LIB_NULL
            };
            status = x86_core_machine_install_memory_device_routes(machine,
                mode == 3u ? LIB_NULL : &route, mode == 3u ? 0u : 1u,
                mode == 3u ? filler_observer : LIB_NULL, LIB_NULL, &owners[count]);
            if (status != LIB_STATUS_OK) break;
        }
        /* The local array is containment, not a copy of a private capacity. */
        if (status != LIB_STATUS_NO_MEMORY || count == 0u ||
            count == D4_TEST_REGISTRATION_CAPACITY) goto done;
        if (mode == 1u && x86_core_machine_remove_memory_device_routes(machine,
                &owners[--count]) != LIB_STATUS_OK) goto done;
    } else if (mode == 2u) {
        if (x86_core_machine_install_memory_device_routes(machine, LIB_NULL, 0u,
                LIB_NULL, &parity, &owners[count++]) != LIB_STATUS_OK) goto done;
    }
    if (configure_d4_memory(machine, &state) != expected || state.failed)
        goto done;
    for (lib_size index = 0u; index < count; ++index) {
        if (x86_core_machine_remove_memory_device_routes(machine, &owners[index]) !=
            LIB_STATUS_OK) goto done;
    }
    if (configure_d4_memory(machine, &state) != LIB_STATUS_OK || state.failed ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        exercise_d4_memory(machine, &state) ||
        x86_core_machine_remove_memory_device_routes(machine, &state.memory) != LIB_STATUS_OK ||
        x86_core_machine_memory_query(machine, MYDESKPRO386_D4_CONTROL_PHYSICAL, 1u,
            X86_CORE_MACHINE_MEMORY_ACCESS_READ, &observed_route) != LIB_STATUS_INTERNAL_ERROR)
        goto done;
    failed = 0;
done:
    x86_core_machine_destroy(machine);
    if (failed) lib_c_printf("D4-MEMORY-TRANSACTION:mode=%u:attempts=%u:fillers=%llu\n",
        mode, state.attempts, (unsigned long long)count);

    return failed || state.failed;
}

lib_i32 main(void)
{
    lib_i32 failed = 0;

    for (lib_u32 mode = 0u; mode < 5u; ++mode) failed |= run_case(mode);
    if (!failed) lib_c_printf("D4-MEMORY-TRANSACTION:OK\n");
    return failed ? 1 : 0;
}
