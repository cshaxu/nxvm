#include "myraidenii/core/sound.h"

typedef struct sound_fixture {
    lib_u8 command[2];
    lib_u8 response[2];
    lib_bool pending;
    lib_bool response_pending;
    lib_bool interrupt;
    lib_u8 coin;
    lib_u8 acknowledgements;
} sound_fixture;

static lib_status sound_command_get(void *context, lib_u8 index, lib_u8 *out_value)
{
    sound_fixture *fixture = context;

    if (index >= 2u || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_value = fixture->command[index];
    return LIB_STATUS_OK;
}

static lib_status sound_response_is_pending(void *context, lib_bool *out_pending)
{
    if (out_pending == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_pending = ((sound_fixture *)context)->response_pending;
    return LIB_STATUS_OK;
}

static lib_status sound_command_acknowledge(void *context)
{
    sound_fixture *fixture = context;

    fixture->pending = LIB_FALSE;
    fixture->response_pending = LIB_TRUE;
    ++fixture->acknowledgements;
    return LIB_STATUS_OK;
}

static lib_status sound_interrupt_is_pending(void *context, lib_bool *out_pending)
{
    if (out_pending == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_pending = ((sound_fixture *)context)->interrupt;
    return LIB_STATUS_OK;
}

static lib_status sound_interrupt_accept(void *context)
{
    ((sound_fixture *)context)->interrupt = LIB_FALSE;
    return LIB_STATUS_OK;
}

static lib_status sound_response_set(void *context, lib_u8 index, lib_u8 value)
{
    sound_fixture *fixture = context;

    if (index >= 2u) return LIB_STATUS_INVALID_ARGUMENT;
    fixture->response[index] = value;
    return LIB_STATUS_OK;
}

static lib_status sound_coin_get(void *context, lib_u8 *out_value)
{
    if (out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_value = ((sound_fixture *)context)->coin;
    return LIB_STATUS_OK;
}

static lib_status sound_create_with_samples(const lib_u8 *program,
    const lib_u8 *samples_1, const lib_u8 *samples_2, sound_fixture *fixture,
    myraidenii_core_sound **out_sound)
{
    const myraidenii_core_sound_config config = {
        .program = program,
        .program_bytes = 0x2000u,
        .samples_1 = samples_1,
        .samples_1_bytes = samples_1 == LIB_NULL ? 0u :
            MYRAIDENII_CORE_SOUND_SAMPLE_BYTES,
        .samples_2 = samples_2,
        .samples_2_bytes = samples_2 == LIB_NULL ? 0u :
            MYRAIDENII_CORE_SOUND_SAMPLE_BYTES,
        .transport = {
            .context = fixture,
            .command_get = sound_command_get,
            .response_is_pending = sound_response_is_pending,
            .command_acknowledge = sound_command_acknowledge,
            .interrupt_is_pending = sound_interrupt_is_pending,
            .interrupt_accept = sound_interrupt_accept,
            .response_set = sound_response_set,
            .coin_get = sound_coin_get
        }
    };

    return myraidenii_core_sound_create(&config, out_sound);
}

static lib_status sound_create(const lib_u8 *program, sound_fixture *fixture,
    myraidenii_core_sound **out_sound)
{
    return sound_create_with_samples(program, LIB_NULL, LIB_NULL, fixture, out_sound);
}

static void sound_program_write_oki(lib_u8 *program, lib_size *at, lib_u8 value)
{
    program[(*at)++] = 0x3eu; program[(*at)++] = value;
    program[(*at)++] = 0x32u; program[(*at)++] = 0x00u; program[(*at)++] = 0x60u;
}

static void sound_program_write_ym(lib_u8 *program, lib_size *at,
    lib_u8 address, lib_u8 value)
{
    program[(*at)++] = 0x3eu; program[(*at)++] = address;
    program[(*at)++] = 0x32u; program[(*at)++] = 0x08u; program[(*at)++] = 0x40u;
    program[(*at)++] = 0x3eu; program[(*at)++] = value;
    program[(*at)++] = 0x32u; program[(*at)++] = 0x09u; program[(*at)++] = 0x40u;
}

static void sound_program_read_oki_response(lib_u8 *program, lib_size *at,
    lib_u8 response)
{
    program[(*at)++] = 0x3au; program[(*at)++] = 0x00u; program[(*at)++] = 0x60u;
    program[(*at)++] = 0x32u; program[(*at)++] = response; program[(*at)++] = 0x40u;
}

static void sound_program_expect_zero(lib_u8 *program, lib_size *at)
{
    program[(*at)++] = 0x20u; program[(*at)++] = 1u; /* JR NZ,+1 */
    program[(*at)++] = 0x04u; /* INC B */
}

static void sound_program_expect_carry(lib_u8 *program, lib_size *at)
{
    program[(*at)++] = 0x30u; program[(*at)++] = 1u; /* JR NC,+1 */
    program[(*at)++] = 0x04u; /* INC B */
}

static void sound_program_load_pair(lib_u8 *program, lib_size *at, lib_u8 pair,
    lib_u16 value)
{
    program[(*at)++] = (lib_u8)(0x01u | (pair << 4u));
    program[(*at)++] = (lib_u8)value;
    program[(*at)++] = (lib_u8)(value >> 8u);
}

static void sound_program_publish_hl(lib_u8 *program, lib_size *at)
{
    program[(*at)++] = 0x22u; program[(*at)++] = 0x18u; program[(*at)++] = 0x40u;
}

static lib_i32 sound_executes_fixed_transport(void)
{
    static lib_u8 program[0x2000u];
    sound_fixture fixture = {.command = {0x5au, 0x3cu}, .pending = LIB_TRUE};
    myraidenii_core_sound *sound = LIB_NULL;
    myraidenii_core_sound_observation observation;
    lib_i32 failed = 1;

    lib_memory_set(program, 0, sizeof(program));
    program[0u] = 0x3au; program[1u] = 0x10u; program[2u] = 0x40u;
    program[3u] = 0x32u; program[4u] = 0x18u; program[5u] = 0x40u;
    program[6u] = 0x3au; program[7u] = 0x11u; program[8u] = 0x40u;
    program[9u] = 0x32u; program[10u] = 0u; program[11u] = 0x20u;
    program[12u] = 0x3au; program[13u] = 0x12u; program[14u] = 0x40u;
    program[15u] = 0x32u; program[16u] = 0x19u; program[17u] = 0x40u;
    program[18u] = 0x3eu; program[19u] = 0u;
    program[20u] = 0x32u; program[21u] = 0x00u; program[22u] = 0x40u;
    program[23u] = 0x76u;
    if (sound_create(program, &fixture, &sound) != LIB_STATUS_OK ||
        myraidenii_core_sound_advance(sound,
            MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR) != LIB_STATUS_OK ||
        fixture.response[0] != 0x5au || fixture.response[1] != 0u || fixture.pending ||
        !fixture.response_pending ||
        fixture.acknowledgements != 1u ||
        myraidenii_core_sound_observation_get(sound, &observation) != LIB_STATUS_OK ||
        observation.transport_command_reads[0u] != 1u ||
        observation.transport_command_reads[1u] != 1u ||
        observation.transport_command_acknowledgements != 1u ||
        observation.transport_interrupt_accepts != 0u)
        goto done;
    myraidenii_core_sound_reset(sound);
    fixture.command[0] = 0x96u;
    fixture.pending = LIB_TRUE;
    if (myraidenii_core_sound_advance(sound,
            MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR) != LIB_STATUS_OK ||
        fixture.response[0] != 0x96u || fixture.response[1] != 1u || fixture.pending ||
        fixture.acknowledgements != 2u ||
        myraidenii_core_sound_observation_get(sound, &observation) != LIB_STATUS_OK ||
        observation.transport_command_reads[0u] != 1u ||
        observation.transport_command_reads[1u] != 1u ||
        observation.transport_command_acknowledgements != 1u ||
        observation.transport_interrupt_accepts != 0u)
        goto done;
    failed = 0;
done:
    myraidenii_core_sound_destroy(sound);
    return failed;
}

static lib_i32 sound_rejects_invalid_configuration(void)
{
    sound_fixture fixture = {0};
    myraidenii_core_sound *sound = LIB_NULL;

    return myraidenii_core_sound_create(LIB_NULL, &sound) != LIB_STATUS_INVALID_ARGUMENT ||
        sound_create(LIB_NULL, &fixture, &sound) != LIB_STATUS_INVALID_ARGUMENT ||
        myraidenii_core_sound_advance(LIB_NULL, 1u) != LIB_STATUS_INVALID_ARGUMENT;
}

static lib_i32 sound_switches_fixed_bank(void)
{
    static lib_u8 program[0x10000u];
    sound_fixture fixture = {0};
    myraidenii_core_sound *sound = LIB_NULL;
    const myraidenii_core_sound_config config = {
        .program = program,
        .program_bytes = sizeof(program),
        .transport = {
            .context = &fixture,
            .command_get = sound_command_get,
            .response_is_pending = sound_response_is_pending,
            .command_acknowledge = sound_command_acknowledge,
            .interrupt_is_pending = sound_interrupt_is_pending,
            .interrupt_accept = sound_interrupt_accept,
            .response_set = sound_response_set,
            .coin_get = sound_coin_get
        }
    };
    lib_i32 failed = 1;

    lib_memory_set(program, 0, sizeof(program));
    program[0u] = 0x3eu; program[1u] = 2u;
    program[2u] = 0x32u; program[3u] = 0x1au; program[4u] = 0x40u;
    program[5u] = 0xc3u; program[6u] = 0u; program[7u] = 0x82u;
    program[0x8200u] = 0x3eu; program[0x8201u] = 0x3cu;
    program[0x8202u] = 0x32u; program[0x8203u] = 0x18u; program[0x8204u] = 0x40u;
    program[0x8205u] = 0x3eu; program[0x8206u] = 3u;
    program[0x8207u] = 0x32u; program[0x8208u] = 0x1au; program[0x8209u] = 0x40u;
    program[0x020au] = 0xc3u; program[0x020bu] = 0u; program[0x020cu] = 0x83u;
    program[0x0300u] = 0x3eu; program[0x0301u] = 0x69u;
    program[0x0302u] = 0x32u; program[0x0303u] = 0x19u; program[0x0304u] = 0x40u;
    program[0x0305u] = 0x76u;
    if (myraidenii_core_sound_create(&config, &sound) != LIB_STATUS_OK ||
        myraidenii_core_sound_advance(sound,
            MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR) != LIB_STATUS_OK ||
        fixture.response[0] != 0x3cu || fixture.response[1] != 0x69u)
        goto done;
    failed = 0;
done:
    myraidenii_core_sound_destroy(sound);
    return failed;
}

static lib_i32 sound_executes_bootstrap_control_flow(void)
{
    static lib_u8 program[0x2000u];
    sound_fixture fixture = {0};
    myraidenii_core_sound *sound = LIB_NULL;
    lib_i32 failed = 1;

    lib_memory_set(program, 0, sizeof(program));
    program[0u] = 0x31u; program[1u] = 0u; program[2u] = 0x28u;
    program[3u] = 0x3eu; program[4u] = 1u;
    program[5u] = 0x3du;
    program[6u] = 0x28u; program[7u] = 2u;
    program[8u] = 0x3eu; program[9u] = 0x7fu;
    program[10u] = 0xcdu; program[11u] = 0x10u; program[12u] = 0u;
    program[13u] = 0x76u;
    program[16u] = 0xddu; program[17u] = 0x21u;
    program[18u] = 0x34u; program[19u] = 0x12u;
    program[20u] = 0xddu; program[21u] = 0xe5u;
    program[22u] = 0xddu; program[23u] = 0xe1u;
    program[24u] = 0xddu; program[25u] = 0x21u;
    program[26u] = 0u; program[27u] = 0x20u;
    program[28u] = 0xddu; program[29u] = 0x36u;
    program[30u] = 1u; program[31u] = 0x4au;
    program[32u] = 0xddu; program[33u] = 0xcbu;
    program[34u] = 1u; program[35u] = 0x46u;
    program[36u] = 0xddu; program[37u] = 0x6eu; program[38u] = 1u;
    program[39u] = 0x7du;
    program[40u] = 0x32u; program[41u] = 0x18u; program[42u] = 0x40u;
    program[43u] = 0x21u; program[44u] = 0u; program[45u] = 0x20u;
    program[46u] = 0x11u; program[47u] = 0x10u; program[48u] = 0x20u;
    program[49u] = 0x01u; program[50u] = 1u; program[51u] = 0u;
    program[52u] = 0x36u; program[53u] = 0x3cu;
    program[54u] = 0xedu; program[55u] = 0xb0u;
    program[56u] = 0x3au; program[57u] = 0x10u; program[58u] = 0x20u;
    program[59u] = 0x32u; program[60u] = 0x19u; program[61u] = 0x40u;
    program[62u] = 0xc9u;
    if (sound_create(program, &fixture, &sound) != LIB_STATUS_OK ||
        myraidenii_core_sound_advance(sound,
            MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR) != LIB_STATUS_OK ||
        fixture.response[0] != 0x4au || fixture.response[1] != 0x3cu)
        goto done;
    failed = 0;
done:
    myraidenii_core_sound_destroy(sound);
    return failed;
}

static lib_i32 sound_allows_no_device_map_slot(void)
{
    static lib_u8 program[0x2000u];
    sound_fixture fixture = {0};
    myraidenii_core_sound *sound = LIB_NULL;
    lib_i32 failed = 1;

    lib_memory_set(program, 0, sizeof(program));
    program[0u] = 0x3au; program[1u] = 0x04u; program[2u] = 0x40u;
    program[3u] = 0x32u; program[4u] = 0x18u; program[5u] = 0x40u;
    program[6u] = 0x3eu; program[7u] = 0x5au;
    program[8u] = 0x32u; program[9u] = 0x04u; program[10u] = 0x40u;
    program[11u] = 0x3au; program[12u] = 0x04u; program[13u] = 0x40u;
    program[14u] = 0x32u; program[15u] = 0x19u; program[16u] = 0x40u;
    program[17u] = 0x76u;
    if (sound_create(program, &fixture, &sound) != LIB_STATUS_OK ||
        myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
        fixture.response[0] != 0xffu || fixture.response[1] != 0xffu)
        goto done;
    failed = 0;
done:
    myraidenii_core_sound_destroy(sound);
    return failed;
}

static lib_i32 sound_executes_device_map(void)
{
    static lib_u8 program[0x2000u];
    static lib_u8 samples_1[MYRAIDENII_CORE_SOUND_SAMPLE_BYTES];
    sound_fixture fixture = {0};
    myraidenii_core_sound *sound = LIB_NULL;
    myraidenii_core_sound_observation observation;
    lib_size at = 0u;
    lib_i32 failed = 1;

    lib_memory_set(program, 0, sizeof(program));
    lib_memory_set(samples_1, 0, sizeof(samples_1));
    samples_1[10u] = 0x20u;
    samples_1[13u] = 0x21u;
    program[at++] = 0x3eu; program[at++] = 0x10u;
    program[at++] = 0x32u; program[at++] = 0x08u; program[at++] = 0x40u;
    program[at++] = 0x3eu; program[at++] = 0xffu;
    program[at++] = 0x32u; program[at++] = 0x09u; program[at++] = 0x40u;
    program[at++] = 0x3eu; program[at++] = 0x11u;
    program[at++] = 0x32u; program[at++] = 0x08u; program[at++] = 0x40u;
    program[at++] = 0x3eu; program[at++] = 0x03u;
    program[at++] = 0x32u; program[at++] = 0x09u; program[at++] = 0x40u;
    program[at++] = 0x3eu; program[at++] = 0x14u;
    program[at++] = 0x32u; program[at++] = 0x08u; program[at++] = 0x40u;
    program[at++] = 0x3eu; program[at++] = 0x05u;
    program[at++] = 0x32u; program[at++] = 0x09u; program[at++] = 0x40u;
    for (lib_u8 index = 0u; index < 20u; ++index) program[at++] = 0x00u;
    program[at++] = 0x3au; program[at++] = 0x09u; program[at++] = 0x40u;
    program[at++] = 0x32u; program[at++] = 0x18u; program[at++] = 0x40u;
    program[at++] = 0x3eu; program[at++] = 0x81u;
    program[at++] = 0x32u; program[at++] = 0x00u; program[at++] = 0x60u;
    program[at++] = 0x3eu; program[at++] = 0x10u;
    program[at++] = 0x32u; program[at++] = 0x00u; program[at++] = 0x60u;
    program[at++] = 0x3au; program[at++] = 0x00u; program[at++] = 0x60u;
    program[at++] = 0xe6u; program[at++] = 0x01u;
    program[at++] = 0x21u; program[at++] = 0x00u; program[at++] = 0x20u;
    program[at++] = 0x77u;
    for (lib_u16 index = 0u; index < 160u; ++index) program[at++] = 0x00u;
    program[at++] = 0x3eu; program[at++] = 0x08u;
    program[at++] = 0x32u; program[at++] = 0x00u; program[at++] = 0x60u;
    program[at++] = 0x3au; program[at++] = 0x00u; program[at++] = 0x60u;
    program[at++] = 0xb6u;
    program[at++] = 0x32u; program[at++] = 0x19u; program[at++] = 0x40u;
    program[at++] = 0x76u;
    if (sound_create_with_samples(program, samples_1, LIB_NULL, &fixture, &sound) !=
            LIB_STATUS_OK ||
        myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
        fixture.response[0] != 1u || fixture.response[1] != 1u ||
        myraidenii_core_sound_observation_get(sound, &observation) != LIB_STATUS_OK ||
        observation.ym_address_writes != 3u || observation.ym_data_writes != 3u ||
        observation.oki_writes[0u] != 3u || observation.oki_writes[1u] != 0u ||
        observation.oki_sample_commands[0u] != 1u ||
        observation.oki_voice_selections[0u] != 1u || observation.oki_stops[0u] != 1u ||
        observation.oki_rejected_selections[0u] != 0u ||
        observation.oki_ignored_controls[0u] != 0u ||
        observation.oki_voice_starts[0u] != 1u || observation.oki_voice_starts[1u] != 0u ||
        observation.pcm_nonzero_frames == 0u)
        goto done;
    myraidenii_core_sound_reset(sound);
    if (myraidenii_core_sound_observation_get(sound, &observation) != LIB_STATUS_OK ||
        observation.ym_address_writes != 0u || observation.ym_data_writes != 0u ||
        observation.oki_writes[0u] != 0u || observation.oki_writes[1u] != 0u ||
        observation.oki_sample_commands[0u] != 0u ||
        observation.oki_voice_selections[0u] != 0u || observation.oki_stops[0u] != 0u ||
        observation.oki_rejected_selections[0u] != 0u ||
        observation.oki_ignored_controls[0u] != 0u ||
        observation.oki_voice_starts[0u] != 0u || observation.oki_voice_starts[1u] != 0u ||
        observation.pcm_nonzero_frames != 0u)
        goto done;
    failed = 0;
done:
    myraidenii_core_sound_destroy(sound);
    return failed;
}

static lib_i32 sound_classifies_oki_write_dispositions(void)
{
    static lib_u8 program[0x2000u];
    static lib_u8 samples_1[MYRAIDENII_CORE_SOUND_SAMPLE_BYTES];
    sound_fixture fixture = {0};
    myraidenii_core_sound *sound = LIB_NULL;
    myraidenii_core_sound_observation observation;
    lib_size at = 0u;
    lib_i32 failed = 1;

    lib_memory_set(program, 0, sizeof(program));
    lib_memory_set(samples_1, 0, sizeof(samples_1));
    sound_program_write_oki(program, &at, 0x81u);
    sound_program_write_oki(program, &at, 0x0fu);
    sound_program_write_oki(program, &at, 0x00u);
    sound_program_write_oki(program, &at, 0x10u);
    program[at] = 0x76u;
    if (sound_create_with_samples(program, samples_1, LIB_NULL, &fixture, &sound) !=
            LIB_STATUS_OK ||
        myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
        myraidenii_core_sound_observation_get(sound, &observation) != LIB_STATUS_OK ||
        observation.oki_writes[0u] != 4u ||
        observation.oki_sample_commands[0u] != 1u ||
        observation.oki_voice_selections[0u] != 0u ||
        observation.oki_stops[0u] != 1u ||
        observation.oki_rejected_selections[0u] != 1u ||
        observation.oki_ignored_controls[0u] != 1u ||
        observation.oki_voice_starts[0u] != 0u)
        goto done;
    failed = 0;
done:
    myraidenii_core_sound_destroy(sound);
    return failed;
}

static lib_i32 sound_ym_timer_status_enable(void)
{
    static lib_u8 program[0x2000u];
    sound_fixture fixture = {0};
    myraidenii_core_sound *sound = LIB_NULL;
    lib_size at = 0u;
    lib_status status;

    lib_memory_set(program, 0, sizeof(program));
    sound_program_write_ym(program, &at, 0x10u, 0xffu);
    sound_program_write_ym(program, &at, 0x11u, 0x03u);
    sound_program_write_ym(program, &at, 0x12u, 0xffu);
    sound_program_write_ym(program, &at, 0x14u, 0x03u);
    at += 500u; /* Let both timers expire with flag generation disabled. */
    sound_program_write_ym(program, &at, 0x14u, 0x0cu);
    program[at++] = 0x3au; program[at++] = 0x08u; program[at++] = 0x40u;
    program[at++] = 0x32u; program[at++] = 0x18u; program[at++] = 0x40u;
    sound_program_write_ym(program, &at, 0x14u, 0x0fu);
    at += 500u; /* Enabled expirations latch both flags. */
    sound_program_write_ym(program, &at, 0x14u, 0x00u);
    program[at++] = 0x3au; program[at++] = 0x08u; program[at++] = 0x40u;
    program[at++] = 0x32u; program[at++] = 0x19u; program[at++] = 0x40u;
    program[at] = 0x76u;

    status = sound_create(program, &fixture, &sound);
    if (status == LIB_STATUS_OK) status = myraidenii_core_sound_advance(sound, 20000u);
    myraidenii_core_sound_destroy(sound);
    return status != LIB_STATUS_OK || fixture.response[0] != 0u || fixture.response[1] != 3u;
}

static lib_i32 sound_delivers_ym_timer_interrupt(void)
{
    static lib_u8 program[0x2000u];
    static const lib_u8 handler[] = {
        0xf5u, 0x04u, 0x78u, /* PUSH AF / INC B / LD A,B */
        0x32u, 0x18u, 0x40u,
        0xf1u, 0xfbu, 0xedu, 0x4du /* POP AF / EI / RETI */
    };
    sound_fixture fixture = {0};
    myraidenii_core_sound *sound = LIB_NULL;
    lib_size at = 0x100u;
    lib_i32 failed = 1;

    lib_memory_set(program, 0, sizeof(program));
    program[0u] = 0xc3u; program[1u] = 0u; program[2u] = 1u;
    program[at++] = 0x31u; program[at++] = 0xffu; program[at++] = 0x27u;
    program[at++] = 0x06u; program[at++] = 0u;
    program[at++] = 0x3eu; program[at++] = 0x10u;
    program[at++] = 0x32u; program[at++] = 0x08u; program[at++] = 0x40u;
    program[at++] = 0x3eu; program[at++] = 0xffu;
    program[at++] = 0x32u; program[at++] = 0x09u; program[at++] = 0x40u;
    program[at++] = 0x3eu; program[at++] = 0x11u;
    program[at++] = 0x32u; program[at++] = 0x08u; program[at++] = 0x40u;
    program[at++] = 0x3eu; program[at++] = 0x03u;
    program[at++] = 0x32u; program[at++] = 0x09u; program[at++] = 0x40u;
    program[at++] = 0x3eu; program[at++] = 0x14u;
    program[at++] = 0x32u; program[at++] = 0x08u; program[at++] = 0x40u;
    program[at++] = 0x3eu; program[at++] = 0x05u;
    program[at++] = 0x32u; program[at++] = 0x09u; program[at++] = 0x40u;
    program[at++] = 0xedu; program[at++] = 0x46u;
    program[at++] = 0xfbu;
    program[at++] = 0x00u;
    /* Keep the timer asserted; only its EOI may permit another delivery. */
    program[at++] = 0x3au; program[at++] = 0x10u; program[at++] = 0x40u;
    program[at++] = 0xb7u;
    program[at++] = 0x28u; program[at++] = 0xfau;
    program[at++] = 0x32u; program[at++] = 0x02u; program[at++] = 0x40u;
    program[at++] = 0x18u; program[at++] = 0xf5u;
    lib_memory_copy(program + 0x10u, handler, sizeof(handler));
    if (sound_create(program, &fixture, &sound) != LIB_STATUS_OK ||
        myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
        fixture.response[0] != 1u ||
        myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
        fixture.response[0] != 1u)
        goto done;
    fixture.command[0] = 1u;
    if (myraidenii_core_sound_advance(sound, 1000u) != LIB_STATUS_OK ||
        fixture.response[0] <= 1u)
        goto done;
    fixture.command[0] = 0u;
    myraidenii_core_sound_reset(sound);
    if (myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
        fixture.response[0] != 1u) goto done;
    failed = 0;
done:
    myraidenii_core_sound_destroy(sound);
    return failed;
}

static lib_i32 sound_delivers_transport_interrupt(void)
{
    static lib_u8 program[0x2000u];
    sound_fixture fixture = {.interrupt = LIB_TRUE};
    myraidenii_core_sound *sound = LIB_NULL;
    myraidenii_core_sound_observation observation;
    lib_i32 failed = 1;

    lib_memory_set(program, 0, sizeof(program));
    program[0u] = 0xc3u; program[1u] = 0u; program[2u] = 1u;
    program[0x18u] = 0x3eu; program[0x19u] = 0x5au;
    program[0x1au] = 0x32u; program[0x1bu] = 0x18u; program[0x1cu] = 0x40u;
    program[0x1du] = 0x76u;
    program[0x100u] = 0x31u; program[0x101u] = 0xffu; program[0x102u] = 0x27u;
    program[0x103u] = 0xedu; program[0x104u] = 0x46u;
    program[0x105u] = 0xfbu;
    program[0x106u] = 0x00u;
    program[0x107u] = 0x76u;
    if (sound_create(program, &fixture, &sound) != LIB_STATUS_OK ||
        myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
        fixture.response[0] != 0x5au || fixture.interrupt ||
        myraidenii_core_sound_observation_get(sound, &observation) != LIB_STATUS_OK ||
        observation.transport_interrupt_accepts != 1u)
        goto done;
    failed = 0;
done:
    myraidenii_core_sound_destroy(sound);
    return failed;
}

static lib_i32 sound_interrupt_after_ei_instruction(void)
{
    static lib_u8 program[0x2000u];
    sound_fixture fixture = {.interrupt = LIB_TRUE};
    myraidenii_core_sound *sound = LIB_NULL;
    lib_i32 failed = 1;

    lib_memory_set(program, 0, sizeof(program));
    program[0u] = 0xc3u; program[1u] = 0u; program[2u] = 1u;
    program[0x18u] = 0x32u; program[0x19u] = 0x18u; program[0x1au] = 0x40u;
    program[0x1bu] = 0x76u;
    program[0x100u] = 0x31u; program[0x101u] = 0xffu; program[0x102u] = 0x27u;
    program[0x103u] = 0x3eu; program[0x104u] = 0x12u;
    program[0x105u] = 0xfbu; program[0x106u] = 0x00u;
    program[0x107u] = 0x3eu; program[0x108u] = 0x34u;
    if (sound_create(program, &fixture, &sound) != LIB_STATUS_OK ||
        myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
        fixture.response[0] != 0x12u || fixture.interrupt)
        goto done;
    failed = 0;
done:
    myraidenii_core_sound_destroy(sound);
    return failed;
}

static lib_i32 sound_acknowledges_transport_service(void)
{
    static lib_u8 program[0x2000u];
    static const lib_u8 entry[] = {
        0x31u, 0xffu, 0x27u, /* LD SP,27ff */
        0x06u, 0x00u,       /* LD B,0: interrupt count */
        0xedu, 0x46u,       /* IM 0 */
        0xf3u, 0xfbu, 0x00u, /* DI/EI must not end device service. */
        0x3au, 0x10u, 0x40u, /* LD A,(4010): permit EOI */
        0xb7u, 0x28u, 0xf7u, /* OR A / JR Z,0107 */
        0x32u, 0x01u, 0x40u, /* LD (EOI),A */
        0xc3u, 0x07u, 0x01u
    };
    static const lib_u8 handler[] = {
        0xf5u,             /* Preserve the polling loop's A/flags. */
        0x04u, 0x78u,       /* INC B / LD A,B */
        0x32u, 0x18u, 0x40u, /* LD (4018),A */
        0xf1u, 0xfbu, 0xedu, 0x4du /* POP AF / EI / RETI */
    };

    for (lib_u8 acknowledge = 1u; acknowledge <= 3u; ++acknowledge) {
        sound_fixture fixture = {.interrupt = LIB_TRUE};
        myraidenii_core_sound *sound = LIB_NULL;
        lib_i32 failed = 1;

        lib_memory_set(program, 0, sizeof(program));
        program[0u] = 0xc3u; program[1u] = 0u; program[2u] = 1u;
        lib_memory_copy(program + 0x18u, handler, sizeof(handler));
        lib_memory_copy(program + 0x100u, entry, sizeof(entry));
        program[0x111u] = acknowledge;
        if (sound_create(program, &fixture, &sound) != LIB_STATUS_OK ||
            myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
            fixture.response[0] != 1u || fixture.interrupt)
            goto done;
        fixture.interrupt = LIB_TRUE;
        if (myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
            fixture.response[0] != 1u || !fixture.interrupt)
            goto done;
        fixture.command[0] = 1u;
        if (myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
            fixture.response[0] != (acknowledge == 2u ? 1u : 2u) ||
            fixture.interrupt != (acknowledge == 2u))
            goto done;
        fixture.command[0] = 0u;
        fixture.interrupt = LIB_TRUE;
        myraidenii_core_sound_reset(sound);
        if (myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
            fixture.response[0] != 1u || fixture.interrupt)
            goto done;
        failed = 0;
done:
        myraidenii_core_sound_destroy(sound);
        if (failed) return failed;
    }
    return 0;
}

static lib_i32 sound_routes_ym_channels(void)
{
    static const lib_u8 carriers[8] = {8u, 8u, 8u, 8u, 10u, 14u, 14u, 15u};
    static lib_u8 program[0x2000u];
    lib_i16 frames[256u * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS];

    for (lib_u8 mode = 0u; mode < 37u; ++mode) {
        sound_fixture fixture = {0};
        myraidenii_core_sound *sound = LIB_NULL;
        lib_u32 copied = 0u;
        lib_u64 dropped = 0u;
        lib_bool left = LIB_FALSE, right = LIB_FALSE;
        lib_size at = 0u;
        lib_status status;

        lib_memory_set(program, 0, sizeof(program));
        const lib_u8 algorithm = mode >= 5u ? (lib_u8)((mode - 5u) / 4u) : 0u;
        const lib_u8 op = mode >= 5u ? (lib_u8)((mode - 5u) % 4u) : 0u;
        const lib_u8 slot = (lib_u8)((((op & 1u) << 1u) | ((op & 2u) >> 1u)) * 8u);
        const lib_bool expected_left = mode >= 5u ?
            (carriers[algorithm] & (1u << op)) != 0u : (mode == 0u || mode == 4u);

        sound_program_write_ym(program, &at, 0x20u, mode >= 5u ? (lib_u8)(0x40u | algorithm) : mode >= 3u ? 0x40u :
            (lib_u8)(0x07u | (mode == 0u ? 0x40u : mode == 1u ? 0x80u : 0u)));
        sound_program_write_ym(program, &at, 0x28u, 0x4au);
        sound_program_write_ym(program, &at, 0x78u, 0u);
        sound_program_write_ym(program, &at, 0x98u, 0x1fu);
        sound_program_write_ym(program, &at, 0x80u, 0x1fu);
        if (mode >= 5u) sound_program_write_ym(program, &at, (lib_u8)(0x80u + slot), 0x1fu);
        sound_program_write_ym(program, &at, 0x08u, mode >= 5u ?
            (lib_u8)(1u << (op + 3u)) : mode == 3u ? 0x08u : 0x40u);
        program[at] = 0x76u;
        status = sound_create(program, &fixture, &sound);
        if (status == LIB_STATUS_OK) status = myraidenii_core_sound_advance(sound, 100000u);
        if (status == LIB_STATUS_OK)
            status = myraidenii_core_sound_pcm_copy(sound, frames, 256u, &copied, &dropped);
        for (lib_u32 index = 0u; index < copied; ++index) {
            left |= frames[index * 2u] != 0;
            right |= frames[index * 2u + 1u] != 0;
        }
        myraidenii_core_sound_destroy(sound);
        if (status != LIB_STATUS_OK || copied == 0u || dropped != 0u ||
            left != expected_left || right != (mode == 1u)) return 1;
    }
    return 0;
}

static lib_i32 sound_ym_pitch_and_level(void)
{
    static lib_u8 program[0x2000u];
    lib_i16 frames[2048u * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS];
    lib_u32 crossings[6] = {0u};
    lib_u32 first_crossing = 0u, last_crossing = 0u;
    lib_i32 peak[6] = {0};

    for (lib_u8 mode = 0u; mode < 6u; ++mode) {
        sound_fixture fixture = {0};
        myraidenii_core_sound *sound = LIB_NULL;
        lib_u32 copied = 0u;
        lib_u64 dropped = 0u;
        lib_size at = 0u;
        lib_status status;

        lib_memory_set(program, 0, sizeof(program));
        sound_program_write_ym(program, &at, 0x20u, 0x47u);
        sound_program_write_ym(program, &at, 0x28u, mode == 1u ? 0x5cu : 0x4cu);
        sound_program_write_ym(program, &at, 0x58u, 1u);
        sound_program_write_ym(program, &at, 0x78u, mode == 2u ? 40u : 0u);
        sound_program_write_ym(program, &at, 0x98u, 0x1fu);
        if (mode >= 3u) sound_program_write_ym(program, &at, 0xd8u,
            (lib_u8)((mode - 2u) << 6u));
        sound_program_write_ym(program, &at, 0x08u, 0x40u);
        program[at] = 0x76u;
        status = sound_create(program, &fixture, &sound);
        if (status == LIB_STATUS_OK) status = myraidenii_core_sound_advance(sound, 1000000u);
        if (status == LIB_STATUS_OK)
            status = myraidenii_core_sound_pcm_copy(sound, frames, 2048u, &copied, &dropped);
        for (lib_u32 index = 1u; index < copied; ++index) {
            const lib_i32 sample = frames[index * 2u];
            const lib_i32 previous = frames[(index - 1u) * 2u];
            const lib_i32 magnitude = sample < 0 ? -sample : sample;
            if (magnitude > peak[mode]) peak[mode] = magnitude;
            if (previous <= 0 && sample > 0) {
                if (mode == 0u) {
                    if (crossings[mode] == 0u) first_crossing = index;
                    last_crossing = index;
                }
                ++crossings[mode];
            }
        }
        myraidenii_core_sound_destroy(sound);
        if (status != LIB_STATUS_OK || copied != 2048u || dropped != 0u) return 1;
    }
    return peak[0] < 100 || peak[2] >= peak[0] / 4 ||
        crossings[0] < 10u ||
        last_crossing - first_crossing < (crossings[0] - 1u) * 101u ||
        last_crossing - first_crossing > (crossings[0] - 1u) * 105u ||
        crossings[1] < crossings[0] * 19u / 10u ||
        crossings[1] > crossings[0] * 21u / 10u ||
        crossings[3] < crossings[0] * 13u / 10u ||
        crossings[3] > crossings[0] * 15u / 10u ||
        crossings[4] < crossings[0] * 15u / 10u ||
        crossings[4] > crossings[0] * 17u / 10u ||
        crossings[5] < crossings[0] * 16u / 10u ||
        crossings[5] > crossings[0] * 18u / 10u;
}

static lib_i32 sound_ym_modulation(void)
{
    static lib_u8 program[0x2000u];
    lib_i16 baseline[256u * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS];
    lib_i16 frames[256u * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS];

    for (lib_u8 algorithm = 0u; algorithm < 7u; ++algorithm) {
        for (lib_u8 keyed = 0u; keyed < 2u; ++keyed) {
            sound_fixture fixture = {0};
            myraidenii_core_sound *sound = LIB_NULL;
            lib_u32 copied = 0u;
            lib_u64 dropped = 0u;
            lib_bool changed = LIB_FALSE;
            lib_size at = 0u;
            lib_status status;

            lib_memory_set(program, 0, sizeof(program));
            sound_program_write_ym(program, &at, 0x20u, (lib_u8)(0x40u | algorithm));
            sound_program_write_ym(program, &at, 0x28u, 0x4cu);
            sound_program_write_ym(program, &at, 0x40u, 2u);
            for (lib_u8 slot = 0u; slot < 32u; slot += 8u)
                sound_program_write_ym(program, &at, (lib_u8)(0x80u + slot), 0x1fu);
            sound_program_write_ym(program, &at, 0x08u, keyed ? 0x78u : 0x70u);
            program[at] = 0x76u;
            status = sound_create(program, &fixture, &sound);
            if (status == LIB_STATUS_OK) status = myraidenii_core_sound_advance(sound, 100000u);
            if (status == LIB_STATUS_OK)
                status = myraidenii_core_sound_pcm_copy(sound, frames, 256u, &copied, &dropped);
            myraidenii_core_sound_destroy(sound);
            if (status != LIB_STATUS_OK || copied != 256u || dropped != 0u) return 1;
            for (lib_u32 index = 0u; index < 256u; ++index) {
                if (keyed == 0u) baseline[index * 2u] = frames[index * 2u];
                else if (baseline[index * 2u] != frames[index * 2u]) changed = LIB_TRUE;
            }
            if (keyed != 0u && !changed) return 1;
        }
    }
    return 0;
}

static lib_i32 sound_ym_key_off(void)
{
    static lib_u8 program[0x2000u];
    lib_i16 frames[2048u * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS];
    sound_fixture fixture = {0};
    myraidenii_core_sound *sound = LIB_NULL;
    lib_u32 copied = 0u;
    lib_u64 dropped = 0u;
    lib_size at = 0u;
    lib_bool sounded = LIB_FALSE;
    lib_status status;

    lib_memory_set(program, 0, sizeof(program));
    sound_program_write_ym(program, &at, 0x20u, 0x47u);
    sound_program_write_ym(program, &at, 0x28u, 0x4cu);
    sound_program_write_ym(program, &at, 0x78u, 0u);
    sound_program_write_ym(program, &at, 0x98u, 0x1fu);
    sound_program_write_ym(program, &at, 0xf8u, 0x0fu);
    sound_program_write_ym(program, &at, 0x08u, 0x40u);
    at += 2000u; /* NOPs allow the keyed operator to reach an audible level. */
    sound_program_write_ym(program, &at, 0x08u, 0u);
    program[at] = 0x76u;
    status = sound_create(program, &fixture, &sound);
    if (status == LIB_STATUS_OK) status = myraidenii_core_sound_advance(sound, 1000000u);
    if (status == LIB_STATUS_OK)
        status = myraidenii_core_sound_pcm_copy(sound, frames, 2048u, &copied, &dropped);
    for (lib_u32 index = 0u; index < copied; ++index) {
        if (index < 1024u && frames[index * 2u] != 0) sounded = LIB_TRUE;
        if (index >= 1920u && frames[index * 2u] != 0) status = LIB_STATUS_INTERNAL_ERROR;
    }
    myraidenii_core_sound_destroy(sound);
    return status != LIB_STATUS_OK || copied != 2048u || dropped != 0u || !sounded;
}

static lib_i32 sound_ym_sustain_level(void)
{
    static lib_u8 program[0x2000u];
    lib_i16 frames[4096u * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS];
    lib_i32 peaks[3] = {0};

    for (lib_u8 mode = 0u; mode < 3u; ++mode) {
        sound_fixture fixture = {0};
        myraidenii_core_sound *sound = LIB_NULL;
        lib_u32 copied = 0u;
        lib_u64 dropped = 0u;
        lib_size at = 0u;
        lib_status status;

        lib_memory_set(program, 0, sizeof(program));
        sound_program_write_ym(program, &at, 0x20u, 0x47u);
        sound_program_write_ym(program, &at, 0x28u, 0x4cu);
        sound_program_write_ym(program, &at, 0x98u, 0x1fu);
        sound_program_write_ym(program, &at, 0xb8u, 0x1fu);
        sound_program_write_ym(program, &at, 0xf8u,
            mode == 0u ? 0u : mode == 1u ? 0x80u : 0xf0u);
        sound_program_write_ym(program, &at, 0x08u, 0x40u);
        program[at] = 0x76u;
        status = sound_create(program, &fixture, &sound);
        if (status == LIB_STATUS_OK) status = myraidenii_core_sound_advance(sound, 2000000u);
        if (status == LIB_STATUS_OK)
            status = myraidenii_core_sound_pcm_copy(sound, frames, 4096u, &copied, &dropped);
        myraidenii_core_sound_destroy(sound);
        if (status != LIB_STATUS_OK || copied != 4096u || dropped == 0u) return 1;
        for (lib_u32 index = 3072u; index < copied; ++index) {
            const lib_i32 sample = frames[index * 2u];
            const lib_i32 magnitude = sample < 0 ? -sample : sample;
            if (magnitude > peaks[mode]) peaks[mode] = magnitude;
        }
    }
    return peaks[0] < 100 || peaks[1] >= peaks[0] / 5 || peaks[2] >= peaks[1] / 2;
}

static lib_i32 sound_ym_noise_channel(void)
{
    static lib_u8 program[0x2000u];
    lib_i16 reference[512u * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS];
    lib_i16 frames[512u * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS];

    for (lib_u8 mode = 0u; mode < 4u; ++mode) {
        sound_fixture fixture = {0};
        myraidenii_core_sound *sound = LIB_NULL;
        lib_u32 copied = 0u;
        lib_u64 dropped = 0u;
        lib_bool differs = LIB_FALSE;
        const lib_u8 channel = mode < 2u ? 6u : 7u;
        lib_size at = 0u;
        lib_status status;

        lib_memory_set(program, 0, sizeof(program));
        sound_program_write_ym(program, &at, 0x0fu, (mode & 1u) != 0u ? 0x9fu : 0x1fu);
        sound_program_write_ym(program, &at, (lib_u8)(0x20u + channel), 0x47u);
        sound_program_write_ym(program, &at, (lib_u8)(0x28u + channel), 0x4cu);
        sound_program_write_ym(program, &at, (lib_u8)(0x98u + channel), 0x1fu);
        sound_program_write_ym(program, &at, 0x08u, (lib_u8)(0x40u | channel));
        program[at] = 0x76u;
        status = sound_create(program, &fixture, &sound);
        if (status == LIB_STATUS_OK) status = myraidenii_core_sound_advance(sound, 250000u);
        if (status == LIB_STATUS_OK)
            status = myraidenii_core_sound_pcm_copy(sound, frames, 512u, &copied, &dropped);
        myraidenii_core_sound_destroy(sound);
        if (status != LIB_STATUS_OK || copied != 512u || dropped != 0u) return 1;
        for (lib_u32 index = 0u; index < 512u; ++index) {
            if ((mode & 1u) == 0u) reference[index * 2u] = frames[index * 2u];
            else if (reference[index * 2u] != frames[index * 2u]) differs = LIB_TRUE;
        }
        if ((mode & 1u) != 0u && differs != (channel == 7u)) return 1;
    }
    return 0;
}

static lib_i32 sound_ym_lfo_routing(void)
{
    static lib_u8 program[0x2000u];
    lib_i16 reference[2048u * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS];
    lib_i16 frames[2048u * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS];

    for (lib_u8 mode = 0u; mode < 7u; ++mode) {
        sound_fixture fixture = {0};
        myraidenii_core_sound *sound = LIB_NULL;
        lib_u32 copied = 0u;
        lib_u64 dropped = 0u;
        lib_bool differs = LIB_FALSE;
        lib_size at = 0u;
        lib_status status;

        lib_memory_set(program, 0, sizeof(program));
        sound_program_write_ym(program, &at, 0x18u, 0xffu);
        sound_program_write_ym(program, &at, 0x19u, 0x7fu);
        sound_program_write_ym(program, &at, 0x19u, 0xffu);
        sound_program_write_ym(program, &at, 0x1bu,
            mode == 4u ? 0u : mode == 5u ? 2u : mode == 6u ? 3u : 1u);
        sound_program_write_ym(program, &at, 0x20u, 0x47u);
        sound_program_write_ym(program, &at, 0x28u, 0x4cu);
        sound_program_write_ym(program, &at, 0x38u, mode == 3u ? 0x70u : mode == 0u ? 0u : 1u);
        sound_program_write_ym(program, &at, 0x98u, 0x1fu);
        sound_program_write_ym(program, &at, 0xb8u, mode == 2u || mode >= 4u ? 0x80u : 0u);
        sound_program_write_ym(program, &at, 0x08u, 0x40u);
        program[at] = 0x76u;
        status = sound_create(program, &fixture, &sound);
        if (status == LIB_STATUS_OK) status = myraidenii_core_sound_advance(sound, 1000000u);
        if (status == LIB_STATUS_OK)
            status = myraidenii_core_sound_pcm_copy(sound, frames, 2048u, &copied, &dropped);
        myraidenii_core_sound_destroy(sound);
        if (status != LIB_STATUS_OK || copied != 2048u || dropped != 0u) return 1;
        for (lib_u32 index = 0u; index < 2048u; ++index) {
            if (mode == 0u) reference[index * 2u] = frames[index * 2u];
            else if (reference[index * 2u] != frames[index * 2u]) differs = LIB_TRUE;
        }
        if (mode != 0u && differs != (mode >= 2u)) return 1;
    }
    return 0;
}

static lib_i32 sound_ym_key_scale(void)
{
    static lib_u8 program[0x2000u];
    lib_i16 frames[512u * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS];
    lib_i32 peaks[3] = {0};

    for (lib_u8 mode = 0u; mode < 3u; ++mode) {
        sound_fixture fixture = {0};
        myraidenii_core_sound *sound = LIB_NULL;
        lib_u32 copied = 0u;
        lib_u64 dropped = 0u;
        lib_size at = 0u;
        lib_status status;

        lib_memory_set(program, 0, sizeof(program));
        sound_program_write_ym(program, &at, 0x20u, 0x47u);
        sound_program_write_ym(program, &at, 0x28u, 0x7cu);
        sound_program_write_ym(program, &at, 0x98u,
            mode == 0u ? 10u : mode == 1u ? 0xcau : 0xc0u);
        sound_program_write_ym(program, &at, 0x08u, 0x40u);
        program[at] = 0x76u;
        status = sound_create(program, &fixture, &sound);
        if (status == LIB_STATUS_OK) status = myraidenii_core_sound_advance(sound, 250000u);
        if (status == LIB_STATUS_OK)
            status = myraidenii_core_sound_pcm_copy(sound, frames, 512u, &copied, &dropped);
        myraidenii_core_sound_destroy(sound);
        if (status != LIB_STATUS_OK || copied != 512u || dropped != 0u) return 1;
        for (lib_u32 index = 0u; index < copied; ++index) {
            const lib_i32 sample = frames[index * 2u];
            const lib_i32 magnitude = sample < 0 ? -sample : sample;
            if (magnitude > peaks[mode]) peaks[mode] = magnitude;
        }
    }
    return peaks[0] == 0 || peaks[1] <= peaks[0] * 2 || peaks[2] != 0;
}

static lib_i32 sound_publishes_bounded_pcm(void)
{
    static lib_u8 program[0x2000u];
    static lib_u8 samples_1[MYRAIDENII_CORE_SOUND_SAMPLE_BYTES];
    lib_i16 frames[32u * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS];
    sound_fixture fixture = {0};
    myraidenii_core_sound *sound = LIB_NULL;
    lib_u32 copied = 0u;
    lib_u64 dropped = 0u;
    lib_size at = 0u;
    lib_i32 failed = 1;
    lib_bool non_silent = LIB_FALSE;

    lib_memory_set(program, 0, sizeof(program));
    lib_memory_set(samples_1, 0, sizeof(samples_1));
    samples_1[10u] = 0x20u;
    samples_1[13u] = 0x21u;
    samples_1[0x20u] = 0xf1u;
    samples_1[0x21u] = 0x23u;
    sound_program_write_ym(program, &at, 0x20u, 0xc0u);
    program[at++] = 0x3eu; program[at++] = 0x81u;
    program[at++] = 0x32u; program[at++] = 0x00u; program[at++] = 0x60u;
    program[at++] = 0x3eu; program[at++] = 0x10u;
    program[at++] = 0x32u; program[at++] = 0x00u; program[at++] = 0x60u;
    program[at++] = 0x3eu; program[at++] = 0x08u;
    program[at++] = 0x32u; program[at++] = 0x08u; program[at++] = 0x40u;
    program[at++] = 0x3eu; program[at++] = 0x78u;
    program[at++] = 0x32u; program[at++] = 0x09u; program[at++] = 0x40u;
    program[at++] = 0x76u;
    if (sound_create_with_samples(program, samples_1, LIB_NULL, &fixture, &sound) !=
            LIB_STATUS_OK ||
        myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
        myraidenii_core_sound_pcm_copy(sound, frames, 32u, &copied, &dropped) !=
            LIB_STATUS_OK || copied == 0u || dropped != 0u)
        goto done;
    for (lib_u32 index = 0u; index < copied; ++index) {
        if (frames[index * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS] !=
            frames[index * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS + 1u])
            goto done;
        if (frames[index * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS] != 0)
            non_silent = LIB_TRUE;
    }
    if (!non_silent || myraidenii_core_sound_advance(sound,
            MYRAIDENII_CORE_SOUND_BOARD_TICKS_PER_SECOND) != LIB_STATUS_OK ||
        myraidenii_core_sound_pcm_copy(sound, frames, 32u, &copied, &dropped) !=
            LIB_STATUS_OK || copied != 32u || dropped == 0u)
        goto done;
    myraidenii_core_sound_reset(sound);
    if (myraidenii_core_sound_pcm_copy(sound, frames, 32u, &copied, &dropped) !=
            LIB_STATUS_OK || copied != 0u || dropped != 0u)
        goto done;
    failed = 0;
done:
    myraidenii_core_sound_destroy(sound);
    return failed;
}

static lib_i32 sound_validates_oki_attenuation_field(void)
{
    static lib_u8 program[0x2000u];
    static lib_u8 samples_1[MYRAIDENII_CORE_SOUND_SAMPLE_BYTES];
    sound_fixture fixture = {0};
    myraidenii_core_sound *sound = LIB_NULL;
    lib_size at = 0u;
    lib_i32 failed = 1;

    lib_memory_set(program, 0, sizeof(program));
    lib_memory_set(samples_1, 0, sizeof(samples_1));
    samples_1[10u] = 0x20u;
    samples_1[13u] = 0x21u;
    sound_program_write_oki(program, &at, 0x81u);
    sound_program_write_oki(program, &at, 0x19u);
    sound_program_read_oki_response(program, &at, 0x18u);
    sound_program_write_oki(program, &at, 0x81u);
    sound_program_write_oki(program, &at, 0x18u);
    sound_program_read_oki_response(program, &at, 0x19u);
    program[at++] = 0x76u;
    if (sound_create_with_samples(program, samples_1, LIB_NULL, &fixture, &sound) !=
            LIB_STATUS_OK ||
        myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
        fixture.response[0] != 0u || fixture.response[1] != 1u)
        goto done;
    failed = 0;
done:
    myraidenii_core_sound_destroy(sound);
    return failed;
}

static lib_i32 sound_stops_selected_oki_channel(void)
{
    static lib_u8 program[0x2000u];
    static lib_u8 samples_1[MYRAIDENII_CORE_SOUND_SAMPLE_BYTES];
    sound_fixture fixture = {0};
    myraidenii_core_sound *sound = LIB_NULL;
    lib_size at = 0u;
    lib_i32 failed = 1;

    lib_memory_set(program, 0, sizeof(program));
    lib_memory_set(samples_1, 0, sizeof(samples_1));
    samples_1[10u] = 0x20u;
    samples_1[13u] = 0x21u;
    sound_program_write_oki(program, &at, 0x81u);
    sound_program_write_oki(program, &at, 0x20u);
    sound_program_write_oki(program, &at, 0x08u);
    sound_program_read_oki_response(program, &at, 0x18u);
    sound_program_write_oki(program, &at, 0x10u);
    sound_program_read_oki_response(program, &at, 0x19u);
    sound_program_write_oki(program, &at, 0x81u);
    sound_program_write_oki(program, &at, 0x80u);
    sound_program_read_oki_response(program, &at, 0x19u);
    program[at++] = 0x76u;
    if (sound_create_with_samples(program, samples_1, LIB_NULL, &fixture, &sound) !=
            LIB_STATUS_OK ||
        myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
        fixture.response[0] != 2u || fixture.response[1] != 8u)
        goto done;
    failed = 0;
done:
    myraidenii_core_sound_destroy(sound);
    return failed;
}

static lib_i32 sound_does_not_restart_busy_oki_voice(void)
{
    static lib_u8 program[0x2000u];
    static lib_u8 samples[MYRAIDENII_CORE_SOUND_SAMPLE_BYTES];
    sound_fixture fixture = {0};
    myraidenii_core_sound_observation observation = {0};
    myraidenii_core_sound *sound = LIB_NULL;
    lib_size at = 0u;
    lib_i32 failed = 1;

    lib_memory_set(program, 0, sizeof(program));
    lib_memory_set(samples, 0, sizeof(samples));
    samples[10u] = 0x20u;
    samples[13u] = 0x7fu;
    sound_program_write_oki(program, &at, 0x81u);
    sound_program_write_oki(program, &at, 0x10u);
    sound_program_write_oki(program, &at, 0x81u);
    sound_program_write_oki(program, &at, 0x30u);
    sound_program_read_oki_response(program, &at, 0x18u);
    sound_program_write_oki(program, &at, 0x08u);
    sound_program_write_oki(program, &at, 0x81u);
    sound_program_write_oki(program, &at, 0x10u);
    sound_program_read_oki_response(program, &at, 0x19u);
    program[at] = 0x76u;
    if (sound_create_with_samples(program, samples, LIB_NULL, &fixture, &sound) !=
            LIB_STATUS_OK ||
        myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
        myraidenii_core_sound_observation_get(sound, &observation) != LIB_STATUS_OK ||
        observation.oki_voice_starts[0u] != 3u ||
        fixture.response[0] != 3u || fixture.response[1] != 3u)
        goto done;
    failed = 0;
done:
    myraidenii_core_sound_destroy(sound);
    return failed;
}

static lib_status sound_first_oki_pcm(lib_u8 first_sample_byte,
    lib_i16 *out_sample)
{
    static lib_u8 program[0x2000u];
    static lib_u8 samples_1[MYRAIDENII_CORE_SOUND_SAMPLE_BYTES];
    lib_i16 frames[32u * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS];
    sound_fixture fixture = {0};
    myraidenii_core_sound *sound = LIB_NULL;
    lib_u32 copied = 0u;
    lib_u64 dropped = 0u;
    lib_size at = 0u;
    lib_status status;

    if (out_sample == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_set(program, 0, sizeof(program));
    lib_memory_set(samples_1, 0, sizeof(samples_1));
    samples_1[10u] = 0x20u;
    samples_1[13u] = 0x2fu;
    samples_1[0x20u] = first_sample_byte;
    sound_program_write_oki(program, &at, 0x81u);
    sound_program_write_oki(program, &at, 0x10u);
    program[at] = 0x76u;
    status = sound_create_with_samples(program, samples_1, LIB_NULL, &fixture, &sound);
    if (status == LIB_STATUS_OK)
        status = myraidenii_core_sound_advance(sound, 10000u);
    if (status == LIB_STATUS_OK)
        status = myraidenii_core_sound_pcm_copy(sound, frames, 32u, &copied, &dropped);
    if (status == LIB_STATUS_OK && (copied == 0u || dropped != 0u))
        status = LIB_STATUS_INTERNAL_ERROR;
    if (status == LIB_STATUS_OK) {
        *out_sample = 0;
        for (lib_u32 index = 0u; index < copied; ++index) {
            if (frames[index * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS] != 0) {
                *out_sample = frames[index * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS];
                break;
            }
        }
        if (*out_sample == 0) status = LIB_STATUS_INTERNAL_ERROR;
    }
    myraidenii_core_sound_destroy(sound);
    return status;
}

static lib_i32 sound_decodes_oki_nibble_sign(void)
{
    lib_i16 positive = 0;
    lib_i16 negative = 0;

    return sound_first_oki_pcm(0x70u, &positive) != LIB_STATUS_OK ||
        sound_first_oki_pcm(0xf0u, &negative) != LIB_STATUS_OK ||
        positive <= 0 || negative >= 0;
}

static lib_i32 sound_executes_index_ignored_prefix(void)
{
    static lib_u8 program[0x2000u];
    sound_fixture fixture = {0};
    myraidenii_core_sound *sound = LIB_NULL;
    lib_i32 failed = 1;

    lib_memory_set(program, 0, sizeof(program));
    program[0u] = 0xddu;
    program[1u] = 0x3eu;
    program[2u] = 0x5au;
    program[3u] = 0x32u;
    program[4u] = 0x18u;
    program[5u] = 0x40u;
    program[6u] = 0x76u;
    if (sound_create_with_samples(program, LIB_NULL, LIB_NULL, &fixture, &sound) !=
            LIB_STATUS_OK ||
        myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
        fixture.response[0u] != 0x5au)
        goto done;
    failed = 0;
done:
    myraidenii_core_sound_destroy(sound);
    return failed;
}

static lib_i32 sound_executes_index_high_low_forms(void)
{
    static lib_u8 program[0x2000u];
    sound_fixture fixture = {0};
    myraidenii_core_sound *sound = LIB_NULL;
    lib_i32 failed = 1;
    lib_size at = 0u;

    lib_memory_set(program, 0, sizeof(program));
    program[at++] = 0xddu; program[at++] = 0x21u;
    program[at++] = 0u; program[at++] = 0x20u;
    program[at++] = 0x3eu; program[at++] = 0x44u;
    program[at++] = 0xddu; program[at++] = 0x67u;
    program[at++] = 0xddu; program[at++] = 0x26u; program[at++] = 0x4au;
    program[at++] = 0xddu; program[at++] = 0x2eu; program[at++] = 0x55u;
    program[at++] = 0xddu; program[at++] = 0x7du;
    program[at++] = 0xddu; program[at++] = 0x84u;
    program[at++] = 0x32u; program[at++] = 0x18u; program[at++] = 0x40u;
    program[at++] = 0xddu; program[at++] = 0x21u;
    program[at++] = 0u; program[at++] = 0x20u;
    program[at++] = 0xddu; program[at++] = 0x36u;
    program[at++] = 1u; program[at++] = 0x5au;
    program[at++] = 0xddu; program[at++] = 0x34u; program[at++] = 1u;
    program[at++] = 0xddu; program[at++] = 0x7eu; program[at++] = 1u;
    program[at++] = 0xddu; program[at++] = 0x21u;
    program[at++] = 0u; program[at++] = 0x20u;
    program[at++] = 0xddu; program[at++] = 0x29u;
    program[at++] = 0xddu; program[at++] = 0x84u;
    program[at++] = 0x32u; program[at++] = 0x19u; program[at++] = 0x40u;
    program[at] = 0x76u;
    if (sound_create_with_samples(program, LIB_NULL, LIB_NULL, &fixture, &sound) !=
            LIB_STATUS_OK ||
        myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
        fixture.response[0u] != 0x9fu || fixture.response[1u] != 0x9bu)
        goto done;
    failed = 0;
done:
    myraidenii_core_sound_destroy(sound);
    return failed;
}

static lib_i32 sound_executes_index_bit_transform(void)
{
    static lib_u8 program[0x2000u];
    sound_fixture fixture = {0};
    myraidenii_core_sound *sound = LIB_NULL;
    lib_i32 failed = 1;
    lib_size at = 0u;

    lib_memory_set(program, 0, sizeof(program));
    program[at++] = 0xddu; program[at++] = 0x21u;
    program[at++] = 0u; program[at++] = 0x20u;
    program[at++] = 0xddu; program[at++] = 0x36u;
    program[at++] = 1u; program[at++] = 0x4au;
    program[at++] = 0xddu; program[at++] = 0xcbu;
    program[at++] = 1u; program[at++] = 0x07u;
    program[at++] = 0x32u; program[at++] = 0x18u; program[at++] = 0x40u;
    program[at++] = 0xddu; program[at++] = 0xcbu;
    program[at++] = 1u; program[at++] = 0x37u;
    program[at++] = 0x32u; program[at++] = 0x19u; program[at++] = 0x40u;
    program[at] = 0x76u;
    if (sound_create_with_samples(program, LIB_NULL, LIB_NULL, &fixture, &sound) !=
            LIB_STATUS_OK ||
        myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
        fixture.response[0u] != 0x94u || fixture.response[1u] != 0x29u)
        goto done;
    failed = 0;
done:
    myraidenii_core_sound_destroy(sound);
    return failed;
}

static lib_i32 sound_executes_accumulator_carry_forms(void)
{
    static lib_u8 program[0x2000u];
    sound_fixture fixture = {0};
    myraidenii_core_sound *sound = LIB_NULL;
    lib_size at = 0u;
    lib_i32 failed = 1;

    lib_memory_set(program, 0, sizeof(program));
    program[at++] = 0x06u; program[at++] = 0u; /* LD B,0 */

    program[at++] = 0xafu; /* XOR A: preserve Z through RLCA */
    program[at++] = 0x3eu; program[at++] = 0x81u;
    program[at++] = 0x07u; /* RLCA */
    sound_program_expect_zero(program, &at);
    sound_program_expect_carry(program, &at);
    program[at++] = 0xfeu; program[at++] = 0x03u;
    sound_program_expect_zero(program, &at);

    program[at++] = 0xafu; program[at++] = 0x37u; /* XOR A / SCF */
    program[at++] = 0x3eu; program[at++] = 0x80u;
    program[at++] = 0x17u; /* RLA */
    sound_program_expect_zero(program, &at);
    sound_program_expect_carry(program, &at);
    program[at++] = 0xfeu; program[at++] = 0x01u;
    sound_program_expect_zero(program, &at);

    program[at++] = 0xafu;
    program[at++] = 0x3eu; program[at++] = 0x01u;
    program[at++] = 0x0fu; /* RRCA */
    sound_program_expect_zero(program, &at);
    sound_program_expect_carry(program, &at);
    program[at++] = 0xfeu; program[at++] = 0x80u;
    sound_program_expect_zero(program, &at);

    program[at++] = 0xafu; program[at++] = 0x37u;
    program[at++] = 0x3eu; program[at++] = 0x01u;
    program[at++] = 0x1fu; /* RRA */
    sound_program_expect_zero(program, &at);
    sound_program_expect_carry(program, &at);
    program[at++] = 0xfeu; program[at++] = 0x80u;
    sound_program_expect_zero(program, &at);

    program[at++] = 0x3eu; program[at++] = 0x09u;
    program[at++] = 0xc6u; program[at++] = 0x01u;
    program[at++] = 0x27u; /* DAA after addition */
    program[at++] = 0xfeu; program[at++] = 0x10u;
    sound_program_expect_zero(program, &at);
    program[at++] = 0x3eu; program[at++] = 0x99u;
    program[at++] = 0xc6u; program[at++] = 0x01u;
    program[at++] = 0x27u;
    sound_program_expect_carry(program, &at);
    program[at++] = 0xfeu; program[at++] = 0u;
    sound_program_expect_zero(program, &at);
    program[at++] = 0x3eu; program[at++] = 0x10u;
    program[at++] = 0xd6u; program[at++] = 0x01u;
    program[at++] = 0x27u; /* DAA after subtraction */
    program[at++] = 0xfeu; program[at++] = 0x09u;
    sound_program_expect_zero(program, &at);

    program[at++] = 0xafu; program[at++] = 0x37u; program[at++] = 0x2fu; /* CPL */
    program[at++] = 0x27u;
    sound_program_expect_carry(program, &at);
    program[at++] = 0xfeu; program[at++] = 0x99u;
    sound_program_expect_zero(program, &at);
    program[at++] = 0x37u; program[at++] = 0x3fu; /* SCF / CCF */
    program[at++] = 0x3eu; program[at++] = 0u;
    program[at++] = 0x27u;
    program[at++] = 0xfeu; program[at++] = 0x06u;
    sound_program_expect_zero(program, &at);
    program[at++] = 0xafu; program[at++] = 0x37u;
    sound_program_expect_carry(program, &at);

    program[at++] = 0x78u; /* LD A,B */
    program[at++] = 0x32u; program[at++] = 0x18u; program[at++] = 0x40u;
    program[at++] = 0x76u;
    if (sound_create(program, &fixture, &sound) != LIB_STATUS_OK ||
        myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
        fixture.response[0u] != 20u)
        goto done;
    myraidenii_core_sound_reset(sound);
    if (myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
        fixture.response[0u] != 20u)
        goto done;
    failed = 0;
done:
    myraidenii_core_sound_destroy(sound);
    return failed;
}

static lib_i32 sound_executes_extended_pair_forms(void)
{
    static lib_u8 program[0x2000u];
    const struct {
        lib_u8 opcode;
        lib_u8 pair;
        lib_u16 left;
        lib_u16 right;
        lib_u16 result;
    } arithmetic_cases[] = {
        {0x4au, 0u, 0x0001u, 0x0001u, 0x0003u},
        {0x5au, 1u, 0x1000u, 0x2000u, 0x3001u},
        {0x6au, 2u, 0x3000u, 0x3000u, 0x6001u},
        {0x7au, 3u, 0x0100u, 0x00ffu, 0x0200u},
        {0x42u, 0u, 0x1000u, 0x0001u, 0x0ffeu},
        {0x52u, 1u, 0x3000u, 0x1000u, 0x1fffu},
        {0x62u, 2u, 0x3000u, 0x3000u, 0xffffu},
        {0x72u, 3u, 0x5000u, 0x0001u, 0x4ffeu}
    };
    const lib_u16 transfer_values[] = {0x1122u, 0x3344u, 0x5566u, 0x7788u};
    sound_fixture fixture = {0};
    myraidenii_core_sound *sound = LIB_NULL;
    lib_i32 failed = 1;

    for (lib_size index = 0u; index < sizeof(arithmetic_cases) /
            sizeof(arithmetic_cases[0u]); ++index) {
        lib_size at = 0u;

        lib_memory_set(program, 0, sizeof(program));
        sound_program_load_pair(program, &at, 2u, arithmetic_cases[index].left);
        sound_program_load_pair(program, &at, arithmetic_cases[index].pair,
            arithmetic_cases[index].right);
        program[at++] = 0x37u; program[at++] = 0xedu;
        program[at++] = arithmetic_cases[index].opcode;
        sound_program_publish_hl(program, &at);
        program[at++] = 0x76u;
        if (sound_create(program, &fixture, &sound) != LIB_STATUS_OK ||
            myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
            fixture.response[0u] != (lib_u8)arithmetic_cases[index].result ||
            fixture.response[1u] != (lib_u8)(arithmetic_cases[index].result >> 8u)) {
            goto done;
        }
        myraidenii_core_sound_destroy(sound);
        sound = LIB_NULL;
    }
    for (lib_u8 pair = 0u; pair < 4u; ++pair) {
        const lib_u16 value = transfer_values[pair];
        lib_size at = 0u;

        lib_memory_set(program, 0, sizeof(program));
        program[at++] = 0x3eu; program[at++] = (lib_u8)value;
        program[at++] = 0x32u; program[at++] = 0x00u; program[at++] = 0x20u;
        program[at++] = 0x3eu; program[at++] = (lib_u8)(value >> 8u);
        program[at++] = 0x32u; program[at++] = 0x01u; program[at++] = 0x20u;
        program[at++] = 0xedu; program[at++] = (lib_u8)(0x4bu | (pair << 4u));
        program[at++] = 0x00u; program[at++] = 0x20u;
        program[at++] = 0xedu; program[at++] = (lib_u8)(0x43u | (pair << 4u));
        program[at++] = 0x02u; program[at++] = 0x20u;
        program[at++] = 0x3au; program[at++] = 0x02u; program[at++] = 0x20u;
        program[at++] = 0x32u; program[at++] = 0x18u; program[at++] = 0x40u;
        program[at++] = 0x3au; program[at++] = 0x03u; program[at++] = 0x20u;
        program[at++] = 0x32u; program[at++] = 0x19u; program[at++] = 0x40u;
        program[at++] = 0x76u;
        if (sound_create(program, &fixture, &sound) != LIB_STATUS_OK ||
            myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK)
            goto done;
        if (fixture.response[0u] != (lib_u8)value ||
            fixture.response[1u] != (lib_u8)(value >> 8u))
            goto done;
        myraidenii_core_sound_destroy(sound);
        sound = LIB_NULL;
    }
    lib_memory_set(program, 0, sizeof(program));
    {
        lib_size at = 0u;

        sound_program_load_pair(program, &at, 3u, 0x2800u);
        sound_program_load_pair(program, &at, 2u, 0x7fffu);
        sound_program_load_pair(program, &at, 0u, 0u);
        program[at++] = 0x37u; program[at++] = 0xedu; program[at++] = 0x4au;
        program[at++] = 0xf5u; program[at++] = 0xe1u;
        sound_program_publish_hl(program, &at);
        program[at] = 0x76u;
    }
    if (sound_create(program, &fixture, &sound) != LIB_STATUS_OK ||
        myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
        fixture.response[0u] != 0x94u)
        goto done;
    myraidenii_core_sound_destroy(sound);
    sound = LIB_NULL;
    lib_memory_set(program, 0, sizeof(program));
    {
        lib_size at = 0u;

        sound_program_load_pair(program, &at, 3u, 0x2800u);
        sound_program_load_pair(program, &at, 2u, 0u);
        sound_program_load_pair(program, &at, 0u, 0u);
        program[at++] = 0x37u; program[at++] = 0xedu; program[at++] = 0x42u;
        program[at++] = 0xf5u; program[at++] = 0xe1u;
        sound_program_publish_hl(program, &at);
        program[at] = 0x76u;
    }
    if (sound_create(program, &fixture, &sound) != LIB_STATUS_OK ||
        myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
        fixture.response[0u] != 0x93u)
        goto done;
    failed = 0;
done:
    myraidenii_core_sound_destroy(sound);
    return failed;
}

static lib_i32 sound_executes_extended_scalar_forms(void)
{
    static lib_u8 program[0x2000u];
    const lib_u8 neg_opcodes[] = {0x44u, 0x4cu, 0x54u, 0x5cu,
        0x64u, 0x6cu, 0x74u, 0x7cu};
    const struct {
        lib_u8 opcode;
        lib_u8 accumulator;
        lib_u8 memory;
        lib_u8 flags;
    } decimal_cases[] = {
        {0x67u, 0x32u, 0x41u, 0x01u},
        {0x6fu, 0x31u, 0x24u, 0x01u}
    };
    sound_fixture fixture = {0};
    myraidenii_core_sound *sound = LIB_NULL;
    lib_i32 failed = 1;

    for (lib_size index = 0u; index < sizeof(neg_opcodes) / sizeof(neg_opcodes[0u]);
            ++index) {
        lib_size at = 0u;

        lib_memory_set(program, 0, sizeof(program));
        sound_program_load_pair(program, &at, 3u, 0x2800u);
        program[at++] = 0x3eu; program[at++] = 0x81u;
        program[at++] = 0xedu; program[at++] = neg_opcodes[index];
        program[at++] = 0xf5u; program[at++] = 0xd1u;
        program[at++] = 0x7bu; program[at++] = 0x32u;
        program[at++] = 0x18u; program[at++] = 0x40u;
        program[at++] = 0x7au; program[at++] = 0x32u;
        program[at++] = 0x19u; program[at++] = 0x40u;
        program[at++] = 0x76u;
        if (sound_create(program, &fixture, &sound) != LIB_STATUS_OK ||
            myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
            fixture.response[0u] != 0x13u || fixture.response[1u] != 0x7fu)
            goto done;
        myraidenii_core_sound_destroy(sound);
        sound = LIB_NULL;
    }
    for (lib_size index = 0u;
            index < sizeof(decimal_cases) / sizeof(decimal_cases[0u]); ++index) {
        lib_size at = 0u;

        lib_memory_set(program, 0, sizeof(program));
        program[at++] = 0x06u; program[at++] = 0u;
        sound_program_load_pair(program, &at, 3u, 0x2800u);
        program[at++] = 0x3eu; program[at++] = 0x12u;
        program[at++] = 0x32u; program[at++] = 0x00u; program[at++] = 0x20u;
        sound_program_load_pair(program, &at, 2u, 0x2000u);
        program[at++] = 0x3eu; program[at++] = 0x34u;
        program[at++] = 0x37u; program[at++] = 0xedu;
        program[at++] = decimal_cases[index].opcode;
        program[at++] = 0xf5u; program[at++] = 0xd1u;
        program[at++] = 0x7bu; program[at++] = 0xfeu;
        program[at++] = decimal_cases[index].flags;
        sound_program_expect_zero(program, &at);
        program[at++] = 0x7au; program[at++] = 0xfeu;
        program[at++] = decimal_cases[index].accumulator;
        sound_program_expect_zero(program, &at);
        program[at++] = 0x3au; program[at++] = 0x00u; program[at++] = 0x20u;
        program[at++] = 0xfeu; program[at++] = decimal_cases[index].memory;
        sound_program_expect_zero(program, &at);
        program[at++] = 0x78u; program[at++] = 0x32u;
        program[at++] = 0x18u; program[at++] = 0x40u;
        program[at++] = 0x76u;
        if (sound_create(program, &fixture, &sound) != LIB_STATUS_OK ||
            myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
            fixture.response[0u] != 3u)
            goto done;
        myraidenii_core_sound_destroy(sound);
        sound = LIB_NULL;
    }
    failed = 0;
done:
    myraidenii_core_sound_destroy(sound);
    return failed;
}

static lib_i32 sound_classifies_unsupported_failures(void)
{
    static lib_u8 program[0x2000u];
    sound_fixture fixture = {0};
    myraidenii_core_sound *sound = LIB_NULL;
    myraidenii_core_sound_observation observation;
    lib_i32 failed = 1;

    lib_memory_set(program, 0, sizeof(program));
    program[0u] = 0xedu; program[1u] = 0x47u;
    if (sound_create(program, &fixture, &sound) != LIB_STATUS_OK ||
        myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_UNSUPPORTED ||
        myraidenii_core_sound_observation_get(sound, &observation) != LIB_STATUS_OK ||
        observation.unsupported_category != MYRAIDENII_CORE_SOUND_UNSUPPORTED_DECODER)
        goto done;
    myraidenii_core_sound_reset(sound);
    if (myraidenii_core_sound_observation_get(sound, &observation) != LIB_STATUS_OK ||
        observation.unsupported_category != MYRAIDENII_CORE_SOUND_UNSUPPORTED_NONE)
        goto done;
    myraidenii_core_sound_destroy(sound);
    sound = LIB_NULL;
    lib_memory_set(program, 0, sizeof(program));
    program[0u] = 0x3au; program[1u] = 0u; program[2u] = 0x50u;
    if (sound_create(program, &fixture, &sound) != LIB_STATUS_OK ||
        myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_UNSUPPORTED ||
        myraidenii_core_sound_observation_get(sound, &observation) != LIB_STATUS_OK ||
        observation.unsupported_category != MYRAIDENII_CORE_SOUND_UNSUPPORTED_MAP)
        goto done;
    myraidenii_core_sound_destroy(sound);
    sound = LIB_NULL;
    lib_memory_set(program, 0, sizeof(program));
    program[0u] = 0x21u; program[1u] = 0u; program[2u] = 0x50u;
    program[3u] = 0xedu; program[4u] = 0x67u;
    if (sound_create(program, &fixture, &sound) != LIB_STATUS_OK ||
        myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_UNSUPPORTED ||
        myraidenii_core_sound_observation_get(sound, &observation) != LIB_STATUS_OK ||
        observation.unsupported_category != MYRAIDENII_CORE_SOUND_UNSUPPORTED_MAP)
        goto done;
    failed = 0;
done:
    myraidenii_core_sound_destroy(sound);
    return failed;
}

int main(void)
{
    return sound_executes_fixed_transport() != 0 ||
        sound_rejects_invalid_configuration() != 0 ||
        sound_switches_fixed_bank() != 0 ||
        sound_executes_bootstrap_control_flow() != 0 ||
        sound_allows_no_device_map_slot() != 0 ||
        sound_executes_device_map() != 0 ||
        sound_classifies_oki_write_dispositions() != 0 ||
        sound_ym_timer_status_enable() != 0 ||
        sound_delivers_ym_timer_interrupt() != 0 ||
        sound_delivers_transport_interrupt() != 0 ||
        sound_interrupt_after_ei_instruction() != 0 ||
        sound_acknowledges_transport_service() != 0 ||
        sound_routes_ym_channels() != 0 ||
        sound_ym_pitch_and_level() != 0 ||
        sound_ym_modulation() != 0 ||
        sound_ym_key_off() != 0 ||
        sound_ym_sustain_level() != 0 ||
        sound_ym_noise_channel() != 0 ||
        sound_ym_lfo_routing() != 0 ||
        sound_ym_key_scale() != 0 ||
        sound_publishes_bounded_pcm() != 0 ||
        sound_validates_oki_attenuation_field() != 0 ||
        sound_stops_selected_oki_channel() != 0 ||
        sound_does_not_restart_busy_oki_voice() != 0 ||
        sound_decodes_oki_nibble_sign() != 0 ||
        sound_executes_index_ignored_prefix() != 0 ||
        sound_executes_index_high_low_forms() != 0 ||
        sound_executes_index_bit_transform() != 0 ||
        sound_executes_accumulator_carry_forms() != 0 ||
        sound_executes_extended_pair_forms() != 0 ||
        sound_executes_extended_scalar_forms() != 0 ||
        sound_classifies_unsupported_failures() != 0 ? 1 : 0;
}
