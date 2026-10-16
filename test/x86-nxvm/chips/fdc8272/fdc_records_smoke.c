/* Command tables migrated from the NXVM FDC corpus without changing oracles. */
#include "fixture.h"
#include "x86-nxvm/chips/fdc8272/fdc.h"
#include "lib/types/file.h"

static lib_u8 read_data(fixture *state)
{
    lib_u8 byte = 0u;
    x86_fdc_read_data(state->chip, &byte);
    return byte;
}

static lib_i32 scan_status(fixture *state)
{
    x86_fdc *fdc = state->chip;
    /* Intel 8272A Table 10: rows are Equal, Low/Equal, High/Equal;
       columns are disk < host, disk == host, disk > host. These literal
       wire results deliberately do not reuse the implementation's masks. */
    static const lib_u8 commands[] = {0x51u, 0x59u, 0x5du};
    static const lib_u8 operands[][2] = {
        {0x40u, 0x50u}, {0x50u, 0x50u}, {0x60u, 0x50u},
        {0x00u, 0xffu}, {0xfeu, 0xffu}, {0xffu, 0xfeu}, {0xffu, 0xffu}
    };
    static const lib_u8 expected[][7] = {
        {0x04u, 0x08u, 0x04u, 0x04u, 0x04u, 0x04u, 0x08u},
        {0x00u, 0x08u, 0x04u, 0x00u, 0x00u, 0x04u, 0x08u},
        {0x04u, 0x08u, 0x00u, 0x04u, 0x04u, 0x00u, 0x08u}
    };
    lib_i32 failed = 0;

    state->present = LIB_TRUE;
    state->deleted[0] = LIB_FALSE;
    for (lib_size mode = 0u; mode < 3u; ++mode) {
        for (lib_size relation = 0u; relation < sizeof(operands) / sizeof(operands[0]);
                ++relation) {
            lib_u8 result[7];
            lib_u8 command[] = {commands[mode], 0u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 1u};
            fixture_arm(state);
            fixture_command(state,
                (const lib_u8[]){0x03u, 0xdfu, 0x03u}, 3u);
            lib_memory_set(state->bytes, operands[relation][0],
                sizeof(state->bytes));
            fixture_command(state, command, sizeof(command));
            for (lib_size byte = 0u; byte < 512u; ++byte)
                x86_fdc_write_data(fdc, operands[relation][1]);
            if (!fixture_result(state, result, sizeof(result)) ||
                result[0] != 0x00u || result[2] != expected[mode][relation]) {
                lib_c_fprintf(lib_c_stderr, "FDC scan wire result mode=%u relation=%u\n",
                    (lib_u32)mode, (lib_u32)relation);
                failed = 1;
            }
        }
    }
    return failed;
}

static lib_i32 scan_sequence(fixture *state)
{
    x86_fdc *fdc = state->chip;
    static const struct {
        lib_u8 step, eot, deleted, skip;
        lib_u8 bytes[3];
        lib_u16 compared;
        lib_u8 st1, st2, next_sector;
    } cases[] = {
        {1u, 3u, 0u, 0u, {0u, 1u, 1u}, 1024u, 0u, 0x08u, 3u},
        {2u, 3u, 0u, 0u, {0u, 1u, 1u}, 1024u, 0u, 0x08u, 5u},
        {2u, 2u, 0u, 0u, {0u, 1u, 1u}, 512u, 0x80u, 0x04u, 3u},
        {2u, 3u, 0u, 0u, {0u, 0u, 0u}, 1024u, 0u, 0x04u, 5u},
        {1u, 3u, 1u, 0u, {0u, 1u, 1u}, 512u, 0u, 0x44u, 2u},
        {1u, 3u, 1u, 1u, {0u, 1u, 0u}, 512u, 0u, 0x48u, 3u},
        {2u, 3u, 5u, 1u, {0u, 1u, 0u}, 0u, 0u, 0x44u, 5u},
        {2u, 2u, 1u, 1u, {0u, 1u, 0u}, 0u, 0x80u, 0x44u, 3u},
        {1u, 3u, 2u, 0u, {0u, 0u, 1u}, 1024u, 0u, 0x44u, 3u}
    };
    lib_i32 failed = 0;

    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        lib_u8 result[7];
        lib_u16 compared = 0u;
        const lib_u32 reads = state->read_count;
        for (lib_u8 sector = 0u; sector < 3u; ++sector) {
            lib_memory_set(state->bytes + sector * 512u, cases[index].bytes[sector], 512u);
            state->deleted[sector] = (cases[index].deleted & (1u << sector)) != 0u ?
                LIB_TRUE : LIB_FALSE;
        }
        fixture_arm(state);
        fixture_command(state, (const lib_u8[]){0x03u, 0xdfu, 3u}, 3u);
        fixture_command(state, (const lib_u8[]){
            0x51u | (cases[index].skip ? 0x20u : 0u), 0u, 0u, 0u, 1u, 2u,
            cases[index].eot, 0x1bu, cases[index].step}, 9u);
        while (fdc->data.phase == x86_fdc_PHASE_EXECUTION_SCAN && compared < 1536u) {
            x86_fdc_write_data(fdc, 1u);
            ++compared;
        }
        if (!fixture_result(state, result, 7u) ||
            compared != cases[index].compared || state->read_count - reads != compared ||
            result[0] != (cases[index].st1 ? 0x40u : 0u) ||
            result[1] != cases[index].st1 || result[2] != cases[index].st2 ||
            result[5] != cases[index].next_sector) {
            lib_c_fprintf(lib_c_stderr, "FDC SCAN sequence case=%zu bytes=%u status=%02x/%02x/%02x R=%u\n",
                index, compared, result[0], result[1], result[2], result[5]);
            failed = 1;
        }
    }
    state->deleted[0] = LIB_FALSE;
    x86_fdc_reset(fdc);
    return failed;
}

static lib_i32 read_sequence(fixture *state)
{
    x86_fdc *fdc = state->chip;
    static const struct {
        lib_u8 mismatched, skip, delivered, st1, st2;
    } cases[] = {
        {0u, 0u, 7u, 0u, 0u},
        {1u, 0u, 1u, 0u, 0x40u},
        {2u, 0u, 3u, 0u, 0x40u},
        {1u, 1u, 6u, 0u, 0u},
        {2u, 1u, 5u, 0u, 0u},
        {7u, 1u, 0u, 0x80u, 0u}
    };
    lib_i32 failed = 0;

    for (lib_u8 deleted = 0u; deleted < 2u; ++deleted) {
        for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
            lib_u8 result[7];
            const lib_u32 reads = state->read_count;
            lib_u32 expected = 0u;
            for (lib_u8 sector = 0u; sector < 3u; ++sector) {
                lib_memory_set(state->bytes + sector * 512u, sector + 1u, 512u);
                state->deleted[sector] = (deleted != 0u) !=
                    ((cases[index].mismatched & (1u << sector)) != 0u) ?
                    LIB_TRUE : LIB_FALSE;
            }
            fixture_arm(state);
            fixture_command(state, (const lib_u8[]){0x03u, 0xdfu, 3u}, 3u);
            fixture_command(state, (const lib_u8[]){
                (deleted ? 0x4cu : 0x46u) | (cases[index].skip ? 0x20u : 0u),
                0u, 0u, 0u, 1u, 2u, 3u, 0x1bu, 0xffu}, 9u);
            for (lib_u8 sector = 0u; sector < 3u; ++sector) {
                if ((cases[index].delivered & (1u << sector)) == 0u) continue;
                for (lib_u16 byte = 0u; byte < 512u; ++byte) {
                    failed |= fdc->data.phase != x86_fdc_PHASE_EXECUTION_READ ||
                        read_data(state) != sector + 1u;
                    ++expected;
                }
            }
            if (!fixture_result(state, result, 7u) ||
                state->read_count - reads != expected ||
                result[0] != (cases[index].st1 ? 0x40u : 0u) ||
                result[1] != cases[index].st1 || result[2] != cases[index].st2) {
                lib_c_fprintf(lib_c_stderr, "FDC READ sequence deleted=%u case=%zu\n", deleted, index);
                failed = 1;
            }
        }
    }
    state->deleted[0] = LIB_FALSE;
    x86_fdc_reset(fdc);
    return failed;
}

static lib_i32 read_track_forms(void)
{
    lib_i32 failed = 0;
    for (lib_u8 row = 0u; row < 64u; ++row) {
        const lib_u8 unit = row & 3u;
        const lib_bool ndma = (row & 4u) != 0u;
        const lib_u8 command_code = 0x02u | ((row & 8u) != 0u ? 0x40u : 0u) |
            ((row & 16u) != 0u ? 0x20u : 0u);
        const lib_u8 eot = (row & 32u) != 0u ? 2u : 3u;
        fixture state = {.present = LIB_TRUE, .ready_mask = 0x0fu};
        const x86_fdc_timing timing = {.step_denominator = 1u};
        lib_u8 result[7];
        if (fixture_create(&state, &timing) != LIB_STATUS_OK) return 1;
        fixture_arm(&state);
        fixture_command(&state, (const lib_u8[]){3u, 0xdfu, ndma ? 3u : 2u}, 3u);
        for (lib_u8 sector = 0u; sector < 3u; ++sector)
            lib_memory_set(state.bytes + sector * 512u, sector + 1u, 512u);
        fixture_command(&state, (const lib_u8[]){command_code, unit, 0u, 0u,
            1u, 2u, eot, 0x1bu, 0xffu}, 9u);
        for (lib_u16 i = 0u; i < (lib_u16)eot * 512u; ++i) {
            lib_u8 byte = 0u;
            failed |= state.chip->data.selected_drive != unit ||
                state.chip->data.phase != x86_fdc_PHASE_EXECUTION_READ ||
                (ndma && state.drq);
            if (ndma) x86_fdc_read_data(state.chip, &byte);
            else x86_fdc_dma_read(state.chip, &byte);
            failed |= byte != i / 512u + 1u;
        }
        failed |= !fixture_result(&state, result, 7u) || result[0] != unit ||
            result[1] != 0u || result[2] != 0u || result[5] != eot + 1u ||
            state.read_count != (lib_u32)eot * 512u || state.drq;
        if (failed) lib_c_fprintf(lib_c_stderr,
            "FDC Read Track row=%u code=%02x ND=%u result=%02x/%02x/%02x R=%u bytes=%u\n",
            row, command_code, (lib_u32)ndma, result[0], result[1], result[2],
            result[5], state.read_count);
        x86_fdc_destroy(state.chip);
        if (failed) return failed;
    }
    return failed;
}

static lib_i32 id_and_format_results(void)
{
    fixture state = {.present = LIB_TRUE, .ready_mask = 0x0fu};
    const x86_fdc_timing timing = {.reset_ticks = 0u, .step_numerator = 1u, .step_denominator = 1u, .fm_byte_ticks = 0u, .mfm_byte_ticks = 0u};
    const lib_u8 ids[] = {0u, 1u, 3u, 2u, 0u, 1u, 1u, 2u,
        0u, 1u, 2u, 2u};
    lib_u8 result[7];
    lib_i32 failed = 0;
    if (fixture_create(&state, &timing) != LIB_STATUS_OK) return 1;
    fixture_arm(&state);
    fixture_command(&state, (const lib_u8[]){3u, 0xdfu, 3u}, 3u);
    fixture_command(&state, (const lib_u8[]){0x4du, 4u, 2u, 3u, 0x1bu, 0xa5u}, 6u);
    for (lib_size index = 0u; index < sizeof(ids); ++index)
        x86_fdc_write_data(state.chip, ids[index]);
    failed |= !fixture_result(&state, result, 7u) || result[0] != 4u ||
        result[1] != 0u || result[2] != 0u || result[3] != 0u ||
        result[4] != 1u || result[5] != 3u || result[6] != 2u;
    for (lib_size index = 0u; index < sizeof(state.head_bytes); ++index)
        failed |= state.head_bytes[index] != 0xa5u || state.bytes[index] != 0u;
    /* Read ID must not reuse Format's incremented final R. */
    fixture_command(&state, (const lib_u8[]){0x4au, 4u}, 2u);
    failed |= !fixture_result(&state, result, 7u) || result[0] != 4u ||
        result[1] != 0u || result[2] != 0u || result[3] != 0u ||
        result[4] != 1u || result[5] != 1u || result[6] != 2u;
    if (failed) lib_c_fprintf(lib_c_stderr, "FDC Format/Read ID result failed\n");
    x86_fdc_destroy(state.chip);
    return failed;
}

static lib_i32 multi_track_payload(void)
{
    static const lib_u8 commands[] = {0x46u, 0x4cu, 0x45u, 0x49u, 0x51u, 0x59u, 0x5du};
    static const lib_u8 heads[] = {0u, 0u, 1u, 1u, 1u};
    static const lib_u8 sectors[] = {2u, 3u, 1u, 2u, 3u};
    lib_i32 failed = 0;

    for (lib_size command = 0u; command < sizeof(commands); ++command) {
        const lib_bool writing = command == 2u || command == 3u;
        const lib_bool scanning = command >= 4u;
        const lib_bool deleted = command == 1u || command == 3u;
        for (lib_u8 variant = 0u; variant < (writing ? 4u : 8u); ++variant) {
            const lib_bool ndma = (variant & 1u) != 0u;
            const lib_bool skip = !writing && (variant & 2u) != 0u;
            const lib_u8 first_head = writing ? (variant >> 1u) & 1u :
                (variant >> 2u) & 1u;
            const lib_u8 compare = command == 5u ? 0u : command == 6u ? 0xffu : 0x80u;
            fixture state = {.present = LIB_TRUE, .ready_mask = 0x0fu};
            const x86_fdc_timing timing = {.step_denominator = 1u};
            lib_u8 result[7];
            lib_u32 transferred = 0u;
            if (fixture_create(&state, &timing) != LIB_STATUS_OK) return 1;
            fixture_arm(&state);
            fixture_command(&state, (const lib_u8[]){3u, 0xdfu, ndma ? 3u : 2u}, 3u);
            for (lib_u8 sector = 0u; sector < 3u; ++sector) {
                lib_memory_set(state.bytes + sector * 512u, 0x10u + sector, 512u);
                lib_memory_set(state.head_bytes + sector * 512u, 0x20u + sector, 512u);
                state.deleted[sector] = deleted;
                state.head_deleted[sector] = deleted;
            }
            if (scanning) lib_memory_set(state.head_bytes + 1024u, compare, 512u);
            if (skip) {
                state.deleted[2] = !deleted;
                state.head_deleted[0] = !deleted;
            }
            fixture_command(&state, (const lib_u8[]){commands[command] | 0x80u |
                (skip ? 0x20u : 0u), first_head << 2u, 0u, first_head,
                2u, 2u, 3u, 0x1bu, 1u}, 9u);
            for (lib_size record = first_head != 0u ? 3u : 0u;
                    record < sizeof(heads); ++record) {
                if (skip && (record == 1u || record == 2u)) continue;
                const lib_u8 expected = scanning ? compare :
                    (lib_u8)((heads[record] == 0u ? 0x10u : 0x20u) +
                        (writing ? 0x30u : 0u) + sectors[record] - 1u);
                for (lib_u16 offset = 0u; offset < 512u; ++offset) {
                    failed |= state.chip->data.phase != (writing ? x86_fdc_PHASE_EXECUTION_WRITE :
                            scanning ? x86_fdc_PHASE_EXECUTION_SCAN : x86_fdc_PHASE_EXECUTION_READ);
                    if (writing || scanning) {
                        if (ndma) x86_fdc_write_data(state.chip, expected);
                        else x86_fdc_dma_write(state.chip, expected);
                    } else {
                        lib_u8 byte = 0u;
                        if (ndma) x86_fdc_read_data(state.chip, &byte);
                        else x86_fdc_dma_read(state.chip, &byte);
                        failed |= byte != expected;
                    }
                    failed |= state.last_record.head != heads[record] ||
                        state.last_record.sector != sectors[record] ||
                        state.last_record.offset != offset || state.last_record.cylinder != 0u;
                    ++transferred;
                    advance(&state);
                }
            }
            failed |= !fixture_result(&state, result, 7u) || result[0] != 4u || result[1] != 0u ||
                result[2] != (scanning ? (skip && first_head == 0u ? 0x48u : 0x08u) : 0u) ||
                result[3] != 0u || result[4] != 1u || result[5] != 4u ||
                transferred != (first_head != 0u ? 1024u : skip ? 1536u : 2560u) ||
                state.read_count != transferred;
            if (writing) {
                for (lib_size record = first_head != 0u ? 3u : 0u;
                        record < sizeof(heads); ++record) {
                    const lib_u8 *bytes = heads[record] == 0u ? state.bytes : state.head_bytes;
                    const lib_bool *marks = heads[record] == 0u ? state.deleted : state.head_deleted;
                    const lib_u8 expected = (heads[record] == 0u ? 0x40u : 0x50u) + sectors[record] - 1u;
                    failed |= marks[sectors[record] - 1u] != deleted;
                    for (lib_u16 offset = 0u; offset < 512u; ++offset)
                        failed |= bytes[(sectors[record] - 1u) * 512u + offset] != expected;
                }
            }
            if (failed) lib_c_fprintf(lib_c_stderr,
                "FDC MT payload command=%02x variant=%u CHR=%u/%u/%u status=%02x/%02x/%02x reads=%u\n",
                commands[command], variant, result[3], result[4], result[5],
                result[0], result[1], result[2], state.read_count);
            x86_fdc_destroy(state.chip);
            if (failed) return failed;
        }
    }
    return failed;
}

static lib_i32 terminal_count_ids(void)
{
    /* Intel Table 8: explicit next-ID rows, separate from the active ST0 head. */
    static const struct { lib_u8 mt, head, sector, c, h, r; } ids[] = {
        {0u, 0u, 1u, 7u, 0u, 2u}, {0u, 0u, 3u, 8u, 0u, 1u},
        {0u, 1u, 1u, 7u, 1u, 2u}, {0u, 1u, 3u, 8u, 1u, 1u},
        {1u, 0u, 1u, 7u, 0u, 2u}, {1u, 0u, 3u, 7u, 1u, 1u},
        {1u, 1u, 1u, 7u, 1u, 2u}, {1u, 1u, 3u, 8u, 0u, 1u}
    };
    static const lib_u8 commands[] = {0x46u, 0x4cu, 0x45u, 0x49u};
    for (lib_size command = 0u; command < sizeof(commands); ++command) {
        const lib_bool writing = command >= 2u;
        const lib_bool deleted = command == 1u || command == 3u;
        for (lib_size row = 0u; row < sizeof(ids) / sizeof(ids[0]); ++row) {
            for (lib_u8 variant = 0u; variant < 8u; ++variant) {
                const lib_bool ndma = (variant & 1u) != 0u;
                const lib_u16 count = (variant & 2u) != 0u ? 31u : 512u;
                fixture state = {.present = LIB_TRUE, .ready_mask = 0x0fu};
                const x86_fdc_timing timing = {.step_denominator = 1u,
                    .mfm_byte_ticks = (variant & 4u) != 0u ? 1u : 0u};
                lib_u8 result[7] = {0};
                lib_bool failed = LIB_FALSE;
                if (fixture_create(&state, &timing) != LIB_STATUS_OK) return 1;
                fixture_arm(&state);
                state.cylinder[1] = 7u;
                lib_memory_set(state.bytes, 0x5au, sizeof(state.bytes));
                lib_memory_set(state.head_bytes, 0x5au, sizeof(state.head_bytes));
                for (lib_size sector = 0u; sector < 3u; ++sector) {
                    state.deleted[sector] = deleted;
                    state.head_deleted[sector] = deleted;
                }
                fixture_command(&state, (const lib_u8[]){3u, 0xdfu, ndma ? 3u : 2u}, 3u);
                fixture_command(&state, (const lib_u8[]){commands[command] |
                    (ids[row].mt != 0u ? 0x80u : 0u),
                    (lib_u8)((ids[row].head << 2u) | 1u), 7u, ids[row].head,
                    ids[row].sector, 2u, 3u, 0x1bu, 0xffu}, 9u);
                for (lib_u16 offset = 0u; offset < count; ++offset) {
                    lib_u8 byte = 0xa5u;
                    if (writing) {
                        if (ndma) x86_fdc_write_data(state.chip, byte);
                        else x86_fdc_dma_write(state.chip, byte);
                    } else {
                        if (ndma) x86_fdc_read_data(state.chip, &byte);
                        else x86_fdc_dma_read(state.chip, &byte);
                        failed |= byte != 0x5au;
                    }
                    /* Deliver TC before the next progression, including EOT. */
                    if (offset + 1u != count) advance(&state);
                }
                x86_fdc_terminal_count(state.chip);
                if (writing && count != 512u) {
                    for (lib_u16 offset = count; offset < 512u; ++offset) advance(&state);
                }
                failed |= !fixture_result(&state, result, sizeof(result)) ||
                    result[0] != (lib_u8)((ids[row].head << 2u) | 1u) ||
                    result[1] != 0u || result[2] != 0u || result[3] != ids[row].c ||
                    result[4] != ids[row].h || result[5] != ids[row].r || result[6] != 2u ||
                    state.drq || state.terminals != 1u ||
                    state.read_count != (writing ? 512u : count);
                if (writing) {
                    const lib_u8 *bytes = ids[row].head == 0u ? state.bytes : state.head_bytes;
                    for (lib_u16 offset = 0u; offset < 512u; ++offset)
                        failed |= bytes[(ids[row].sector - 1u) * 512u + offset] !=
                            (offset < count ? 0xa5u : 0u);
                }
                x86_fdc_destroy(state.chip);
                if (failed) {
                    lib_c_fprintf(lib_c_stderr,
                        "FDC Table 8 command=%02x row=%u variant=%u CHR=%u/%u/%u status=%02x/%02x/%02x\n",
                        commands[command], (lib_u32)row, variant, result[3], result[4],
                        result[5], result[0], result[1], result[2]);
                    return 1;
                }
            }
        }
    }
    return 0;
}

static lib_i32 multi_track_scan_step(void)
{
    static const lib_u8 commands[] = {0xd1u, 0xd9u, 0xddu};
    lib_i32 failed = 0;

    for (lib_size command = 0u; command < sizeof(commands); ++command) {
        for (lib_u8 eot = 2u; eot <= 3u; ++eot) {
            fixture state = {.present = LIB_TRUE, .ready_mask = 0x0fu};
            const x86_fdc_timing timing = {.step_denominator = 1u};
            const lib_u8 compare = command == 1u ? 0u : 1u;
            const lib_u16 count = eot == 3u ? 2048u : 512u;
            lib_u8 result[7];
            if (fixture_create(&state, &timing) != LIB_STATUS_OK) return 1;
            fixture_arm(&state);
            fixture_command(&state, (const lib_u8[]){3u, 0xdfu, 3u}, 3u);
            lib_memory_set(state.bytes, !compare, sizeof(state.bytes));
            lib_memory_set(state.head_bytes, !compare, sizeof(state.head_bytes));
            lib_memory_set(state.head_bytes + 1024u, compare, 512u);
            fixture_command(&state, (const lib_u8[]){commands[command], 0u, 0u,
                0u, 1u, 2u, eot, 0x1bu, 2u}, 9u);
            for (lib_u16 byte = 0u; byte < count; ++byte) {
                x86_fdc_write_data(state.chip, compare);
                failed |= state.last_record.head != byte / 1024u ||
                    state.last_record.sector != (byte / 512u % 2u) * 2u + 1u ||
                    state.last_record.offset != byte % 512u;
            }
            failed |= !fixture_result(&state, result, 7u) ||
                result[0] != (eot == 3u ? 4u : 0x40u) ||
                result[1] != (eot == 3u ? 0u : 0x80u) ||
                result[2] != (eot == 3u ? 0x08u : 0x04u) ||
                result[4] != (eot == 3u ? 1u : 0u) ||
                result[5] != (eot == 3u ? 5u : 3u) || state.read_count != count;
            if (failed) lib_c_fprintf(lib_c_stderr,
                "FDC MT Scan STP=2 command=%02x EOT=%u status=%02x/%02x/%02x H/R=%u/%u\n",
                commands[command], eot, result[0], result[1], result[2], result[4], result[5]);
            x86_fdc_destroy(state.chip);
            if (failed) return failed;
        }
    }
    return failed;
}

int main(void)
{
    fixture state = {.present = LIB_TRUE, .ready_mask = 0x0fu};
    const x86_fdc_timing timing = {.reset_ticks = 0u, .step_numerator = 1u, .step_denominator = 1u, .fm_byte_ticks = 0u, .mfm_byte_ticks = 0u};
    if (fixture_create(&state, &timing) != LIB_STATUS_OK) return 1;
    lib_i32 failed = scan_status(&state);
    failed |= scan_sequence(&state);
    failed |= read_sequence(&state);
    failed |= read_track_forms();
    failed |= id_and_format_results();
    failed |= multi_track_payload();
    failed |= terminal_count_ids();
    failed |= multi_track_scan_step();
    x86_fdc_destroy(state.chip);
    return failed != 0;
}
