/* Copyright 2012-2014 Neko. */

/* VPIT implements the deterministic elapsed-tick subset of Intel 8254. */
#include "lib/types/types_interface.h"
#include "x86/chips/pit825x/pit.h"

static lib_u8 x86_pit_mode(const x86_pit *pit,
    lib_u8 id)
{
    lib_u8 mode = VPIT_GetCW_M(pit->data.cw[id]);
    return mode == 6u ? 2u : mode == 7u ? 3u : mode;
}

static lib_u32 x86_pit_bcd_decode(lib_u16 value)
{
    return (lib_u32)(value & 0x000fu) +
        (lib_u32)((value >> 4) & 0x000fu) * 10u +
        (lib_u32)((value >> 8) & 0x000fu) * 100u +
        (lib_u32)((value >> 12) & 0x000fu) * 1000u;
}

static lib_u16 x86_pit_bcd_encode(lib_u32 value)
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

static lib_u32 x86_pit_decode_reload(const x86_pit *pit,
    lib_u8 id)
{
    if ((pit->data.cw[id] & VPIT_CW_BCD) != 0u) {
        lib_u32 result = x86_pit_bcd_decode(pit->data.init[id]);
        return result == 0u ? 10000u : result;
    }
    return pit->data.init[id] == 0u ? 65536u : pit->data.init[id];
}

static lib_u16 x86_pit_encode_count(const x86_pit *pit,
    lib_u8 id, lib_u32 count)
{
    if ((pit->data.cw[id] & VPIT_CW_BCD) != 0u) {
        return count == 10000u ? 0u : x86_pit_bcd_encode(count);
    }
    return count == 65536u ? 0u : (lib_u16)count;
}

static void x86_pit_sync_count(x86_pit *pit, lib_u8 id)
{
    pit->data.count[id] = x86_pit_encode_count(pit, id,
        pit->data.remaining[id]);
}

static void x86_pit_set_output_level(x86_pit *pit,
    lib_u8 id, lib_bool asserted, lib_bool notify_rise)
{
    if (pit->data.flagOutput[id] == asserted) return;
    pit->data.flagOutput[id] = asserted;
    if (pit->connect.output[id] == LIB_NULL) return;
    if (!asserted || notify_rise) {
        pit->connect.output[id](pit->connect.output_owner[id], asserted);
    }
}

static lib_u32 x86_pit_mode3_high_length(const x86_pit *pit,
    lib_u8 id)
{
    return (pit->data.reload[id] + 1u) / 2u;
}

static lib_u32 x86_pit_mode3_low_length(const x86_pit *pit,
    lib_u8 id)
{
    return pit->data.reload[id] / 2u;
}

static lib_u32 x86_pit_mode3_count(const x86_pit *pit,
    lib_u8 id)
{
    return pit->data.reload[id] - (pit->data.reload[id] & 1u);
}

static void x86_pit_load(x86_pit *pit, lib_u8 id)
{
    lib_u8 mode = x86_pit_mode(pit, id);

    pit->data.reload[id] = x86_pit_decode_reload(pit, id);
    pit->data.remaining[id] = mode == 3u ?
        x86_pit_mode3_count(pit, id) : pit->data.reload[id];
    pit->data.phase[id] = mode == 3u ?
        x86_pit_mode3_high_length(pit, id) : 0u;
    pit->data.flagReady[id] = LIB_TRUE;
    pit->data.flagLoadPending[id] = LIB_FALSE;
    pit->data.flagRestart[id] = LIB_FALSE;
    pit->data.flagActive[id] = mode != 1u && mode != 5u;
    pit->data.flagPulseLow[id] = LIB_FALSE;
    x86_pit_sync_count(pit, id);

}

static t_pit_data_status_rw x86_pit_read_start(const x86_pit *pit,
    lib_u8 id)
{
    return VPIT_GetCW_RW(pit->data.cw[id]) == 0x02 ?
        VPIT_STATUS_RW_MSB : VPIT_STATUS_RW_LSB;
}

static lib_u8 x86_pit_capture_status(const x86_pit *pit,
    lib_u8 id)
{
    lib_u8 status = pit->data.cw[id] &
        (VPIT_SB_BCD | VPIT_SB_M | VPIT_SB_RW);

    if (pit->data.flagOutput[id]) status |= VPIT_SB_OUT;
    if (!pit->data.flagReady[id]) status |= VPIT_SB_NC;
    return status;
}

static void x86_pit_latch_count(x86_pit *pit, lib_u8 id)
{
    if (pit->data.flagLatch[id]) return;
    x86_pit_sync_count(pit, id);
    pit->data.latch[id] = pit->data.count[id];
    pit->data.flagLatch[id] = LIB_TRUE;
    pit->data.flagRead[id] = x86_pit_read_start(pit, id);
}

static void x86_pit_latch_status(x86_pit *pit, lib_u8 id)
{
    if (pit->data.flagStatusLatch[id]) return;
    pit->data.status_latch[id] = x86_pit_capture_status(pit, id);
    pit->data.flagStatusLatch[id] = LIB_TRUE;
}

lib_status x86_pit_read_counter(x86_pit *pit, lib_u8 id,
    lib_u8 *inout_value)
{
    lib_u16 value;
    if (pit == LIB_NULL || inout_value == LIB_NULL || id >= 3u)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (pit->data.flagStatusLatch[id]) {
        *inout_value = pit->data.status_latch[id];
        pit->data.flagStatusLatch[id] = LIB_FALSE;
        return LIB_STATUS_OK;
    }
    if (pit->data.flagLatch[id]) {
        value = pit->data.latch[id];
    } else {
        x86_pit_sync_count(pit, id);
        value = pit->data.count[id];
    }
    switch (VPIT_GetCW_RW(pit->data.cw[id])) {
    case 0x01:
        *inout_value = (lib_u8)(value);
        pit->data.flagRead[id] = VPIT_STATUS_RW_READY;
        pit->data.flagLatch[id] = LIB_FALSE;
        break;
    case 0x02:
        *inout_value = (lib_u8)(value >> 8);
        pit->data.flagRead[id] = VPIT_STATUS_RW_READY;
        pit->data.flagLatch[id] = LIB_FALSE;
        break;
    case 0x03:
        if (pit->data.flagRead[id] == VPIT_STATUS_RW_MSB) {
            *inout_value = (lib_u8)(value >> 8);
            pit->data.flagRead[id] = VPIT_STATUS_RW_READY;
            pit->data.flagLatch[id] = LIB_FALSE;
        } else {
            *inout_value = (lib_u8)(value);
            pit->data.flagRead[id] = VPIT_STATUS_RW_MSB;
        }
        break;
    default:
        pit->data.flagLatch[id] = LIB_FALSE;
        break;
    }
    return LIB_STATUS_OK;
}

static void x86_pit_write_counter(x86_pit *pit, lib_u8 id,
    lib_u8 value)
{
    lib_bool complete = LIB_FALSE;
    switch (VPIT_GetCW_RW(pit->data.cw[id])) {
    case 0x01:
        pit->data.init[id] = (lib_u16)value;
        complete = LIB_TRUE;
        break;
    case 0x02:
        pit->data.init[id] = (lib_u16)(value << 8);
        complete = LIB_TRUE;
        break;
    case 0x03:
        if (pit->data.flagWrite[id] == VPIT_STATUS_RW_MSB) {
            pit->data.init[id] = (lib_u16)(value << 8) |
                (lib_u8)(pit->data.init[id]);
            complete = LIB_TRUE;
        } else {
            pit->data.init[id] = (lib_u16)value;
            pit->data.flagWrite[id] = VPIT_STATUS_RW_MSB;
            if (x86_pit_mode(pit, id) == 0u) {
                pit->data.flagReady[id] = LIB_FALSE;
                pit->data.flagActive[id] = LIB_FALSE;
                pit->data.flagPulseLow[id] = LIB_FALSE;
                x86_pit_set_output_level(pit, id, LIB_FALSE, LIB_TRUE);
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
        if (x86_pit_mode(pit, id) == 0u) {
            pit->data.flagActive[id] = LIB_FALSE;
            pit->data.flagPulseLow[id] = LIB_FALSE;
            x86_pit_set_output_level(pit, id, LIB_FALSE, LIB_TRUE);
        }
    }
}

static void x86_pit_commit_pending(x86_pit *pit, lib_u8 id)
{
    x86_pit_load(pit, id);
}

static void x86_pit_tick_mode0(x86_pit *pit, lib_u8 id)
{
    if (pit->data.flagLoadPending[id]) {
        x86_pit_commit_pending(pit, id);
        return;
    }
    if (!pit->data.flagActive[id] || !pit->connect.flagGate[id]) return;
    if (--pit->data.remaining[id] == 0u) {
        pit->data.flagActive[id] = LIB_FALSE;
        x86_pit_set_output_level(pit, id, LIB_TRUE, LIB_TRUE);
    }
    x86_pit_sync_count(pit, id);
}

static void x86_pit_tick_mode1(x86_pit *pit, lib_u8 id)
{
    /* Mode 1 loads CE on the clock following a completed count write.  The
     * later gate trigger starts the one-shot; it does not defer that load. */
    if (pit->data.flagLoadPending[id]) {
        x86_pit_commit_pending(pit, id);
        return;
    }
    if (pit->data.flagTrigger[id]) {
        pit->data.flagTrigger[id] = LIB_FALSE;
        pit->data.remaining[id] = pit->data.reload[id];
        pit->data.flagPulseLow[id] = LIB_FALSE;
        x86_pit_sync_count(pit, id);
        pit->data.flagActive[id] = LIB_TRUE;
        x86_pit_set_output_level(pit, id, LIB_FALSE, LIB_TRUE);
        return;
    }
    if (!pit->data.flagActive[id]) return;
    if (--pit->data.remaining[id] == 0u) {
        pit->data.flagActive[id] = LIB_FALSE;
        x86_pit_set_output_level(pit, id, LIB_TRUE, LIB_TRUE);
    }
    x86_pit_sync_count(pit, id);
}

static void x86_pit_tick_mode2(x86_pit *pit, lib_u8 id)
{
    if (pit->data.flagRestart[id]) {
        x86_pit_commit_pending(pit, id);
        return;
    }
    if (pit->data.flagLoadPending[id] && !pit->data.flagActive[id]) {
        x86_pit_commit_pending(pit, id);
        return;
    }
    if (!pit->data.flagActive[id] || !pit->connect.flagGate[id]) return;
    if (pit->data.flagPulseLow[id]) {
        pit->data.flagPulseLow[id] = LIB_FALSE;
        if (pit->data.flagLoadPending[id]) x86_pit_commit_pending(pit, id);
        else pit->data.remaining[id] = pit->data.reload[id];
        x86_pit_set_output_level(pit, id, LIB_TRUE, LIB_TRUE);
        if (pit->data.remaining[id] > 0u) --pit->data.remaining[id];
    } else if (pit->data.remaining[id] <= 1u) {
        pit->data.remaining[id] = 0u;
        pit->data.flagPulseLow[id] = LIB_TRUE;
        x86_pit_set_output_level(pit, id, LIB_FALSE, LIB_TRUE);
    } else {
        --pit->data.remaining[id];
    }
    x86_pit_sync_count(pit, id);
}

static void x86_pit_tick_mode3(x86_pit *pit, lib_u8 id)
{
    lib_bool was_output;
    if (pit->data.flagRestart[id]) {
        x86_pit_commit_pending(pit, id);
        return;
    }
    if (pit->data.flagLoadPending[id] && !pit->data.flagActive[id]) {
        x86_pit_commit_pending(pit, id);
        return;
    }
    if (!pit->data.flagActive[id] || !pit->connect.flagGate[id]) return;
    if (pit->data.remaining[id] > 1u) pit->data.remaining[id] -= 2u;
    else pit->data.remaining[id] = 0u;
    if (--pit->data.phase[id] == 0u) {
        was_output = pit->data.flagOutput[id];
        if (pit->data.flagLoadPending[id]) x86_pit_commit_pending(pit, id);
        pit->data.remaining[id] = x86_pit_mode3_count(pit, id);
        if (was_output) {
            pit->data.phase[id] = x86_pit_mode3_low_length(pit, id);
            if (pit->data.phase[id] == 0u) {
                pit->data.phase[id] = x86_pit_mode3_high_length(pit, id);
            } else {
                x86_pit_set_output_level(pit, id, LIB_FALSE, LIB_TRUE);
            }
        } else {
            pit->data.phase[id] = x86_pit_mode3_high_length(pit, id);
            x86_pit_set_output_level(pit, id, LIB_TRUE, LIB_TRUE);
        }
    }
    x86_pit_sync_count(pit, id);
}

static void x86_pit_tick_mode4_or_5(x86_pit *pit,
    lib_u8 id)
{
    if (x86_pit_mode(pit, id) == 4u && pit->data.flagLoadPending[id]) {
        x86_pit_commit_pending(pit, id);
        return;
    }
    if (x86_pit_mode(pit, id) == 5u && pit->data.flagTrigger[id]) {
        pit->data.flagTrigger[id] = LIB_FALSE;
        if (pit->data.flagLoadPending[id]) x86_pit_commit_pending(pit, id);
        else {
            pit->data.remaining[id] = pit->data.reload[id];
            pit->data.flagPulseLow[id] = LIB_FALSE;
            x86_pit_sync_count(pit, id);
        }
        pit->data.flagActive[id] = LIB_TRUE;
        x86_pit_set_output_level(pit, id, LIB_TRUE, LIB_FALSE);
        return;
    }
    if (!pit->data.flagActive[id]) return;
    if (x86_pit_mode(pit, id) == 4u && !pit->connect.flagGate[id]) {
        return;
    }
    if (pit->data.flagPulseLow[id]) {
        pit->data.flagPulseLow[id] = LIB_FALSE;
        pit->data.flagActive[id] = LIB_FALSE;
        x86_pit_set_output_level(pit, id, LIB_TRUE, LIB_TRUE);
    } else if (--pit->data.remaining[id] == 0u) {
        pit->data.flagPulseLow[id] = LIB_TRUE;
        x86_pit_set_output_level(pit, id, LIB_FALSE, LIB_TRUE);
    }
    x86_pit_sync_count(pit, id);
}

static void x86_pit_tick(x86_pit *pit, lib_u8 id)
{
    /* A completed CR write is intentionally not ready until this CLK commits
     * it into CE; it must nevertheless reach its mode-specific load edge. */
    if (!pit->data.flagReady[id] && !pit->data.flagLoadPending[id] &&
        !pit->data.flagTrigger[id]) return;
    switch (x86_pit_mode(pit, id)) {
    case 0u: x86_pit_tick_mode0(pit, id); break;
    case 1u: x86_pit_tick_mode1(pit, id); break;
    case 2u: x86_pit_tick_mode2(pit, id); break;
    case 3u: x86_pit_tick_mode3(pit, id); break;
    case 4u:
    case 5u: x86_pit_tick_mode4_or_5(pit, id); break;
    default: break;
    }
}

static void x86_pit_write_control(x86_pit *pit, lib_u8 value)
{
    lib_u8 id = VPIT_GetCW_SC(value);
    lib_u8 selected;
    if (id == 3u) {
        if (pit->personality != X86_PIT_PERSONALITY_8254) return;
        for (selected = 0u; selected < 3u; ++selected) {
            if ((value & VPIT_RB_CNT(selected)) != 0u) continue;
            if ((value & VPIT_RB_COUNT) == 0u) {
                x86_pit_latch_count(pit, selected);
            }
            if ((value & VPIT_RB_STATUS) == 0u) {
                x86_pit_latch_status(pit, selected);
            }
        }
        return;
    }
    if (VPIT_GetCW_RW(value) == 0u) {
        x86_pit_latch_count(pit, id);
        return;
    }
    pit->data.flagLatch[id] = LIB_FALSE;
    pit->data.flagStatusLatch[id] = LIB_FALSE;
    pit->data.cw[id] = value;
    pit->data.flagReady[id] = LIB_FALSE;
    pit->data.flagLoadPending[id] = LIB_FALSE;
    pit->data.flagTrigger[id] = LIB_FALSE;
    pit->data.flagRestart[id] = LIB_FALSE;
    pit->data.flagActive[id] = LIB_FALSE;
    pit->data.flagPulseLow[id] = LIB_FALSE;
    pit->data.remaining[id] = 0u;
    pit->data.phase[id] = 0u;
    pit->data.count[id] = 0u;
    pit->data.flagRead[id] = VPIT_GetCW_RW(value) == 2u ?
        VPIT_STATUS_RW_MSB : VPIT_STATUS_RW_LSB;
    pit->data.flagWrite[id] = pit->data.flagRead[id];
    x86_pit_set_output_level(pit, id,
        x86_pit_mode(pit, id) != 0u, LIB_FALSE);
}

void x86_pit_set_output(x86_pit *pit, lib_u8 id,
    x86_pit_output_provider provider, void *owner)
{
    if (pit == LIB_NULL || id >= 3u) return;
    pit->connect.output[id] = provider;
    pit->connect.output_owner[id] = owner;
}

void x86_pit_set_gate(x86_pit *pit, lib_u8 id,
    lib_bool asserted)
{
    lib_bool was_asserted;
    lib_u8 mode;
    if (pit == LIB_NULL || id >= 3u) return;
    was_asserted = pit->connect.flagGate[id];
    if (was_asserted == asserted) return;
    pit->connect.flagGate[id] = asserted;
    mode = x86_pit_mode(pit, id);
    if (!asserted) {
        if (mode == 2u || mode == 3u) {
            pit->data.flagPulseLow[id] = LIB_FALSE;
            x86_pit_set_output_level(pit, id, LIB_TRUE, LIB_FALSE);
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

lib_bool x86_pit_get_output(const x86_pit *pit, lib_u8 id)
{
    return pit != LIB_NULL && id < 3u ? pit->data.flagOutput[id] : LIB_FALSE;
}

lib_status x86_pit_ticks_until_output(const x86_pit *pit,
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
    mode = x86_pit_mode(pit, id);
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

lib_status x86_pit_write_register(x86_pit *pit, lib_u8 selector, lib_u8 value)
{
    if (pit == LIB_NULL || selector >= 4u) return LIB_STATUS_INVALID_ARGUMENT;
    if (selector == 3u) x86_pit_write_control(pit, value);
    else x86_pit_write_counter(pit, selector, value);
    return LIB_STATUS_OK;
}

lib_status x86_pit_create(x86_pit_personality personality, x86_pit **out_pit)
{
    x86_pit *pit;
    if (out_pit == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_pit = LIB_NULL;
    if (personality != X86_PIT_PERSONALITY_8254 &&
        personality != X86_PIT_PERSONALITY_8253) return LIB_STATUS_INVALID_ARGUMENT;
    pit = lib_allocate_zero(1u, sizeof(*pit));
    if (pit == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    pit->personality = personality;
    *out_pit = pit;
    return LIB_STATUS_OK;
}

void x86_pit_reset(x86_pit *pit)
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

void x86_pit_advance(x86_pit *pit, lib_u64 elapsed_ticks)
{
    lib_u64 tick;
    lib_u8 id;
    if (pit == LIB_NULL) return;
    for (tick = 0u; tick < elapsed_ticks; ++tick) {
        for (id = 0u; id < 3u; ++id) x86_pit_tick(pit, id);
    }
}

void x86_pit_destroy(x86_pit *pit)
{
    lib_uptr id;
    if (pit == LIB_NULL) return;
    for (id = 0u; id < 3u; ++id) {
        if (pit->data.flagOutput[id] && pit->connect.output[id] != LIB_NULL) {
            pit->connect.output[id](pit->connect.output_owner[id], LIB_FALSE);
        }
    }
    lib_release(pit);
}
