#include "video_registration_fixture.h"
#include "x86-nxvm/board-base/vadp.h"

static lib_u32 rejected_dependency;
static lib_bool rejection_reached;
static lib_size submitted_ports;
static lib_size published_memory;
static lib_size removed_memory;
static const void *published_owner;
static const void *removed_owner;

static lib_status video_test_ports(x86_core_machine *machine,
    const x86_core_machine_port_route *routes, lib_size count)
{
    submitted_ports = count;
    if (rejected_dependency == 2u) {
        rejection_reached = LIB_TRUE;
        return LIB_STATUS_NO_MEMORY;
    }
    return x86_core_machine_install_port_routes(machine, routes, count);
}

static lib_status video_test_memory(x86_core_machine *machine,
    const x86_core_machine_memory_device_route *routes, lib_size count,
    x86_core_machine_memory_write_observer observer,
    const x86_core_machine_memory_parity_config *parity, void *owner)
{
    if (rejected_dependency == 1u) {
        rejection_reached = LIB_TRUE;
        return LIB_STATUS_NO_MEMORY;
    }
    const lib_status status = x86_core_machine_install_memory_device_routes(
        machine, routes, count, observer, parity, owner);
    if (status == LIB_STATUS_OK) {
        ++published_memory;
        published_owner = owner;
    }
    return status;
}

static lib_status video_test_remove_memory(x86_core_machine *machine,
    const void *owner)
{
    const lib_status status = x86_core_machine_remove_memory_device_routes(machine, owner);

    if (status == LIB_STATUS_OK) {
        ++removed_memory;
        removed_owner = owner;
    }
    return status;
}

/* Observe the real Board publication/rollback calls, never Machine storage. */
#define x86_core_machine_install_port_routes video_test_ports
#define x86_core_machine_install_memory_device_routes video_test_memory
#define x86_core_machine_remove_memory_device_routes video_test_remove_memory
#include "x86-nxvm/board-base/vadp.c"
#undef x86_core_machine_remove_memory_device_routes
#undef x86_core_machine_install_memory_device_routes
#undef x86_core_machine_install_port_routes

lib_i32 test_video_registration_initial_failure(t_vadp *adapter,
    x86_core_machine *machine)
{
    rejected_dependency = 2u;
    rejection_reached = LIB_FALSE;
    submitted_ports = 0u;
    const lib_i32 failed = test_video_registration_initialize(adapter, machine,
        LIB_STATUS_NO_MEMORY) || !rejection_reached || submitted_ports != 8u;

    rejected_dependency = 0u;
    return failed;
}

lib_i32 test_video_registration_dependency_failure(t_vadp *adapter,
    const x86_nxvm_board_display_config *config, lib_bool reject_memory,
    lib_size port_count)
{
    rejected_dependency = reject_memory ? 1u : 2u;
    rejection_reached = LIB_FALSE;
    submitted_ports = published_memory = removed_memory = 0u;
    published_owner = removed_owner = LIB_NULL;
    const lib_i32 failed = test_video_registration_configure_rollback(adapter,
        config, LIB_STATUS_NO_MEMORY) || !rejection_reached ||
        submitted_ports != (reject_memory ? 0u : port_count) ||
        published_memory != (reject_memory ? 0u : 1u) || removed_memory != 1u ||
        (!reject_memory && removed_owner != published_owner);

    rejected_dependency = 0u;
    return failed;
}

lib_i32 test_video_registration_initialize(t_vadp *adapter,
    x86_core_machine *machine, lib_status expected)
{
    const lib_status status = x86_nxvm_board_vadp_initialize(adapter, machine);

    return status != expected ||
        (expected == LIB_STATUS_NO_MEMORY && adapter->chip != LIB_NULL);
}

void test_video_registration_finalize(t_vadp *adapter)
{
    x86_nxvm_board_vadp_finalize(adapter);
}

lib_bool test_video_registration_unconfigured(const t_vadp *adapter)
{
    return !adapter->configured &&
        !x86_video_ega_aperture_contains(adapter->chip, 0xa0000u, 1u);
}

lib_i32 test_video_registration_configure_rollback(t_vadp *adapter,
    const x86_nxvm_board_display_config *config, lib_status expected)
{
    x86_video *original = adapter->chip;
    const lib_status status = x86_nxvm_board_vadp_configure(adapter, config);

    return status != expected || adapter->chip != original;
}

lib_i32 main(void)
{
    t_vadp adapter;

    return test_video_registration_routes(&adapter);
}
