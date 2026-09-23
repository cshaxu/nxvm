#include "core/apu.h"

#define CORE_APU_MASTER_HZ 236250000u
#define CORE_APU_SAMPLE_HZ 48000u
#define CORE_APU_CPU_DIVIDER 132u

static const lib_u8 core_apu_length_table[32] = {
    10u,254u,20u,2u,40u,4u,80u,6u,160u,8u,60u,10u,14u,12u,26u,14u,
    12u,16u,24u,18u,48u,20u,96u,22u,192u,24u,72u,26u,16u,28u,32u,30u
};

static const lib_u16 core_apu_noise_period[16] = {
    4u,8u,16u,32u,64u,96u,128u,160u,202u,254u,380u,508u,762u,1016u,2034u,4068u
};

static const lib_u16 core_apu_dmc_period[16] = {
    428u,380u,340u,320u,286u,254u,226u,214u,190u,160u,142u,128u,106u,85u,72u,54u
};

static void core_apu_emit_sample(core_apu *apu)
{
    static const lib_u8 duty[4] = { 1u,2u,4u,6u };
    lib_i32 level = 0;
    lib_u8 index;

    for (index = 0u; index < 2u; ++index) {
        lib_u8 control = apu->registers[index * 4u];
        lib_u8 volume = (control & 0x10u) != 0u ? control & 0x0fu :
            apu->envelope_decay[index];
        if ((apu->enabled & (1u << index)) != 0u && apu->length[index] != 0u &&
            apu->pulse_timer[index] >= 8u &&
            apu->pulse_phase[index] < duty[control >> 6u]) level += volume * 64;
    }
    if ((apu->enabled & 4u) != 0u && apu->length[2] != 0u &&
        apu->triangle_linear != 0u && apu->triangle_timer >= 2u)
        level += (apu->triangle_phase < 16u ? apu->triangle_phase :
            31u - apu->triangle_phase) * 32;
    if ((apu->enabled & 8u) != 0u && apu->length[3] != 0u &&
        (apu->noise_shift & 1u) == 0u) level += ((apu->registers[12] & 0x10u) != 0u ?
            apu->registers[12] & 0x0fu : apu->envelope_decay[2]) * 48;
    if ((apu->enabled & 16u) != 0u) level += apu->dmc_output * 8;
    /* The RP2A03 mixer is unipolar. PCM silence must be digital zero; the
       former arbitrary midpoint emitted a constant -24576 frame whenever all
       channels were silent. Core owns this machine-specific normalization. */
    level *= 7;
    if (level > 32767) level = 32767;
    if (level < -32768) level = -32768;
    if (apu->sample_count == CORE_APU_SAMPLE_CAPACITY) {
        ++apu->dropped_samples;
        return;
    }
    apu->samples[apu->sample_write] = level;
    apu->sample_write = (lib_u16)((apu->sample_write + 1u) % CORE_APU_SAMPLE_CAPACITY);
    ++apu->sample_count;
}

static void core_apu_clock_length(core_apu *apu)
{
    lib_u8 index;
    for (index = 0u; index < 4u; ++index)
        if ((apu->registers[index == 0u ? 0u : index == 1u ? 4u : index == 2u ? 8u : 12u] &
            0x20u) == 0u && apu->length[index] != 0u) --apu->length[index];
}

static void core_apu_clock_quarter_frame(core_apu *apu)
{
    static const lib_u8 control_register[3] = { 0u,4u,12u };
    lib_u8 index;

    for (index = 0u; index < 3u; ++index) {
        lib_u8 control = apu->registers[control_register[index]];
        if (apu->envelope_start[index]) {
            apu->envelope_start[index] = LIB_FALSE;
            apu->envelope_decay[index] = 15u;
            apu->envelope_divider[index] = control & 15u;
        } else if (apu->envelope_divider[index] != 0u) --apu->envelope_divider[index];
        else {
            apu->envelope_divider[index] = control & 15u;
            if (apu->envelope_decay[index] != 0u) --apu->envelope_decay[index];
            else if ((control & 0x20u) != 0u) apu->envelope_decay[index] = 15u;
        }
    }
    if (apu->triangle_reload) apu->triangle_linear = apu->registers[8] & 0x7fu;
    else if (apu->triangle_linear != 0u) --apu->triangle_linear;
    if ((apu->registers[8] & 0x80u) == 0u) apu->triangle_reload = LIB_FALSE;
}

static void core_apu_clock_sweep(core_apu *apu, lib_u8 index)
{
    lib_u8 control = apu->registers[index * 4u + 1u];
    lib_u16 change = (lib_u16)(apu->pulse_timer[index] >> (control & 7u));
    lib_u16 target = (lib_u16)((control & 8u) != 0u ? apu->pulse_timer[index] -
        change - (index == 0u ? 1u : 0u) : apu->pulse_timer[index] + change);

    if (apu->sweep_divider[index] == 0u && (control & 0x80u) != 0u &&
        (control & 7u) != 0u && apu->pulse_timer[index] >= 8u && target <= 0x7ffu)
        apu->pulse_timer[index] = target;
    if (apu->sweep_divider[index] == 0u || apu->sweep_reload[index]) {
        apu->sweep_divider[index] = (control >> 4u) & 7u;
        apu->sweep_reload[index] = LIB_FALSE;
    } else --apu->sweep_divider[index];
}

static void core_apu_clock_frame(core_apu *apu)
{
    ++apu->frame_cycles;
    if (apu->frame_cycles == 7457u || apu->frame_cycles == 22371u) {
        core_apu_clock_quarter_frame(apu);
        return;
    }
    if (apu->frame_cycles == 14913u) {
        core_apu_clock_quarter_frame(apu);
        core_apu_clock_length(apu);
        core_apu_clock_sweep(apu, 0u);
        core_apu_clock_sweep(apu, 1u);
        return;
    }
    if (apu->frame_mode == 0u && apu->frame_cycles == 29829u) {
        core_apu_clock_quarter_frame(apu);
        core_apu_clock_length(apu);
        core_apu_clock_sweep(apu, 0u);
        core_apu_clock_sweep(apu, 1u);
        if ((apu->registers[23] & 0x40u) == 0u) apu->frame_irq = LIB_TRUE;
        apu->frame_cycles = 0u;
    } else if (apu->frame_mode != 0u && apu->frame_cycles == 37281u) {
        core_apu_clock_quarter_frame(apu);
        core_apu_clock_length(apu);
        core_apu_clock_sweep(apu, 0u);
        core_apu_clock_sweep(apu, 1u);
        apu->frame_cycles = 0u;
    }
}

void core_apu_reset(core_apu *apu)
{
    if (apu == LIB_NULL) return;
    *apu = (core_apu) { .noise_shift = 1u, .dmc_sample_empty = LIB_TRUE };
}

void core_apu_tick(core_apu *apu)
{
    lib_u8 index;

    if (apu == LIB_NULL) return;
    core_apu_clock_frame(apu);
    apu->pulse_even_cycle = !apu->pulse_even_cycle;
    if (apu->pulse_even_cycle) for (index = 0u; index < 2u; ++index) {
        if (apu->pulse_counter[index] == 0u) {
            apu->pulse_counter[index] = apu->pulse_timer[index];
            apu->pulse_phase[index] = (lib_u8)((apu->pulse_phase[index] + 1u) & 7u);
        } else --apu->pulse_counter[index];
    }
    if (apu->triangle_counter == 0u) {
        apu->triangle_counter = apu->triangle_timer;
        if ((apu->enabled & 4u) != 0u && apu->length[2] != 0u &&
            apu->triangle_linear != 0u && apu->triangle_timer >= 2u)
            apu->triangle_phase = (lib_u8)((apu->triangle_phase + 1u) & 31u);
    } else --apu->triangle_counter;
    if (apu->noise_counter == 0u) {
        lib_u16 feedback = (lib_u16)((apu->noise_shift ^
            (apu->noise_shift >> ((apu->registers[14] & 0x80u) != 0u ? 6u : 1u))) & 1u);
        apu->noise_counter = core_apu_noise_period[apu->registers[14] & 15u];
        apu->noise_shift = (lib_u16)((apu->noise_shift >> 1u) | (feedback << 14u));
    } else --apu->noise_counter;
    if (apu->dmc_counter == 0u) {
        apu->dmc_counter = core_apu_dmc_period[apu->registers[16] & 15u];
        if (apu->dmc_bits == 0u && !apu->dmc_sample_empty) {
            apu->dmc_shift = apu->dmc_sample;
            apu->dmc_bits = 8u;
            apu->dmc_sample_empty = LIB_TRUE;
        }
        if (apu->dmc_bits != 0u) {
            if ((apu->dmc_shift & 1u) != 0u) {
                if (apu->dmc_output <= 125u) apu->dmc_output += 2u;
            } else if (apu->dmc_output >= 2u) apu->dmc_output -= 2u;
            apu->dmc_shift >>= 1u;
            --apu->dmc_bits;
        }
        if (apu->dmc_sample_empty && apu->dmc_bytes_remaining != 0u)
            apu->dmc_dma_requested = LIB_TRUE;
    } else --apu->dmc_counter;
    apu->sample_phase += CORE_APU_SAMPLE_HZ * CORE_APU_CPU_DIVIDER;
    while (apu->sample_phase >= CORE_APU_MASTER_HZ) {
        apu->sample_phase -= CORE_APU_MASTER_HZ;
        core_apu_emit_sample(apu);
    }
}

void core_apu_cpu_write(core_apu *apu, lib_u8 register_index, lib_u8 value)
{
    lib_u8 channel;

    if (apu == LIB_NULL || register_index >= 24u) return;
    apu->registers[register_index] = value;
    if (register_index < 8u) {
        channel = register_index >> 2u;
        if ((register_index & 3u) == 2u)
            apu->pulse_timer[channel] = (lib_u16)((apu->pulse_timer[channel] & 0x0700u) | value);
        else if ((register_index & 3u) == 3u) {
            apu->pulse_timer[channel] = (lib_u16)((apu->pulse_timer[channel] & 0x00ffu) |
                ((lib_u16)(value & 7u) << 8u));
            if ((apu->enabled & (1u << channel)) != 0u)
                apu->length[channel] = core_apu_length_table[value >> 3u];
            apu->pulse_phase[channel] = 0u;
            apu->envelope_start[channel] = LIB_TRUE;
        }
        if ((register_index & 3u) == 1u) apu->sweep_reload[channel] = LIB_TRUE;
    } else if (register_index == 10u || register_index == 11u) {
        apu->triangle_timer = (lib_u16)((apu->registers[10] |
            ((lib_u16)(apu->registers[11] & 7u) << 8u)));
        if (register_index == 11u && (apu->enabled & 4u) != 0u)
            apu->length[2] = core_apu_length_table[value >> 3u];
        if (register_index == 11u) apu->triangle_reload = LIB_TRUE;
    } else if (register_index == 15u && (apu->enabled & 8u) != 0u) {
        apu->length[3] = core_apu_length_table[value >> 3u];
        apu->envelope_start[2] = LIB_TRUE;
    } else if (register_index == 21u) {
        apu->enabled = value & 31u;
        for (channel = 0u; channel < 4u; ++channel)
            if ((apu->enabled & (1u << channel)) == 0u) apu->length[channel] = 0u;
        if ((apu->enabled & 16u) == 0u) apu->dmc_bytes_remaining = 0u;
        else if (apu->dmc_bytes_remaining == 0u) {
            apu->dmc_address = (lib_u16)(0xc000u + ((lib_u16)apu->registers[18] << 6u));
            apu->dmc_bytes_remaining = (lib_u16)(((lib_u16)apu->registers[19] << 4u) + 1u);
        }
        apu->dmc_irq = LIB_FALSE;
    } else if (register_index == 23u) {
        apu->frame_mode = (value >> 7u) & 1u;
        apu->frame_cycles = 0u;
        if ((value & 0x40u) != 0u) apu->frame_irq = LIB_FALSE;
        if (apu->frame_mode != 0u) core_apu_clock_length(apu);
    }
}

lib_u8 core_apu_cpu_read(core_apu *apu)
{
    lib_u8 status;

    if (apu == LIB_NULL) return 0u;
    status = (apu->length[0] != 0u ? 1u : 0u) |
        (apu->length[1] != 0u ? 2u : 0u) |
        (apu->length[2] != 0u ? 4u : 0u) |
        (apu->length[3] != 0u ? 8u : 0u) |
        (apu->dmc_bytes_remaining != 0u ? 16u : 0u) |
        (apu->frame_irq ? 0x40u : 0u) | (apu->dmc_irq ? 0x80u : 0u);
    apu->frame_irq = LIB_FALSE;
    return status;
}

lib_bool core_apu_irq_asserted(const core_apu *apu)
{
    return apu != LIB_NULL && (apu->frame_irq || apu->dmc_irq);
}

lib_bool core_apu_take_dmc_request(core_apu *apu, lib_u16 *out_address)
{
    if (apu == LIB_NULL || out_address == LIB_NULL || !apu->dmc_dma_requested) return LIB_FALSE;
    apu->dmc_dma_requested = LIB_FALSE;
    *out_address = apu->dmc_address;
    return LIB_TRUE;
}

void core_apu_complete_dmc_read(core_apu *apu, lib_u8 value)
{
    if (apu == LIB_NULL || apu->dmc_bytes_remaining == 0u) return;
    apu->dmc_sample = value;
    apu->dmc_sample_empty = LIB_FALSE;
    ++apu->dmc_address;
    if (apu->dmc_address == 0u) apu->dmc_address = 0x8000u;
    --apu->dmc_bytes_remaining;
    if (apu->dmc_bytes_remaining == 0u && (apu->registers[16] & 0x40u) != 0u) {
        apu->dmc_address = (lib_u16)(0xc000u + ((lib_u16)apu->registers[18] << 6u));
        apu->dmc_bytes_remaining = (lib_u16)(((lib_u16)apu->registers[19] << 4u) + 1u);
    } else if (apu->dmc_bytes_remaining == 0u && (apu->registers[16] & 0x80u) != 0u)
        apu->dmc_irq = LIB_TRUE;
}

lib_u16 core_apu_take_samples(core_apu *apu, lib_i32 *out_samples, lib_u16 capacity)
{
    lib_u16 count = 0u;

    if (apu == LIB_NULL || out_samples == LIB_NULL) return 0u;
    while (count < capacity && apu->sample_count != 0u) {
        out_samples[count++] = apu->samples[apu->sample_read];
        apu->sample_read = (lib_u16)((apu->sample_read + 1u) % CORE_APU_SAMPLE_CAPACITY);
        --apu->sample_count;
    }
    return count;
}
