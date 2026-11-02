#include "app-mypc110/pc110_pcic_interface.h"
#include "lib/types/test.h"

static lib_bool write_register(x86_core_machine *machine, lib_u8 index, lib_u8 value)
{
    return x86_core_machine_bus_write(machine, X86_RB5C396_INDEX_PORT, index) == LIB_STATUS_OK &&
        x86_core_machine_bus_write(machine, X86_RB5C396_DATA_PORT, value) == LIB_STATUS_OK;
}

static lib_status lower_read(void *owner, lib_u32 physical, lib_uptr destination,
    lib_uptr bytes, lib_bool observe_only)
{
    (void)observe_only;
    lib_memory_copy((void *)destination, (lib_u8 *)owner + physical - 0xc0000u, bytes);
    return LIB_STATUS_OK;
}

static lib_status lower_write(void *owner, lib_u32 physical, lib_uptr source,
    lib_uptr bytes)
{
    lib_memory_copy((lib_u8 *)owner + physical - 0xc0000u, (const void *)source, bytes);
    return LIB_STATUS_OK;
}

static lib_status lower_query(void *owner, lib_u32 physical, lib_uptr bytes,
    x86_core_machine_memory_access access)
{
    (void)owner; (void)physical; (void)bytes; (void)access;
    return LIB_STATUS_OK;
}

static lib_bool suspend_machine(x86_vl82c420 *chip)
{
    x86_vl82c420_observe_unlock_read(chip, X86_VL82C420_UNLOCK_READ_FC23);
    x86_vl82c420_observe_unlock_read(chip, X86_VL82C420_UNLOCK_READ_F023);
    x86_vl82c420_observe_unlock_read(chip, X86_VL82C420_UNLOCK_READ_C023);
    x86_vl82c420_observe_unlock_read(chip, X86_VL82C420_UNLOCK_READ_0023);
    return x86_vl82c420_select(chip, X86_VL82C420_BANK_BLOCK2, 0xffu) == LIB_STATUS_OK &&
        x86_vl82c420_write_selected(chip, X86_VL82C420_BANK_BLOCK2, 0x82u) == LIB_STATUS_OK &&
        x86_vl82c420_suspend_refresh_enabled(chip);
}

static int wake_smoke(x86_core_machine *machine, mypc110_pcic *pcic)
{
    x86_vl82c420 *chip = LIB_NULL;
    mypc110_firmware firmware = {0};
    x86_rb5c396_slot_inputs input = {0};
    lib_u32 value = 0u;
    int failed = x86_vl82c420_create(&chip) != LIB_STATUS_OK;
    if (failed) return 1;
    firmware.machine = machine;
    firmware.configuration_chip = chip;
    pcic->firmware = &firmware;
    mypc110_pcic_reset(pcic);
    failed |= mypc110_pcic_set_slot_inputs(pcic, 0u, &input) != LIB_STATUS_OK ||
        !write_register(machine, 4u, 0u) ||
        x86_core_machine_bus_read(machine, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
        !write_register(machine, 5u, 8u) || !write_register(machine, 0x16u, 0x10u) ||
        !suspend_machine(chip);
    input.card_present = LIB_TRUE;
    failed |= mypc110_pcic_set_slot_inputs(pcic, 0u, &input) != LIB_STATUS_OK ||
        x86_vl82c420_suspend_refresh_enabled(chip);
    /* A held card indication must not hide a later independent F12 edge. */
    failed |= !suspend_machine(chip) ||
        mypc110_pcic_set_resume_button(pcic, LIB_TRUE) != LIB_STATUS_OK ||
        x86_vl82c420_suspend_refresh_enabled(chip) ||
        mypc110_pcic_set_resume_button(pcic, LIB_FALSE) != LIB_STATUS_OK ||
        !suspend_machine(chip) || !write_register(machine, 4u, 0u) ||
        x86_core_machine_bus_read(machine, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
        !x86_vl82c420_suspend_refresh_enabled(chip);
    input.card_present = LIB_FALSE;
    failed |= mypc110_pcic_set_slot_inputs(pcic, 0u, &input) != LIB_STATUS_OK ||
        x86_vl82c420_suspend_refresh_enabled(chip) || !write_register(machine, 4u, 0u) ||
        x86_core_machine_bus_read(machine, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
        mypc110_pcic_set_resume_button(pcic, LIB_TRUE) != LIB_STATUS_OK ||
        !suspend_machine(chip);
    input.card_present = LIB_TRUE;
    failed |= mypc110_pcic_set_slot_inputs(pcic, 0u, &input) != LIB_STATUS_OK ||
        x86_vl82c420_suspend_refresh_enabled(chip) ||
        mypc110_pcic_set_resume_button(pcic, LIB_FALSE) != LIB_STATUS_OK;
    pcic->firmware = LIB_NULL;
    pcic->resume_button = LIB_FALSE;
    x86_vl82c420_destroy(chip);
    return failed;
}

static int status_pic_smoke(x86_core_machine *machine, mypc110_pcic *pcic,
    x86_nxvm_board_pic_bus *master, x86_nxvm_board_pic_bus *slave)
{
    x86_rb5c396_slot_inputs input = {.card_present = LIB_TRUE,
        .battery_detect_1 = LIB_TRUE, .battery_detect_2 = LIB_TRUE};
    lib_u32 value = 0u;
    int failed = x86_core_machine_bus_write(machine, 0x20u, 0x11u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x21u, 0x08u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x21u, 0x04u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x21u, 0x01u) != LIB_STATUS_OK ||
        !write_register(machine, 5u, 0x54u) ||
        mypc110_pcic_set_slot_inputs(pcic, 0u, &input) != LIB_STATUS_OK ||
        !write_register(machine, 4u, 0u) ||
        x86_core_machine_bus_read(machine, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK || value != 8u;
    input.ready = LIB_TRUE;
    if (!failed) failed = mypc110_pcic_set_slot_inputs(pcic, 0u, &input) != LIB_STATUS_OK ||
        !x86_nxvm_board_pic_irq_source_is_asserted(pcic->irq[0]) ||
        x86_nxvm_board_pic_get_interrupt(master, slave) != 0x0du ||
        x86_core_machine_bus_read(machine, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK || value != 4u ||
        x86_nxvm_board_pic_irq_source_is_asserted(pcic->irq[0]);
    input.io_irq_asserted = LIB_TRUE;
    input.io_status_asserted = LIB_TRUE;
    if (!failed) failed = x86_core_machine_bus_write(machine, 0x20u, 0x20u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0xa0u, 0x11u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0xa1u, 0x70u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0xa1u, 0x02u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0xa1u, 0x01u) != LIB_STATUS_OK ||
        !write_register(machine, 3u, 0x65u) || !write_register(machine, 5u, 0xa1u) ||
        mypc110_pcic_set_slot_inputs(pcic, 0u, &input) != LIB_STATUS_OK ||
        !x86_nxvm_board_pic_irq_source_is_asserted(pcic->irq[0]) ||
        !x86_nxvm_board_pic_irq_source_is_asserted(pcic->io_irq[0]) ||
        x86_nxvm_board_pic_get_interrupt(master, slave) != 0x72u ||
        x86_core_machine_bus_write(machine, 0xa0u, 0x20u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x20u, 0x20u) != LIB_STATUS_OK ||
        x86_nxvm_board_pic_get_interrupt(master, slave) != 0x0du ||
        !write_register(machine, 4u, 0u) ||
        x86_core_machine_bus_read(machine, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK || value != 1u ||
        x86_nxvm_board_pic_irq_source_is_asserted(pcic->irq[0]) ||
        !x86_nxvm_board_pic_irq_source_is_asserted(pcic->io_irq[0]);
    input.io_status_asserted = LIB_FALSE;
    if (!failed) failed = mypc110_pcic_set_slot_inputs(pcic, 0u, &input) != LIB_STATUS_OK ||
        !write_register(machine, 5u, 0x51u);
    input.io_status_asserted = LIB_TRUE;
    if (!failed) failed = mypc110_pcic_set_slot_inputs(pcic, 0u, &input) != LIB_STATUS_OK ||
        pcic->selected_irq[0] != 5u || pcic->selected_io_irq[0] != 5u ||
        !x86_nxvm_board_pic_irq_source_is_asserted(pcic->irq[0]) ||
        !x86_nxvm_board_pic_irq_source_is_asserted(pcic->io_irq[0]) ||
        !write_register(machine, 4u, 0u) ||
        x86_core_machine_bus_read(machine, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK || value != 1u ||
        x86_nxvm_board_pic_irq_source_is_asserted(pcic->irq[0]) ||
        !x86_nxvm_board_pic_irq_source_is_asserted(pcic->io_irq[0]);
    input.io_irq_asserted = LIB_FALSE;
    if (!failed) failed = mypc110_pcic_set_slot_inputs(pcic, 0u, &input) != LIB_STATUS_OK ||
        x86_nxvm_board_pic_irq_source_is_asserted(pcic->io_irq[0]);
    return failed;
}

static int memory_card_smoke(x86_core_machine *machine, mypc110_pcic *pcic, lib_u8 *lower)
{
    lib_storage_medium *common = LIB_NULL, *attribute = LIB_NULL, *second = LIB_NULL;
    lib_u8 data = 0x5au, actual = 0u;
    int failed = 0;

    if (lib_storage_medium_create_zero_overlay(8192u, &common) != LIB_STATUS_OK ||
        lib_storage_medium_create_zero_overlay(8192u, &attribute) != LIB_STATUS_OK ||
        lib_storage_medium_create_zero_overlay(8192u, &second) != LIB_STATUS_OK) {
        failed = 1;
        goto done;
    }
    failed |= lib_storage_medium_write_at(common, 0u, &data, 1u) != LIB_STATUS_OK;
    failed |= mypc110_pcic_set_memory_card(pcic, 0u, common, attribute, LIB_FALSE) != LIB_STATUS_OK;
    failed |= !write_register(machine, 0x10u, 0xc0u) ||
        !write_register(machine, 0x11u, 0x80u) ||
        !write_register(machine, 0x12u, 0xc0u) ||
        !write_register(machine, 0x13u, 0u) ||
        !write_register(machine, 0x14u, 0x40u) ||
        !write_register(machine, 0x15u, 0x3fu) ||
        !write_register(machine, 0x06u, 1u);
    failed |= x86_core_machine_memory_read(machine, 0xc0000u, &actual, 1u) != LIB_STATUS_OK ||
        actual != data;
    data = 0xa5u;
    failed |= x86_core_machine_memory_write(machine, 0xc0000u, &data, 1u) != LIB_STATUS_OK ||
        lib_storage_medium_read_at(common, 0u, &actual, 1u) != LIB_STATUS_OK || actual != data;
    {
        const lib_u8 pair[2] = {0x81u, 0x82u};
        lib_u8 observed[2] = {0u, 0u};
        failed |= x86_core_machine_memory_write(machine, 0xc0fffu, pair, 2u) != LIB_STATUS_OK ||
            x86_core_machine_memory_read(machine, 0xc0fffu, observed, 2u) != LIB_STATUS_OK ||
            lib_memory_compare(pair, observed, 2u) != 0 || lower[0x1000u] != pair[1] ||
            lib_storage_medium_read_at(common, 0xfffu, &actual, 1u) != LIB_STATUS_OK || actual != pair[0];
    }
    failed |= !write_register(machine, 0x15u, 0x7fu) ||
        x86_core_machine_memory_read(machine, 0xc0000u, &actual, 1u) != LIB_STATUS_OK || actual != 0u;
    data = 0x33u;
    failed |= x86_core_machine_memory_write(machine, 0xc0000u, &data, 1u) != LIB_STATUS_OK ||
        lib_storage_medium_read_at(attribute, 0u, &actual, 1u) != LIB_STATUS_OK || actual != data;
    failed |= !write_register(machine, 0x15u, 0xffu);
    data = 0x44u;
    failed |= x86_core_machine_memory_write(machine, 0xc0000u, &data, 1u) != LIB_STATUS_OK ||
        lib_storage_medium_read_at(attribute, 0u, &actual, 1u) != LIB_STATUS_OK || actual != 0x33u;
    {
        const x86_rb5c396_slot_inputs absent = {0};
        const x86_rb5c396_slot_inputs present = {.card_present = LIB_TRUE, .ready = LIB_TRUE};
        failed |= mypc110_pcic_set_slot_inputs(pcic, 0u, &absent) != LIB_STATUS_OK ||
            pcic->range_bytes != 0u ||
            x86_core_machine_memory_read(machine, 0xc0000u, &actual, 1u) != LIB_STATUS_OK ||
            actual != lower[0] ||
            mypc110_pcic_set_slot_inputs(pcic, 0u, &present) != LIB_STATUS_OK ||
            x86_core_machine_memory_read(machine, 0xc0000u, &actual, 1u) != LIB_STATUS_OK ||
            actual != 0x33u;
    }
    data = 0x77u;
    failed |= lib_storage_medium_write_at(second, 0u, &data, 1u) != LIB_STATUS_OK ||
        mypc110_pcic_set_memory_card(pcic, 1u, second, LIB_NULL, LIB_TRUE) != LIB_STATUS_OK ||
        !write_register(machine, 0x50u, 0xd0u) || !write_register(machine, 0x51u, 0u) ||
        !write_register(machine, 0x52u, 0xd0u) || !write_register(machine, 0x53u, 0u) ||
        !write_register(machine, 0x54u, 0x30u) || !write_register(machine, 0x55u, 0x3fu) ||
        !write_register(machine, 0x46u, 1u) ||
        x86_core_machine_memory_read(machine, 0xd0000u, &actual, 1u) != LIB_STATUS_OK || actual != data;
    data = 0x99u;
    failed |= x86_core_machine_memory_write(machine, 0xd0000u, &data, 1u) != LIB_STATUS_OK ||
        lib_storage_medium_read_at(second, 0u, &actual, 1u) != LIB_STATUS_OK || actual != 0x77u;
    mypc110_pcic_reset(pcic);
    failed |= pcic->range_bytes != 0u ||
        x86_core_machine_memory_read(machine, 0xc0000u, &actual, 1u) != LIB_STATUS_OK ||
        actual != lower[0];
    failed |= mypc110_pcic_set_memory_card(pcic, 0u, LIB_NULL, LIB_NULL, LIB_FALSE) != LIB_STATUS_OK;
    failed |= mypc110_pcic_set_memory_card(pcic, 1u, LIB_NULL, LIB_NULL, LIB_FALSE) != LIB_STATUS_OK;
    failed |= pcic->range_bytes != 0u;
done:
    (void)mypc110_pcic_set_memory_card(pcic, 0u, LIB_NULL, LIB_NULL, LIB_FALSE);
    (void)mypc110_pcic_set_memory_card(pcic, 1u, LIB_NULL, LIB_NULL, LIB_FALSE);
    if (common != LIB_NULL) (void)lib_storage_medium_destroy(&common);
    if (attribute != LIB_NULL) (void)lib_storage_medium_destroy(&attribute);
    if (second != LIB_NULL) (void)lib_storage_medium_destroy(&second);
    return failed;
}

int main(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 2u * 1024u * 1024u, .cpu_profile = X86_CORE_CPU_PROFILE_80486SX};
    x86_core_machine *machine = LIB_NULL;
    x86_rb5c396 *controller = LIB_NULL;
    x86_nxvm_board_pic_bus *master = LIB_NULL, *slave = LIB_NULL;
    mypc110_pcic pcic = {0};
    lib_u8 lower[8192] = {0};
    const x86_core_machine_memory_device_route lower_route = {
        0xc0000u, sizeof(lower), {lower_read, lower_write, lower_query},
        X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT, LIB_NULL};
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
        x86_core_machine_install_memory_device_routes(machine, &lower_route, 1u,
            LIB_NULL, LIB_NULL, lower) != LIB_STATUS_OK ||
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
    if (result == 0 && memory_card_smoke(machine, &pcic, lower)) result = 12;
    mypc110_pcic_reset(&pcic);
    if (x86_nxvm_board_pic_irq_source_is_asserted(pcic.irq[0]) ||
        x86_nxvm_board_pic_irq_source_is_asserted(pcic.irq[1])) result = 11;
    if (result == 0 && status_pic_smoke(machine, &pcic, master, slave)) result = 13;
    if (result == 0 && wake_smoke(machine, &pcic)) result = 14;
    x86_nxvm_board_pic_finalize(master, slave);
    x86_rb5c396_destroy(controller);
    x86_core_machine_destroy(machine);
    return result;
}
