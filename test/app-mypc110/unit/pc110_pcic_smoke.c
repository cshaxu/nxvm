#include "app-mypc110/pc110_pcic_interface.h"
#include "lib/types/test.h"

int main(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 65536u, .cpu_profile = X86_CORE_CPU_PROFILE_80486SX};
    x86_core_machine *machine = LIB_NULL;
    x86_rb5c396 *controller = LIB_NULL;
    x86_nxvm_board_pic_bus *master = LIB_NULL, *slave = LIB_NULL;
    mypc110_pcic pcic = {0};
    lib_u32 value = 0u;
    const x86_rb5c396_slot_inputs present = {.card_present = LIB_TRUE};
    const x86_rb5c396_slot_inputs absent = {0};
    int result = 0;

    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        x86_rb5c396_create(&controller) != LIB_STATUS_OK ||
        x86_nxvm_board_pic_initialize(&master, &slave, machine,
            X86_NXVM_BOARD_PIC_TOPOLOGY_CASCADED) != LIB_STATUS_OK ||
        mypc110_pcic_initialize(&pcic, controller, master, slave) != LIB_STATUS_OK ||
        mypc110_pcic_install_routes(machine, &pcic) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK) result = 1;
    else if (x86_core_machine_bus_write(machine, X86_RB5C396_INDEX_PORT, 0u) !=
            LIB_STATUS_OK || x86_core_machine_bus_read(machine,
            X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
            value != X86_RB5C396_IDENTIFICATION_VALUE) result = 2;
    else if (x86_core_machine_bus_write(machine, X86_RB5C396_INDEX_PORT, 0x7au) !=
            LIB_STATUS_OK || x86_core_machine_bus_read(machine,
            X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
            value != X86_RB5C396_CHIP_IDENTIFICATION_VALUE) result = 3;
    else if (x86_core_machine_bus_write(machine, X86_RB5C396_DATA_PORT, 0x55u) !=
            LIB_STATUS_OK || x86_core_machine_bus_read(machine,
            X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
            value != X86_RB5C396_CHIP_IDENTIFICATION_VALUE) result = 4;
    else if (x86_core_machine_bus_write(machine, X86_RB5C396_INDEX_PORT, 0x18u) !=
            LIB_STATUS_OK || x86_core_machine_bus_write(machine,
            X86_RB5C396_DATA_PORT, 0x5au) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, X86_RB5C396_DATA_PORT, &value) !=
            LIB_STATUS_OK || value != 0x5au) result = 5;
    else if (mypc110_pcic_set_slot_inputs(&pcic, 0u, &present) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, X86_RB5C396_INDEX_PORT, 0x04u) !=
            LIB_STATUS_OK || x86_core_machine_bus_read(machine,
            X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK || value != 0x08u ||
            x86_core_machine_bus_read(machine, X86_RB5C396_DATA_PORT, &value) !=
            LIB_STATUS_OK || value != 0u) result = 6;
    else if (x86_core_machine_bus_write(machine, 0x20u, 0x11u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x21u, 0x08u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x21u, 0x04u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x21u, 0x01u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0xa0u, 0x11u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0xa1u, 0x70u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0xa1u, 0x02u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0xa1u, 0x01u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, X86_RB5C396_INDEX_PORT, 0x05u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, X86_RB5C396_DATA_PORT, 0x58u) != LIB_STATUS_OK ||
            mypc110_pcic_set_slot_inputs(&pcic, 0u, &absent) != LIB_STATUS_OK ||
            !x86_nxvm_board_pic_irq_source_is_asserted(pcic.irq[0]) ||
            x86_nxvm_board_pic_get_interrupt(master, slave) != 0x0du ||
            x86_core_machine_bus_write(machine, X86_RB5C396_INDEX_PORT, 0x04u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
            value != 0x08u || x86_nxvm_board_pic_irq_source_is_asserted(pcic.irq[0])) result = 7;
    else if (x86_core_machine_bus_write(machine, 0x20u, 0x20u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, X86_RB5C396_INDEX_PORT, 0x45u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, X86_RB5C396_DATA_PORT, 0x58u) != LIB_STATUS_OK ||
            mypc110_pcic_set_slot_inputs(&pcic, 0u, &present) != LIB_STATUS_OK ||
            mypc110_pcic_set_slot_inputs(&pcic, 1u, &present) != LIB_STATUS_OK ||
            !x86_nxvm_board_pic_irq_source_is_asserted(pcic.irq[0]) ||
            !x86_nxvm_board_pic_irq_source_is_asserted(pcic.irq[1]) ||
            x86_nxvm_board_pic_get_interrupt(master, slave) != 0x0du ||
            x86_core_machine_bus_write(machine, X86_RB5C396_INDEX_PORT, 0x04u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
            value != 0x08u || x86_nxvm_board_pic_irq_source_is_asserted(pcic.irq[0]) ||
            !x86_nxvm_board_pic_irq_source_is_asserted(pcic.irq[1]) ||
            x86_core_machine_bus_write(machine, X86_RB5C396_INDEX_PORT, 0x44u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
            value != 0x08u || x86_nxvm_board_pic_irq_source_is_asserted(pcic.irq[1])) result = 8;
    else if (x86_core_machine_bus_write(machine, 0x20u, 0x20u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, X86_RB5C396_INDEX_PORT, 0x05u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, X86_RB5C396_DATA_PORT, 0xa8u) != LIB_STATUS_OK ||
            mypc110_pcic_set_slot_inputs(&pcic, 0u, &absent) != LIB_STATUS_OK ||
            pcic.selected_irq[0] != 10u ||
            x86_nxvm_board_pic_get_interrupt(master, slave) != 0x72u ||
            x86_core_machine_bus_write(machine, X86_RB5C396_DATA_PORT, 0x58u) != LIB_STATUS_OK ||
            pcic.selected_irq[0] != 5u ||
            !x86_nxvm_board_pic_irq_source_is_asserted(pcic.irq[0]) ||
            x86_core_machine_bus_write(machine, X86_RB5C396_DATA_PORT, 0u) != LIB_STATUS_OK ||
            x86_nxvm_board_pic_irq_source_is_asserted(pcic.irq[0])) result = 9;
    else if (x86_core_machine_bus_write(machine, X86_RB5C396_DATA_PORT, 0x58u) != LIB_STATUS_OK ||
            !x86_nxvm_board_pic_irq_source_is_asserted(pcic.irq[0])) result = 10;
    mypc110_pcic_reset(&pcic);
    if (x86_nxvm_board_pic_irq_source_is_asserted(pcic.irq[0]) ||
        x86_nxvm_board_pic_irq_source_is_asserted(pcic.irq[1])) result = 11;
    x86_nxvm_board_pic_finalize(master, slave);
    x86_rb5c396_destroy(controller);
    x86_core_machine_destroy(machine);
    return result;
}
