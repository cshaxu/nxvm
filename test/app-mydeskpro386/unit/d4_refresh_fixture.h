#ifndef TEST_MODEL40_D4_REFRESH_FIXTURE_H
#define TEST_MODEL40_D4_REFRESH_FIXTURE_H
#include "app-mydeskpro386/d4_platform_interface.h"

lib_bool test_model40_refresh_hold_matches(const mydeskpro386_d4_platform *platform,
    lib_bool pending, lib_u8 address);
lib_bool test_model40_refresh_reset_is_clear(const mydeskpro386_d4_platform *platform);
void test_model40_refresh_set_pending(mydeskpro386_d4_platform *platform);
lib_bool test_model40_refresh_is_pending(const mydeskpro386_d4_platform *platform);
#endif
