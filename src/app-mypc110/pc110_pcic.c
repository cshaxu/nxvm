/* Copyright 2012-2026 Neko. */
#include "app-mypc110/pc110_pcic_interface.h"

static lib_status mypc110_pcic_port_read(void *owner, lib_u16 port,
    lib_u64 tick, lib_u32 *out_value)
{
    lib_u8 value;
    lib_status status;

    (void)tick;
    if (out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_rb5c396_read(owner, port, &value);
    if (status == LIB_STATUS_OK) *out_value = value;
    return status;
}

static lib_status mypc110_pcic_port_write(void *owner, lib_u16 port,
    lib_u8 access_bytes, lib_u32 value)
{
    if (access_bytes != 1u || value > 0xffu) return LIB_STATUS_INVALID_ARGUMENT;
    return x86_rb5c396_write(owner, port, (lib_u8)value);
}

lib_status mypc110_pcic_install_routes(x86_core_machine *machine,
    x86_rb5c396 *controller)
{
    const x86_core_machine_port_route routes[] = {
        {X86_RB5C396_INDEX_PORT, mypc110_pcic_port_read,
            mypc110_pcic_port_write, controller, LIB_FALSE, X86_RB5C396_DATA_PORT},
        {X86_RB5C396_DATA_PORT, mypc110_pcic_port_read,
            mypc110_pcic_port_write, controller, LIB_FALSE, 0u}
    };

    if (machine == LIB_NULL || controller == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return x86_core_machine_install_port_routes(machine, routes,
        sizeof(routes) / sizeof(routes[0]));
}
