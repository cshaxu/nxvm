/* Copyright 2012-2014 Neko. */

/* VPORT is the hub that connects all devices with the I/O port. */

#include "type.h"


#include "core/machine/port.h"

struct core_machine_port_provider_entry {
    type_unsigned_16 port_id;
    type_bool write;
    core_machine_port_handler legacy_handler;
    core_machine_port_read_provider read_provider;
    core_machine_port_write_provider write_provider;
    C_VOID *owner;
    core_machine_port_provider_entry *next;
};

static core_machine_port_provider_entry *core_machine_port_find_provider(
    t_port *port, type_unsigned_16 port_id, type_bool write)
{
    core_machine_port_provider_entry *entry;

    if (port == STD_NULL) return STD_NULL;
    for (entry = port->connect.providers; entry != STD_NULL; entry = entry->next) {
        if (entry->port_id == port_id && entry->write == write) return entry;
    }
    return STD_NULL;
}

static type_status core_machine_port_add_provider(t_port *port,
    type_unsigned_16 port_id, type_bool write, core_machine_port_handler handler,
    core_machine_port_read_provider read_provider,
    core_machine_port_write_provider write_provider, C_VOID *owner)
{
    core_machine_port_provider_entry *entry =
        core_machine_port_find_provider(port, port_id, write);

    if (port == STD_NULL || (handler == STD_NULL && read_provider == STD_NULL &&
        write_provider == STD_NULL)) return TYPE_STATUS_INVALID_ARGUMENT;
    if (entry != STD_NULL) return TYPE_STATUS_INVALID_STATE;
    entry = (core_machine_port_provider_entry *)STD_CALLOC(1u, sizeof(*entry));
    if (entry == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    entry->port_id = port_id;
    entry->write = write;
    entry->legacy_handler = handler;
    entry->read_provider = read_provider;
    entry->write_provider = write_provider;
    entry->owner = owner;
    entry->next = port->connect.providers;
    port->connect.providers = entry;
    return TYPE_STATUS_OK;
}


C_VOID core_machine_port_execute_read(t_port *port, type_unsigned_16 port_id)
{
    core_machine_port_provider_entry *provider;

    if (port == STD_NULL) return;
    provider = core_machine_port_find_provider(port, port_id, TYPE_FALSE);
    if (provider != STD_NULL) {
        if (provider->legacy_handler != STD_NULL) {
            provider->legacy_handler(port, port_id, provider->owner);
        } else if (provider->read_provider != STD_NULL) {
            uint32_t value = 0u;

            (C_VOID)provider->read_provider(provider->owner, port_id, &value);
            port->data.ioDWord = value;
        }
        return;
    }
}

C_VOID core_machine_port_execute_write(t_port *port, type_unsigned_16 port_id)
{
    core_machine_port_provider_entry *provider;

    if (port == STD_NULL) return;
    provider = core_machine_port_find_provider(port, port_id, TYPE_TRUE);
    if (provider != STD_NULL) {
        if (provider->legacy_handler != STD_NULL) {
            provider->legacy_handler(port, port_id, provider->owner);
        } else if (provider->write_provider != STD_NULL) {
            (C_VOID)provider->write_provider(provider->owner, port_id,
                port->data.ioDWord);
        }
        return;
    }
}

type_status core_machine_port_add_read(t_port *port, type_unsigned_16 port_id,
    core_machine_port_handler handler, C_VOID *owner)
{
    return core_machine_port_add_provider(port, port_id, TYPE_FALSE, handler,
        STD_NULL, STD_NULL, owner);
}

type_status core_machine_port_add_write(t_port *port, type_unsigned_16 port_id,
    core_machine_port_handler handler, C_VOID *owner)
{
    return core_machine_port_add_provider(port, port_id, TYPE_TRUE, handler,
        STD_NULL, STD_NULL, owner);
}

type_status core_machine_port_add_read_provider(t_port *port,
    type_unsigned_16 port_id, core_machine_port_read_provider provider,
    C_VOID *owner)
{
    return core_machine_port_add_provider(port, port_id, TYPE_FALSE, STD_NULL,
        provider, STD_NULL, owner);
}

type_status core_machine_port_add_write_provider(t_port *port,
    type_unsigned_16 port_id, core_machine_port_write_provider provider,
    C_VOID *owner)
{
    return core_machine_port_add_provider(port, port_id, TYPE_TRUE, STD_NULL,
        STD_NULL, provider, owner);
}

C_INT core_machine_port_has_read(const t_port *port, type_unsigned_16 port_id)
{
    return core_machine_port_find_provider((t_port *)port, port_id,
        TYPE_FALSE) != STD_NULL;
}

C_INT core_machine_port_has_write(const t_port *port, type_unsigned_16 port_id)
{
    return core_machine_port_find_provider((t_port *)port, port_id,
        TYPE_TRUE) != STD_NULL;
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
    STD_MEMSET((C_VOID *)port, TYPE_ZERO_8, sizeof(*port));
}

C_VOID core_machine_port_reset(t_port *port)
{
    if (port == STD_NULL) return;
    STD_MEMSET((C_VOID *)&port->data, TYPE_ZERO_8, sizeof(port->data));
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
