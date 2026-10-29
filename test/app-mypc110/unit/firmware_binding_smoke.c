#include "app-mypc110/pc110_config_interface.h"
#include "app-mypc110/pc110_firmware_interface.h"
#include "x86-core/machine/machine_interface.h"

static lib_bool firmware_register(x86_core_machine *machine,
    lib_u8 index, lib_u8 value)
{
    return x86_core_machine_bus_write(machine, 0x00ecu, index) == LIB_STATUS_OK &&
        x86_core_machine_bus_write(machine, 0x00edu, value) == LIB_STATUS_OK;
}

lib_i32 main(void)
{
    static lib_u8 bios[MYPC110_FIRMWARE_BYTES];
    const x86_core_machine_executor_config config = {
        .memory_bytes = 65536u, .cpu_profile = X86_CORE_CPU_PROFILE_80486SX
    };
    const lib_u32 banks[] = {0x000c0000u, 0x000e0000u, 0x000f0000u};
    mypc110_firmware firmware = {0};
    mypc110_firmware unmatched = {0};
    x86_core_machine *machine = LIB_NULL;
    x86_vl82c420 *chip = LIB_NULL;
    lib_u32 value = 0u;
    lib_bool failed = LIB_FALSE;

    for (lib_size bank = 0u; bank < 4u; ++bank)
        for (lib_size block = 0u; block < 4u; ++block)
            bios[bank * MYPC110_FIRMWARE_BANK_BYTES + block * 0x4000u] =
                (lib_u8)(0x80u + bank * 4u + block);
    bios[0x33ff8u] = 0xa8u;
    bios[0x33ff9u] = 0x08u;
    bios[0x33ffau] = 0x74u;
    bios[0x33ffbu] = 0x25u;
    failed |= mypc110_firmware_create(bios, sizeof(bios), &unmatched) != LIB_STATUS_OK;
    if (unmatched.bios != LIB_NULL)
        failed |= unmatched.bios[0x33ffau] != 0x74u || unmatched.bios[0x33ffbu] != 0x25u;
    mypc110_firmware_destroy(&unmatched);
    bios[0x33ffbu] = 0x24u;
    failed |= mypc110_firmware_create(LIB_NULL, sizeof(bios), &firmware) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        mypc110_firmware_create(bios, sizeof(bios) - 1u, &firmware) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        mypc110_firmware_create(bios, sizeof(bios), &firmware) != LIB_STATUS_OK ||
        x86_vl82c420_create(&chip) != LIB_STATUS_OK ||
        mypc110_config_initialize_cold_image(chip) != LIB_STATUS_OK ||
        x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        x86_core_machine_bind_firmware_provider(machine,
            mypc110_firmware_provider(), &firmware) != LIB_STATUS_OK ||
        mypc110_firmware_install_routes(machine, &firmware) != LIB_STATUS_OK ||
        mypc110_config_install_routes(machine, &firmware, chip) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x000c0000u, &value, 1u) !=
            LIB_STATUS_OK || value != 0xffu ||
        x86_core_machine_memory_read(machine, 0x000e0000u, &value, 1u) !=
            LIB_STATUS_OK || value != 0xffu ||
        x86_core_machine_memory_read(machine, 0x000f0000u, &value, 1u) !=
            LIB_STATUS_OK || value != 0x8cu ||
        x86_core_machine_memory_read(machine, 0xffff0000u, &value, 1u) !=
            LIB_STATUS_OK || value != 0x8cu ||
        x86_core_machine_memory_read(machine, 0x000f3ffau, &value, 1u) !=
            LIB_STATUS_OK || value != 0x90u ||
        x86_core_machine_memory_read(machine, 0x000f3ffbu, &value, 1u) !=
            LIB_STATUS_OK || value != 0x90u ||
        bios[0x33ffau] != 0x74u || bios[0x33ffbu] != 0x24u ||
        x86_core_machine_bus_write(machine, 0x00fbu, 0u) != LIB_STATUS_OK ||
        !firmware_register(machine, 0x0cu, 0x09u);

    /* All four 16 KiB fields, all four access choices, and all C/E/F windows.
     * Prepare through guest writes; neither the test nor reset prefills shadow. */
    for (lib_size bank = 0u; !failed && bank < sizeof(banks) / sizeof(banks[0]); ++bank) {
        const lib_u8 index = (lib_u8)(0x0fu + ((banks[bank] - 0xc0000u) >> 16u));

        for (lib_u8 mode = 0u; !failed && mode < 4u; ++mode) {
            failed |= !firmware_register(machine, index, 0xffu);
            for (lib_u32 block = 0u; !failed && block < 4u; ++block) {
                const lib_u32 address = banks[bank] + block * 0x4000u;
                const lib_u8 original = 0x44u;

                failed |= x86_core_machine_memory_write(machine, address,
                    &original, 1u) != LIB_STATUS_OK;
            }
            failed |= !firmware_register(machine, index, (lib_u8)(mode * 0x55u));
            for (lib_u32 block = 0u; !failed && block < 4u; ++block) {
                const lib_u32 address = banks[bank] + block * 0x4000u;
                const lib_u8 patch = 0x77u;
                const lib_u8 external = bank == 0u ? 0xffu :
                    bios[address - 0xc0000u];
                const lib_u8 expected = (mode & 2u) != 0u ?
                    ((mode & 1u) != 0u ? patch : 0x44u) : external;

                failed |= x86_core_machine_memory_write(machine, address,
                        &patch, 1u) != LIB_STATUS_OK ||
                    x86_core_machine_memory_read(machine, address, &value, 1u) !=
                        LIB_STATUS_OK || value != expected ||
                    bios[address - 0xc0000u] != (lib_u8)(0x80u +
                        ((address - 0xc0000u) >> 16u) * 4u + block);
            }
        }
    }
    if (!failed) {
        x86_core_machine_run_result reset = {0};

        failed |= x86_core_machine_bus_read(machine, 0x98u, &value) != LIB_STATUS_OK ||
            value != 0u || !firmware_register(machine, 0x11u, 0u) ||
            !firmware_register(machine, 0x12u, 0u) ||
            x86_core_machine_bus_write(machine, 0x98u, 4u) != LIB_STATUS_OK;
        for (lib_u32 bank = 0u; !failed && bank < 2u; ++bank) {
            for (lib_u32 block = 0u; !failed && block < 4u; ++block) {
                const lib_u32 address = 0xe0000u + bank * 0x10000u + block * 0x4000u;
                const lib_u8 expected = (lib_u8)(0x80u + bank * 4u + block);
                const lib_u8 patch = 0x22u;

                failed |= x86_core_machine_memory_write(machine, address, &patch, 1u) !=
                        LIB_STATUS_OK ||
                    x86_core_machine_memory_read(machine, address, &value, 1u) !=
                        LIB_STATUS_OK || value != expected;
            }
        }
        failed |= x86_core_machine_memory_read(machine, 0xffff0000u, &value, 1u) !=
                LIB_STATUS_OK || value != 0x8cu ||
            !firmware_register(machine, 0x11u, 0xaau) ||
            x86_core_machine_memory_read(machine, 0xe0000u, &value, 1u) !=
                LIB_STATUS_OK || value != 0x77u;
        x86_core_machine_signal_processor_reset(machine);
        failed |= x86_core_machine_run(machine,
                (x86_core_machine_run_budget){.instructions = 1u}, &reset) != LIB_STATUS_OK ||
            reset.reason != X86_CORE_MACHINE_STOP_RESET_REQUESTED ||
            x86_core_machine_bus_read(machine, 0x98u, &value) != LIB_STATUS_OK || value != 4u ||
            x86_core_machine_bus_write(machine, 0x98u, 0u) != LIB_STATUS_OK ||
            !firmware_register(machine, 0x11u, 0u) ||
            x86_core_machine_memory_read(machine, 0xe0000u, &value, 1u) !=
                LIB_STATUS_OK || value != 0x88u ||
            x86_core_machine_memory_read(machine, 0xf0000u, &value, 1u) !=
                LIB_STATUS_OK || value != 0x8cu ||
            x86_core_machine_bus_write(machine, 0x98u, 4u) != LIB_STATUS_OK;
    }
    if (!failed) {
        const lib_u8 cross[] = {0x90u, 0x91u};
        lib_u8 copied[2u] = {0};

        failed |= !firmware_register(machine, 0x0fu, 0x03u) ||
            x86_core_machine_memory_write(machine, 0xc3fffu, cross, sizeof(cross)) !=
                LIB_STATUS_OK ||
            x86_core_machine_memory_read(machine, 0xc3fffu, copied, sizeof(copied)) !=
                LIB_STATUS_OK || copied[0] != 0x90u || copied[1] != 0xffu ||
            !firmware_register(machine, 0x0fu, 0x0fu) ||
            x86_core_machine_memory_read(machine, 0xc3fffu, copied, sizeof(copied)) !=
                LIB_STATUS_OK || copied[0] != 0x90u || copied[1] != 0x77u ||
            x86_core_machine_bus_write(machine, 0x00ecu, 0x03u) != LIB_STATUS_OK ||
            x86_core_machine_memory_read(machine, 0xc0000u, &value, 1u) !=
                LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x00edu, &value) != LIB_STATUS_OK ||
            value != 0xccu ||
            !firmware_register(machine, 0x0cu, MYPC110_RESET_ROMSET) ||
            !firmware_register(machine, 0x11u, 0u) ||
            x86_core_machine_memory_read(machine, 0xe0000u, &value, 1u) !=
                LIB_STATUS_OK || value != 0xffu ||
            !firmware_register(machine, 0x11u, 0xaau) ||
            x86_core_machine_memory_read(machine, 0xe0000u, &value, 1u) !=
                LIB_STATUS_OK || value != 0x77u ||
            x86_core_machine_memory_read(machine, 0xffff0000u, &value, 1u) !=
                LIB_STATUS_OK || value != 0x8cu ||
            x86_core_machine_reset(machine) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x98u, &value) != LIB_STATUS_OK || value != 0u ||
            x86_core_machine_memory_read(machine, 0xc0000u, &value, 1u) !=
                LIB_STATUS_OK || value != 0xffu ||
            x86_core_machine_memory_read(machine, 0xf0000u, &value, 1u) !=
                LIB_STATUS_OK || value != 0x8cu ||
            x86_core_machine_bus_write(machine, 0x00fbu, 0u) != LIB_STATUS_OK ||
            !firmware_register(machine, 0x12u, 0xaau) ||
            x86_core_machine_memory_read(machine, 0xf0000u, &value, 1u) !=
                LIB_STATUS_OK || value != 0u;
    }
    x86_core_machine_destroy(machine);
    mypc110_firmware_destroy(&firmware);
    x86_vl82c420_destroy(chip);
    return failed ? 1 : 0;
}
