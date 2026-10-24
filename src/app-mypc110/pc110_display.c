/* Copyright 2012-2026 Neko. */
#include "app-mypc110/pc110_display_interface.h"

static lib_status mypc110_display_port_read(void *owner, lib_u16 port,
    lib_u64 tick, lib_u32 *out_value)
{
    lib_u8 value;
    lib_status status;

    (void)tick;
    if (out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_f65535_vga_read(owner, port, &value);
    if (status == LIB_STATUS_OK) *out_value = value;
    return status;
}

static lib_status mypc110_display_port_write(void *owner, lib_u16 port,
    lib_u32 value)
{
    if (value > 0xffu) return LIB_STATUS_INVALID_ARGUMENT;
    return x86_f65535_vga_write(owner, port, (lib_u8)value);
}

static lib_status mypc110_display_memory_query(void *owner, lib_u32 physical,
    lib_uptr bytes, x86_core_machine_memory_access access)
{
    return x86_f65535_memory_query(owner, physical, bytes,
        access == X86_CORE_MACHINE_MEMORY_ACCESS_WRITE ? LIB_TRUE : LIB_FALSE);
}

static lib_status mypc110_display_memory_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    (void)observe_only;
    if (destination == 0u || mypc110_display_memory_query(owner, physical,
            bytes, X86_CORE_MACHINE_MEMORY_ACCESS_READ) != LIB_STATUS_OK) {
        return LIB_STATUS_UNSUPPORTED;
    }
    return x86_f65535_memory_read(owner, physical, (void *)destination, bytes);
}

static lib_status mypc110_display_memory_write(void *owner, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    if (source == 0u || mypc110_display_memory_query(owner, physical,
            bytes, X86_CORE_MACHINE_MEMORY_ACCESS_WRITE) != LIB_STATUS_OK) {
        return LIB_STATUS_UNSUPPORTED;
    }
    return x86_f65535_memory_write(owner, physical, (const void *)source, bytes);
}

lib_status mypc110_display_install_routes(x86_core_machine *machine,
    x86_f65535 *controller)
{
    const x86_core_machine_port_route ports[] = {
        {0x03c0u, mypc110_display_port_read,
            mypc110_display_port_write, controller, LIB_FALSE, 0u},
        {0x03c1u, mypc110_display_port_read,
            mypc110_display_port_write, controller, LIB_FALSE, 0u},
        {0x03c2u, mypc110_display_port_read,
            mypc110_display_port_write, controller, LIB_FALSE, 0u},
        {0x03c4u, mypc110_display_port_read,
            mypc110_display_port_write, controller, LIB_FALSE, 0u},
        {0x03c5u, mypc110_display_port_read,
            mypc110_display_port_write, controller, LIB_FALSE, 0u},
        {0x03c6u, mypc110_display_port_read,
            mypc110_display_port_write, controller, LIB_FALSE, 0u},
        {0x03c7u, mypc110_display_port_read,
            mypc110_display_port_write, controller, LIB_FALSE, 0u},
        {0x03c8u, mypc110_display_port_read,
            mypc110_display_port_write, controller, LIB_FALSE, 0u},
        {0x03c9u, mypc110_display_port_read,
            mypc110_display_port_write, controller, LIB_FALSE, 0u},
        {0x03ccu, mypc110_display_port_read,
            mypc110_display_port_write, controller, LIB_FALSE, 0u},
        {0x03ceu, mypc110_display_port_read,
            mypc110_display_port_write, controller, LIB_FALSE, 0u},
        {0x03cfu, mypc110_display_port_read,
            mypc110_display_port_write, controller, LIB_FALSE, 0u},
        {0x03d4u, mypc110_display_port_read,
            mypc110_display_port_write, controller, LIB_FALSE, 0u},
        {0x03d5u, mypc110_display_port_read,
            mypc110_display_port_write, controller, LIB_FALSE, 0u},
        {X86_F65535_EXTENSION_INDEX_PORT, mypc110_display_port_read,
            mypc110_display_port_write, controller, LIB_FALSE, 0u},
        {X86_F65535_EXTENSION_DATA_PORT, mypc110_display_port_read,
            mypc110_display_port_write, controller, LIB_FALSE, 0u},
        {0x03dau, mypc110_display_port_read,
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
