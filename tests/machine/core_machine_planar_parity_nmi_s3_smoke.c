#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"

#include "../support/core_machine_cpu_fixture.h"

int main(void)
{
    core_machine_config config = {0};
    core_machine_planar_parity_config parity = {CORE_MACHINE_PC_AT_PORT_B};
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
            0x000ffff0u, 64u) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_bus_read(machine, 0x0061u, &value) != TYPE_STATUS_OK ||
        (value & 0x84u) != 0x04u ||
        core_machine_bus_write(machine, 0x0070u, 0x80u) != TYPE_STATUS_OK ||
        core_machine_report_planar_parity_fault(machine) != TYPE_STATUS_OK ||
        core_machine_get_planar_parity_observation(machine, &observation) !=
            TYPE_STATUS_OK || !observation.latched || observation.nmi_signaled ||
        core_machine_bus_write(machine, 0x0070u, 0u) != TYPE_STATUS_OK ||
        core_machine_get_planar_parity_observation(machine, &observation) !=
            TYPE_STATUS_OK || !observation.nmi_signaled ||
        core_machine_bus_read(machine, 0x0061u, &value) != TYPE_STATUS_OK ||
        (value & 0x84u) != 0x84u ||
        core_machine_bus_write(machine, 0x0061u, 0u) != TYPE_STATUS_OK ||
        core_machine_bus_write(machine, 0x0061u, 0x04u) != TYPE_STATUS_OK ||
        core_machine_get_planar_parity_observation(machine, &observation) !=
            TYPE_STATUS_OK || observation.latched || observation.nmi_signaled ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_get_planar_parity_observation(machine, &observation) !=
            TYPE_STATUS_OK || !observation.enabled || observation.latched;
    core_machine_destroy(machine);
    if (failed) return 1;
    STD_PRINTF("M5:T366:S3:PLANAR-PARITY-NMI:OK\n");
    return 0;
}
