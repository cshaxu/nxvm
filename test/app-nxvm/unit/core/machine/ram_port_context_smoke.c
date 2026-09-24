#include "lib/types/types_interface.h"
#include <stdio.h>



#include "app-nxvm/devices/memory.h"

#include "app-nxvm/devices/port.h"

lib_i32 main(void)
{
    t_ram first_memory = {0};
    t_ram second_memory = {0};
    t_port *first_port;
    t_port *second_port;
    lib_u8 value = 0x5au;
    lib_u8 observed = 0xffu;
    lib_i32 result = 0;

    first_port = (t_port *)lib_allocate_zero(1u, sizeof(*first_port));
    second_port = (t_port *)lib_allocate_zero(1u, sizeof(*second_port));
    if (first_port == LIB_NULL || second_port == LIB_NULL) {
        lib_release(first_port);
        lib_release(second_port);
        return 1;
    }
    result |= core_machine_memory_initialize_for(&first_memory,
        16u * 1024u * 1024u, LIB_NULL) != LIB_STATUS_OK;
    result |= core_machine_memory_initialize_for(&second_memory,
        16u * 1024u * 1024u, LIB_NULL) != LIB_STATUS_OK;
    core_machine_port_initialize(first_port);
    core_machine_port_initialize(second_port);

    result |= core_machine_memory_allocate_for(&first_memory, 2u * 1024u * 1024u) !=
        LIB_STATUS_OK;
    result |= core_machine_memory_write_physical(&first_memory, 0u,
        (lib_uptr)&value, 1u) != LIB_STATUS_OK;
    result |= core_machine_memory_read_physical(&second_memory, 0u,
        (lib_uptr)&observed, 1u) != LIB_STATUS_OK;
    result |= observed != 0u;
    result |= first_memory.connect.installed_bytes != 2u * 1024u * 1024u;
    result |= core_machine_memory_write_physical(&first_memory,
        2u * 1024u * 1024u, (lib_uptr)&value, 1u) !=
        LIB_STATUS_INTERNAL_ERROR;
    result |= core_machine_memory_write_physical(&first_memory, 0x00100000u,
        (lib_uptr)&value, 1u) != LIB_STATUS_OK;
    observed = 0u;
    result |= core_machine_memory_read_physical(&first_memory, 0u,
        (lib_uptr)&observed, 1u) != LIB_STATUS_OK;
    result |= observed != value;
    first_memory.data.flagA20 = LIB_TRUE;
    value = 0xa5u;
    result |= core_machine_memory_write_physical(&first_memory, 0x00100000u,
        (lib_uptr)&value, 1u) != LIB_STATUS_OK;
    observed = 0u;
    result |= core_machine_memory_read_physical(&first_memory, 0x00100000u,
        (lib_uptr)&observed, 1u) != LIB_STATUS_OK;
    result |= observed != value;
    result |= core_machine_memory_register_mapping(&first_memory, 0xfffffff0u,
        0x000ffff0u, 16u, LIB_FALSE) != LIB_STATUS_OK;
    {
        const lib_uptr mapping_count = first_memory.connect.mapping_count;

        result |= core_machine_memory_register_mapping(&first_memory, 0xfffffff0u,
            0x000ffff0u, 17u, LIB_FALSE) != LIB_STATUS_INVALID_ARGUMENT;
        result |= first_memory.connect.mapping_count != mapping_count;
    }
    value = 0xebu;
    result |= core_machine_memory_write_physical(&first_memory, 0x000ffff0u,
        (lib_uptr)&value, 1u) != LIB_STATUS_OK;
    observed = 0u;
    result |= core_machine_memory_read_physical(&first_memory, 0xfffffff0u,
        (lib_uptr)&observed, 1u) != LIB_STATUS_OK;
    result |= observed != value;
    core_machine_memory_freeze_mappings(&first_memory);
    result |= core_machine_memory_register_mapping(&first_memory, 0xfff00000u,
        0x000f0000u, 16u, LIB_FALSE) != LIB_STATUS_INVALID_ARGUMENT;

    core_machine_port_write(first_port, 0xffffu, 0xa5a55a5au);
    result |= first_port->data.ioDWord != 0xa5a55a5au;
    result |= second_port->data.ioDWord != 0u;

    core_machine_memory_reset(&first_memory);
    observed = 0xffu;
    result |= core_machine_memory_read_physical(&first_memory, 0u,
        (lib_uptr)&observed, 1u) != LIB_STATUS_OK;
    result |= observed != 0u;

    core_machine_memory_finalize(&second_memory);
    core_machine_memory_finalize(&first_memory);
    lib_release(second_port);
    lib_release(first_port);
    if (result != 0) return 1;

    puts("M5:T171:S1:RAM-PORT-CONTEXT:OK");
    puts("M5:T442:S1:PHYSICAL-MAPPING-SPAN-BOUNDARY:OK");
    return 0;
}
