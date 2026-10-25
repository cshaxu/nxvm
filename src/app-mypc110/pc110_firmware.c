#include "app-mypc110/pc110_firmware_interface.h"

#include "x86-core/machine/memory_interface.h"

#define MYPC110_SYSTEM_FLASH_OFFSET (3u * MYPC110_TFT_FIRMWARE_BANK_BYTES)
#define MYPC110_VIDEO_FLASH_OFFSET (2u * MYPC110_TFT_FIRMWARE_BANK_BYTES)
#define MYPC110_FLASH_PHYSICAL_START MYPC110_VIDEO_PHYSICAL_START
#define MYPC110_SYSTEM_PHYSICAL_START 0x000f0000u
#define MYPC110_RESET_PHYSICAL_START 0xffff0000u

static lib_status mypc110_tft_firmware_shadow(const mypc110_tft_firmware *firmware,
    lib_u32 physical, lib_uptr bytes, x86_core_machine_memory_access access,
    lib_u8 **out_shadow, lib_size *out_offset)
{
    lib_size offset;

    if (firmware == LIB_NULL || firmware->bios == LIB_NULL ||
        firmware->video_shadow == LIB_NULL || firmware->system_shadow == LIB_NULL ||
        out_shadow == LIB_NULL || out_offset == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (physical >= MYPC110_VIDEO_PHYSICAL_START &&
        physical - MYPC110_VIDEO_PHYSICAL_START < MYPC110_VIDEO_SHADOW_BYTES) {
        if (access == X86_CORE_MACHINE_MEMORY_ACCESS_WRITE &&
            !firmware->video_shadow_writable) {
            return LIB_STATUS_UNSUPPORTED;
        }
        offset = (lib_size)(physical - MYPC110_VIDEO_PHYSICAL_START);
        if (bytes > MYPC110_VIDEO_SHADOW_BYTES - offset) return LIB_STATUS_UNSUPPORTED;
        *out_shadow = firmware->video_shadow;
        *out_offset = offset;
        return LIB_STATUS_OK;
    }
    if ((physical >= MYPC110_SYSTEM_PHYSICAL_START &&
         physical - MYPC110_SYSTEM_PHYSICAL_START < MYPC110_TFT_FIRMWARE_BANK_BYTES) ||
        (physical >= MYPC110_RESET_PHYSICAL_START &&
         physical - MYPC110_RESET_PHYSICAL_START < MYPC110_TFT_FIRMWARE_BANK_BYTES)) {
        offset = (lib_size)(physical >= MYPC110_RESET_PHYSICAL_START ?
            physical - MYPC110_RESET_PHYSICAL_START :
            physical - MYPC110_SYSTEM_PHYSICAL_START);
        if (bytes > MYPC110_TFT_FIRMWARE_BANK_BYTES - offset) return LIB_STATUS_UNSUPPORTED;
        *out_shadow = firmware->system_shadow;
        *out_offset = offset;
        return LIB_STATUS_OK;
    }
    return LIB_STATUS_UNSUPPORTED;
}

static lib_status mypc110_tft_firmware_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    const mypc110_tft_firmware *firmware = owner;
    lib_size offset;
    lib_u8 *shadow;
    lib_status status;

    (void)observe_only;
    if (firmware == LIB_NULL || firmware->bios == LIB_NULL || destination == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = mypc110_tft_firmware_shadow(firmware, physical, bytes,
        X86_CORE_MACHINE_MEMORY_ACCESS_READ, &shadow, &offset);
    if (status != LIB_STATUS_OK) return status;
    lib_memory_copy((void *)destination, shadow + offset, bytes);
    return LIB_STATUS_OK;
}

static lib_status mypc110_tft_firmware_read_instruction(void *owner,
    lib_u32 physical, lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    const mypc110_tft_firmware *firmware = owner;
    lib_size offset;

    (void)observe_only;
    if (firmware == LIB_NULL || firmware->bios == LIB_NULL || destination == 0u ||
        physical < MYPC110_VIDEO_PHYSICAL_START) return LIB_STATUS_UNSUPPORTED;
    offset = (lib_size)(physical - MYPC110_VIDEO_PHYSICAL_START);
    if (offset > MYPC110_VIDEO_SHADOW_BYTES ||
        bytes > MYPC110_VIDEO_SHADOW_BYTES - offset) return LIB_STATUS_UNSUPPORTED;
    lib_memory_copy((void *)destination,
        firmware->bios + MYPC110_VIDEO_FLASH_OFFSET + offset, bytes);
    return LIB_STATUS_OK;
}

static lib_status mypc110_tft_firmware_write(void *owner, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    mypc110_tft_firmware *firmware = owner;
    lib_size offset;
    lib_u8 *shadow;
    lib_status status;

    if (source == 0u) return LIB_STATUS_INVALID_ARGUMENT;
    status = mypc110_tft_firmware_shadow(firmware, physical, bytes,
        X86_CORE_MACHINE_MEMORY_ACCESS_WRITE, &shadow, &offset);
    if (status != LIB_STATUS_OK) return status;
    lib_memory_copy(shadow + offset, (const void *)source, bytes);
    return LIB_STATUS_OK;
}

static lib_status mypc110_tft_firmware_query(void *owner, lib_u32 physical,
    lib_uptr bytes, x86_core_machine_memory_access access)
{
    lib_size offset;
    lib_u8 *shadow;

    return mypc110_tft_firmware_shadow(owner, physical, bytes, access,
        &shadow, &offset);
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
        binding->video_shadow == LIB_NULL || binding->system_shadow == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    /* Raw Flash remains immutable at C0000--FFFFF.  App-owned replacement
     * routes expose the independently evidenced C000 video and F000 system
     * shadows; all remaining bytes retain their raw immutable owner. */
    return x86_core_machine_firmware_register_immutable_rom(firmware,
        MYPC110_FLASH_PHYSICAL_START, binding->bios, MYPC110_TFT_BIOS_BYTES);
}

static lib_status mypc110_tft_firmware_reset(void *opaque,
    x86_core_machine_firmware_context *firmware)
{
    mypc110_tft_firmware *binding = opaque;

    (void)firmware;
    if (binding == LIB_NULL || binding->bios == LIB_NULL ||
        binding->video_shadow == LIB_NULL || binding->system_shadow == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_copy(binding->video_shadow,
        binding->bios + MYPC110_VIDEO_FLASH_OFFSET,
        MYPC110_VIDEO_SHADOW_BYTES);
    binding->eced_index = 0u;
    binding->video_shadow_writable = LIB_FALSE;
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
    out_firmware->video_shadow = lib_allocate_zero(1u,
        MYPC110_VIDEO_SHADOW_BYTES);
    if (out_firmware->video_shadow == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    out_firmware->system_shadow = lib_allocate_zero(1u,
        MYPC110_TFT_FIRMWARE_BANK_BYTES);
    if (out_firmware->system_shadow == LIB_NULL) {
        lib_release(out_firmware->video_shadow);
        *out_firmware = (mypc110_tft_firmware) {0};
        return LIB_STATUS_NO_MEMORY;
    }
    out_firmware->bios = bios;
    lib_memory_copy(out_firmware->video_shadow,
        bios + MYPC110_VIDEO_FLASH_OFFSET, MYPC110_VIDEO_SHADOW_BYTES);
    lib_memory_copy(out_firmware->system_shadow,
        bios + MYPC110_SYSTEM_FLASH_OFFSET,
        MYPC110_TFT_FIRMWARE_BANK_BYTES);
    return LIB_STATUS_OK;
}

void mypc110_tft_firmware_note_eced_write(mypc110_tft_firmware *firmware,
    lib_u8 index, lib_u8 value)
{
    if (firmware == LIB_NULL) return;
    if (index == 0x0cu) {
        firmware->video_shadow_writable = (value & 0x20u) == 0u;
    }
}

void mypc110_tft_firmware_destroy(mypc110_tft_firmware *firmware)
{
    if (firmware == LIB_NULL) return;
    lib_release(firmware->video_shadow);
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
        {MYPC110_VIDEO_PHYSICAL_START, MYPC110_VIDEO_SHADOW_BYTES,
            mypc110_tft_firmware_callbacks,
            X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT, LIB_NULL},
        {MYPC110_SYSTEM_PHYSICAL_START, MYPC110_TFT_FIRMWARE_BANK_BYTES,
            mypc110_tft_firmware_callbacks,
            X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT, LIB_NULL},
        {MYPC110_RESET_PHYSICAL_START, MYPC110_TFT_FIRMWARE_BANK_BYTES,
            mypc110_tft_firmware_callbacks,
            X86_CORE_MACHINE_MEMORY_PROVIDER_RESET_OVERLAY, LIB_NULL}
    };

    if (firmware == LIB_NULL || firmware->bios == LIB_NULL ||
        firmware->video_shadow == LIB_NULL || firmware->system_shadow == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    lib_status status = x86_core_machine_install_memory_device_routes(machine, routes,
        sizeof(routes) / sizeof(routes[0]), LIB_NULL, LIB_NULL, firmware);

    if (status == LIB_STATUS_OK)
        status = x86_core_machine_set_memory_instruction_read(machine, firmware,
            mypc110_tft_firmware_read_instruction);
    return status;
}
