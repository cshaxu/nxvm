#include "lib/types/types_interface.h"

#include "app-nxvm/devices/machine.h"




lib_status core_machine_register_memory_write_observer(core_machine *machine,
    core_machine_memory_write_observer callback, void *owner)
{
    if (!core_machine_configuration_is_open(machine)) return LIB_STATUS_INVALID_STATE;
    return core_machine_memory_register_write_observer(&machine->executor_memory,
        callback, owner);
}
lib_status core_machine_register_memory_device(core_machine *machine,
    lib_u32 physical_start, lib_size bytes,
    const core_machine_memory_device_callbacks *callbacks, void *owner)
{
    if (callbacks == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (!core_machine_configuration_is_open(machine)) return LIB_STATUS_INVALID_STATE;
    return core_machine_memory_register_overlay_device_provider(&machine->executor_memory,
        physical_start, bytes, callbacks->read, callbacks->write, callbacks->query,
        owner);
}

lib_status core_machine_register_memory_replacement_device(core_machine *machine,
    lib_u32 physical_start, lib_size bytes,
    const core_machine_memory_device_callbacks *callbacks, void *owner)
{
    if (callbacks == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (!core_machine_configuration_is_open(machine)) return LIB_STATUS_INVALID_STATE;
    return core_machine_memory_register_replacement_device_provider(
        &machine->executor_memory, physical_start, bytes, callbacks->read,
        callbacks->write, callbacks->query, owner);
}
lib_status core_machine_memory_read(
    const core_machine *machine,
    lib_u32 physical,
    void *out_data,
    lib_size size)
{
    if (machine == LIB_NULL || out_data == LIB_NULL || size == 0u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED) {
        return LIB_STATUS_INVALID_STATE;
    }

    if (machine->executor_memory.connect.backing == 0u) {
        return LIB_STATUS_INVALID_STATE;
    }
    {
        lib_status status = core_machine_memory_read_physical(
            (t_ram *)&machine->executor_memory, physical,
            (lib_uptr)out_data, size);

        core_machine_trace_record((core_machine *)machine,
            CORE_MACHINE_TRACE_MEMORY_READ, physical, (lib_u32)size,
            (lib_u32)status);
        return status;
    }
}

lib_status core_machine_memory_write(
    core_machine *machine,
    lib_u32 physical,
    const void *data,
    lib_size size)
{
    if (machine == LIB_NULL || !core_machine_mutable_operation_is_allowed(machine) ||
        data == LIB_NULL || size == 0u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED) {
        return LIB_STATUS_INVALID_STATE;
    }

    if (machine->executor_memory.connect.backing == 0u) {
        return LIB_STATUS_INVALID_STATE;
    }
    {
        lib_status status = core_machine_memory_write_physical(
            &machine->executor_memory, physical, (lib_uptr)data, size);

        if (status == LIB_STATUS_OK) {
            core_machine_cpu_execution_invalidate_prefetch(
                &machine->executor_cpu_execution);
        }
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_MEMORY_WRITE,
            physical, (lib_u32)size, (lib_u32)status);
        return status;
    }
}

lib_status core_machine_memory_query(
    const core_machine *machine,
    lib_u32 physical,
    lib_size size,
    core_machine_memory_access access,
    core_machine_memory_route *out_route)
{
    if (machine == LIB_NULL || out_route == LIB_NULL || size == 0u ||
        (access != CORE_MACHINE_MEMORY_ACCESS_READ &&
         access != CORE_MACHINE_MEMORY_ACCESS_WRITE)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED) {
        return LIB_STATUS_INVALID_STATE;
    }
    if (machine->executor_memory.connect.backing == 0u) {
        return LIB_STATUS_INVALID_STATE;
    }
    return core_machine_memory_query_physical(&machine->executor_memory, physical,
        size, access, out_route);
}

lib_status core_machine_set_a20(
    core_machine *machine,
    lib_i32 enabled)
{
    if (machine == LIB_NULL || !core_machine_mutable_operation_is_allowed(machine)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED) {
        return LIB_STATUS_INVALID_STATE;
    }

    if (machine->executor_memory.connect.backing == 0u) {
        return LIB_STATUS_INVALID_STATE;
    }
    machine->executor_memory.data.flagA20 = enabled != 0;
    return LIB_STATUS_OK;
}
