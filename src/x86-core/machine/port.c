/* Copyright 2012-2014 Neko. */

/* VPORT is the hub that connects all devices with the I/O port. */
#include "lib/types/types_interface.h"



#include "x86-core/machine/port.h"

struct x86_core_machine_port_provider_entry {
    lib_u16 port_id;
    lib_u8 write;
    lib_u8 wired_or;
    lib_u16 byte_lane_end;
    x86_core_machine_port_read_provider read_provider;
    x86_core_machine_port_write_provider write_provider;
    void *owner;
    x86_core_machine_port_provider_entry *next;
};

static x86_core_machine_port_provider_entry *x86_core_machine_port_find_provider(
    t_port *port, lib_u16 port_id, lib_u8 write)
{
    x86_core_machine_port_provider_entry *entry;

    if (port == LIB_NULL) return LIB_NULL;
    for (entry = port->connect.providers; entry != LIB_NULL; entry = entry->next) {
        if (entry->port_id == port_id && entry->write == write && !entry->wired_or) return entry;
    }
    return LIB_NULL;
}

static lib_status x86_core_machine_port_add_provider(t_port *port,
    lib_u16 port_id, lib_u8 write,
    x86_core_machine_port_read_provider read_provider,
    x86_core_machine_port_write_provider write_provider, void *owner,
    lib_u8 wired_or, lib_u16 byte_lane_end)
{
    x86_core_machine_port_provider_entry *entry;

    if (port == LIB_NULL || (read_provider == LIB_NULL &&
        write_provider == LIB_NULL)) return LIB_STATUS_INVALID_ARGUMENT;
    if (port->connect.registration_status != LIB_STATUS_OK) {
        return port->connect.registration_status;
    }
    entry = x86_core_machine_port_find_provider(port, port_id, write);
    if ((!wired_or && entry != LIB_NULL) ||
        (wired_or && (write || entry == LIB_NULL))) {
        port->connect.registration_status = LIB_STATUS_INVALID_STATE;
        return LIB_STATUS_INVALID_STATE;
    }
    if (port->connect.test_allocation != LIB_NULL) {
        ++port->connect.test_allocation->attempts;
        if (port->connect.test_allocation->fail_at != 0u &&
            port->connect.test_allocation->attempts ==
                port->connect.test_allocation->fail_at) {
            port->connect.registration_status = LIB_STATUS_NO_MEMORY;
            return LIB_STATUS_NO_MEMORY;
        }
    }
    entry = (x86_core_machine_port_provider_entry *)lib_allocate_zero(1u, sizeof(*entry));
    if (entry == LIB_NULL) {
        port->connect.registration_status = LIB_STATUS_NO_MEMORY;
        return LIB_STATUS_NO_MEMORY;
    }
    entry->port_id = port_id;
    entry->write = write;
    entry->wired_or = wired_or;
    entry->byte_lane_end = byte_lane_end;
    entry->read_provider = read_provider;
    entry->write_provider = write_provider;
    entry->owner = owner;
    entry->next = port->connect.providers;
    port->connect.providers = entry;
    return LIB_STATUS_OK;
}


static lib_status x86_core_machine_port_execute_read_current(t_port *port,
    lib_u16 port_id, lib_u64 tick)
{
    x86_core_machine_port_provider_entry *provider;
    lib_u32 value = 0u;

    if (port == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    provider = x86_core_machine_port_find_provider(port, port_id, LIB_FALSE);
    if (provider != LIB_NULL) {
        value = port->data.ioDWord;
        if (provider->read_provider != LIB_NULL) {
            if (port->data.access_bytes > 1u && provider->byte_lane_end != 0u &&
                (lib_u32)port_id + port->data.access_bytes <=
                    provider->byte_lane_end) {
                lib_u8 lane;
                value = 0u;
                for (lane = 0u; lane < port->data.access_bytes; ++lane) {
                    lib_u32 byte = 0u;
                    lib_status status = provider->read_provider(provider->owner,
                        (lib_u16)(port_id + lane), tick, &byte);
                    if (status != LIB_STATUS_OK) return status;
                    value |= (byte & 0xffu) << (lane * 8u);
                }
            } else {
                lib_status status = provider->read_provider(provider->owner,
                    port_id, tick, &value);
                if (status != LIB_STATUS_OK) return status;
            }
        }
    }
    for (provider = port->connect.providers; provider != LIB_NULL; provider = provider->next) {
        lib_u32 contribution = 0u;
        lib_status status;

        if (provider->port_id != port_id || provider->write || !provider->wired_or ||
            provider->read_provider == LIB_NULL) continue;
        status = provider->read_provider(provider->owner, port_id, tick, &contribution);
        if (status != LIB_STATUS_OK) return status;
        value |= contribution;
    }
    port->data.ioDWord = value;
    return LIB_STATUS_OK;
}
static lib_status x86_core_machine_port_execute_write_current(t_port *port,
    lib_u16 port_id)
{
    x86_core_machine_port_provider_entry *provider;

    if (port == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    provider = x86_core_machine_port_find_provider(port, port_id, LIB_TRUE);
    if (provider != LIB_NULL) {
        if (provider->write_provider != LIB_NULL) {
            if (port->data.access_bytes > 1u && provider->byte_lane_end != 0u &&
                (lib_u32)port_id + port->data.access_bytes <=
                    provider->byte_lane_end) {
                lib_u8 lane;
                lib_u32 value = port->data.ioDWord;
                for (lane = 0u; lane < port->data.access_bytes; ++lane) {
                    lib_status status = provider->write_provider(provider->owner,
                        (lib_u16)(port_id + lane),
                        (value >> (lane * 8u)) & 0xffu);
                    if (status != LIB_STATUS_OK) return status;
                }
            } else {
                lib_status status = provider->write_provider(provider->owner,
                    port_id, port->data.ioDWord);
                if (status != LIB_STATUS_OK) return status;
            }
        }
        return LIB_STATUS_OK;
    }
    return LIB_STATUS_OK;
}
lib_status x86_core_machine_port_execute_read_width(t_port *port,
    lib_u16 port_id, lib_u8 bytes, lib_u64 tick)
{
    if (port == LIB_NULL || (bytes != 1u && bytes != 2u && bytes != 4u) ||
        (lib_u32)port_id + bytes > VPORT_MAX_PORT_COUNT) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    port->data.access_bytes = bytes;
    return x86_core_machine_port_execute_read_current(port, port_id, tick);
}
lib_status x86_core_machine_port_execute_write_width(t_port *port,
    lib_u16 port_id, lib_u8 bytes)
{
    if (port == LIB_NULL || (bytes != 1u && bytes != 2u && bytes != 4u) ||
        (lib_u32)port_id + bytes > VPORT_MAX_PORT_COUNT) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    port->data.access_bytes = bytes;
    return x86_core_machine_port_execute_write_current(port, port_id);
}

lib_status x86_core_machine_port_execute_read(t_port *port, lib_u16 port_id,
    lib_u64 tick)
{
    if (port == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    port->data.access_bytes = 1u;
    return x86_core_machine_port_execute_read_current(port, port_id, tick);
}

lib_status x86_core_machine_port_execute_write(t_port *port, lib_u16 port_id)
{
    if (port == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    port->data.access_bytes = 1u;
    return x86_core_machine_port_execute_write_current(port, port_id);
}

lib_status x86_core_machine_port_add_route(t_port *port,
    const x86_core_machine_port_route *route)
{
    lib_status status = LIB_STATUS_OK;

    if (route == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (route->read != LIB_NULL) {
        status = x86_core_machine_port_add_provider(port, route->address, LIB_FALSE,
            route->read, LIB_NULL, route->owner,
            route->wired_or_read, route->byte_lane_end);
    }
    if (status == LIB_STATUS_OK && route->write != LIB_NULL) {
        status = x86_core_machine_port_add_provider(port, route->address, LIB_TRUE,
            LIB_NULL, route->write, route->owner, LIB_FALSE,
            route->byte_lane_end);
    }
    return status;
}

lib_i32 x86_core_machine_port_has_read(const t_port *port, lib_u16 port_id)
{
    return x86_core_machine_port_find_provider((t_port *)port, port_id,
        LIB_FALSE) != LIB_NULL;
}

lib_i32 x86_core_machine_port_has_write(const t_port *port, lib_u16 port_id)
{
    return x86_core_machine_port_find_provider((t_port *)port, port_id,
        LIB_TRUE) != LIB_NULL;
}

lib_u32 x86_core_machine_port_read(t_port *port, lib_u16 port_id)
{
    if (port == LIB_NULL) return 0u;
    (void)x86_core_machine_port_execute_read(port, port_id, 0u);
    return port->data.ioDWord;
}

void x86_core_machine_port_write(t_port *port, lib_u16 port_id, lib_u32 value)
{
    if (port == LIB_NULL) return;
    port->data.ioDWord = value;
    (void)x86_core_machine_port_execute_write(port, port_id);
}


void x86_core_machine_port_initialize(t_port *port)
{
    if (port == LIB_NULL) return;
    lib_memory_set((void *)port, 0, sizeof(*port));
}

void x86_core_machine_port_reset(t_port *port)
{
    if (port == LIB_NULL) return;
    lib_memory_set((void *)&port->data, 0, sizeof(port->data));
}

void x86_core_machine_port_finalize(t_port *port)
{
    x86_core_machine_port_provider_entry *entry;

    if (port == LIB_NULL) return;
    entry = port->connect.providers;
    while (entry != LIB_NULL) {
        x86_core_machine_port_provider_entry *next = entry->next;
        lib_release(entry);
        entry = next;
    }
    port->connect.providers = LIB_NULL;
}

x86_core_machine_port_provider_entry *x86_core_machine_port_registration_begin(t_port *port)
{
    if (port == LIB_NULL) return LIB_NULL;
    port->connect.registration_status = LIB_STATUS_OK;
    return port->connect.providers;
}

void x86_core_machine_port_unregister_owner(t_port *port, const void *owner)
{
    x86_core_machine_port_provider_entry **link;

    if (port == LIB_NULL || owner == LIB_NULL) return;
    link = &port->connect.providers;
    while (*link != LIB_NULL) {
        x86_core_machine_port_provider_entry *entry = *link;

        if (entry->owner == owner) {
            *link = entry->next;
            lib_release(entry);
        } else link = &entry->next;
    }
}

lib_status x86_core_machine_port_registration_status(const t_port *port)
{
    return port == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        port->connect.registration_status;
}

void x86_core_machine_port_rollback_registration(t_port *port,
    x86_core_machine_port_provider_entry *checkpoint)
{
    if (port == LIB_NULL) return;
    while (port->connect.providers != checkpoint) {
        x86_core_machine_port_provider_entry *entry = port->connect.providers;

        if (entry == LIB_NULL) break;
        port->connect.providers = entry->next;
        lib_release(entry);
    }
    port->connect.registration_status = LIB_STATUS_OK;
}

void x86_core_machine_port_set_test_allocation(t_port *port,
    x86_core_machine_port_test_allocation *test_allocation)
{
    if (port != LIB_NULL) port->connect.test_allocation = test_allocation;
}
