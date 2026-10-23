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
    lib_bool failed = LIB_FALSE;

    bios[0] = 0x55u;
    bios[MYPC110_TFT_BIOS_BYTES - 1u] = 0xaau;
    failed |= mypc110_tft_firmware_create(LIB_NULL, MYPC110_TFT_BIOS_BYTES,
        &firmware) != LIB_STATUS_INVALID_ARGUMENT ||
        mypc110_tft_firmware_create(bios, MYPC110_TFT_BIOS_BYTES - 1u,
            &firmware) != LIB_STATUS_INVALID_ARGUMENT ||
        mypc110_tft_firmware_create(bios, MYPC110_TFT_BIOS_BYTES,
            &firmware) != LIB_STATUS_OK ||
        x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        x86_core_machine_bind_firmware_provider(machine,
            mypc110_tft_firmware_provider(), &firmware) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        !x86_core_machine_immutable_rom_mapping_contains(machine,
            MYPC110_TFT_BIOS_PHYSICAL_START, MYPC110_TFT_BIOS_BYTES) ||
        x86_core_machine_immutable_rom_mapping_contains(machine,
            MYPC110_TFT_BIOS_PHYSICAL_START - 1u, 1u);
    x86_core_machine_destroy(machine);
    return failed ? 1 : 0;
}
