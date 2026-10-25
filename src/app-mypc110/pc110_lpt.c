#include "app-mypc110/pc110_lpt_interface.h"

#define MYPC110_LPT_DATA_PORT 0x03bcu
#define MYPC110_LPT_STATUS_PORT 0x03bdu
#define MYPC110_LPT_CONTROL_PORT 0x03beu
#define MYPC110_LPT_BIOS_PROBE_CONTROL 0x21u
#define MYPC110_LPT_DEFAULT_STATUS 0x90u
#define MYPC110_LPT_PROBE_STATUS 0xd0u

static lib_status mypc110_lpt_read(void *owner, lib_u16 port,
    lib_u64 tick, lib_u32 *out_value)
{
    mypc110_lpt *lpt = owner;

    (void)tick;
    if (lpt == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    switch (port) {
    case MYPC110_LPT_DATA_PORT:
        *out_value = 0xffu;
        return LIB_STATUS_OK;
    case MYPC110_LPT_STATUS_PORT:
        /* L2: the admitted BIOS writes 21h while probing this legacy range.
         * PC110-EMU observes the corresponding status-bit transition; do not
         * represent this as a sourced physical printer implementation. */
        *out_value = lpt->control == MYPC110_LPT_BIOS_PROBE_CONTROL ?
            MYPC110_LPT_PROBE_STATUS : MYPC110_LPT_DEFAULT_STATUS;
        return LIB_STATUS_OK;
    case MYPC110_LPT_CONTROL_PORT:
        *out_value = lpt->control;
        return LIB_STATUS_OK;
    default: return LIB_STATUS_UNSUPPORTED;
    }
}

static lib_status mypc110_lpt_write(void *owner, lib_u16 port,
    lib_u8 access_bytes, lib_u32 value)
{
    mypc110_lpt *lpt = owner;

    (void)access_bytes;
    if (lpt == LIB_NULL || value > 0xffu) return LIB_STATUS_INVALID_ARGUMENT;
    if (port != MYPC110_LPT_CONTROL_PORT) return LIB_STATUS_UNSUPPORTED;
    lpt->control = (lib_u8)value;
    return LIB_STATUS_OK;
}

lib_status mypc110_lpt_install_routes(x86_core_machine *machine,
    mypc110_lpt *lpt)
{
    const x86_core_machine_port_route routes[] = {
        {MYPC110_LPT_DATA_PORT, mypc110_lpt_read, LIB_NULL, lpt, LIB_FALSE, 0u},
        {MYPC110_LPT_STATUS_PORT, mypc110_lpt_read, LIB_NULL, lpt, LIB_FALSE, 0u},
        {MYPC110_LPT_CONTROL_PORT, mypc110_lpt_read, mypc110_lpt_write,
            lpt, LIB_FALSE, 0u}
    };

    if (machine == LIB_NULL || lpt == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return x86_core_machine_install_port_routes(machine, routes,
        sizeof(routes) / sizeof(routes[0]));
}
