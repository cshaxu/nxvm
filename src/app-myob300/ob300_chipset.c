#include "app-myob300/ob300_chipset.h"
#include "x86-core/machine/machine_interface.h"
#include "x86-core/machine/memory_interface.h"

#define MYOB300_CHIPSET_INDEX_PORT 0xecu
#define MYOB300_CHIPSET_DATA_PORT 0xedu
#define MYOB300_CHIPSET_FAST_RESET_PORT 0xefu
#define MYOB300_CHIPSET_LOCK_PORT 0xf9u
#define MYOB300_CHIPSET_UNLOCK_PORT 0xfbu
#define MYOB300_CHIPSET_VERSION 0x00u
#define MYOB300_CHIPSET_MISCELLANEOUS 0x14u
#define MYOB300_CHIPSET_AB_ACCESS 0x0eu
#define MYOB300_CHIPSET_AB_BASE 0xa0000u
#define MYOB300_CHIPSET_AB_BLOCK_BYTES 0x8000u
#define MYOB300_CHIPSET_MANAGEMENT 0x18u
#define MYOB300_KEYBOARD_ROW_SELECT 0x3au
#define MYOB300_KEYBOARD_COLUMN_LOW 0x3bu
#define MYOB300_KEYBOARD_COLUMN_HIGH 0x3cu
#define MYOB300_CHIPSET_VSF_DISABLED 0x80u
#define MYOB300_CHIPSET_MANAGEMENT_ACTIVE 0x08u
#define MYOB300_CHIPSET_MANAGEMENT_LOCK 0x10u
#define MYOB300_CHIPSET_MANAGEMENT_WRITABLE 0xb7u

/* Table 65 gives SCAMP defaults, with the 2 MiB strap modeled as map 3.
 * The HP extension entries are observed startup latches. Their
 * electrical effects and unexercised SCAMP modes are not modeled here. */
static const struct {
    lib_u8 index;
    lib_u8 reset;
} myob300_chipset_startup[MYOB300_CHIPSET_STARTUP_REGISTER_COUNT] = {
    {0x02u, 0x00u}, {0x03u, 0x03u}, {0x05u, 0x00u}, {0x07u, 0x10u},
    {0x0au, 0x00u}, {0x0fu, 0x00u}, {0x10u, 0x00u},
    {0x11u, 0x00u}, {0x15u, 0x00u},
    {0x1eu, 0x00u}, {0x1fu, 0x00u}, {0x20u, 0x00u}, {0x21u, 0x00u},
    {0x22u, 0x00u}, {0x23u, 0x00u}, {0x24u, 0x00u}, {0x25u, 0x00u},
    {0x32u, 0x00u}, {0x33u, 0x00u}, {0x34u, 0x00u}, {0x35u, 0x00u},
    {0x36u, 0x00u}, {0x37u, 0x00u}, {0x38u, 0x00u}, {0x39u, 0x00u},
    {0x3du, 0x00u}, {0x40u, 0x00u}, {0x41u, 0x00u}, {0x42u, 0x00u}, {0x43u, 0x00u},
    {0x44u, 0x00u}, {0x47u, 0x00u}, {0x48u, 0x00u}
};

static lib_u8 *myob300_chipset_startup_register(myob300_chipset *chipset)
{
    for (lib_size slot = 0u; slot < MYOB300_CHIPSET_STARTUP_REGISTER_COUNT; ++slot)
        if (myob300_chipset_startup[slot].index == chipset->configuration_index)
            return &chipset->startup_registers[slot];
    return LIB_NULL;
}

static lib_u16 myob300_chipset_keyboard_columns(const myob300_chipset *chipset)
{
    lib_u16 columns = LIB_UINT16_MAX;

    for (lib_size row = 0u; row < MYOB300_KEYBOARD_ROWS; ++row)
        if ((chipset->keyboard_row_select & (1u << row)) == 0u)
            columns &= chipset->keyboard_columns[row];
    return columns;
}

static lib_status myob300_chipset_ab_query(void *owner, lib_u32 address,
    lib_uptr bytes, x86_core_machine_memory_access access)
{
    const myob300_chipset *chipset = owner;
    const lib_u32 offset = address - MYOB300_CHIPSET_AB_BASE;
    const lib_u8 mask = access == X86_CORE_MACHINE_MEMORY_ACCESS_WRITE ? 1u : 2u;
    lib_uptr last_block;

    if (offset >= sizeof(chipset->ab_shadow) || bytes == 0u ||
        bytes > sizeof(chipset->ab_shadow) - offset) return LIB_STATUS_UNSUPPORTED;
    last_block = (offset + bytes - 1u) / MYOB300_CHIPSET_AB_BLOCK_BYTES;
    for (lib_uptr block = offset / MYOB300_CHIPSET_AB_BLOCK_BYTES; block <= last_block; ++block)
        if (((chipset->ab_access >> (2u * block)) & mask) == 0u)
            return LIB_STATUS_UNSUPPORTED;
    return LIB_STATUS_OK;
}

static lib_status myob300_chipset_ab_read(void *owner, lib_u32 address,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    const myob300_chipset *chipset = owner;
    (void)observe_only;
    lib_memory_copy((void *)destination, chipset->ab_shadow + (address - MYOB300_CHIPSET_AB_BASE),
        bytes);
    return LIB_STATUS_OK;
}

static lib_status myob300_chipset_ab_write(void *owner, lib_u32 address,
    lib_uptr source, lib_uptr bytes)
{
    myob300_chipset *chipset = owner;
    lib_memory_copy(chipset->ab_shadow + (address - MYOB300_CHIPSET_AB_BASE), (const void *)source,
        bytes);
    return LIB_STATUS_OK;
}

void myob300_chipset_reset(myob300_chipset *chipset)
{
    chipset->configuration_index = 0u;
    /* SCAMP II Table 51 defaults, with the unverified MA9 strap low. */
    chipset->miscellaneous = 0x11u;
    chipset->management_control = 0x20u;
    chipset->configuration_enabled = LIB_FALSE;
    chipset->management_active = LIB_FALSE;
    chipset->ab_access = 0u;
    lib_memory_set(chipset->ab_shadow, 0, sizeof(chipset->ab_shadow));
    chipset->keyboard_row_select = 0u;
    for (lib_size row = 0u; row < MYOB300_KEYBOARD_ROWS; ++row)
        chipset->keyboard_columns[row] = LIB_UINT16_MAX;
    for (lib_size slot = 0u; slot < MYOB300_CHIPSET_STARTUP_REGISTER_COUNT; ++slot)
        chipset->startup_registers[slot] = myob300_chipset_startup[slot].reset;
    myob300_card_reset(&chipset->card);
}

static lib_status myob300_chipset_read(void *owner, lib_u16 port,
    lib_u64 tick, lib_u32 *out_value)
{
    myob300_chipset *chipset = owner;
    const lib_u8 *startup;

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
    case MYOB300_CHIPSET_AB_ACCESS: *out_value = chipset->ab_access; break;
    case MYOB300_CHIPSET_MISCELLANEOUS: *out_value = chipset->miscellaneous; break;
    case MYOB300_CHIPSET_MANAGEMENT:
        *out_value = chipset->management_control |
            (chipset->management_active ? MYOB300_CHIPSET_MANAGEMENT_ACTIVE : 0u);
        break;
    case MYOB300_KEYBOARD_COLUMN_LOW:
        *out_value = myob300_chipset_keyboard_columns(chipset) & 0xffu;
        break;
    case MYOB300_KEYBOARD_COLUMN_HIGH:
        *out_value = myob300_chipset_keyboard_columns(chipset) >> 8u;
        break;
    default:
        startup = myob300_chipset_startup_register(chipset);
        if (startup == LIB_NULL)
            return myob300_card_register_read(&chipset->card,
                chipset->configuration_index, out_value);
        *out_value = *startup;
        break;
    }
    return LIB_STATUS_OK;
}

static lib_status myob300_chipset_write(void *owner, lib_u16 port,
    lib_u8 access_bytes, lib_u32 value)
{
    myob300_chipset *chipset = owner;
    lib_u8 *startup;
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
    case MYOB300_CHIPSET_AB_ACCESS: chipset->ab_access = (lib_u8)value; break;
    case MYOB300_CHIPSET_MISCELLANEOUS: chipset->miscellaneous = (lib_u8)value; break;
    case MYOB300_CHIPSET_MANAGEMENT:
        if ((chipset->management_control & MYOB300_CHIPSET_MANAGEMENT_LOCK) == 0u ||
            chipset->management_active)
            chipset->management_control = (lib_u8)value & MYOB300_CHIPSET_MANAGEMENT_WRITABLE;
        break;
    case MYOB300_KEYBOARD_ROW_SELECT:
        chipset->keyboard_row_select = (lib_u8)value;
        break;
    default:
        startup = myob300_chipset_startup_register(chipset);
        if (startup == LIB_NULL)
            return myob300_card_register_write(&chipset->card,
                chipset->configuration_index, (lib_u8)value);
        *startup = (lib_u8)value;
        break;
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
    const x86_core_machine_memory_device_route ab_shadow = {
        MYOB300_CHIPSET_AB_BASE, MYOB300_CHIPSET_AB_SHADOW_BYTES,
        {myob300_chipset_ab_read, myob300_chipset_ab_write, myob300_chipset_ab_query},
        X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT_PARTIAL, LIB_NULL
    };
    lib_status status = x86_core_machine_install_port_routes(core, routes,
        sizeof(routes) / sizeof(routes[0]));

    if (status != LIB_STATUS_OK) return status;
    status = x86_core_machine_install_memory_device_routes(core, &ab_shadow, 1u,
        LIB_NULL, LIB_NULL, chipset);
    if (status != LIB_STATUS_OK) {
        (void)x86_core_machine_remove_port_routes(core, chipset);
        return status;
    }
    chipset->core = core;
    myob300_chipset_reset(chipset);
    return LIB_STATUS_OK;
}
