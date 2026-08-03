#include "type.h"

#include "core/machine/machine.h"



#define NXVM_CORE_PORT_COUNT 65536u

ntvdm64_status core_machine_bus_initialize(core_machine *machine)
{
    if (machine == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }

    machine->test_ports.slots = (core_machine_port_slot *)STD_CALLOC(
        NXVM_CORE_PORT_COUNT,
        sizeof(*machine->test_ports.slots));
    if (machine->test_ports.slots == STD_NULL) {
        return NTVDM64_STATUS_NO_MEMORY;
    }

    return NTVDM64_STATUS_OK;
}

C_VOID core_machine_bus_finalize(core_machine *machine)
{
    if (machine != STD_NULL) {
        STD_FREE(machine->test_ports.slots);
        machine->test_ports.slots = STD_NULL;
    }
}

ntvdm64_status core_machine_install_port_provider(
    core_machine *machine,
    uint16_t first,
    uint16_t last,
    const core_machine_port_provider *provider,
    C_VOID *owner)
{
    uint32_t port;

    if (machine == STD_NULL || provider == STD_NULL || first > last ||
        (provider->read == STD_NULL && provider->write == STD_NULL)) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }

    for (port = first; port <= last; ++port) {
        if (machine->test_ports.slots[port].provider.read != STD_NULL ||
            machine->test_ports.slots[port].provider.write != STD_NULL) {
            return NTVDM64_STATUS_INVALID_STATE;
        }
    }

    for (port = first; port <= last; ++port) {
        machine->test_ports.slots[port].provider = *provider;
        machine->test_ports.slots[port].owner = owner;
    }

    return NTVDM64_STATUS_OK;
}

ntvdm64_status core_machine_bus_read(
    core_machine *machine,
    uint16_t port,
    uint32_t *out_value)
{
    core_machine_port_slot *slot;

    if (machine == STD_NULL || out_value == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }

    slot = &machine->test_ports.slots[port];
    if (slot->provider.read == STD_NULL) {
        return NTVDM64_STATUS_UNSUPPORTED;
    }

    {
        ntvdm64_status status = slot->provider.read(slot->owner, port, out_value);
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_PORT_READ, port,
                               status == NTVDM64_STATUS_OK ? *out_value : 0u,
                               (uint32_t)status);
        return status;
    }
}

ntvdm64_status core_machine_bus_write(
    core_machine *machine,
    uint16_t port,
    uint32_t value)
{
    core_machine_port_slot *slot;

    if (machine == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }

    slot = &machine->test_ports.slots[port];
    if (slot->provider.write == STD_NULL) {
        return NTVDM64_STATUS_UNSUPPORTED;
    }

    {
        ntvdm64_status status = slot->provider.write(slot->owner, port, value);
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_PORT_WRITE, port, value,
                               (uint32_t)status);
        return status;
    }
}
