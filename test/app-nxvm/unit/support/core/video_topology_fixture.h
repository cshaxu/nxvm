#ifndef TEST_CORE_VIDEO_TOPOLOGY_FIXTURE_H
#define TEST_CORE_VIDEO_TOPOLOGY_FIXTURE_H
#include "core/x86/memory_interface.h"

lib_bool test_video_missing_write_port(core_machine *machine, lib_u16 port);
lib_i32 test_video_cga_ports(core_machine *machine);
lib_i32 test_video_ega_ports(core_machine *machine);
lib_i32 test_video_memory_route(core_machine *machine, lib_u32 physical,
    core_machine_memory_route expected);
#endif
