#include "app-mypc110/pc110_config_interface.h"

#define MYPC110_SCAMP_INDEX_PORT 0x0074u
#define MYPC110_SCAMP_DATA_PORT 0x0076u
#define MYPC110_ECED_INDEX_PORT 0x00ecu
#define MYPC110_ECED_DATA_PORT 0x00edu

static lib_status mypc110_config_read(void *owner, lib_u16 port,
    lib_u64 tick, lib_u32 *out_value)
{
    x86_vl82c420_bank bank;
    lib_u8 value;
    lib_status status;

    (void)tick;
    if (owner == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (port == MYPC110_SCAMP_DATA_PORT) bank = X86_VL82C420_BANK_SCAMP;
    else if (port == MYPC110_ECED_DATA_PORT) bank = X86_VL82C420_BANK_ECED;
    else return LIB_STATUS_UNSUPPORTED;
    status = x86_vl82c420_read_selected((const x86_vl82c420 *)owner, bank, &value);
    if (status != LIB_STATUS_OK) return status;
    *out_value = value;
    return LIB_STATUS_OK;
}

static lib_status mypc110_config_write(void *owner, lib_u16 port, lib_u32 value)
{
    x86_vl82c420_bank bank;

    if (owner == LIB_NULL || value > 0xffu) return LIB_STATUS_INVALID_ARGUMENT;
    if (port == MYPC110_SCAMP_INDEX_PORT) {
        return x86_vl82c420_select((x86_vl82c420 *)owner,
            X86_VL82C420_BANK_SCAMP, (lib_u8)value);
    }
    if (port == MYPC110_ECED_INDEX_PORT) {
        return x86_vl82c420_select((x86_vl82c420 *)owner,
            X86_VL82C420_BANK_ECED, (lib_u8)value);
    }
    if (port == MYPC110_SCAMP_DATA_PORT) bank = X86_VL82C420_BANK_SCAMP;
    else if (port == MYPC110_ECED_DATA_PORT) bank = X86_VL82C420_BANK_ECED;
    else return LIB_STATUS_UNSUPPORTED;
    return x86_vl82c420_write_selected((x86_vl82c420 *)owner, bank,
        (lib_u8)value);
}

lib_status mypc110_config_install_routes(x86_core_machine *machine,
    x86_vl82c420 *chip)
{
    const x86_core_machine_port_route routes[] = {
        {MYPC110_SCAMP_INDEX_PORT, LIB_NULL, mypc110_config_write, chip, LIB_FALSE, 0u},
        {MYPC110_SCAMP_DATA_PORT, mypc110_config_read, mypc110_config_write, chip, LIB_FALSE, 0u},
        {MYPC110_ECED_INDEX_PORT, LIB_NULL, mypc110_config_write, chip, LIB_FALSE, 0u},
        {MYPC110_ECED_DATA_PORT, mypc110_config_read, mypc110_config_write, chip, LIB_FALSE, 0u}
    };

    if (machine == LIB_NULL || chip == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return x86_core_machine_install_port_routes(machine, routes,
        sizeof(routes) / sizeof(routes[0]));
}
