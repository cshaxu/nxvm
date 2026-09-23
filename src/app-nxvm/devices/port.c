/* Copyright 2012-2014 Neko. */

/* VPORT is the hub that connects all devices with the I/O port. */
#include "lib/types/types_interface.h"

#include "type.h"


#include "app-nxvm/devices/port.h"

struct core_machine_port_provider_entry {
    lib_u16 port_id;
    type_bool write;
    type_bool wired_or;
    core_machine_port_handler legacy_handler;
    core_machine_port_read_provider read_provider;
    core_machine_port_write_provider write_provider;
    C_VOID *owner;
    core_machine_port_provider_entry *next;
};

static core_machine_port_provider_entry *core_machine_port_find_provider(
    t_port *port, lib_u16 port_id, type_bool write)
{
    core_machine_port_provider_entry *entry;

    if (port == LIB_NULL) return LIB_NULL;
    for (entry = port->connect.providers; entry != LIB_NULL; entry = entry->next) {
        if (entry->port_id == port_id && entry->write == write && !entry->wired_or) return entry;
    }
    return LIB_NULL;
}

static type_status core_machine_port_add_provider(t_port *port,
    lib_u16 port_id, type_bool write, core_machine_port_handler handler,
    core_machine_port_read_provider read_provider,
    core_machine_port_write_provider write_provider, C_VOID *owner, type_bool wired_or)
{
    core_machine_port_provider_entry *entry;

    if (port == LIB_NULL || (handler == LIB_NULL && read_provider == LIB_NULL &&
        write_provider == LIB_NULL)) return TYPE_STATUS_INVALID_ARGUMENT;
    if (port->connect.registration_status != TYPE_STATUS_OK) {
        return port->connect.registration_status;
    }
    entry = core_machine_port_find_provider(port, port_id, write);
    if ((!wired_or && entry != LIB_NULL) ||
        (wired_or && (write || entry == LIB_NULL))) {
        port->connect.registration_status = TYPE_STATUS_INVALID_STATE;
        return TYPE_STATUS_INVALID_STATE;
    }
    if (port->connect.test_allocation != LIB_NULL) {
        ++port->connect.test_allocation->attempts;
        if (port->connect.test_allocation->fail_at != 0u &&
            port->connect.test_allocation->attempts ==
                port->connect.test_allocation->fail_at) {
            port->connect.registration_status = TYPE_STATUS_NO_MEMORY;
            return TYPE_STATUS_NO_MEMORY;
        }
    }
    entry = (core_machine_port_provider_entry *)lib_allocate_zero(1u, sizeof(*entry));
    if (entry == LIB_NULL) {
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
    lib_u16 port_id)
{
    core_machine_port_provider_entry *provider;
    lib_u32 value = 0u;

    if (port == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    provider = core_machine_port_find_provider(port, port_id, LIB_FALSE);
    if (provider != LIB_NULL) {
        if (provider->legacy_handler != LIB_NULL) {
            provider->legacy_handler(port, port_id, provider->owner);
            value = port->data.ioDWord;
        } else if (provider->read_provider != LIB_NULL) {
            type_status status = provider->read_provider(provider->owner, port_id, &value);

            if (status != TYPE_STATUS_OK) return status;
        }
    }
    for (provider = port->connect.providers; provider != LIB_NULL; provider = provider->next) {
        lib_u32 contribution = 0u;
        type_status status;

        if (provider->port_id != port_id || provider->write || !provider->wired_or ||
            provider->read_provider == LIB_NULL) continue;
        status = provider->read_provider(provider->owner, port_id, &contribution);
        if (status != TYPE_STATUS_OK) return status;
        value |= contribution;
    }
    port->data.ioDWord = value;
    return TYPE_STATUS_OK;
}
static type_status core_machine_port_execute_write_current(t_port *port,
    lib_u16 port_id)
{
    core_machine_port_provider_entry *provider;

    if (port == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    provider = core_machine_port_find_provider(port, port_id, LIB_TRUE);
    if (provider != LIB_NULL) {
        if (provider->legacy_handler != LIB_NULL) {
            provider->legacy_handler(port, port_id, provider->owner);
        } else if (provider->write_provider != LIB_NULL) {
            type_status status = provider->write_provider(provider->owner, port_id,
                port->data.ioDWord);

            if (status != TYPE_STATUS_OK) return status;
        }
        return TYPE_STATUS_OK;
    }
    return TYPE_STATUS_OK;
}
type_status core_machine_port_execute_read_width(t_port *port,
    lib_u16 port_id, lib_u8 bytes)
{
    if (port == LIB_NULL || (bytes != 1u && bytes != 2u && bytes != 4u) ||
        (lib_u32)port_id + bytes > VPORT_MAX_PORT_COUNT) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    port->data.access_bytes = bytes;
    return core_machine_port_execute_read_current(port, port_id);
}
type_status core_machine_port_execute_write_width(t_port *port,
    lib_u16 port_id, lib_u8 bytes)
{
    if (port == LIB_NULL || (bytes != 1u && bytes != 2u && bytes != 4u) ||
        (lib_u32)port_id + bytes > VPORT_MAX_PORT_COUNT) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    port->data.access_bytes = bytes;
    return core_machine_port_execute_write_current(port, port_id);
}

type_status core_machine_port_execute_read(t_port *port, lib_u16 port_id)
{
    if (port == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    port->data.access_bytes = 1u;
    return core_machine_port_execute_read_current(port, port_id);
}

type_status core_machine_port_execute_write(t_port *port, lib_u16 port_id)
{
    if (port == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    port->data.access_bytes = 1u;
    return core_machine_port_execute_write_current(port, port_id);
}

type_status core_machine_port_add_read(t_port *port, lib_u16 port_id,
    core_machine_port_handler handler, C_VOID *owner)
{
    return core_machine_port_add_provider(port, port_id, LIB_FALSE, handler,
        LIB_NULL, LIB_NULL, owner, LIB_FALSE);
}

type_status core_machine_port_add_write(t_port *port, lib_u16 port_id,
    core_machine_port_handler handler, C_VOID *owner)
{
    return core_machine_port_add_provider(port, port_id, LIB_TRUE, handler,
        LIB_NULL, LIB_NULL, owner, LIB_FALSE);
}

type_status core_machine_port_add_read_provider(t_port *port,
    lib_u16 port_id, core_machine_port_read_provider provider,
    C_VOID *owner)
{
    return core_machine_port_add_provider(port, port_id, LIB_FALSE, LIB_NULL,
        provider, LIB_NULL, owner, LIB_FALSE);
}

type_status core_machine_port_add_read_wired_or_provider(t_port *port,
    lib_u16 port_id, core_machine_port_read_provider provider,
    C_VOID *owner)
{
    return core_machine_port_add_provider(port, port_id, LIB_FALSE, LIB_NULL,
        provider, LIB_NULL, owner, LIB_TRUE);
}
type_status core_machine_port_add_write_provider(t_port *port,
    lib_u16 port_id, core_machine_port_write_provider provider,
    C_VOID *owner)
{
    return core_machine_port_add_provider(port, port_id, LIB_TRUE, LIB_NULL,
        LIB_NULL, provider, owner, LIB_FALSE);
}

C_INT core_machine_port_has_read(const t_port *port, lib_u16 port_id)
{
    return core_machine_port_find_provider((t_port *)port, port_id,
        LIB_FALSE) != LIB_NULL;
}

C_INT core_machine_port_has_write(const t_port *port, lib_u16 port_id)
{
    return core_machine_port_find_provider((t_port *)port, port_id,
        LIB_TRUE) != LIB_NULL;
}

lib_u32 core_machine_port_read(t_port *port, lib_u16 port_id)
{
    if (port == LIB_NULL) return 0u;
    (C_VOID)core_machine_port_execute_read(port, port_id);
    return port->data.ioDWord;
}

C_VOID core_machine_port_write(t_port *port, lib_u16 port_id, lib_u32 value)
{
    if (port == LIB_NULL) return;
    port->data.ioDWord = value;
    (C_VOID)core_machine_port_execute_write(port, port_id);
}


C_VOID core_machine_port_initialize(t_port *port)
{
    if (port == LIB_NULL) return;
    lib_memory_set((C_VOID *)port, TYPE_ZERO_8, sizeof(*port));
}

C_VOID core_machine_port_reset(t_port *port)
{
    if (port == LIB_NULL) return;
    lib_memory_set((C_VOID *)&port->data, TYPE_ZERO_8, sizeof(port->data));
}

C_VOID core_machine_port_finalize(t_port *port)
{
    core_machine_port_provider_entry *entry;

    if (port == LIB_NULL) return;
    entry = port->connect.providers;
    while (entry != LIB_NULL) {
        core_machine_port_provider_entry *next = entry->next;
        lib_release(entry);
        entry = next;
    }
    port->connect.providers = LIB_NULL;
}

core_machine_port_provider_entry *core_machine_port_registration_begin(t_port *port)
{
    if (port == LIB_NULL) return LIB_NULL;
    port->connect.registration_status = TYPE_STATUS_OK;
    return port->connect.providers;
}

type_status core_machine_port_registration_status(const t_port *port)
{
    return port == LIB_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
        port->connect.registration_status;
}

C_VOID core_machine_port_rollback_registration(t_port *port,
    core_machine_port_provider_entry *checkpoint)
{
    if (port == LIB_NULL) return;
    while (port->connect.providers != checkpoint) {
        core_machine_port_provider_entry *entry = port->connect.providers;

        if (entry == LIB_NULL) break;
        port->connect.providers = entry->next;
        lib_release(entry);
    }
    port->connect.registration_status = TYPE_STATUS_OK;
}

C_VOID core_machine_port_set_test_allocation(t_port *port,
    core_machine_port_test_allocation *test_allocation)
{
    if (port != LIB_NULL) port->connect.test_allocation = test_allocation;
}
