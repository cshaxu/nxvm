#ifndef CORE_APU_H
#define CORE_APU_H

#include "lib/types/types_interface.h"

#define CORE_APU_SAMPLE_CAPACITY 512u

typedef struct core_apu {
    lib_u8 registers[24];
    lib_u16 pulse_timer[2];
    lib_u16 pulse_counter[2];
    lib_u16 triangle_timer;
    lib_u16 triangle_counter;
    lib_u16 noise_counter;
    lib_u16 noise_shift;
    lib_u16 dmc_counter;
    lib_u16 dmc_address;
    lib_u16 dmc_bytes_remaining;
    lib_u16 length[4];
    lib_u8 pulse_phase[2];
    lib_u8 triangle_phase;
    lib_u8 envelope_decay[3];
    lib_u8 envelope_divider[3];
    lib_u8 sweep_divider[2];
    lib_u8 triangle_linear;
    lib_u8 dmc_shift;
    lib_u8 dmc_sample;
    lib_u8 dmc_bits;
    lib_u8 frame_mode;
    lib_u8 enabled;
    lib_u8 dmc_output;
    lib_u32 frame_cycles;
    lib_u32 sample_phase;
    /* Lib exposes fixed-width signed 32-bit values, not a signed-16 alias.
       Values are clamped to the s16 domain before entering this FIFO. */
    lib_i32 samples[CORE_APU_SAMPLE_CAPACITY];
    lib_u16 sample_read;
    lib_u16 sample_write;
    lib_u16 sample_count;
    lib_u64 dropped_samples;
    lib_bool frame_irq;
    lib_bool dmc_irq;
    lib_bool dmc_dma_requested;
    lib_bool envelope_start[3];
    lib_bool sweep_reload[2];
    lib_bool triangle_reload;
    lib_bool dmc_sample_empty;
    lib_bool pulse_even_cycle;
} core_apu;

void core_apu_reset(core_apu *apu);
void core_apu_tick(core_apu *apu);
void core_apu_cpu_write(core_apu *apu, lib_u8 register_index, lib_u8 value);
lib_u8 core_apu_cpu_read(core_apu *apu);
lib_bool core_apu_irq_asserted(const core_apu *apu);
lib_bool core_apu_take_dmc_request(core_apu *apu, lib_u16 *out_address);
void core_apu_complete_dmc_read(core_apu *apu, lib_u8 value);
lib_u16 core_apu_take_samples(core_apu *apu, lib_i32 *out_samples, lib_u16 capacity);

#endif
