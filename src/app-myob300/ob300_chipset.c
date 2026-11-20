#include "app-myob300/ob300_chipset.h"
#include "x86-core/machine/machine_interface.h"

#define MYOB300_CHIPSET_INDEX_PORT 0xecu
#define MYOB300_CHIPSET_DATA_PORT 0xedu
#define MYOB300_CHIPSET_FAST_RESET_PORT 0xefu
#define MYOB300_CHIPSET_LOCK_PORT 0xf9u
#define MYOB300_CHIPSET_UNLOCK_PORT 0xfbu
#define MYOB300_CHIPSET_VERSION 0x00u
#define MYOB300_CHIPSET_MISCELLANEOUS 0x14u
#define MYOB300_CHIPSET_MANAGEMENT 0x18u
#define MYOB300_CHIPSET_VSF_DISABLED 0x80u
#define MYOB300_CHIPSET_MANAGEMENT_ACTIVE 0x08u
#define MYOB300_CHIPSET_MANAGEMENT_LOCK 0x10u
#define MYOB300_CHIPSET_MANAGEMENT_WRITABLE 0xb7u

void myob300_chipset_reset(myob300_chipset *chipset)
{
    chipset->configuration_index = 0u;
    /* SCAMP II Table 51 defaults, with the unverified MA9 strap low. */
    chipset->miscellaneous = 0x11u;
    chipset->management_control = 0x20u;
    chipset->configuration_enabled = LIB_FALSE;
    chipset->management_active = LIB_FALSE;
}

static lib_status myob300_chipset_read(void *owner, lib_u16 port,
    lib_u64 tick, lib_u32 *out_value)
{
    myob300_chipset *chipset = owner;

    (void)tick;
    switch (port) {
    case MYOB300_CHIPSET_INDEX_PORT:
        *out_value = chipset->configuration_index;
        return LIB_STATUS_OK;
    case MYOB300_CHIPSET_FAST_RESET_PORT:
        if ((chipset->miscellaneous & MYOB300_CHIPSET_VSF_DISABLED) != 0u)
            return LIB_STATUS_UNSUPPORTED;
        x86_core_machine_signal_processor_reset(chipset->core);
        *out_value = 0xffu;
        return LIB_STATUS_OK;
    case MYOB300_CHIPSET_DATA_PORT:
        break;
    default:
        return LIB_STATUS_UNSUPPORTED;
    }
    switch (chipset->configuration_index) {
    case MYOB300_CHIPSET_VERSION: *out_value = 0x6au; break;
    case MYOB300_CHIPSET_MISCELLANEOUS: *out_value = chipset->miscellaneous; break;
    case MYOB300_CHIPSET_MANAGEMENT:
        *out_value = chipset->management_control |
            (chipset->management_active ? MYOB300_CHIPSET_MANAGEMENT_ACTIVE : 0u);
        break;
    default: return LIB_STATUS_UNSUPPORTED;
    }
    return LIB_STATUS_OK;
}

static lib_status myob300_chipset_write(void *owner, lib_u16 port,
    lib_u8 access_bytes, lib_u32 value)
{
    myob300_chipset *chipset = owner;
    const lib_bool vsf_disabled =
        (chipset->miscellaneous & MYOB300_CHIPSET_VSF_DISABLED) != 0u;

    if (access_bytes != 1u) return LIB_STATUS_UNSUPPORTED;
    if (port == MYOB300_CHIPSET_UNLOCK_PORT || port == MYOB300_CHIPSET_LOCK_PORT) {
        if (vsf_disabled) return LIB_STATUS_UNSUPPORTED;
        chipset->configuration_enabled = port == MYOB300_CHIPSET_UNLOCK_PORT;
        return LIB_STATUS_OK;
    }
    /* MISCSET disables the F9/FB gate itself. Locked writes consume the
     * selected I/O cycle without changing the configuration latches. */
    if (!vsf_disabled && !chipset->configuration_enabled) return LIB_STATUS_OK;
    if (port == MYOB300_CHIPSET_INDEX_PORT) {
        chipset->configuration_index = (lib_u8)value;
        return LIB_STATUS_OK;
    }
    if (port != MYOB300_CHIPSET_DATA_PORT) return LIB_STATUS_UNSUPPORTED;
    switch (chipset->configuration_index) {
    case MYOB300_CHIPSET_VERSION: break;
    case MYOB300_CHIPSET_MISCELLANEOUS: chipset->miscellaneous = (lib_u8)value; break;
    case MYOB300_CHIPSET_MANAGEMENT:
        if ((chipset->management_control & MYOB300_CHIPSET_MANAGEMENT_LOCK) == 0u ||
            chipset->management_active)
            chipset->management_control = (lib_u8)value & MYOB300_CHIPSET_MANAGEMENT_WRITABLE;
        break;
    default: return LIB_STATUS_UNSUPPORTED;
    }
    return LIB_STATUS_OK;
}

lib_status myob300_chipset_install(myob300_chipset *chipset, x86_core_machine *core)
{
    const x86_core_machine_port_route routes[] = {
        {MYOB300_CHIPSET_INDEX_PORT, myob300_chipset_read, myob300_chipset_write,
            chipset, LIB_FALSE, MYOB300_CHIPSET_DATA_PORT + 1u},
        {MYOB300_CHIPSET_DATA_PORT, myob300_chipset_read, myob300_chipset_write,
            chipset, LIB_FALSE, MYOB300_CHIPSET_DATA_PORT + 1u},
        {MYOB300_CHIPSET_FAST_RESET_PORT, myob300_chipset_read, LIB_NULL,
            chipset, LIB_FALSE, 0u},
        {MYOB300_CHIPSET_LOCK_PORT, LIB_NULL, myob300_chipset_write,
            chipset, LIB_FALSE, 0u},
        {MYOB300_CHIPSET_UNLOCK_PORT, LIB_NULL, myob300_chipset_write,
            chipset, LIB_FALSE, 0u}
    };
    const lib_status status = x86_core_machine_install_port_routes(core, routes,
        sizeof(routes) / sizeof(routes[0]));

    if (status == LIB_STATUS_OK) {
        chipset->core = core;
        myob300_chipset_reset(chipset);
    }
    return status;
}
