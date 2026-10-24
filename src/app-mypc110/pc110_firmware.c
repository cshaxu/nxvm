#include "app-mypc110/pc110_firmware_interface.h"

static lib_status mypc110_tft_firmware_configure(void *opaque,
    x86_core_machine_firmware_context *firmware)
{
    const mypc110_tft_firmware *binding = opaque;

    if (binding == LIB_NULL || binding->bios == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    lib_status status = x86_core_machine_firmware_register_immutable_rom(firmware,
        MYPC110_TFT_BIOS_PHYSICAL_START, binding->bios, MYPC110_TFT_BIOS_BYTES);

    if (status != LIB_STATUS_OK) return status;
    /* The 486SX begins at FFFF:FFF0, whose reset fetch is in the top 64 KiB
     * of physical space.  This is an alias of the BIOS's real F0000-FFFFF
     * window, not a PC/AT board facade or a firmware interception. */
    return x86_core_machine_firmware_register_immutable_rom_alias(firmware,
        0x000f0000u, 0xffff0000u, 0x00010000u);
}

static lib_status mypc110_tft_firmware_reset(void *opaque,
    x86_core_machine_firmware_context *firmware)
{
    const mypc110_tft_firmware *binding = opaque;

    (void)firmware;
    return binding == LIB_NULL || binding->bios == LIB_NULL ?
        LIB_STATUS_INVALID_ARGUMENT : LIB_STATUS_OK;
}

static const x86_core_machine_firmware_provider mypc110_tft_provider = {
    mypc110_tft_firmware_configure,
    mypc110_tft_firmware_reset,
    LIB_NULL
};

lib_status mypc110_tft_firmware_create(const lib_u8 *bios, lib_size bytes,
    mypc110_tft_firmware *out_firmware)
{
    if (bios == LIB_NULL || bytes != MYPC110_TFT_BIOS_BYTES ||
        out_firmware == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_firmware = (mypc110_tft_firmware) {bios};
    return LIB_STATUS_OK;
}

const x86_core_machine_firmware_provider *mypc110_tft_firmware_provider(void)
{
    return &mypc110_tft_provider;
}
