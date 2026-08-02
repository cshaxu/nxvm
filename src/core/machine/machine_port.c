#include "core/machine/machine_impl.h"

#include <stdlib.h>

#define NXVM_CORE_PORT_COUNT 65536u

nxvm_core_status nxvm_core_port_initialize(nxvm_core_machine *machine)
{
    if (machine == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    machine->ports.slots = (nxvm_core_port_slot *)calloc(
        NXVM_CORE_PORT_COUNT,
        sizeof(*machine->ports.slots));
    if (machine->ports.slots == NULL) {
        return NXVM_CORE_STATUS_NO_MEMORY;
    }

    return NXVM_CORE_STATUS_OK;
}

void nxvm_core_port_finalize(nxvm_core_machine *machine)
{
    if (machine != NULL) {
        free(machine->ports.slots);
        machine->ports.slots = NULL;
    }
}

nxvm_core_status nxvm_core_machine_install_port(
    nxvm_core_machine *machine,
    uint16_t first,
    uint16_t last,
    const nxvm_core_port_ops *ops,
    void *owner)
{
    uint32_t port;

    if (machine == NULL || ops == NULL || first > last ||
        (ops->read == NULL && ops->write == NULL)) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    for (port = first; port <= last; ++port) {
        if (machine->ports.slots[port].ops.read != NULL ||
            machine->ports.slots[port].ops.write != NULL) {
            return NXVM_CORE_STATUS_INVALID_STATE;
        }
    }

    for (port = first; port <= last; ++port) {
        machine->ports.slots[port].ops = *ops;
        machine->ports.slots[port].owner = owner;
    }

    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_core_machine_port_read(
    nxvm_core_machine *machine,
    uint16_t port,
    uint32_t *out_value)
{
    nxvm_core_port_slot *slot;

    if (machine == NULL || out_value == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    slot = &machine->ports.slots[port];
    if (slot->ops.read == NULL) {
        return NXVM_CORE_STATUS_UNSUPPORTED;
    }

    {
        nxvm_core_status status = slot->ops.read(slot->owner, port, out_value);
        nxvm_core_trace_record(machine, NXVM_CORE_TRACE_PORT_READ, port,
                               status == NXVM_CORE_STATUS_OK ? *out_value : 0u,
                               (uint32_t)status);
        return status;
    }
}

nxvm_core_status nxvm_core_machine_port_write(
    nxvm_core_machine *machine,
    uint16_t port,
    uint32_t value)
{
    nxvm_core_port_slot *slot;

    if (machine == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    slot = &machine->ports.slots[port];
    if (slot->ops.write == NULL) {
        return NXVM_CORE_STATUS_UNSUPPORTED;
    }

    {
        nxvm_core_status status = slot->ops.write(slot->owner, port, value);
        nxvm_core_trace_record(machine, NXVM_CORE_TRACE_PORT_WRITE, port, value,
                               (uint32_t)status);
        return status;
    }
}
