#ifndef NXVM_CORE_PC_DEVICES_H
#define NXVM_CORE_PC_DEVICES_H

#include <stdint.h>

#include "machine/core/machine.h"

typedef struct nxvm_core_pc_devices {
    uint8_t pic_pending;
    uint8_t pic_mask;
    uint8_t pic_in_service;
    uint16_t pit_reload;
    uint16_t pit_counter;
    uint64_t pit_ticks;
    uint8_t dma_registered;
    uint8_t dma_requested;
    int ports_registered;
} nxvm_core_pc_devices;

void nxvm_core_pc_devices_initialize(nxvm_core_pc_devices *devices);
void nxvm_core_pc_devices_reset(nxvm_core_pc_devices *devices);
nxvm_core_status nxvm_core_pc_devices_register(
    nxvm_core_machine *machine, nxvm_core_pc_devices *devices);
nxvm_core_status nxvm_core_pic_raise(nxvm_core_pc_devices *devices, uint8_t irq);
nxvm_core_status nxvm_core_pic_acknowledge(
    nxvm_core_pc_devices *devices, uint8_t *out_irq);
nxvm_core_status nxvm_core_pic_eoi(nxvm_core_pc_devices *devices, uint8_t irq);
nxvm_core_status nxvm_core_pit_program(nxvm_core_pc_devices *devices, uint16_t reload);
nxvm_core_status nxvm_core_pit_tick(
    nxvm_core_pc_devices *devices, uint32_t ticks, uint32_t *out_pulses);
nxvm_core_status nxvm_core_dma_register_channel(
    nxvm_core_pc_devices *devices, uint8_t channel);
nxvm_core_status nxvm_core_dma_request(
    nxvm_core_pc_devices *devices, uint8_t channel);
nxvm_core_status nxvm_core_dma_acknowledge(
    nxvm_core_pc_devices *devices, uint8_t *out_channel);

#endif
