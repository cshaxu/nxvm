#ifndef TEST_X86_FDC_FIXTURE_H
#define TEST_X86_FDC_FIXTURE_H
#include "x86/devices/fdc8272/fdc8272_interface.h"

typedef struct fixture {
    x86_fdc *chip;
    lib_u64 now;
    lib_u16 cylinder[4];
    lib_u16 step_count[4];
    lib_u8 bytes[1536];
    lib_u8 ready_mask;
    lib_bool deleted[3];
    lib_bool irq, drq;
    lib_bool present, protected;
    lib_bool track_zero_missing, channel_unreadable;
    lib_u32 terminals, read_count, track_queries;
} fixture;

static x86_fdc_pins sample(void *context, lib_u8 unit)
{
    fixture *state = context;
    return (x86_fdc_pins) {.ready = (state->ready_mask & (1u << unit)) != 0u,
        .track_zero = !state->track_zero_missing && state->cylinder[unit] == 0u,
        .two_sided = LIB_TRUE, .write_protected = state->protected,
        .data_available = state->present};
}

static void step(void *context, lib_u8 unit, lib_bool outward)
{
    fixture *state = context;
    ++state->step_count[unit];
    if (outward) {
        if (state->cylinder[unit] != 0u) --state->cylinder[unit];
    } else ++state->cylinder[unit];
}

static lib_bool track(void *context, lib_u8 unit, lib_bool mfm, x86_fdc_track *out_track)
{
    fixture *state = context;
    (void)mfm;
    ++state->track_queries;
    *out_track = (x86_fdc_track) {.cylinder = state->cylinder[unit]};
    if (!state->present) return LIB_FALSE;
    *out_track = (x86_fdc_track) {state->cylinder[unit], 80u, 2u, 3u, 512u,
        !state->channel_unreadable};
    return LIB_TRUE;
}

static x86_fdc_record_result read_byte(void *context, lib_u8 unit,
    const x86_fdc_record *record, lib_u8 *out_byte)
{
    fixture *state = context;
    (void)unit;
    if (!state->present || record->sector == 0u || record->sector > 3u ||
        record->offset >= 512u) return X86_FDC_RECORD_ABSENT;
    ++state->read_count;
    *out_byte = state->bytes[(record->sector - 1u) * 512u + record->offset];
    return X86_FDC_RECORD_OK;
}

static x86_fdc_record_result write_byte(void *context, lib_u8 unit,
    const x86_fdc_record *record, lib_u8 byte)
{
    fixture *state = context;
    lib_u8 old;
    x86_fdc_record_result result = read_byte(context, unit, record, &old);
    if (state->protected) return X86_FDC_RECORD_PROTECTED;
    if (result != X86_FDC_RECORD_OK) return result;
    state->bytes[(record->sector - 1u) * 512u + record->offset] = byte;
    return X86_FDC_RECORD_OK;
}

static x86_fdc_record_result read_mark(void *context, lib_u8 unit,
    const x86_fdc_record *record, lib_bool *out_deleted)
{
    fixture *state = context;
    (void)unit;
    if (record->sector == 0u || record->sector > 3u) return X86_FDC_RECORD_ABSENT;
    *out_deleted = state->deleted[record->sector - 1u];
    return X86_FDC_RECORD_OK;
}

static x86_fdc_record_result write_mark(void *context, lib_u8 unit,
    const x86_fdc_record *record, lib_bool deleted)
{
    fixture *state = context;
    (void)unit;
    if (state->protected) return X86_FDC_RECORD_PROTECTED;
    if (record->sector == 0u || record->sector > 3u) return X86_FDC_RECORD_ABSENT;
    state->deleted[record->sector - 1u] = deleted;
    return X86_FDC_RECORD_OK;
}

static x86_fdc_record_result format(void *context, lib_u8 unit,
    const x86_fdc_record *record, lib_u8 fill)
{
    fixture *state = context;
    x86_fdc_record_result result = write_mark(context, unit, record, LIB_FALSE);
    if (result != X86_FDC_RECORD_OK) return result;
    lib_memory_set(state->bytes + (record->sector - 1u) * 512u, fill, 512u);
    return X86_FDC_RECORD_OK;
}

static void irq(void *context, lib_bool asserted) { ((fixture *)context)->irq = asserted; }
static void drq(void *context, lib_bool asserted) { ((fixture *)context)->drq = asserted; }
static void terminal(void *context, const x86_fdc_terminal *result)
{
    fixture *state = context;
    (void)result;
    ++state->terminals;
}

static void advance(fixture *state)
{
    x86_fdc_advance_at(state->chip, ++state->now);
}

static void fixture_command(fixture *state, const lib_u8 *bytes, lib_size count)
{
    for (lib_size index = 0u; index < count; ++index) x86_fdc_write_data(state->chip, bytes[index]);
    advance(state);
}

static inline void fixture_arm(fixture *state)
{
    x86_fdc_reset(state->chip);
    x86_fdc_set_service_enabled(state->chip, LIB_TRUE);
    x86_fdc_set_reset(state->chip, LIB_TRUE);
}

static inline lib_bool fixture_result(fixture *state, lib_u8 *result, lib_size count)
{
    advance(state);
    for (lib_size index = 0u; index < count; ++index) {
        if ((x86_fdc_read_status(state->chip) & 0xc0u) != 0xc0u) return LIB_FALSE;
        x86_fdc_read_data(state->chip, &result[index]);
    }
    return (x86_fdc_read_status(state->chip) & 0x50u) == 0u;
}

static inline lib_bool fixture_finish_seeks(fixture *state)
{
    for (lib_u16 steps = 0u; steps < 1024u; ++steps) {
        if ((x86_fdc_read_status(state->chip) & 15u) == 0u) return LIB_TRUE;
        if (x86_fdc_next_due_tick(state->chip, &state->now) != LIB_STATUS_OK) return LIB_FALSE;
        x86_fdc_advance_at(state->chip, state->now);
    }
    return LIB_FALSE;
}

static lib_status fixture_create(fixture *state, const x86_fdc_timing *timing)
{
    const x86_fdc_connection connection = {
        .sample = sample, .step = step, .track = track, .read = read_byte, .write = write_byte,
        .read_mark = read_mark, .write_mark = write_mark, .format = format,
        .irq = irq, .drq = drq, .terminal = terminal, .context = state
    };
    return x86_fdc_create(&connection, timing, &state->chip);
}
#endif
