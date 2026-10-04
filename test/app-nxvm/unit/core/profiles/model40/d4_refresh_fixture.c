#include "d4_refresh_fixture.h"
#include "app-nxvm/profiles/model40/d4_platform.h"

void test_model40_refresh_set_pending(core_machine_d4_platform *platform)
{
    platform->d4_refresh_hold_pending = LIB_TRUE;
}

lib_bool test_model40_refresh_is_pending(const core_machine_d4_platform *platform)
{
    return platform->d4_refresh_hold_pending;
}

lib_bool test_model40_refresh_hold_matches(const core_machine_d4_platform *platform,
    lib_bool pending, lib_u8 address)
{
    return (platform->d4_refresh_hold_pending != LIB_FALSE) == pending &&
        platform->d4_refresh_address == address;
}

lib_bool test_model40_refresh_reset_is_clear(const core_machine_d4_platform *platform)
{
    return !(platform->d4_refresh_hold_pending || platform->d4_refresh_pulse_active ||
        platform->d4_refresh_address != 0u);
}
