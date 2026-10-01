/* Command tables migrated from the NXVM FDC corpus without changing oracles. */
#include "fixture.h"
#include "x86/chips/fdc8272/fdc.h"
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
    static const lib_u8 expected[][3] = {
        {0x04u, 0x08u, 0x04u},
        {0x00u, 0x08u, 0x04u},
        {0x04u, 0x08u, 0x00u}
    };
    lib_i32 failed = 0;

    state->present = LIB_TRUE;
    state->deleted[0] = LIB_FALSE;
    for (lib_size mode = 0u; mode < 3u; ++mode) {
        for (lib_size relation = 0u; relation < 3u; ++relation) {
            lib_u8 result[7];
            lib_u8 command[] = {commands[mode], 0u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 1u};
            fixture_arm(state);
            fixture_command(state,
                (const lib_u8[]){0x03u, 0xdfu, 0x03u}, 3u);
            lib_memory_set(state->bytes, 0x40u + relation * 0x10u,
                sizeof(state->bytes));
            fixture_command(state, command, sizeof(command));
            for (lib_size byte = 0u; byte < 512u; ++byte)
                x86_fdc_write_data(fdc, 0x50u);
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

int main(void)
{
    fixture state = {.present = LIB_TRUE, .ready_mask = 0x0fu};
    const x86_fdc_timing timing = {0u, 1u, 1u, 0u, 0u};
    if (fixture_create(&state, &timing) != LIB_STATUS_OK) return 1;
    lib_i32 failed = scan_status(&state);
    failed |= scan_sequence(&state);
    failed |= read_sequence(&state);
    x86_fdc_destroy(state.chip);
    return failed != 0;
}
