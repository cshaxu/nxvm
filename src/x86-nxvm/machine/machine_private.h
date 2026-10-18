#include "lib/types/types_interface.h"
#ifndef X86_NXVM_MACHINE_H
#define X86_NXVM_MACHINE_H

#include "x86-nxvm/machine/machine_interface.h"

#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/board-base/media_interface.h"
#include "x86-nxvm/board-base/display_interface.h"
#include "lib/base/sync_interface.h"
#include "x86-nxvm/machine/control.h"
#include "x86-nxvm/machine/fault.h"
#include "x86-nxvm/machine/lifecycle.h"
#include "x86-nxvm/machine/debug.h"
#include "x86-nxvm/machine/media/fdd_interface.h"
#include "x86-nxvm/machine/media/hdd_interface.h"
#include "emulator/machine/machine_interface.h"
#include "x86-nxvm/board-base/floppy_interface.h"

struct x86_nxvm_machine {
    lib_bool active;
    /* Non-owning composition link. */
    emulator_machine *executor;
    x86_nxvm_board_plan *x86_nxvm_board_plan;
    x86_nxvm_machine_construction construction;
    x86_core_machine *x86_core_machine;
    /* Borrowed from the sole Core attachment lifetime. */
    x86_nxvm_board_state *board;
    x86_nxvm_board_dma_request_binding fdc_dma_request;
    t_fdd *floppy[X86_NXVM_MACHINE_FLOPPY_SLOT_COUNT];
    t_hdd *fixed_disk[X86_NXVM_MACHINE_FIXED_DISK_SLOT_COUNT];
    t_debug debug;
    x86_nxvm_board_media_registry *media_registry;
    x86_nxvm_board_display_provider_slot *display_provider;
    emulator_machine_executor_callback executor_callback;
    void *executor_callback_context;
    x86_video_snapshot latest_snapshot;
    lib_bool display_frame_pending;
    lib_u64 display_generation;
    lib_u64 display_snapshot_generation;
    lib_bool display_snapshot_generation_valid;
    lib_u64 last_display_publish_milliseconds;
    x86_video_kind display_kind;
    x86_nxvm_machine_fault_outcome fault_outcome;
    /* The bounded runner owns this single fact for the existing Common
     * boolean driver result.  It distinguishes an abnormal executor unwind
     * from an ordinary product stop without inventing a second Core fault. */
    lib_bool runner_failed;
    x86_nxvm_machine_control_state control;
    x86_nxvm_machine_speed speed;
    lib_u64 pacing_host_origin_units;
    lib_u64 pacing_host_units_per_second;
    lib_u64 pacing_core_origin_ticks;
    lib_bool pacing_origin_valid;
    char floppy_image_path[X86_NXVM_MACHINE_FLOPPY_SLOT_COUNT][1024];
    char fixed_disk_image_path[X86_NXVM_MACHINE_FIXED_DISK_SLOT_COUNT][1024];
};

lib_status x86_nxvm_machine_storage_initialize(x86_nxvm_machine *machine);
lib_status x86_nxvm_machine_apply_cmos_seed(const x86_nxvm_machine *session,
    x86_nxvm_board_plan_topology *topology);
void x86_nxvm_machine_storage_finalize(x86_nxvm_machine *machine);
lib_status x86_nxvm_machine_deliver_emulator_input(x86_nxvm_machine *machine,
    const kvm_input_event *event);
lib_status x86_nxvm_machine_copy_emulator_frame(x86_nxvm_machine *machine,
    kvm_window_frame_storage *frame, kvm_console_text_frame *console, lib_bool *produced);
lib_status x86_nxvm_machine_set_emulator_media(x86_nxvm_machine *machine, const char *path,
    lib_storage_medium_mode mode);
#endif
