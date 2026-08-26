#ifndef VM_SESSION_INTERFACE_H
#define VM_SESSION_INTERFACE_H

#include "type.h"

#include "core/machine/cpu_interface.h"
#include "core/machine/fpu_interface.h"
#include "core/platform/input_interface.h"
#include "vm/profile/model40/model40.h"
#include "vm/profile/xt/xt_5160_268.h"

typedef enum vm_session_profile_kind {
    VM_SESSION_PROFILE_DEFAULT_PC_AT,
    VM_SESSION_PROFILE_IBM_5170_MODEL_339,
    VM_SESSION_PROFILE_IBM_5160_MODEL_268,
    VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40
} vm_session_profile_kind;

typedef enum vm_session_speed {
    VM_SESSION_SPEED_STANDARD,
    VM_SESSION_SPEED_TURBO
} vm_session_speed;

const C_CHAR *vm_session_profile_name(vm_session_profile_kind kind);

typedef struct vm_session_config {
    vm_session_profile_kind profile_kind;
    STD_SIZE_T memory_bytes;
    const C_CHAR *fdd_image;
    const C_CHAR *hdd_image;
    const C_CHAR *hdd_slave_image;
    C_INT create_fdd;
    type_unsigned_16 create_hdd_cylinders;
    C_INT boot_hdd;
    core_machine_cpu_profile cpu_profile;
    core_machine_fpu_profile fpu_profile;
    vm_profile_model40_byob_manifest model40_firmware;
    vm_profile_xt_5160_268_byob_manifest xt_firmware;
} vm_session_config;

typedef struct vm_session vm_session;

typedef struct vm_session_reset_vector {
    type_unsigned_16 cs;
    type_unsigned_16 ip;
} vm_session_reset_vector;

C_INT vm_session_create(const vm_session_config *config, vm_session **out_session);
C_VOID vm_session_destroy(vm_session *session);
type_status vm_session_reconfigure_memory(vm_session *session,
    STD_SIZE_T memory_bytes);
type_status vm_session_get_speed(const vm_session *session,
    vm_session_speed *out_speed);
type_status vm_session_set_speed(vm_session *session, vm_session_speed speed);
C_INT vm_session_insert_fdd(vm_session *session, const C_CHAR *path);
C_INT vm_session_insert_hdd(vm_session *session, const C_CHAR *path);
/* Production host-input ingress.  Events are copied into the session's
 * ordered request transport; they do not mutate guest devices synchronously. */
type_status vm_session_submit_host_input(vm_session *session,
    const core_platform_input_event *event);
type_status vm_session_get_reset_vector(const vm_session *session,
    vm_session_reset_vector *out_vector);

#endif
