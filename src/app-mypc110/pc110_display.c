/* Copyright 2012-2026 Neko. */
#include "app-mypc110/pc110_display_interface.h"

static lib_status mypc110_display_port_read(void *owner, lib_u16 port,
    lib_u64 tick, lib_u32 *out_value)
{
    lib_u8 value;
    lib_status status;

    (void)tick;
    if (out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_f65535_extension_read(owner, port, &value);
    if (status == LIB_STATUS_OK) *out_value = value;
    return status;
}

static lib_status mypc110_display_port_write(void *owner, lib_u16 port,
    lib_u32 value)
{
    if (value > 0xffu) return LIB_STATUS_INVALID_ARGUMENT;
    return x86_f65535_extension_write(owner, port, (lib_u8)value);
}

static lib_status mypc110_display_memory_query(void *owner, lib_u32 physical,
    lib_uptr bytes, x86_core_machine_memory_access access)
{
    (void)owner;
    (void)access;
    if (physical < MYPC110_DISPLAY_MEMORY_START ||
        bytes > MYPC110_DISPLAY_MEMORY_BYTES -
            (physical - MYPC110_DISPLAY_MEMORY_START)) return LIB_STATUS_UNSUPPORTED;
    return LIB_STATUS_OK;
}

static lib_status mypc110_display_memory_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    (void)observe_only;
    if (destination == 0u || mypc110_display_memory_query(owner, physical,
            bytes, X86_CORE_MACHINE_MEMORY_ACCESS_READ) != LIB_STATUS_OK) {
        return LIB_STATUS_UNSUPPORTED;
    }
    return x86_f65535_vram_read(owner, physical - MYPC110_DISPLAY_MEMORY_START,
        (void *)destination, bytes);
}

static lib_status mypc110_display_memory_write(void *owner, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    if (source == 0u || mypc110_display_memory_query(owner, physical,
            bytes, X86_CORE_MACHINE_MEMORY_ACCESS_WRITE) != LIB_STATUS_OK) {
        return LIB_STATUS_UNSUPPORTED;
    }
    return x86_f65535_vram_write(owner, physical - MYPC110_DISPLAY_MEMORY_START,
        (const void *)source, bytes);
}

lib_status mypc110_display_install_routes(x86_core_machine *machine,
    x86_f65535 *controller)
{
    const x86_core_machine_port_route ports[] = {
        {X86_F65535_EXTENSION_INDEX_PORT, mypc110_display_port_read,
            mypc110_display_port_write, controller, LIB_FALSE, 0u},
        {X86_F65535_EXTENSION_DATA_PORT, mypc110_display_port_read,
            mypc110_display_port_write, controller, LIB_FALSE, 0u}
    };
    const x86_core_machine_memory_device_route memory = {
        MYPC110_DISPLAY_MEMORY_START, MYPC110_DISPLAY_MEMORY_BYTES,
        {mypc110_display_memory_read, mypc110_display_memory_write,
            mypc110_display_memory_query},
        X86_CORE_MACHINE_MEMORY_PROVIDER_STANDARD, LIB_NULL
    };
    lib_status status;

    if (machine == LIB_NULL || controller == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_core_machine_install_port_routes(machine, ports,
        sizeof(ports) / sizeof(ports[0]));
    if (status != LIB_STATUS_OK) return status;
    status = x86_core_machine_install_memory_device_routes(machine, &memory, 1u,
        LIB_NULL, LIB_NULL, controller);
    if (status != LIB_STATUS_OK)
        (void)x86_core_machine_remove_port_routes(machine, controller);
    return status;
}
