#include "app-mypc110/pc110_es488.h"

/* L1 default address; the real machine's software configures the audio base. */
#define MYPC110_ES488_RESET_PORT 0x0226u
#define MYPC110_ES488_DATA_PORT 0x022au
#define MYPC110_ES488_COMMAND_PORT 0x022cu
#define MYPC110_ES488_STATUS_PORT 0x022eu

void mypc110_es488_reset(mypc110_es488 *device)
{
    if (device != LIB_NULL) *device = (mypc110_es488){0};
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
