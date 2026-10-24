#include "app-mypc110/pc110_firmware_interface.h"
#include "x86-core/machine/machine_interface.h"

int main(void)
{
    static lib_u8 bios[MYPC110_TFT_BIOS_BYTES];
    const x86_core_machine_executor_config config = {
        .memory_bytes = 65536u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80486SX
    };
    mypc110_tft_firmware firmware;
    x86_core_machine *machine = LIB_NULL;
    lib_u32 value = 0u;
    const lib_u8 shadow_patch = 0x44u;
    lib_bool failed = LIB_FALSE;

    bios[0] = 0x55u;
    bios[MYPC110_TFT_FIRMWARE_BANK_BYTES] = 0x11u;
    bios[2u * MYPC110_TFT_FIRMWARE_BANK_BYTES] = 0x22u;
    bios[3u * MYPC110_TFT_FIRMWARE_BANK_BYTES] = 0x33u;
    failed |= mypc110_tft_firmware_create(LIB_NULL, MYPC110_TFT_BIOS_BYTES,
        &firmware) != LIB_STATUS_INVALID_ARGUMENT ||
        mypc110_tft_firmware_create(bios, MYPC110_TFT_BIOS_BYTES - 1u,
            &firmware) != LIB_STATUS_INVALID_ARGUMENT ||
        mypc110_tft_firmware_create(bios, MYPC110_TFT_BIOS_BYTES,
            &firmware) != LIB_STATUS_OK ||
        x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        x86_core_machine_bind_firmware_provider(machine,
            mypc110_tft_firmware_provider(), &firmware) != LIB_STATUS_OK ||
        mypc110_tft_firmware_install_routes(machine, &firmware) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x000c0000u, &value, 1u) !=
            LIB_STATUS_OK || value != 0x22u ||
        x86_core_machine_memory_read(machine, 0x000d0000u, &value, 1u) !=
            LIB_STATUS_OK || value != 0x11u ||
        x86_core_machine_memory_read(machine, 0x000f0000u, &value, 1u) !=
            LIB_STATUS_OK || value != 0x33u ||
        x86_core_machine_memory_read(machine, 0xffff0000u, &value, 1u) !=
            LIB_STATUS_OK || value != 0x33u ||
        x86_core_machine_memory_write(machine, 0x000f0000u, &shadow_patch, 1u) !=
            LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x000f0000u, &value, 1u) !=
            LIB_STATUS_OK || value != shadow_patch ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x000f0000u, &value, 1u) !=
            LIB_STATUS_OK || value != 0x33u;
    x86_core_machine_destroy(machine);
    mypc110_tft_firmware_destroy(&firmware);
    return failed ? 1 : 0;
}
