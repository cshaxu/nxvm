#include "app-mypc110/pc110_es488.h"
#include "app-mypc110/pc110_clock.h"

/* L1 default address; the real machine's software configures the audio base. */
#define MYPC110_ES488_RESET_PORT 0x0226u
#define MYPC110_ES488_DATA_PORT 0x022au
#define MYPC110_ES488_COMMAND_PORT 0x022cu
#define MYPC110_ES488_STATUS_PORT 0x022eu

void mypc110_es488_reset(mypc110_es488 *device)
{
    x86_core_machine *core;
    x86_nxvm_board_dma_bus *dma;
    x86_nxvm_board_dma_request_binding binding;
    x86_nxvm_board_pic_irq_source *irq;
    lib_u64 reset_epoch;
    if (device == LIB_NULL) return;
    core = device->core;
    dma = device->dma;
    binding = device->dma_binding;
    irq = device->irq;
    reset_epoch = device->reset_epoch + 1u;
    if (dma != LIB_NULL) x86_nxvm_board_dma_request_deassert(dma, &binding);
    if (irq != LIB_NULL) x86_nxvm_board_pic_irq_source_deassert(irq);
    *device = (mypc110_es488){0};
    device->core = core;
    device->dma = dma;
    device->dma_binding = binding;
    device->irq = irq;
    device->reset_epoch = reset_epoch;
    device->period_ticks = MYPC110_CPU_CLOCK_HZ / 10000u;
}

static void mypc110_es488_dma_write(void *owner, t_latch *value)
{
    mypc110_es488 *device = owner;
    if (device == LIB_NULL || value == LIB_NULL || !device->requested ||
        !device->playing || device->sample_count >= MYPC110_ES488_SAMPLE_CAPACITY) return;
    const lib_u16 index = (device->sample_read + device->sample_count) %
        MYPC110_ES488_SAMPLE_CAPACITY;
    device->samples[index] = device->speaker ? value->data.byte : 0x80u;
    device->sample_periods[index] = (lib_u16)device->period_ticks;
    ++device->sample_count;
    device->requested = LIB_FALSE;
    x86_nxvm_board_dma_request_deassert(device->dma, &device->dma_binding);
    if (--device->remaining == 0u) {
        device->playing = LIB_FALSE;
        x86_nxvm_board_pic_irq_source_assert(device->irq);
    } else device->next_tick += device->period_ticks;
}

lib_status mypc110_es488_initialize(mypc110_es488 *device,
    x86_core_machine *core, x86_nxvm_board_dma_bus *dma,
    x86_nxvm_board_pic_bus *master, x86_nxvm_board_pic_bus *slave)
{
    static const x86_nxvm_board_dma_channel_provider provider = {
        LIB_NULL, mypc110_es488_dma_write, LIB_NULL};
    lib_status status;
    if (device == LIB_NULL || core == LIB_NULL || dma == LIB_NULL || master == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    device->core = core;
    device->dma = dma;
    /* L1 startup resource assignment; PS2-configurable wiring is later work. */
    status = x86_nxvm_board_pic_irq_source_bind(&device->irq, master, slave, 5u);
    if (status == LIB_STATUS_OK)
        status = x86_nxvm_board_dma_bind_channel(dma, 1u, &provider, device,
            &device->dma_binding);
    if (status == LIB_STATUS_OK) mypc110_es488_reset(device);
    return status;
}

lib_bool mypc110_es488_next_deadline(mypc110_es488 *device, lib_u64 *out_tick)
{
    if (device == LIB_NULL || out_tick == LIB_NULL || !device->playing ||
        device->paused || device->requested ||
        device->sample_count == MYPC110_ES488_SAMPLE_CAPACITY) return LIB_FALSE;
    *out_tick = device->next_tick;
    return LIB_TRUE;
}

void mypc110_es488_advance(mypc110_es488 *device, lib_u64 tick)
{
    lib_u64 next;
    if (!mypc110_es488_next_deadline(device, &next) || tick < next) return;
    device->requested = LIB_TRUE;
    x86_nxvm_board_dma_request_assert(device->dma, &device->dma_binding);
}

static lib_status mypc110_es488_read(void *owner, lib_u16 port,
    lib_u64 tick, lib_u32 *out_value)
{
    mypc110_es488 *device = owner;
    (void)tick;
    if (device == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    switch (port) {
    case MYPC110_ES488_DATA_PORT:
        *out_value = device->response_index < device->response_count ?
            device->response[device->response_index++] : 0xffu;
        return LIB_STATUS_OK;
    case MYPC110_ES488_COMMAND_PORT:
        *out_value = 0u; /* L1: DSP is ready to accept a command. */
        return LIB_STATUS_OK;
    case MYPC110_ES488_STATUS_PORT:
        if (device->irq != LIB_NULL) x86_nxvm_board_pic_irq_source_deassert(device->irq);
        *out_value = device->response_index < device->response_count ? 0x80u : 0u;
        return LIB_STATUS_OK;
    default: return LIB_STATUS_UNSUPPORTED;
    }
}

static lib_status mypc110_es488_write(void *owner, lib_u16 port,
    lib_u8 access_bytes, lib_u32 value)
{
    mypc110_es488 *device = owner;
    (void)access_bytes;
    if (device == LIB_NULL || value > 0xffu) return LIB_STATUS_INVALID_ARGUMENT;
    if (port == MYPC110_ES488_RESET_PORT) {
        if ((value & 1u) != 0u) {
            device->reset_asserted = LIB_TRUE;
            device->response_count = 0u;
            device->response_index = 0u;
        } else if (device->reset_asserted) {
            mypc110_es488_reset(device);
            device->response[0] = 0xaau;
            device->response_count = 1u;
        }
        return LIB_STATUS_OK;
    }
    if (port != MYPC110_ES488_COMMAND_PORT) return LIB_STATUS_UNSUPPORTED;
    if (device->reset_asserted) return LIB_STATUS_OK;
    if (device->parameter_count != 0u) {
        device->parameters[--device->parameter_count] = (lib_u8)value;
        if (device->parameter_count == 0u) {
            if (device->command == 0x40u) {
                device->period_ticks = (MYPC110_CPU_CLOCK_HZ / 1000000u) *
                    (256u - device->parameters[0]);
            } else {
                lib_u64 now = 0u;
                lib_status status = x86_core_machine_get_elapsed_ticks(device->core, &now);
                if (status != LIB_STATUS_OK) return status;
                device->remaining = 1u + device->parameters[1] +
                    ((lib_u32)device->parameters[0] << 8u);
                device->next_tick = now + device->period_ticks;
                device->playing = LIB_TRUE;
                device->paused = LIB_FALSE;
            }
        }
        return LIB_STATUS_OK;
    }
    if (value == 0x40u || value == 0x14u) {
        device->command = (lib_u8)value;
        device->parameter_count = value == 0x40u ? 1u : 2u;
        return LIB_STATUS_OK;
    }
    if (value == 0xd1u || value == 0xd3u) {
        device->speaker = value == 0xd1u ? LIB_TRUE : LIB_FALSE;
        return LIB_STATUS_OK;
    }
    if (value == 0xd0u) {
        device->paused = LIB_TRUE;
        if (device->requested) {
            x86_nxvm_board_dma_request_deassert(device->dma, &device->dma_binding);
            device->requested = LIB_FALSE;
        }
        return LIB_STATUS_OK;
    }
    if (value == 0xd4u) {
        if (device->playing && device->paused) {
            lib_u64 now = 0u;
            lib_status status = x86_core_machine_get_elapsed_ticks(device->core, &now);
            if (status != LIB_STATUS_OK) return status;
            device->next_tick = now + device->period_ticks;
        }
        device->paused = LIB_FALSE;
        return LIB_STATUS_OK;
    }
    if (value == 0xe1u || value == 0xe7u) {
        device->response[0] = value == 0xe1u ? 0x02u : 0x48u;
        device->response[1] = value == 0xe1u ? 0x01u : 0x82u;
        device->response_count = 2u;
        device->response_index = 0u;
    }
    return LIB_STATUS_OK;
}

lib_status mypc110_es488_install_routes(x86_core_machine *machine,
    mypc110_es488 *device)
{
    const x86_core_machine_port_route routes[] = {
        {MYPC110_ES488_RESET_PORT, LIB_NULL, mypc110_es488_write, device, LIB_FALSE, 0u},
        {MYPC110_ES488_DATA_PORT, mypc110_es488_read, LIB_NULL, device, LIB_FALSE, 0u},
        {MYPC110_ES488_COMMAND_PORT, mypc110_es488_read, mypc110_es488_write, device, LIB_FALSE, 0u},
        {MYPC110_ES488_STATUS_PORT, mypc110_es488_read, LIB_NULL, device, LIB_FALSE, 0u}
    };
    if (machine == LIB_NULL || device == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return x86_core_machine_install_port_routes(machine, routes,
        sizeof(routes) / sizeof(routes[0]));
}
