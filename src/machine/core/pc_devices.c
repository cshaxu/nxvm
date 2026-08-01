#include "machine/core/pc_devices.h"

#include <string.h>

static int first_set(uint8_t value)
{
    int bit;
    for (bit = 0; bit < 8; ++bit) {
        if ((value & (uint8_t)(1u << bit)) != 0u) return bit;
    }
    return -1;
}

void nxvm_core_pc_devices_initialize(nxvm_core_pc_devices *devices)
{
    if (devices != NULL) {
        memset(devices, 0, sizeof(*devices));
        devices->pit_reload = 1u;
        devices->pit_counter = 1u;
    }
}

void nxvm_core_pc_devices_reset(nxvm_core_pc_devices *devices)
{
    int registered;
    if (devices == NULL) return;
    registered = devices->ports_registered;
    nxvm_core_pc_devices_initialize(devices);
    devices->ports_registered = registered;
}

nxvm_core_status nxvm_core_pc_devices_register(
    nxvm_core_machine *machine, nxvm_core_pc_devices *devices)
{
    if (machine == NULL || devices == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    if (devices->ports_registered) return NXVM_CORE_STATUS_INVALID_STATE;
    devices->ports_registered = 1;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_core_pic_raise(nxvm_core_pc_devices *devices, uint8_t irq)
{
    if (devices == NULL || irq >= 8u) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    devices->pic_pending |= (uint8_t)(1u << irq);
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_core_pic_acknowledge(nxvm_core_pc_devices *devices, uint8_t *out_irq)
{
    int irq;
    if (devices == NULL || out_irq == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    irq = first_set((uint8_t)(devices->pic_pending & (uint8_t)~devices->pic_mask));
    if (irq < 0) return NXVM_CORE_STATUS_UNSUPPORTED;
    devices->pic_pending &= (uint8_t)~(1u << irq);
    devices->pic_in_service |= (uint8_t)(1u << irq);
    *out_irq = (uint8_t)irq;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_core_pic_eoi(nxvm_core_pc_devices *devices, uint8_t irq)
{
    if (devices == NULL || irq >= 8u) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    devices->pic_in_service &= (uint8_t)~(1u << irq);
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_core_pit_program(nxvm_core_pc_devices *devices, uint16_t reload)
{
    if (devices == NULL || reload == 0u) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    devices->pit_reload = reload;
    devices->pit_counter = reload;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_core_pit_tick(
    nxvm_core_pc_devices *devices, uint32_t ticks, uint32_t *out_pulses)
{
    uint32_t pulses = 0u;
    if (devices == NULL || out_pulses == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    while (ticks-- > 0u) {
        ++devices->pit_ticks;
        if (--devices->pit_counter == 0u) {
            devices->pit_counter = devices->pit_reload;
            ++pulses;
            (void)nxvm_core_pic_raise(devices, 0u);
        }
    }
    *out_pulses = pulses;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_core_dma_register_channel(
    nxvm_core_pc_devices *devices, uint8_t channel)
{
    if (devices == NULL || channel >= 8u) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    if ((devices->dma_registered & (uint8_t)(1u << channel)) != 0u) return NXVM_CORE_STATUS_INVALID_STATE;
    devices->dma_registered |= (uint8_t)(1u << channel);
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_core_dma_request(nxvm_core_pc_devices *devices, uint8_t channel)
{
    if (devices == NULL || channel >= 8u) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    if ((devices->dma_registered & (uint8_t)(1u << channel)) == 0u) return NXVM_CORE_STATUS_UNSUPPORTED;
    devices->dma_requested |= (uint8_t)(1u << channel);
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_core_dma_acknowledge(nxvm_core_pc_devices *devices, uint8_t *out_channel)
{
    int channel;
    if (devices == NULL || out_channel == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    channel = first_set(devices->dma_requested);
    if (channel < 0) return NXVM_CORE_STATUS_UNSUPPORTED;
    devices->dma_requested &= (uint8_t)~(1u << channel);
    *out_channel = (uint8_t)channel;
    return NXVM_CORE_STATUS_OK;
}
