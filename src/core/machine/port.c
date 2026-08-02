/* Copyright 2012-2014 Neko. */

/* VPORT is the hub that connects all devices with the I/O port. */

#include "type.h"

#include <stdlib.h>

#include "core/machine/port.h"

static t_port *coreMachinePort;

struct core_machine_port_provider_entry {
    t_nubit16 port_id;
    t_bool write;
    core_machine_port_handler handler;
    void *owner;
    core_machine_port_provider_entry *next;
};

static core_machine_port_provider_entry *core_machine_port_find_provider(
    t_port *port, t_nubit16 port_id, t_bool write)
{
    core_machine_port_provider_entry *entry;

    if (port == NULL) return NULL;
    for (entry = port->connect.providers; entry != NULL; entry = entry->next) {
        if (entry->port_id == port_id && entry->write == write) return entry;
    }
    return NULL;
}

static void core_machine_port_add_provider(t_port *port, t_nubit16 port_id,
    t_bool write, core_machine_port_handler handler, void *owner)
{
    core_machine_port_provider_entry *entry =
        core_machine_port_find_provider(port, port_id, write);

    if (port == NULL) return;
    if (entry == NULL) {
        entry = (core_machine_port_provider_entry *)malloc(sizeof(*entry));
        if (entry == NULL) return;
        entry->port_id = port_id;
        entry->write = write;
        entry->next = port->connect.providers;
        port->connect.providers = entry;
    }
    entry->handler = handler;
    entry->owner = owner;
}

t_port *core_machine_port_current(void)
{
    return coreMachinePort;
}

void core_machine_port_bind_live(t_port *port)
{
    coreMachinePort = port;
}

void core_machine_port_unbind_live(void)
{
    coreMachinePort = NULL;
}

void vportAddRead(t_nubit16 portId, t_faddrcc fpIn) {
    vport.connect.legacy_read[portId] = fpIn;
}
void vportAddWrite(t_nubit16 portId, t_faddrcc fpOut) {
    vport.connect.legacy_write[portId] = fpOut;
}
void vportExecRead(t_nubit16 portId) {
    core_machine_port_execute_read(core_machine_port_current(), portId);
}
void vportExecWrite(t_nubit16 portId) {
    core_machine_port_execute_write(core_machine_port_current(), portId);
}

void core_machine_port_execute_read(t_port *port, t_nubit16 port_id)
{
    core_machine_port_provider_entry *provider;

    if (port == NULL) return;
    provider = core_machine_port_find_provider(port, port_id, False);
    if (provider != NULL && provider->handler != NULL) {
        provider->handler(port, port_id, provider->owner);
        return;
    }
    ExecFun(port->connect.legacy_read[port_id]);
}

void core_machine_port_execute_write(t_port *port, t_nubit16 port_id)
{
    core_machine_port_provider_entry *provider;

    if (port == NULL) return;
    provider = core_machine_port_find_provider(port, port_id, True);
    if (provider != NULL && provider->handler != NULL) {
        provider->handler(port, port_id, provider->owner);
        return;
    }
    ExecFun(port->connect.legacy_write[port_id]);
}

void core_machine_port_add_read(t_port *port, t_nubit16 port_id,
    core_machine_port_handler handler, void *owner)
{
    core_machine_port_add_provider(port, port_id, False, handler, owner);
}

void core_machine_port_add_write(t_port *port, t_nubit16 port_id,
    core_machine_port_handler handler, void *owner)
{
    core_machine_port_add_provider(port, port_id, True, handler, owner);
}

uint32_t core_machine_port_read(t_port *port, uint16_t port_id)
{
    if (port == NULL) return 0u;
    core_machine_port_execute_read(port, port_id);
    return port->data.ioDWord;
}

void core_machine_port_write(t_port *port, uint16_t port_id, uint32_t value)
{
    if (port == NULL) return;
    port->data.ioDWord = value;
    core_machine_port_execute_write(port, port_id);
}

void vportInit() {
    core_machine_port_initialize(core_machine_port_current());
}
void vportReset() {
    core_machine_port_reset(core_machine_port_current());
}
void vportRefresh() {}
void vportFinal() {
    core_machine_port_finalize(core_machine_port_current());
}

void core_machine_port_initialize(t_port *port)
{
    if (port == NULL) return;
    MEMSET((void *)port, Zero8, sizeof(*port));
}

void core_machine_port_reset(t_port *port)
{
    if (port == NULL) return;
    MEMSET((void *)&port->data, Zero8, sizeof(port->data));
}

void core_machine_port_finalize(t_port *port)
{
    core_machine_port_provider_entry *entry;

    if (port == NULL) return;
    entry = port->connect.providers;
    while (entry != NULL) {
        core_machine_port_provider_entry *next = entry->next;
        free(entry);
        entry = next;
    }
    port->connect.providers = NULL;
}

uint32_t core_machine_port_read_legacy(uint16_t port)
{
    return core_machine_port_read(core_machine_port_current(), port);
}

void core_machine_port_write_legacy(uint16_t port, uint32_t value)
{
    core_machine_port_write(core_machine_port_current(), port, value);
}
