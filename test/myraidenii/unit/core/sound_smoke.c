#include "myraidenii/core/sound.h"

typedef struct sound_fixture {
    lib_u8 command[2];
    lib_u8 response[2];
    lib_bool pending;
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

static lib_status sound_command_is_pending(void *context, lib_bool *out_pending)
{
    if (out_pending == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_pending = ((sound_fixture *)context)->pending;
    return LIB_STATUS_OK;
}

static lib_status sound_command_acknowledge(void *context)
{
    sound_fixture *fixture = context;

    fixture->pending = LIB_FALSE;
    ++fixture->acknowledgements;
    return LIB_STATUS_OK;
}

static lib_status sound_interrupt_acknowledge(void *context)
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
            .command_is_pending = sound_command_is_pending,
            .command_acknowledge = sound_command_acknowledge,
            .interrupt_acknowledge = sound_interrupt_acknowledge,
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

static lib_i32 sound_executes_fixed_transport(void)
{
    static lib_u8 program[0x2000u];
    sound_fixture fixture = {.command = {0x5au, 0x3cu}, .pending = LIB_TRUE};
    myraidenii_core_sound *sound = LIB_NULL;
    lib_i32 failed = 1;

    lib_memory_set(program, 0, sizeof(program));
    program[0u] = 0x3au; program[1u] = 0x10u; program[2u] = 0x40u;
    program[3u] = 0x32u; program[4u] = 0x18u; program[5u] = 0x40u;
    program[6u] = 0x3au; program[7u] = 0x12u; program[8u] = 0x40u;
    program[9u] = 0x32u; program[10u] = 0x19u; program[11u] = 0x40u;
    program[12u] = 0x3eu; program[13u] = 0u;
    program[14u] = 0x32u; program[15u] = 0x00u; program[16u] = 0x40u;
    program[17u] = 0x76u;
    if (sound_create(program, &fixture, &sound) != LIB_STATUS_OK ||
        myraidenii_core_sound_advance(sound,
            MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR) != LIB_STATUS_OK ||
        fixture.response[0] != 0x5au || fixture.response[1] != 1u || fixture.pending ||
        fixture.acknowledgements != 1u)
        goto done;
    myraidenii_core_sound_reset(sound);
    fixture.command[0] = 0x96u;
    fixture.pending = LIB_TRUE;
    if (myraidenii_core_sound_advance(sound,
            MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR) != LIB_STATUS_OK ||
        fixture.response[0] != 0x96u || fixture.response[1] != 1u || fixture.pending ||
        fixture.acknowledgements != 2u)
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
    static lib_u8 program[0xa100u];
    sound_fixture fixture = {0};
    myraidenii_core_sound *sound = LIB_NULL;
    const myraidenii_core_sound_config config = {
        .program = program,
        .program_bytes = sizeof(program),
        .transport = {
            .context = &fixture,
            .command_get = sound_command_get,
            .command_is_pending = sound_command_is_pending,
            .command_acknowledge = sound_command_acknowledge,
            .interrupt_acknowledge = sound_interrupt_acknowledge,
            .response_set = sound_response_set,
            .coin_get = sound_coin_get
        }
    };
    lib_i32 failed = 1;

    lib_memory_set(program, 0, sizeof(program));
    program[0u] = 0x3eu; program[1u] = 1u;
    program[2u] = 0x32u; program[3u] = 0x1au; program[4u] = 0x40u;
    program[5u] = 0xc3u; program[6u] = 0u; program[7u] = 0x80u;
    program[0xa000u] = 0x3eu; program[0xa001u] = 0x3cu;
    program[0xa002u] = 0x32u; program[0xa003u] = 0x19u; program[0xa004u] = 0x40u;
    program[0xa005u] = 0x76u;
    if (myraidenii_core_sound_create(&config, &sound) != LIB_STATUS_OK ||
        myraidenii_core_sound_advance(sound,
            MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR) != LIB_STATUS_OK ||
        fixture.response[1] != 0x3cu)
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

static lib_i32 sound_executes_device_map(void)
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
    program[at++] = 0x3eu; program[at++] = 0x01u;
    program[at++] = 0x32u; program[at++] = 0x09u; program[at++] = 0x40u;
    for (lib_u8 index = 0u; index < 20u; ++index) program[at++] = 0x00u;
    program[at++] = 0x3au; program[at++] = 0x08u; program[at++] = 0x40u;
    program[at++] = 0x32u; program[at++] = 0x18u; program[at++] = 0x40u;
    program[at++] = 0x3eu; program[at++] = 0x81u;
    program[at++] = 0x32u; program[at++] = 0x00u; program[at++] = 0x60u;
    program[at++] = 0x3eu; program[at++] = 0x10u;
    program[at++] = 0x32u; program[at++] = 0x00u; program[at++] = 0x60u;
    program[at++] = 0x3au; program[at++] = 0x00u; program[at++] = 0x60u;
    program[at++] = 0xe6u; program[at++] = 0x01u;
    program[at++] = 0x21u; program[at++] = 0x00u; program[at++] = 0x20u;
    program[at++] = 0x77u;
    program[at++] = 0x3eu; program[at++] = 0x08u;
    program[at++] = 0x32u; program[at++] = 0x00u; program[at++] = 0x60u;
    program[at++] = 0x3au; program[at++] = 0x00u; program[at++] = 0x60u;
    program[at++] = 0xb6u;
    program[at++] = 0x32u; program[at++] = 0x19u; program[at++] = 0x40u;
    program[at++] = 0x76u;
    if (sound_create_with_samples(program, samples_1, LIB_NULL, &fixture, &sound) !=
            LIB_STATUS_OK ||
        myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK ||
        fixture.response[0] != 1u || fixture.response[1] != 1u)
        goto done;
    failed = 0;
done:
    myraidenii_core_sound_destroy(sound);
    return failed;
}

static lib_i32 sound_delivers_ym_timer_interrupt(void)
{
    static lib_u8 program[0x2000u];
    sound_fixture fixture = {0};
    myraidenii_core_sound *sound = LIB_NULL;
    lib_size at = 0u;
    lib_i32 failed = 1;

    lib_memory_set(program, 0, sizeof(program));
    program[at++] = 0x31u; program[at++] = 0xffu; program[at++] = 0x27u;
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
    program[at++] = 0xedu; program[at++] = 0x56u;
    program[at++] = 0xfbu;
    program[at++] = 0x00u;
    program[at++] = 0x76u;
    program[at++] = 0xc3u; program[at++] = 0x25u; program[at++] = 0x00u;
    program[0x38u] = 0x3eu; program[0x39u] = 0x14u;
    program[0x3au] = 0x32u; program[0x3bu] = 0x08u; program[0x3cu] = 0x40u;
    program[0x3du] = 0x3eu; program[0x3eu] = 0x10u;
    program[0x3fu] = 0x32u; program[0x40u] = 0x09u; program[0x41u] = 0x40u;
    program[0x42u] = 0x3eu; program[0x43u] = 0x5au;
    program[0x44u] = 0x32u; program[0x45u] = 0x18u; program[0x46u] = 0x40u;
    program[0x47u] = 0xedu; program[0x48u] = 0x4du;
    if (sound_create(program, &fixture, &sound) != LIB_STATUS_OK ||
        myraidenii_core_sound_advance(sound, 10000u) != LIB_STATUS_OK)
        goto done;
    if (fixture.response[0] != 0x5au) {
        failed = 4;
        goto done;
    }
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
        sound_executes_device_map() != 0 ||
        sound_delivers_ym_timer_interrupt() != 0 ? 1 : 0;
}
