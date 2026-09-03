#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"

#include "../support/core_machine_cpu_fixture.h"

static C_INT planar_parity_s4_shared_memory(C_VOID)
{
    core_machine_config config = {0};
    core_machine_planar_parity_config parity = {
        .port = CORE_MACHINE_PC_AT_PORT_B,
        .memory_bytes = 512u * 1024u,
        .refresh_status_source = CORE_MACHINE_PLANAR_PARITY_REFRESH_STATUS_PIT_COUNTER_1,
        .refresh_status_toggle_ticks = 0u
    };
    core_machine_planar_parity_observation observation;
    core_machine *machine = STD_NULL;
    type_unsigned_8 written = 0x5au;
    type_unsigned_8 read = 0u;
    C_INT failed = 0;

    config.memory_bytes = 512u * 1024u;
    if (core_machine_create(&config, &machine) != TYPE_STATUS_OK) failed = 1;
    else if (core_machine_configure_planar_parity(machine, &parity) != TYPE_STATUS_OK) failed = 2;
    else if (core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK) failed = 3;
    else if (core_machine_reset(machine) != TYPE_STATUS_OK) failed = 4;
    if (!failed) failed |= core_machine_memory_write(machine, 0x1234u, &written,
            sizeof(written)) != TYPE_STATUS_OK ||
        machine->executor_memory.connect.parity == 0u ||
        core_machine_reconfigure_memory(machine, 512u * 1024u) != TYPE_STATUS_INVALID_STATE;
    if (!failed) ((type_unsigned_8 *)machine->executor_memory.connect.parity)[0x1234u] ^= 1u;
    if (!failed) failed |= core_machine_memory_read(machine, 0x1234u, &read,
            sizeof(read)) != TYPE_STATUS_OK || read != written ||
        core_machine_get_planar_parity_observation(machine, &observation) !=
            TYPE_STATUS_OK || !observation.latched || !observation.nmi_signaled;
    core_machine_destroy(machine);
    return failed;
}

static C_INT planar_parity_s4_unbound_reconfigure(C_VOID)
{
    core_machine_config config = {0};
    core_machine *machine = STD_NULL;
    C_INT failed = 0;

    config.memory_bytes = 2u * 1024u * 1024u;
    if (core_machine_create(&config, &machine) != TYPE_STATUS_OK) failed = 1;
    else if (core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK) failed = 2;
    else if (core_machine_reset(machine) != TYPE_STATUS_OK) failed = 3;
    else if (core_machine_reconfigure_memory(machine, 512u * 1024u) != TYPE_STATUS_INVALID_STATE) failed = 4;
    core_machine_destroy(machine);
    return failed;
}

C_INT main(C_VOID)
{
    core_machine_config config = {0};
    core_machine_planar_parity_config parity = {
        .port = CORE_MACHINE_PC_AT_PORT_B,
        .memory_bytes = 512u * 1024u,
        .refresh_status_source = CORE_MACHINE_PLANAR_PARITY_REFRESH_STATUS_PIT_COUNTER_1,
        .refresh_status_toggle_ticks = 0u
    };
    core_machine_rtc_cmos_config cmos = {0};
    core_machine_planar_parity_observation observation;
    core_machine *machine = STD_NULL;
    type_unsigned_32 value = 0u;
    C_INT failed = 0;

    config.ticks_per_instruction = 1u;
    cmos.index_port = 0x0070u;
    cmos.data_port = 0x0071u;
    cmos.irq = 8u;
    cmos.nmi_mask_bit = 0x80u;
    cmos.ticks_per_second = 1u;
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        core_machine_configure_planar_parity(machine, &parity) != TYPE_STATUS_OK ||
        core_machine_configure_rtc_cmos(machine, &cmos) != TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
            0x000ffff0u, 16u) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_bus_read(machine, 0x0061u, &value) != TYPE_STATUS_OK ||
        (value & 0x94u) != 0x14u ||
        (core_machine_pit_advance(&machine->shared_pit, 19u),
         core_machine_bus_read(machine, 0x0061u, &value) != TYPE_STATUS_OK) ||
        (value & 0x10u) != 0u ||

        core_machine_bus_write(machine, 0x0070u, 0x80u) != TYPE_STATUS_OK ||
        core_machine_report_planar_parity_fault(machine) != TYPE_STATUS_OK ||
        core_machine_get_planar_parity_observation(machine, &observation) !=
            TYPE_STATUS_OK || !observation.latched || observation.nmi_signaled ||
        core_machine_bus_write(machine, 0x0070u, 0u) != TYPE_STATUS_OK ||
        core_machine_get_planar_parity_observation(machine, &observation) !=
            TYPE_STATUS_OK || !observation.nmi_signaled ||
        core_machine_bus_read(machine, 0x0061u, &value) != TYPE_STATUS_OK ||
        (value & 0x84u) != 0x84u ||
        core_machine_bus_write(machine, 0x0061u, 0xc4u) != TYPE_STATUS_OK ||
        core_machine_bus_read(machine, 0x0061u, &value) != TYPE_STATUS_OK ||
        (value & 0xc0u) != 0x80u ||
        core_machine_bus_write(machine, 0x0061u, 0u) != TYPE_STATUS_OK ||
        core_machine_bus_write(machine, 0x0061u, 0x04u) != TYPE_STATUS_OK ||
        core_machine_get_planar_parity_observation(machine, &observation) !=
            TYPE_STATUS_OK || observation.latched || observation.nmi_signaled ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_get_planar_parity_observation(machine, &observation) !=
            TYPE_STATUS_OK || !observation.enabled || observation.latched;
    core_machine_destroy(machine);
    if (failed) return 1;
    failed |= planar_parity_s4_shared_memory() || planar_parity_s4_unbound_reconfigure();
    if (failed) return 1;
    STD_PRINTF("M5:T366:S4:PLANAR-MEMORY-PARITY:OK\n");
    return 0;
}
