#ifndef CORE_MACHINE_BOARD_PROFILE_INTERFACE_H
#define CORE_MACHINE_BOARD_PROFILE_INTERFACE_H

#include "lib/types/types_interface.h"
#include "core/x86/machine_interface.h"
#include "core/chips/pit825x/pit825x_interface.h"

/* Existing serialized board reset boundaries, in execution order. */
typedef enum core_machine_board_reset_phase {
    CORE_MACHINE_BOARD_RESET_BEFORE_DEVICES,
    CORE_MACHINE_BOARD_RESET_PORT_LATCHES,
    CORE_MACHINE_BOARD_RESET_AFTER_PIT,
    CORE_MACHINE_BOARD_RESET_FINAL_REFRESH
} core_machine_board_reset_phase;

/* One frozen board-specific attachment, never a device registry. Composition
 * transfers its lifetime only after successful construction. Core remains
 * the sole clock/route owner; callbacks neither run a worker nor advance time.
 * finalize runs before borrowed board devices are destroyed. */
typedef struct core_machine_board_profile_binding {
    void *context;
    void (*reset)(void *context, core_machine_board_reset_phase phase);
    void (*refresh_nmi)(void *context);
    lib_bool (*refresh_request)(void *context, lib_u8 *out_address);
    void (*refresh_complete)(void *context);
    lib_bool (*next_deadline)(void *context, lib_u64 now, lib_u64 *out_tick);
    void (*finalize)(void *context);
    lib_bool owns_refresh_output;
    lib_bool shutdown_resets;
} core_machine_board_profile_binding;

/* Scoped construction inputs, not a runtime board getter. Handles are
 * borrowed; a successfully bound Profile must release its routes and sinks
 * before the common board destroys these devices. */
typedef struct core_machine_board_profile_services {
    core_machine *core;
    x86_pit *pit;
    x86_pit *auxiliary_pit;
    void (*speaker)(void *context, lib_u8 lines);
    void *speaker_context;
} core_machine_board_profile_services;

/* On failure leave out_binding empty and release the unpublished candidate.
 * The context is used only for construction, never retained by the board. */
typedef lib_status (*core_machine_board_profile_factory)(
    const core_machine_board_profile_services *services, void *context,
    core_machine_board_profile_binding *out_binding);

#endif
