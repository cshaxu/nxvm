#ifndef VM_SESSION_H
#define VM_SESSION_H

#include "vm/composition/session/session_interface.h"

#include "core/machine/machine_interface.h"
#include "core/machine/media_interface.h"
#include "core/machine/display_interface.h"
#include "core/platform/input_interface.h"
#include "core/platform/presentation_mailbox_interface.h"
#include "core/product/debug/debug.h"
#include "core/product/debug/debug_target.h"
#include "core/utils/wait_provider.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/fault.h"
#include "vm/composition/session/model40_composition.h"
#include "vm/machine/debug.h"
#include "vm/machine/fdd_private.h"
#include "vm/machine/hdd_private.h"
#include "vm/platform/platform.h"
#include "vm/platform/vm_request_transport.h"
#include "vm/profile/default_profile/firmware/bios.h"
#include "vm/profile/default_profile/firmware/context.h"
#include "vm/profile/default_profile/pc_at_profile_private.h"
#include "vm/profile/device/floppy.h"
#include "vm/profile/model40/model40_private.h"
#include "vm/profile/xt/xt_5160_268.h"

typedef enum vm_session_boot_preference {
    VM_SESSION_BOOT_PREFERENCE_AUTO,
    VM_SESSION_BOOT_PREFERENCE_FDD,
    VM_SESSION_BOOT_PREFERENCE_HDD
} vm_session_boot_preference;

typedef enum vm_session_firmware_kind {
    VM_SESSION_FIRMWARE_DEFAULT_PC_AT,
    VM_SESSION_FIRMWARE_MODEL40_BYOB,
    VM_SESSION_FIRMWARE_XT_BYOB
} vm_session_firmware_kind;

typedef struct vm_session_start_outcome {
    C_INT valid;
    type_status status;
} vm_session_start_outcome;

struct vm_session {
    C_INT active;
    vm_platform_request_transport *request_transport;
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
    t_fdd fdd;
    t_hdd hdd;
    t_debug debug;
    t_bios default_bios;
    vm_profile_default_context default_profile_context;
    core_machine_media_registry *media_registry;
    core_machine_display_provider_slot *display_provider;
    core_platform_presentation_mailbox *presentation_mailbox;
    vm_platform_execution_transport *execution_transport;
    core_platform_input_source *input_source;
    core_utils_wait_scope wait_scope;
    vm_platform_run_context *platform_run_context;
    vm_platform_run_handle *platform_run_handle;
    vm_session_start_outcome start_outcome;
    core_product_debugger *debugger;
    type_unsigned_64 display_generation;
    type_unsigned_64 display_snapshot_generation;
    type_bool display_snapshot_generation_valid;
    type_unsigned_64 last_display_publish_milliseconds;
    core_machine_display_kind display_kind;
    core_product_debug_target *debug_target;
    vm_session_fault_outcome fault_outcome;
    vm_session_control_state control;
    vm_session_speed speed;
    type_unsigned_64 pacing_host_origin_units;
    type_unsigned_64 pacing_host_units_per_second;
    type_unsigned_64 pacing_core_origin_ticks;
    type_bool pacing_origin_valid;
    vm_session_config retained_config;
    vm_session_boot_preference boot_preference;
    vm_session_firmware_kind firmware_kind;
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
    core_machine_fdc_terminal_observation model40_fdc_terminal_observation;
    type_bool model40_fdc_terminal_observation_valid;
    C_CHAR fdd_image_path[1024];
    C_CHAR hdd_image_path[1024];
};

type_status vm_session_storage_initialize(vm_session *machine);
C_VOID vm_session_storage_finalize(vm_session *machine);
C_VOID vm_session_apply_boot_preference(vm_session *session);
C_VOID vm_session_set_boot_hdd(vm_session *session, C_INT enabled);
C_INT vm_session_remove_fdd(vm_session *session, const C_CHAR *path);
C_VOID vm_session_consume_request(C_VOID *opaque,
    const vm_platform_request *request);
#endif
