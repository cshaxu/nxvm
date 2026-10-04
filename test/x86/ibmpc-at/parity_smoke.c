#include "x86/ibmpc-at/parity_interface.h"
#include "x86/core/port_interface.h"

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
    core_machine_planar_parity_refresh_status_source source)
{
    const core_machine_executor_config executor = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086
    };
    const core_machine_planar_parity_config config = {
        .port = 0x61u, .memory_bytes = memory_present ? 0x10000u : 0u,
        .refresh_status_source = source, .refresh_status_toggle_ticks = 8u
    };
    core_machine *machine = LIB_NULL;
    core_machine_at_parity *parity = LIB_NULL;
    x86_pit *pit = LIB_NULL;
    core_machine_planar_parity_observation observation;
    lib_u8 speaker = 0xffu;
    lib_u32 value = 0u;
    lib_i32 failed = 1;
    lib_u8 conflict_owner = 0u;
    const core_machine_port_route conflict = {
        .address = 0x61u, .read = conflict_read, .owner = &conflict_owner
    };

    if (core_machine_neutral_create(&executor, &machine) != LIB_STATUS_OK ||
        x86_pit_create(X86_PIT_PERSONALITY_8254, &pit) != LIB_STATUS_OK ||
        core_machine_install_port_routes(machine, &conflict, 1u) != LIB_STATUS_OK ||
        core_machine_at_parity_create(machine, pit, &config, speaker_lines, &speaker,
            &parity) != LIB_STATUS_INVALID_ARGUMENT || parity != LIB_NULL ||
        core_machine_remove_port_routes(machine, &conflict_owner) != LIB_STATUS_OK ||
        core_machine_at_parity_create(machine, pit, &config, speaker_lines, &speaker,
            &parity) != LIB_STATUS_OK || speaker != 0u ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK) goto done;
    core_machine_at_parity_observe(parity, &observation);
    if (observation.configured != memory_present || observation.enabled != memory_present ||
        observation.latched || observation.nmi_signaled ||
        core_machine_bus_write(machine, 0x61u, 0x07u) != LIB_STATUS_OK ||
        speaker != 0x03u ||
        core_machine_bus_read(machine, 0x61u, &value) != LIB_STATUS_OK ||
        (value & 0x0fu) != 0x07u ||
        core_machine_at_parity_report_fault(parity) !=
            (memory_present ? LIB_STATUS_OK : LIB_STATUS_INVALID_STATE)) goto done;
    core_machine_at_parity_observe(parity, &observation);
    if (observation.latched != memory_present ||
        core_machine_bus_write(machine, 0x61u, 0x03u) != LIB_STATUS_OK) goto done;
    core_machine_at_parity_observe(parity, &observation);
    if (observation.enabled || observation.latched || observation.nmi_signaled) goto done;
    core_machine_at_parity_reset(parity);
    core_machine_at_parity_observe(parity, &observation);
    if (speaker != 0u || observation.enabled != memory_present || observation.latched ||
        core_machine_bus_read(machine, 0x61u, &value) != LIB_STATUS_OK ||
        (value & 0x0fu) != 0x04u) goto done;
    failed = 0;
done:
    core_machine_at_parity_destroy(parity);
    core_machine_destroy(machine);
    x86_pit_destroy(pit);
    return failed;
}

lib_i32 main(void)
{
    lib_i32 failed = 0;
    for (lib_u8 present = 0u; present < 2u; ++present) {
        failed |= check_parity(present, CORE_MACHINE_PLANAR_PARITY_REFRESH_STATUS_PIT_COUNTER_1);
        failed |= check_parity(present, CORE_MACHINE_PLANAR_PARITY_REFRESH_STATUS_ELAPSED_TICK_TOGGLE);
    }
    return failed;
}
