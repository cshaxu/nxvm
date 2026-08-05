/* Copyright 2012-2014 Neko. */

/* VPIT implements the deterministic elapsed-tick subset of Intel 8254. */

#include "type.h"

#include "core/machine/port.h"
#include "core/machine/pit.h"

static type_unsigned_8 core_machine_pit_mode(const t_pit *pit,
    type_unsigned_8 id)
{
    type_unsigned_8 mode = VPIT_GetCW_M(pit->data.cw[id]);
    return mode == 6u ? 2u : mode == 7u ? 3u : mode;
}

static uint32_t core_machine_pit_bcd_decode(type_unsigned_16 value)
{
    return (uint32_t)(value & 0x000fu) +
        (uint32_t)((value >> 4) & 0x000fu) * 10u +
        (uint32_t)((value >> 8) & 0x000fu) * 100u +
        (uint32_t)((value >> 12) & 0x000fu) * 1000u;
}

static type_unsigned_16 core_machine_pit_bcd_encode(uint32_t value)
{
    type_unsigned_16 result = 0u;
    result |= (type_unsigned_16)(value % 10u);
    value /= 10u;
    result |= (type_unsigned_16)((value % 10u) << 4);
    value /= 10u;
    result |= (type_unsigned_16)((value % 10u) << 8);
    value /= 10u;
    result |= (type_unsigned_16)((value % 10u) << 12);
    return result;
}

static uint32_t core_machine_pit_decode_reload(const t_pit *pit,
    type_unsigned_8 id)
{
    if ((pit->data.cw[id] & VPIT_CW_BCD) != 0u) {
        uint32_t result = core_machine_pit_bcd_decode(pit->data.init[id]);
        return result == 0u ? 10000u : result;
    }
    return pit->data.init[id] == 0u ? 65536u : pit->data.init[id];
}

static type_unsigned_16 core_machine_pit_encode_count(const t_pit *pit,
    type_unsigned_8 id, uint32_t count)
{
    if ((pit->data.cw[id] & VPIT_CW_BCD) != 0u) {
        return count == 10000u ? 0u : core_machine_pit_bcd_encode(count);
    }
    return count == 65536u ? 0u : (type_unsigned_16)count;
}

static C_VOID core_machine_pit_sync_count(t_pit *pit, type_unsigned_8 id)
{
    pit->data.count[id] = core_machine_pit_encode_count(pit, id,
        pit->data.remaining[id]);
}

static C_VOID core_machine_pit_set_output_level(t_pit *pit,
    type_unsigned_8 id, type_bool asserted, type_bool notify_rise)
{
    if (pit->data.flagOutput[id] == asserted) return;
    pit->data.flagOutput[id] = asserted;
    if (pit->connect.output[id] == STD_NULL) return;
    if (!asserted || notify_rise) {
        pit->connect.output[id](pit->connect.output_owner[id], asserted);
    }
}

static uint32_t core_machine_pit_mode3_high_length(const t_pit *pit,
    type_unsigned_8 id)
{
    return (pit->data.reload[id] + 1u) / 2u;
}

static uint32_t core_machine_pit_mode3_low_length(const t_pit *pit,
    type_unsigned_8 id)
{
    return pit->data.reload[id] / 2u;
}

static C_VOID core_machine_pit_load(t_pit *pit, type_unsigned_8 id)
{
    type_unsigned_8 mode = core_machine_pit_mode(pit, id);

    pit->data.reload[id] = core_machine_pit_decode_reload(pit, id);
    pit->data.remaining[id] = pit->data.reload[id];
    pit->data.phase[id] = mode == 3u ?
        core_machine_pit_mode3_high_length(pit, id) : 0u;
    pit->data.flagReady[id] = TYPE_TRUE;
    pit->data.flagActive[id] = mode != 1u && mode != 5u;
    pit->data.flagPulseLow[id] = TYPE_FALSE;
    core_machine_pit_sync_count(pit, id);

    /* Programming establishes an initial pin level, but not an IRQ edge. */
    core_machine_pit_set_output_level(pit, id, mode != 0u, TYPE_FALSE);
}

static t_pit_data_status_rw core_machine_pit_read_start(const t_pit *pit,
    type_unsigned_8 id)
{
    return VPIT_GetCW_RW(pit->data.cw[id]) == 0x02 ?
        VPIT_STATUS_RW_MSB : VPIT_STATUS_RW_LSB;
}

static type_unsigned_8 core_machine_pit_capture_status(const t_pit *pit,
    type_unsigned_8 id)
{
    type_unsigned_8 status = pit->data.cw[id] &
        (VPIT_SB_BCD | VPIT_SB_M | VPIT_SB_RW);

    if (pit->data.flagOutput[id]) status |= VPIT_SB_OUT;
    if (!pit->data.flagReady[id]) status |= VPIT_SB_NC;
    return status;
}

static C_VOID core_machine_pit_latch_count(t_pit *pit, type_unsigned_8 id)
{
    if (pit->data.flagLatch[id]) return;
    core_machine_pit_sync_count(pit, id);
    pit->data.latch[id] = pit->data.count[id];
    pit->data.flagLatch[id] = TYPE_TRUE;
    pit->data.flagRead[id] = core_machine_pit_read_start(pit, id);
}

static C_VOID core_machine_pit_latch_status(t_pit *pit, type_unsigned_8 id)
{
    if (pit->data.flagStatusLatch[id]) return;
    pit->data.status_latch[id] = core_machine_pit_capture_status(pit, id);
    pit->data.flagStatusLatch[id] = TYPE_TRUE;
}

static C_VOID core_machine_pit_read(t_pit *pit, t_port *port,
    type_unsigned_8 id)
{
    type_unsigned_16 value;
    if (pit->data.flagStatusLatch[id]) {
        port->data.ioByte = pit->data.status_latch[id];
        pit->data.flagStatusLatch[id] = TYPE_FALSE;
        return;
    }
    if (pit->data.flagLatch[id]) {
        value = pit->data.latch[id];
    } else {
        core_machine_pit_sync_count(pit, id);
        value = pit->data.count[id];
    }
    switch (VPIT_GetCW_RW(pit->data.cw[id])) {
    case 0x01:
        port->data.ioByte = TYPE_MASK_UNSIGNED_8(value);
        pit->data.flagRead[id] = VPIT_STATUS_RW_READY;
        pit->data.flagLatch[id] = TYPE_FALSE;
        break;
    case 0x02:
        port->data.ioByte = TYPE_MASK_UNSIGNED_8(value >> 8);
        pit->data.flagRead[id] = VPIT_STATUS_RW_READY;
        pit->data.flagLatch[id] = TYPE_FALSE;
        break;
    case 0x03:
        if (pit->data.flagRead[id] == VPIT_STATUS_RW_MSB) {
            port->data.ioByte = TYPE_MASK_UNSIGNED_8(value >> 8);
            pit->data.flagRead[id] = VPIT_STATUS_RW_READY;
            pit->data.flagLatch[id] = TYPE_FALSE;
        } else {
            port->data.ioByte = TYPE_MASK_UNSIGNED_8(value);
            pit->data.flagRead[id] = VPIT_STATUS_RW_MSB;
        }
        break;
    default:
        pit->data.flagLatch[id] = TYPE_FALSE;
        break;
    }
}

static C_VOID core_machine_pit_write(t_pit *pit, t_port *port,
    type_unsigned_8 id)
{
    type_bool complete = TYPE_FALSE;
    switch (VPIT_GetCW_RW(pit->data.cw[id])) {
    case 0x01:
        pit->data.init[id] = TYPE_MASK_UNSIGNED_16(port->data.ioByte);
        complete = TYPE_TRUE;
        break;
    case 0x02:
        pit->data.init[id] = TYPE_MASK_UNSIGNED_16(port->data.ioByte << 8);
        complete = TYPE_TRUE;
        break;
    case 0x03:
        if (pit->data.flagWrite[id] == VPIT_STATUS_RW_MSB) {
            pit->data.init[id] = TYPE_MASK_UNSIGNED_16(port->data.ioByte << 8) |
                TYPE_MASK_UNSIGNED_8(pit->data.init[id]);
            complete = TYPE_TRUE;
        } else {
            pit->data.init[id] = TYPE_MASK_UNSIGNED_16(port->data.ioByte);
            pit->data.flagWrite[id] = VPIT_STATUS_RW_MSB;
        }
        break;
    default:
        return;
    }
    if (complete) {
        pit->data.flagWrite[id] = VPIT_STATUS_RW_READY;
        core_machine_pit_load(pit, id);
    }
}

static C_VOID core_machine_pit_tick_mode0(t_pit *pit, type_unsigned_8 id)
{
    if (!pit->data.flagActive[id] || !pit->connect.flagGate[id]) return;
    if (--pit->data.remaining[id] == 0u) {
        pit->data.flagActive[id] = TYPE_FALSE;
        core_machine_pit_set_output_level(pit, id, TYPE_TRUE, TYPE_TRUE);
    }
    core_machine_pit_sync_count(pit, id);
}

static C_VOID core_machine_pit_tick_mode1(t_pit *pit, type_unsigned_8 id)
{
    if (!pit->data.flagActive[id]) return;
    if (--pit->data.remaining[id] == 0u) {
        pit->data.flagActive[id] = TYPE_FALSE;
        core_machine_pit_set_output_level(pit, id, TYPE_TRUE, TYPE_TRUE);
    }
    core_machine_pit_sync_count(pit, id);
}

static C_VOID core_machine_pit_tick_mode2(t_pit *pit, type_unsigned_8 id)
{
    if (!pit->data.flagActive[id] || !pit->connect.flagGate[id]) return;
    if (pit->data.flagPulseLow[id]) {
        pit->data.flagPulseLow[id] = TYPE_FALSE;
        pit->data.remaining[id] = pit->data.reload[id];
        core_machine_pit_set_output_level(pit, id, TYPE_TRUE, TYPE_TRUE);
        if (pit->data.remaining[id] > 0u) --pit->data.remaining[id];
    } else if (pit->data.remaining[id] <= 1u) {
        pit->data.remaining[id] = 0u;
        pit->data.flagPulseLow[id] = TYPE_TRUE;
        core_machine_pit_set_output_level(pit, id, TYPE_FALSE, TYPE_TRUE);
    } else {
        --pit->data.remaining[id];
    }
    core_machine_pit_sync_count(pit, id);
}

static C_VOID core_machine_pit_tick_mode3(t_pit *pit, type_unsigned_8 id)
{
    if (!pit->data.flagActive[id] || !pit->connect.flagGate[id]) return;
    if (pit->data.remaining[id] > 0u) --pit->data.remaining[id];
    if (--pit->data.phase[id] == 0u) {
        if (pit->data.flagOutput[id]) {
            pit->data.phase[id] = core_machine_pit_mode3_low_length(pit, id);
            if (pit->data.phase[id] == 0u) {
                pit->data.remaining[id] = pit->data.reload[id];
                pit->data.phase[id] = core_machine_pit_mode3_high_length(pit, id);
            } else {
                core_machine_pit_set_output_level(pit, id, TYPE_FALSE, TYPE_TRUE);
            }
        } else {
            pit->data.remaining[id] = pit->data.reload[id];
            pit->data.phase[id] = core_machine_pit_mode3_high_length(pit, id);
            core_machine_pit_set_output_level(pit, id, TYPE_TRUE, TYPE_TRUE);
        }
    }
    core_machine_pit_sync_count(pit, id);
}

static C_VOID core_machine_pit_tick_mode4_or_5(t_pit *pit,
    type_unsigned_8 id)
{
    if (!pit->data.flagActive[id]) return;
    if (core_machine_pit_mode(pit, id) == 4u && !pit->connect.flagGate[id]) {
        return;
    }
    if (pit->data.flagPulseLow[id]) {
        pit->data.flagPulseLow[id] = TYPE_FALSE;
        pit->data.flagActive[id] = TYPE_FALSE;
        core_machine_pit_set_output_level(pit, id, TYPE_TRUE, TYPE_TRUE);
    } else if (--pit->data.remaining[id] == 0u) {
        pit->data.flagPulseLow[id] = TYPE_TRUE;
        core_machine_pit_set_output_level(pit, id, TYPE_FALSE, TYPE_TRUE);
    }
    core_machine_pit_sync_count(pit, id);
}

static C_VOID core_machine_pit_tick(t_pit *pit, type_unsigned_8 id)
{
    if (!pit->data.flagReady[id]) return;
    switch (core_machine_pit_mode(pit, id)) {
    case 0u: core_machine_pit_tick_mode0(pit, id); break;
    case 1u: core_machine_pit_tick_mode1(pit, id); break;
    case 2u: core_machine_pit_tick_mode2(pit, id); break;
    case 3u: core_machine_pit_tick_mode3(pit, id); break;
    case 4u:
    case 5u: core_machine_pit_tick_mode4_or_5(pit, id); break;
    default: break;
    }
}

static C_VOID io_read_0040(t_port *port, type_unsigned_16 port_id, C_VOID *owner)
{ (C_VOID)port_id; core_machine_pit_read((t_pit *)owner, port, 0u); }
static C_VOID io_read_0041(t_port *port, type_unsigned_16 port_id, C_VOID *owner)
{ (C_VOID)port_id; core_machine_pit_read((t_pit *)owner, port, 1u); }
static C_VOID io_read_0042(t_port *port, type_unsigned_16 port_id, C_VOID *owner)
{ (C_VOID)port_id; core_machine_pit_read((t_pit *)owner, port, 2u); }
static C_VOID io_write_0040(t_port *port, type_unsigned_16 port_id, C_VOID *owner)
{ (C_VOID)port_id; core_machine_pit_write((t_pit *)owner, port, 0u); }
static C_VOID io_write_0041(t_port *port, type_unsigned_16 port_id, C_VOID *owner)
{ (C_VOID)port_id; core_machine_pit_write((t_pit *)owner, port, 1u); }
static C_VOID io_write_0042(t_port *port, type_unsigned_16 port_id, C_VOID *owner)
{ (C_VOID)port_id; core_machine_pit_write((t_pit *)owner, port, 2u); }

static C_VOID io_write_0043(t_port *port, type_unsigned_16 port_id, C_VOID *owner)
{
    t_pit *pit = (t_pit *)owner;
    type_unsigned_8 id = VPIT_GetCW_SC(port->data.ioByte);
    type_unsigned_8 selected;
    (C_VOID)port_id;
    if (id == 3u) {
        for (selected = 0u; selected < 3u; ++selected) {
            if ((port->data.ioByte & VPIT_RB_CNT(selected)) != 0u) continue;
            if ((port->data.ioByte & VPIT_RB_COUNT) == 0u) {
                core_machine_pit_latch_count(pit, selected);
            }
            if ((port->data.ioByte & VPIT_RB_STATUS) == 0u) {
                core_machine_pit_latch_status(pit, selected);
            }
        }
        return;
    }
    if (VPIT_GetCW_RW(port->data.ioByte) == 0u) {
        core_machine_pit_latch_count(pit, id);
        return;
    }
    pit->data.flagLatch[id] = TYPE_FALSE;
    pit->data.flagStatusLatch[id] = TYPE_FALSE;
    pit->data.cw[id] = port->data.ioByte;
    pit->data.flagReady[id] = TYPE_FALSE;
    pit->data.flagActive[id] = TYPE_FALSE;
    pit->data.flagPulseLow[id] = TYPE_FALSE;
    pit->data.remaining[id] = 0u;
    pit->data.phase[id] = 0u;
    pit->data.count[id] = 0u;
    pit->data.flagRead[id] = VPIT_GetCW_RW(port->data.ioByte) == 2u ?
        VPIT_STATUS_RW_MSB : VPIT_STATUS_RW_LSB;
    pit->data.flagWrite[id] = pit->data.flagRead[id];
    core_machine_pit_set_output_level(pit, id,
        core_machine_pit_mode(pit, id) != 0u, TYPE_FALSE);
}

C_VOID core_machine_pit_set_output(t_pit *pit, type_unsigned_8 id,
    core_machine_pit_output_provider provider, C_VOID *owner)
{
    if (pit == STD_NULL || id >= 3u) return;
    pit->connect.output[id] = provider;
    pit->connect.output_owner[id] = owner;
    pit->connect.flagGate[id] = TYPE_TRUE;
}

C_VOID core_machine_pit_set_gate(t_pit *pit, type_unsigned_8 id,
    type_bool asserted)
{
    type_bool was_asserted;
    type_unsigned_8 mode;
    if (pit == STD_NULL || id >= 3u) return;
    was_asserted = pit->connect.flagGate[id];
    if (was_asserted == asserted) return;
    pit->connect.flagGate[id] = asserted;
    mode = core_machine_pit_mode(pit, id);
    if (!asserted) {
        if (mode == 2u || mode == 3u) {
            pit->data.remaining[id] = pit->data.reload[id];
            pit->data.phase[id] = mode == 3u ?
                core_machine_pit_mode3_high_length(pit, id) : 0u;
            pit->data.flagPulseLow[id] = TYPE_FALSE;
            core_machine_pit_set_output_level(pit, id, TYPE_TRUE, TYPE_FALSE);
            core_machine_pit_sync_count(pit, id);
        }
        return;
    }
    if (mode == 1u || mode == 5u) {
        if (!pit->data.flagReady[id]) return;
        pit->data.remaining[id] = pit->data.reload[id];
        pit->data.flagActive[id] = TYPE_TRUE;
        pit->data.flagPulseLow[id] = TYPE_FALSE;
        core_machine_pit_set_output_level(pit, id, mode == 5u, TYPE_FALSE);
        core_machine_pit_sync_count(pit, id);
    } else if (mode == 2u || mode == 3u) {
        pit->data.remaining[id] = pit->data.reload[id];
        pit->data.phase[id] = mode == 3u ?
            core_machine_pit_mode3_high_length(pit, id) : 0u;
        pit->data.flagActive[id] = TYPE_TRUE;
        pit->data.flagPulseLow[id] = TYPE_FALSE;
        core_machine_pit_set_output_level(pit, id, TYPE_TRUE, TYPE_FALSE);
        core_machine_pit_sync_count(pit, id);
    }
}

type_bool core_machine_pit_get_output(const t_pit *pit, type_unsigned_8 id)
{
    return pit != STD_NULL && id < 3u ? pit->data.flagOutput[id] : TYPE_FALSE;
}

C_VOID core_machine_pit_initialize(t_pit *pit, t_port *port)
{
    if (pit == STD_NULL || port == STD_NULL) return;
    STD_MEMSET((C_VOID *)pit, TYPE_ZERO_8, sizeof(*pit));
    core_machine_port_add_read(port, 0x0040, io_read_0040, pit);
    core_machine_port_add_read(port, 0x0041, io_read_0041, pit);
    core_machine_port_add_read(port, 0x0042, io_read_0042, pit);
    core_machine_port_add_write(port, 0x0040, io_write_0040, pit);
    core_machine_port_add_write(port, 0x0041, io_write_0041, pit);
    core_machine_port_add_write(port, 0x0042, io_write_0042, pit);
    core_machine_port_add_write(port, 0x0043, io_write_0043, pit);
}

C_VOID core_machine_pit_reset(t_pit *pit)
{
    type_native_unsigned id;
    if (pit == STD_NULL) return;
    STD_MEMSET((C_VOID *)&pit->data, TYPE_ZERO_8, sizeof(pit->data));
    for (id = 0u; id < 3u; ++id) {
        pit->data.flagReady[id] = TYPE_TRUE;
        pit->data.flagRead[id] = VPIT_STATUS_RW_READY;
        pit->data.flagWrite[id] = VPIT_STATUS_RW_READY;
        pit->connect.flagGate[id] = TYPE_TRUE;
    }
}

C_VOID core_machine_pit_advance(t_pit *pit, uint64_t elapsed_ticks)
{
    uint64_t tick;
    type_unsigned_8 id;
    if (pit == STD_NULL) return;
    for (tick = 0u; tick < elapsed_ticks; ++tick) {
        for (id = 0u; id < 3u; ++id) core_machine_pit_tick(pit, id);
    }
}

C_VOID core_machine_pit_finalize(t_pit *pit) { (C_VOID)pit; }
