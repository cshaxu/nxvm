#include "lib/types/types_interface.h"

#include "x86-core/machine/machine.h"




lib_status x86_core_machine_install_memory_aliases(x86_core_machine *machine,
    const x86_core_machine_memory_alias_config *aliases, lib_size count,
    lib_bool selected)
{
    lib_size boundary;

    if (!x86_core_machine_configuration_is_open(machine)) return LIB_STATUS_INVALID_STATE;
    if ((count != 0u && aliases == LIB_NULL) ||
        (selected != LIB_FALSE && selected != LIB_TRUE)) return LIB_STATUS_INVALID_ARGUMENT;
    boundary = machine->executor_memory.connect.mapping_count;
    for (lib_size index = 0u; index < count; ++index) {
        const x86_core_machine_memory_alias_config *alias = &aliases[index];
        lib_status status = x86_core_machine_memory_register_mapping(
            &machine->executor_memory, alias->physical_start,
            alias->backing_start, alias->bytes, selected);

        if (status != LIB_STATUS_OK) {
            machine->executor_memory.connect.mapping_count = boundary;
            return status;
        }
    }
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_set_memory_alias_decode(x86_core_machine *machine,
    lib_u32 physical_start, lib_size bytes, lib_bool selected,
    lib_bool writable)
{
    if (machine == LIB_NULL || bytes == 0u ||
        (selected != LIB_FALSE && selected != LIB_TRUE) ||
        (writable != LIB_FALSE && writable != LIB_TRUE)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    for (lib_size index = 0u;
        index < machine->executor_memory.connect.mapping_count; ++index) {
        x86_core_machine_memory_mapping *mapping =
            &machine->executor_memory.connect.mappings[index];

        if (mapping->physical_start != physical_start || mapping->bytes != bytes) continue;
        mapping->selected = selected;
        mapping->writable = writable;
        return LIB_STATUS_OK;
    }
    return LIB_STATUS_INVALID_ARGUMENT;
}

lib_status x86_core_machine_install_memory_device_routes(x86_core_machine *machine,
    const x86_core_machine_memory_device_route *routes, lib_size count,
    x86_core_machine_memory_write_observer observer,
    const x86_core_machine_memory_parity_config *parity, void *owner)
{
    t_ram *memory;
    lib_size provider_count;
    lib_size observer_count;
    lib_status status = LIB_STATUS_OK;

    if (!x86_core_machine_configuration_is_open(machine) &&
        (machine == LIB_NULL || machine->lifecycle != X86_CORE_MACHINE_INITIALIZED ||
        machine->execution_provider_frozen || !machine->firmware_operation_active ||
        !machine->firmware_context.active ||
        !machine->firmware_context.configuring ||
        machine->firmware_context.machine != machine)) return LIB_STATUS_INVALID_STATE;
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
        status = x86_core_machine_memory_enable_parity(memory, parity->bytes,
            parity->fault, owner);
        if (status != LIB_STATUS_OK) return status;
    }
    for (lib_size index = 0u; index < count; ++index) {
        const x86_core_machine_memory_device_route *route = &routes[index];

        switch (route->mode) {
        case X86_CORE_MACHINE_MEMORY_PROVIDER_STANDARD:
            status = x86_core_machine_memory_register_device_provider(memory,
                route->physical_start, route->bytes, route->callbacks.read,
                route->callbacks.write, route->callbacks.query, owner, route->failure);
            break;
        case X86_CORE_MACHINE_MEMORY_PROVIDER_OVERLAY:
            status = x86_core_machine_memory_register_overlay_device_provider(memory,
                route->physical_start, route->bytes, route->callbacks.read,
                route->callbacks.write, route->callbacks.query, owner, route->failure);
            break;
        case X86_CORE_MACHINE_MEMORY_PROVIDER_RESET_OVERLAY:
        case X86_CORE_MACHINE_MEMORY_PROVIDER_CPU_OVERLAY:
            status = x86_core_machine_memory_register_raw_overlay_device_provider(
                memory, route->physical_start, route->bytes,
                route->callbacks.read, route->callbacks.write,
                route->callbacks.query, owner, route->failure);
            if (status == LIB_STATUS_OK)
                memory->connect.device_providers[memory->connect.device_provider_count - 1u].cpu_only =
                    route->mode == X86_CORE_MACHINE_MEMORY_PROVIDER_CPU_OVERLAY;
            break;
        case X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT:
            status = x86_core_machine_memory_register_replacement_device_provider(memory,
                route->physical_start, route->bytes, route->callbacks.read,
                route->callbacks.write, route->callbacks.query, owner, route->failure);
            break;
        case X86_CORE_MACHINE_MEMORY_PROVIDER_FALLBACK:
            status = x86_core_machine_memory_register_fallback_device_provider(memory,
                route->physical_start, route->bytes, route->callbacks.read,
                route->callbacks.write, route->callbacks.query, owner, route->failure);
            break;
        default:
            status = LIB_STATUS_INVALID_ARGUMENT;
            break;
        }
        if (status != LIB_STATUS_OK) break;
    }
    if (status == LIB_STATUS_OK && observer != LIB_NULL)
        status = x86_core_machine_memory_register_write_observer(memory, observer, owner);
    if (status != LIB_STATUS_OK) {
        memory->connect.device_provider_count = provider_count;
        memory->connect.write_observer_count = observer_count;
        if (parity != LIB_NULL) x86_core_machine_memory_release_parity(memory);
    }
    return status;
}

lib_status x86_core_machine_set_memory_instruction_read(x86_core_machine *machine,
    const void *owner, x86_core_machine_memory_device_instruction_read read)
{
    lib_bool found = LIB_FALSE;

    if (!x86_core_machine_configuration_is_open(machine) || owner == LIB_NULL ||
        read == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    for (lib_size index = 0u;
        index < machine->executor_memory.connect.device_provider_count; ++index) {
        x86_core_machine_memory_device_provider *provider =
            &machine->executor_memory.connect.device_providers[index];

        if (provider->owner != owner) continue;
        provider->read_instruction = read;
        found = LIB_TRUE;
    }
    return found ? LIB_STATUS_OK : LIB_STATUS_INVALID_ARGUMENT;
}

lib_status x86_core_machine_remove_memory_device_routes(x86_core_machine *machine,
    const void *owner)
{
    if (machine == LIB_NULL || owner == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (machine->lifecycle == X86_CORE_MACHINE_RUNNING) return LIB_STATUS_INVALID_STATE;
    x86_core_machine_memory_unregister_owner(&machine->executor_memory, owner);
    if (machine->executor_memory.connect.parity_owner == owner)
        x86_core_machine_memory_release_parity(&machine->executor_memory);
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_memory_inspect(const x86_core_machine *machine,
    lib_u32 physical, void *out_data, lib_size size)
{
    if (machine == LIB_NULL || out_data == LIB_NULL || size == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (machine->lifecycle == X86_CORE_MACHINE_RUNNING ||
        machine->executor_memory.connect.backing == 0u)
        return LIB_STATUS_INVALID_STATE;
    return x86_core_machine_memory_inspect_physical(
        (t_ram *)&machine->executor_memory, physical, (lib_uptr)out_data,
        size, LIB_FALSE);
}

lib_status x86_core_machine_memory_read(
    const x86_core_machine *machine,
    lib_u32 physical,
    void *out_data,
    lib_size size)
{
    if (machine == LIB_NULL || out_data == LIB_NULL || size == 0u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle != X86_CORE_MACHINE_STOPPED &&
        machine->lifecycle != X86_CORE_MACHINE_PAUSED) {
        return LIB_STATUS_INVALID_STATE;
    }

    if (machine->executor_memory.connect.backing == 0u) {
        return LIB_STATUS_INVALID_STATE;
    }
    {
        lib_status status = x86_core_machine_memory_read_physical(
            (t_ram *)&machine->executor_memory, physical,
            (lib_uptr)out_data, size, LIB_TRUE);

        x86_core_machine_trace_record((x86_core_machine *)machine,
            X86_CORE_MACHINE_TRACE_MEMORY_READ, physical, (lib_u32)size,
            (lib_u32)status);
        return status;
    }
}

lib_status x86_core_machine_memory_write(
    x86_core_machine *machine,
    lib_u32 physical,
    const void *data,
    lib_size size)
{
    if (machine == LIB_NULL || !x86_core_machine_mutable_operation_is_allowed(machine) ||
        data == LIB_NULL || size == 0u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle != X86_CORE_MACHINE_STOPPED &&
        machine->lifecycle != X86_CORE_MACHINE_PAUSED) {
        return LIB_STATUS_INVALID_STATE;
    }

    if (machine->executor_memory.connect.backing == 0u) {
        return LIB_STATUS_INVALID_STATE;
    }
    {
        lib_status status = x86_core_machine_memory_write_physical(
            &machine->executor_memory, physical, (lib_uptr)data, size, LIB_TRUE);

        if (status == LIB_STATUS_OK) {
            x86_core_cpu_execution_invalidate_prefetch(
                machine->executor_cpu_execution);
        }
        x86_core_machine_trace_record(machine, X86_CORE_MACHINE_TRACE_MEMORY_WRITE,
            physical, (lib_u32)size, (lib_u32)status);
        return status;
    }
}

lib_status x86_core_machine_memory_query(
    const x86_core_machine *machine,
    lib_u32 physical,
    lib_size size,
    x86_core_machine_memory_access access,
    x86_core_machine_memory_route *out_route)
{
    if (machine == LIB_NULL || out_route == LIB_NULL || size == 0u ||
        (access != X86_CORE_MACHINE_MEMORY_ACCESS_READ &&
         access != X86_CORE_MACHINE_MEMORY_ACCESS_WRITE)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (machine->lifecycle != X86_CORE_MACHINE_STOPPED &&
        machine->lifecycle != X86_CORE_MACHINE_PAUSED) {
        return LIB_STATUS_INVALID_STATE;
    }
    if (machine->executor_memory.connect.backing == 0u) {
        return LIB_STATUS_INVALID_STATE;
    }
    return x86_core_machine_memory_query_physical(&machine->executor_memory, physical,
        size, access, out_route, LIB_TRUE);
}

lib_status x86_core_machine_set_address_clear_mask(
    x86_core_machine *machine,
    lib_u32 clear_mask)
{
    if (machine == LIB_NULL || !x86_core_machine_mutable_operation_is_allowed(machine)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle != X86_CORE_MACHINE_STOPPED &&
        machine->lifecycle != X86_CORE_MACHINE_PAUSED) {
        return LIB_STATUS_INVALID_STATE;
    }

    return x86_core_machine_signal_address_clear_mask(machine, clear_mask);
}

lib_status x86_core_machine_signal_address_clear_mask(x86_core_machine *machine,
    lib_u32 clear_mask)
{
    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (machine->executor_memory.connect.backing == 0u)
        return LIB_STATUS_INVALID_STATE;
    machine->executor_memory.data.address_clear_mask = clear_mask;
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_observe_address_clear_mask(const x86_core_machine *machine,
    lib_u32 *out_clear_mask)
{
    if (machine == LIB_NULL || out_clear_mask == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (machine->executor_memory.connect.backing == 0u)
        return LIB_STATUS_INVALID_STATE;
    *out_clear_mask = machine->executor_memory.data.address_clear_mask;
    return LIB_STATUS_OK;
}

void x86_core_machine_signal_memory_decode_change(x86_core_machine *machine,
    lib_u32 physical, lib_size bytes)
{
    if (machine == LIB_NULL) return;
    x86_core_cpu_execution_invalidate_memory_view(machine->executor_cpu_execution,
        physical, bytes);
}

lib_status x86_core_machine_dma_memory_cycle(x86_core_machine *machine,
    lib_u32 physical, lib_u8 bytes, lib_u8 channel,
    x86_core_machine_memory_access access, lib_u16 *value,
    x86_core_machine_dma_device_effect before_memory,
    x86_core_machine_dma_device_effect after_memory, void *device_owner)
{
    x86_core_machine_memory_route route;
    lib_status status;

    if (machine == LIB_NULL || value == LIB_NULL || (bytes != 1u && bytes != 2u) ||
        (access != X86_CORE_MACHINE_MEMORY_ACCESS_READ &&
         access != X86_CORE_MACHINE_MEMORY_ACCESS_WRITE))
        return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_core_machine_memory_query_physical(&machine->executor_memory,
        physical, bytes, access, &route, LIB_FALSE);
    if (status != LIB_STATUS_OK) return status;
    status = x86_core_machine_transaction_begin(&machine->transaction,
        X86_CORE_MACHINE_TRANSACTION_OWNER_DMA,
        access == X86_CORE_MACHINE_MEMORY_ACCESS_WRITE ?
            X86_CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE :
            X86_CORE_MACHINE_TRANSACTION_DMA_MEMORY_READ,
        physical, bytes, channel);
    if (status != LIB_STATUS_OK) return status;
    if (before_memory != LIB_NULL)
        before_memory(device_owner, channel, value);
    status = access == X86_CORE_MACHINE_MEMORY_ACCESS_WRITE ?
        x86_core_machine_memory_write_physical(&machine->executor_memory,
            physical, (lib_uptr)value, bytes, LIB_FALSE) :
        x86_core_machine_memory_read_physical(&machine->executor_memory,
            physical, (lib_uptr)value, bytes, LIB_FALSE);
    if (status != LIB_STATUS_OK) {
        x86_core_machine_transaction_cancel(&machine->transaction);
        return status;
    }
    if (after_memory != LIB_NULL)
        after_memory(device_owner, channel, value);
    x86_core_machine_transaction_commit(&machine->transaction);
    return LIB_STATUS_OK;
}
