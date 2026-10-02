#include "lib/types/types_interface.h"

#include "app-nxvm/devices/machine.h"




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

lib_status core_machine_install_memory_device_routes(core_machine *machine,
    const core_machine_memory_device_route *routes, lib_size count,
    core_machine_memory_write_observer observer,
    const core_machine_memory_parity_config *parity, void *owner)
{
    t_ram *memory;
    lib_size provider_count;
    lib_size observer_count;
    lib_status status = LIB_STATUS_OK;

    if (!core_machine_configuration_is_open(machine)) return LIB_STATUS_INVALID_STATE;
    if (owner == LIB_NULL || (count == 0u && observer == LIB_NULL &&
        parity == LIB_NULL) ||
        (count != 0u && routes == LIB_NULL)) return LIB_STATUS_INVALID_ARGUMENT;
    memory = &machine->executor_memory;
    for (lib_size index = 0u; index < memory->connect.device_provider_count; ++index)
        if (memory->connect.device_providers[index].owner == owner)
            return LIB_STATUS_INVALID_STATE;
    for (lib_size index = 0u; index < memory->connect.write_observer_count; ++index)
        if (memory->connect.write_observers[index].owner == owner)
            return LIB_STATUS_INVALID_STATE;
    provider_count = memory->connect.device_provider_count;
    observer_count = memory->connect.write_observer_count;
    if (parity != LIB_NULL) {
        status = core_machine_memory_enable_parity(memory, parity->bytes,
            parity->fault, owner);
        if (status != LIB_STATUS_OK) return status;
    }
    for (lib_size index = 0u; index < count; ++index) {
        const core_machine_memory_device_route *route = &routes[index];

        status = route->replacement ?
            core_machine_memory_register_replacement_device_provider(memory,
                route->physical_start, route->bytes, route->callbacks.read,
                route->callbacks.write, route->callbacks.query, owner) :
            core_machine_memory_register_device_provider(memory,
                route->physical_start, route->bytes, route->callbacks.read,
                route->callbacks.write, route->callbacks.query, owner);
        if (status != LIB_STATUS_OK) break;
    }
    if (status == LIB_STATUS_OK && observer != LIB_NULL)
        status = core_machine_memory_register_write_observer(memory, observer, owner);
    if (status != LIB_STATUS_OK) {
        memory->connect.device_provider_count = provider_count;
        memory->connect.write_observer_count = observer_count;
        if (parity != LIB_NULL) core_machine_memory_release_parity(memory);
    }
    return status;
}

lib_status core_machine_remove_memory_device_routes(core_machine *machine,
    const void *owner)
{
    if (machine == LIB_NULL || owner == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (machine->lifecycle == CORE_MACHINE_RUNNING) return LIB_STATUS_INVALID_STATE;
    core_machine_memory_unregister_owner(&machine->executor_memory, owner);
    if (machine->executor_memory.connect.parity_owner == owner)
        core_machine_memory_release_parity(&machine->executor_memory);
    return LIB_STATUS_OK;
}

lib_status core_machine_memory_inspect(const core_machine *machine,
    lib_u32 physical, void *out_data, lib_size size)
{
    if (machine == LIB_NULL || out_data == LIB_NULL || size == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (machine->lifecycle == CORE_MACHINE_RUNNING ||
        machine->executor_memory.connect.backing == 0u)
        return LIB_STATUS_INVALID_STATE;
    return core_machine_memory_inspect_physical(
        (t_ram *)&machine->executor_memory, physical, (lib_uptr)out_data,
        size, LIB_FALSE);
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
                machine->executor_cpu_execution);
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
