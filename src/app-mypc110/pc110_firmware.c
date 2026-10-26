#include "app-mypc110/pc110_firmware_interface.h"
#include "app-mypc110/pc110_config_interface.h"

#include "x86-core/machine/memory_interface.h"

#define MYPC110_FLASH_PHYSICAL_START 0x000e0000u
#define MYPC110_RESET_PHYSICAL_START 0xffff0000u
#define MYPC110_SHADOW_BLOCK_BYTES (16u * 1024u)

/* L2 PC110 decode: external Flash banks 2/3 occupy E/F, not C/D.
 * Segment fields select internal shadow independently for reads and writes.
 * ROMSET selects the external E Flash view; it is not a shadow-write lock. */
static lib_status mypc110_firmware_decode(const mypc110_firmware *firmware,
    lib_u32 physical, x86_core_machine_memory_access access,
    lib_size *out_offset, lib_bool *out_internal, lib_bool *out_flash)
{
    lib_u8 field;
    lib_u8 romset;
    lib_status status;

    if (firmware == LIB_NULL || firmware->bios == LIB_NULL ||
        firmware->shadow == LIB_NULL || out_offset == LIB_NULL ||
        out_internal == LIB_NULL || out_flash == LIB_NULL ||
        (access != X86_CORE_MACHINE_MEMORY_ACCESS_READ &&
         access != X86_CORE_MACHINE_MEMORY_ACCESS_WRITE))
        return LIB_STATUS_INVALID_ARGUMENT;
    if (physical >= MYPC110_RESET_PHYSICAL_START) {
        *out_offset = 3u * MYPC110_FIRMWARE_BANK_BYTES +
            (physical - MYPC110_RESET_PHYSICAL_START);
        *out_internal = LIB_FALSE;
        *out_flash = LIB_TRUE;
        return LIB_STATUS_OK;
    }
    if (physical >= MYPC110_SMRAM_PHYSICAL_START &&
        physical - MYPC110_SMRAM_PHYSICAL_START < MYPC110_SMRAM_BYTES) {
        if ((firmware->memory_control & 8u) == 0u && !firmware->smram_active)
            return LIB_STATUS_UNSUPPORTED;
        *out_offset = MYPC110_FIRMWARE_BYTES + physical - MYPC110_SMRAM_PHYSICAL_START;
        *out_internal = LIB_TRUE;
        *out_flash = LIB_FALSE;
        return LIB_STATUS_OK;
    }
    if (physical < MYPC110_VIDEO_PHYSICAL_START || physical >= 0x00100000u)
        return LIB_STATUS_UNSUPPORTED;
    if (firmware->configuration_chip == LIB_NULL) return LIB_STATUS_INVALID_STATE;
    *out_offset = physical - MYPC110_VIDEO_PHYSICAL_START;
    status = x86_vl82c420_read_register(firmware->configuration_chip,
        X86_VL82C420_BANK_ECED, (lib_u8)(0x0fu + (*out_offset >> 16u)), &field);
    if (status != LIB_STATUS_OK) return status;
    field = (field >> (2u * ((physical >> 14u) & 3u))) & 3u;
    *out_internal = (field & (access == X86_CORE_MACHINE_MEMORY_ACCESS_WRITE ?
        1u : 2u)) != 0u;
    *out_flash = LIB_FALSE;
    if (*out_internal) return LIB_STATUS_OK;
    if (physical >= 0x000f0000u) *out_flash = LIB_TRUE;
    else if (physical >= 0x000e0000u) {
        status = x86_vl82c420_read_register(firmware->configuration_chip,
            X86_VL82C420_BANK_ECED, 0x0cu, &romset);
        if (status != LIB_STATUS_OK) return status;
        *out_flash = (romset & 0x30u) == 0u;
    } else if (physical >= 0x000d0000u) {
        /* Leave external D accesses to the font window or open-bus fallback. */
        return LIB_STATUS_UNSUPPORTED;
    }
    /* The BIOS RAM trampoline toggles planar bit 2 to copy lower Flash,
     * then restores it. Internal shadow and the high reset alias are separate. */
    if (*out_flash && (firmware->planar_control & 4u) != 0u)
        *out_offset ^= 2u * MYPC110_FIRMWARE_BANK_BYTES;
    return LIB_STATUS_OK;
}

static lib_status mypc110_firmware_query(void *owner, lib_u32 physical,
    lib_uptr bytes, x86_core_machine_memory_access access)
{
    lib_size offset;
    lib_bool internal;
    lib_bool flash;

    if (physical >= MYPC110_SMRAM_PHYSICAL_START &&
        physical - MYPC110_SMRAM_PHYSICAL_START < MYPC110_SMRAM_BYTES) {
        if (bytes == 0u || bytes > MYPC110_SMRAM_BYTES -
                (physical - MYPC110_SMRAM_PHYSICAL_START)) return LIB_STATUS_UNSUPPORTED;
        return mypc110_firmware_decode(owner, physical, access, &offset, &internal, &flash);
    }
    if (physical < MYPC110_VIDEO_PHYSICAL_START ||
        (physical >= 0x00100000u && physical < MYPC110_RESET_PHYSICAL_START) ||
        bytes == 0u || bytes > (physical >= MYPC110_RESET_PHYSICAL_START ?
            (lib_u64)LIB_UINT32_MAX + 1u - physical : 0x00100000ull - physical))
        return LIB_STATUS_UNSUPPORTED;
    while (bytes != 0u) {
        lib_uptr chunk = MYPC110_SHADOW_BLOCK_BYTES -
            (physical & (MYPC110_SHADOW_BLOCK_BYTES - 1u));
        lib_status status = mypc110_firmware_decode(owner, physical, access,
            &offset, &internal, &flash);

        if (status != LIB_STATUS_OK) return status;
        if (chunk > bytes) chunk = bytes;
        physical += (lib_u32)chunk;
        bytes -= chunk;
    }
    return LIB_STATUS_OK;
}

static lib_status mypc110_firmware_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    const mypc110_firmware *firmware = owner;
    lib_status status;

    (void)observe_only;
    if (destination == 0u) return LIB_STATUS_INVALID_ARGUMENT;
    while (bytes != 0u) {
        lib_size offset;
        lib_bool internal;
        lib_bool flash;
        lib_uptr chunk = MYPC110_SHADOW_BLOCK_BYTES -
            (physical & (MYPC110_SHADOW_BLOCK_BYTES - 1u));

        status = mypc110_firmware_decode(firmware, physical,
            X86_CORE_MACHINE_MEMORY_ACCESS_READ, &offset, &internal, &flash);
        if (status != LIB_STATUS_OK) return status;
        if (chunk > bytes) chunk = bytes;
        if (internal || flash)
            lib_memory_copy((void *)destination,
                (internal ? firmware->shadow : firmware->bios) + offset, chunk);
        else lib_memory_set((void *)destination, 0xffu, chunk);
        physical += (lib_u32)chunk;
        destination += chunk;
        bytes -= chunk;
    }
    return LIB_STATUS_OK;
}

static lib_status mypc110_firmware_write(void *owner, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    mypc110_firmware *firmware = owner;
    lib_status status;

    if (source == 0u) return LIB_STATUS_INVALID_ARGUMENT;
    while (bytes != 0u) {
        lib_size offset;
        lib_bool internal;
        lib_bool flash;
        lib_uptr chunk = MYPC110_SHADOW_BLOCK_BYTES -
            (physical & (MYPC110_SHADOW_BLOCK_BYTES - 1u));

        status = mypc110_firmware_decode(firmware, physical,
            X86_CORE_MACHINE_MEMORY_ACCESS_WRITE, &offset, &internal, &flash);
        if (status != LIB_STATUS_OK) return status;
        if (chunk > bytes) chunk = bytes;
        if (internal)
            lib_memory_copy(firmware->shadow + offset, (const void *)source, chunk);
        physical += (lib_u32)chunk;
        source += chunk;
        bytes -= chunk;
    }
    return LIB_STATUS_OK;
}

static const x86_core_machine_memory_device_callbacks mypc110_firmware_callbacks = {
    mypc110_firmware_read, mypc110_firmware_write, mypc110_firmware_query
};

static lib_status mypc110_firmware_configure(void *opaque,
    x86_core_machine_firmware_context *firmware)
{
    const mypc110_firmware *binding = opaque;

    if (binding == LIB_NULL || binding->bios == LIB_NULL || binding->shadow == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    return x86_core_machine_firmware_register_immutable_rom(firmware,
        MYPC110_FLASH_PHYSICAL_START,
        binding->bios + 2u * MYPC110_FIRMWARE_BANK_BYTES,
        2u * MYPC110_FIRMWARE_BANK_BYTES);
}

static lib_status mypc110_firmware_reset(void *opaque,
    x86_core_machine_firmware_context *firmware)
{
    mypc110_firmware *binding = opaque;
    lib_status status;

    (void)firmware;
    if (binding == LIB_NULL || binding->bios == LIB_NULL || binding->shadow == LIB_NULL ||
        binding->configuration_chip == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    /* Full machine reset uses the same cold register image as construction.
     * Processor-only resume never enters this firmware-reset operation. */
    status = mypc110_config_initialize_cold_image(binding->configuration_chip);
    if (status != LIB_STATUS_OK) return status;
    lib_memory_set(binding->shadow, 0u, MYPC110_FIRMWARE_BYTES + MYPC110_SMRAM_BYTES);
    binding->memory_control = 0u;
    binding->smram_active = LIB_FALSE;
    binding->cpu_unlock_step = 0u;
    binding->planar_control = 0u;
    binding->cpu_power_mode = 0u;
    x86_vl82c420_reset_protocol(binding->configuration_chip);
    return LIB_STATUS_OK;
}

static const x86_core_machine_firmware_provider mypc110_provider = {
    mypc110_firmware_configure, mypc110_firmware_reset, LIB_NULL
};

lib_status mypc110_firmware_create(const lib_u8 *bios, lib_size bytes,
    mypc110_firmware *out_firmware)
{
    if (bios == LIB_NULL || bytes != MYPC110_FIRMWARE_BYTES || out_firmware == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_firmware = (mypc110_firmware) {0};
    out_firmware->shadow = lib_allocate_zero(1u, MYPC110_FIRMWARE_BYTES + MYPC110_SMRAM_BYTES);
    if (out_firmware->shadow == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    out_firmware->bios = bios;
    return LIB_STATUS_OK;
}

void mypc110_firmware_destroy(mypc110_firmware *firmware)
{
    if (firmware == LIB_NULL) return;
    lib_release(firmware->shadow);
    *firmware = (mypc110_firmware) {0};
}

const x86_core_machine_firmware_provider *mypc110_firmware_provider(void)
{
    return &mypc110_provider;
}

lib_status mypc110_firmware_install_routes(x86_core_machine *machine,
    mypc110_firmware *firmware)
{
    const x86_core_machine_memory_device_route routes[] = {
        {MYPC110_SMRAM_PHYSICAL_START, MYPC110_SMRAM_BYTES,
            mypc110_firmware_callbacks,
            X86_CORE_MACHINE_MEMORY_PROVIDER_CPU_OVERLAY, LIB_NULL},
        {MYPC110_VIDEO_PHYSICAL_START, MYPC110_FIRMWARE_BYTES,
            mypc110_firmware_callbacks,
            X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT, LIB_NULL},
        {MYPC110_RESET_PHYSICAL_START, MYPC110_FIRMWARE_BANK_BYTES,
            mypc110_firmware_callbacks,
            X86_CORE_MACHINE_MEMORY_PROVIDER_RESET_OVERLAY, LIB_NULL}
    };

    if (firmware == LIB_NULL || firmware->bios == LIB_NULL || firmware->shadow == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    lib_status status = x86_core_machine_install_memory_device_routes(machine, routes,
        sizeof(routes) / sizeof(routes[0]), LIB_NULL, LIB_NULL, firmware);
    if (status == LIB_STATUS_OK) firmware->machine = machine;
    return status;
}

void mypc110_firmware_system_management_active(void *context, lib_bool active)
{
    mypc110_firmware *firmware = context;

    if (firmware->smram_active == active) return;
    firmware->smram_active = active;
    x86_core_machine_signal_memory_decode_change(firmware->machine,
        MYPC110_SMRAM_PHYSICAL_START, MYPC110_SMRAM_BYTES);
}

/* Preserve the existing board-owned processor-reset route if architectural
 * shutdown is later observed.  The admitted BIOS's first reset is the normal
 * 8042 FEh command; this binding makes no contrary firmware claim. */
static void mypc110_firmware_profile_reset(void *context,
    x86_nxvm_board_reset_phase phase)
{
    (void)context;
    (void)phase;
}

static void mypc110_firmware_profile_refresh_nmi(void *context)
{
    (void)context;
}

static lib_bool mypc110_firmware_profile_refresh_request(void *context,
    lib_u8 *out_address)
{
    (void)context;
    (void)out_address;
    return LIB_FALSE;
}

static void mypc110_firmware_profile_refresh_complete(void *context)
{
    (void)context;
}

static lib_bool mypc110_firmware_profile_next_deadline(void *context,
    lib_u64 now, lib_u64 *out_tick)
{
    (void)context;
    (void)now;
    (void)out_tick;
    return LIB_FALSE;
}

static void mypc110_firmware_profile_finalize(void *context)
{
    (void)context;
}

lib_status mypc110_firmware_profile_create(
    const x86_nxvm_board_profile_services *services, void *context,
    x86_nxvm_board_profile_binding *out_binding)
{
    if (services == LIB_NULL || services->core == LIB_NULL || context == LIB_NULL ||
        out_binding == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_binding = (x86_nxvm_board_profile_binding) {
        .context = context,
        .reset = mypc110_firmware_profile_reset,
        .refresh_nmi = mypc110_firmware_profile_refresh_nmi,
        .refresh_request = mypc110_firmware_profile_refresh_request,
        .refresh_complete = mypc110_firmware_profile_refresh_complete,
        .next_deadline = mypc110_firmware_profile_next_deadline,
        .finalize = mypc110_firmware_profile_finalize,
        .shutdown_resets = LIB_TRUE,
        .system_management_active = mypc110_firmware_system_management_active};
    return LIB_STATUS_OK;
}
