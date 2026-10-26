#include "app-mypc110/pc110_font_interface.h"

static lib_status mypc110_font_query(void *owner, lib_u32 physical,
    lib_uptr bytes, x86_core_machine_memory_access access)
{
    const mypc110_font_rom *font = owner;
    const lib_u32 offset = physical - MYPC110_FONT_PHYSICAL_START;

    (void)access;
    if (font == LIB_NULL || font->bytes == LIB_NULL ||
        physical < MYPC110_FONT_PHYSICAL_START ||
        bytes > MYPC110_FONT_BANK_BYTES - offset) return LIB_STATUS_UNSUPPORTED;
    return LIB_STATUS_OK;
}

static lib_status mypc110_font_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    const mypc110_font_rom *font = owner;
    const lib_u32 offset = physical - MYPC110_FONT_PHYSICAL_START;

    (void)observe_only;
    if (mypc110_font_query(owner, physical, bytes,
            X86_CORE_MACHINE_MEMORY_ACCESS_READ) != LIB_STATUS_OK ||
        destination == 0u) return LIB_STATUS_UNSUPPORTED;
    if (!font->enabled) {
        lib_memory_set((void *)destination, 0xffu, bytes);
        return LIB_STATUS_OK;
    }
    lib_memory_copy((void *)destination,
        font->bytes + (lib_size)font->bank * MYPC110_FONT_BANK_BYTES + offset,
        bytes);
    return LIB_STATUS_OK;
}

static lib_status mypc110_font_write(void *owner, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    (void)source;
    return mypc110_font_query(owner, physical, bytes,
        X86_CORE_MACHINE_MEMORY_ACCESS_WRITE);
}

static lib_status mypc110_font_port_read(void *owner, lib_u16 port,
    lib_u64 tick, lib_u32 *out_value)
{
    const mypc110_font_rom *font = owner;

    (void)tick;
    if (font == LIB_NULL || font->bytes == LIB_NULL || out_value == LIB_NULL ||
        port < MYPC110_FONT_PORT_START || port >= MYPC110_FONT_PORT_END) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    switch (port) {
    case MYPC110_FONT_BANK_PORT:
        *out_value = font->bank;
        break;
    case MYPC110_FONT_UNUSED_PORT:
        *out_value = 0xffu;
        break;
    case MYPC110_FONT_SEGMENT_PORT:
        *out_value = MYPC110_FONT_SEGMENT;
        break;
    default:
        *out_value = font->enabled ? 1u : 0u;
        break;
    }
    return LIB_STATUS_OK;
}

static lib_status mypc110_font_port_write(void *owner, lib_u16 port,
    lib_u8 access_bytes, lib_u32 value)
{
    mypc110_font_rom *font = owner;

    if (font == LIB_NULL || font->bytes == LIB_NULL ||
        access_bytes != 1u || port < MYPC110_FONT_PORT_START ||
        port >= MYPC110_FONT_PORT_END || value > 0xffu) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (port == MYPC110_FONT_BANK_PORT)
        font->bank = (lib_u8)value & (MYPC110_FONT_BANK_COUNT - 1u);
    else if (port == MYPC110_FONT_ENABLE_PORT)
        font->enabled = (value & 1u) != 0u;
    return LIB_STATUS_OK;
}

lib_status mypc110_font_rom_create(const lib_u8 *bytes, lib_size byte_count,
    mypc110_font_rom *out_font)
{
    if (bytes == LIB_NULL || byte_count != MYPC110_FONT_ROM_BYTES ||
        out_font == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_font = (mypc110_font_rom) {.bytes = bytes, .bank = 0u, .enabled = LIB_TRUE};
    return LIB_STATUS_OK;
}

lib_status mypc110_font_rom_install(x86_core_machine *machine,
    mypc110_font_rom *font)
{
    const x86_core_machine_port_provider port = {
        mypc110_font_port_read, mypc110_font_port_write
    };
    const x86_core_machine_memory_device_route memory = {
        MYPC110_FONT_PHYSICAL_START, MYPC110_FONT_BANK_BYTES,
        {mypc110_font_read, mypc110_font_write, mypc110_font_query},
        /* The live DE000h font aperture replaces the overlapping C0000h-DFFFFh
         * BIOS/shadow decode only while this device claims the access. */
        X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT, LIB_NULL
    };
    lib_status status;

    if (machine == LIB_NULL || font == LIB_NULL || font->bytes == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    status = x86_core_machine_install_port_provider(machine, MYPC110_FONT_PORT_START,
        MYPC110_FONT_PORT_END - 1u, &port, font);
    if (status != LIB_STATUS_OK) return status;
    status = x86_core_machine_install_memory_device_routes(machine, &memory, 1u,
        LIB_NULL, LIB_NULL, font);
    if (status != LIB_STATUS_OK) {
        (void)x86_core_machine_remove_port_routes(machine, font);
        return status;
    }
    return LIB_STATUS_OK;
}
