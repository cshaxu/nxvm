#include "type.h"

#include "core/machine/machine.h"



type_status core_machine_bus_initialize(core_machine *machine)
{
    if (machine == STD_NULL || !core_machine_mutable_operation_is_allowed(machine)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    return TYPE_STATUS_OK;
}

C_VOID core_machine_bus_finalize(core_machine *machine)
{
    (C_VOID)machine;
}

type_status core_machine_install_port_provider(
    core_machine *machine,
    uint16_t first,
    uint16_t last,
    const core_machine_port_provider *provider,
    C_VOID *owner)
{
    uint32_t port;

    if (!core_machine_configuration_is_open(machine)) {
        return TYPE_STATUS_INVALID_STATE;
    }

    if (provider == STD_NULL || first > last ||
        (provider->read == STD_NULL && provider->write == STD_NULL)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }

    for (port = first; port <= last; ++port) {
        if ((provider->read != STD_NULL && core_machine_port_has_read(
                &machine->executor_port, (uint16_t)port)) ||
            (provider->write != STD_NULL && core_machine_port_has_write(
                &machine->executor_port, (uint16_t)port))) {
            return TYPE_STATUS_INVALID_STATE;
        }
    }

    for (port = first; port <= last; ++port) {
        if (provider->read != STD_NULL) {
            type_status status = core_machine_port_add_read_provider(
                &machine->executor_port, (uint16_t)port, provider->read, owner);

            if (status != TYPE_STATUS_OK) return status;
        }
        if (provider->write != STD_NULL) {
            type_status status = core_machine_port_add_write_provider(
                &machine->executor_port, (uint16_t)port, provider->write, owner);

            if (status != TYPE_STATUS_OK) return status;
        }
    }

    return TYPE_STATUS_OK;
}

type_status core_machine_bus_read(
    core_machine *machine,
    uint16_t port,
    uint32_t *out_value)
{
    if (machine == STD_NULL || out_value == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED) {
        return TYPE_STATUS_INVALID_STATE;
    }

    if (!core_machine_port_has_read(&machine->executor_port, port)) {
        return TYPE_STATUS_UNSUPPORTED;
    }
    {
        type_status status = core_machine_port_execute_read(&machine->executor_port,
            port);

        if (status != TYPE_STATUS_OK) {
            core_machine_trace_record(machine, CORE_MACHINE_TRACE_PORT_READ, port,
                0u, (uint32_t)status);
            return status;
        }
    }
    *out_value = machine->executor_port.data.ioDWord;
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_PORT_READ, port,
        *out_value, (uint32_t)TYPE_STATUS_OK);
    return TYPE_STATUS_OK;
}

type_status core_machine_bus_write(
    core_machine *machine,
    uint16_t port,
    uint32_t value)
{
    if (machine == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED) {
        return TYPE_STATUS_INVALID_STATE;
    }

    if (!core_machine_port_has_write(&machine->executor_port, port)) {
        return TYPE_STATUS_UNSUPPORTED;
    }
    {
        uint32_t prior_value = machine->executor_port.data.ioDWord;
        type_status status;

        machine->executor_port.data.ioDWord = value;
        status = core_machine_port_execute_write(&machine->executor_port, port);
        if (status != TYPE_STATUS_OK) {
            machine->executor_port.data.ioDWord = prior_value;
            core_machine_trace_record(machine, CORE_MACHINE_TRACE_PORT_WRITE, port,
                value, (uint32_t)status);
            return status;
        }
    }
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_PORT_WRITE, port,
        value, (uint32_t)TYPE_STATUS_OK);
    return TYPE_STATUS_OK;
}
