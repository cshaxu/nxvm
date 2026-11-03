#include "app-mypc110/pc110_opl2.h"
#include "app-mypc110/pc110_clock.h"

/* L2 nominal OPL2 timer quanta: 80 us and 320 us, as in 86Box. */
static lib_u64 mypc110_opl2_period(const mypc110_opl2 *device, lib_u8 timer)
{
    const lib_u32 quantum = timer == 0u ? 80u : 320u;
    return (lib_u64)(256u - device->registers[2u + timer]) * quantum *
        (MYPC110_CPU_CLOCK_HZ / 1000000u);
}

void mypc110_opl2_reset(mypc110_opl2 *device)
{
    if (device != LIB_NULL) {
        x86_core_machine *core = device->core;
        void *context = device->output_context;
        lib_status (*before_write)(void *, lib_u64) = device->before_write;
        *device = (mypc110_opl2){.core = core, .output_context = context,
            .before_write = before_write};
    }
}

static void mypc110_opl2_update(mypc110_opl2 *device, lib_u64 tick)
{
    for (lib_u8 i = 0u; i < 2u; ++i) {
        const lib_u8 start = (lib_u8)(1u << i);
        const lib_u8 flag = (lib_u8)(0x40u >> i);
        if ((device->control & start) == 0u || tick < device->deadline[i]) continue;
        if ((device->control & flag) == 0u) device->overflow |= flag;
        const lib_u64 period = mypc110_opl2_period(device, i);
        device->deadline[i] = tick + period - (tick - device->deadline[i]) % period;
    }
}

static lib_status mypc110_opl2_read(void *owner, lib_u16 port,
    lib_u64 tick, lib_u32 *out_value)
{
    mypc110_opl2 *device = owner;
    if (device == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    mypc110_opl2_update(device, tick);
    *out_value = port == 0x388u ? (lib_u32)(device->overflow |
        (device->overflow != 0u ? 0x80u : 0u)) : 0xffu;
    return LIB_STATUS_OK;
}

static lib_status mypc110_opl2_write(void *owner, lib_u16 port,
    lib_u8 access_bytes, lib_u32 value)
{
    mypc110_opl2 *device = owner;
    lib_u64 tick = 0u;
    lib_status status;
    (void)access_bytes;
    if (device == LIB_NULL || value > 0xffu) return LIB_STATUS_INVALID_ARGUMENT;
    if (port == 0x388u) { device->address = (lib_u8)value; return LIB_STATUS_OK; }
    if (port != 0x389u) return LIB_STATUS_UNSUPPORTED;
    status = x86_core_machine_get_callback_ticks(device->core, &tick);
    if (status != LIB_STATUS_OK) return status;
    if (device->before_write != LIB_NULL) {
        status = device->before_write(device->output_context, tick);
        if (status != LIB_STATUS_OK) return status;
    }
    mypc110_opl2_update(device, tick);
    if (device->address >= 0xb0u && device->address <= 0xb8u) {
        const lib_u8 channel = (lib_u8)(device->address - 0xb0u);
        const lib_bool was_on = (device->registers[device->address] & 0x20u) != 0u;
        const lib_bool is_on = (value & 0x20u) != 0u;
        if (was_on != is_on) {
            for (lib_u8 op = 0u; op < 2u; ++op) {
                device->envelope[channel][op] = is_on ? 1u : 4u;
                if (is_on) {
                    device->phase[channel][op] = 0u;
                    device->level[channel][op] = 0.0;
                }
            }
            if (is_on) device->feedback[channel] = 0.0;
        }
    }
    device->registers[device->address] = (lib_u8)value;
    if (device->address == 4u) {
        if ((value & 0x80u) != 0u) device->overflow = 0u;
        else {
            device->control = (lib_u8)value;
            for (lib_u8 i = 0u; i < 2u; ++i) {
                if ((value & (1u << i)) != 0u)
                    device->deadline[i] = tick + mypc110_opl2_period(device, i);
                else device->overflow &= (lib_u8)~(0x40u >> i);
            }
        }
    }
    return LIB_STATUS_OK;
}

lib_status mypc110_opl2_install_routes(x86_core_machine *core, mypc110_opl2 *device)
{
    const x86_core_machine_port_route routes[] = {
        {0x388u, mypc110_opl2_read, mypc110_opl2_write, device, LIB_FALSE, 0u},
        {0x389u, mypc110_opl2_read, mypc110_opl2_write, device, LIB_FALSE, 0u}};
    if (core == LIB_NULL || device == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    device->core = core;
    return x86_core_machine_install_port_routes(core, routes, sizeof(routes) / sizeof(routes[0]));
}
