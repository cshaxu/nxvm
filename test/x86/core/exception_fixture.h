#ifndef TEST_CORE_EXCEPTION_FIXTURE_H
#define TEST_CORE_EXCEPTION_FIXTURE_H

#include "x86/core/machine_interface.h"
#include "x86/core/memory_interface.h"

/* Negative producer/rollback tests must explicitly prevent IVT delivery. */
static inline lib_status test_core_exception_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    (void)owner; (void)physical; (void)destination;
    (void)bytes; (void)observe_only;
    return LIB_STATUS_IO_ERROR;
}

static inline lib_status test_core_exception_write(void *owner, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    (void)owner; (void)physical; (void)source; (void)bytes;
    return LIB_STATUS_IO_ERROR;
}

static inline lib_status test_core_exception_query(void *owner, lib_u32 physical,
    lib_uptr bytes, core_machine_memory_access access)
{
    (void)owner; (void)physical; (void)bytes; (void)access;
    return LIB_STATUS_OK;
}

static inline lib_status test_core_exception_block_vector(core_machine *machine,
    lib_u8 vector, void *owner)
{
    const core_machine_memory_device_route route = {
        .physical_start = (lib_u32)vector * 4u, .bytes = 4u,
        .callbacks = { test_core_exception_read, test_core_exception_write,
            test_core_exception_query },
        .mode = CORE_MACHINE_MEMORY_PROVIDER_OVERLAY
    };
    return core_machine_install_memory_device_routes(machine, &route, 1u,
        LIB_NULL, LIB_NULL, owner);
}

#endif
