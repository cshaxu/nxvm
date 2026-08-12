#ifndef VM_SESSION_INTERFACE_H
#define VM_SESSION_INTERFACE_H

#include "type.h"

#include "core/machine/cpu_interface.h"
#include "core/machine/fpu_interface.h"



typedef struct vm_session_config {
    STD_SIZE_T memory_bytes;
    const C_CHAR *fdd_image;
    const C_CHAR *hdd_image;
    C_INT create_fdd;
    type_unsigned_16 create_hdd_cylinders;
    C_INT boot_hdd;
    core_machine_cpu_profile cpu_profile;
    core_machine_fpu_profile fpu_profile;
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
C_INT vm_session_insert_fdd(vm_session *session, const C_CHAR *path);
C_INT vm_session_insert_hdd(vm_session *session, const C_CHAR *path);
C_INT vm_session_get_reset_vector(const vm_session *session,
    vm_session_reset_vector *out_vector);

#endif

