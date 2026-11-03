#ifndef MYPC110_ES488_H
#define MYPC110_ES488_H

#include "x86-core/machine/machine_interface.h"
#include "x86-nxvm/board-base/dma_bus_interface.h"
#include "x86-nxvm/board-base/pic_bus_interface.h"

#define MYPC110_ES488_SAMPLE_CAPACITY 4096u

typedef struct mypc110_es488 {
    lib_bool reset_asserted;
    lib_u8 response[2];
    lib_u8 response_count;
    lib_u8 response_index;
    x86_core_machine *core;
    x86_nxvm_board_dma_bus *dma;
    x86_nxvm_board_dma_request_binding dma_binding;
    x86_nxvm_board_pic_irq_source *irq;
    lib_u64 reset_epoch;
    lib_u64 next_tick;
    lib_u32 period_ticks;
    lib_u32 remaining;
    lib_u16 sample_read;
    lib_u16 sample_count;
    lib_u8 samples[MYPC110_ES488_SAMPLE_CAPACITY];
    lib_u64 sample_ticks[MYPC110_ES488_SAMPLE_CAPACITY];
    lib_u16 sample_periods[MYPC110_ES488_SAMPLE_CAPACITY];
    lib_u8 command;
    lib_u8 parameter_count;
    lib_u8 parameters[2];
    lib_bool playing;
    lib_bool requested;
    lib_bool paused;
    lib_bool speaker;
} mypc110_es488;

lib_status mypc110_es488_initialize(mypc110_es488 *device,
    x86_core_machine *core, x86_nxvm_board_dma_bus *dma,
    x86_nxvm_board_pic_bus *master, x86_nxvm_board_pic_bus *slave);
void mypc110_es488_reset(mypc110_es488 *device);
lib_bool mypc110_es488_next_deadline(mypc110_es488 *device, lib_u64 *out_tick);
void mypc110_es488_advance(mypc110_es488 *device, lib_u64 tick);
lib_status mypc110_es488_install_routes(x86_core_machine *machine,
    mypc110_es488 *device);

#endif
