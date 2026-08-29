/* Copyright 2012-2014 Neko. */

#ifndef VM_SESSION_PROVIDER_LIFECYCLE_H
#define VM_SESSION_PROVIDER_LIFECYCLE_H

#include "type.h"

typedef struct vm_session vm_session;

type_status vm_session_provider_lifecycle_initialize(vm_session *session);
C_VOID vm_session_provider_lifecycle_reset(vm_session *session);
C_VOID vm_session_provider_lifecycle_finalize(vm_session *session);

#endif
