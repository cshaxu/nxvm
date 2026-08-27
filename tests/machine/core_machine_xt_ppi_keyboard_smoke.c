#include "type.h"

#include "core/machine/machine.h"

static C_INT core_machine_xt_ppi_keyboard_path(C_VOID)
{
    const core_machine_config configuration = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .keyboard_topology = CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI,
        .xt_ppi_keyboard = {0x0060u, 0x0061u, 0x0062u, 0x0063u, 1u}
    };
    core_machine *machine = STD_NULL;
    type_unsigned_32 value = 0u;
    type_unsigned_8 scan_set = 0u;
    core_machine_speaker_observation speaker;
    C_INT failed = 0;

    failed |= core_machine_create(&configuration, &machine) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_bus_read(machine, 0x0064u, &value) !=
        TYPE_STATUS_UNSUPPORTED;
    failed |= !failed && core_machine_keyboard_get_native_scan_set(machine,
        &scan_set) != TYPE_STATUS_OK;
    failed |= !failed && scan_set != CORE_MACHINE_KEYBOARD_SCAN_SET_1;
    failed |= !failed && core_machine_bus_write(machine, 0x0063u, 0x99u) !=
        TYPE_STATUS_OK;
    failed |= !failed && (core_machine_get_speaker_observation(machine, &speaker) !=
        TYPE_STATUS_OK || !speaker.configured || speaker.timer_gate ||
        speaker.data_enabled || speaker.output);
    failed |= !failed && (core_machine_bus_write(machine, 0x0061u, 0x02u) !=
        TYPE_STATUS_OK || core_machine_get_speaker_observation(machine, &speaker) !=
        TYPE_STATUS_OK || speaker.timer_gate || !speaker.data_enabled ||
        !speaker.output);
    failed |= !failed && (core_machine_bus_write(machine, 0x0061u, 0x03u) !=
        TYPE_STATUS_OK || core_machine_get_speaker_observation(machine, &speaker) !=
        TYPE_STATUS_OK || !speaker.timer_gate || !speaker.data_enabled);
    failed |= !failed && (core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_get_speaker_observation(machine, &speaker) != TYPE_STATUS_OK ||
        !speaker.configured || speaker.timer_gate || speaker.data_enabled ||
        speaker.output);
    failed |= !failed && core_machine_bus_read(machine, 0x0063u, &value) !=
        TYPE_STATUS_UNSUPPORTED;
    failed |= !failed && core_machine_bus_write(machine, 0x0063u, 0x98u) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_bus_write(machine, 0x0063u, 0x01u) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_bus_read(machine, 0x0062u, &value) !=
        TYPE_STATUS_OK;
    failed |= !failed && value != 0x01u;
    failed |= !failed && core_machine_bus_write(machine, 0x0063u, 0x99u) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0x40u) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_keyboard_receive_native_byte(machine, 0x1eu) !=
        TYPE_STATUS_OK;
    failed |= !failed && !machine->xt_ppi_keyboard.byte_ready;
    failed |= !failed && !machine->xt_ppi_keyboard.irq1_asserted;
    failed |= !failed && core_machine_bus_read(machine, 0x0060u, &value) !=
        TYPE_STATUS_OK;
    failed |= !failed && value != 0x1eu;
    /* IBM's IRQ handler clears the PPI shift register through PB7; reading
     * PA alone must not create a second consumption rule. */
    failed |= !failed && core_machine_bus_read(machine, 0x0060u, &value) !=
        TYPE_STATUS_OK;
    failed |= !failed && value != 0x1eu;
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0xc0u) !=
        TYPE_STATUS_OK;
    failed |= !failed && machine->xt_ppi_keyboard.byte_ready;
    failed |= !failed && machine->xt_ppi_keyboard.irq1_asserted;
    failed |= !failed && core_machine_keyboard_receive_native_byte(machine, 0x9eu) !=
        TYPE_STATUS_OK;
    failed |= !failed && machine->xt_ppi_keyboard.byte_ready;
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0x40u) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_bus_read(machine, 0x0060u, &value) !=
        TYPE_STATUS_OK;
    failed |= !failed && value != 0x9eu;
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0xc0u) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0x00u) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_keyboard_receive_native_byte(machine, 0xaau) !=
        TYPE_STATUS_OK;
    failed |= !failed && machine->xt_ppi_keyboard.byte_ready;
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0x40u) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_bus_read(machine, 0x0060u, &value) !=
        TYPE_STATUS_OK;
    failed |= !failed && value != 0xaau;
    core_machine_destroy(machine);
    return failed;
}

static C_INT core_machine_xt_ppi_does_not_change_at_8042(C_VOID)
{
    const core_machine_config configuration = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES
    };
    core_machine *machine = STD_NULL;
    type_unsigned_32 value = 0u;
    type_unsigned_8 scan_set = 0u;
    C_INT failed = 0;

    failed |= core_machine_create(&configuration, &machine) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_bus_read(machine, 0x0064u, &value) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_keyboard_get_native_scan_set(machine,
        &scan_set) != TYPE_STATUS_OK;
    failed |= !failed && scan_set != CORE_MACHINE_KEYBOARD_SCAN_SET_2;
    core_machine_destroy(machine);
    return failed;
}

static C_INT core_machine_xt_ppi_parity_nmi_path(C_VOID)
{
    const core_machine_config configuration = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .keyboard_topology = CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI,
        .xt_ppi_keyboard = {0x0060u, 0x0061u, 0x0062u, 0x0063u, 1u}
    };
    core_machine *machine = STD_NULL;
    type_unsigned_32 value = 0u;
    C_INT failed = 0;

    failed |= core_machine_create(&configuration, &machine) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_bus_write(machine, 0x0063u, 0x99u) !=
        TYPE_STATUS_OK;
    /* PB4/PB5 are active-low parity/I/O-check enables. A live PC7 status line
     * remains observable while PB4 suppresses its NMI request. */
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0x30u) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_set_xt_ppi_fault_input(machine,
        CORE_MACHINE_XT_PPI_FAULT_RAM_PARITY, TYPE_TRUE) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_bus_read(machine, 0x0062u, &value) !=
        TYPE_STATUS_OK;
    failed |= !failed && value != 0x80u;
    failed |= !failed && machine->executor_cpu.data.flagNMI;
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0x20u) !=
        TYPE_STATUS_OK;
    failed |= !failed && !machine->executor_cpu.data.flagNMI;
    failed |= !failed && !machine->xt_ppi_keyboard.nmi_signaled;
    failed |= !failed && core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_bus_write(machine, 0x0063u, 0x99u) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0x30u) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_set_nmi_mask(machine, TYPE_TRUE) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_set_xt_ppi_fault_input(machine,
        CORE_MACHINE_XT_PPI_FAULT_IO_CHECK, TYPE_TRUE) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_bus_read(machine, 0x0062u, &value) !=
        TYPE_STATUS_OK;
    failed |= !failed && value != 0x40u;
    failed |= !failed && core_machine_bus_write(machine, 0x0061u, 0x10u) !=
        TYPE_STATUS_OK;
    failed |= !failed && machine->executor_cpu.data.flagNMI;
    failed |= !failed && core_machine_set_nmi_mask(machine, TYPE_FALSE) != TYPE_STATUS_OK;
    failed |= !failed && !machine->executor_cpu.data.flagNMI;
    failed |= !failed && core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_bus_write(machine, 0x0063u, 0x99u) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_bus_read(machine, 0x0062u, &value) !=
        TYPE_STATUS_OK;
    failed |= !failed && value != 0u;
    core_machine_destroy(machine);
    return failed;
}

int main(void)
{
    if (core_machine_xt_ppi_keyboard_path() ||
        core_machine_xt_ppi_does_not_change_at_8042() ||
        core_machine_xt_ppi_parity_nmi_path()) return 1;
    STD_PRINTF("M5:T484:S8:XT-PPI-KEYBOARD:OK\n");
    STD_PRINTF("M5:T484:S8:XT-IRQ1-RESET:OK\n");
    STD_PRINTF("M5:T484:S8:NO-8042-ALIAS:OK\n");
    STD_PRINTF("M5:T484:S19:XT-PPI-PARITY:OK\n");
    return 0;
}
