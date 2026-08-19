#ifndef VM_SESSION_MODEL40_COMPOSITION_H
#define VM_SESSION_MODEL40_COMPOSITION_H

#include "type.h"

typedef struct vm_session vm_session;

type_status vm_session_model40_storage_initialize(vm_session *session);
type_status vm_session_model40_configure_controllers(vm_session *session);
C_INT vm_session_model40_insert_hdd_at_startup(vm_session *session, const C_CHAR *path);
C_INT vm_session_model40_insert_hdd_slave_at_startup(vm_session *session, const C_CHAR *path);

#endif