#ifndef VM_MACHINE_H
#define VM_MACHINE_H

#include "vm/machine/runtime/machine_interface.h"

#include "core/machine/machine_interface.h"
#include "core/machine/media_interface.h"
#include "core/machine/display_interface.h"
#include "core/machine/guest_input_interface.h"
#include "core/machine/guest_presentation_mailbox_interface.h"
#include "lib/host/sync_interface.h"
#include "vm/machine/runtime/control.h"
#include "vm/machine/runtime/fault.h"
#include "vm/machine/runtime/lifecycle.h"
#include "vm/machine/runtime/model40_composition.h"
#include "vm/machine/runtime/rom/external_pc_at.h"
#include "vm/machine/debug.h"
#include "vm/media/fdd_private.h"
#include "vm/media/hdd_private.h"
#include "common/machine/machine_interface.h"
#include "vm/machine/runtime/event_interface.h"
#include "vm/profile/default_profile/pc_at_profile_private.h"
#include "vm/profile/device/floppy.h"
#include "vm/profile/model40/model40_private.h"
#include "vm/profile/xt/xt_5160_268.h"

typedef enum vm_machine_firmware_kind {
    VM_MACHINE_FIRMWARE_EXTERNAL_PC_AT_ROM,
    VM_MACHINE_FIRMWARE_MODEL40_BYOB,
    VM_MACHINE_FIRMWARE_XT_BYOB
} vm_machine_firmware_kind;

struct vm_machine {
    C_INT active;
    vm_machine_result_sink result_sink;
    C_VOID *result_sink_context;
    common_machine *executor;
    core_machine_config core_machine_config;
    core_machine_controller_timing_rules controller_timing_rules;
    core_machine_plan *core_machine_plan;
    const vm_profile_default_pc_at_descriptor *profile;
    const core_machine_plan_topology *profile_topology;
    vm_profile_default_pc_at_resolved_profile ibm_5170_root;
    vm_profile_default_pc_at_resolved_profile default_at_resolved;
    vm_resolved_profile model40_resolved;
    core_machine *core_machine;
    core_machine_dma_request_binding fdc_dma_request;
    union { t_fdd fdd; t_fdd floppy[VM_MACHINE_FLOPPY_SLOT_COUNT]; };
    union { t_hdd hdd; t_hdd fixed_disk[VM_MACHINE_FIXED_DISK_SLOT_COUNT]; };
    t_debug debug;
    type_unsigned_8 pc_at_rom[VM_MACHINE_PC_AT_ROM_BYTES];
    type_unsigned_8 pc_at_video_rom[VM_MACHINE_PC_AT_VIDEO_ROM_MAX_BYTES];
    STD_SIZE_T pc_at_video_rom_bytes;
    type_bool pc_at_rom_external;
    vm_machine_external_pc_at_rom_context pc_at_rom_context;
    core_machine_media_registry *media_registry;
    core_machine_display_provider_slot *display_provider;
    core_machine_guest_presentation_mailbox *presentation_mailbox;
    core_machine_guest_input_source *input_source;
    host_sync_task *execution_task;
    host_sync_event *execution_started;
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
    vm_machine_firmware_kind firmware_kind;
    C_INT model40_private;
    C_INT xt_private;
    vm_profile_floppy_kind floppy_kind;
    vm_profile_floppy_kind fdd_media_kind;
    vm_profile_model40_external_rom model40_rom;
    type_unsigned_8 model40_even_rom[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    type_unsigned_8 model40_odd_rom[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    type_unsigned_8 model40_video_rom[VM_PROFILE_MODEL40_VIDEO_ROM_BYTES];
    vm_profile_xt_5160_268_resolved_profile xt_resolved;
    vm_profile_xt_5160_268_external_rom xt_rom;
    type_unsigned_8 *xt_system_rom;
    type_unsigned_8 *xt_xebec_rom;
    type_unsigned_8 *xt_video_rom;
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
C_VOID vm_machine_consume_request(C_VOID *opaque,
    const common_machine_request *request);
C_VOID vm_machine_publish_result(vm_machine *machine,
    const vm_machine_result *result);
#endif
