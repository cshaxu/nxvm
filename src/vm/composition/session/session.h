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
#include "core/machine/fdc.h"
#include "vm/machine/fdd.h"
#include "core/machine/hdc.h"
#include "vm/machine/hdd.h"
#include "vm/platform/platform.h"
#include "vm/platform/virtual_time.h"
#include "vm/platform/vm_request_transport.h"
#include "vm/profile/default_profile/firmware/bios.h"
#include "vm/profile/default_profile/firmware/context.h"
#include "vm/profile/default_profile/pc_at_profile.h"
#include "vm/profile/device/floppy.h"
#include "vm/profile/model40/model40.h"

typedef enum vm_session_boot_preference {
    VM_SESSION_BOOT_PREFERENCE_AUTO,
    VM_SESSION_BOOT_PREFERENCE_FDD,
    VM_SESSION_BOOT_PREFERENCE_HDD
} vm_session_boot_preference;

typedef struct vm_session_start_outcome {
    C_INT valid;
    type_status status;
} vm_session_start_outcome;

struct vm_session {
    C_INT active;
    vm_platform_request_transport request_transport;
    core_machine_config core_machine_config;
    const vm_profile_default_pc_at_descriptor *profile;
    core_machine *core_machine;
    core_machine_dma_request_binding fdc_dma_request;
    t_fdd fdd;
    t_hdd hdd;
    t_debug debug;
    t_bios default_bios;
    vm_profile_default_context default_profile_context;
    core_machine_media_registry media_registry;
    core_machine_display_provider_slot display_provider;
    core_platform_presentation_mailbox presentation_mailbox;
    vm_platform_execution_transport execution_transport;
    core_platform_input_source input_source;
    core_utils_wait_scope wait_scope;
    vm_platform_run_context platform_run_context;
    vm_platform_run_handle platform_run_handle;
    vm_session_start_outcome start_outcome;
    core_product_debug_context debugger_context;
    type_unsigned_64 display_generation;
    core_product_debug_target *debug_target;
    vm_session_fault_outcome fault_outcome;
    vm_session_control_state control;
    vm_session_config retained_config;
    vm_session_virtual_time_source virtual_time_source;
    vm_platform_virtual_time_source model_339_virtual_time_source;
    vm_session_boot_preference boot_preference;
    C_INT model40_private;
    vm_profile_floppy_kind floppy_kind;
    vm_profile_model40_external_rom model40_rom;
    type_unsigned_8 model40_even_rom[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    type_unsigned_8 model40_odd_rom[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    vm_profile_model40_d4_memory model40_d4_memory;
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
type_status vm_session_create_model40_private(
    const vm_profile_model40_external_rom *rom, vm_session **out_session);

#endif
