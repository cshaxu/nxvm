#include "lib/types/test.h"
#ifndef TEST_IBMPC_COMMON_VIDEO_FIXTURE_H
#define TEST_IBMPC_COMMON_VIDEO_FIXTURE_H
#include "lib/types/types_interface.h"
#include "core/x86/machine_interface.h"
#include "core/board-base/vadp.h"

/* Compose the real adapter with opaque Core. Configuration stays open until
 * the first guest-visible operation; no private executor storage is borrowed. */
static inline core_machine *test_video_create(t_vadp *adapter)
{
    const core_machine_executor_config config = {
        .memory_bytes = CORE_MACHINE_DEFAULT_MEMORY_BYTES
    };
    core_machine *machine = LIB_NULL;
    if (core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        core_machine_vadp_initialize(adapter, machine) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
    return machine;
}

static inline void test_video_ready(core_machine *machine)
{
    if (core_machine_configuration_is_open(machine) &&
        (core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
         core_machine_reset(machine) != LIB_STATUS_OK)) lib_test_assert(LIB_FALSE);
}

static inline lib_u32 test_video_port_read(core_machine *machine, lib_u16 port)
{
    lib_u32 value = 0u;
    test_video_ready(machine);
    if (core_machine_bus_read(machine, port, &value) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
    return value;
}

static inline void test_video_port_write(core_machine *machine,
    lib_u16 port, lib_u32 value)
{
    test_video_ready(machine);
    if (core_machine_bus_write(machine, port, value) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
}

static inline lib_status test_video_probe_read(void *owner, lib_u16 port,
    lib_u64 tick, lib_u32 *value)
{
    (void)owner; (void)port; (void)tick;
    *value = 0u;
    return LIB_STATUS_OK;
}

static inline lib_status test_video_probe_write(void *owner, lib_u16 port,
    lib_u32 value)
{
    (void)owner; (void)port; (void)value;
    return LIB_STATUS_OK;
}

/* Before freeze, exclusive registration proves directional ownership without
 * touching a device register. A successful unused probe removes its own route. */
static inline lib_bool test_video_port_is_owned(core_machine *machine,
    lib_u16 port, lib_bool write)
{
    lib_u8 owner = 0u;
    const core_machine_port_provider provider = {
        .read = write ? LIB_NULL : test_video_probe_read,
        .write = write ? test_video_probe_write : LIB_NULL
    };
    lib_status status;
    if (!core_machine_configuration_is_open(machine)) lib_test_assert(LIB_FALSE);
    status = core_machine_install_port_provider(machine, port, port, &provider,
        &owner);
    if (status == LIB_STATUS_INVALID_STATE) return LIB_TRUE;
    if (status != LIB_STATUS_OK ||
        core_machine_remove_port_routes(machine, &owner) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
    return LIB_FALSE;
}
#endif
