#ifndef VM_MACHINE_CONSTRUCTION_INTERFACE_H
#define VM_MACHINE_CONSTRUCTION_INTERFACE_H

#include "lib/types/types_interface.h"
#include "lib/storage/medium_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include "ibmpc/board-common/floppy_interface.h"
#include "x86/core/firmware_interface.h"

#define VM_MACHINE_FLOPPY_SLOT_COUNT 2u
#define VM_MACHINE_FIXED_DISK_SLOT_COUNT 2u
#define VM_MACHINE_CMOS_SEED_BYTES 64u

typedef enum vm_machine_profile_event {
    VM_MACHINE_PROFILE_RESET_COMPLETED,
    VM_MACHINE_PROFILE_BOARD_DETACHED
} vm_machine_profile_event;

/* Frozen integration operations, not per-field configuration callbacks.
 * configure runs once before Core publication. notify runs only after a
 * successful full reset or after Core teardown. release runs last, after
 * Core routes, registries and media have ceased using the context. */
typedef struct vm_machine_profile_binding {
    void *context;
    lib_status (*configure)(void *context, core_machine_plan *plan);
    void (*notify)(void *context, vm_machine_profile_event event);
    void (*release)(void *context);
} vm_machine_profile_binding;

/* Prepared values are copied into the sole Machine owner. Firmware/context
 * pointers refer to the transferred Profile lifetime; there is no model ID.
 * CMOS supplies only 0Eh-3Fh configuration; Core owns live RTC registers. */
typedef struct vm_machine_construction {
    core_machine_config core_config;
    core_machine_controller_timing_rules timing_rules;
    core_machine_plan_topology topology;
    vm_profile_floppy_kind floppy_kind;
    vm_profile_floppy_kind media_kind;
    lib_u8 floppy_slot_count;
    lib_bool hdc_present;
    lib_bool memory_reconfigurable;
    lib_bool fixed_geometry;
    lib_u16 cylinders;
    lib_u8 heads;
    lib_u8 sectors;
    lib_u8 cmos_seed[VM_MACHINE_CMOS_SEED_BYTES];
    lib_bool cmos_seed_present;
    x86_video_text_glyph_config text_glyphs;
    const core_machine_firmware_provider *firmware_provider;
    void *firmware_context;
    vm_machine_profile_binding profile;
} vm_machine_construction;

#endif
