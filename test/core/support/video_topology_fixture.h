#ifndef TEST_CORE_VIDEO_TOPOLOGY_FIXTURE_H
#define TEST_CORE_VIDEO_TOPOLOGY_FIXTURE_H
#include "x86-core/machine/memory_interface.h"

/* Operational bus probes; reset before testing power-on display state. */
lib_bool test_video_missing_write_port(x86_core_machine *machine, lib_u16 port);
lib_i32 test_video_cga_ports(x86_core_machine *machine);
lib_i32 test_video_ega_ports(x86_core_machine *machine);
lib_i32 test_video_memory_route(x86_core_machine *machine, lib_u32 physical,
    x86_core_machine_memory_route expected);
#endif
