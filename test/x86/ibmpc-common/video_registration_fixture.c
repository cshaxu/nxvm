#include "video_registration_fixture.h"
#include "x86/ibmpc-common/vadp.h"

lib_i32 test_video_registration_initialize(t_vadp *adapter,
    core_machine *machine, lib_status expected)
{
    const lib_status status = core_machine_vadp_initialize(adapter, machine);

    return status != expected ||
        (expected == LIB_STATUS_NO_MEMORY && adapter->chip != LIB_NULL);
}

void test_video_registration_finalize(t_vadp *adapter)
{
    core_machine_vadp_finalize(adapter);
}

lib_bool test_video_registration_unconfigured(const t_vadp *adapter)
{
    return !adapter->configured &&
        !x86_video_ega_aperture_contains(adapter->chip, 0xa0000u, 1u);
}

lib_i32 test_video_registration_configure_rollback(t_vadp *adapter,
    const core_machine_display_config *config, lib_status expected)
{
    x86_video *original = adapter->chip;
    const lib_status status = core_machine_vadp_configure(adapter, config);

    return status != expected || adapter->chip != original;
}

lib_i32 main(void)
{
    t_vadp adapter;

    return test_video_registration_routes(&adapter);
}
