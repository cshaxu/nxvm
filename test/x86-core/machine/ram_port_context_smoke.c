#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "x86-core/machine/memory.h"
#include "x86-core/machine/port.h"
#include "x86-core/machine/machine.h"

static lib_status registration_read(void *owner, lib_u16 address, lib_u64 tick,
    lib_u32 *value)
{
    (void)address;
    (void)tick;
    *value = *(lib_u8 *)owner;
    return LIB_STATUS_OK;
}

static lib_status registration_write(void *owner, lib_u16 address,
    lib_u8 access_bytes, lib_u32 value)
{
    (void)access_bytes;
    (void)address;
    *(lib_u8 *)owner = (lib_u8)value;
    return LIB_STATUS_OK;
}

static lib_i32 port_registration_atomicity(void)
{
    lib_i32 failed = 0;

    for (lib_size fail_at = 1u; fail_at <= 9u; ++fail_at) {
        x86_core_machine machine = {.lifecycle = X86_CORE_MACHINE_INITIALIZED};
        t_port *port = &machine.executor_port;
        x86_core_machine_port_test_allocation allocation = {fail_at, 0u};
        lib_u8 sentinel = 0x5au;
        lib_u8 owner = 0x3cu;
        const x86_core_machine_port_route saved = {
            0x80u, registration_read, registration_write, &sentinel, LIB_FALSE, 0u
        };
        const x86_core_machine_port_route routes[] = {
            {0x100u, registration_read, registration_write, &owner, LIB_FALSE, 0u},
            {0x101u, registration_read, registration_write, &owner, LIB_FALSE, 0u},
            {0x102u, registration_read, registration_write, &owner, LIB_FALSE, 0u},
            {0x103u, registration_read, registration_write, &owner, LIB_FALSE, 0u}
        };
        const lib_bool exhausted = fail_at <= 8u;

        x86_core_machine_port_initialize(port);
        failed |= x86_core_machine_install_port_routes(&machine, &saved, 1u) != LIB_STATUS_OK;
        x86_core_machine_port_set_test_allocation(port, &allocation);
        failed |= x86_core_machine_install_port_routes(&machine, routes, 4u) !=
                (exhausted ? LIB_STATUS_NO_MEMORY : LIB_STATUS_OK) ||
            allocation.attempts != (exhausted ? fail_at : 8u) ||
            x86_core_machine_port_registration_status(port) != LIB_STATUS_OK ||
            x86_core_machine_port_read(port, 0x80u) != sentinel;
        for (lib_size index = 0u; index < 4u; ++index)
            failed |= x86_core_machine_port_has_read(port, routes[index].address) != !exhausted ||
                x86_core_machine_port_has_write(port, routes[index].address) != !exhausted;
        allocation.fail_at = 0u;
        if (exhausted)
            failed |= x86_core_machine_install_port_routes(&machine, routes, 4u) != LIB_STATUS_OK;
        failed |= x86_core_machine_remove_port_routes(&machine, &owner) != LIB_STATUS_OK;
        for (lib_size index = 0u; index < 4u; ++index)
            failed |= x86_core_machine_port_has_read(port, routes[index].address) ||
                x86_core_machine_port_has_write(port, routes[index].address);
        /* Sticky failure belongs to the private single-entry mechanism, not
         * to a DMA consumer of the all-or-none public batch. */
        failed |= x86_core_machine_port_add_route(port,
                &(x86_core_machine_port_route){.address = 0x80u,
                    .write = registration_write, .owner = &owner}) != LIB_STATUS_INVALID_STATE ||
            x86_core_machine_install_port_routes(&machine, routes, 4u) != LIB_STATUS_INVALID_STATE ||
            x86_core_machine_port_registration_status(port) != LIB_STATUS_INVALID_STATE ||
            x86_core_machine_port_read(port, 0x80u) != sentinel;
        x86_core_machine_port_finalize(port);
    }
    return failed;
}

lib_i32 main(void)
{
    t_ram first_memory = {0};
    t_ram second_memory = {0};
    t_port *first_port;
    t_port *second_port;
    lib_u8 value = 0x5au;
    lib_u8 observed = 0xffu;
    lib_i32 result = port_registration_atomicity();

    first_port = (t_port *)lib_allocate_zero(1u, sizeof(*first_port));
    second_port = (t_port *)lib_allocate_zero(1u, sizeof(*second_port));
    if (first_port == LIB_NULL || second_port == LIB_NULL) {
        lib_release(first_port);
        lib_release(second_port);
        return 1;
    }
    result |= x86_core_machine_memory_initialize_for(&first_memory,
        16u * 1024u * 1024u, LIB_NULL) != LIB_STATUS_OK;
    result |= x86_core_machine_memory_initialize_for(&second_memory,
        16u * 1024u * 1024u, LIB_NULL) != LIB_STATUS_OK;
    result |= first_memory.connect.installed_bytes != 16u * 1024u * 1024u;
    first_memory.connect.address_decode = (x86_core_machine_address_decode){
        0u, LIB_UINT32_MAX, 0x00100000u};
    first_memory.data.address_clear_mask = 0x00100000u;
    x86_core_machine_port_initialize(first_port);
    x86_core_machine_port_initialize(second_port);

    result |= x86_core_machine_memory_allocate_for(&first_memory, 2u * 1024u * 1024u) !=
        LIB_STATUS_OK;
    result |= x86_core_machine_memory_write_physical(&first_memory, 0u,
        (lib_uptr)&value, 1u) != LIB_STATUS_OK;
    result |= x86_core_machine_memory_read_physical(&first_memory, 0u,
        (lib_uptr)&observed, 1u) != LIB_STATUS_OK;
    result |= observed != value;
    result |= x86_core_machine_memory_read_physical(&second_memory, 0u,
        (lib_uptr)&observed, 1u) != LIB_STATUS_OK;
    result |= observed != 0u;
    result |= first_memory.connect.installed_bytes != 2u * 1024u * 1024u;
    result |= x86_core_machine_memory_write_physical(&first_memory,
        2u * 1024u * 1024u, (lib_uptr)&value, 1u) !=
        LIB_STATUS_INTERNAL_ERROR;
    result |= x86_core_machine_memory_write_physical(&first_memory, 0x00100000u,
        (lib_uptr)&value, 1u) != LIB_STATUS_OK;
    observed = 0u;
    result |= x86_core_machine_memory_read_physical(&first_memory, 0u,
        (lib_uptr)&observed, 1u) != LIB_STATUS_OK;
    result |= observed != value;
    first_memory.data.address_clear_mask = 0u;
    value = 0xa5u;
    result |= x86_core_machine_memory_write_physical(&first_memory, 0x00100000u,
        (lib_uptr)&value, 1u) != LIB_STATUS_OK;
    observed = 0u;
    result |= x86_core_machine_memory_read_physical(&first_memory, 0x00100000u,
        (lib_uptr)&observed, 1u) != LIB_STATUS_OK;
    result |= observed != value;
    result |= x86_core_machine_memory_register_mapping(&first_memory, 0xfffffff0u,
        0x000ffff0u, 16u, LIB_FALSE) != LIB_STATUS_OK;
    {
        const lib_uptr mapping_count = first_memory.connect.mapping_count;

        result |= x86_core_machine_memory_register_mapping(&first_memory, 0xfffffff0u,
            0x000ffff0u, 17u, LIB_FALSE) != LIB_STATUS_INVALID_ARGUMENT;
        result |= first_memory.connect.mapping_count != mapping_count;
    }
    value = 0xebu;
    result |= x86_core_machine_memory_write_physical(&first_memory, 0x000ffff0u,
        (lib_uptr)&value, 1u) != LIB_STATUS_OK;
    observed = 0u;
    result |= x86_core_machine_memory_read_physical(&first_memory, 0xfffffff0u,
        (lib_uptr)&observed, 1u) != LIB_STATUS_OK;
    result |= observed != value;
    x86_core_machine_memory_freeze_mappings(&first_memory);
    result |= x86_core_machine_memory_register_mapping(&first_memory, 0xfff00000u,
        0x000f0000u, 16u, LIB_FALSE) != LIB_STATUS_INVALID_ARGUMENT;

    x86_core_machine_port_write(first_port, 0xffffu, 0xa5a55a5au);
    result |= first_port->data.ioDWord != 0xa5a55a5au;
    result |= second_port->data.ioDWord != 0u;

    x86_core_machine_memory_reset(&first_memory);
    observed = 0xffu;
    result |= x86_core_machine_memory_read_physical(&first_memory, 0u,
        (lib_uptr)&observed, 1u) != LIB_STATUS_OK;
    result |= observed != 0u;

    x86_core_machine_memory_finalize(&second_memory);
    x86_core_machine_memory_finalize(&first_memory);
    lib_release(second_port);
    lib_release(first_port);
    if (result != 0) return 1;

    lib_c_printf("%s\n", "RAM-PORT-CONTEXT:OK");
    lib_c_printf("%s\n", "PHYSICAL-MAPPING-SPAN-BOUNDARY:OK");
    return 0;
}
