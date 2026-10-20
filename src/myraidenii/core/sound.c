#include "myraidenii/core/sound.h"

enum {
    MYRAIDENII_CORE_SOUND_FIXED_ROM_BYTES = 0x2000u,
    MYRAIDENII_CORE_SOUND_RAM_START = 0x2000u,
    MYRAIDENII_CORE_SOUND_RAM_BYTES = 0x0800u,
    MYRAIDENII_CORE_SOUND_BANK_START = 0x8000u,
    MYRAIDENII_CORE_SOUND_BANK_BYTES = 0x8000u,
    MYRAIDENII_CORE_SOUND_COMMAND_ACKNOWLEDGE = 0x4000u,
    MYRAIDENII_CORE_SOUND_INTERRUPT_ACKNOWLEDGE = 0x4001u,
    MYRAIDENII_CORE_SOUND_RST10_ACKNOWLEDGE = 0x4002u,
    MYRAIDENII_CORE_SOUND_RST18_ACKNOWLEDGE = 0x4003u,
    MYRAIDENII_CORE_SOUND_NO_DEVICE = 0x4004u,
    MYRAIDENII_CORE_SOUND_COMMAND_0 = 0x4010u,
    MYRAIDENII_CORE_SOUND_COMMAND_1 = 0x4011u,
    MYRAIDENII_CORE_SOUND_COMMAND_PENDING = 0x4012u,
    MYRAIDENII_CORE_SOUND_COIN = 0x4013u,
    MYRAIDENII_CORE_SOUND_RESPONSE_0 = 0x4018u,
    MYRAIDENII_CORE_SOUND_RESPONSE_1 = 0x4019u,
    MYRAIDENII_CORE_SOUND_BANK = 0x401au,
    MYRAIDENII_CORE_SOUND_COIN_OUTPUT = 0x401bu,
    MYRAIDENII_CORE_SOUND_YM_ADDRESS = 0x4008u,
    MYRAIDENII_CORE_SOUND_YM_DATA = 0x4009u,
    MYRAIDENII_CORE_SOUND_OKI_1 = 0x6000u,
    MYRAIDENII_CORE_SOUND_OKI_2 = 0x6002u,
    MYRAIDENII_CORE_SOUND_YM_TIMER_A_TICKS = 64u,
    MYRAIDENII_CORE_SOUND_YM_TIMER_B_TICKS = 1024u,
    MYRAIDENII_CORE_SOUND_OKI_NIBBLE_TICKS = 462u,
    MYRAIDENII_CORE_SOUND_OKI_VOICE_COUNT = 4u,
    MYRAIDENII_CORE_SOUND_OKI_VOLUME_STEPS = 9u,
    MYRAIDENII_CORE_SOUND_OKI_VOLUME_SCALE = 4096u,
    MYRAIDENII_CORE_SOUND_OKI_VOLUME_RATIO = 181u,
    MYRAIDENII_CORE_SOUND_OKI_ADPCM_MAX_STEP = 48u,
    MYRAIDENII_CORE_SOUND_PCM_TICKS_NUMERATOR =
        8u * MYRAIDENII_CORE_AUDIO_PCM_SAMPLE_RATE,
    MYRAIDENII_CORE_SOUND_MAX_IMPLEMENTED_TICKS = 23u,
    MYRAIDENII_CORE_SOUND_FLAG_CARRY = 0x01u,
    MYRAIDENII_CORE_SOUND_FLAG_SUBTRACT = 0x02u,
    MYRAIDENII_CORE_SOUND_FLAG_PARITY_OVERFLOW = 0x04u,
    MYRAIDENII_CORE_SOUND_FLAG_HALF_CARRY = 0x10u,
    MYRAIDENII_CORE_SOUND_FLAG_ZERO = 0x40u,
    MYRAIDENII_CORE_SOUND_FLAG_SIGN = 0x80u
};

typedef struct myraidenii_core_sound_ym {
    lib_u8 address;
    lib_u8 registers[256u];
    lib_u8 status;
    lib_u64 timer_a_phase;
    lib_u64 timer_b_phase;
    lib_bool channel_key_on[8u];
    lib_u32 output_phase[8u];
} myraidenii_core_sound_ym;

typedef struct myraidenii_core_sound_oki_voice {
    lib_u32 position;
    lib_u32 end;
    lib_bool high_nibble;
    lib_bool active;
    lib_u8 volume;
    lib_u8 step;
    lib_i16 signal;
} myraidenii_core_sound_oki_voice;

typedef struct myraidenii_core_sound_oki {
    lib_u8 *samples;
    lib_size samples_bytes;
    lib_u8 pending_sample;
    lib_u64 nibble_phase;
    lib_bool has_pending_sample;
    myraidenii_core_sound_oki_voice voices[MYRAIDENII_CORE_SOUND_OKI_VOICE_COUNT];
} myraidenii_core_sound_oki;

struct myraidenii_core_sound {
    myraidenii_core_sound_transport transport;
    lib_u8 *program;
    lib_size program_bytes;
    myraidenii_core_sound_ym ym;
    myraidenii_core_sound_oki oki[2u];
    lib_u64 z80_ticks;
    lib_u64 z80_phase;
    lib_u64 available_ticks;
    lib_u64 pcm_phase;
    lib_u64 pcm_dropped_frames;
    lib_u32 pcm_read;
    lib_u32 pcm_count;
    lib_i16 pcm_samples[MYRAIDENII_CORE_SOUND_PCM_FRAME_CAPACITY *
        MYRAIDENII_CORE_AUDIO_PCM_CHANNELS];
    lib_u16 pc;
    lib_u16 sp;
    lib_u8 a;
    lib_u8 b;
    lib_u8 c;
    lib_u8 d;
    lib_u8 e;
    lib_u8 h;
    lib_u8 l;
    lib_u8 f;
    lib_u8 alternate_a;
    lib_u8 alternate_b;
    lib_u8 alternate_c;
    lib_u8 alternate_d;
    lib_u8 alternate_e;
    lib_u8 alternate_h;
    lib_u8 alternate_l;
    lib_u8 alternate_f;
    lib_u16 ix;
    lib_u16 iy;
    lib_u8 bank;
    lib_u8 interrupt_mode;
    lib_u8 interrupt_enable_delay;
    lib_bool interrupt_enable;
    lib_bool halted;
    lib_u8 ram[MYRAIDENII_CORE_SOUND_RAM_BYTES];
};

static lib_bool myraidenii_core_sound_transport_valid(
    const myraidenii_core_sound_transport *transport)
{
    return transport != LIB_NULL && transport->command_get != LIB_NULL &&
        transport->command_is_pending != LIB_NULL &&
        transport->command_acknowledge != LIB_NULL &&
        transport->interrupt_acknowledge != LIB_NULL &&
        transport->response_set != LIB_NULL && transport->coin_get != LIB_NULL;
}

static lib_bool myraidenii_core_sound_image_valid(const lib_u8 *image,
    lib_size image_bytes)
{
    return (image == LIB_NULL && image_bytes == 0u) ||
        (image != LIB_NULL && image_bytes == MYRAIDENII_CORE_SOUND_SAMPLE_BYTES);
}

static lib_status myraidenii_core_sound_image_copy(const lib_u8 *source,
    lib_size source_bytes, lib_u8 **out_copy)
{
    lib_u8 *copy;

    if (out_copy == LIB_NULL || !myraidenii_core_sound_image_valid(source, source_bytes))
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_copy = LIB_NULL;
    if (source_bytes == 0u) return LIB_STATUS_OK;
    copy = lib_allocate(source_bytes);
    if (copy == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    lib_memory_copy(copy, source, source_bytes);
    *out_copy = copy;
    return LIB_STATUS_OK;
}

static lib_u32 myraidenii_core_sound_ym_timer_a_period(
    const myraidenii_core_sound_ym *ym)
{
    const lib_u16 value = (lib_u16)(((lib_u16)ym->registers[0x10u] << 2u) |
        (ym->registers[0x11u] & 3u));

    return (1024u - value) * MYRAIDENII_CORE_SOUND_YM_TIMER_A_TICKS;
}

static lib_u32 myraidenii_core_sound_ym_timer_b_period(
    const myraidenii_core_sound_ym *ym)
{
    return (256u - ym->registers[0x12u]) * MYRAIDENII_CORE_SOUND_YM_TIMER_B_TICKS;
}

static lib_bool myraidenii_core_sound_ym_irq_pending(
    const myraidenii_core_sound_ym *ym)
{
    const lib_u8 control = ym->registers[0x14u];

    return ((ym->status & 1u) != 0u && (control & 4u) != 0u) ||
        ((ym->status & 2u) != 0u && (control & 8u) != 0u);
}

static void myraidenii_core_sound_ym_advance(myraidenii_core_sound_ym *ym,
    lib_u64 ticks)
{
    const lib_u8 control = ym->registers[0x14u];
    lib_u32 period;

    if ((control & 1u) != 0u) {
        period = myraidenii_core_sound_ym_timer_a_period(ym);
        ym->timer_a_phase += ticks;
        if (ym->timer_a_phase >= period) {
            ym->timer_a_phase %= period;
            ym->status |= 1u;
        }
    }
    if ((control & 2u) != 0u) {
        period = myraidenii_core_sound_ym_timer_b_period(ym);
        ym->timer_b_phase += ticks;
        if (ym->timer_b_phase >= period) {
            ym->timer_b_phase %= period;
            ym->status |= 2u;
        }
    }
}

static lib_status myraidenii_core_sound_ym_write(myraidenii_core_sound *sound,
    lib_u16 address, lib_u8 value)
{
    myraidenii_core_sound_ym *ym = &sound->ym;

    if (address == MYRAIDENII_CORE_SOUND_YM_ADDRESS) {
        ym->address = value;
        return LIB_STATUS_OK;
    }
    if (address != MYRAIDENII_CORE_SOUND_YM_DATA) return LIB_STATUS_UNSUPPORTED;
    ym->registers[ym->address] = value;
    if (ym->address == 8u) {
        ym->channel_key_on[value & 7u] = (value & 0x78u) != 0u;
    } else if (ym->address == 0x14u) {
        if ((value & 0x10u) != 0u) ym->status &= (lib_u8)~1u;
        if ((value & 0x20u) != 0u) ym->status &= (lib_u8)~2u;
        if ((value & 1u) == 0u) ym->timer_a_phase = 0u;
        if ((value & 2u) == 0u) ym->timer_b_phase = 0u;
    }
    return LIB_STATUS_OK;
}

static lib_status myraidenii_core_sound_ym_read(const myraidenii_core_sound *sound,
    lib_u16 address, lib_u8 *out_value)
{
    if (out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (address != MYRAIDENII_CORE_SOUND_YM_ADDRESS &&
        address != MYRAIDENII_CORE_SOUND_YM_DATA)
        return LIB_STATUS_UNSUPPORTED;
    *out_value = sound->ym.status;
    return LIB_STATUS_OK;
}

static lib_status myraidenii_core_sound_oki_sample_bounds(
    const myraidenii_core_sound_oki *oki, lib_u8 sample, lib_u32 *out_start,
    lib_u32 *out_end)
{
    const lib_size table = (lib_size)sample * 8u;
    lib_u32 start;
    lib_u32 end;

    if (oki->samples == LIB_NULL || table + 5u >= oki->samples_bytes ||
        out_start == LIB_NULL || out_end == LIB_NULL)
        return LIB_STATUS_INVALID_STATE;
    start = (lib_u32)((lib_u32)oki->samples[table] << 16u |
        (lib_u32)oki->samples[table + 1u] << 8u | oki->samples[table + 2u]);
    end = (lib_u32)((lib_u32)oki->samples[table + 3u] << 16u |
        (lib_u32)oki->samples[table + 4u] << 8u | oki->samples[table + 5u]);
    if (start > end || end >= oki->samples_bytes) return LIB_STATUS_INVALID_ARGUMENT;
    *out_start = start;
    *out_end = end;
    return LIB_STATUS_OK;
}

static lib_u8 myraidenii_core_sound_oki_status(
    const myraidenii_core_sound_oki *oki)
{
    lib_u8 status = 0u;

    for (lib_u8 index = 0u; index < MYRAIDENII_CORE_SOUND_OKI_VOICE_COUNT; ++index) {
        if (oki->voices[index].active) status |= (lib_u8)(1u << index);
    }
    return status;
}

/* The MSM6295 manual gives D0--D3 as nine 3 dB attenuation steps, from
 * 0 dB through -24 dB. This local fixed-point ratio is only an L1 mapping for
 * the existing diagnostic waveform; it is not a chip decoder table. */
static lib_u32 myraidenii_core_sound_oki_volume_scale(lib_u8 volume)
{
    lib_u32 scale = MYRAIDENII_CORE_SOUND_OKI_VOLUME_SCALE;

    for (lib_u8 index = 0u; index < volume; ++index)
        scale = (scale * MYRAIDENII_CORE_SOUND_OKI_VOLUME_RATIO + 128u) / 256u;
    return scale;
}

static lib_status myraidenii_core_sound_oki_write(myraidenii_core_sound_oki *oki,
    lib_u8 value)
{
    lib_u32 start;
    lib_u32 end;
    lib_status status;

    if ((value & 0x80u) != 0u) {
        oki->pending_sample = value & 0x7fu;
        oki->has_pending_sample = LIB_TRUE;
        return LIB_STATUS_OK;
    }
    if (!oki->has_pending_sample && (value & 0x08u) != 0u) {
        for (lib_u8 index = 0u; index < MYRAIDENII_CORE_SOUND_OKI_VOICE_COUNT; ++index) {
            if ((value & (lib_u8)(1u << index)) != 0u)
                oki->voices[index].active = LIB_FALSE;
        }
        return LIB_STATUS_OK;
    }
    if (!oki->has_pending_sample) return LIB_STATUS_INVALID_STATE;
    if ((value & 0x0fu) >= MYRAIDENII_CORE_SOUND_OKI_VOLUME_STEPS) {
        oki->has_pending_sample = LIB_FALSE;
        return LIB_STATUS_OK;
    }
    status = myraidenii_core_sound_oki_sample_bounds(oki, oki->pending_sample,
        &start, &end);
    if (status != LIB_STATUS_OK) return status;
    for (lib_u8 index = 0u; index < MYRAIDENII_CORE_SOUND_OKI_VOICE_COUNT; ++index) {
        if ((value & (lib_u8)(0x10u << index)) != 0u) {
            oki->voices[index].position = start;
            oki->voices[index].end = end;
            oki->voices[index].high_nibble = LIB_TRUE;
            oki->voices[index].active = LIB_TRUE;
            oki->voices[index].volume = value & 0x0fu;
            oki->voices[index].step = 0u;
            oki->voices[index].signal = 0;
        }
    }
    oki->has_pending_sample = LIB_FALSE;
    return LIB_STATUS_OK;
}

/* This is an independently written L2 state model of the published OKI ADPCM
 * behavior. It derives its magnitude instead of retaining an imported table. */
static lib_u32 myraidenii_core_sound_oki_step_magnitude(lib_u8 step)
{
    lib_u64 magnitude = (lib_u64)16u << 32u;

    for (lib_u8 index = 0u; index < step; ++index)
        magnitude = magnitude * 11u / 10u;
    return (lib_u32)(magnitude >> 32u);
}

static void myraidenii_core_sound_oki_decode(myraidenii_core_sound_oki_voice *voice,
    lib_u8 nibble)
{
    const lib_u32 magnitude = myraidenii_core_sound_oki_step_magnitude(voice->step);
    lib_i32 difference = (lib_i32)(magnitude >> 3u);
    lib_i32 signal;
    lib_u8 low = nibble & 7u;

    if ((low & 1u) != 0u) difference += (lib_i32)(magnitude >> 2u);
    if ((low & 2u) != 0u) difference += (lib_i32)(magnitude >> 1u);
    if ((low & 4u) != 0u) difference += (lib_i32)magnitude;
    signal = voice->signal + ((nibble & 8u) != 0u ? -difference : difference);
    if (signal > 2047) signal = 2047;
    if (signal < -2048) signal = -2048;
    voice->signal = (lib_i16)signal;
    if (low < 4u) {
        if (voice->step != 0u) --voice->step;
    } else {
        voice->step = (lib_u8)(voice->step + 2u * (low - 3u));
        if (voice->step > MYRAIDENII_CORE_SOUND_OKI_ADPCM_MAX_STEP)
            voice->step = MYRAIDENII_CORE_SOUND_OKI_ADPCM_MAX_STEP;
    }
}

static void myraidenii_core_sound_oki_advance(myraidenii_core_sound_oki *oki,
    lib_u64 ticks)
{
    lib_u64 nibbles;

    oki->nibble_phase += ticks;
    nibbles = oki->nibble_phase / MYRAIDENII_CORE_SOUND_OKI_NIBBLE_TICKS;
    oki->nibble_phase %= MYRAIDENII_CORE_SOUND_OKI_NIBBLE_TICKS;
    for (lib_u8 index = 0u; index < MYRAIDENII_CORE_SOUND_OKI_VOICE_COUNT; ++index) {
        myraidenii_core_sound_oki_voice *voice = &oki->voices[index];

        if (!voice->active || nibbles == 0u) continue;
        for (lib_u64 consumed = 0u; consumed < nibbles && voice->active; ++consumed) {
            const lib_u8 byte = oki->samples[voice->position];
            const lib_u8 nibble = voice->high_nibble ? byte >> 4u : byte & 0x0fu;

            myraidenii_core_sound_oki_decode(voice, nibble);
            if (voice->high_nibble) {
                voice->high_nibble = LIB_FALSE;
            } else if (voice->position == voice->end) {
                voice->active = LIB_FALSE;
            } else {
                ++voice->position;
                voice->high_nibble = LIB_TRUE;
            }
        }
    }
}

static void myraidenii_core_sound_devices_advance(myraidenii_core_sound *sound,
    lib_u64 ticks)
{
    myraidenii_core_sound_ym_advance(&sound->ym, ticks);
    myraidenii_core_sound_oki_advance(&sound->oki[0u], ticks);
    myraidenii_core_sound_oki_advance(&sound->oki[1u], ticks);
}

static void myraidenii_core_sound_pcm_drop(myraidenii_core_sound *sound,
    lib_u64 frames)
{
    if (LIB_UINT64_MAX - sound->pcm_dropped_frames < frames)
        sound->pcm_dropped_frames = LIB_UINT64_MAX;
    else sound->pcm_dropped_frames += frames;
}

/* This first local mixer is deliberately an L1 diagnostic waveform, not a
 * reproduction of either chip's analogue path. Its sole contract is stable
 * PCM publication from the already-owned register/channel state. */
static lib_i16 myraidenii_core_sound_pcm_mix(myraidenii_core_sound *sound)
{
    lib_i32 mixed = 0;

    for (lib_u8 index = 0u; index < 8u; ++index) {
        myraidenii_core_sound_ym *ym = &sound->ym;

        if (!ym->channel_key_on[index]) continue;
        ym->output_phase[index] += 1u +
            ((lib_u32)(ym->registers[0x28u + index] & 0x7fu) << 4u);
        mixed += (ym->output_phase[index] & 0x8000u) == 0u ? 192 : -192;
    }
    for (lib_u8 device = 0u; device < 2u; ++device) {
        const myraidenii_core_sound_oki *oki = &sound->oki[device];

        for (lib_u8 index = 0u; index < MYRAIDENII_CORE_SOUND_OKI_VOICE_COUNT;
            ++index) {
            const myraidenii_core_sound_oki_voice *voice = &oki->voices[index];
            if (!voice->active || voice->position >= oki->samples_bytes) continue;
            mixed += (lib_i32)voice->signal *
                (lib_i32)myraidenii_core_sound_oki_volume_scale(voice->volume) / 8;
        }
    }
    if (mixed > 32767) return 32767;
    if (mixed < -32768) return -32768;
    return (lib_i16)mixed;
}

static void myraidenii_core_sound_pcm_enqueue(myraidenii_core_sound *sound,
    lib_i16 sample)
{
    lib_u32 write;

    if (sound->pcm_count == MYRAIDENII_CORE_SOUND_PCM_FRAME_CAPACITY) {
        sound->pcm_read = (sound->pcm_read + 1u) %
            MYRAIDENII_CORE_SOUND_PCM_FRAME_CAPACITY;
        myraidenii_core_sound_pcm_drop(sound, 1u);
    } else ++sound->pcm_count;
    write = (sound->pcm_read + sound->pcm_count - 1u) %
        MYRAIDENII_CORE_SOUND_PCM_FRAME_CAPACITY;
    sound->pcm_samples[write * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS] = sample;
    sound->pcm_samples[write * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS + 1u] = sample;
}

static void myraidenii_core_sound_pcm_skip(myraidenii_core_sound *sound,
    lib_u64 frames)
{
    for (lib_u8 index = 0u; index < 8u; ++index) {
        const lib_u32 frequency = 1u +
            ((lib_u32)(sound->ym.registers[0x28u + index] & 0x7fu) << 4u);

        sound->ym.output_phase[index] += (lib_u32)(frames * frequency);
    }
}

static lib_status myraidenii_core_sound_pcm_advance(myraidenii_core_sound *sound,
    lib_u64 z80_ticks)
{
    lib_u64 whole = z80_ticks / MYRAIDENII_CORE_SOUND_Z80_CLOCK_NUMERATOR;
    lib_u64 remainder = z80_ticks % MYRAIDENII_CORE_SOUND_Z80_CLOCK_NUMERATOR;
    lib_u64 frames;
    lib_u64 phase;

    if (whole > LIB_UINT64_MAX / MYRAIDENII_CORE_SOUND_PCM_TICKS_NUMERATOR)
        return LIB_STATUS_LIMIT_EXCEEDED;
    frames = whole * MYRAIDENII_CORE_SOUND_PCM_TICKS_NUMERATOR;
    phase = remainder * MYRAIDENII_CORE_SOUND_PCM_TICKS_NUMERATOR + sound->pcm_phase;
    if (frames > LIB_UINT64_MAX -
            phase / MYRAIDENII_CORE_SOUND_Z80_CLOCK_NUMERATOR)
        return LIB_STATUS_LIMIT_EXCEEDED;
    frames += phase / MYRAIDENII_CORE_SOUND_Z80_CLOCK_NUMERATOR;
    sound->pcm_phase = phase % MYRAIDENII_CORE_SOUND_Z80_CLOCK_NUMERATOR;
    if (frames > MYRAIDENII_CORE_SOUND_PCM_FRAME_CAPACITY) {
        myraidenii_core_sound_pcm_drop(sound, sound->pcm_count +
            frames - MYRAIDENII_CORE_SOUND_PCM_FRAME_CAPACITY);
        sound->pcm_read = 0u;
        sound->pcm_count = 0u;
        myraidenii_core_sound_pcm_skip(sound,
            frames - MYRAIDENII_CORE_SOUND_PCM_FRAME_CAPACITY);
        frames = MYRAIDENII_CORE_SOUND_PCM_FRAME_CAPACITY;
    }
    while (frames != 0u) {
        myraidenii_core_sound_pcm_enqueue(sound, myraidenii_core_sound_pcm_mix(sound));
        --frames;
    }
    return LIB_STATUS_OK;
}

static lib_status myraidenii_core_sound_time_advance(myraidenii_core_sound *sound,
    lib_u64 z80_ticks)
{
    myraidenii_core_sound_devices_advance(sound, z80_ticks);
    return myraidenii_core_sound_pcm_advance(sound, z80_ticks);
}

static lib_u16 myraidenii_core_sound_hl(const myraidenii_core_sound *sound)
{
    return (lib_u16)((lib_u16)sound->h << 8u | sound->l);
}

static void myraidenii_core_sound_hl_set(myraidenii_core_sound *sound,
    lib_u16 value)
{
    sound->h = (lib_u8)(value >> 8u);
    sound->l = (lib_u8)value;
}

static lib_bool myraidenii_core_sound_even_parity(lib_u8 value)
{
    value ^= (lib_u8)(value >> 4u);
    value ^= (lib_u8)(value >> 2u);
    value ^= (lib_u8)(value >> 1u);
    return (value & 1u) == 0u;
}

static void myraidenii_core_sound_flags_logic(myraidenii_core_sound *sound,
    lib_u8 value, lib_bool half_carry)
{
    sound->f = (value & MYRAIDENII_CORE_SOUND_FLAG_SIGN) |
        (value == 0u ? MYRAIDENII_CORE_SOUND_FLAG_ZERO : 0u) |
        (myraidenii_core_sound_even_parity(value) ?
            MYRAIDENII_CORE_SOUND_FLAG_PARITY_OVERFLOW : 0u) |
        (half_carry ? MYRAIDENII_CORE_SOUND_FLAG_HALF_CARRY : 0u);
}

static lib_u8 myraidenii_core_sound_add8(myraidenii_core_sound *sound,
    lib_u8 left, lib_u8 right, lib_u8 carry)
{
    const lib_u16 sum = (lib_u16)left + right + carry;
    const lib_u8 result = (lib_u8)sum;

    sound->f = (result & MYRAIDENII_CORE_SOUND_FLAG_SIGN) |
        (result == 0u ? MYRAIDENII_CORE_SOUND_FLAG_ZERO : 0u) |
        (((left ^ result) & (right ^ result) & 0x80u) != 0u ?
            MYRAIDENII_CORE_SOUND_FLAG_PARITY_OVERFLOW : 0u) |
        (((left & 0x0fu) + (right & 0x0fu) + carry) > 0x0fu ?
            MYRAIDENII_CORE_SOUND_FLAG_HALF_CARRY : 0u) |
        (sum > 0xffu ? MYRAIDENII_CORE_SOUND_FLAG_CARRY : 0u);
    return result;
}

static lib_u8 myraidenii_core_sound_sub8(myraidenii_core_sound *sound,
    lib_u8 left, lib_u8 right, lib_u8 carry)
{
    const lib_i16 difference = (lib_i16)left - right - carry;
    const lib_u8 result = (lib_u8)difference;

    sound->f = (result & MYRAIDENII_CORE_SOUND_FLAG_SIGN) |
        (result == 0u ? MYRAIDENII_CORE_SOUND_FLAG_ZERO : 0u) |
        (((left ^ right) & (left ^ result) & 0x80u) != 0u ?
            MYRAIDENII_CORE_SOUND_FLAG_PARITY_OVERFLOW : 0u) |
        ((lib_i16)(left & 0x0fu) - (lib_i16)(right & 0x0fu) - carry < 0 ?
            MYRAIDENII_CORE_SOUND_FLAG_HALF_CARRY : 0u) |
        MYRAIDENII_CORE_SOUND_FLAG_SUBTRACT |
        (difference < 0 ? MYRAIDENII_CORE_SOUND_FLAG_CARRY : 0u);
    return result;
}

static lib_u16 myraidenii_core_sound_add16(myraidenii_core_sound *sound,
    lib_u16 left, lib_u16 right, lib_u8 carry, lib_bool subtract)
{
    const lib_i32 result = subtract ? (lib_i32)left - right - carry :
        (lib_i32)left + right + carry;
    const lib_u16 value = (lib_u16)result;
    lib_u8 flags;

    if (!subtract) {
        flags = sound->f & (MYRAIDENII_CORE_SOUND_FLAG_SIGN |
            MYRAIDENII_CORE_SOUND_FLAG_ZERO |
            MYRAIDENII_CORE_SOUND_FLAG_PARITY_OVERFLOW);
        if ((lib_u32)(left & 0x0fffu) + (right & 0x0fffu) > 0x0fffu)
            flags |= MYRAIDENII_CORE_SOUND_FLAG_HALF_CARRY;
        if (result > 0xffff) flags |= MYRAIDENII_CORE_SOUND_FLAG_CARRY;
        sound->f = flags;
        return value;
    }
    flags = MYRAIDENII_CORE_SOUND_FLAG_SUBTRACT;
    if (value & 0x8000u) flags |= MYRAIDENII_CORE_SOUND_FLAG_SIGN;
    if (value == 0u) flags |= MYRAIDENII_CORE_SOUND_FLAG_ZERO;
    if (((left ^ right) & (left ^ value) & 0x8000u) != 0u)
        flags |= MYRAIDENII_CORE_SOUND_FLAG_PARITY_OVERFLOW;
    if ((lib_i32)(left & 0x0fffu) - (lib_i32)(right & 0x0fffu) -
        (lib_i32)carry < 0)
        flags |= MYRAIDENII_CORE_SOUND_FLAG_HALF_CARRY;
    if (result < 0) flags |= MYRAIDENII_CORE_SOUND_FLAG_CARRY;
    sound->f = flags;
    return value;
}

static void myraidenii_core_sound_accumulator_operation(
    myraidenii_core_sound *sound, lib_u8 operation, lib_u8 value)
{
    const lib_u8 carry =
        (sound->f & MYRAIDENII_CORE_SOUND_FLAG_CARRY) != 0u;

    switch (operation) {
    case 0u: sound->a = myraidenii_core_sound_add8(sound, sound->a, value, 0u); break;
    case 1u: sound->a = myraidenii_core_sound_add8(sound, sound->a, value, carry); break;
    case 2u: sound->a = myraidenii_core_sound_sub8(sound, sound->a, value, 0u); break;
    case 3u: sound->a = myraidenii_core_sound_sub8(sound, sound->a, value, carry); break;
    case 4u: sound->a &= value; myraidenii_core_sound_flags_logic(sound, sound->a, LIB_TRUE); break;
    case 5u: sound->a ^= value; myraidenii_core_sound_flags_logic(sound, sound->a, LIB_FALSE); break;
    case 6u: sound->a |= value; myraidenii_core_sound_flags_logic(sound, sound->a, LIB_FALSE); break;
    default: (void)myraidenii_core_sound_sub8(sound, sound->a, value, 0u); break;
    }
}

static lib_status myraidenii_core_sound_read(myraidenii_core_sound *sound,
    lib_u16 address, lib_u8 *out_value)
{
    lib_size index;

    if (address < MYRAIDENII_CORE_SOUND_FIXED_ROM_BYTES) {
        *out_value = address < sound->program_bytes ? sound->program[address] : 0xffu;
        return LIB_STATUS_OK;
    }
    if (address >= MYRAIDENII_CORE_SOUND_RAM_START &&
        address - MYRAIDENII_CORE_SOUND_RAM_START < MYRAIDENII_CORE_SOUND_RAM_BYTES) {
        *out_value = sound->ram[address - MYRAIDENII_CORE_SOUND_RAM_START];
        return LIB_STATUS_OK;
    }
    if (address >= MYRAIDENII_CORE_SOUND_BANK_START) {
        index = MYRAIDENII_CORE_SOUND_FIXED_ROM_BYTES +
            (lib_size)sound->bank * MYRAIDENII_CORE_SOUND_BANK_BYTES +
            (address - MYRAIDENII_CORE_SOUND_BANK_START);
        *out_value = index < sound->program_bytes ? sound->program[index] : 0xffu;
        return LIB_STATUS_OK;
    }
    if (address == MYRAIDENII_CORE_SOUND_YM_ADDRESS ||
        address == MYRAIDENII_CORE_SOUND_YM_DATA)
        return myraidenii_core_sound_ym_read(sound, address, out_value);
    if (address == MYRAIDENII_CORE_SOUND_OKI_1)
    {
        *out_value = myraidenii_core_sound_oki_status(&sound->oki[0u]);
        return LIB_STATUS_OK;
    }
    if (address == MYRAIDENII_CORE_SOUND_OKI_2) {
        *out_value = myraidenii_core_sound_oki_status(&sound->oki[1u]);
        return LIB_STATUS_OK;
    }
    if (address == MYRAIDENII_CORE_SOUND_NO_DEVICE) {
        *out_value = 0xffu;
        return LIB_STATUS_OK;
    }
    switch (address) {
    case MYRAIDENII_CORE_SOUND_COMMAND_0:
        return sound->transport.command_get(sound->transport.context, 0u, out_value);
    case MYRAIDENII_CORE_SOUND_COMMAND_1:
        return sound->transport.command_get(sound->transport.context, 1u, out_value);
    case MYRAIDENII_CORE_SOUND_COMMAND_PENDING: {
        lib_bool pending;
        lib_status status = sound->transport.command_is_pending(
            sound->transport.context, &pending);

        if (status != LIB_STATUS_OK) return status;
        *out_value = pending ? 1u : 0u;
        return LIB_STATUS_OK;
    }
    case MYRAIDENII_CORE_SOUND_COIN:
        return sound->transport.coin_get(sound->transport.context, out_value);
    default:
        return LIB_STATUS_UNSUPPORTED;
    }
}

static lib_status myraidenii_core_sound_write(myraidenii_core_sound *sound,
    lib_u16 address, lib_u8 value)
{
    if (address >= MYRAIDENII_CORE_SOUND_RAM_START &&
        address - MYRAIDENII_CORE_SOUND_RAM_START < MYRAIDENII_CORE_SOUND_RAM_BYTES) {
        sound->ram[address - MYRAIDENII_CORE_SOUND_RAM_START] = value;
        return LIB_STATUS_OK;
    }
    if (address == MYRAIDENII_CORE_SOUND_YM_ADDRESS ||
        address == MYRAIDENII_CORE_SOUND_YM_DATA)
        return myraidenii_core_sound_ym_write(sound, address, value);
    if (address == MYRAIDENII_CORE_SOUND_OKI_1)
        return myraidenii_core_sound_oki_write(&sound->oki[0u], value);
    if (address == MYRAIDENII_CORE_SOUND_OKI_2)
        return myraidenii_core_sound_oki_write(&sound->oki[1u], value);
    if (address == MYRAIDENII_CORE_SOUND_NO_DEVICE) return LIB_STATUS_OK;
    switch (address) {
    case MYRAIDENII_CORE_SOUND_COMMAND_ACKNOWLEDGE:
        return sound->transport.command_acknowledge(sound->transport.context);
    case MYRAIDENII_CORE_SOUND_INTERRUPT_ACKNOWLEDGE:
    case MYRAIDENII_CORE_SOUND_RST10_ACKNOWLEDGE:
    case MYRAIDENII_CORE_SOUND_RST18_ACKNOWLEDGE:
        return sound->transport.interrupt_acknowledge(sound->transport.context);
    case MYRAIDENII_CORE_SOUND_RESPONSE_0:
        return sound->transport.response_set(sound->transport.context, 0u, value);
    case MYRAIDENII_CORE_SOUND_RESPONSE_1:
        return sound->transport.response_set(sound->transport.context, 1u, value);
    case MYRAIDENII_CORE_SOUND_BANK:
        /* The selected L2 map gives this byte one bank-select owner. */
        sound->bank = value;
        return LIB_STATUS_OK;
    case MYRAIDENII_CORE_SOUND_COIN_OUTPUT:
        return LIB_STATUS_OK;
    default:
        return LIB_STATUS_UNSUPPORTED;
    }
}

static lib_status myraidenii_core_sound_fetch(myraidenii_core_sound *sound,
    lib_u8 *out_value)
{
    lib_status status = myraidenii_core_sound_read(sound, sound->pc, out_value);

    if (status == LIB_STATUS_OK) ++sound->pc;
    return status;
}

static lib_status myraidenii_core_sound_fetch_word(myraidenii_core_sound *sound,
    lib_u16 *out_value)
{
    lib_u8 low;
    lib_u8 high;
    lib_status status = myraidenii_core_sound_fetch(sound, &low);

    if (status != LIB_STATUS_OK) return status;
    status = myraidenii_core_sound_fetch(sound, &high);
    if (status != LIB_STATUS_OK) return status;
    *out_value = (lib_u16)((lib_u16)high << 8u | low);
    return LIB_STATUS_OK;
}

static lib_status myraidenii_core_sound_register_get(myraidenii_core_sound *sound,
    lib_u8 code, lib_u8 *out_value)
{
    switch (code) {
    case 0u: *out_value = sound->b; return LIB_STATUS_OK;
    case 1u: *out_value = sound->c; return LIB_STATUS_OK;
    case 2u: *out_value = sound->d; return LIB_STATUS_OK;
    case 3u: *out_value = sound->e; return LIB_STATUS_OK;
    case 4u: *out_value = sound->h; return LIB_STATUS_OK;
    case 5u: *out_value = sound->l; return LIB_STATUS_OK;
    case 6u: return myraidenii_core_sound_read(sound,
        myraidenii_core_sound_hl(sound), out_value);
    default: *out_value = sound->a; return LIB_STATUS_OK;
    }
}

static lib_status myraidenii_core_sound_register_set(myraidenii_core_sound *sound,
    lib_u8 code, lib_u8 value)
{
    switch (code) {
    case 0u: sound->b = value; return LIB_STATUS_OK;
    case 1u: sound->c = value; return LIB_STATUS_OK;
    case 2u: sound->d = value; return LIB_STATUS_OK;
    case 3u: sound->e = value; return LIB_STATUS_OK;
    case 4u: sound->h = value; return LIB_STATUS_OK;
    case 5u: sound->l = value; return LIB_STATUS_OK;
    case 6u: return myraidenii_core_sound_write(sound,
        myraidenii_core_sound_hl(sound), value);
    default: sound->a = value; return LIB_STATUS_OK;
    }
}

static lib_u16 myraidenii_core_sound_pair_get(const myraidenii_core_sound *sound,
    lib_u8 code)
{
    switch (code) {
    case 0u: return (lib_u16)((lib_u16)sound->b << 8u | sound->c);
    case 1u: return (lib_u16)((lib_u16)sound->d << 8u | sound->e);
    case 2u: return myraidenii_core_sound_hl(sound);
    default: return sound->sp;
    }
}

static void myraidenii_core_sound_pair_set(myraidenii_core_sound *sound,
    lib_u8 code, lib_u16 value)
{
    switch (code) {
    case 0u: sound->b = (lib_u8)(value >> 8u); sound->c = (lib_u8)value; break;
    case 1u: sound->d = (lib_u8)(value >> 8u); sound->e = (lib_u8)value; break;
    case 2u: myraidenii_core_sound_hl_set(sound, value); break;
    default: sound->sp = value; break;
    }
}

static lib_u16 myraidenii_core_sound_stack_pair_get(const myraidenii_core_sound *sound,
    lib_u8 code)
{
    if (code != 3u) return myraidenii_core_sound_pair_get(sound, code);
    return (lib_u16)((lib_u16)sound->a << 8u | sound->f);
}

static void myraidenii_core_sound_stack_pair_set(myraidenii_core_sound *sound,
    lib_u8 code, lib_u16 value)
{
    if (code != 3u) {
        myraidenii_core_sound_pair_set(sound, code, value);
        return;
    }
    sound->a = (lib_u8)(value >> 8u);
    sound->f = (lib_u8)value;
}

static lib_bool myraidenii_core_sound_condition(const myraidenii_core_sound *sound,
    lib_u8 code)
{
    const lib_bool zero = (sound->f & MYRAIDENII_CORE_SOUND_FLAG_ZERO) != 0u;
    const lib_bool carry = (sound->f & MYRAIDENII_CORE_SOUND_FLAG_CARRY) != 0u;
    const lib_bool parity =
        (sound->f & MYRAIDENII_CORE_SOUND_FLAG_PARITY_OVERFLOW) != 0u;
    const lib_bool sign = (sound->f & MYRAIDENII_CORE_SOUND_FLAG_SIGN) != 0u;

    switch (code & 7u) {
    case 0u: return !zero;
    case 1u: return zero;
    case 2u: return !carry;
    case 3u: return carry;
    case 4u: return !parity;
    case 5u: return parity;
    case 6u: return !sign;
    default: return sign;
    }
}

static lib_status myraidenii_core_sound_push(myraidenii_core_sound *sound,
    lib_u16 value)
{
    lib_status status;

    sound->sp = (lib_u16)(sound->sp - 1u);
    status = myraidenii_core_sound_write(sound, sound->sp, (lib_u8)(value >> 8u));
    if (status != LIB_STATUS_OK) return status;
    sound->sp = (lib_u16)(sound->sp - 1u);
    return myraidenii_core_sound_write(sound, sound->sp, (lib_u8)value);
}

static lib_status myraidenii_core_sound_pop(myraidenii_core_sound *sound,
    lib_u16 *out_value)
{
    lib_u8 low;
    lib_u8 high;
    lib_status status = myraidenii_core_sound_read(sound, sound->sp, &low);

    if (status != LIB_STATUS_OK) return status;
    sound->sp = (lib_u16)(sound->sp + 1u);
    status = myraidenii_core_sound_read(sound, sound->sp, &high);
    if (status != LIB_STATUS_OK) return status;
    sound->sp = (lib_u16)(sound->sp + 1u);
    *out_value = (lib_u16)((lib_u16)high << 8u | low);
    return LIB_STATUS_OK;
}

static void myraidenii_core_sound_exchange_registers(myraidenii_core_sound *sound)
{
    lib_u8 value;

    value = sound->b; sound->b = sound->alternate_b; sound->alternate_b = value;
    value = sound->c; sound->c = sound->alternate_c; sound->alternate_c = value;
    value = sound->d; sound->d = sound->alternate_d; sound->alternate_d = value;
    value = sound->e; sound->e = sound->alternate_e; sound->alternate_e = value;
    value = sound->h; sound->h = sound->alternate_h; sound->alternate_h = value;
    value = sound->l; sound->l = sound->alternate_l; sound->alternate_l = value;
}

static lib_status myraidenii_core_sound_execute_indexed(myraidenii_core_sound *sound,
    lib_u16 *index, lib_u8 *out_ticks)
{
    lib_u8 opcode;
    lib_u8 displacement;
    lib_u8 value;
    lib_u16 word;
    lib_status status = myraidenii_core_sound_fetch(sound, &opcode);

    if (status != LIB_STATUS_OK) return status;
    if (opcode == 0xcbu) {
        status = myraidenii_core_sound_fetch(sound, &displacement);
        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_sound_fetch(sound, &opcode);
        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_sound_read(sound,
            (lib_u16)(*index + (lib_i8)displacement), &value);
        if (status != LIB_STATUS_OK) return status;
        if ((opcode & 0xc0u) == 0x40u) {
            const lib_u8 mask = (lib_u8)(1u << ((opcode >> 3u) & 7u));
            const lib_bool set = (value & mask) != 0u;

            sound->f = (sound->f & MYRAIDENII_CORE_SOUND_FLAG_CARRY) |
                MYRAIDENII_CORE_SOUND_FLAG_HALF_CARRY |
                (!set ? MYRAIDENII_CORE_SOUND_FLAG_ZERO |
                    MYRAIDENII_CORE_SOUND_FLAG_PARITY_OVERFLOW : 0u) |
                (((opcode >> 3u) & 7u) == 7u && set ?
                    MYRAIDENII_CORE_SOUND_FLAG_SIGN : 0u);
            *out_ticks = 20u;
            return LIB_STATUS_OK;
        }
        if ((opcode & 0xc0u) == 0x80u || (opcode & 0xc0u) == 0xc0u) {
            const lib_u8 mask = (lib_u8)(1u << ((opcode >> 3u) & 7u));

            value = (opcode & 0xc0u) == 0x80u ?
                (lib_u8)(value & (lib_u8)~mask) : (lib_u8)(value | mask);
            status = myraidenii_core_sound_write(sound,
                (lib_u16)(*index + (lib_i8)displacement), value);
            if (status != LIB_STATUS_OK) return status;
            if ((opcode & 7u) != 6u) {
                status = myraidenii_core_sound_register_set(sound,
                    (lib_u8)(opcode & 7u), value);
                if (status != LIB_STATUS_OK) return status;
            }
            *out_ticks = 23u;
            return LIB_STATUS_OK;
        }
        return LIB_STATUS_UNSUPPORTED;
    }
    if ((opcode & 0xc7u) == 0x46u) {
        status = myraidenii_core_sound_fetch(sound, &displacement);
        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_sound_read(sound,
            (lib_u16)(*index + (lib_i8)displacement), &value);
        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_sound_register_set(sound,
            (lib_u8)((opcode >> 3u) & 7u), value);
        *out_ticks = 19u;
        return status;
    }
    if ((opcode & 0xf8u) == 0x70u && opcode != 0x76u) {
        status = myraidenii_core_sound_fetch(sound, &displacement);
        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_sound_register_get(sound, (lib_u8)(opcode & 7u), &value);
        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_sound_write(sound,
            (lib_u16)(*index + (lib_i8)displacement), value);
        *out_ticks = 19u;
        return status;
    }
    if ((opcode & 0xc7u) == 0x06u) {
        status = myraidenii_core_sound_fetch(sound, &displacement);
        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_sound_fetch(sound, &value);
        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_sound_write(sound,
            (lib_u16)(*index + (lib_i8)displacement), value);
        *out_ticks = 19u;
        return status;
    }
    if ((opcode & 0xc7u) == 0x86u) {
        const lib_u8 operation = (lib_u8)((opcode >> 3u) & 7u);

        status = myraidenii_core_sound_fetch(sound, &displacement);
        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_sound_read(sound,
            (lib_u16)(*index + (lib_i8)displacement), &value);
        if (status != LIB_STATUS_OK) return status;
        myraidenii_core_sound_accumulator_operation(sound, operation, value);
        *out_ticks = 19u;
        return LIB_STATUS_OK;
    }
    switch (opcode) {
    case 0x09u:
    case 0x19u:
    case 0x29u:
    case 0x39u:
        word = myraidenii_core_sound_pair_get(sound, (lib_u8)((opcode >> 4u) & 3u));
        *index = (lib_u16)(*index + word);
        *out_ticks = 15u;
        return LIB_STATUS_OK;
    case 0x21u:
        status = myraidenii_core_sound_fetch_word(sound, index);
        *out_ticks = 14u;
        return status;
    case 0x22u:
        status = myraidenii_core_sound_fetch_word(sound, &word);
        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_sound_write(sound, word, (lib_u8)*index);
        if (status == LIB_STATUS_OK)
            status = myraidenii_core_sound_write(sound, (lib_u16)(word + 1u),
                (lib_u8)(*index >> 8u));
        *out_ticks = 20u;
        return status;
    case 0x2au:
        status = myraidenii_core_sound_fetch_word(sound, &word);
        if (status != LIB_STATUS_OK) return status;
        {
            lib_u8 low;
            lib_u8 high;

            status = myraidenii_core_sound_read(sound, word, &low);
            if (status == LIB_STATUS_OK)
                status = myraidenii_core_sound_read(sound, (lib_u16)(word + 1u), &high);
            if (status == LIB_STATUS_OK)
                *index = (lib_u16)((lib_u16)high << 8u | low);
        }
        *out_ticks = 20u;
        return status;
    case 0x23u: ++*index; *out_ticks = 10u; return LIB_STATUS_OK;
    case 0x2bu: --*index; *out_ticks = 10u; return LIB_STATUS_OK;
    case 0xe1u:
        status = myraidenii_core_sound_pop(sound, index);
        *out_ticks = 14u;
        return status;
    case 0xe3u:
        status = myraidenii_core_sound_pop(sound, &word);
        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_sound_push(sound, *index);
        if (status == LIB_STATUS_OK) *index = word;
        *out_ticks = 23u;
        return status;
    case 0xe5u:
        status = myraidenii_core_sound_push(sound, *index);
        *out_ticks = 15u;
        return status;
    case 0xe9u: sound->pc = *index; *out_ticks = 8u; return LIB_STATUS_OK;
    case 0xf9u: sound->sp = *index; *out_ticks = 10u; return LIB_STATUS_OK;
    default: return LIB_STATUS_UNSUPPORTED;
    }
}

static lib_status myraidenii_core_sound_execute_extended(myraidenii_core_sound *sound,
    lib_u8 *out_ticks)
{
    lib_u8 opcode;
    lib_u8 low;
    lib_u8 high;
    lib_u16 word;
    lib_status status = myraidenii_core_sound_fetch(sound, &opcode);

    if (status != LIB_STATUS_OK) return status;
    switch (opcode) {
    case 0x46u:
    case 0x4eu:
    case 0x66u:
    case 0x6eu:
        sound->interrupt_mode = 0u;
        *out_ticks = 8u;
        return LIB_STATUS_OK;
    case 0x56u:
    case 0x76u:
        sound->interrupt_mode = 1u;
        *out_ticks = 8u;
        return LIB_STATUS_OK;
    case 0x5eu:
    case 0x7eu:
        sound->interrupt_mode = 2u;
        *out_ticks = 8u;
        return LIB_STATUS_OK;
    case 0x45u:
    case 0x4du:
        status = myraidenii_core_sound_pop(sound, &sound->pc);
        if (status == LIB_STATUS_OK) sound->interrupt_enable = LIB_TRUE;
        *out_ticks = 14u;
        return status;
    case 0x52u:
        myraidenii_core_sound_hl_set(sound,
            myraidenii_core_sound_add16(sound, myraidenii_core_sound_hl(sound),
                (lib_u16)((lib_u16)sound->d << 8u | sound->e),
                (sound->f & MYRAIDENII_CORE_SOUND_FLAG_CARRY) != 0u, LIB_TRUE));
        *out_ticks = 15u;
        return LIB_STATUS_OK;
    case 0x53u:
        status = myraidenii_core_sound_fetch_word(sound, &word);
        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_sound_write(sound, word, sound->e);
        if (status == LIB_STATUS_OK)
            status = myraidenii_core_sound_write(sound, (lib_u16)(word + 1u), sound->d);
        *out_ticks = 20u;
        return status;
    case 0x5bu:
        status = myraidenii_core_sound_fetch_word(sound, &word);
        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_sound_read(sound, word, &low);
        if (status == LIB_STATUS_OK)
            status = myraidenii_core_sound_read(sound, (lib_u16)(word + 1u), &high);
        if (status == LIB_STATUS_OK) {
            sound->e = low;
            sound->d = high;
        }
        *out_ticks = 20u;
        return status;
    case 0xb0u:
        status = myraidenii_core_sound_read(sound, myraidenii_core_sound_hl(sound), &low);
        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_sound_write(sound,
            (lib_u16)((lib_u16)sound->d << 8u | sound->e), low);
        if (status != LIB_STATUS_OK) return status;
        myraidenii_core_sound_hl_set(sound,
            (lib_u16)(myraidenii_core_sound_hl(sound) + 1u));
        myraidenii_core_sound_pair_set(sound, 1u,
            (lib_u16)(myraidenii_core_sound_pair_get(sound, 1u) + 1u));
        word = (lib_u16)(myraidenii_core_sound_pair_get(sound, 0u) - 1u);
        myraidenii_core_sound_pair_set(sound, 0u, word);
        if (word != 0u) {
            sound->pc = (lib_u16)(sound->pc - 2u);
            *out_ticks = 21u;
        } else {
            *out_ticks = 16u;
        }
        return LIB_STATUS_OK;
    default: return LIB_STATUS_UNSUPPORTED;
    }
}

static lib_status myraidenii_core_sound_execute_bit(myraidenii_core_sound *sound,
    lib_u8 *out_ticks)
{
    lib_u8 opcode;
    lib_u8 value;
    lib_u8 result;
    const lib_u8 carry = sound->f & MYRAIDENII_CORE_SOUND_FLAG_CARRY;
    lib_status status = myraidenii_core_sound_fetch(sound, &opcode);

    if (status != LIB_STATUS_OK) return status;
    status = myraidenii_core_sound_register_get(sound, (lib_u8)(opcode & 7u), &value);
    if (status != LIB_STATUS_OK) return status;
    if ((opcode & 0xc0u) == 0x40u) {
        const lib_u8 mask = (lib_u8)(1u << ((opcode >> 3u) & 7u));
        const lib_bool set = (value & mask) != 0u;

        sound->f = carry | MYRAIDENII_CORE_SOUND_FLAG_HALF_CARRY |
            (!set ? MYRAIDENII_CORE_SOUND_FLAG_ZERO |
                MYRAIDENII_CORE_SOUND_FLAG_PARITY_OVERFLOW : 0u) |
            (((opcode >> 3u) & 7u) == 7u && set ?
                MYRAIDENII_CORE_SOUND_FLAG_SIGN : 0u);
        *out_ticks = (opcode & 7u) == 6u ? 12u : 8u;
        return LIB_STATUS_OK;
    }
    if ((opcode & 0xc0u) == 0x80u || (opcode & 0xc0u) == 0xc0u) {
        const lib_u8 mask = (lib_u8)(1u << ((opcode >> 3u) & 7u));

        result = (opcode & 0xc0u) == 0x80u ?
            (lib_u8)(value & (lib_u8)~mask) : (lib_u8)(value | mask);
    } else {
        switch ((opcode >> 3u) & 7u) {
        case 0u: result = (lib_u8)((value << 1u) | (value >> 7u)); break;
        case 1u: result = (lib_u8)((value >> 1u) | (value << 7u)); break;
        case 2u: result = (lib_u8)((value << 1u) | (carry != 0u)); break;
        case 3u: result = (lib_u8)((value >> 1u) | (carry << 7u)); break;
        case 4u: result = (lib_u8)(value << 1u); break;
        case 5u: result = (lib_u8)((value >> 1u) | (value & 0x80u)); break;
        case 7u: result = (lib_u8)(value >> 1u); break;
        default: return LIB_STATUS_UNSUPPORTED;
        }
        myraidenii_core_sound_flags_logic(sound, result, LIB_FALSE);
        if ((value & (((opcode >> 3u) & 7u) == 1u ? 1u : 0x80u)) != 0u)
            sound->f |= MYRAIDENII_CORE_SOUND_FLAG_CARRY;
    }
    status = myraidenii_core_sound_register_set(sound, (lib_u8)(opcode & 7u), result);
    *out_ticks = (opcode & 7u) == 6u ? 15u : 8u;
    return status;
}

static lib_status myraidenii_core_sound_interrupt_accept(
    myraidenii_core_sound *sound, lib_u8 *out_ticks)
{
    lib_status status;

    if (!sound->interrupt_enable || sound->interrupt_mode != 1u ||
        !myraidenii_core_sound_ym_irq_pending(&sound->ym))
        return LIB_STATUS_UNSUPPORTED;
    sound->halted = LIB_FALSE;
    sound->interrupt_enable = LIB_FALSE;
    status = myraidenii_core_sound_push(sound, sound->pc);
    if (status == LIB_STATUS_OK) sound->pc = 0x0038u;
    *out_ticks = 13u;
    return status;
}

static lib_status myraidenii_core_sound_execute(myraidenii_core_sound *sound,
    lib_u8 *out_ticks)
{
    lib_u8 opcode;
    lib_u8 value;
    lib_u16 word;
    lib_status status;

    status = myraidenii_core_sound_interrupt_accept(sound, out_ticks);
    if (status != LIB_STATUS_UNSUPPORTED) return status;
    if (sound->interrupt_enable_delay != 0u) {
        --sound->interrupt_enable_delay;
        if (sound->interrupt_enable_delay == 0u) sound->interrupt_enable = LIB_TRUE;
    }
    if (sound->halted) {
        *out_ticks = 4u;
        return LIB_STATUS_OK;
    }
    status = myraidenii_core_sound_fetch(sound, &opcode);
    if (status != LIB_STATUS_OK) return status;
    if (opcode == 0xddu) {
        return myraidenii_core_sound_execute_indexed(sound, &sound->ix, out_ticks);
    }
    if (opcode == 0xfdu) {
        return myraidenii_core_sound_execute_indexed(sound, &sound->iy, out_ticks);
    }
    if (opcode == 0xedu) {
        return myraidenii_core_sound_execute_extended(sound, out_ticks);
    }
    if (opcode == 0xcbu) {
        return myraidenii_core_sound_execute_bit(sound, out_ticks);
    }
    if ((opcode & 0xc0u) == 0x40u) {
        if (opcode == 0x76u) {
            sound->halted = LIB_TRUE;
            *out_ticks = 4u;
            return LIB_STATUS_OK;
        }
        status = myraidenii_core_sound_register_get(sound, (lib_u8)(opcode & 7u), &value);
        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_sound_register_set(sound,
            (lib_u8)((opcode >> 3u) & 7u), value);
        *out_ticks = ((opcode & 7u) == 6u || ((opcode >> 3u) & 7u) == 6u) ? 7u : 4u;
        return status;
    }
    if ((opcode & 0xc7u) == 0x06u) {
        status = myraidenii_core_sound_fetch(sound, &value);
        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_sound_register_set(sound,
            (lib_u8)((opcode >> 3u) & 7u), value);
        *out_ticks = ((opcode >> 3u) & 7u) == 6u ? 10u : 7u;
        return status;
    }
    if ((opcode & 0xcfu) == 0x01u) {
        status = myraidenii_core_sound_fetch_word(sound, &word);
        if (status == LIB_STATUS_OK)
            myraidenii_core_sound_pair_set(sound, (lib_u8)((opcode >> 4u) & 3u), word);
        *out_ticks = 10u;
        return status;
    }
    if ((opcode & 0xcfu) == 0x03u) {
        const lib_u8 pair = (lib_u8)((opcode >> 4u) & 3u);

        myraidenii_core_sound_pair_set(sound, pair,
            (lib_u16)(myraidenii_core_sound_pair_get(sound, pair) + 1u));
        *out_ticks = 6u;
        return LIB_STATUS_OK;
    }
    if ((opcode & 0xcfu) == 0x0bu) {
        const lib_u8 pair = (lib_u8)((opcode >> 4u) & 3u);

        myraidenii_core_sound_pair_set(sound, pair,
            (lib_u16)(myraidenii_core_sound_pair_get(sound, pair) - 1u));
        *out_ticks = 6u;
        return LIB_STATUS_OK;
    }
    if ((opcode & 0xcfu) == 0x09u) {
        const lib_u16 left = myraidenii_core_sound_hl(sound);
        const lib_u16 right = myraidenii_core_sound_pair_get(sound,
            (lib_u8)((opcode >> 4u) & 3u));

        myraidenii_core_sound_hl_set(sound,
            myraidenii_core_sound_add16(sound, left, right, 0u, LIB_FALSE));
        *out_ticks = 11u;
        return LIB_STATUS_OK;
    }
    if ((opcode & 0xc7u) == 0x04u || (opcode & 0xc7u) == 0x05u) {
        const lib_u8 code = (lib_u8)((opcode >> 3u) & 7u);
        const lib_bool subtract = (opcode & 1u) != 0u;
        const lib_u8 carry = sound->f & MYRAIDENII_CORE_SOUND_FLAG_CARRY;

        status = myraidenii_core_sound_register_get(sound, code, &value);
        if (status != LIB_STATUS_OK) return status;
        value = subtract ? myraidenii_core_sound_sub8(sound, value, 1u, 0u) :
            myraidenii_core_sound_add8(sound, value, 1u, 0u);
        sound->f = (sound->f & (lib_u8)~MYRAIDENII_CORE_SOUND_FLAG_CARRY) | carry;
        status = myraidenii_core_sound_register_set(sound, code, value);
        *out_ticks = code == 6u ? 11u : 4u;
        return status;
    }
    if ((opcode & 0xc0u) == 0x80u) {
        const lib_u8 operation = (lib_u8)((opcode >> 3u) & 7u);

        status = myraidenii_core_sound_register_get(sound, (lib_u8)(opcode & 7u), &value);
        if (status != LIB_STATUS_OK) return status;
        myraidenii_core_sound_accumulator_operation(sound, operation, value);
        *out_ticks = (opcode & 7u) == 6u ? 7u : 4u;
        return LIB_STATUS_OK;
    }
    if ((opcode & 0xcfu) == 0xc1u) {
        status = myraidenii_core_sound_pop(sound, &word);
        if (status == LIB_STATUS_OK)
            myraidenii_core_sound_stack_pair_set(sound,
                (lib_u8)((opcode >> 4u) & 3u), word);
        *out_ticks = 10u;
        return status;
    }
    if ((opcode & 0xcfu) == 0xc5u) {
        status = myraidenii_core_sound_push(sound,
            myraidenii_core_sound_stack_pair_get(sound,
                (lib_u8)((opcode >> 4u) & 3u)));
        *out_ticks = 11u;
        return status;
    }
    if ((opcode & 0xc7u) == 0xc2u) {
        status = myraidenii_core_sound_fetch_word(sound, &word);
        if (status == LIB_STATUS_OK &&
            myraidenii_core_sound_condition(sound, (lib_u8)((opcode >> 3u) & 7u)))
            sound->pc = word;
        *out_ticks = 10u;
        return status;
    }
    if ((opcode & 0xc7u) == 0xc4u) {
        status = myraidenii_core_sound_fetch_word(sound, &word);
        if (status != LIB_STATUS_OK) return status;
        if (!myraidenii_core_sound_condition(sound, (lib_u8)((opcode >> 3u) & 7u))) {
            *out_ticks = 10u;
            return LIB_STATUS_OK;
        }
        status = myraidenii_core_sound_push(sound, sound->pc);
        if (status == LIB_STATUS_OK) sound->pc = word;
        *out_ticks = 17u;
        return status;
    }
    if ((opcode & 0xc7u) == 0xc6u) {
        status = myraidenii_core_sound_fetch(sound, &value);
        if (status != LIB_STATUS_OK) return status;
        myraidenii_core_sound_accumulator_operation(sound,
            (lib_u8)((opcode >> 3u) & 7u), value);
        *out_ticks = 7u;
        return LIB_STATUS_OK;
    }
    if ((opcode & 0xc7u) == 0xc7u) {
        status = myraidenii_core_sound_push(sound, sound->pc);
        if (status == LIB_STATUS_OK) sound->pc = (lib_u16)(opcode & 0x38u);
        *out_ticks = 11u;
        return status;
    }
    switch (opcode) {
    case 0x00u: *out_ticks = 4u; return LIB_STATUS_OK;
    case 0x02u:
        status = myraidenii_core_sound_write(sound,
            (lib_u16)((lib_u16)sound->b << 8u | sound->c), sound->a);
        *out_ticks = 7u;
        return status;
    case 0x08u:
        value = sound->a; sound->a = sound->alternate_a; sound->alternate_a = value;
        value = sound->f; sound->f = sound->alternate_f; sound->alternate_f = value;
        *out_ticks = 4u;
        return LIB_STATUS_OK;
    case 0x0au:
        status = myraidenii_core_sound_read(sound,
            (lib_u16)((lib_u16)sound->b << 8u | sound->c), &sound->a);
        *out_ticks = 7u;
        return status;
    case 0x17u:
        value = (sound->f & MYRAIDENII_CORE_SOUND_FLAG_CARRY) != 0u;
        sound->f = (lib_u8)((sound->a & 0x80u) != 0u ?
            MYRAIDENII_CORE_SOUND_FLAG_CARRY : 0u);
        sound->a = (lib_u8)((sound->a << 1u) | value);
        *out_ticks = 4u;
        return LIB_STATUS_OK;
    case 0x12u:
        status = myraidenii_core_sound_write(sound,
            (lib_u16)((lib_u16)sound->d << 8u | sound->e), sound->a);
        *out_ticks = 7u;
        return status;
    case 0x1au:
        status = myraidenii_core_sound_read(sound,
            (lib_u16)((lib_u16)sound->d << 8u | sound->e), &sound->a);
        *out_ticks = 7u;
        return status;
    case 0x07u:
        value = (sound->a & 0x80u) != 0u;
        sound->a = (lib_u8)((sound->a << 1u) | value);
        sound->f = (sound->f & (lib_u8)~MYRAIDENII_CORE_SOUND_FLAG_CARRY) | value;
        *out_ticks = 4u;
        return LIB_STATUS_OK;
    case 0x0fu:
        value = sound->a & 1u;
        sound->a = (lib_u8)((sound->a >> 1u) | (value << 7u));
        sound->f = (sound->f & (lib_u8)~MYRAIDENII_CORE_SOUND_FLAG_CARRY) | value;
        *out_ticks = 4u;
        return LIB_STATUS_OK;
    case 0x10u:
        status = myraidenii_core_sound_fetch(sound, &value);
        if (status != LIB_STATUS_OK) return status;
        --sound->b;
        if (sound->b != 0u) {
            sound->pc = (lib_u16)(sound->pc + (lib_i8)value);
            *out_ticks = 13u;
        } else {
            *out_ticks = 8u;
        }
        return LIB_STATUS_OK;
    case 0x01u:
        status = myraidenii_core_sound_fetch_word(sound, &word);
        sound->b = (lib_u8)(word >> 8u); sound->c = (lib_u8)word;
        *out_ticks = 10u;
        return status;
    case 0x06u:
        status = myraidenii_core_sound_fetch(sound, &sound->b);
        *out_ticks = 7u;
        return status;
    case 0x0eu:
        status = myraidenii_core_sound_fetch(sound, &sound->c);
        *out_ticks = 7u;
        return status;
    case 0x11u:
        status = myraidenii_core_sound_fetch_word(sound, &word);
        sound->d = (lib_u8)(word >> 8u); sound->e = (lib_u8)word;
        *out_ticks = 10u;
        return status;
    case 0x16u:
        status = myraidenii_core_sound_fetch(sound, &sound->d);
        *out_ticks = 7u;
        return status;
    case 0x1eu:
        status = myraidenii_core_sound_fetch(sound, &sound->e);
        *out_ticks = 7u;
        return status;
    case 0x18u:
        status = myraidenii_core_sound_fetch(sound, &value);
        if (status == LIB_STATUS_OK) sound->pc = (lib_u16)(sound->pc + (lib_i8)value);
        *out_ticks = 12u;
        return status;
    case 0x20u:
    case 0x28u:
    case 0x30u:
    case 0x38u:
        status = myraidenii_core_sound_fetch(sound, &value);
        if (status != LIB_STATUS_OK) return status;
        if (myraidenii_core_sound_condition(sound, (lib_u8)((opcode >> 3u) & 3u))) {
            sound->pc = (lib_u16)(sound->pc + (lib_i8)value);
            *out_ticks = 12u;
        } else {
            *out_ticks = 7u;
        }
        return LIB_STATUS_OK;
    case 0x21u:
        status = myraidenii_core_sound_fetch_word(sound, &word);
        if (status == LIB_STATUS_OK) myraidenii_core_sound_hl_set(sound, word);
        *out_ticks = 10u;
        return status;
    case 0x26u:
        status = myraidenii_core_sound_fetch(sound, &sound->h);
        *out_ticks = 7u;
        return status;
    case 0x2eu:
        status = myraidenii_core_sound_fetch(sound, &sound->l);
        *out_ticks = 7u;
        return status;
    case 0x31u:
        status = myraidenii_core_sound_fetch_word(sound, &sound->sp);
        *out_ticks = 10u;
        return status;
    case 0x2fu:
        sound->a = (lib_u8)~sound->a;
        sound->f |= MYRAIDENII_CORE_SOUND_FLAG_HALF_CARRY |
            MYRAIDENII_CORE_SOUND_FLAG_SUBTRACT;
        *out_ticks = 4u;
        return LIB_STATUS_OK;
    case 0x22u:
        status = myraidenii_core_sound_fetch_word(sound, &word);
        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_sound_write(sound, word, sound->l);
        if (status == LIB_STATUS_OK)
            status = myraidenii_core_sound_write(sound, (lib_u16)(word + 1u), sound->h);
        *out_ticks = 16u;
        return status;
    case 0x2au:
        status = myraidenii_core_sound_fetch_word(sound, &word);
        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_sound_read(sound, word, &sound->l);
        if (status == LIB_STATUS_OK)
            status = myraidenii_core_sound_read(sound, (lib_u16)(word + 1u), &sound->h);
        *out_ticks = 16u;
        return status;
    case 0x32u:
        status = myraidenii_core_sound_fetch_word(sound, &word);
        if (status == LIB_STATUS_OK)
            status = myraidenii_core_sound_write(sound, word, sound->a);
        *out_ticks = 13u;
        return status;
    case 0x36u:
        status = myraidenii_core_sound_fetch(sound, &value);
        if (status == LIB_STATUS_OK)
            status = myraidenii_core_sound_write(sound, myraidenii_core_sound_hl(sound), value);
        *out_ticks = 10u;
        return status;
    case 0x3au:
        status = myraidenii_core_sound_fetch_word(sound, &word);
        if (status == LIB_STATUS_OK)
            status = myraidenii_core_sound_read(sound, word, &sound->a);
        *out_ticks = 13u;
        return status;
    case 0x3eu:
        status = myraidenii_core_sound_fetch(sound, &sound->a);
        *out_ticks = 7u;
        return status;
    case 0x37u:
        sound->f = (sound->f & (lib_u8)~(MYRAIDENII_CORE_SOUND_FLAG_HALF_CARRY |
            MYRAIDENII_CORE_SOUND_FLAG_SUBTRACT | MYRAIDENII_CORE_SOUND_FLAG_CARRY)) |
            MYRAIDENII_CORE_SOUND_FLAG_CARRY;
        *out_ticks = 4u;
        return LIB_STATUS_OK;
    case 0x76u:
        sound->halted = LIB_TRUE;
        *out_ticks = 4u;
        return LIB_STATUS_OK;
    case 0xf3u:
        sound->interrupt_enable = LIB_FALSE;
        sound->interrupt_enable_delay = 0u;
        *out_ticks = 4u;
        return LIB_STATUS_OK;
    case 0xfbu:
        sound->interrupt_enable_delay = 2u;
        *out_ticks = 4u;
        return LIB_STATUS_OK;
    case 0x7eu:
        status = myraidenii_core_sound_read(sound, myraidenii_core_sound_hl(sound), &sound->a);
        *out_ticks = 7u;
        return status;
    case 0xafu:
        sound->a = 0u;
        myraidenii_core_sound_flags_logic(sound, sound->a, LIB_FALSE);
        *out_ticks = 4u;
        return LIB_STATUS_OK;
    case 0xc0u:
    case 0xc8u:
    case 0xd0u:
    case 0xd8u:
    case 0xe0u:
    case 0xe8u:
    case 0xf0u:
    case 0xf8u:
        if (!myraidenii_core_sound_condition(sound, (lib_u8)((opcode >> 3u) & 7u))) {
            *out_ticks = 5u;
            return LIB_STATUS_OK;
        }
        status = myraidenii_core_sound_pop(sound, &sound->pc);
        *out_ticks = 11u;
        return status;
    case 0xc3u:
        status = myraidenii_core_sound_fetch_word(sound, &sound->pc);
        *out_ticks = 10u;
        return status;
    case 0xc9u:
        status = myraidenii_core_sound_pop(sound, &sound->pc);
        *out_ticks = 10u;
        return status;
    case 0xcdu:
        status = myraidenii_core_sound_fetch_word(sound, &word);
        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_sound_push(sound, sound->pc);
        if (status == LIB_STATUS_OK) sound->pc = word;
        *out_ticks = 17u;
        return status;
    case 0xd9u:
        myraidenii_core_sound_exchange_registers(sound);
        *out_ticks = 4u;
        return LIB_STATUS_OK;
    case 0xe3u:
        status = myraidenii_core_sound_pop(sound, &word);
        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_sound_push(sound, myraidenii_core_sound_hl(sound));
        if (status == LIB_STATUS_OK) myraidenii_core_sound_hl_set(sound, word);
        *out_ticks = 19u;
        return status;
    case 0xebu:
        word = myraidenii_core_sound_hl(sound);
        myraidenii_core_sound_hl_set(sound,
            (lib_u16)((lib_u16)sound->d << 8u | sound->e));
        sound->d = (lib_u8)(word >> 8u);
        sound->e = (lib_u8)word;
        *out_ticks = 4u;
        return LIB_STATUS_OK;
    case 0xe9u:
        sound->pc = myraidenii_core_sound_hl(sound);
        *out_ticks = 4u;
        return LIB_STATUS_OK;
    case 0xf9u:
        sound->sp = myraidenii_core_sound_hl(sound);
        *out_ticks = 6u;
        return LIB_STATUS_OK;
    case 0xd3u:
        status = myraidenii_core_sound_fetch(sound, &value);
        if (status == LIB_STATUS_OK)
            status = myraidenii_core_sound_write(sound,
                (lib_u16)((lib_u16)sound->a << 8u | value), sound->a);
        *out_ticks = 11u;
        return status;
    case 0xdbu:
        status = myraidenii_core_sound_fetch(sound, &value);
        if (status == LIB_STATUS_OK)
            status = myraidenii_core_sound_read(sound,
                (lib_u16)((lib_u16)sound->a << 8u | value), &sound->a);
        *out_ticks = 11u;
        return status;
    default:
        return LIB_STATUS_UNSUPPORTED;
    }
}

lib_status myraidenii_core_sound_create(
    const myraidenii_core_sound_config *config, myraidenii_core_sound **out_sound)
{
    myraidenii_core_sound *sound;
    lib_status status;

    if (out_sound == LIB_NULL || config == LIB_NULL ||
        !myraidenii_core_sound_transport_valid(&config->transport) ||
        (config->program == LIB_NULL && config->program_bytes != 0u) ||
        (config->program != LIB_NULL &&
            config->program_bytes < MYRAIDENII_CORE_SOUND_FIXED_ROM_BYTES) ||
        !myraidenii_core_sound_image_valid(config->samples_1,
            config->samples_1_bytes) ||
        !myraidenii_core_sound_image_valid(config->samples_2,
            config->samples_2_bytes))
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_sound = LIB_NULL;
    sound = lib_allocate_zero(1u, sizeof(*sound));
    if (sound == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    sound->transport = config->transport;
    sound->program_bytes = config->program_bytes;
    if (sound->program_bytes != 0u) {
        sound->program = lib_allocate(sound->program_bytes);
        if (sound->program == LIB_NULL) {
            lib_release(sound);
            return LIB_STATUS_NO_MEMORY;
        }
        lib_memory_copy(sound->program, config->program, sound->program_bytes);
    }
    status = myraidenii_core_sound_image_copy(config->samples_1,
        config->samples_1_bytes, &sound->oki[0u].samples);
    if (status == LIB_STATUS_OK) {
        sound->oki[0u].samples_bytes = config->samples_1_bytes;
        status = myraidenii_core_sound_image_copy(config->samples_2,
            config->samples_2_bytes, &sound->oki[1u].samples);
        sound->oki[1u].samples_bytes = config->samples_2_bytes;
    }
    if (status != LIB_STATUS_OK) {
        lib_release(sound->oki[0u].samples);
        lib_release(sound->program);
        lib_release(sound);
        return status;
    }
    myraidenii_core_sound_reset(sound);
    *out_sound = sound;
    return LIB_STATUS_OK;
}

void myraidenii_core_sound_destroy(myraidenii_core_sound *sound)
{
    if (sound == LIB_NULL) return;
    lib_release(sound->oki[0u].samples);
    lib_release(sound->oki[1u].samples);
    lib_release(sound->program);
    lib_release(sound);
}

void myraidenii_core_sound_reset(myraidenii_core_sound *sound)
{
    if (sound != LIB_NULL) {
        sound->z80_ticks = 0u;
        sound->z80_phase = 0u;
        sound->available_ticks = 0u;
        sound->pcm_phase = 0u;
        sound->pcm_dropped_frames = 0u;
        sound->pcm_read = 0u;
        sound->pcm_count = 0u;
        sound->pc = 0u;
        sound->sp = 0xffffu;
        sound->a = 0u;
        sound->b = 0u;
        sound->c = 0u;
        sound->d = 0u;
        sound->e = 0u;
        sound->h = 0u;
        sound->l = 0u;
        sound->bank = 0u;
        sound->interrupt_mode = 0u;
        sound->interrupt_enable_delay = 0u;
        sound->interrupt_enable = LIB_FALSE;
        sound->halted = LIB_FALSE;
        lib_memory_set(sound->ram, 0, sizeof(sound->ram));
        lib_memory_set(&sound->ym, 0, sizeof(sound->ym));
        for (lib_u8 index = 0u; index < 2u; ++index) {
            sound->oki[index].pending_sample = 0u;
            sound->oki[index].nibble_phase = 0u;
            sound->oki[index].has_pending_sample = LIB_FALSE;
            lib_memory_set(sound->oki[index].voices, 0,
                sizeof(sound->oki[index].voices));
        }
    }
}

lib_status myraidenii_core_sound_advance(myraidenii_core_sound *sound,
    lib_u64 board_ticks)
{
    lib_u64 source_whole;
    lib_u64 source_remainder;
    lib_u64 delivered;
    lib_u64 phase;
    lib_status status;

    if (sound == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    source_whole = board_ticks / MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR;
    source_remainder = board_ticks % MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR;
    if (source_whole > LIB_UINT64_MAX / MYRAIDENII_CORE_SOUND_Z80_CLOCK_NUMERATOR)
        return LIB_STATUS_LIMIT_EXCEEDED;
    delivered = source_whole * MYRAIDENII_CORE_SOUND_Z80_CLOCK_NUMERATOR;
    phase = source_remainder * MYRAIDENII_CORE_SOUND_Z80_CLOCK_NUMERATOR + sound->z80_phase;
    if (delivered > LIB_UINT64_MAX -
            phase / MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR)
        return LIB_STATUS_LIMIT_EXCEEDED;
    delivered += phase / MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR;
    if (sound->z80_ticks > LIB_UINT64_MAX - delivered ||
        sound->available_ticks > LIB_UINT64_MAX - delivered)
        return LIB_STATUS_LIMIT_EXCEEDED;
    sound->z80_ticks += delivered;
    sound->available_ticks += delivered;
    sound->z80_phase = phase % MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR;
    if (sound->program == LIB_NULL) {
        return myraidenii_core_sound_time_advance(sound, delivered);
    }
    while (sound->available_ticks >= MYRAIDENII_CORE_SOUND_MAX_IMPLEMENTED_TICKS) {
        lib_u8 ticks;

        status = myraidenii_core_sound_execute(sound, &ticks);
        if (status != LIB_STATUS_OK) return status;
        sound->available_ticks -= ticks;
        status = myraidenii_core_sound_time_advance(sound, ticks);
        if (status != LIB_STATUS_OK) return status;
    }
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_sound_pcm_copy(myraidenii_core_sound *sound,
    lib_i16 *out_samples, lib_u32 out_frame_capacity, lib_u32 *out_frames,
    lib_u64 *out_dropped_frames)
{
    lib_u32 frames;

    if (sound == LIB_NULL || out_frames == LIB_NULL || out_dropped_frames == LIB_NULL ||
        (out_samples == LIB_NULL && out_frame_capacity != 0u))
        return LIB_STATUS_INVALID_ARGUMENT;
    frames = sound->pcm_count < out_frame_capacity ? sound->pcm_count :
        out_frame_capacity;
    for (lib_u32 index = 0u; index < frames; ++index) {
        const lib_u32 source = (sound->pcm_read + index) %
            MYRAIDENII_CORE_SOUND_PCM_FRAME_CAPACITY;

        out_samples[index * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS] =
            sound->pcm_samples[source * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS];
        out_samples[index * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS + 1u] =
            sound->pcm_samples[source * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS + 1u];
    }
    sound->pcm_read = (sound->pcm_read + frames) %
        MYRAIDENII_CORE_SOUND_PCM_FRAME_CAPACITY;
    sound->pcm_count -= frames;
    *out_frames = frames;
    *out_dropped_frames = sound->pcm_dropped_frames;
    sound->pcm_dropped_frames = 0u;
    return LIB_STATUS_OK;
}
