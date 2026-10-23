#include "app-mypc110/pc110_config_interface.h"

#define MYPC110_BLOCK2_LOCK_PORT 0x0022u
#define MYPC110_BLOCK2_UNLOCK_FC23_PORT 0xfc23u
#define MYPC110_BLOCK2_UNLOCK_F023_PORT 0xf023u
#define MYPC110_BLOCK2_UNLOCK_C023_PORT 0xc023u
#define MYPC110_BLOCK2_UNLOCK_0023_PORT 0x0023u
#define MYPC110_BLOCK2_INDEX_PORT 0x0024u
#define MYPC110_BLOCK2_DATA_PORT 0x0025u
#define MYPC110_SCAMP_INDEX_PORT 0x0074u
#define MYPC110_SCAMP_DATA_PORT 0x0076u
#define MYPC110_ECED_INDEX_PORT 0x00ecu
#define MYPC110_ECED_DATA_PORT 0x00edu

static lib_status mypc110_config_read(void *owner, lib_u16 port,
    lib_u64 tick, lib_u32 *out_value)
{
    mypc110_config_routes *routes = owner;
    x86_vl82c420_bank bank;
    lib_u8 value;
    lib_status status;

    (void)tick;
    if (routes == LIB_NULL || routes->chip == LIB_NULL || out_value == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    switch (port) {
    case MYPC110_BLOCK2_UNLOCK_FC23_PORT:
        x86_vl82c420_observe_unlock_read(routes->chip,
            X86_VL82C420_UNLOCK_READ_FC23);
        *out_value = 0xffu;
        return LIB_STATUS_OK;
    case MYPC110_BLOCK2_UNLOCK_F023_PORT:
        x86_vl82c420_observe_unlock_read(routes->chip,
            X86_VL82C420_UNLOCK_READ_F023);
        *out_value = 0xffu;
        return LIB_STATUS_OK;
    case MYPC110_BLOCK2_UNLOCK_C023_PORT:
        x86_vl82c420_observe_unlock_read(routes->chip,
            X86_VL82C420_UNLOCK_READ_C023);
        *out_value = 0xffu;
        return LIB_STATUS_OK;
    case MYPC110_BLOCK2_UNLOCK_0023_PORT:
        x86_vl82c420_observe_unlock_read(routes->chip,
            X86_VL82C420_UNLOCK_READ_0023);
        *out_value = 0xffu;
        return LIB_STATUS_OK;
    case MYPC110_BLOCK2_DATA_PORT:
        bank = X86_VL82C420_BANK_BLOCK2;
        break;
    case MYPC110_SCAMP_DATA_PORT:
        bank = X86_VL82C420_BANK_SCAMP;
        break;
    case MYPC110_ECED_DATA_PORT:
        bank = X86_VL82C420_BANK_ECED;
        break;
    default: return LIB_STATUS_UNSUPPORTED;
    }
    status = x86_vl82c420_read_selected(routes->chip, bank, &value);
    if (status == LIB_STATUS_INVALID_STATE && bank == X86_VL82C420_BANK_BLOCK2) {
        *out_value = 0xffu;
        return LIB_STATUS_OK;
    }
    if (status != LIB_STATUS_OK) return status;
    *out_value = value;
    return LIB_STATUS_OK;
}

static lib_status mypc110_config_write(void *owner, lib_u16 port, lib_u32 value)
{
    mypc110_config_routes *routes = owner;
    x86_vl82c420_bank bank;

    if (routes == LIB_NULL || routes->chip == LIB_NULL || value > 0xffffu) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (port == MYPC110_BLOCK2_LOCK_PORT) {
        if ((value & 0x0100u) != 0u) x86_vl82c420_relock(routes->chip);
        return LIB_STATUS_OK;
    }
    if (value > 0xffu) return LIB_STATUS_INVALID_ARGUMENT;
    if (port == MYPC110_BLOCK2_INDEX_PORT) {
        lib_status status = x86_vl82c420_select(routes->chip,
            X86_VL82C420_BANK_BLOCK2, (lib_u8)value);
        return status == LIB_STATUS_INVALID_STATE ? LIB_STATUS_OK : status;
    }
    if (port == MYPC110_SCAMP_INDEX_PORT) {
        return x86_vl82c420_select(routes->chip,
            X86_VL82C420_BANK_SCAMP, (lib_u8)value);
    }
    if (port == MYPC110_ECED_INDEX_PORT) {
        return x86_vl82c420_select(routes->chip,
            X86_VL82C420_BANK_ECED, (lib_u8)value);
    }
    if (port == MYPC110_BLOCK2_DATA_PORT) {
        bank = X86_VL82C420_BANK_BLOCK2;
    } else if (port == MYPC110_SCAMP_DATA_PORT) bank = X86_VL82C420_BANK_SCAMP;
    else if (port == MYPC110_ECED_DATA_PORT) bank = X86_VL82C420_BANK_ECED;
    else return LIB_STATUS_UNSUPPORTED;
    {
        lib_status status = x86_vl82c420_write_selected(routes->chip, bank,
            (lib_u8)value);

        return status == LIB_STATUS_INVALID_STATE &&
            bank == X86_VL82C420_BANK_BLOCK2 ? LIB_STATUS_OK : status;
    }
}

lib_status mypc110_config_routes_initialize(mypc110_config_routes *routes,
    x86_vl82c420 *chip)
{
    if (routes == LIB_NULL || chip == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *routes = (mypc110_config_routes) {.chip = chip};
    return LIB_STATUS_OK;
}

lib_status mypc110_config_install_routes(x86_core_machine *machine,
    mypc110_config_routes *routes)
{
    const x86_core_machine_port_route port_routes[] = {
        {MYPC110_BLOCK2_LOCK_PORT, LIB_NULL, mypc110_config_write, routes, LIB_FALSE, 0u},
        {MYPC110_BLOCK2_INDEX_PORT, LIB_NULL, mypc110_config_write, routes, LIB_FALSE, 0u},
        {MYPC110_BLOCK2_DATA_PORT, mypc110_config_read, mypc110_config_write, routes, LIB_FALSE, 0u},
        {MYPC110_SCAMP_INDEX_PORT, LIB_NULL, mypc110_config_write, routes, LIB_FALSE, 0u},
        {MYPC110_SCAMP_DATA_PORT, mypc110_config_read, mypc110_config_write, routes, LIB_FALSE, 0u},
        {MYPC110_ECED_INDEX_PORT, LIB_NULL, mypc110_config_write, routes, LIB_FALSE, 0u},
        {MYPC110_ECED_DATA_PORT, mypc110_config_read, mypc110_config_write, routes, LIB_FALSE, 0u},
        {MYPC110_BLOCK2_UNLOCK_FC23_PORT, mypc110_config_read, LIB_NULL, routes, LIB_FALSE, 0u},
        {MYPC110_BLOCK2_UNLOCK_F023_PORT, mypc110_config_read, LIB_NULL, routes, LIB_FALSE, 0u},
        {MYPC110_BLOCK2_UNLOCK_C023_PORT, mypc110_config_read, LIB_NULL, routes, LIB_FALSE, 0u},
        {MYPC110_BLOCK2_UNLOCK_0023_PORT, mypc110_config_read, LIB_NULL, routes, LIB_FALSE, 0u}
    };

    if (machine == LIB_NULL || routes == LIB_NULL || routes->chip == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    return x86_core_machine_install_port_routes(machine, port_routes,
        sizeof(port_routes) / sizeof(port_routes[0]));
}
