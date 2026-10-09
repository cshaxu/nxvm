#ifndef TEST_VIDEO_REGISTRATION_FIXTURE_H
#define TEST_VIDEO_REGISTRATION_FIXTURE_H
#include "core/board-base/vadp_interface.h"

/* Serialized test assertions at the video owner; Core remains opaque here. */
lib_i32 test_video_registration_initialize(t_vadp *adapter,
    core_machine *machine, lib_status expected);
void test_video_registration_finalize(t_vadp *adapter);
lib_bool test_video_registration_unconfigured(const t_vadp *adapter);
lib_i32 test_video_registration_configure_rollback(t_vadp *adapter,
    const core_machine_display_config *config, lib_status expected);
lib_i32 test_video_registration_routes(t_vadp *adapter);
#endif
