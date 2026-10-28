#ifndef X86_NXVM_BOARD_PROFILE_INTERFACE_H
#define X86_NXVM_BOARD_PROFILE_INTERFACE_H

#include "lib/types/types_interface.h"
#include "x86-core/machine/machine_interface.h"
#include "x86-nxvm/chips/pit825x/pit825x_interface.h"
#include "x86-nxvm/board-at/kbc_interface.h"
#include "x86-nxvm/board-base/pic_bus_interface.h"

/* Existing serialized board reset boundaries, in execution order. */
typedef enum x86_nxvm_board_reset_phase {
    X86_NXVM_BOARD_RESET_BEFORE_DEVICES,
    X86_NXVM_BOARD_RESET_PORT_LATCHES,
    X86_NXVM_BOARD_RESET_AFTER_PIT,
    X86_NXVM_BOARD_RESET_FINAL_REFRESH
} x86_nxvm_board_reset_phase;

/* One frozen board-specific attachment, never a device registry. Composition
 * transfers its lifetime only after successful construction. Core remains
 * the sole clock/route owner; callbacks neither run a worker nor advance time.
 * finalize runs before borrowed board devices are destroyed. */
typedef struct x86_nxvm_board_profile_binding {
    void *context;
    void (*reset)(void *context, x86_nxvm_board_reset_phase phase);
    void (*refresh_nmi)(void *context);
    lib_bool (*refresh_request)(void *context, lib_u8 *out_address);
    void (*refresh_complete)(void *context);
    /* An asserted profile hold withholds CPU retirement while the existing
     * scheduler continues clock and DMA ownership. */
    lib_bool (*cpu_hold_asserted)(void *context);
    x86_nxvm_board_kbc_command_extension kbc_command_extension;
    lib_bool (*next_deadline)(void *context, lib_u64 now, lib_u64 *out_tick);
    void (*finalize)(void *context);
    lib_bool owns_refresh_output;
    lib_bool shutdown_resets;
    /* Forward the existing physical CPU output; the profile owns its decode. */
    void (*system_management_active)(void *context, lib_bool active);
} x86_nxvm_board_profile_binding;

/* Scoped construction inputs, not a runtime board getter. Handles are
 * borrowed; a successfully bound Profile must release its routes and sinks
 * before the common board destroys these devices. */
typedef struct x86_nxvm_board_profile_services {
    x86_core_machine *core;
    x86_pit *pit;
    x86_pit *auxiliary_pit;
    void (*speaker)(void *context, lib_u8 lines);
    void *speaker_context;
    /* Borrowed PIC endpoints; IRQ sources remain owned by the board. */
    x86_nxvm_board_pic_bus *pic_master;
    x86_nxvm_board_pic_bus *pic_slave;
} x86_nxvm_board_profile_services;

/* On failure leave out_binding empty and release the unpublished candidate.
 * The context is used only for construction, never retained by the board. */
typedef lib_status (*x86_nxvm_board_profile_factory)(
    const x86_nxvm_board_profile_services *services, void *context,
    x86_nxvm_board_profile_binding *out_binding);

#endif
