#include "type.h"

#include <stdio.h>


#include "core/machine/memory.h"

#include "core/machine/port.h"

C_INT main(C_VOID)
{
    t_ram first_memory = {0};
    t_ram second_memory = {0};
    t_port first_port = {0};
    t_port second_port = {0};
    ntvdm64_type_unsigned_8 value = 0x5au;
    ntvdm64_type_unsigned_8 observed = 0xffu;
    C_INT result = 0;

    core_machine_memory_initialize(&first_memory);
    core_machine_memory_initialize(&second_memory);
    core_machine_port_initialize(&first_port);
    core_machine_port_initialize(&second_port);

    core_machine_memory_write_physical(&first_memory, 0u,
        (ntvdm64_type_virtual_address)&value, 1u);
    core_machine_memory_read_physical(&second_memory, 0u,
        (ntvdm64_type_virtual_address)&observed, 1u);
    result |= observed != 0u;

    core_machine_port_write(&first_port, 0xffffu, 0xa5a55a5au);
    result |= first_port.data.ioDWord != 0xa5a55a5au;
    result |= second_port.data.ioDWord != 0u;

    core_machine_memory_reset(&first_memory);
    observed = 0xffu;
    core_machine_memory_read_physical(&first_memory, 0u,
        (ntvdm64_type_virtual_address)&observed, 1u);
    result |= observed != 0u;

    core_machine_memory_finalize(&second_memory);
    core_machine_memory_finalize(&first_memory);
    if (result != 0) return 1;

    puts("M5:T67:S1:RAM-PORT-CONTEXT:OK");
    return 0;
}
