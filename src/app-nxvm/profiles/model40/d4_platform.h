#ifndef CORE_MACHINE_D4_PLATFORM_H
#define CORE_MACHINE_D4_PLATFORM_H

#include "app-nxvm/profiles/model40/d4_platform_interface.h"
#include "app-nxvm/profiles/model40/d4_memory.h"

struct core_machine_d4_platform {
    core_machine *core;
    core_machine_d4_memory memory;
    x86_pit *shared_pit;
    x86_pit *auxiliary_pit;
    lib_bool outputs_bound;
    core_machine_d4_platform_config d4_platform_config;
    lib_u8 d4_platform_port_b;
    lib_u8 d4_platform_iochk_latched;
    lib_u8 d4_platform_failsafe_latched;
    lib_u8 d4_platform_nmi_signaled;
    lib_u8 d4_refresh_hold_pending;
    lib_u8 d4_refresh_pulse_active;
    lib_u8 d4_refresh_address;
    core_machine_d4_speaker_output speaker;
    void *speaker_context;
};

#endif
