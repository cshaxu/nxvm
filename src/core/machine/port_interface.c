#include "core/machine/machine.h"

#include <stdlib.h>

#define NXVM_CORE_PORT_COUNT 65536u

ntvdm64_status core_machine_bus_initialize(core_machine *machine)
{
    if (machine == NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }

    machine->ports.slots = (core_machine_port_slot *)STD_CALLOC(
        NXVM_CORE_PORT_COUNT,
        sizeof(*machine->ports.slots));
    if (machine->ports.slots == NULL) {
        return NTVDM64_STATUS_NO_MEMORY;
    }

    return NTVDM64_STATUS_OK;
}

void core_machine_bus_finalize(core_machine *machine)
{
    if (machine != NULL) {
        STD_FREE(machine->ports.slots);
        machine->ports.slots = NULL;
    }
}

ntvdm64_status core_machine_install_port_provider(
    core_machine *machine,
    uint16_t first,
    uint16_t last,
    const core_machine_port_provider *provider,
    void *owner)
{
    uint32_t port;

    if (machine == NULL || provider == NULL || first > last ||
        (provider->read == NULL && provider->write == NULL)) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }

    for (port = first; port <= last; ++port) {
        if (machine->ports.slots[port].provider.read != NULL ||
            machine->ports.slots[port].provider.write != NULL) {
            return NTVDM64_STATUS_INVALID_STATE;
        }
    }

    for (port = first; port <= last; ++port) {
        machine->ports.slots[port].provider = *provider;
        machine->ports.slots[port].owner = owner;
    }

    return NTVDM64_STATUS_OK;
}

ntvdm64_status core_machine_bus_read(
    core_machine *machine,
    uint16_t port,
    uint32_t *out_value)
{
    core_machine_port_slot *slot;

    if (machine == NULL || out_value == NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }

    slot = &machine->ports.slots[port];
    if (slot->provider.read == NULL) {
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

    if (machine == NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }

    slot = &machine->ports.slots[port];
    if (slot->provider.write == NULL) {
        return NTVDM64_STATUS_UNSUPPORTED;
    }

    {
        ntvdm64_status status = slot->provider.write(slot->owner, port, value);
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_PORT_WRITE, port, value,
                               (uint32_t)status);
        return status;
    }
}
