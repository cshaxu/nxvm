#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "app-mydeskpro386/profiles/d4_memory.h"
#include "../../../core/board-base/composition/composition_fixture.h"
#include "../../../core/support/memory_registration_fixture.h"

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
    core_machine_d4_memory memory;
    core_machine_d4_memory_config config;
    iochk_probe probe;
    lib_i32 failed;
} d4_memory_case;

static lib_status configure_d4_memory(core_machine *machine, void *context)
{
    d4_memory_case *state = context;
    lib_status status = core_machine_d4_memory_configure(machine, &state->memory,
        &state->config, observe_iochk, &state->probe);

    if (status == LIB_STATUS_OK) state->failed |= !state->memory.configured;
    else state->failed |= state->memory.configured ||
        state->memory.iochk_output != LIB_NULL ||
        state->probe.assertions != 0u || state->probe.clears != 0u;
    return status;
}

static lib_i32 exercise_d4_memory(core_machine *machine, void *context)
{
    d4_memory_case *state = context;
    lib_u8 value = 0u;
    lib_i32 failed = 0;

    failed |= test_core_read_physical(machine, 0x80c00000u,
        (lib_uptr)&value, 1u) != LIB_STATUS_OK || value != 0xf7u;
    /* Exercise the installed owner's electrical boundary, not a board lookup. */
    test_core_invoke_parity_fault(machine, 3u);
    failed |= state->memory.parity_fault_mask != 0x08u ||
        state->probe.assertions != 1u || state->probe.clears != 0u;
    failed |= test_core_write_physical(machine, 0x100u,
        (lib_uptr)&value, 1u) != LIB_STATUS_OK || state->probe.clears != 1u;
    state->memory.parity_fault_mask = 1u;
    state->memory.ram_setup = 1u;
    core_machine_d4_memory_reset(&state->memory);
    failed |= state->memory.parity_fault_mask != 0u ||
        state->memory.ram_setup != state->config.ram_setup ||
        state->memory.control != 0xffu;
    return failed;
}

static lib_i32 run_case(lib_u32 mode)
{
    d4_memory_case state = {.config = {LIB_TRUE, 0xf7u, 0x80u, 0x0002u}};
    lib_i32 failed = test_core_memory_registration_case(mode, &state.memory,
        configure_d4_memory, exercise_d4_memory, &state);

    return failed || state.failed;
}

lib_i32 main(void)
{
    lib_i32 failed = 0;

    for (lib_u32 mode = 0u; mode < 4u; ++mode) failed |= run_case(mode);
    if (!failed) lib_c_printf("D4-MEMORY-TRANSACTION:OK\n");
    return failed ? 1 : 0;
}
