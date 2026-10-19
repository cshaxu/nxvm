#include "myraidenii/core/cop.h"

enum {
    MYRAIDENII_CORE_COP_STORAGE_START = 0x00400u,
    MYRAIDENII_CORE_COP_STORAGE_BYTES = 0x00106u
};

typedef struct myraidenii_core_cop_range {
    lib_u32 physical_start;
    lib_size bytes;
    lib_bool command;
} myraidenii_core_cop_range;

struct myraidenii_core_cop {
    lib_u8 registers[MYRAIDENII_CORE_COP_STORAGE_BYTES];
};

static const myraidenii_core_cop_range myraidenii_core_cop_ranges[] = {
    {0x00428u, 0x00006u, LIB_FALSE},
    {0x00432u, 0x0000cu, LIB_FALSE},
    {0x00444u, 0x00002u, LIB_FALSE},
    {0x00476u, 0x0000au, LIB_FALSE},
    {0x004a0u, 0x0000eu, LIB_FALSE},
    {0x004c0u, 0x0000eu, LIB_FALSE},
    {0x00500u, 0x00006u, LIB_TRUE}
};

static const myraidenii_core_cop_range *myraidenii_core_cop_range_find(lib_u32 physical,
    lib_uptr bytes)
{
    lib_size index;

    if (bytes == 0u || bytes > 2u) return LIB_NULL;
    for (index = 0u; index < sizeof(myraidenii_core_cop_ranges) /
        sizeof(myraidenii_core_cop_ranges[0]); ++index) {
        const myraidenii_core_cop_range *range = &myraidenii_core_cop_ranges[index];

        if (physical >= range->physical_start &&
            (lib_u64)physical - range->physical_start + bytes <= range->bytes)
            return range;
    }
    return LIB_NULL;
}

static lib_status myraidenii_core_cop_query(void *owner, lib_u32 physical,
    lib_uptr bytes, x86_core_machine_memory_access access)
{
    (void)access;
    if (owner == LIB_NULL) return LIB_STATUS_INTERNAL_ERROR;
    return myraidenii_core_cop_range_find(physical, bytes) == LIB_NULL ?
        LIB_STATUS_UNSUPPORTED : LIB_STATUS_OK;
}

static lib_status myraidenii_core_cop_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    (void)observe_only;
    if (owner == LIB_NULL || destination == 0u || bytes == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (myraidenii_core_cop_range_find(physical, bytes) == LIB_NULL)
        return LIB_STATUS_UNSUPPORTED;
    /* S5 owns route decoding, not undocumented COP result/readback semantics. */
    return LIB_STATUS_INVALID_STATE;
}

static lib_status myraidenii_core_cop_write(void *owner, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    myraidenii_core_cop *cop = owner;
    const myraidenii_core_cop_range *range;

    if (cop == LIB_NULL || source == 0u || bytes == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    range = myraidenii_core_cop_range_find(physical, bytes);
    if (range == LIB_NULL) return LIB_STATUS_UNSUPPORTED;
    if (range->command) return LIB_STATUS_INVALID_STATE;
    lib_memory_copy(cop->registers + physical - MYRAIDENII_CORE_COP_STORAGE_START,
        (const void *)source, bytes);
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_cop_create(myraidenii_core_cop **out_cop)
{
    myraidenii_core_cop *cop;

    if (out_cop == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_cop = LIB_NULL;
    cop = lib_allocate_zero(1u, sizeof(*cop));
    if (cop == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    *out_cop = cop;
    return LIB_STATUS_OK;
}

void myraidenii_core_cop_destroy(myraidenii_core_cop *cop)
{
    lib_release(cop);
}

void myraidenii_core_cop_reset(myraidenii_core_cop *cop)
{
    if (cop != LIB_NULL) lib_memory_set(cop->registers, 0, sizeof(cop->registers));
}

lib_status myraidenii_core_cop_install(myraidenii_core_cop *cop,
    x86_core_machine *machine)
{
    const x86_core_machine_memory_device_callbacks callbacks = {
        myraidenii_core_cop_read,
        myraidenii_core_cop_write,
        myraidenii_core_cop_query
    };
    x86_core_machine_memory_device_route routes[
        sizeof(myraidenii_core_cop_ranges) / sizeof(myraidenii_core_cop_ranges[0])];
    lib_size index;

    if (cop == LIB_NULL || machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    for (index = 0u; index < sizeof(routes) / sizeof(routes[0]); ++index) {
        routes[index].physical_start = myraidenii_core_cop_ranges[index].physical_start;
        routes[index].bytes = myraidenii_core_cop_ranges[index].bytes;
        routes[index].callbacks = callbacks;
        routes[index].mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT;
    }
    return x86_core_machine_install_memory_device_routes(machine, routes,
        sizeof(routes) / sizeof(routes[0]), LIB_NULL, LIB_NULL, cop);
}
