/* Original READY and seek-cause regressions, now owned by the chip. */
#include "fixture.h"
#include "x86/devices/fdc8272/fdc.h"
#include "lib/types/file.h"

static lib_bool seek_command(fixture *state, const lib_u8 *bytes, lib_size count)
{
    fixture_command(state, bytes, count);
    return fixture_finish_seeks(state);
}

static lib_i32 ready_edges(fixture *state)
{
    x86_fdc *fdc = state->chip;
    const lib_u8 saved_ready = state->ready_mask;
    const lib_bool saved_present = state->present;
    lib_u8 result[7];
    lib_i32 failed = 0;

    state->ready_mask = 0x0fu;
    fixture_arm(state);
    state->now = fdc->data.reset_due_tick;
    x86_fdc_advance_at(fdc, state->now);
    for (lib_u8 drive = 0u; drive < 4u; ++drive) {
        failed |= !seek_command(state, (const lib_u8[]){0x08u}, 1u);
        failed |= !fixture_result(state, result, 2u);
    }
    failed |= !seek_command(state, (const lib_u8[]){0x03u, 0xdfu, 0x02u}, 3u);
    state->present = LIB_FALSE;
    x86_fdc_poll_ready(fdc);
    failed |= state->irq;
    state->present = LIB_TRUE;
    x86_fdc_poll_ready(fdc);
    failed |= state->irq;

    /* Inject electrical transitions independently from media in this fixture.
     * Production's selected IBM/Compaq boards keep READY tied high. */
    state->ready_mask = 0x0eu;
    x86_fdc_refresh(fdc);
    failed |= state->irq;
    x86_fdc_poll_ready(fdc);
    failed |= !state->irq;
    failed |= !seek_command(state, (const lib_u8[]){0x08u}, 1u);
    failed |= !fixture_result(state, result, 2u) ||
        result[0] != 0xc8u || state->irq;
    state->ready_mask = 0x0fu;
    x86_fdc_poll_ready(fdc);
    failed |= !state->irq;
    failed |= !seek_command(state, (const lib_u8[]){0x08u}, 1u);
    failed |= !fixture_result(state, result, 2u) ||
        result[0] != 0xc0u || state->irq;
    /* All four causes survive unrelated ST3 reads and retain their own PCN. */
    state->ready_mask = 0u;
    x86_fdc_poll_ready(fdc);
    failed |= !seek_command(state, (const lib_u8[]){0x4au, 0u}, 2u);
    failed |= !fixture_result(state, result, 7u) ||
        result[0] != 0x48u || !state->irq;
    for (lib_u8 drive = 0u; drive < 4u; ++drive) {
        fdc->data.pcn[drive] = drive + 3u;
        failed |= !seek_command(state, (const lib_u8[]){0x04u, drive}, 2u);
        failed |= !fixture_result(state, result, 1u) ||
            !state->irq;
        failed |= !seek_command(state, (const lib_u8[]){0x08u}, 1u);
        failed |= !fixture_result(state, result, 2u) ||
            result[0] != (0xc8u | drive) || result[1] != drive + 3u ||
            !!state->irq != (drive != 3u);
    }
    /* SEEK completion must not overwrite already observed READY changes. */
    state->ready_mask = 0x0fu;
    x86_fdc_poll_ready(fdc);
    failed |= !seek_command(state, (const lib_u8[]){0x0fu, 0u, 3u}, 3u);
    failed |= !seek_command(state, (const lib_u8[]){0x08u}, 1u);
    failed |= !fixture_result(state, result, 2u) ||
        result[0] != 0x20u || result[1] != 3u || !state->irq;
    for (lib_u8 drive = 0u; drive < 4u; ++drive) {
        failed |= !seek_command(state, (const lib_u8[]){0x08u}, 1u);
        failed |= !fixture_result(state, result, 2u) ||
            result[0] != (0xc0u | drive) || result[1] != drive + 3u;
    }
    failed |= state->irq;
    state->ready_mask = saved_ready;
    state->present = saved_present;
    x86_fdc_reset(fdc);
    if (failed) lib_c_fprintf(lib_c_stderr, "FDC READY edge/medium independence failed\n");
    return failed;
}

static lib_i32 seek_ownership(fixture *state)
{
    x86_fdc *fdc = state->chip;
    lib_u8 result[2];
    lib_i32 failed = 0;

    /* Use absent mechanics to distinguish SEEK from RECALIBRATE under the
       existing model. READY/Track0 qualification is a separate cutover gate. */
    state->track_zero_missing = LIB_TRUE;
    for (lib_u8 drive = 0u; drive < X86_FDC_DRIVE_COUNT; ++drive) {
        for (lib_u8 recalibrate = 0u; recalibrate < 2u; ++recalibrate) {
            fixture_arm(state);
            failed |= !seek_command(state, (const lib_u8[]){0x03u, 0xdfu, 0x02u}, 3u);
            failed |= !seek_command(state, (const lib_u8[]){0x0fu, drive, 4u}, 3u);
            failed |= !seek_command(state, (const lib_u8[]){0x08u}, 1u);
            failed |= !fixture_result(state, result, 2u);
            if (recalibrate) {
                fixture_command(state, (const lib_u8[]){0x07u, drive}, 2u);
            } else {
                fixture_command(state, (const lib_u8[]){0x0fu, drive, 6u}, 3u);
            }
            fixture_command(state, (const lib_u8[]){0x04u, drive}, 2u);
            failed |= !fixture_result(state, result, 1u);
            failed |= !fixture_finish_seeks(state);
            fixture_command(state, (const lib_u8[]){0x08u}, 1u);
            failed |= !fixture_result(state, result, 2u) ||
                result[0] != ((recalibrate ? 0x70u : 0x20u) | drive) ||
                result[1] != (recalibrate ? 0u : 6u);
        }
    }
    if (failed) lib_c_fprintf(lib_c_stderr, "FDC seek identity changed by intervening command\n");

    /* Mixed commands finish at their own deadlines: absent-mechanism
       recalibration requires 77 pulses, not SEEK's four PCN steps. */
    fixture_arm(state);
    failed |= !seek_command(state, (const lib_u8[]){0x03u, 0xdfu, 0x02u}, 3u);
    for (lib_u8 drive = 0u; drive < X86_FDC_DRIVE_COUNT; ++drive) {
        failed |= !seek_command(state, (const lib_u8[]){0x0fu, drive, 4u}, 3u);
        failed |= !seek_command(state, (const lib_u8[]){0x08u}, 1u);
        failed |= !fixture_result(state, result, 2u);
    }
    for (lib_u8 drive = 0u; drive < X86_FDC_DRIVE_COUNT; ++drive) {
        const lib_u8 command[] = {(drive & 1u) ? 0x07u : 0x0fu, drive, 8u};
        fixture_command(state, command, (drive & 1u) ? 2u : 3u);
    }
    failed |= !fixture_finish_seeks(state);
    for (lib_u8 index = 0u; index < X86_FDC_DRIVE_COUNT; ++index) {
        const lib_u8 drive = (const lib_u8[]){0u, 2u, 1u, 3u}[index];
        fixture_command(state, (const lib_u8[]){0x08u}, 1u);
        failed |= !fixture_result(state, result, 2u) ||
            result[0] != (((drive & 1u) ? 0x70u : 0x20u) | drive) ||
            result[1] != ((drive & 1u) ? 0u : 8u);
    }

    /* A second command cannot replace the same unit's active operation. */
    for (lib_u8 drive = 0u; drive < X86_FDC_DRIVE_COUNT; ++drive) {
        lib_u64 due;
        fixture_arm(state);
        failed |= !seek_command(state, (const lib_u8[]){0x03u, 0xdfu, 0x02u}, 3u);
        fixture_command(state, (const lib_u8[]){0x0fu, drive, 3u}, 3u);
        due = fdc->data.seek_due_tick[drive];
        fixture_command(state, (const lib_u8[]){0x0fu, drive, 7u}, 3u);
        if (fdc->data.phase != x86_fdc_PHASE_RESULT ||
            fdc->data.seek_due_tick[drive] != due ||
            fdc->data.seek_target[drive] != 3u ||
            !fixture_result(state, result, 1u) ||
            result[0] != 0x80u || state->irq) {
            lib_c_fprintf(lib_c_stderr, "FDC duplicate unit seek replaced active request\n");
            failed = 1;
        }
    }

    /* Fill the four legal outstanding slots. On the old implementation the
       fifth request is inspected before its deadline, never executing the
       out-of-bounds append as part of the negative control. */
    fixture_arm(state);
    failed |= !seek_command(state, (const lib_u8[]){0x03u, 0xdfu, 0x02u}, 3u);
    for (lib_u8 drive = 0u; drive < X86_FDC_DRIVE_COUNT; ++drive) {
        fixture_command(state, (const lib_u8[]){0x0fu, drive, 1u}, 3u);
    }
    state->now = fdc->data.seek_due_tick[3u];
    x86_fdc_advance_at(fdc, state->now);
    failed |= fdc->data.seek_result_count != X86_FDC_DRIVE_COUNT;
    fixture_command(state, (const lib_u8[]){0x0fu, 0u, 2u}, 3u);
    if (fdc->data.phase != x86_fdc_PHASE_RESULT ||
        fdc->data.seek_pending[0u] ||
        !fixture_result(state, result, 1u) || result[0] != 0x80u) {
        lib_c_fprintf(lib_c_stderr, "FDC accepted fifth undrained seek completion\n");
        failed = 1;
    } else {
        static const lib_u8 blocked[][2] = {
            {0x03u, 3u}, {0x04u, 2u}, {0x07u, 2u}, {0x0fu, 3u},
            {0x0au, 2u}, {0x05u, 9u}, {0x06u, 9u}, {0x09u, 9u},
            {0x0cu, 9u}, {0x11u, 9u}, {0x19u, 9u}, {0x1du, 9u},
            {0x0du, 6u}, {0x02u, 9u}, {0x00u, 1u}
        };
        for (lib_size index = 0u; index < sizeof(blocked) / sizeof(blocked[0]); ++index) {
            lib_u8 command[9] = {0};
            command[0] = blocked[index][0];
            fixture_command(state, command, blocked[index][1]);
            failed |= !fixture_result(state, result, 1u) ||
                result[0] != 0x80u ||
                fdc->data.seek_result_count != X86_FDC_DRIVE_COUNT;
        }
        for (lib_u8 drive = 0u; drive < X86_FDC_DRIVE_COUNT; ++drive) {
            fixture_command(state, (const lib_u8[]){0x08u}, 1u);
            failed |= !fixture_result(state, result, 2u) ||
                result[0] != (0x20u | drive) || result[1] != 1u;
        }
        failed |= fdc->data.seek_result_count != 0u;
    }
    x86_fdc_reset(fdc);
    failed |= fdc->data.seek_result_count != 0u || state->irq;
    for (lib_u8 drive = 0u; drive < X86_FDC_DRIVE_COUNT; ++drive)
        failed |= fdc->data.seek_pending[drive];
    return failed;
}

static lib_i32 specify_reset(fixture *state)
{
    x86_fdc *fdc = state->chip;
    lib_i32 failed = 0;

    fixture_arm(state);
    fixture_command(state, (const lib_u8[]){0x03u, 0xdfu, 0x03u}, 3u);
    /* Both reset edges preserve the programmed SRT/HUT/HLT fields. */
    failed |= fdc->data.srt != 0x0du || fdc->data.hut != 0x0fu ||
        fdc->data.hlt != 0x01u;
    x86_fdc_set_reset(fdc, LIB_FALSE);
    failed |= fdc->data.srt != 0x0du || fdc->data.hut != 0x0fu ||
        fdc->data.hlt != 0x01u;
    x86_fdc_set_reset(fdc, LIB_TRUE);
    state->now = fdc->data.reset_due_tick;
    x86_fdc_advance_at(fdc, state->now);
    failed |= fdc->data.srt != 0x0du || fdc->data.hut != 0x0fu ||
        fdc->data.hlt != 0x01u;
    if (failed) lib_c_fprintf(lib_c_stderr, "FDC SPECIFY reset retention failed\n");
    return failed;
}

static lib_i32 seek_cadence(fixture *state)
{
    x86_fdc *fdc = state->chip;
    const x86_fdc_timing timing = {0u, 8000u, 1u, 248u, 120u};
    lib_u8 result[2];
    lib_i32 failed = 0;

    /* A fresh local axis avoids the old mixed fixture's backward time jump.
     * Only reset latency is zero here; each step retains the 24000-tick oracle. */
    failed |= x86_fdc_set_timing(fdc, &timing) != LIB_STATUS_OK;
    fixture_arm(state);
    state->now = 0u;
    state->track_zero_missing = LIB_FALSE;
    lib_memory_set(state->cylinder, 0, sizeof(state->cylinder));
    fixture_command(state, (const lib_u8[]){0x03u, 0xdfu, 0x03u}, 3u);
    state->now = 99u;
    fixture_command(state, (const lib_u8[]){0x0fu, 0u, 3u}, 3u);
    failed |= !fdc->data.seek_pending[0u] || fdc->data.pcn[0u] != 0u ||
        state->irq || fdc->data.seek_due_tick[0u] != 24100u;
    state->now = 72099u;
    x86_fdc_advance_at(fdc, state->now);
    failed |= !fdc->data.seek_pending[0u] || state->irq || fdc->data.pcn[0u] != 2u;
    state->now = 72100u;
    x86_fdc_advance_at(fdc, state->now);
    failed |= fdc->data.pcn[0u] != 3u || !state->irq;
    /* Drain without advancing time; the second pair begins at 72101/72102. */
    x86_fdc_write_data(fdc, 0x08u);
    x86_fdc_advance_at(fdc, state->now);
    x86_fdc_read_data(fdc, &result[0]);
    x86_fdc_read_data(fdc, &result[1]);
    failed |= result[0] != 0x20u || result[1] != 3u || state->irq;

    fixture_command(state, (const lib_u8[]){0x0fu, 0u, 7u}, 3u);
    fixture_command(state, (const lib_u8[]){0x0fu, 1u, 1u}, 3u);
    failed |= !fdc->data.seek_pending[0u] || !fdc->data.seek_pending[1u] ||
        (x86_fdc_read_status(fdc) & 3u) != 3u;
    state->now = 96102u;
    x86_fdc_advance_at(fdc, state->now);
    failed |= fdc->data.seek_pending[1u] || !fdc->data.seek_pending[0u] || !state->irq;
    fixture_command(state, (const lib_u8[]){0x08u}, 1u);
    failed |= !fixture_result(state, result, 2u) ||
        (result[0] & 3u) != 1u || result[1] != 1u;
    state->now = 168101u;
    x86_fdc_advance_at(fdc, state->now);
    fixture_command(state, (const lib_u8[]){0x08u}, 1u);
    failed |= !fixture_result(state, result, 2u) ||
        (result[0] & 3u) != 0u || result[1] != 7u;
    if (failed) lib_c_fprintf(lib_c_stderr, "FDC step cadence/parallel seek failed\n");
    return failed;
}

static lib_i32 recalibrate_pulses(fixture *state)
{
    static const struct { lib_u16 cylinder, pulses, remaining; lib_u8 st0; } cases[] = {
        {39u, 39u, 0u, 0x20u}, {79u, 77u, 2u, 0x70u}, {2u, 2u, 0u, 0x20u}
    };
    lib_u8 result[2];
    lib_i32 failed = 0;

    state->track_zero_missing = LIB_FALSE;
    state->ready_mask = 0x0fu;
    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        fixture_arm(state);
        state->cylinder[0] = cases[index].cylinder;
        state->step_count[0] = 0u;
        failed |= !seek_command(state, (const lib_u8[]){0x07u, 0u}, 2u);
        fixture_command(state, (const lib_u8[]){0x08u}, 1u);
        failed |= !fixture_result(state, result, 2u) ||
            result[0] != cases[index].st0 || result[1] != 0u ||
            state->step_count[0] != cases[index].pulses ||
            state->cylinder[0] != cases[index].remaining;
    }
    state->ready_mask = 0x0eu;
    fixture_arm(state);
    state->step_count[0] = 0u;
    failed |= !seek_command(state, (const lib_u8[]){0x0fu, 0u, 9u}, 3u);
    fixture_command(state, (const lib_u8[]){0x08u}, 1u);
    failed |= !fixture_result(state, result, 2u) || result[0] != 0x68u ||
        state->step_count[0] != 0u;
    if (failed) lib_c_fprintf(lib_c_stderr, "FDC recalibration pulse count failed\n");
    return failed;
}

int main(void)
{
    fixture state = {.present = LIB_TRUE, .ready_mask = 0x0fu};
    const x86_fdc_timing timing = {8192u, 8000u, 1u, 248u, 120u};
    if (fixture_create(&state, &timing) != LIB_STATUS_OK) return 1;
    lib_i32 failed = ready_edges(&state);
    failed |= seek_ownership(&state);
    failed |= specify_reset(&state);
    failed |= seek_cadence(&state);
    failed |= recalibrate_pulses(&state);
    x86_fdc_destroy(state.chip);
    return failed != 0;
}
