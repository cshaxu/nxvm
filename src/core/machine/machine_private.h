#ifndef VM_MACHINE_H
#define VM_MACHINE_H

#include "core/machine/machine_interface.h"

#include "core/devices/machine_interface.h"
#include "core/devices/media_interface.h"
#include "core/devices/display_interface.h"
#include "lib/base/sync_interface.h"
#include "core/machine/control.h"
#include "core/machine/fault.h"
#include "core/machine/lifecycle.h"
#include "core/profiles/machine_plan_interface.h"
#include "core/machine/debug.h"
#include "core/machine/media/fdd_private.h"
#include "core/machine/media/hdd_private.h"
#include "common/machine/machine_interface.h"
#include "core/machine/event_interface.h"
#include "core/profiles/device/floppy.h"

struct vm_machine {
    C_INT active;
    /* Non-owning App-composition link. */
    common_machine *executor;
    core_machine_config core_machine_config;
    core_machine_controller_timing_rules controller_timing_rules;
    core_machine_plan *core_machine_plan;
    vm_profile_machine_plan *profile_plan;
    /* Read-only descriptor projection for legacy diagnostics.  The selected
     * plan remains the sole owner; Machine never resolves or mutates it. */
    const vm_profile_default_pc_at_descriptor *profile;
    core_machine *core_machine;
    core_machine_dma_request_binding fdc_dma_request;
    union { t_fdd fdd; t_fdd floppy[VM_MACHINE_FLOPPY_SLOT_COUNT]; };
    union { t_hdd hdd; t_hdd fixed_disk[VM_MACHINE_FIXED_DISK_SLOT_COUNT]; };
    t_debug debug;
    core_machine_media_registry *media_registry;
    core_machine_display_provider_slot *display_provider;
    common_machine_executor_callback executor_callback;
    C_VOID *executor_callback_context;
    common_machine_frame latest_frame;
    type_bool latest_frame_valid;
    struct {
        type_unsigned_64 sequence;
        type_status status;
        type_bool valid;
    } start_outcome;
    type_unsigned_64 display_generation;
    type_unsigned_64 display_snapshot_generation;
    type_bool display_snapshot_generation_valid;
    type_unsigned_64 last_display_publish_milliseconds;
    core_machine_display_kind display_kind;
    vm_machine_fault_outcome fault_outcome;
    vm_machine_control_state control;
    vm_machine_speed speed;
    type_unsigned_64 pacing_host_origin_units;
    type_unsigned_64 pacing_host_units_per_second;
    type_unsigned_64 pacing_core_origin_ticks;
    type_bool pacing_origin_valid;
    vm_machine_config retained_config;
    type_unsigned_8 cmos_seed[VM_MACHINE_CMOS_SEED_BYTES];
    type_bool cmos_seed_present;
    core_machine_vadp_text_glyph_config text_glyphs;
    vm_profile_floppy_kind floppy_kind;
    vm_profile_floppy_kind fdd_media_kind;
    core_machine_fdc_terminal_observation model40_fdc_terminal_observation;
    type_bool model40_fdc_terminal_observation_valid;
    union { C_CHAR fdd_image_path[1024];
        C_CHAR floppy_image_path[VM_MACHINE_FLOPPY_SLOT_COUNT][1024]; };
    union { C_CHAR hdd_image_path[1024];
        C_CHAR fixed_disk_image_path[VM_MACHINE_FIXED_DISK_SLOT_COUNT][1024]; };
};

type_status vm_machine_storage_initialize(vm_machine *machine);
type_status vm_machine_apply_cmos_seed(const vm_machine *session,
    core_machine_plan_topology *topology);
C_VOID vm_machine_storage_finalize(vm_machine *machine);
type_status vm_machine_deliver_common_input(vm_machine *machine,
    const kvm_input_event *event);
type_bool vm_machine_copy_common_frame(vm_machine *machine, common_machine_frame *frame);
type_status vm_machine_set_common_media(vm_machine *machine, const C_CHAR *path,
    lib_storage_medium_mode mode);
#endif
