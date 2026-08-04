#include "type.h"

#include "core/machine/machine.h"




type_status core_machine_memory_read(
    const core_machine *machine,
    uint32_t physical,
    C_VOID *out_data,
    STD_SIZE_T size)
{
    if (machine == STD_NULL || out_data == STD_NULL || size == 0u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED) {
        return TYPE_STATUS_INVALID_STATE;
    }

    if (machine->executor_memory.connect.backing == 0u) {
        return TYPE_STATUS_INVALID_STATE;
    }
    return core_machine_memory_read_physical((t_ram *)&machine->executor_memory,
        physical, (type_virtual_address)out_data, size);
}

type_status core_machine_memory_write(
    core_machine *machine,
    uint32_t physical,
    const C_VOID *data,
    STD_SIZE_T size)
{
    if (machine == STD_NULL || data == STD_NULL || size == 0u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED) {
        return TYPE_STATUS_INVALID_STATE;
    }

    if (machine->executor_memory.connect.backing == 0u) {
        return TYPE_STATUS_INVALID_STATE;
    }
    return core_machine_memory_write_physical(&machine->executor_memory, physical,
        (type_virtual_address)data, size);
}

type_status core_machine_set_a20(
    core_machine *machine,
    C_INT enabled)
{
    if (machine == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED) {
        return TYPE_STATUS_INVALID_STATE;
    }

    if (machine->executor_memory.connect.backing == 0u) {
        return TYPE_STATUS_INVALID_STATE;
    }
    machine->executor_memory.data.flagA20 = enabled != 0;
    return TYPE_STATUS_OK;
}
