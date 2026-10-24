#include "app-mypc110/pc110_firmware_interface.h"

#include "x86-core/machine/memory_interface.h"

#define MYPC110_SYSTEM_FLASH_OFFSET (3u * MYPC110_TFT_FIRMWARE_BANK_BYTES)
#define MYPC110_FLASH_PHYSICAL_START 0x000c0000u
#define MYPC110_SYSTEM_PHYSICAL_START 0x000f0000u
#define MYPC110_RESET_PHYSICAL_START 0xffff0000u

static lib_status mypc110_tft_firmware_offset(const mypc110_tft_firmware *firmware,
    lib_u32 physical, lib_uptr bytes, lib_size *out_offset, lib_bool *out_writable)
{
    lib_size offset;

    if (firmware == LIB_NULL || firmware->bios == LIB_NULL ||
        firmware->system_shadow == LIB_NULL || out_offset == LIB_NULL ||
        out_writable == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if ((physical >= MYPC110_SYSTEM_PHYSICAL_START &&
         physical - MYPC110_SYSTEM_PHYSICAL_START < MYPC110_TFT_FIRMWARE_BANK_BYTES) ||
        (physical >= MYPC110_RESET_PHYSICAL_START &&
         physical - MYPC110_RESET_PHYSICAL_START < MYPC110_TFT_FIRMWARE_BANK_BYTES)) {
        offset = (lib_size)(physical >= MYPC110_RESET_PHYSICAL_START ?
            physical - MYPC110_RESET_PHYSICAL_START :
            physical - MYPC110_SYSTEM_PHYSICAL_START);
        if (bytes > MYPC110_TFT_FIRMWARE_BANK_BYTES - offset) return LIB_STATUS_UNSUPPORTED;
        *out_offset = offset;
        *out_writable = LIB_TRUE;
        return LIB_STATUS_OK;
    }
    return LIB_STATUS_UNSUPPORTED;
}

static lib_status mypc110_tft_firmware_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    const mypc110_tft_firmware *firmware = owner;
    lib_size offset;
    lib_bool writable;
    lib_status status;

    (void)observe_only;
    if (destination == 0u) return LIB_STATUS_INVALID_ARGUMENT;
    status = mypc110_tft_firmware_offset(firmware, physical, bytes, &offset, &writable);
    if (status != LIB_STATUS_OK) return status;
    if (writable) lib_memory_copy((void *)destination, firmware->system_shadow + offset, bytes);
    else lib_memory_copy((void *)destination, firmware->bios + offset, bytes);
    return LIB_STATUS_OK;
}

static lib_status mypc110_tft_firmware_write(void *owner, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    mypc110_tft_firmware *firmware = owner;
    lib_size offset;
    lib_bool writable;
    lib_status status;

    if (source == 0u) return LIB_STATUS_INVALID_ARGUMENT;
    status = mypc110_tft_firmware_offset(firmware, physical, bytes, &offset, &writable);
    if (status != LIB_STATUS_OK) return status;
    if (writable) lib_memory_copy(firmware->system_shadow + offset, (const void *)source, bytes);
    return LIB_STATUS_OK;
}

static lib_status mypc110_tft_firmware_query(void *owner, lib_u32 physical,
    lib_uptr bytes, x86_core_machine_memory_access access)
{
    lib_size offset;
    lib_bool writable;

    (void)access;
    return mypc110_tft_firmware_offset(owner, physical, bytes, &offset, &writable);
}

static const x86_core_machine_memory_device_callbacks mypc110_tft_firmware_callbacks = {
    mypc110_tft_firmware_read, mypc110_tft_firmware_write,
    mypc110_tft_firmware_query
};

static lib_status mypc110_tft_firmware_configure(void *opaque,
    x86_core_machine_firmware_context *firmware)
{
    const mypc110_tft_firmware *binding = opaque;

    if (binding == LIB_NULL || binding->bios == LIB_NULL ||
        binding->system_shadow == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    /* Raw Flash remains immutable at C0000--FFFFF.  The App overlays only the
     * independently evidenced writable F-segment shadow after configuration. */
    return x86_core_machine_firmware_register_immutable_rom(firmware,
        MYPC110_FLASH_PHYSICAL_START, binding->bios, MYPC110_TFT_BIOS_BYTES);
}

static lib_status mypc110_tft_firmware_reset(void *opaque,
    x86_core_machine_firmware_context *firmware)
{
    const mypc110_tft_firmware *binding = opaque;

    (void)firmware;
    if (binding == LIB_NULL || binding->bios == LIB_NULL ||
        binding->system_shadow == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_copy(binding->system_shadow,
        binding->bios + MYPC110_SYSTEM_FLASH_OFFSET,
        MYPC110_TFT_FIRMWARE_BANK_BYTES);
    return LIB_STATUS_OK;
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
    *out_firmware = (mypc110_tft_firmware) {0};
    out_firmware->system_shadow = lib_allocate_zero(1u,
        MYPC110_TFT_FIRMWARE_BANK_BYTES);
    if (out_firmware->system_shadow == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    out_firmware->bios = bios;
    lib_memory_copy(out_firmware->system_shadow,
        bios + MYPC110_SYSTEM_FLASH_OFFSET,
        MYPC110_TFT_FIRMWARE_BANK_BYTES);
    return LIB_STATUS_OK;
}

void mypc110_tft_firmware_destroy(mypc110_tft_firmware *firmware)
{
    if (firmware == LIB_NULL) return;
    lib_release(firmware->system_shadow);
    *firmware = (mypc110_tft_firmware) {0};
}

const x86_core_machine_firmware_provider *mypc110_tft_firmware_provider(void)
{
    return &mypc110_tft_provider;
}

lib_status mypc110_tft_firmware_install_routes(x86_core_machine *machine,
    mypc110_tft_firmware *firmware)
{
    const x86_core_machine_memory_device_route routes[] = {
        {MYPC110_SYSTEM_PHYSICAL_START, MYPC110_TFT_FIRMWARE_BANK_BYTES,
            mypc110_tft_firmware_callbacks,
            X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT, LIB_NULL},
        {MYPC110_RESET_PHYSICAL_START, MYPC110_TFT_FIRMWARE_BANK_BYTES,
            mypc110_tft_firmware_callbacks,
            X86_CORE_MACHINE_MEMORY_PROVIDER_RESET_OVERLAY, LIB_NULL}
    };

    if (firmware == LIB_NULL || firmware->bios == LIB_NULL ||
        firmware->system_shadow == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    return x86_core_machine_install_memory_device_routes(machine, routes,
        sizeof(routes) / sizeof(routes[0]), LIB_NULL, LIB_NULL, firmware);
}
