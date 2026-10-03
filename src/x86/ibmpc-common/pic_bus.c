/* Copyright 2012-2026 Neko. */
#include "x86/ibmpc-common/pic_bus.h"

static lib_u8 pic_bus_levels(const core_machine_pic_bus *bus)
{
    lib_u8 id;
    lib_u8 levels = 0u;
    for (id = 0u; id < 8u; ++id) {
        if (bus->asserted[id] != 0u) levels |= (lib_u8)(1u << id);
    }
    return levels;
}

void core_machine_pic_refresh(core_machine_pic_bus *master,
    core_machine_pic_bus *slave)
{
    x86_pic_request request;
    lib_u8 address;
    lib_u8 cascade = 0u;
    if (master == LIB_NULL || slave == LIB_NULL) return;
    x86_pic_set_inputs(slave->device, pic_bus_levels(slave), 0u, 0u);
    if (x86_pic_cascade_address(slave->device, &address) &&
        x86_pic_select(slave->device, &request) == LIB_STATUS_OK) {
        cascade = (lib_u8)(1u << address);
    }
    x86_pic_set_inputs(master->device, pic_bus_levels(master), 0u, cascade);
}

lib_status core_machine_pic_read_register(core_machine_pic_bus *bus,
    lib_u8 selector, lib_u8 *out_value)
{
    if (bus == LIB_NULL || out_value == LIB_NULL || selector > 1u)
        return LIB_STATUS_INVALID_ARGUMENT;
    x86_pic_read_register(bus->device, selector, out_value);
    return LIB_STATUS_OK;
}

lib_status core_machine_pic_write_register(core_machine_pic_bus *bus,
    lib_u8 selector, lib_u8 value)
{
    if (bus == LIB_NULL || selector > 1u) return LIB_STATUS_INVALID_ARGUMENT;
    x86_pic_write_register(bus->device, selector, value);
    core_machine_pic_refresh(bus->master, bus->slave);
    return LIB_STATUS_OK;
}

lib_status core_machine_pic_capture_registers(const core_machine_pic_bus *bus,
    x86_pic_register_state *out_state)
{
    if (bus == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return x86_pic_capture_registers(bus->device, out_state);
}

static lib_status pic_bus_read(void *owner, lib_u16 port_id, lib_u64 tick,
    lib_u32 *out_value)
{
    (void)tick;
    core_machine_pic_bus *bus = owner;
    lib_u8 value = 0u;

    if (bus == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    core_machine_pic_read_register(bus, (lib_u8)(port_id & 1u), &value);
    *out_value = value;
    return LIB_STATUS_OK;
}

static lib_status pic_bus_write(void *owner, lib_u16 port_id, lib_u32 value)
{
    core_machine_pic_bus *bus = owner;

    if (bus == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return core_machine_pic_write_register(bus, (lib_u8)(port_id & 1u),
        (lib_u8)value);
}

lib_status core_machine_pic_initialize(core_machine_pic_bus **out_master,
    core_machine_pic_bus **out_slave, core_machine *machine,
    core_machine_pic_topology topology)
{
    core_machine_port_route routes[4];
    lib_status status;
    lib_u8 index;
    lib_u8 count;
    core_machine_pic_bus *master;
    core_machine_pic_bus *slave;
    if (out_master == LIB_NULL || out_slave == LIB_NULL || out_master == out_slave ||
        machine == LIB_NULL || (topology != CORE_MACHINE_PIC_TOPOLOGY_CASCADED &&
        topology != CORE_MACHINE_PIC_TOPOLOGY_SINGLE)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_master = *out_slave = LIB_NULL;
    master = lib_allocate_zero(2u, sizeof(*master));
    if (master == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    slave = master + 1;
    master->master = slave->master = master;
    master->slave = slave->slave = slave;
    status = x86_pic_create(LIB_TRUE, &master->device);
    if (status == LIB_STATUS_OK) status = x86_pic_create(LIB_FALSE, &slave->device);
    if (status != LIB_STATUS_OK) {
        core_machine_pic_finalize(master, slave);
        return status;
    }
    count = topology == CORE_MACHINE_PIC_TOPOLOGY_CASCADED ? 4u : 2u;
    for (index = 0u; index < count; ++index) {
        core_machine_pic_bus *bus = index < 2u ? master : slave;
        lib_u16 address = (lib_u16)((index < 2u ? 0x20u : 0xa0u) + (index & 1u));
        routes[index] = (core_machine_port_route) {address, pic_bus_read,
            pic_bus_write, bus, LIB_FALSE, 0u};
    }
    status = core_machine_install_port_routes(machine, routes, count);
    if (status != LIB_STATUS_OK) {
        core_machine_pic_finalize(master, slave);
        return status;
    }
    *out_master = master;
    *out_slave = slave;
    return LIB_STATUS_OK;
}

void core_machine_pic_reset(core_machine_pic_bus *master, core_machine_pic_bus *slave)
{
    if (master == LIB_NULL || slave == LIB_NULL) return;
    lib_memory_set(master->asserted, 0u, sizeof(master->asserted));
    lib_memory_set(slave->asserted, 0u, sizeof(slave->asserted));
    x86_pic_reset(master->device);
    x86_pic_reset(slave->device);
}

void core_machine_pic_finalize(core_machine_pic_bus *master, core_machine_pic_bus *slave)
{
    core_machine_pic_irq_source *source;
    if (master != LIB_NULL) {
        while ((source = master->sources) != LIB_NULL) {
            master->sources = source->next;
            lib_release(source);
        }
        x86_pic_destroy(master->device);
        master->device = LIB_NULL;
    }
    if (slave != LIB_NULL) {
        x86_pic_destroy(slave->device);
        slave->device = LIB_NULL;
    }
    lib_release(master);
}

void core_machine_pic_set_irq_timing(core_machine_pic_bus *master,
    core_machine_pic_bus *slave, const core_machine_pic_irq_timing *timing)
{
    if (master == LIB_NULL || slave == LIB_NULL || timing == LIB_NULL) return;
    x86_pic_set_irq_timing(master->device, timing->unmask_delivery_ticks);
    x86_pic_set_irq_timing(slave->device, timing->unmask_delivery_ticks + 8u);
}

void core_machine_pic_advance(core_machine_pic_bus *master,
    core_machine_pic_bus *slave, lib_u64 elapsed_ticks)
{
    if (master == LIB_NULL || slave == LIB_NULL) return;
    x86_pic_advance(master->device, elapsed_ticks);
    x86_pic_advance(slave->device, elapsed_ticks);
}

lib_status core_machine_pic_ticks_until_event(const core_machine_pic_bus *master,
    const core_machine_pic_bus *slave, lib_u64 *out_ticks)
{
    lib_u64 first;
    lib_u64 second;
    lib_status first_status;
    lib_status second_status;
    if (master == LIB_NULL || slave == LIB_NULL || out_ticks == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    first_status = x86_pic_ticks_until_event(master->device, &first);
    second_status = x86_pic_ticks_until_event(slave->device, &second);
    if (first_status != LIB_STATUS_OK && second_status != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_STATE;
    }
    *out_ticks = first_status != LIB_STATUS_OK ? second :
        (second_status == LIB_STATUS_OK && second < first ? second : first);
    return LIB_STATUS_OK;
}

lib_status core_machine_pic_irq_source_bind(core_machine_pic_irq_source **out_source,
    core_machine_pic_bus *master, core_machine_pic_bus *slave, lib_u8 irq_id)
{
    core_machine_pic_irq_source *source;
    if (out_source == LIB_NULL || master == LIB_NULL || slave == LIB_NULL ||
        master->master != master || master->slave != slave ||
        irq_id >= 16u || irq_id == 2u) return LIB_STATUS_INVALID_ARGUMENT;
    source = *out_source;
    if (source != LIB_NULL &&
        (source->master != master || source->slave != slave))
        return LIB_STATUS_INVALID_STATE;
    if (source == LIB_NULL) {
        source = lib_allocate_zero(1u, sizeof(*source));
        if (source == LIB_NULL) return LIB_STATUS_NO_MEMORY;
        source->next = master->sources;
        master->sources = source;
    }
    source->asserted = LIB_FALSE;
    source->master = master;
    source->slave = slave;
    source->irq = irq_id;
    *out_source = source;
    return LIB_STATUS_OK;
}

lib_bool core_machine_pic_irq_source_is_asserted(const core_machine_pic_irq_source *source)
{
    return source != LIB_NULL && source->asserted;
}

void core_machine_pic_irq_source_assert(core_machine_pic_irq_source *source)
{
    core_machine_pic_bus *bus;
    lib_u8 line;
    if (source == LIB_NULL || source->asserted || source->master == LIB_NULL ||
        source->slave == LIB_NULL || source->irq >= 16u) return;
    bus = source->irq < 8u ? source->master : source->slave;
    line = source->irq & 7u;
    source->asserted = LIB_TRUE;
    if (bus->asserted[line] != 0xffu) ++bus->asserted[line];
    x86_pic_set_inputs(bus->device, pic_bus_levels(bus), (lib_u8)(1u << line), 0u);
    core_machine_pic_refresh(source->master, source->slave);
}

void core_machine_pic_irq_source_deassert(core_machine_pic_irq_source *source)
{
    core_machine_pic_bus *bus;
    lib_u8 line;
    if (source == LIB_NULL || !source->asserted) return;
    source->asserted = LIB_FALSE;
    if (source->master == LIB_NULL || source->slave == LIB_NULL || source->irq >= 16u) return;
    bus = source->irq < 8u ? source->master : source->slave;
    line = source->irq & 7u;
    if (bus->asserted[line] == 0u) return;
    --bus->asserted[line];
    core_machine_pic_refresh(source->master, source->slave);
}

void core_machine_pic_timer_output(void *owner, lib_u8 asserted)
{
    if (asserted) core_machine_pic_irq_source_assert(owner);
    else core_machine_pic_irq_source_deassert(owner);
}

lib_u8 core_machine_pic_scan_interrupt(core_machine_pic_bus *master,
    core_machine_pic_bus *slave)
{
    x86_pic_request request;
    if (master == LIB_NULL || slave == LIB_NULL) return LIB_FALSE;
    if (x86_pic_select(master->device, &request) != LIB_STATUS_OK) return LIB_FALSE;
    return !request.cascade ||
        x86_pic_select(slave->device, &request) == LIB_STATUS_OK;
}

lib_u8 core_machine_pic_peek_interrupt(core_machine_pic_bus *master,
    core_machine_pic_bus *slave)
{
    x86_pic_request request;
    if (master == LIB_NULL || slave == LIB_NULL ||
        x86_pic_select(master->device, &request) != LIB_STATUS_OK) return 0u;
    if (request.cascade &&
        x86_pic_select(slave->device, &request) != LIB_STATUS_OK) return 0u;
    return request.vector;
}

lib_u8 core_machine_pic_get_interrupt(core_machine_pic_bus *master,
    core_machine_pic_bus *slave)
{
    x86_pic_request request;
    if (master == LIB_NULL || slave == LIB_NULL) return 0u;
    request = x86_pic_acknowledge(master->device);
    if (request.cascade) {
        request = x86_pic_acknowledge(slave->device);
        core_machine_pic_refresh(master, slave);
    }
    return request.vector;
}
