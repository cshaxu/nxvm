#ifndef MYDESKPRO386_D4_PLATFORM_INTERFACE_H
#define MYDESKPRO386_D4_PLATFORM_INTERFACE_H

#include "x86-core/machine/machine_interface.h"
#include "core/chips/pit825x/pit825x_interface.h"
#include "core/board-at/kbc_interface.h"
#include "core/board-base/board_profile_interface.h"
#include "app-mydeskpro386/d4_memory_interface.h"

typedef struct mydeskpro386_d4_platform mydeskpro386_d4_platform;
typedef struct core_machine_board_state core_machine_board_state;
typedef struct mydeskpro386_d4_platform_config {
    lib_u16 port;
    lib_u8 failsafe_pit_counter;
    lib_u8 speed_pit_counter;
    lib_u8 speed_pit_count;
} mydeskpro386_d4_platform_config;

typedef struct mydeskpro386_d4_platform_observation {
    lib_i32 configured;
    lib_i32 iochk_enabled;
    lib_i32 failsafe_enabled;
    lib_i32 iochk_latched;
    lib_i32 failsafe_latched;
    lib_i32 nmi_signaled;
    lib_bool memory_configured;
    lib_u8 memory_control;
    lib_u16 memory_ram_setup;
} mydeskpro386_d4_platform_observation;

typedef void (*mydeskpro386_d4_speaker_output)(void *context, lib_u8 lines);

/* Profile-owned state, serialized with Core execution. Core, PITs and output context are borrowed until
 * destroy. Destroy before those providers, after dispatch has stopped. */
lib_status mydeskpro386_d4_platform_create(x86_core_machine *core, x86_pit *pit,
    x86_pit *auxiliary_pit, const mydeskpro386_d4_platform_config *config,
    mydeskpro386_d4_speaker_output speaker, void *context,
    mydeskpro386_d4_platform **out_platform);
/* Construction transfers ownership to the board only on success. */
lib_status mydeskpro386_d4_platform_attach(core_machine_board_state *board,
    const mydeskpro386_d4_platform_config *config,
    mydeskpro386_d4_platform **out_platform);
void mydeskpro386_d4_platform_destroy(mydeskpro386_d4_platform *platform);
lib_status mydeskpro386_d4_platform_configure_memory(mydeskpro386_d4_platform *platform,
    const mydeskpro386_d4_memory_config *config);
core_machine_board_profile_binding mydeskpro386_d4_platform_binding(
    mydeskpro386_d4_platform *platform);
void mydeskpro386_d4_platform_reset_latches(mydeskpro386_d4_platform *platform);
void mydeskpro386_d4_platform_after_pit_reset(mydeskpro386_d4_platform *platform);
void mydeskpro386_d4_platform_reset_refresh(mydeskpro386_d4_platform *platform);
void mydeskpro386_d4_platform_refresh_nmi(mydeskpro386_d4_platform *platform);
lib_bool mydeskpro386_d4_platform_refresh_request(void *owner, lib_u8 *address);
void mydeskpro386_d4_platform_refresh_complete(void *owner);
lib_bool mydeskpro386_d4_platform_cpu_hold_asserted(void *owner);
x86_kbc8042_command_extension_result mydeskpro386_d4_platform_kbc_command(
    void *owner, lib_u8 command, lib_u8 input_port, lib_u8 command_byte,
    lib_u8 *out_data);
lib_status mydeskpro386_d4_platform_clear_iochk(mydeskpro386_d4_platform *platform);
lib_status mydeskpro386_d4_platform_report_iochk(mydeskpro386_d4_platform *platform);
void mydeskpro386_d4_platform_iochk_output(void *owner, lib_bool asserted);
lib_status mydeskpro386_d4_platform_observe(const mydeskpro386_d4_platform *platform,
    mydeskpro386_d4_platform_observation *observation);

#endif
