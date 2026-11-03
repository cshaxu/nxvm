#include "app-mypc110/pc110_opl2.h"
#include "app-mypc110/pc110_clock.h"
#include "lib/types/math.h"

/* L1 continuous approximation: not the chip's quantized envelope/phase ROMs.
 * Register layout and two-operator routing follow Yamaha LSI-2138123. */
static double mypc110_opl2_operator(mypc110_opl2 *device, lib_u8 channel,
    lib_u8 op, double frequency, double modulation)
{
    static const lib_u8 offsets[9] = {0u, 1u, 2u, 8u, 9u, 10u, 16u, 17u, 18u};
    static const double multiples[16] = {
        0.5, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0,
        8.0, 9.0, 10.0, 10.0, 12.0, 12.0, 15.0, 15.0};
    static const double sustain_levels[16] = {
        1.0, 0.7079457844, 0.5011872336, 0.3548133892,
        0.2511886432, 0.1778279410, 0.1258925412, 0.0891250938,
        0.0630957344, 0.0446683592, 0.0316227766, 0.0223872114,
        0.0158489319, 0.0112201845, 0.0079432823, 0.0056234133};
    const lib_u8 slot = (lib_u8)(offsets[channel] + op * 3u);
    const lib_u8 properties = device->registers[0x20u + slot];
    const lib_u8 attenuation = device->registers[0x40u + slot] & 0x3fu;
    const lib_u8 attack = device->registers[0x60u + slot] >> 4u;
    const lib_u8 decay = device->registers[0x60u + slot] & 15u;
    const lib_u8 sustain = device->registers[0x80u + slot] >> 4u;
    const lib_u8 release = device->registers[0x80u + slot] & 15u;
    lib_u8 *stage = &device->envelope[channel][op];
    double *level = &device->level[channel][op];
    double factor = 0.9172759353897796, gain = 1.0, step, wave;
    lib_u8 remaining = attenuation;

    if (*stage == 0u) return 0.0;
    if (*stage == 1u && attack != 0u) {
        *level += (double)(attack * attack) / 400000.0;
        if (*level >= 1.0) { *level = 1.0; *stage = 2u; }
    } else if (*stage == 2u && decay != 0u) {
        *level -= (double)(decay * decay) / 4000000.0;
        if (*level <= sustain_levels[sustain]) {
            *level = sustain_levels[sustain];
            *stage = 3u;
        }
    } else if ((*stage == 3u && (properties & 0x20u) == 0u) || *stage == 4u) {
        *level -= (double)(release * release) / 1000000.0;
    }
    if (*level <= 0.0 && *stage != 1u) { *level = 0.0; *stage = 0u; return 0.0; }
    while (remaining != 0u) {
        if ((remaining & 1u) != 0u) gain *= factor;
        factor *= factor;
        remaining >>= 1u;
    }
    step = frequency * multiples[properties & 15u] / MYPC110_AUDIO_SAMPLE_RATE;
    while (step >= 1.0) step -= 1.0;
    device->phase[channel][op] += (lib_u32)(step * 4294967296.0);
    wave = lib_c_sin((double)device->phase[channel][op] *
        (6.283185307179586 / 4294967296.0) + modulation);
    if ((device->registers[1u] & 0x20u) != 0u) {
        switch (device->registers[0xe0u + slot] & 3u) {
        case 1u: if (wave < 0.0) wave = 0.0; break;
        case 2u: if (wave < 0.0) wave = -wave; break;
        case 3u:
            if ((device->phase[channel][op] & 0x80000000u) != 0u) wave = 0.0;
            else if (wave < 0.0) wave = -wave;
            break;
        default: break;
        }
    }
    return wave * *level * gain;
}

void mypc110_opl2_render(mypc110_opl2 *device, lib_i16 *samples, lib_u32 count)
{
    if (device == LIB_NULL || samples == LIB_NULL) return;
    for (lib_u32 sample = 0u; sample < count; ++sample) {
        double mixed = 0.0;
        for (lib_u8 channel = 0u; channel < 9u; ++channel) {
            const lib_u8 high = device->registers[0xb0u + channel];
            const lib_u16 f_number = (lib_u16)(device->registers[0xa0u + channel] |
                ((lib_u16)(high & 3u) << 8u));
            const lib_u8 block = (high >> 2u) & 7u;
            const lib_u8 mode = device->registers[0xc0u + channel];
            const lib_u8 feedback = (mode >> 1u) & 7u;
            const double frequency = (double)f_number * (double)(1u << block) *
                49716.0 / 1048576.0;
            const double modulator = mypc110_opl2_operator(device, channel, 0u,
                frequency, feedback == 0u ? 0.0 : device->feedback[channel] * feedback);
            const double carrier = mypc110_opl2_operator(device, channel, 1u,
                frequency, (mode & 1u) != 0u ? 0.0 : modulator * 4.0);
            device->feedback[channel] = modulator;
            mixed += carrier + ((mode & 1u) != 0u ? modulator : 0.0);
        }
        /* Allow all nine channels to play without wrapping the signed output. */
        mixed *= 2048.0;
        if (mixed > 32767.0) mixed = 32767.0;
        if (mixed < -32768.0) mixed = -32768.0;
        samples[sample] = (lib_i16)mixed;
    }
}
