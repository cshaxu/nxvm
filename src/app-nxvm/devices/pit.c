/* Copyright 2012-2014 Neko. */

/* VPIT implements the deterministic elapsed-tick subset of Intel 8254. */
#include "lib/types/types_interface.h"
#include "app-nxvm/devices/device_support.h"


#include "app-nxvm/devices/port.h"
#include "app-nxvm/devices/pit.h"

static lib_u8 core_machine_pit_mode(const t_pit *pit,
    lib_u8 id)
{
    lib_u8 mode = VPIT_GetCW_M(pit->data.cw[id]);
    return mode == 6u ? 2u : mode == 7u ? 3u : mode;
}

static lib_u32 core_machine_pit_bcd_decode(lib_u16 value)
{
    return (lib_u32)(value & 0x000fu) +
        (lib_u32)((value >> 4) & 0x000fu) * 10u +
        (lib_u32)((value >> 8) & 0x000fu) * 100u +
        (lib_u32)((value >> 12) & 0x000fu) * 1000u;
}

static lib_u16 core_machine_pit_bcd_encode(lib_u32 value)
{
    lib_u16 result = 0u;
    result |= (lib_u16)(value % 10u);
    value /= 10u;
    result |= (lib_u16)((value % 10u) << 4);
    value /= 10u;
    result |= (lib_u16)((value % 10u) << 8);
    value /= 10u;
    result |= (lib_u16)((value % 10u) << 12);
    return result;
}

static lib_u32 core_machine_pit_decode_reload(const t_pit *pit,
    lib_u8 id)
{
    if ((pit->data.cw[id] & VPIT_CW_BCD) != 0u) {
        lib_u32 result = core_machine_pit_bcd_decode(pit->data.init[id]);
        return result == 0u ? 10000u : result;
    }
    return pit->data.init[id] == 0u ? 65536u : pit->data.init[id];
}

static lib_u16 core_machine_pit_encode_count(const t_pit *pit,
    lib_u8 id, lib_u32 count)
{
    if ((pit->data.cw[id] & VPIT_CW_BCD) != 0u) {
        return count == 10000u ? 0u : core_machine_pit_bcd_encode(count);
    }
    return count == 65536u ? 0u : (lib_u16)count;
}

static void core_machine_pit_sync_count(t_pit *pit, lib_u8 id)
{
    pit->data.count[id] = core_machine_pit_encode_count(pit, id,
        pit->data.remaining[id]);
}

static void core_machine_pit_set_output_level(t_pit *pit,
    lib_u8 id, lib_u8 asserted, lib_u8 notify_rise)
{
    if (pit->data.flagOutput[id] == asserted) return;
    pit->data.flagOutput[id] = asserted;
    if (pit->connect.output[id] == LIB_NULL) return;
    if (!asserted || notify_rise) {
        pit->connect.output[id](pit->connect.output_owner[id], asserted);
    }
}

static lib_u32 core_machine_pit_mode3_high_length(const t_pit *pit,
    lib_u8 id)
{
    return (pit->data.reload[id] + 1u) / 2u;
}

static lib_u32 core_machine_pit_mode3_low_length(const t_pit *pit,
    lib_u8 id)
{
    return pit->data.reload[id] / 2u;
}

static lib_u32 core_machine_pit_mode3_count(const t_pit *pit,
    lib_u8 id)
{
    return pit->data.reload[id] - (pit->data.reload[id] & 1u);
}

static void core_machine_pit_load(t_pit *pit, lib_u8 id)
{
    lib_u8 mode = core_machine_pit_mode(pit, id);

    pit->data.reload[id] = core_machine_pit_decode_reload(pit, id);
    pit->data.remaining[id] = mode == 3u ?
        core_machine_pit_mode3_count(pit, id) : pit->data.reload[id];
    pit->data.phase[id] = mode == 3u ?
        core_machine_pit_mode3_high_length(pit, id) : 0u;
    pit->data.flagReady[id] = LIB_TRUE;
    pit->data.flagLoadPending[id] = LIB_FALSE;
    pit->data.flagRestart[id] = LIB_FALSE;
    pit->data.flagActive[id] = mode != 1u && mode != 5u;
    pit->data.flagPulseLow[id] = LIB_FALSE;
    core_machine_pit_sync_count(pit, id);

}

static t_pit_data_status_rw core_machine_pit_read_start(const t_pit *pit,
    lib_u8 id)
{
    return VPIT_GetCW_RW(pit->data.cw[id]) == 0x02 ?
        VPIT_STATUS_RW_MSB : VPIT_STATUS_RW_LSB;
}

static lib_u8 core_machine_pit_capture_status(const t_pit *pit,
    lib_u8 id)
{
    lib_u8 status = pit->data.cw[id] &
        (VPIT_SB_BCD | VPIT_SB_M | VPIT_SB_RW);

    if (pit->data.flagOutput[id]) status |= VPIT_SB_OUT;
    if (!pit->data.flagReady[id]) status |= VPIT_SB_NC;
    return status;
}

static void core_machine_pit_latch_count(t_pit *pit, lib_u8 id)
{
    if (pit->data.flagLatch[id]) return;
    core_machine_pit_sync_count(pit, id);
    pit->data.latch[id] = pit->data.count[id];
    pit->data.flagLatch[id] = LIB_TRUE;
    pit->data.flagRead[id] = core_machine_pit_read_start(pit, id);
}

static void core_machine_pit_latch_status(t_pit *pit, lib_u8 id)
{
    if (pit->data.flagStatusLatch[id]) return;
    pit->data.status_latch[id] = core_machine_pit_capture_status(pit, id);
    pit->data.flagStatusLatch[id] = LIB_TRUE;
}

static void core_machine_pit_read(t_pit *pit, t_port *port,
    lib_u8 id)
{
    lib_u16 value;
    if (pit->data.flagStatusLatch[id]) {
        port->data.ioByte = pit->data.status_latch[id];
        pit->data.flagStatusLatch[id] = LIB_FALSE;
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
        port->data.ioByte = CORE_MACHINE_MASK_U8(value);
        pit->data.flagRead[id] = VPIT_STATUS_RW_READY;
        pit->data.flagLatch[id] = LIB_FALSE;
        break;
    case 0x02:
        port->data.ioByte = CORE_MACHINE_MASK_U8(value >> 8);
        pit->data.flagRead[id] = VPIT_STATUS_RW_READY;
        pit->data.flagLatch[id] = LIB_FALSE;
        break;
    case 0x03:
        if (pit->data.flagRead[id] == VPIT_STATUS_RW_MSB) {
            port->data.ioByte = CORE_MACHINE_MASK_U8(value >> 8);
            pit->data.flagRead[id] = VPIT_STATUS_RW_READY;
            pit->data.flagLatch[id] = LIB_FALSE;
        } else {
            port->data.ioByte = CORE_MACHINE_MASK_U8(value);
            pit->data.flagRead[id] = VPIT_STATUS_RW_MSB;
        }
        break;
    default:
        pit->data.flagLatch[id] = LIB_FALSE;
        break;
    }
}

static void core_machine_pit_write(t_pit *pit, t_port *port,
    lib_u8 id)
{
    lib_u8 complete = LIB_FALSE;
    switch (VPIT_GetCW_RW(pit->data.cw[id])) {
    case 0x01:
        pit->data.init[id] = CORE_MACHINE_MASK_U16(port->data.ioByte);
        complete = LIB_TRUE;
        break;
    case 0x02:
        pit->data.init[id] = CORE_MACHINE_MASK_U16(port->data.ioByte << 8);
        complete = LIB_TRUE;
        break;
    case 0x03:
        if (pit->data.flagWrite[id] == VPIT_STATUS_RW_MSB) {
            pit->data.init[id] = CORE_MACHINE_MASK_U16(port->data.ioByte << 8) |
                CORE_MACHINE_MASK_U8(pit->data.init[id]);
            complete = LIB_TRUE;
        } else {
            pit->data.init[id] = CORE_MACHINE_MASK_U16(port->data.ioByte);
            pit->data.flagWrite[id] = VPIT_STATUS_RW_MSB;
            if (core_machine_pit_mode(pit, id) == 0u) {
                pit->data.flagReady[id] = LIB_FALSE;
                pit->data.flagActive[id] = LIB_FALSE;
                pit->data.flagPulseLow[id] = LIB_FALSE;
                core_machine_pit_set_output_level(pit, id, LIB_FALSE, LIB_TRUE);
            }
        }
        break;
    default:
        return;
    }
    if (complete) {
        pit->data.flagWrite[id] = VPIT_STATUS_RW_READY;
        pit->data.flagReady[id] = LIB_FALSE;
        pit->data.flagLoadPending[id] = LIB_TRUE;
        /* 8254 mode 0 holds OUT high after terminal count only until a new
         * count is written.  The new count restarts the mode with OUT low;
         * this applies equally to an LSB-only count and to the completed
         * second byte of an LSB/MSB count. */
        if (core_machine_pit_mode(pit, id) == 0u) {
            pit->data.flagActive[id] = LIB_FALSE;
            pit->data.flagPulseLow[id] = LIB_FALSE;
            core_machine_pit_set_output_level(pit, id, LIB_FALSE, LIB_TRUE);
        }
    }
}

static void core_machine_pit_commit_pending(t_pit *pit, lib_u8 id)
{
    core_machine_pit_load(pit, id);
}

static void core_machine_pit_tick_mode0(t_pit *pit, lib_u8 id)
{
    if (pit->data.flagLoadPending[id]) {
        core_machine_pit_commit_pending(pit, id);
        return;
    }
    if (!pit->data.flagActive[id] || !pit->connect.flagGate[id]) return;
    if (--pit->data.remaining[id] == 0u) {
        pit->data.flagActive[id] = LIB_FALSE;
        core_machine_pit_set_output_level(pit, id, LIB_TRUE, LIB_TRUE);
    }
    core_machine_pit_sync_count(pit, id);
}

static void core_machine_pit_tick_mode1(t_pit *pit, lib_u8 id)
{
    /* Mode 1 loads CE on the clock following a completed count write.  The
     * later gate trigger starts the one-shot; it does not defer that load. */
    if (pit->data.flagLoadPending[id]) {
        core_machine_pit_commit_pending(pit, id);
        return;
    }
    if (pit->data.flagTrigger[id]) {
        pit->data.flagTrigger[id] = LIB_FALSE;
        pit->data.remaining[id] = pit->data.reload[id];
        pit->data.flagPulseLow[id] = LIB_FALSE;
        core_machine_pit_sync_count(pit, id);
        pit->data.flagActive[id] = LIB_TRUE;
        core_machine_pit_set_output_level(pit, id, LIB_FALSE, LIB_TRUE);
        return;
    }
    if (!pit->data.flagActive[id]) return;
    if (--pit->data.remaining[id] == 0u) {
        pit->data.flagActive[id] = LIB_FALSE;
        core_machine_pit_set_output_level(pit, id, LIB_TRUE, LIB_TRUE);
    }
    core_machine_pit_sync_count(pit, id);
}

static void core_machine_pit_tick_mode2(t_pit *pit, lib_u8 id)
{
    if (pit->data.flagRestart[id]) {
        core_machine_pit_commit_pending(pit, id);
        return;
    }
    if (pit->data.flagLoadPending[id] && !pit->data.flagActive[id]) {
        core_machine_pit_commit_pending(pit, id);
        return;
    }
    if (!pit->data.flagActive[id] || !pit->connect.flagGate[id]) return;
    if (pit->data.flagPulseLow[id]) {
        pit->data.flagPulseLow[id] = LIB_FALSE;
        if (pit->data.flagLoadPending[id]) core_machine_pit_commit_pending(pit, id);
        else pit->data.remaining[id] = pit->data.reload[id];
        core_machine_pit_set_output_level(pit, id, LIB_TRUE, LIB_TRUE);
        if (pit->data.remaining[id] > 0u) --pit->data.remaining[id];
    } else if (pit->data.remaining[id] <= 1u) {
        pit->data.remaining[id] = 0u;
        pit->data.flagPulseLow[id] = LIB_TRUE;
        core_machine_pit_set_output_level(pit, id, LIB_FALSE, LIB_TRUE);
    } else {
        --pit->data.remaining[id];
    }
    core_machine_pit_sync_count(pit, id);
}

static void core_machine_pit_tick_mode3(t_pit *pit, lib_u8 id)
{
    lib_u8 was_output;
    if (pit->data.flagRestart[id]) {
        core_machine_pit_commit_pending(pit, id);
        return;
    }
    if (pit->data.flagLoadPending[id] && !pit->data.flagActive[id]) {
        core_machine_pit_commit_pending(pit, id);
        return;
    }
    if (!pit->data.flagActive[id] || !pit->connect.flagGate[id]) return;
    if (pit->data.remaining[id] > 1u) pit->data.remaining[id] -= 2u;
    else pit->data.remaining[id] = 0u;
    if (--pit->data.phase[id] == 0u) {
        was_output = pit->data.flagOutput[id];
        if (pit->data.flagLoadPending[id]) core_machine_pit_commit_pending(pit, id);
        pit->data.remaining[id] = core_machine_pit_mode3_count(pit, id);
        if (was_output) {
            pit->data.phase[id] = core_machine_pit_mode3_low_length(pit, id);
            if (pit->data.phase[id] == 0u) {
                pit->data.phase[id] = core_machine_pit_mode3_high_length(pit, id);
            } else {
                core_machine_pit_set_output_level(pit, id, LIB_FALSE, LIB_TRUE);
            }
        } else {
            pit->data.phase[id] = core_machine_pit_mode3_high_length(pit, id);
            core_machine_pit_set_output_level(pit, id, LIB_TRUE, LIB_TRUE);
        }
    }
    core_machine_pit_sync_count(pit, id);
}

static void core_machine_pit_tick_mode4_or_5(t_pit *pit,
    lib_u8 id)
{
    if (core_machine_pit_mode(pit, id) == 4u && pit->data.flagLoadPending[id]) {
        core_machine_pit_commit_pending(pit, id);
        return;
    }
    if (core_machine_pit_mode(pit, id) == 5u && pit->data.flagTrigger[id]) {
        pit->data.flagTrigger[id] = LIB_FALSE;
        if (pit->data.flagLoadPending[id]) core_machine_pit_commit_pending(pit, id);
        else {
            pit->data.remaining[id] = pit->data.reload[id];
            pit->data.flagPulseLow[id] = LIB_FALSE;
            core_machine_pit_sync_count(pit, id);
        }
        pit->data.flagActive[id] = LIB_TRUE;
        core_machine_pit_set_output_level(pit, id, LIB_TRUE, LIB_FALSE);
        return;
    }
    if (!pit->data.flagActive[id]) return;
    if (core_machine_pit_mode(pit, id) == 4u && !pit->connect.flagGate[id]) {
        return;
    }
    if (pit->data.flagPulseLow[id]) {
        pit->data.flagPulseLow[id] = LIB_FALSE;
        pit->data.flagActive[id] = LIB_FALSE;
        core_machine_pit_set_output_level(pit, id, LIB_TRUE, LIB_TRUE);
    } else if (--pit->data.remaining[id] == 0u) {
        pit->data.flagPulseLow[id] = LIB_TRUE;
        core_machine_pit_set_output_level(pit, id, LIB_FALSE, LIB_TRUE);
    }
    core_machine_pit_sync_count(pit, id);
}

static void core_machine_pit_tick(t_pit *pit, lib_u8 id)
{
    /* A completed CR write is intentionally not ready until this CLK commits
     * it into CE; it must nevertheless reach its mode-specific load edge. */
    if (!pit->data.flagReady[id] && !pit->data.flagLoadPending[id] &&
        !pit->data.flagTrigger[id]) return;
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

static void io_read_0040(t_port *port, lib_u16 port_id, void *owner)
{ (void)port_id; core_machine_pit_read((t_pit *)owner, port, 0u); }
static void io_read_0041(t_port *port, lib_u16 port_id, void *owner)
{ (void)port_id; core_machine_pit_read((t_pit *)owner, port, 1u); }
static void io_read_0042(t_port *port, lib_u16 port_id, void *owner)
{ (void)port_id; core_machine_pit_read((t_pit *)owner, port, 2u); }
static void io_write_0040(t_port *port, lib_u16 port_id, void *owner)
{ (void)port_id; core_machine_pit_write((t_pit *)owner, port, 0u); }
static void io_write_0041(t_port *port, lib_u16 port_id, void *owner)
{ (void)port_id; core_machine_pit_write((t_pit *)owner, port, 1u); }
static void io_write_0042(t_port *port, lib_u16 port_id, void *owner)
{ (void)port_id; core_machine_pit_write((t_pit *)owner, port, 2u); }

static void io_write_0043(t_port *port, lib_u16 port_id, void *owner)
{
    t_pit *pit = (t_pit *)owner;
    lib_u8 id = VPIT_GetCW_SC(port->data.ioByte);
    lib_u8 selected;
    (void)port_id;
    if (id == 3u) {
        if (pit->personality != CORE_MACHINE_PIT_PERSONALITY_8254) return;
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
    pit->data.flagLatch[id] = LIB_FALSE;
    pit->data.flagStatusLatch[id] = LIB_FALSE;
    pit->data.cw[id] = port->data.ioByte;
    pit->data.flagReady[id] = LIB_FALSE;
    pit->data.flagLoadPending[id] = LIB_FALSE;
    pit->data.flagTrigger[id] = LIB_FALSE;
    pit->data.flagRestart[id] = LIB_FALSE;
    pit->data.flagActive[id] = LIB_FALSE;
    pit->data.flagPulseLow[id] = LIB_FALSE;
    pit->data.remaining[id] = 0u;
    pit->data.phase[id] = 0u;
    pit->data.count[id] = 0u;
    pit->data.flagRead[id] = VPIT_GetCW_RW(port->data.ioByte) == 2u ?
        VPIT_STATUS_RW_MSB : VPIT_STATUS_RW_LSB;
    pit->data.flagWrite[id] = pit->data.flagRead[id];
    core_machine_pit_set_output_level(pit, id,
        core_machine_pit_mode(pit, id) != 0u, LIB_FALSE);
}

void core_machine_pit_set_output(t_pit *pit, lib_u8 id,
    core_machine_pit_output_provider provider, void *owner)
{
    if (pit == LIB_NULL || id >= 3u) return;
    pit->connect.output[id] = provider;
    pit->connect.output_owner[id] = owner;
}

void core_machine_pit_set_gate(t_pit *pit, lib_u8 id,
    lib_u8 asserted)
{
    lib_u8 was_asserted;
    lib_u8 mode;
    if (pit == LIB_NULL || id >= 3u) return;
    was_asserted = pit->connect.flagGate[id];
    if (was_asserted == asserted) return;
    pit->connect.flagGate[id] = asserted;
    mode = core_machine_pit_mode(pit, id);
    if (!asserted) {
        if (mode == 2u || mode == 3u) {
            pit->data.flagPulseLow[id] = LIB_FALSE;
            core_machine_pit_set_output_level(pit, id, LIB_TRUE, LIB_FALSE);
        }
        return;
    }
    if (mode == 1u || mode == 5u) {
        if (!pit->data.flagReady[id] && !pit->data.flagLoadPending[id]) return;
        pit->data.flagTrigger[id] = LIB_TRUE;
    } else if (mode == 2u || mode == 3u) {
        if (pit->data.flagReady[id] || pit->data.flagLoadPending[id]) {
            pit->data.flagRestart[id] = LIB_TRUE;
        }
    }
}

lib_u8 core_machine_pit_get_output(const t_pit *pit, lib_u8 id)
{
    return pit != LIB_NULL && id < 3u ? pit->data.flagOutput[id] : LIB_FALSE;
}

lib_status core_machine_pit_ticks_until_output(const t_pit *pit,
    lib_u8 id, lib_u64 *out_ticks)
{
    lib_u8 mode;

    if (pit == LIB_NULL || out_ticks == LIB_NULL || id >= 3u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (pit->data.flagLoadPending[id] || pit->data.flagRestart[id] ||
        pit->data.flagTrigger[id]) {
        *out_ticks = 1u;
        return LIB_STATUS_OK;
    }
    if (!pit->data.flagActive[id] || !pit->connect.flagGate[id]) {
        return LIB_STATUS_INVALID_STATE;
    }
    mode = core_machine_pit_mode(pit, id);
    switch (mode) {
    case 0u:
    case 1u:
        *out_ticks = pit->data.remaining[id];
        break;
    case 2u:
        *out_ticks = pit->data.flagPulseLow[id] ||
            pit->data.remaining[id] <= 1u ? 1u : pit->data.remaining[id] - 1u;
        break;
    case 3u:
        *out_ticks = pit->data.phase[id];
        break;
    case 4u:
    case 5u:
        *out_ticks = pit->data.flagPulseLow[id] ? 1u : pit->data.remaining[id];
        break;
    default:
        return LIB_STATUS_INVALID_STATE;
    }
    return *out_ticks == 0u ? LIB_STATUS_INVALID_STATE : LIB_STATUS_OK;
}

void core_machine_pit_initialize_at(t_pit *pit, t_port *port,
    lib_u16 base_port)
{
    if (pit == LIB_NULL || port == LIB_NULL || base_port > 0xfffcu) return;
    lib_memory_set((void *)pit, 0u, sizeof(*pit));
    core_machine_port_add_read(port, base_port, io_read_0040, pit);
    core_machine_port_add_read(port, (lib_u16)(base_port + 1u),
        io_read_0041, pit);
    core_machine_port_add_read(port, (lib_u16)(base_port + 2u),
        io_read_0042, pit);
    core_machine_port_add_write(port, base_port, io_write_0040, pit);
    core_machine_port_add_write(port, (lib_u16)(base_port + 1u),
        io_write_0041, pit);
    core_machine_port_add_write(port, (lib_u16)(base_port + 2u),
        io_write_0042, pit);
    core_machine_port_add_write(port, (lib_u16)(base_port + 3u),
        io_write_0043, pit);
}

void core_machine_pit_initialize_as(t_pit *pit, t_port *port,
    core_machine_pit_personality personality)
{
    if (personality != CORE_MACHINE_PIT_PERSONALITY_8254 &&
        personality != CORE_MACHINE_PIT_PERSONALITY_8253) return;
    core_machine_pit_initialize_at(pit, port, 0x0040u);
    if (pit != LIB_NULL && port != LIB_NULL) pit->personality = personality;
}

void core_machine_pit_initialize(t_pit *pit, t_port *port)
{
    core_machine_pit_initialize_as(pit, port, CORE_MACHINE_PIT_PERSONALITY_8254);
}

void core_machine_pit_reset(t_pit *pit)
{
    lib_uptr id;
    if (pit == LIB_NULL) return;
    /*
     * The PIT owns the output level but not its consumer.  Drop every live
     * output before clearing the local latch so a bound PIC source also
     * releases its asserted state across a machine reset.
     */
    for (id = 0u; id < 3u; ++id) {
        if (pit->data.flagOutput[id] && pit->connect.output[id] != LIB_NULL) {
            pit->connect.output[id](pit->connect.output_owner[id], LIB_FALSE);
        }
    }
    lib_memory_set((void *)&pit->data, 0u, sizeof(pit->data));
    for (id = 0u; id < 3u; ++id) {
        pit->data.flagReady[id] = LIB_TRUE;
        pit->data.flagRead[id] = VPIT_STATUS_RW_READY;
        pit->data.flagWrite[id] = VPIT_STATUS_RW_READY;
        pit->connect.flagGate[id] = LIB_TRUE;
    }
}

void core_machine_pit_advance(t_pit *pit, lib_u64 elapsed_ticks)
{
    lib_u64 tick;
    lib_u8 id;
    if (pit == LIB_NULL) return;
    for (tick = 0u; tick < elapsed_ticks; ++tick) {
        for (id = 0u; id < 3u; ++id) core_machine_pit_tick(pit, id);
    }
}

void core_machine_pit_finalize(t_pit *pit)
{
    lib_uptr id;
    if (pit == LIB_NULL) return;
    for (id = 0u; id < 3u; ++id) {
        if (pit->data.flagOutput[id] && pit->connect.output[id] != LIB_NULL) {
            pit->connect.output[id](pit->connect.output_owner[id], LIB_FALSE);
        }
    }
}
