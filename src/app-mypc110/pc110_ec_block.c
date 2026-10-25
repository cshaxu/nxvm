#include "app-mypc110/pc110_ec_block_interface.h"

#define MYPC110_EC_BLOCK_INDEX_PORT 0x35eau
#define MYPC110_EC_BLOCK_DATA_PORT 0x35ebu
#define MYPC110_EC_BLOCK_REGISTER_COUNT 32u

/* L2 static capture from Open-Source-PC110 Discovery/Pluto.  The source
 * establishes index wrapping but not writes or dynamic battery behavior. */
static const lib_u8 mypc110_ec_block_values[MYPC110_EC_BLOCK_REGISTER_COUNT] = {
    0x00u, 0xffu, 0xf5u, 0xffu, 0x84u, 0xf3u, 0x6cu, 0xfdu,
    0xffu, 0xf7u, 0xffu, 0xfcu, 0xffu, 0xffu, 0xffu, 0xffu,
    0xffu, 0xffu, 0xffu, 0xe8u, 0x35u, 0x04u, 0xffu, 0xffu,
    0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu};

static lib_status mypc110_ec_block_read(void *owner, lib_u16 port,
    lib_u64 tick, lib_u32 *out_value)
{
    mypc110_ec_block *block = owner;

    (void)tick;
    if (block == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (port != MYPC110_EC_BLOCK_DATA_PORT) return LIB_STATUS_UNSUPPORTED;
    *out_value = mypc110_ec_block_values[
        block->index % MYPC110_EC_BLOCK_REGISTER_COUNT];
    return LIB_STATUS_OK;
}

static lib_status mypc110_ec_block_write(void *owner, lib_u16 port, lib_u32 value)
{
    mypc110_ec_block *block = owner;

    if (block == LIB_NULL || value > 0xffu) return LIB_STATUS_INVALID_ARGUMENT;
    if (port != MYPC110_EC_BLOCK_INDEX_PORT) return LIB_STATUS_UNSUPPORTED;
    block->index = (lib_u8)value;
    return LIB_STATUS_OK;
}

lib_status mypc110_ec_block_install_routes(x86_core_machine *machine,
    mypc110_ec_block *block)
{
    const x86_core_machine_port_route routes[] = {
        {MYPC110_EC_BLOCK_INDEX_PORT, LIB_NULL, mypc110_ec_block_write,
            block, LIB_FALSE, 0u},
        {MYPC110_EC_BLOCK_DATA_PORT, mypc110_ec_block_read, LIB_NULL,
            block, LIB_FALSE, 0u}};

    if (machine == LIB_NULL || block == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return x86_core_machine_install_port_routes(machine, routes,
        sizeof(routes) / sizeof(routes[0]));
}
