#include "lib/types/types_interface.h"
#ifndef VM_MACHINE_H
#define VM_MACHINE_H

#include "x86/product/machine/machine_interface.h"

#include "x86/ibmpc-common/machine_board_interface.h"
#include "x86/ibmpc-common/media_interface.h"
#include "x86/ibmpc-common/display_interface.h"
#include "lib/base/sync_interface.h"
#include "x86/product/machine/control.h"
#include "x86/product/machine/fault.h"
#include "x86/product/machine/lifecycle.h"
#include "x86/product/machine/debug.h"
#include "x86/product/machine/media/fdd_interface.h"
#include "x86/product/machine/media/hdd_interface.h"
#include "common/machine/machine_interface.h"
#include "x86/ibmpc-common/floppy_interface.h"

struct vm_machine {
    lib_bool active;
    /* Non-owning App-composition link. */
    common_machine *executor;
    core_machine_plan *core_machine_plan;
    vm_machine_construction construction;
    core_machine *core_machine;
    /* Borrowed from the sole Core attachment lifetime. */
    core_machine_board_state *board;
    core_machine_dma_request_binding fdc_dma_request;
    union { t_fdd *fdd; t_fdd *floppy[VM_MACHINE_FLOPPY_SLOT_COUNT]; };
    union { t_hdd *hdd; t_hdd *fixed_disk[VM_MACHINE_FIXED_DISK_SLOT_COUNT]; };
    t_debug debug;
    core_machine_media_registry *media_registry;
    core_machine_display_provider_slot *display_provider;
    common_machine_executor_callback executor_callback;
    void *executor_callback_context;
    common_machine_frame latest_frame;
    lib_bool latest_frame_valid;
    lib_u64 display_generation;
    lib_u64 display_snapshot_generation;
    lib_bool display_snapshot_generation_valid;
    lib_u64 last_display_publish_milliseconds;
    x86_video_kind display_kind;
    vm_machine_fault_outcome fault_outcome;
    /* The bounded runner owns this single fact for the existing Common
     * boolean driver result.  It distinguishes an abnormal executor unwind
     * from an ordinary product stop without inventing a second Core fault. */
    lib_bool runner_failed;
    vm_machine_control_state control;
    vm_machine_speed speed;
    lib_u64 pacing_host_origin_units;
    lib_u64 pacing_host_units_per_second;
    lib_u64 pacing_core_origin_ticks;
    lib_bool pacing_origin_valid;
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
lib_bool vm_machine_copy_common_frame(vm_machine *machine, common_machine_frame *frame);
lib_status vm_machine_set_common_media(vm_machine *machine, const char *path,
    lib_storage_medium_mode mode);
#endif
