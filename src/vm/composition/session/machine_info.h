/* Copyright 2012-2014 Neko. */

#ifndef NTVDM64_VM_SESSION_MACHINE_INFO_H
#define NTVDM64_VM_SESSION_MACHINE_INFO_H

#include "type.h"

typedef struct vm_session vm_session;

C_VOID vm_session_print_machine(const vm_session *session);

#endif
