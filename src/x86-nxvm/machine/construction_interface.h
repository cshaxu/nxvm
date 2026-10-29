#ifndef X86_NXVM_MACHINE_CONSTRUCTION_INTERFACE_H
#define X86_NXVM_MACHINE_CONSTRUCTION_INTERFACE_H

#include "lib/types/types_interface.h"
#include "lib/storage/medium_interface.h"
#include "lib/kvm-base/event_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/board-base/floppy_interface.h"
#include "x86-core/machine/firmware_interface.h"

#define X86_NXVM_MACHINE_FLOPPY_SLOT_COUNT 2u
#define X86_NXVM_MACHINE_FIXED_DISK_SLOT_COUNT 2u
#define X86_NXVM_MACHINE_CMOS_SEED_BYTES 64u
#define X86_NXVM_MACHINE_EXTENDED_CMOS_SEED_BYTES 128u
#define X86_NXVM_MACHINE_RTC_SEED_BYTES 14u

typedef enum x86_nxvm_machine_profile_event {
    X86_NXVM_MACHINE_PROFILE_RESET_COMPLETED,
    X86_NXVM_MACHINE_PROFILE_BOARD_DETACHED
} x86_nxvm_machine_profile_event;

/* Frozen integration operations, not per-field configuration callbacks.
 * configure runs once before Core publication. after_firmware runs once after
 * immutable firmware mapping but before execution providers freeze. notify
 * runs only after a successful full reset or after Core teardown. release
 * runs last, after Core routes, registries and media have ceased using the
 * context. */
typedef struct x86_nxvm_machine_profile_binding {
    void *context;
    lib_status (*configure)(void *context, x86_nxvm_board_plan *plan);
    lib_status (*after_firmware)(void *context, x86_core_machine *core);
    void (*notify)(void *context, x86_nxvm_machine_profile_event event);
    void (*release)(void *context);
    /* Optional physical input wiring, on the owning executor after publication.
     * True consumes the borrowed event; false retains normal delivery.
     * It must report physical-operation failures through the existing Core
     * fault path, not retain the event or create another input queue. */
    lib_bool (*handle_input)(void *context, const kvm_input_event *event);
} x86_nxvm_machine_profile_binding;

/* Prepared values are copied into the sole Machine owner. Firmware/context
 * pointers refer to the transferred Profile lifetime; there is no model ID.
 * CMOS supplies only configuration bytes beginning at 0Eh; Core owns live RTC
 * registers.  Classic boards accept 64 bytes, while an explicitly extended
 * board may accept the full 128-byte address space. */
typedef struct x86_nxvm_machine_construction {
    x86_nxvm_board_config core_config;
    x86_nxvm_board_controller_timing_rules timing_rules;
    x86_nxvm_board_plan_topology topology;
    x86_nxvm_board_floppy_kind floppy_kind;
    x86_nxvm_board_floppy_kind media_kind;
    lib_u8 floppy_slot_count;
    /* Physical fixed-disk units exposed by the selected board/HDC topology.
     * Config may name two slots, but it cannot create a slave unit that the
     * selected board does not wire. */
    lib_u8 fixed_disk_slot_count;
    lib_bool hdc_present;
    lib_bool memory_reconfigurable;
    lib_bool fixed_geometry;
    lib_u16 cylinders;
    lib_u8 heads;
    lib_u8 sectors;
    lib_u8 cmos_seed[X86_NXVM_MACHINE_EXTENDED_CMOS_SEED_BYTES];
    lib_size cmos_seed_bytes;
    lib_bool cmos_seed_present;
    /* Optional RTC register image; distinct from configuration-only CMOS. */
    lib_u8 rtc_seed[X86_NXVM_MACHINE_RTC_SEED_BYTES];
    lib_bool rtc_seed_present;
    x86_video_text_glyph_config text_glyphs;
    /* A non-VADP board supplies its one display snapshot source here.  The
     * Machine binds and freezes it in the existing board display slot. */
    x86_nxvm_board_display_snapshot_provider display_snapshot_provider;
    void *display_snapshot_context;
    const x86_core_machine_firmware_provider *firmware_provider;
    void *firmware_context;
    x86_nxvm_machine_profile_binding profile;
} x86_nxvm_machine_construction;

#endif
