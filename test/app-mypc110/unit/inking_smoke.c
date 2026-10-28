#include "app-mypc110/pc110_inking_interface.h"

static int check_inking(lib_u8 irq, lib_u16 base)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 65536u, .cpu_profile = X86_CORE_CPU_PROFILE_80486SX};
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_pic_bus *master = LIB_NULL, *slave = LIB_NULL;
    mypc110_inking inking = {0};
    lib_u32 value = 0u;
    lib_bool failed = LIB_FALSE;

    failed |= x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
    if (!failed) failed |= x86_nxvm_board_pic_initialize(&master, &slave, machine,
        X86_NXVM_BOARD_PIC_TOPOLOGY_CASCADED) != LIB_STATUS_OK;
    if (!failed) failed |= x86_nxvm_board_pic_irq_source_bind(&inking.irq,
        master, slave, irq) != LIB_STATUS_OK ||
        mypc110_inking_install_routes(machine, &inking, base) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK;
    if (!failed) {
        failed |= x86_core_machine_bus_write(machine, 0x20u, 0x11u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x21u, 0x08u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x21u, 0x04u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x21u, 0x01u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0xa0u, 0x11u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0xa1u, 0x70u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0xa1u, 0x02u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0xa1u, 0x01u) != LIB_STATUS_OK;
        failed |= x86_core_machine_bus_read(machine, (lib_u16)(base + 1u), &value) != LIB_STATUS_OK ||
            value != 0xffu ||
            mypc110_inking_sample(&inking, 0xabu, 0xcdu, LIB_TRUE) != LIB_STATUS_INVALID_STATE ||
            x86_core_machine_bus_write(machine, (lib_u16)(base + 2u), 0x38u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, (lib_u16)(base + 1u), &value) != LIB_STATUS_OK || value != 0x7fu ||
            mypc110_inking_sample(&inking, 0xabu, 0xcdu, LIB_TRUE) != LIB_STATUS_OK;
        const lib_u8 expected[] = {0x99u, 0x2bu, 0x4du};
        for (lib_size i = 0u; i < sizeof(expected); ++i) {
            failed |= !x86_nxvm_board_pic_irq_source_is_asserted(inking.irq) ||
                x86_nxvm_board_pic_get_interrupt(master, slave) !=
                    (irq == 5u ? 0x0du : 0x72u) ||
                x86_core_machine_bus_read(machine, base, &value) != LIB_STATUS_OK ||
                value != expected[i] ||
                x86_core_machine_bus_write(machine, (lib_u16)(base + 2u), 0x39u) != LIB_STATUS_OK ||
                x86_nxvm_board_pic_irq_source_is_asserted(inking.irq) ||
                x86_core_machine_bus_write(machine, (lib_u16)(base + 2u), 0x38u) != LIB_STATUS_OK;
            if (irq >= 8u)
                failed |= x86_core_machine_bus_write(machine, 0xa0u, 0x20u) != LIB_STATUS_OK;
            failed |= x86_core_machine_bus_write(machine, 0x20u, 0x20u) != LIB_STATUS_OK;
        }
        failed |= inking.pending || x86_nxvm_board_pic_irq_source_is_asserted(inking.irq) ||
            x86_core_machine_bus_read(machine, (lib_u16)(base + 1u), &value) != LIB_STATUS_OK || value != 0x7fu;
        failed |= mypc110_inking_sample(&inking, 0u, 0u, LIB_TRUE) != LIB_STATUS_OK;
        for (lib_u8 i = 0u; i < 8u; ++i)
            failed |= mypc110_inking_sample(&inking, i, i, (i & 1u) != 0u) != LIB_STATUS_OK;
        failed |= mypc110_inking_sample(&inking, 0u, 0u, LIB_FALSE) != LIB_STATUS_LIMIT_EXCEEDED;
        mypc110_inking_reset(&inking);
        failed |= x86_core_machine_bus_write(machine, (lib_u16)(base + 2u), 0x38u) != LIB_STATUS_OK;
        failed |= mypc110_inking_sample(&inking, 1u, 2u, LIB_TRUE) != LIB_STATUS_OK ||
            mypc110_inking_sample(&inking, 2u, 3u, LIB_FALSE) != LIB_STATUS_OK ||
            mypc110_inking_sample(&inking, 3u, 4u, LIB_FALSE) != LIB_STATUS_OK ||
            mypc110_inking_sample(&inking, 4u, 5u, LIB_TRUE) != LIB_STATUS_OK;
        const lib_u8 queued[] = {0x81u, 1u, 2u, 0x80u, 3u, 4u, 0x81u, 4u, 5u};
        for (lib_size i = 0u; i < sizeof(queued); ++i) {
            failed |= x86_nxvm_board_pic_get_interrupt(master, slave) !=
                    (irq == 5u ? 0x0du : 0x72u) ||
                x86_core_machine_bus_read(machine, base, &value) != LIB_STATUS_OK ||
                value != queued[i] ||
                x86_core_machine_bus_write(machine, (lib_u16)(base + 2u), 0x39u) != LIB_STATUS_OK ||
                x86_core_machine_bus_write(machine, (lib_u16)(base + 2u), 0x38u) != LIB_STATUS_OK;
            if (irq >= 8u)
                failed |= x86_core_machine_bus_write(machine, 0xa0u, 0x20u) != LIB_STATUS_OK;
            failed |= x86_core_machine_bus_write(machine, 0x20u, 0x20u) != LIB_STATUS_OK;
        }
        mypc110_inking_reset(&inking);
        failed |= inking.pending || inking.waiting_count != 0u ||
            x86_core_machine_bus_write(machine, (lib_u16)(base + 2u), 0x38u) != LIB_STATUS_OK;
        failed |= mypc110_inking_sample(&inking, 0u, 0u, LIB_FALSE) != LIB_STATUS_OK ||
            !x86_nxvm_board_pic_irq_source_is_asserted(inking.irq) ||
            x86_core_machine_bus_write(machine, (lib_u16)(base + 2u), 0xc0u) != LIB_STATUS_OK ||
            inking.pending || x86_nxvm_board_pic_irq_source_is_asserted(inking.irq) ||
            x86_core_machine_bus_read(machine, (lib_u16)(base + 1u), &value) != LIB_STATUS_OK || value != 0xffu;
    }
    x86_nxvm_board_pic_finalize(master, slave);
    x86_core_machine_destroy(machine);
    return failed ? 1 : 0;
}

static int check_selected_wiring(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 65536u, .cpu_profile = X86_CORE_CPU_PROFILE_80486SX};
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_pic_bus *master = LIB_NULL, *slave = LIB_NULL;
    x86_vl82c420 *chip = LIB_NULL;
    mypc110_inking inking = {0};
    lib_u32 value = 0u;
    lib_bool failed = LIB_FALSE;
    failed |= x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
    if (!failed) failed |= x86_nxvm_board_pic_initialize(&master, &slave, machine,
        X86_NXVM_BOARD_PIC_TOPOLOGY_CASCADED) != LIB_STATUS_OK ||
        x86_vl82c420_create(&chip) != LIB_STATUS_OK;
    if (!failed) {
        inking.configuration = chip;
        inking.pic_master = master;
        inking.pic_slave = slave;
        failed |= x86_vl82c420_select(chip, X86_VL82C420_BANK_SCAMP, 0x16u) != LIB_STATUS_OK ||
            x86_vl82c420_write_selected(chip, X86_VL82C420_BANK_SCAMP, 0xa1u) != LIB_STATUS_OK ||
            mypc110_inking_install_routes(machine, &inking, 0x15e0u) != LIB_STATUS_OK ||
            x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            x86_core_machine_reset(machine) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x20u, 0x11u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x21u, 0x08u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x21u, 0x04u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x21u, 0x01u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0xa0u, 0x11u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0xa1u, 0x70u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0xa1u, 0x02u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0xa1u, 0x01u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x15e2u, &value) != LIB_STATUS_OK || value != 0xc0u ||
            x86_core_machine_bus_read(machine, 0x25e2u, &value) != LIB_STATUS_OK || value != 0xffu ||
            x86_core_machine_bus_write(machine, 0x25e2u, 0x100u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x15e2u, 0x100u) != LIB_STATUS_INVALID_ARGUMENT ||
            x86_core_machine_bus_write(machine, 0x15e2u, 0x38u) != LIB_STATUS_OK ||
            mypc110_inking_sample(&inking, 1u, 2u, LIB_TRUE) != LIB_STATUS_OK ||
            x86_nxvm_board_pic_get_interrupt(master, slave) != 0x72u;
        mypc110_inking_reset(&inking);
        failed |= x86_core_machine_bus_write(machine, 0xa0u, 0x20u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x20u, 0x20u) != LIB_STATUS_OK ||
            x86_vl82c420_write_selected(chip, X86_VL82C420_BANK_SCAMP, 0xe2u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x15e2u, &value) != LIB_STATUS_OK || value != 0xffu ||
            x86_core_machine_bus_read(machine, 0x25e2u, &value) != LIB_STATUS_OK || value != 0xc0u ||
            x86_core_machine_bus_write(machine, 0x25e2u, 0x38u) != LIB_STATUS_OK ||
            mypc110_inking_sample(&inking, 1u, 2u, LIB_TRUE) != LIB_STATUS_OK ||
            !x86_nxvm_board_pic_irq_source_is_asserted(inking.irq) ||
            x86_nxvm_board_pic_get_interrupt(master, slave) != 0x0du;
        mypc110_inking_reset(&inking);
        failed |= x86_core_machine_bus_write(machine, 0x25e2u, 0x38u) != LIB_STATUS_OK ||
            x86_vl82c420_write_selected(chip, X86_VL82C420_BANK_SCAMP, 0x22u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x25e2u, &value) != LIB_STATUS_OK || value != 0xffu ||
            mypc110_inking_sample(&inking, 1u, 2u, LIB_TRUE) != LIB_STATUS_INVALID_STATE;
    }
    x86_vl82c420_destroy(chip);
    x86_nxvm_board_pic_finalize(master, slave);
    x86_core_machine_destroy(machine);
    return failed ? 1 : 0;
}

int main(void)
{
    const lib_u16 bases[] = {0x15e0u, 0x25e0u, 0x35e0u};
    for (lib_size index = 0u; index < sizeof(bases) / sizeof(bases[0]); ++index)
        if (check_inking(5u, bases[index]) || check_inking(10u, bases[index])) return 1;
    return check_selected_wiring();
}
