#include "type.h"

#include "core/machine/machine.h"




type_status core_machine_register_memory_write_observer(core_machine *machine,
    core_machine_memory_write_observer callback, C_VOID *owner)
{
    if (!core_machine_configuration_is_open(machine)) return TYPE_STATUS_INVALID_STATE;
    return core_machine_memory_register_write_observer(&machine->executor_memory,
        callback, owner);
}
type_status core_machine_register_memory_device(core_machine *machine,
    type_unsigned_32 physical_start, STD_SIZE_T bytes,
    const core_machine_memory_device_callbacks *callbacks, C_VOID *owner)
{
    if (callbacks == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (!core_machine_configuration_is_open(machine)) return TYPE_STATUS_INVALID_STATE;
    return core_machine_memory_register_overlay_device_provider(&machine->executor_memory,
        physical_start, bytes, callbacks->read, callbacks->write, callbacks->query,
        owner);
}

type_status core_machine_register_memory_replacement_device(core_machine *machine,
    type_unsigned_32 physical_start, STD_SIZE_T bytes,
    const core_machine_memory_device_callbacks *callbacks, C_VOID *owner)
{
    if (callbacks == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (!core_machine_configuration_is_open(machine)) return TYPE_STATUS_INVALID_STATE;
    return core_machine_memory_register_replacement_device_provider(
        &machine->executor_memory, physical_start, bytes, callbacks->read,
        callbacks->write, callbacks->query, owner);
}
type_status core_machine_memory_read(
    const core_machine *machine,
    type_unsigned_32 physical,
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
    {
        type_status status = core_machine_memory_read_physical(
            (t_ram *)&machine->executor_memory, physical,
            (type_virtual_address)out_data, size);

        core_machine_trace_record((core_machine *)machine,
            CORE_MACHINE_TRACE_MEMORY_READ, physical, (type_unsigned_32)size,
            (type_unsigned_32)status);
        return status;
    }
}

type_status core_machine_memory_write(
    core_machine *machine,
    type_unsigned_32 physical,
    const C_VOID *data,
    STD_SIZE_T size)
{
    if (machine == STD_NULL || !core_machine_mutable_operation_is_allowed(machine) ||
        data == STD_NULL || size == 0u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED) {
        return TYPE_STATUS_INVALID_STATE;
    }

    if (machine->executor_memory.connect.backing == 0u) {
        return TYPE_STATUS_INVALID_STATE;
    }
    {
        type_status status = core_machine_memory_write_physical(
            &machine->executor_memory, physical, (type_virtual_address)data, size);

        if (status == TYPE_STATUS_OK) {
            core_machine_cpu_execution_invalidate_prefetch(
                &machine->executor_cpu_execution);
        }
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_MEMORY_WRITE,
            physical, (type_unsigned_32)size, (type_unsigned_32)status);
        return status;
    }
}

type_status core_machine_memory_query(
    const core_machine *machine,
    type_unsigned_32 physical,
    STD_SIZE_T size,
    core_machine_memory_access access,
    core_machine_memory_route *out_route)
{
    if (machine == STD_NULL || out_route == STD_NULL || size == 0u ||
        (access != CORE_MACHINE_MEMORY_ACCESS_READ &&
         access != CORE_MACHINE_MEMORY_ACCESS_WRITE)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if (machine->executor_memory.connect.backing == 0u) {
        return TYPE_STATUS_INVALID_STATE;
    }
    return core_machine_memory_query_physical(&machine->executor_memory, physical,
        size, access, out_route);
}

type_status core_machine_set_a20(
    core_machine *machine,
    C_INT enabled)
{
    if (machine == STD_NULL || !core_machine_mutable_operation_is_allowed(machine)) {
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
