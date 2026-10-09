#include "lib/types/types_interface.h"

#include "core/x86/machine.h"
lib_status core_machine_install_port_routes(core_machine *machine,
    const core_machine_port_route *routes, lib_size count)
{
    core_machine_port_provider_entry *checkpoint;
    lib_size index;
    lib_status status;

    if (!core_machine_configuration_is_open(machine)) return LIB_STATUS_INVALID_STATE;
    if (routes == LIB_NULL || count == 0u) return LIB_STATUS_INVALID_ARGUMENT;
    for (index = 0u; index < count; ++index) {
        if ((routes[index].read == LIB_NULL && routes[index].write == LIB_NULL) ||
            (routes[index].wired_or_read && routes[index].read == LIB_NULL) ||
            (routes[index].byte_lane_end != 0u &&
                ((lib_u32)routes[index].address + 1u >
                    routes[index].byte_lane_end || routes[index].wired_or_read))) {
            return LIB_STATUS_INVALID_ARGUMENT;
        }
    }
    status = core_machine_port_registration_status(&machine->executor_port);
    if (status != LIB_STATUS_OK) return status;
    checkpoint = core_machine_port_registration_begin(&machine->executor_port);
    for (index = 0u; index < count; ++index) {
        status = core_machine_port_add_route(&machine->executor_port,
            &routes[index]);

        if (status != LIB_STATUS_OK) {
            core_machine_port_rollback_registration(&machine->executor_port,
                checkpoint);
            return status;
        }
    }
    return LIB_STATUS_OK;
}

lib_status core_machine_remove_port_routes(core_machine *machine,
    const void *owner)
{
    if (machine == LIB_NULL || owner == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (machine->lifecycle == CORE_MACHINE_RUNNING) return LIB_STATUS_INVALID_STATE;
    core_machine_port_unregister_owner(&machine->executor_port, owner);
    return LIB_STATUS_OK;
}

lib_status core_machine_install_port_provider(
    core_machine *machine,
    lib_u16 first,
    lib_u16 last,
    const core_machine_port_provider *provider,
    void *owner)
{
    lib_u32 port;
    core_machine_port_provider_entry *checkpoint;

    if (!core_machine_configuration_is_open(machine)) {
        return LIB_STATUS_INVALID_STATE;
    }

    if (provider == LIB_NULL || first > last ||
        (provider->read == LIB_NULL && provider->write == LIB_NULL)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }

    checkpoint = core_machine_port_registration_begin(&machine->executor_port);

    for (port = first; port <= last; ++port) {
        if ((provider->read != LIB_NULL && core_machine_port_has_read(
                &machine->executor_port, (lib_u16)port)) ||
            (provider->write != LIB_NULL && core_machine_port_has_write(
                &machine->executor_port, (lib_u16)port))) {
            return LIB_STATUS_INVALID_STATE;
        }
    }

    for (port = first; port <= last; ++port) {
        core_machine_port_route route = {(lib_u16)port, provider->read,
            provider->write, owner, LIB_FALSE, 0u};
        lib_status status = core_machine_port_add_route(&machine->executor_port,
            &route);

        if (status != LIB_STATUS_OK) {
            core_machine_port_rollback_registration(&machine->executor_port,
                checkpoint);
            return status;
        }
    }

    return LIB_STATUS_OK;
}

lib_status core_machine_bus_read(
    core_machine *machine,
    lib_u16 port,
    lib_u32 *out_value)
{
    if (machine == LIB_NULL || out_value == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED) {
        return LIB_STATUS_INVALID_STATE;
    }

    if (!core_machine_port_has_read(&machine->executor_port, port)) {
        return LIB_STATUS_UNSUPPORTED;
    }
    {
        lib_status status = core_machine_port_execute_read(&machine->executor_port,
            port, machine->elapsed_ticks);

        if (status != LIB_STATUS_OK) {
            core_machine_trace_record(machine, CORE_MACHINE_TRACE_PORT_READ, port,
                0u, (lib_u32)status);
            return status;
        }
    }
    *out_value = machine->executor_port.data.ioDWord;
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_PORT_READ, port,
        *out_value, (lib_u32)LIB_STATUS_OK);
    return LIB_STATUS_OK;
}

lib_status core_machine_bus_write(
    core_machine *machine,
    lib_u16 port,
    lib_u32 value)
{
    if (machine == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED) {
        return LIB_STATUS_INVALID_STATE;
    }

    if (!core_machine_port_has_write(&machine->executor_port, port)) {
        return LIB_STATUS_UNSUPPORTED;
    }
    {
        lib_u32 prior_value = machine->executor_port.data.ioDWord;
        lib_status status;

        machine->executor_port.data.ioDWord = value;
        status = core_machine_port_execute_write(&machine->executor_port, port);
        if (status != LIB_STATUS_OK) {
            machine->executor_port.data.ioDWord = prior_value;
            core_machine_trace_record(machine, CORE_MACHINE_TRACE_PORT_WRITE, port,
                value, (lib_u32)status);
            return status;
        }
    }
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_PORT_WRITE, port,
        value, (lib_u32)LIB_STATUS_OK);
    return LIB_STATUS_OK;
}
