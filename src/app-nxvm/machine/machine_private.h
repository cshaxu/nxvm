#include "lib/types/types_interface.h"
#ifndef VM_MACHINE_H
#define VM_MACHINE_H

#include "app-nxvm/machine/machine_interface.h"

#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/media_interface.h"
#include "app-nxvm/devices/display_interface.h"
#include "lib/base/sync_interface.h"
#include "app-nxvm/machine/control.h"
#include "app-nxvm/machine/fault.h"
#include "app-nxvm/machine/lifecycle.h"
#include "app-nxvm/profiles/machine_plan_interface.h"
#include "app-nxvm/machine/debug.h"
#include "app-nxvm/machine/media/fdd_private.h"
#include "app-nxvm/machine/media/hdd_private.h"
#include "common/machine/machine_interface.h"
#include "app-nxvm/machine/event_interface.h"
#include "app-nxvm/profiles/device/floppy.h"

struct vm_machine {
    lib_i32 active;
    /* Non-owning App-composition link. */
    common_machine *executor;
    core_machine_config core_machine_config;
    core_machine_controller_timing_rules controller_timing_rules;
    core_machine_plan *core_machine_plan;
    vm_profile_machine_plan *profile_plan;
    core_machine *core_machine;
    core_machine_dma_request_binding fdc_dma_request;
    union { t_fdd fdd; t_fdd floppy[VM_MACHINE_FLOPPY_SLOT_COUNT]; };
    union { t_hdd hdd; t_hdd fixed_disk[VM_MACHINE_FIXED_DISK_SLOT_COUNT]; };
    t_debug debug;
    core_machine_media_registry *media_registry;
    core_machine_display_provider_slot *display_provider;
    common_machine_executor_callback executor_callback;
    void *executor_callback_context;
    common_machine_frame latest_frame;
    lib_u8 latest_frame_valid;
    lib_u64 display_generation;
    lib_u64 display_snapshot_generation;
    lib_u8 display_snapshot_generation_valid;
    lib_u64 last_display_publish_milliseconds;
    core_machine_display_kind display_kind;
    vm_machine_fault_outcome fault_outcome;
    /* The bounded runner owns this single fact for the existing Common
     * boolean driver result.  It distinguishes an abnormal executor unwind
     * from an ordinary product stop without inventing a second Core fault. */
    lib_u8 runner_failed;
    vm_machine_control_state control;
    vm_machine_speed speed;
    lib_u64 pacing_host_origin_units;
    lib_u64 pacing_host_units_per_second;
    lib_u64 pacing_core_origin_ticks;
    lib_u8 pacing_origin_valid;
    vm_machine_config retained_config;
    lib_u8 cmos_seed[VM_MACHINE_CMOS_SEED_BYTES];
    lib_u8 cmos_seed_present;
    core_machine_vadp_text_glyph_config text_glyphs;
    vm_profile_floppy_kind floppy_kind;
    vm_profile_floppy_kind fdd_media_kind;
    core_machine_fdc_terminal_observation model40_fdc_terminal_observation;
    lib_u8 model40_fdc_terminal_observation_valid;
    union { char fdd_image_path[1024];
        char floppy_image_path[VM_MACHINE_FLOPPY_SLOT_COUNT][1024]; };
    union { char hdd_image_path[1024];
        char fixed_disk_image_path[VM_MACHINE_FIXED_DISK_SLOT_COUNT][1024]; };
};

lib_status vm_machine_storage_initialize(vm_machine *machine);
lib_status vm_machine_apply_cmos_seed(const vm_machine *session,
    core_machine_plan_topology *topology);
void vm_machine_storage_finalize(vm_machine *machine);
lib_status vm_machine_deliver_common_input(vm_machine *machine,
    const kvm_input_event *event);
lib_u8 vm_machine_copy_common_frame(vm_machine *machine, common_machine_frame *frame);
lib_status vm_machine_set_common_media(vm_machine *machine, const char *path,
    lib_storage_medium_mode mode);
#endif
