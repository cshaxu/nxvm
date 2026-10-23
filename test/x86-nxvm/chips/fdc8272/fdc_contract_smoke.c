#include "fixture.h"
#include "lib/types/file.h"
#include "x86-nxvm/chips/fdc8272/fdc.h"

static lib_bool command_grammar(void)
{
    static const struct { lib_u8 command, flags, length; } forms[] = {
        {0x03u, 0u, 3u}, {0x04u, 0u, 2u}, {0x07u, 0u, 2u},
        {0x08u, 0u, 1u}, {0x0fu, 0u, 3u}, {0x0au, 0x40u, 2u},
        {0x05u, 0xc0u, 9u}, {0x06u, 0xe0u, 9u},
        {0x09u, 0xc0u, 9u}, {0x0cu, 0xe0u, 9u},
        {0x11u, 0xe0u, 9u}, {0x19u, 0xe0u, 9u},
        {0x1du, 0xe0u, 9u}, {0x0du, 0x40u, 6u}, {0x02u, 0x60u, 9u}
    };
    lib_bool failed = LIB_FALSE;

    for (lib_u16 code = 0u; code < 256u; ++code) {
        fixture state = {0};
        const x86_fdc_timing timing = {.step_denominator = 1u};
        lib_u8 length = 1u, result = 0u;
        lib_bool valid = LIB_FALSE;
        for (lib_size row = 0u; row < sizeof(forms) / sizeof(forms[0]); ++row) {
            if ((code & (lib_u8)~forms[row].flags) == forms[row].command) {
                length = forms[row].length;
                valid = LIB_TRUE;
                break;
            }
        }
        if (fixture_create(&state, &timing) != LIB_STATUS_OK) return LIB_FALSE;
        x86_fdc_write_data(state.chip, (lib_u8)code);
        failed |= state.chip->data.command_length != length;
        if (!valid) {
            advance(&state);
            failed |= !fixture_result(&state, &result, 1u) || result != 0x80u ||
                state.irq || state.drq || state.track_queries != 0u || state.read_count != 0u;
        } else if (length > 1u) {
            failed |= state.chip->data.phase != x86_fdc_PHASE_COMMAND ||
                state.chip->data.command_index != 1u;
        }
        x86_fdc_destroy(state.chip);
        if (failed) {
            lib_c_fprintf(lib_c_stderr, "FDC command grammar failed byte=%02x length=%u valid=%u\n",
                code, length, (lib_u32)valid);
            return LIB_FALSE;
        }
    }
    return !failed;
}

static lib_bool recording_boundary(void)
{
    static const lib_u8 commands[] = {0x42u, 0x45u, 0x46u, 0x49u, 0x4cu,
        0x51u, 0x59u, 0x5du};
    lib_bool failed = LIB_FALSE;
    for (lib_size row = 0u; row < sizeof(commands); ++row) {
        fixture state = {.present = LIB_TRUE, .ready_mask = 0x0fu};
        const x86_fdc_timing timing = {.reset_ticks = 2u, .step_numerator = 1u, .step_denominator = 1u, .fm_byte_ticks = 1u, .mfm_byte_ticks = 1u};
        lib_u8 command[] = {commands[row], 0u, 0u, 0u, 1u, 2u,
            commands[row] == 0x42u ? 3u : 1u, 0x1bu, 1u};
        lib_u8 result[7] = {0};
        const lib_u16 count = commands[row] == 0x42u ? 1536u : 512u;
        const lib_bool read = commands[row] == 0x42u || commands[row] == 0x46u ||
            commands[row] == 0x4cu;
        if (fixture_create(&state, &timing) != LIB_STATUS_OK) return LIB_FALSE;
        fixture_arm(&state);
        fixture_command(&state, (lib_u8[]) {3u, 0xf0u, 0u}, 3u);
        lib_memory_set(state.bytes, 0xa5u, sizeof(state.bytes));
        state.deleted[0] = commands[row] == 0x4cu;
        fixture_command(&state, command, sizeof(command));
        state.channel_unreadable = LIB_TRUE;
        for (lib_u16 index = 0u; index < count; ++index) {
            lib_u8 byte = 0xa5u;
            failed |= !state.drq;
            if (read) x86_fdc_dma_read(state.chip, &byte);
            else x86_fdc_dma_write(state.chip, byte);
            failed |= byte != 0xa5u;
            if (index + 1u != count) advance(&state);
        }
        x86_fdc_terminal_count(state.chip);
        failed |= !fixture_result(&state, result, 7u) || result[1] != 0u;
        fixture_command(&state, command, sizeof(command));
        failed |= !fixture_result(&state, result, 7u) || result[1] != 4u;
        x86_fdc_destroy(state.chip);
    }
    {
        fixture state = {.present = LIB_TRUE, .ready_mask = 0x0fu};
        const x86_fdc_timing timing = {.reset_ticks = 2u, .step_numerator = 1u, .step_denominator = 1u, .fm_byte_ticks = 1u, .mfm_byte_ticks = 1u};
        const lib_u8 command[] = {0x4du, 0u, 2u, 1u, 0x1bu, 0xa5u};
        const lib_u8 header[] = {0u, 0u, 1u, 2u};
        lib_u8 result[7] = {0};
        if (fixture_create(&state, &timing) != LIB_STATUS_OK) return LIB_FALSE;
        fixture_arm(&state);
        fixture_command(&state, command, sizeof(command));
        failed |= state.track_queries != 1u;
        state.channel_unreadable = LIB_TRUE;
        for (lib_size index = 0u; index < sizeof(header); ++index) {
            failed |= !state.drq;
            x86_fdc_dma_write(state.chip, header[index]);
            if (index + 1u != sizeof(header)) advance(&state);
        }
        failed |= !fixture_result(&state, result, 7u) || result[1] != 0u;
        for (lib_size index = 0u; index < 512u; ++index) failed |= state.bytes[index] != 0xa5u;
        fixture_command(&state, command, sizeof(command));
        failed |= !fixture_result(&state, result, 7u) || result[1] != 4u;
        x86_fdc_destroy(state.chip);
    }
    return !failed;
}

int main(void)
{
    fixture state = {.present = LIB_TRUE, .ready_mask = 0x0fu};
    x86_fdc_timing timing = {.reset_ticks = 2u, .step_numerator = 1u, .step_denominator = 1u, .fm_byte_ticks = 1u, .mfm_byte_ticks = 1u};
    x86_fdc_observation observation;
    lib_u8 result[7] = {0};
    lib_u64 due;
    lib_i32 failed = !recording_boundary();
    failed |= !command_grammar();
    if (fixture_create(&state, &timing) != LIB_STATUS_OK) return 1;
    x86_fdc_set_reset(state.chip, LIB_TRUE);
    failed |= x86_fdc_next_due_tick(state.chip, &due) != LIB_STATUS_OK || due != 2u;
    advance(&state);
    failed |= state.irq;
    advance(&state);
    failed |= !state.irq;
    for (lib_u8 unit = 0u; unit < 4u; ++unit) {
        fixture_command(&state, (lib_u8[]) {8u}, 1u);
        x86_fdc_read_data(state.chip, &result[0]);
        x86_fdc_read_data(state.chip, &result[1]);
        failed |= result[0] != (0xc0u | unit) || result[1] != 0u;
    }
    fixture_command(&state, (lib_u8[]) {3u, 0xf0u, 0u}, 3u);
    lib_memory_set(state.bytes, 0xa5u, sizeof(state.bytes));
    fixture_command(&state, (lib_u8[]) {0x46u, 0u, 0u, 0u, 1u, 2u, 1u, 0x1bu, 0xffu}, 9u);
    for (lib_u16 index = 0u; index < 512u; ++index) {
        lib_u8 byte = 0u;
        /* Channel qualification belongs to command entry. A changed input
         * must not replace the accepted per-byte record-bounds contract. */
        if (index == 1u) state.channel_unreadable = LIB_TRUE;
        failed |= !state.drq;
        x86_fdc_dma_read(state.chip, &byte);
        failed |= byte != 0xa5u;
        if (index != 511u) advance(&state);
    }
    x86_fdc_terminal_count(state.chip);
    advance(&state);
    for (lib_u8 index = 0u; index < 7u; ++index) x86_fdc_read_data(state.chip, &result[index]);
    failed |= result[0] != 0u || result[1] != 0u || state.drq || state.irq || state.terminals != 1u;
    failed |= x86_fdc_next_due_tick(state.chip, &due) != LIB_STATUS_INVALID_STATE;
    /* The next command must still qualify the changed recording channel. */
    state.track_queries = 0u;
    fixture_command(&state, (lib_u8[]) {0x4au, 0u}, 2u);
    failed |= state.track_queries != 1u;
    failed |= !fixture_result(&state, result, 7u) || result[1] != 4u;
    state.channel_unreadable = LIB_FALSE;
    state.track_queries = 0u;
    fixture_command(&state, (lib_u8[]) {0x4au, 0u}, 2u);
    failed |= state.track_queries != 1u;
    failed |= !fixture_result(&state, result, 7u) || result[1] != 0u;
    fixture_command(&state, (lib_u8[]) {0x0fu, 2u, 3u}, 3u);
    for (lib_u8 index = 0u; index < 3u; ++index) advance(&state);
    fixture_command(&state, (lib_u8[]) {8u}, 1u);
    x86_fdc_read_data(state.chip, &result[0]);
    x86_fdc_read_data(state.chip, &result[1]);
    failed |= result[0] != 0x22u || result[1] != 3u || state.cylinder[2] != 3u || state.cylinder[0] != 0u;
    failed |= x86_fdc_capture(state.chip, &observation) != LIB_STATUS_OK;
    failed |= observation.pcn[2] != 3u || observation.pcn[0] != 0u;
    /* The copied diagnostic cannot become a second controller-state owner. */
    observation.pcn[2] = 99u;
    failed |= x86_fdc_capture(state.chip, &observation) != LIB_STATUS_OK;
    failed |= observation.pcn[2] != 3u;
    x86_fdc_advance_at(state.chip, 10000u);
    x86_fdc_set_reset(state.chip, LIB_FALSE);
    x86_fdc_set_reset(state.chip, LIB_TRUE);
    failed |= x86_fdc_capture(state.chip, &observation) != LIB_STATUS_OK ||
        observation.elapsed_ticks != 10000u;
    x86_fdc_reset(state.chip);
    failed |= x86_fdc_capture(state.chip, &observation) != LIB_STATUS_OK ||
        observation.elapsed_ticks != 0u;
    x86_fdc_advance_at(state.chip, 1u);
    failed |= x86_fdc_capture(state.chip, &observation) != LIB_STATUS_OK ||
        observation.elapsed_ticks != 1u;
    failed |= state.irq || state.drq || state.cylinder[2] != 3u;
    x86_fdc_destroy(state.chip);
    return failed != 0;
}
