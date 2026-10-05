#ifndef CORE_MACHINE_D4_PLATFORM_INTERFACE_H
#define CORE_MACHINE_D4_PLATFORM_INTERFACE_H

#include "x86/core/machine_interface.h"
#include "x86/chips/pit825x/pit825x_interface.h"
#include "ibmpc/board-common/board_profile_interface.h"
#include "app-nxvm/profiles/model40/d4_memory_interface.h"

typedef struct core_machine_d4_platform core_machine_d4_platform;
typedef struct core_machine_board_state core_machine_board_state;
typedef struct core_machine_d4_platform_config {
    lib_u16 port;
    lib_u8 failsafe_pit_counter;
} core_machine_d4_platform_config;

typedef struct core_machine_d4_platform_observation {
    lib_i32 configured;
    lib_i32 iochk_enabled;
    lib_i32 failsafe_enabled;
    lib_i32 iochk_latched;
    lib_i32 failsafe_latched;
    lib_i32 nmi_signaled;
    lib_bool memory_configured;
    lib_u8 memory_control;
    lib_u16 memory_ram_setup;
} core_machine_d4_platform_observation;

typedef void (*core_machine_d4_speaker_output)(void *context, lib_u8 lines);

/* Profile-owned state, serialized with Core execution. Core, PITs and output context are borrowed until
 * destroy. Destroy before those providers, after dispatch has stopped. */
lib_status core_machine_d4_platform_create(core_machine *core, x86_pit *pit,
    x86_pit *auxiliary_pit, const core_machine_d4_platform_config *config,
    core_machine_d4_speaker_output speaker, void *context,
    core_machine_d4_platform **out_platform);
/* Construction transfers ownership to the board only on success. */
lib_status core_machine_d4_platform_attach(core_machine_board_state *board,
    const core_machine_d4_platform_config *config,
    core_machine_d4_platform **out_platform);
void core_machine_d4_platform_destroy(core_machine_d4_platform *platform);
lib_status core_machine_d4_platform_configure_memory(core_machine_d4_platform *platform,
    const core_machine_d4_memory_config *config);
core_machine_board_profile_binding core_machine_d4_platform_binding(
    core_machine_d4_platform *platform);
void core_machine_d4_platform_reset_latches(core_machine_d4_platform *platform);
void core_machine_d4_platform_after_pit_reset(core_machine_d4_platform *platform);
void core_machine_d4_platform_reset_refresh(core_machine_d4_platform *platform);
void core_machine_d4_platform_refresh_nmi(core_machine_d4_platform *platform);
lib_bool core_machine_d4_platform_refresh_request(void *owner, lib_u8 *address);
void core_machine_d4_platform_refresh_complete(void *owner);
lib_status core_machine_d4_platform_clear_iochk(core_machine_d4_platform *platform);
lib_status core_machine_d4_platform_report_iochk(core_machine_d4_platform *platform);
void core_machine_d4_platform_iochk_output(void *owner, lib_bool asserted);
lib_status core_machine_d4_platform_observe(const core_machine_d4_platform *platform,
    core_machine_d4_platform_observation *observation);

#endif
