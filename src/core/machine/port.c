/* Copyright 2012-2014 Neko. */

/* VPORT is the hub that connects all devices with the I/O port. */

#include "type.h"


#include "core/machine/port.h"

struct core_machine_port_provider_entry {
    type_unsigned_16 port_id;
    type_bool write;
    type_bool wired_or;
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
        if (entry->port_id == port_id && entry->write == write && !entry->wired_or) return entry;
    }
    return STD_NULL;
}

static type_status core_machine_port_add_provider(t_port *port,
    type_unsigned_16 port_id, type_bool write, core_machine_port_handler handler,
    core_machine_port_read_provider read_provider,
    core_machine_port_write_provider write_provider, C_VOID *owner, type_bool wired_or)
{
    core_machine_port_provider_entry *entry;

    if (port == STD_NULL || (handler == STD_NULL && read_provider == STD_NULL &&
        write_provider == STD_NULL)) return TYPE_STATUS_INVALID_ARGUMENT;
    if (port->connect.registration_status != TYPE_STATUS_OK) {
        return port->connect.registration_status;
    }
    entry = core_machine_port_find_provider(port, port_id, write);
    if ((!wired_or && entry != STD_NULL) ||
        (wired_or && (write || entry == STD_NULL))) {
        port->connect.registration_status = TYPE_STATUS_INVALID_STATE;
        return TYPE_STATUS_INVALID_STATE;
    }
    if (port->connect.test_allocation != STD_NULL) {
        ++port->connect.test_allocation->attempts;
        if (port->connect.test_allocation->fail_at != 0u &&
            port->connect.test_allocation->attempts ==
                port->connect.test_allocation->fail_at) {
            port->connect.registration_status = TYPE_STATUS_NO_MEMORY;
            return TYPE_STATUS_NO_MEMORY;
        }
    }
    entry = (core_machine_port_provider_entry *)STD_CALLOC(1u, sizeof(*entry));
    if (entry == STD_NULL) {
        port->connect.registration_status = TYPE_STATUS_NO_MEMORY;
        return TYPE_STATUS_NO_MEMORY;
    }
    entry->port_id = port_id;
    entry->write = write;
    entry->wired_or = wired_or;
    entry->legacy_handler = handler;
    entry->read_provider = read_provider;
    entry->write_provider = write_provider;
    entry->owner = owner;
    entry->next = port->connect.providers;
    port->connect.providers = entry;
    return TYPE_STATUS_OK;
}


static type_status core_machine_port_execute_read_current(t_port *port,
    type_unsigned_16 port_id)
{
    core_machine_port_provider_entry *provider;
    type_unsigned_32 value = 0u;

    if (port == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    provider = core_machine_port_find_provider(port, port_id, TYPE_FALSE);
    if (provider != STD_NULL) {
        if (provider->legacy_handler != STD_NULL) {
            provider->legacy_handler(port, port_id, provider->owner);
            value = port->data.ioDWord;
        } else if (provider->read_provider != STD_NULL) {
            type_status status = provider->read_provider(provider->owner, port_id, &value);

            if (status != TYPE_STATUS_OK) return status;
        }
    }
    for (provider = port->connect.providers; provider != STD_NULL; provider = provider->next) {
        type_unsigned_32 contribution = 0u;
        type_status status;

        if (provider->port_id != port_id || provider->write || !provider->wired_or ||
            provider->read_provider == STD_NULL) continue;
        status = provider->read_provider(provider->owner, port_id, &contribution);
        if (status != TYPE_STATUS_OK) return status;
        value |= contribution;
    }
    port->data.ioDWord = value;
    return TYPE_STATUS_OK;
}
static type_status core_machine_port_execute_write_current(t_port *port,
    type_unsigned_16 port_id)
{
    core_machine_port_provider_entry *provider;

    if (port == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    provider = core_machine_port_find_provider(port, port_id, TYPE_TRUE);
    if (provider != STD_NULL) {
        if (provider->legacy_handler != STD_NULL) {
            provider->legacy_handler(port, port_id, provider->owner);
        } else if (provider->write_provider != STD_NULL) {
            type_status status = provider->write_provider(provider->owner, port_id,
                port->data.ioDWord);

            if (status != TYPE_STATUS_OK) return status;
        }
        return TYPE_STATUS_OK;
    }
    return TYPE_STATUS_OK;
}
type_status core_machine_port_execute_read_width(t_port *port,
    type_unsigned_16 port_id, type_unsigned_8 bytes)
{
    if (port == STD_NULL || (bytes != 1u && bytes != 2u && bytes != 4u) ||
        (type_unsigned_32)port_id + bytes > VPORT_MAX_PORT_COUNT) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    port->data.access_bytes = bytes;
    return core_machine_port_execute_read_current(port, port_id);
}
type_status core_machine_port_execute_write_width(t_port *port,
    type_unsigned_16 port_id, type_unsigned_8 bytes)
{
    if (port == STD_NULL || (bytes != 1u && bytes != 2u && bytes != 4u) ||
        (type_unsigned_32)port_id + bytes > VPORT_MAX_PORT_COUNT) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    port->data.access_bytes = bytes;
    return core_machine_port_execute_write_current(port, port_id);
}

type_status core_machine_port_execute_read(t_port *port, type_unsigned_16 port_id)
{
    if (port == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    port->data.access_bytes = 1u;
    return core_machine_port_execute_read_current(port, port_id);
}

type_status core_machine_port_execute_write(t_port *port, type_unsigned_16 port_id)
{
    if (port == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    port->data.access_bytes = 1u;
    return core_machine_port_execute_write_current(port, port_id);
}

type_status core_machine_port_add_read(t_port *port, type_unsigned_16 port_id,
    core_machine_port_handler handler, C_VOID *owner)
{
    return core_machine_port_add_provider(port, port_id, TYPE_FALSE, handler,
        STD_NULL, STD_NULL, owner, TYPE_FALSE);
}

type_status core_machine_port_add_write(t_port *port, type_unsigned_16 port_id,
    core_machine_port_handler handler, C_VOID *owner)
{
    return core_machine_port_add_provider(port, port_id, TYPE_TRUE, handler,
        STD_NULL, STD_NULL, owner, TYPE_FALSE);
}

type_status core_machine_port_add_read_provider(t_port *port,
    type_unsigned_16 port_id, core_machine_port_read_provider provider,
    C_VOID *owner)
{
    return core_machine_port_add_provider(port, port_id, TYPE_FALSE, STD_NULL,
        provider, STD_NULL, owner, TYPE_FALSE);
}

type_status core_machine_port_add_read_wired_or_provider(t_port *port,
    type_unsigned_16 port_id, core_machine_port_read_provider provider,
    C_VOID *owner)
{
    return core_machine_port_add_provider(port, port_id, TYPE_FALSE, STD_NULL,
        provider, STD_NULL, owner, TYPE_TRUE);
}
type_status core_machine_port_add_write_provider(t_port *port,
    type_unsigned_16 port_id, core_machine_port_write_provider provider,
    C_VOID *owner)
{
    return core_machine_port_add_provider(port, port_id, TYPE_TRUE, STD_NULL,
        STD_NULL, provider, owner, TYPE_FALSE);
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

type_unsigned_32 core_machine_port_read(t_port *port, type_unsigned_16 port_id)
{
    if (port == STD_NULL) return 0u;
    (C_VOID)core_machine_port_execute_read(port, port_id);
    return port->data.ioDWord;
}

C_VOID core_machine_port_write(t_port *port, type_unsigned_16 port_id, type_unsigned_32 value)
{
    if (port == STD_NULL) return;
    port->data.ioDWord = value;
    (C_VOID)core_machine_port_execute_write(port, port_id);
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

core_machine_port_provider_entry *core_machine_port_registration_begin(t_port *port)
{
    if (port == STD_NULL) return STD_NULL;
    port->connect.registration_status = TYPE_STATUS_OK;
    return port->connect.providers;
}

type_status core_machine_port_registration_status(const t_port *port)
{
    return port == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
        port->connect.registration_status;
}

C_VOID core_machine_port_rollback_registration(t_port *port,
    core_machine_port_provider_entry *checkpoint)
{
    if (port == STD_NULL) return;
    while (port->connect.providers != checkpoint) {
        core_machine_port_provider_entry *entry = port->connect.providers;

        if (entry == STD_NULL) break;
        port->connect.providers = entry->next;
        STD_FREE(entry);
    }
    port->connect.registration_status = TYPE_STATUS_OK;
}

C_VOID core_machine_port_set_test_allocation(t_port *port,
    core_machine_port_test_allocation *test_allocation)
{
    if (port != STD_NULL) port->connect.test_allocation = test_allocation;
}
