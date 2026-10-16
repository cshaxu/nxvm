#include "x86-nxvm/board-at/parity_interface.h"
#include "x86-core/machine/port_interface.h"
#include "x86-core/machine/memory_interface.h"

static x86_core_machine_memory_parity_fault_observer registered_fault;
static void *registered_owner;

static lib_status parity_install_memory(x86_core_machine *machine,
    const x86_core_machine_memory_device_route *routes, lib_size count,
    x86_core_machine_memory_write_observer observer,
    const x86_core_machine_memory_parity_config *parity, void *owner)
{
    const lib_status status = x86_core_machine_install_memory_device_routes(
        machine, routes, count, observer, parity, owner);

    if (status == LIB_STATUS_OK && parity != LIB_NULL) {
        registered_fault = parity->fault;
        registered_owner = owner;
    }
    return status;
}

/* Observe the adapter's real public registration, not Machine's storage. */
#define x86_core_machine_install_memory_device_routes parity_install_memory
#include "x86-nxvm/board-at/parity.c"
#undef x86_core_machine_install_memory_device_routes

static void speaker_lines(void *context, lib_u8 value)
{
    *(lib_u8 *)context = value & 0x03u;
}

static lib_status conflict_read(void *context, lib_u16 port, lib_u64 tick,
    lib_u32 *out_value)
{
    (void)context;
    (void)port;
    (void)tick;
    *out_value = 0x5au;
    return LIB_STATUS_OK;
}

static lib_i32 check_parity(lib_bool memory_present,
    x86_nxvm_board_planar_parity_refresh_status_source source)
{
    const x86_core_machine_executor_config executor = {
        .memory_bytes = 2u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_8086
    };
    const x86_nxvm_board_planar_parity_config config = {
        .port = 0x61u, .memory_bytes = memory_present ? 0x10000u : 0u,
        .refresh_status_source = source, .refresh_status_toggle_ticks = 8u
    };
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_at_parity *parity = LIB_NULL;
    x86_pit *pit = LIB_NULL;
    x86_nxvm_board_planar_parity_observation observation;
    lib_u8 speaker = 0xffu;
    lib_u32 value = 0u;
    lib_i32 failed = 1;
    lib_u8 conflict_owner = 0u;
    const x86_core_machine_port_route conflict = {
        .address = 0x61u, .read = conflict_read, .owner = &conflict_owner
    };

    registered_fault = LIB_NULL;
    registered_owner = LIB_NULL;

    if (x86_core_machine_neutral_create(&executor, &machine) != LIB_STATUS_OK ||
        x86_pit_create(X86_PIT_PERSONALITY_8254, &pit) != LIB_STATUS_OK ||
        x86_core_machine_install_port_routes(machine, &conflict, 1u) != LIB_STATUS_OK ||
        x86_nxvm_board_at_parity_create(machine, pit, &config, speaker_lines, &speaker,
            &parity) != LIB_STATUS_INVALID_ARGUMENT || parity != LIB_NULL ||
        x86_core_machine_remove_port_routes(machine, &conflict_owner) != LIB_STATUS_OK ||
        x86_nxvm_board_at_parity_create(machine, pit, &config, speaker_lines, &speaker,
            &parity) != LIB_STATUS_OK || speaker != 0u ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK) goto done;
    x86_nxvm_board_at_parity_observe(parity, &observation);
    if (observation.configured != memory_present || observation.enabled != memory_present ||
        observation.latched || observation.nmi_signaled ||
        x86_core_machine_bus_write(machine, 0x61u, 0x07u) != LIB_STATUS_OK ||
        speaker != 0x03u ||
        x86_core_machine_bus_read(machine, 0x61u, &value) != LIB_STATUS_OK ||
        (value & 0x0fu) != 0x07u ||
        x86_nxvm_board_at_parity_report_fault(parity) !=
            (memory_present ? LIB_STATUS_OK : LIB_STATUS_INVALID_STATE)) goto done;
    x86_nxvm_board_at_parity_observe(parity, &observation);
    if (observation.latched != memory_present ||
        x86_core_machine_bus_write(machine, 0x61u, 0x03u) != LIB_STATUS_OK) goto done;
    x86_nxvm_board_at_parity_observe(parity, &observation);
    if (observation.enabled || observation.latched || observation.nmi_signaled) goto done;
    if (memory_present) {
        if (registered_fault == LIB_NULL || registered_owner != parity) goto done;
        registered_fault(registered_owner, 0x1234u);
        x86_nxvm_board_at_parity_observe(parity, &observation);
        if (!observation.latched || observation.nmi_signaled) goto done;
    } else if (registered_fault != LIB_NULL || registered_owner != LIB_NULL) goto done;
    x86_nxvm_board_at_parity_reset(parity);
    x86_nxvm_board_at_parity_observe(parity, &observation);
    if (speaker != 0u || observation.enabled != memory_present || observation.latched ||
        x86_core_machine_bus_read(machine, 0x61u, &value) != LIB_STATUS_OK ||
        (value & 0x0fu) != 0x04u) goto done;
    failed = 0;
done:
    x86_nxvm_board_at_parity_destroy(parity);
    x86_core_machine_destroy(machine);
    x86_pit_destroy(pit);
    return failed;
}

static lib_i32 check_port_read_tick(void)
{
    const x86_core_machine_executor_config executor = {
        .memory_bytes = 65536u, .cpu_profile = X86_CORE_CPU_PROFILE_8086
    };
    const x86_nxvm_board_planar_parity_config config = {
        .port = 0x61u,
        .refresh_status_source = X86_NXVM_BOARD_PLANAR_PARITY_REFRESH_STATUS_ELAPSED_TICK_TOGGLE,
        .refresh_status_toggle_ticks = 64u
    };
    const lib_u64 ticks[] = {0u, 63u, 64u, 65u, 127u, 128u, 129u,
        0x100000040ull};
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_at_parity *parity = LIB_NULL;
    x86_pit *pit = LIB_NULL;
    lib_u64 elapsed = 0u;
    lib_i32 failed = 1;

    if (x86_core_machine_neutral_create(&executor, &machine) != LIB_STATUS_OK ||
        x86_pit_create(X86_PIT_PERSONALITY_8254, &pit) != LIB_STATUS_OK ||
        x86_nxvm_board_at_parity_create(machine, pit, &config, LIB_NULL, LIB_NULL,
            &parity) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK) goto done;
    failed = 0;
    for (lib_size index = 0u; index < sizeof(ticks) / sizeof(ticks[0]); ++index) {
        lib_u32 value = 0u;
        /* The real endpoint uses the supplied bus tick, not Machine time. */
        failed |= parity_port_read(parity, config.port, ticks[index], &value) !=
            LIB_STATUS_OK || (value & 0x10u) !=
                (((ticks[index] / 64u) & 1u) ? 0x10u : 0u) ||
            x86_core_machine_get_elapsed_ticks(machine, &elapsed) != LIB_STATUS_OK ||
            elapsed != 0u;
    }
done:
    x86_nxvm_board_at_parity_destroy(parity);
    x86_core_machine_destroy(machine);
    x86_pit_destroy(pit);
    return failed;
}

lib_i32 main(void)
{
    lib_i32 failed = check_port_read_tick();
    for (lib_u8 present = 0u; present < 2u; ++present) {
        failed |= check_parity(present, X86_NXVM_BOARD_PLANAR_PARITY_REFRESH_STATUS_PIT_COUNTER_1);
        failed |= check_parity(present, X86_NXVM_BOARD_PLANAR_PARITY_REFRESH_STATUS_ELAPSED_TICK_TOGGLE);
    }
    return failed;
}
