/* Copyright 2012-2014 Neko. */

/* VPORT is the hub that connects all devices with the I/O port. */

#include "type.h"

#include <stdlib.h>

#include "core/machine/port.h"

struct core_machine_port_provider_entry {
    ntvdm64_type_unsigned_16 port_id;
    ntvdm64_type_bool write;
    core_machine_port_handler handler;
    C_VOID *owner;
    core_machine_port_provider_entry *next;
};

static core_machine_port_provider_entry *core_machine_port_find_provider(
    t_port *port, ntvdm64_type_unsigned_16 port_id, ntvdm64_type_bool write)
{
    core_machine_port_provider_entry *entry;

    if (port == STD_NULL) return STD_NULL;
    for (entry = port->connect.providers; entry != STD_NULL; entry = entry->next) {
        if (entry->port_id == port_id && entry->write == write) return entry;
    }
    return STD_NULL;
}

static C_VOID core_machine_port_add_provider(t_port *port, ntvdm64_type_unsigned_16 port_id,
    ntvdm64_type_bool write, core_machine_port_handler handler, C_VOID *owner)
{
    core_machine_port_provider_entry *entry =
        core_machine_port_find_provider(port, port_id, write);

    if (port == STD_NULL) return;
    if (entry == STD_NULL) {
        entry = (core_machine_port_provider_entry *)STD_MALLOC(sizeof(*entry));
        if (entry == STD_NULL) return;
        entry->port_id = port_id;
        entry->write = write;
        entry->next = port->connect.providers;
        port->connect.providers = entry;
    }
    entry->handler = handler;
    entry->owner = owner;
}


C_VOID core_machine_port_execute_read(t_port *port, ntvdm64_type_unsigned_16 port_id)
{
    core_machine_port_provider_entry *provider;

    if (port == STD_NULL) return;
    provider = core_machine_port_find_provider(port, port_id, NTVDM64_TYPE_FALSE);
    if (provider != STD_NULL && provider->handler != STD_NULL) {
        provider->handler(port, port_id, provider->owner);
        return;
    }
    NTVDM64_TYPE_EXECUTE_FUNCTION(port->connect.legacy_read[port_id]);
}

C_VOID core_machine_port_execute_write(t_port *port, ntvdm64_type_unsigned_16 port_id)
{
    core_machine_port_provider_entry *provider;

    if (port == STD_NULL) return;
    provider = core_machine_port_find_provider(port, port_id, NTVDM64_TYPE_TRUE);
    if (provider != STD_NULL && provider->handler != STD_NULL) {
        provider->handler(port, port_id, provider->owner);
        return;
    }
    NTVDM64_TYPE_EXECUTE_FUNCTION(port->connect.legacy_write[port_id]);
}

C_VOID core_machine_port_add_read(t_port *port, ntvdm64_type_unsigned_16 port_id,
    core_machine_port_handler handler, C_VOID *owner)
{
    core_machine_port_add_provider(port, port_id, NTVDM64_TYPE_FALSE, handler, owner);
}

C_VOID core_machine_port_add_write(t_port *port, ntvdm64_type_unsigned_16 port_id,
    core_machine_port_handler handler, C_VOID *owner)
{
    core_machine_port_add_provider(port, port_id, NTVDM64_TYPE_TRUE, handler, owner);
}

uint32_t core_machine_port_read(t_port *port, uint16_t port_id)
{
    if (port == STD_NULL) return 0u;
    core_machine_port_execute_read(port, port_id);
    return port->data.ioDWord;
}

C_VOID core_machine_port_write(t_port *port, uint16_t port_id, uint32_t value)
{
    if (port == STD_NULL) return;
    port->data.ioDWord = value;
    core_machine_port_execute_write(port, port_id);
}


C_VOID core_machine_port_initialize(t_port *port)
{
    if (port == STD_NULL) return;
    STD_MEMSET((C_VOID *)port, NTVDM64_TYPE_ZERO_8, sizeof(*port));
}

C_VOID core_machine_port_reset(t_port *port)
{
    if (port == STD_NULL) return;
    STD_MEMSET((C_VOID *)&port->data, NTVDM64_TYPE_ZERO_8, sizeof(port->data));
}

C_VOID core_machine_port_finalize(t_port *port)
{
    core_machine_port_provider_entry *entry;

    if (port == STD_NULL) return;
    entry = port->connect.providers;
    while (entry != STD_NULL) {
        core_machine_port_provider_entry *next = entry->next;
        STD_FREE(entry);
        entry = next;
    }
    port->connect.providers = STD_NULL;
}
