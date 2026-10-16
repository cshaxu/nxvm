/* Original READY and seek-cause regressions, now owned by the chip. */
#include "fixture.h"
#include "x86-nxvm/chips/fdc8272/fdc.h"
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
    /* Reset retains the complete last Specify command, including ND. */
    failed |= fdc->data.srt != 0x0du || fdc->data.hut != 0x0fu ||
        fdc->data.hlt != 0x01u || !fdc->data.flagNDMA;
    x86_fdc_set_reset(fdc, LIB_FALSE);
    failed |= fdc->data.srt != 0x0du || fdc->data.hut != 0x0fu ||
        fdc->data.hlt != 0x01u || !fdc->data.flagNDMA;
    x86_fdc_set_reset(fdc, LIB_TRUE);
    state->now = fdc->data.reset_due_tick;
    x86_fdc_advance_at(fdc, state->now);
    failed |= fdc->data.srt != 0x0du || fdc->data.hut != 0x0fu ||
        fdc->data.hlt != 0x01u || !fdc->data.flagNDMA;
    if (failed) lib_c_fprintf(lib_c_stderr, "FDC SPECIFY reset retention failed\n");
    return failed;
}

static lib_i32 seek_cadence(void)
{
    fixture local = {.present = LIB_TRUE, .ready_mask = 0x0fu};
    fixture *state = &local;
    const x86_fdc_timing timing = {.reset_ticks = 0u, .step_numerator = 8000u, .step_denominator = 1u, .fm_byte_ticks = 248u, .mfm_byte_ticks = 120u};
    lib_u8 result[2];
    lib_i32 failed = 0;
    if (fixture_create(state, &timing) != LIB_STATUS_OK) return 1;
    x86_fdc *fdc = state->chip;

    /* A fresh local axis avoids the old mixed fixture's backward time jump.
     * Only reset latency is zero here; each step retains the 24000-tick oracle. */
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
    x86_fdc_destroy(fdc);
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

static lib_i32 ndma_byte_interrupts(void)
{
    static const lib_u8 commands[] = {0x46u, 0x45u, 0x51u, 0x4du};
    lib_i32 failed = 0;

    for (lib_size row = 0u; row < sizeof(commands); ++row) {
        fixture state = {.present = LIB_TRUE, .ready_mask = 0x0fu};
        const x86_fdc_timing timing = {.reset_ticks = 0u, .step_numerator = 1u, .step_denominator = 1u, .fm_byte_ticks = 31u, .mfm_byte_ticks = 15u};
        lib_u8 byte = 0xa5u;
        if (fixture_create(&state, &timing) != LIB_STATUS_OK) return 1;
        fixture_arm(&state);
        fixture_command(&state, (const lib_u8[]){3u, 0xdfu, 3u}, 3u);
        if (commands[row] == 0x4du)
            fixture_command(&state, (const lib_u8[]){0x4du, 0u, 2u, 1u, 0x1bu, 0u}, 6u);
        else
            fixture_command(&state, (const lib_u8[]){commands[row], 0u, 0u, 0u,
                1u, 2u, 1u, 0x1bu, 1u}, 9u);
        failed |= !state.irq || state.drq ||
            (x86_fdc_read_status(state.chip) & 0x80u) == 0u;
        if (commands[row] == 0x46u) x86_fdc_read_data(state.chip, &byte);
        else x86_fdc_write_data(state.chip, 0u);
        failed |= state.irq || state.drq ||
            x86_fdc_read_status(state.chip) !=
                (commands[row] == 0x46u ? 0x70u : 0x30u);
        /* An early data access cannot acknowledge or consume the next byte. */
        const lib_u16 offset = state.chip->data.byte_offset;
        if (commands[row] == 0x46u) x86_fdc_read_data(state.chip, &byte);
        else x86_fdc_write_data(state.chip, 0u);
        failed |= state.chip->data.byte_offset != offset || state.irq;
        state.now = state.chip->data.next_ndma_byte_tick - 1u;
        x86_fdc_advance_at(state.chip, state.now);
        failed |= state.irq;
        advance(&state);
        failed |= !state.irq || state.drq ||
            (x86_fdc_read_status(state.chip) & 0x80u) == 0u;
        x86_fdc_reset(state.chip);
        failed |= state.irq || state.drq;
        x86_fdc_destroy(state.chip);
        if (failed) {
            lib_c_fprintf(lib_c_stderr, "FDC NDMA byte interrupt command=%02x\n", commands[row]);
            return 1;
        }
    }
    return failed;
}

static lib_i32 deadline_boundaries(void)
{
    lib_i32 failed = 0;
    for (lib_u8 row = 0u; row < 4u; ++row) {
        fixture state = {.present = LIB_TRUE, .ready_mask = 0x0fu};
        const x86_fdc_timing timing = {.reset_ticks = 5u, .step_numerator = 5u, .step_denominator = 1u, .fm_byte_ticks = 5u, .mfm_byte_ticks = 5u};
        lib_u64 due = 0u;
        lib_u8 byte = 0u;
        if (fixture_create(&state, &timing) != LIB_STATUS_OK) return 1;
        fixture_arm(&state);
        x86_fdc_advance_at(state.chip, 10u);
        x86_fdc_advance_at(state.chip, 9u);
        failed |= state.chip->data.elapsed_ticks != 10u;
        if (row == 0u) {
            x86_fdc_set_reset(state.chip, LIB_FALSE);
            x86_fdc_advance_at(state.chip, LIB_UINT64_MAX - 2u);
            x86_fdc_set_reset(state.chip, LIB_TRUE);
        } else {
            /* Drain reset completion before isolating seek or byte cadence. */
            for (lib_u8 drive = 0u; drive < 4u; ++drive) {
                x86_fdc_write_data(state.chip, 8u);
                x86_fdc_advance_at(state.chip, 10u);
                x86_fdc_read_data(state.chip, &byte);
                x86_fdc_read_data(state.chip, &byte);
            }
            x86_fdc_write_data(state.chip, 3u);
            x86_fdc_write_data(state.chip, 0xf0u);
            x86_fdc_write_data(state.chip, row == 3u ? 1u : 0u);
            x86_fdc_advance_at(state.chip, 10u);
            x86_fdc_advance_at(state.chip, LIB_UINT64_MAX - 2u);
            const lib_u8 seek[] = {0x0fu, 0u, 2u};
            const lib_u8 read[] = {0x46u, 0u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 1u};
            for (lib_size i = 0u; i < (row == 1u ? sizeof(seek) : sizeof(read)); ++i)
                x86_fdc_write_data(state.chip, row == 1u ? seek[i] : read[i]);
            x86_fdc_advance_at(state.chip, LIB_UINT64_MAX - 2u);
            if (row == 2u) x86_fdc_dma_read(state.chip, &byte);
            if (row == 3u) x86_fdc_read_data(state.chip, &byte);
        }
        failed |= x86_fdc_next_due_tick(state.chip, &due) != LIB_STATUS_OK ||
            due != LIB_UINT64_MAX;
        x86_fdc_advance_at(state.chip, LIB_UINT64_MAX - 1u);
        failed |= row == 0u ? state.irq :
            (row == 1u ? state.step_count[0] != 0u : (row == 2u ? state.drq : state.irq));
        x86_fdc_advance_at(state.chip, LIB_UINT64_MAX);
        failed |= row == 1u ? state.step_count[0] != 2u :
            (row == 2u ? !state.drq : !state.irq);
        failed |= x86_fdc_next_due_tick(state.chip, &due) != LIB_STATUS_INVALID_STATE;
        x86_fdc_destroy(state.chip);
    }
    if (failed) lib_c_fprintf(lib_c_stderr, "FDC deadline boundary failed\n");
    return failed;
}

static lib_i32 head_timers(void)
{
    lib_i32 failed = 0;
    for (lib_u8 ndma = 0u; ndma < 2u; ++ndma) {
        fixture state = {.present = LIB_TRUE, .ready_mask = 0x0fu};
        const x86_fdc_timing timing = {.reset_ticks = 0u, .step_numerator = 3u, .step_denominator = 2u, .fm_byte_ticks = 0u, .mfm_byte_ticks = 0u};
        const lib_u8 command[] = {0x46u, 0u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 1u};
        lib_u8 result[7], byte = 0u;
        lib_u64 due, origin;
        if (fixture_create(&state, &timing) != LIB_STATUS_OK) return 1;
        fixture_arm(&state);
        fixture_command(&state, (const lib_u8[]){3u, 0xd2u, (lib_u8)(6u | ndma)}, 3u);
        origin = ++state.now;
        for (lib_size i = 0u; i < sizeof(command); ++i)
            x86_fdc_write_data(state.chip, command[i]);
        x86_fdc_advance_at(state.chip, state.now);
        failed |= state.chip->data.phase != x86_fdc_PHASE_HEAD_LOAD ||
            state.drq || state.irq || state.read_count != 0u ||
            x86_fdc_read_status(state.chip) != 0x10u ||
            x86_fdc_next_due_tick(state.chip, &due) != LIB_STATUS_OK || due != origin + 9u;
        state.now = due - 1u;
        x86_fdc_advance_at(state.chip, state.now);
        failed |= state.drq || state.irq || state.read_count != 0u;
        advance(&state);
        failed |= state.chip->data.phase != x86_fdc_PHASE_EXECUTION_READ ||
            (ndma ? !state.irq || state.drq : !state.drq || state.irq);
        if (ndma) x86_fdc_read_data(state.chip, &byte);
        else x86_fdc_dma_read(state.chip, &byte);
        x86_fdc_terminal_count(state.chip);
        due = state.now + 48u;
        failed |= state.chip->data.head_unload_due_tick[0] != due ||
            !fixture_result(&state, result, 7u);
        /* Another unit cannot borrow the first unit's loaded head. */
        for (lib_size i = 0u; i < sizeof(command); ++i)
            x86_fdc_write_data(state.chip, i == 1u ? 1u : command[i]);
        advance(&state);
        failed |= state.chip->data.phase != x86_fdc_PHASE_HEAD_LOAD ||
            state.chip->data.selected_drive != 1u;
        state.now = state.chip->data.head_load_due_tick;
        x86_fdc_advance_at(state.chip, state.now);
        x86_fdc_terminal_count(state.chip);
        failed |= !fixture_result(&state, result, 7u);
        /* The selected unit stays loaded throughout HUT: no second HLT. */
        fixture_command(&state, command, sizeof(command));
        failed |= state.chip->data.phase != x86_fdc_PHASE_EXECUTION_READ;
        x86_fdc_terminal_count(state.chip);
        due = state.chip->data.head_unload_due_tick[0];
        failed |= !fixture_result(&state, result, 7u);
        state.now = due - 1u;
        x86_fdc_advance_at(state.chip, state.now);
        failed |= (state.chip->data.head_loaded_mask & 1u) == 0u;
        advance(&state);
        failed |= (state.chip->data.head_loaded_mask & 1u) != 0u;
        for (lib_size i = 0u; i < sizeof(command); ++i)
            x86_fdc_write_data(state.chip, command[i]);
        advance(&state);
        failed |= state.chip->data.phase != x86_fdc_PHASE_HEAD_LOAD;
        x86_fdc_reset(state.chip);
        failed |= state.chip->data.head_loaded_mask != 0u ||
            state.chip->data.head_unload_mask != 0u ||
            state.chip->data.hlt != 3u || state.chip->data.hut != 2u;
        fixture_arm(&state);
        state.now = LIB_UINT64_MAX - 2u;
        x86_fdc_advance_at(state.chip, state.now);
        for (lib_size i = 0u; i < sizeof(command); ++i)
            x86_fdc_write_data(state.chip, command[i]);
        x86_fdc_advance_at(state.chip, state.now);
        failed |= x86_fdc_next_due_tick(state.chip, &due) != LIB_STATUS_OK ||
            due != LIB_UINT64_MAX;
        x86_fdc_advance_at(state.chip, LIB_UINT64_MAX - 1u);
        failed |= state.drq || state.irq;
        x86_fdc_advance_at(state.chip, LIB_UINT64_MAX);
        failed |= state.chip->data.phase != x86_fdc_PHASE_EXECUTION_READ;
        x86_fdc_terminal_count(state.chip);
        x86_fdc_advance_at(state.chip, LIB_UINT64_MAX);
        failed |= state.chip->data.head_loaded_mask != 0u ||
            state.chip->data.head_unload_mask != 0u;
        x86_fdc_destroy(state.chip);
    }
    {
        fixture state = {.present = LIB_TRUE, .ready_mask = 0x0fu};
        const x86_fdc_timing timing = {.reset_ticks = 0u, .step_numerator = 3u, .step_denominator = 2u, .fm_byte_ticks = 0u, .mfm_byte_ticks = 0u};
        const lib_u8 command[] = {0x4au, 0u};
        lib_u8 result[7];
        lib_u64 due;
        if (fixture_create(&state, &timing) != LIB_STATUS_OK) return 1;
        fixture_arm(&state);
        fixture_command(&state, (const lib_u8[]){3u, 0xd2u, 6u}, 3u);
        for (lib_size i = 0u; i < sizeof(command); ++i)
            x86_fdc_write_data(state.chip, command[i]);
        advance(&state);
        failed |= state.chip->data.phase != x86_fdc_PHASE_HEAD_LOAD ||
            state.irq || state.drq ||
            x86_fdc_next_due_tick(state.chip, &due) != LIB_STATUS_OK ||
            due != state.now + 9u;
        state.now = due - 1u;
        x86_fdc_advance_at(state.chip, state.now);
        failed |= state.irq || state.drq;
        advance(&state);
        due = state.now + 48u;
        failed |= !state.irq || state.drq ||
            state.chip->data.head_unload_due_tick[0] != due ||
            !fixture_result(&state, result, 7u) ||
            result[0] != 0u || result[1] != 0u || result[2] != 0u ||
            result[5] != 1u || result[6] != 2u;
        /* Read ID reuses a loaded head and restarts HUT at its own completion. */
        fixture_command(&state, command, sizeof(command));
        failed |= state.chip->data.phase != x86_fdc_PHASE_RESULT ||
            state.chip->data.head_unload_due_tick[0] != state.now + 48u ||
            !fixture_result(&state, result, 7u);
        x86_fdc_destroy(state.chip);
    }
    for (lib_u8 ndma = 0u; ndma < 2u; ++ndma) {
        fixture state = {.present = LIB_TRUE, .ready_mask = 0x0fu};
        const x86_fdc_timing timing = {.reset_ticks = 0u, .step_numerator = 3u, .step_denominator = 2u, .fm_byte_ticks = 0u, .mfm_byte_ticks = 0u};
        const lib_u8 command[] = {0x4du, 0u, 2u, 1u, 0x1bu, 0xa5u};
        const lib_u8 id[] = {0u, 0u, 1u, 2u};
        lib_u8 result[7];
        lib_u64 due;
        if (fixture_create(&state, &timing) != LIB_STATUS_OK) return 1;
        fixture_arm(&state);
        fixture_command(&state, (const lib_u8[]){3u, 0xd2u, (lib_u8)(6u | ndma)}, 3u);
        for (lib_size i = 0u; i < sizeof(command); ++i)
            x86_fdc_write_data(state.chip, command[i]);
        advance(&state);
        failed |= state.chip->data.phase != x86_fdc_PHASE_HEAD_LOAD ||
            state.irq || state.drq || state.bytes[0] != 0u ||
            x86_fdc_next_due_tick(state.chip, &due) != LIB_STATUS_OK ||
            due != state.now + 9u;
        state.now = due - 1u;
        x86_fdc_advance_at(state.chip, state.now);
        failed |= state.irq || state.drq;
        advance(&state);
        failed |= state.chip->data.phase != x86_fdc_PHASE_EXECUTION_FORMAT ||
            (ndma ? !state.irq || state.drq : !state.drq || state.irq);
        for (lib_size i = 0u; i < sizeof(id); ++i) {
            if (ndma) x86_fdc_write_data(state.chip, id[i]);
            else x86_fdc_dma_write(state.chip, id[i]);
        }
        failed |= state.chip->data.head_unload_due_tick[0] != state.now + 48u ||
            state.bytes[0] != 0xa5u || state.bytes[511] != 0xa5u ||
            !fixture_result(&state, result, 7u) ||
            result[0] != 0u || result[1] != 0u || result[2] != 0u || result[5] != 2u;
        x86_fdc_destroy(state.chip);
    }
    {
        fixture state = {.present = LIB_TRUE, .ready_mask = 0x0fu};
        const x86_fdc_timing timing = {.reset_ticks = 0u, .step_numerator = LIB_UINT64_MAX / 16u, .step_denominator = LIB_UINT64_MAX, .fm_byte_ticks = 0u, .mfm_byte_ticks = 0u};
        const lib_u8 command[] = {0x46u, 0u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 1u};
        lib_u64 due;
        if (fixture_create(&state, &timing) != LIB_STATUS_OK) return 1;
        fixture_arm(&state);
        fixture_command(&state, (const lib_u8[]){3u, 0xdfu, 0xfeu}, 3u);
        for (lib_size i = 0u; i < sizeof(command); ++i)
            x86_fdc_write_data(state.chip, command[i]);
        advance(&state);
        failed |= x86_fdc_next_due_tick(state.chip, &due) != LIB_STATUS_OK ||
            due != state.now + 16u;
        state.now = due;
        x86_fdc_advance_at(state.chip, state.now);
        x86_fdc_terminal_count(state.chip);
        failed |= state.chip->data.head_unload_due_tick[0] != state.now + 15u;
        x86_fdc_destroy(state.chip);
    }
    if (failed) lib_c_fprintf(lib_c_stderr, "FDC head load/unload timing failed\n");
    return failed;
}

static lib_i32 service_deadlines(void)
{
    static const lib_u8 commands[] = {6u, 12u, 5u, 9u, 17u, 25u, 29u, 2u, 13u};
    const x86_fdc_timing timing = {
        .step_numerator = 1u, .step_denominator = 1u,
        .fm_byte_ticks = 32u, .mfm_byte_ticks = 16u,
        .fm_read_service_ticks = 27u, .mfm_read_service_ticks = 13u,
        .fm_write_service_ticks = 31u, .mfm_write_service_ticks = 15u
    };
    lib_i32 failed = 0;

    for (lib_size command = 0u; command < sizeof(commands); ++command) {
        const lib_bool writing = commands[command] == 5u ||
            commands[command] == 9u || commands[command] == 13u;
        const lib_bool reading = commands[command] == 6u ||
            commands[command] == 12u || commands[command] == 2u;
        for (lib_u8 mode = 0u; mode < 4u; ++mode) {
            const lib_bool ndma = (mode & 1u) != 0u;
            const lib_bool mfm = (mode & 2u) != 0u;
            const lib_u64 service = writing ? (mfm ? 15u : 31u) :
                (mfm ? 13u : 27u);
            for (lib_u8 action = 0u; action < 4u; ++action) {
                fixture state = {.present = LIB_TRUE, .ready_mask = 15u};
                lib_u8 result[7], byte = 0xa5u;
                lib_u64 due;
                const lib_u8 opcode = commands[command] | (mfm ? 0x40u : 0u);
                if (fixture_create(&state, &timing) != LIB_STATUS_OK) return 1;
                state.deleted[0] = commands[command] == 12u;
                lib_memory_set(state.bytes, 0x5au, sizeof(state.bytes));
                fixture_arm(&state);
                fixture_command(&state, (const lib_u8[]){3u, 0xf0u, ndma}, 3u);
                if (commands[command] == 13u)
                    fixture_command(&state, (const lib_u8[]){opcode, 0u, 2u, 1u, 0x1bu, 0u}, 6u);
                else
                    fixture_command(&state,
                        (const lib_u8[]){opcode, 0u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 1u}, 9u);
                due = state.now + service;
                failed |= !state.chip->data.service_pending ||
                    x86_fdc_next_due_tick(state.chip, &state.now) != LIB_STATUS_OK ||
                    state.now != due;
                state.now = due - 1u;
                x86_fdc_advance_at(state.chip, state.now);
                /* Polling MSR is not data service and cannot renew the limit. */
                for (lib_u8 poll = 0u; poll < 4u; ++poll)
                    (void)x86_fdc_read_status(state.chip);
                failed |= state.chip->data.service_due_tick != due ||
                    !state.chip->data.service_pending;
                if (action == 1u) {
                    if (reading) {
                        if (ndma) x86_fdc_read_data(state.chip, &byte);
                        else x86_fdc_dma_read(state.chip, &byte);
                        failed |= byte != 0x5au;
                    } else {
                        if (ndma) x86_fdc_write_data(state.chip, byte);
                        else x86_fdc_dma_write(state.chip, byte);
                    }
                    failed |= state.chip->data.service_pending;
                    failed |= x86_fdc_next_due_tick(state.chip, &state.now) != LIB_STATUS_OK;
                    x86_fdc_advance_at(state.chip, state.now);
                    due = state.now + service;
                    failed |= !state.chip->data.service_pending ||
                        state.chip->data.service_due_tick != due;
                } else if (action == 2u) {
                    x86_fdc_terminal_count(state.chip);
                    failed |= state.chip->data.service_pending;
                } else if (action == 3u) {
                    x86_fdc_reset(state.chip);
                    failed |= state.chip->data.service_pending;
                }
                state.now = due;
                x86_fdc_advance_at(state.chip, state.now);
                if (action != 3u) {
                    failed |= !fixture_result(&state, result, 7u) ||
                        result[0] != (action == 2u ? 0u : 0x40u) ||
                        result[1] != (action == 2u ? 0u : 0x10u) ||
                        result[3] != 0u || result[4] != 0u || result[5] != 1u ||
                        state.terminals != 1u || state.drq;
                } else failed |= state.terminals != 0u || state.irq || state.drq;
                failed |= state.read_count != (action == 1u && commands[command] != 13u ? 1u : 0u);
                failed |= state.bytes[0] !=
                    (action == 1u && writing && commands[command] != 13u ? 0xa5u : 0x5au);
                for (lib_size index = 1u; index < sizeof(state.bytes); ++index)
                    failed |= state.bytes[index] != 0x5au;
                x86_fdc_destroy(state.chip);
            }
        }
    }
    if (failed) lib_c_fprintf(lib_c_stderr, "FDC byte-service deadlines failed\n");
    return failed;
}

int main(void)
{
    fixture state = {.present = LIB_TRUE, .ready_mask = 0x0fu};
    const x86_fdc_timing timing = {.reset_ticks = 8192u, .step_numerator = 8000u, .step_denominator = 1u, .fm_byte_ticks = 248u, .mfm_byte_ticks = 120u};
    if (fixture_create(&state, &timing) != LIB_STATUS_OK) return 1;
    lib_i32 failed = ready_edges(&state);
    failed |= seek_ownership(&state);
    failed |= specify_reset(&state);
    failed |= seek_cadence();
    failed |= recalibrate_pulses(&state);
    failed |= ndma_byte_interrupts();
    failed |= deadline_boundaries();
    failed |= head_timers();
    failed |= service_deadlines();
    x86_fdc_destroy(state.chip);
    return failed != 0;
}
