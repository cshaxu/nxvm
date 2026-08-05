#ifndef VM_SESSION_H
#define VM_SESSION_H

#include "vm/composition/session/session_interface.h"

#include "core/machine/machine_interface.h"
#include "core/machine/block_provider.h"
#include "core/machine/display_interface.h"
#include "core/product/debug/debug.h"
#include "core/product/debug/debug_target.h"
#include "core/utils/wait_provider.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/fault.h"
#include "vm/machine/cmos.h"
#include "vm/machine/debug.h"
#include "vm/machine/fdc.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdc.h"
#include "vm/machine/hdd.h"
#include "vm/platform/platform.h"
#include "vm/platform/vm_request_transport.h"
#include "vm/profile/default_profile/firmware/bios.h"
#include "vm/profile/default_profile/firmware/context.h"
#include "vm/profile/default_profile/pc_at_profile.h"

struct vm_session {
    C_INT active;
    vm_platform_request_transport request_transport;
    core_machine_config core_machine_config;
    const vm_profile_default_pc_at_descriptor *profile;
    core_machine *core_machine;
    t_cmos cmos;
    t_fdd fdd;
    t_fdc fdc;
    t_hdd hdd;
    vm_machine_hdc hdc;
    t_debug debug;
    t_bios default_bios;
    vm_profile_default_context default_profile_context;
    core_machine_block_provider_slot block_provider;
    core_machine_display_provider_slot display_provider;
    vm_platform_presentation_mailbox presentation_mailbox;
    vm_platform_execution_transport execution_transport;
    vm_platform_keyboard_transport keyboard_transport;
    vm_platform_mouse_transport mouse_transport;
    core_utils_wait_scope wait_scope;
    vm_platform_run_context platform_run_context;
    vm_platform_run_handle platform_run_handle;
    core_product_debug_context debugger_context;
    uint64_t display_generation;
    core_product_debug_target *debug_target;
    vm_session_fault_outcome fault_outcome;
    vm_session_control_state control;
    vm_session_config retained_config;
    C_CHAR fdd_image_path[1024];
    C_CHAR hdd_image_path[1024];
};

C_VOID vm_session_storage_initialize(vm_session *machine);
C_VOID vm_session_storage_finalize(vm_session *machine);
C_VOID vm_session_consume_request(C_VOID *opaque,
    const vm_platform_request *request);

#endif
