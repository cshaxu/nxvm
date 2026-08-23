#include "type.h"



#include "core/machine/memory.h"

#include "core/machine/port.h"

C_INT main(C_VOID)
{
    t_ram first_memory = {0};
    t_ram second_memory = {0};
    t_port *first_port;
    t_port *second_port;
    type_unsigned_8 value = 0x5au;
    type_unsigned_8 observed = 0xffu;
    C_INT result = 0;

    first_port = (t_port *)STD_CALLOC(1u, sizeof(*first_port));
    second_port = (t_port *)STD_CALLOC(1u, sizeof(*second_port));
    if (first_port == STD_NULL || second_port == STD_NULL) {
        STD_FREE(first_port);
        STD_FREE(second_port);
        return 1;
    }
    core_machine_memory_initialize(&first_memory);
    core_machine_memory_initialize(&second_memory);
    core_machine_port_initialize(first_port);
    core_machine_port_initialize(second_port);

    result |= core_machine_memory_allocate_for(&first_memory, 2u * 1024u * 1024u) !=
        TYPE_STATUS_OK;
    result |= core_machine_memory_write_physical(&first_memory, 0u,
        (type_virtual_address)&value, 1u) != TYPE_STATUS_OK;
    result |= core_machine_memory_read_physical(&second_memory, 0u,
        (type_virtual_address)&observed, 1u) != TYPE_STATUS_OK;
    result |= observed != 0u;
    result |= first_memory.connect.installed_bytes != 2u * 1024u * 1024u;
    result |= core_machine_memory_write_physical(&first_memory,
        2u * 1024u * 1024u, (type_virtual_address)&value, 1u) !=
        TYPE_STATUS_FAULT;
    result |= core_machine_memory_write_physical(&first_memory, 0x00100000u,
        (type_virtual_address)&value, 1u) != TYPE_STATUS_OK;
    observed = 0u;
    result |= core_machine_memory_read_physical(&first_memory, 0u,
        (type_virtual_address)&observed, 1u) != TYPE_STATUS_OK;
    result |= observed != value;
    first_memory.data.flagA20 = TYPE_TRUE;
    value = 0xa5u;
    result |= core_machine_memory_write_physical(&first_memory, 0x00100000u,
        (type_virtual_address)&value, 1u) != TYPE_STATUS_OK;
    observed = 0u;
    result |= core_machine_memory_read_physical(&first_memory, 0x00100000u,
        (type_virtual_address)&observed, 1u) != TYPE_STATUS_OK;
    result |= observed != value;
    result |= core_machine_memory_register_mapping(&first_memory, 0xfffffff0u,
        0x000ffff0u, 16u) != TYPE_STATUS_OK;
    {
        const type_native_unsigned mapping_count = first_memory.connect.mapping_count;

        result |= core_machine_memory_register_mapping(&first_memory, 0xfffffff0u,
            0x000ffff0u, 17u) != TYPE_STATUS_INVALID_ARGUMENT;
        result |= first_memory.connect.mapping_count != mapping_count;
    }
    value = 0xebu;
    result |= core_machine_memory_write_physical(&first_memory, 0x000ffff0u,
        (type_virtual_address)&value, 1u) != TYPE_STATUS_OK;
    observed = 0u;
    result |= core_machine_memory_read_physical(&first_memory, 0xfffffff0u,
        (type_virtual_address)&observed, 1u) != TYPE_STATUS_OK;
    result |= observed != value;
    core_machine_memory_freeze_mappings(&first_memory);
    result |= core_machine_memory_register_mapping(&first_memory, 0xfff00000u,
        0x000f0000u, 16u) != TYPE_STATUS_INVALID_ARGUMENT;

    core_machine_port_write(first_port, 0xffffu, 0xa5a55a5au);
    result |= first_port->data.ioDWord != 0xa5a55a5au;
    result |= second_port->data.ioDWord != 0u;

    core_machine_memory_reset(&first_memory);
    observed = 0xffu;
    result |= core_machine_memory_read_physical(&first_memory, 0u,
        (type_virtual_address)&observed, 1u) != TYPE_STATUS_OK;
    result |= observed != 0u;

    core_machine_memory_finalize(&second_memory);
    core_machine_memory_finalize(&first_memory);
    STD_FREE(second_port);
    STD_FREE(first_port);
    if (result != 0) return 1;

    puts("M5:T171:S1:RAM-PORT-CONTEXT:OK");
    puts("M5:T442:S1:PHYSICAL-MAPPING-SPAN-BOUNDARY:OK");
    return 0;
}
