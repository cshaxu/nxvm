#ifndef NTVDM64_VM_SESSION_H
#define NTVDM64_VM_SESSION_H

#include "type.h"



#include "core/machine/cpu.h"

#include "core/machine/machine_interface.h"

#include "core/machine/cpu_instructions.h"

#include "core/machine/port.h"

#include "core/machine/memory.h"

#include "core/machine/pic.h"

#include "core/machine/pit.h"

#include "core/machine/dma.h"

#include "core/machine/kbc.h"

#include "core/machine/vadp.h"

#include "core/machine/block_provider.h"

#include "core/machine/keyboard_interface.h"

#include "core/machine/display_interface.h"

#include "vm/machine/cmos.h"

#include "vm/machine/fdd.h"

#include "vm/machine/fdc.h"

#include "vm/machine/hdd.h"

#include "vm/machine/debug.h"

#include "vm/profile/default_profile/firmware/bios.h"

#include "vm/profile/default_profile/firmware/qdx.h"

#include "vm/profile/default_profile/firmware/context.h"

#include "vm/composition/session/control.h"

#include "core/product/debug/debug_target.h"

#include "core/product/debug/debug.h"

#include "core/product/wait_provider.h"

#include "vm/platform/platform.h"
#include "vm/platform/vm_request_transport.h"


typedef struct vm_session_config {
    STD_SIZE_T memory_bytes;
    const C_CHAR *fdd_image;
    const C_CHAR *hdd_image;
    C_INT create_fdd;
    uint16_t create_hdd_cylinders;
    C_INT boot_hdd;
    core_machine_cpu_profile cpu_profile;
    core_machine_fpu_profile fpu_profile;
} vm_session_config;

typedef struct vm_session {
    C_INT active;
    vm_platform_request_transport request_transport;
    core_machine_config core_machine_config;
    core_machine *core_machine;
    t_cmos cmos;
    t_fdd fdd;
    t_fdc fdc;
    t_hdd hdd;
    t_debug debug;
    t_bios default_bios_storage;
    t_qdx default_qdx_storage;
    vm_profile_default_context default_profile_context_storage;
    core_machine_block_provider_slot block_provider_storage;
    core_machine_keyboard_provider_slot keyboard_provider_storage;
    core_machine_display_provider_slot display_provider_storage;
    vm_platform_presentation_mailbox presentation_mailbox;
    vm_platform_execution_transport execution_transport;
    vm_platform_keyboard_transport keyboard_transport;
    core_product_wait_scope wait_scope;
    vm_platform_run_context platform_run_context;
    vm_platform_run_handle platform_run_handle;
    core_product_debug_context debugger_context;
    uint64_t display_generation;
    core_product_debug_target *debug_target;
    t_bios *default_bios;
    t_qdx *default_qdx;
    vm_profile_default_context *default_profile_context;
    core_machine_block_provider_slot *block_provider;
    core_machine_keyboard_provider_slot *keyboard_provider;
    core_machine_display_provider_slot *display_provider;
    vm_session_control_state *control;
    vm_session_config retained_config;
    C_CHAR fdd_image_path[1024];
    C_CHAR hdd_image_path[1024];
} vm_session;

typedef struct vm_session_reset_vector {
    uint16_t cs;
    uint16_t ip;
} vm_session_reset_vector;

C_VOID vm_session_storage_initialize(vm_session *machine);
C_VOID vm_session_storage_finalize(vm_session *machine);
C_INT vm_session_enqueue_keyboard_state(C_VOID *opaque,
    uint32_t asynchronous_keys, uint32_t toggle_keys);
C_VOID vm_session_consume_request(C_VOID *opaque,
    const nxvm_platform_vm_request *request);
C_INT vm_session_create(const vm_session_config *config, vm_session **out_session);
C_VOID vm_session_destroy(vm_session *session);
ntvdm64_status vm_session_reconfigure_memory(vm_session *session,
    STD_SIZE_T memory_bytes);
C_INT vm_session_insert_fdd(vm_session *session, const C_CHAR *path);
C_INT vm_session_insert_hdd(vm_session *session, const C_CHAR *path);
C_INT vm_session_get_reset_vector(const vm_session *session,
    vm_session_reset_vector *out_vector);

#endif
