#include "app-myob300/ob300_display.h"
#include "x86-core/machine/memory_interface.h"

static lib_u16 myob300_display_vga_port(lib_u16 port)
{
    return port < 0x03c0u ? port + 0x20u : port;
}

static lib_status myob300_display_port_read(void *owner, lib_u16 port,
    lib_u64 tick, lib_u32 *out_value)
{
    lib_u8 value;
    lib_status status;

    (void)tick;
    status = x86_ct_vga_vga_read(owner, myob300_display_vga_port(port), &value);
    if (status == LIB_STATUS_OK) *out_value = value;
    return status;
}

static lib_status myob300_display_port_write(void *owner, lib_u16 port,
    lib_u8 access_bytes, lib_u32 value)
{
    if (access_bytes != 1u) return LIB_STATUS_UNSUPPORTED;
    return x86_ct_vga_vga_write(owner, myob300_display_vga_port(port), (lib_u8)value);
}

static lib_status myob300_display_memory_query(void *owner, lib_u32 physical,
    lib_uptr bytes, x86_core_machine_memory_access access)
{
    return x86_ct_vga_memory_query(owner, physical, bytes,
        access == X86_CORE_MACHINE_MEMORY_ACCESS_WRITE);
}

static lib_status myob300_display_memory_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    return x86_ct_vga_memory_read(owner, physical, (void *)destination, bytes, observe_only);
}

static lib_status myob300_display_memory_write(void *owner, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    return x86_ct_vga_memory_write(owner, physical, (const void *)source, bytes);
}

lib_status myob300_display_install(x86_core_machine *core, x86_ct_vga *chip)
{
    static const struct {
        lib_u16 port;
        lib_u16 byte_lane_end;
    } decode[] = {
        {0x03b4u, 0x03b6u}, {0x03b5u, 0u}, {0x03bau, 0u},
        {0x03c0u, 0u}, {0x03c1u, 0u}, {0x03c2u, 0u},
        {0x03c4u, 0x03c6u}, {0x03c5u, 0u},
        {0x03c6u, 0u}, {0x03c7u, 0u}, {0x03c8u, 0u}, {0x03c9u, 0u},
        {0x03ccu, 0u}, {0x03ceu, 0x03d0u}, {0x03cfu, 0u},
        {0x03d4u, 0x03d6u}, {0x03d5u, 0u},
        {X86_CT_VGA_EXTENSION_INDEX_PORT, X86_CT_VGA_EXTENSION_DATA_PORT + 1u},
        {X86_CT_VGA_EXTENSION_DATA_PORT, 0u}, {0x03dau, 0u}
    };
    x86_core_machine_port_route ports[sizeof(decode) / sizeof(decode[0])];
    const x86_core_machine_memory_device_route memory = {
        0x000a0000u, 0x00020000u,
        {myob300_display_memory_read, myob300_display_memory_write,
            myob300_display_memory_query},
        X86_CORE_MACHINE_MEMORY_PROVIDER_STANDARD, LIB_NULL
    };
    lib_status status;

    for (lib_size i = 0u; i < sizeof(decode) / sizeof(decode[0]); ++i) {
        ports[i] = (x86_core_machine_port_route){decode[i].port,
            myob300_display_port_read, myob300_display_port_write, chip,
            LIB_FALSE, decode[i].byte_lane_end};
    }
    status = x86_core_machine_install_port_routes(core, ports,
        sizeof(ports) / sizeof(ports[0]));
    if (status != LIB_STATUS_OK) return status;
    status = x86_core_machine_install_memory_device_routes(core, &memory, 1u,
        LIB_NULL, LIB_NULL, chip);
    if (status != LIB_STATUS_OK)
        (void)x86_core_machine_remove_port_routes(core, chip);
    return status;
}
