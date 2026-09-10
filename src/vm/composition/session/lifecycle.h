/* Copyright 2012-2014 Neko. */

#ifndef VM_SESSION_LIFECYCLE_H
#define VM_SESSION_LIFECYCLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

#include "vm/composition/session/session_interface.h"
#include "core/machine/guest_display_frame.h"

#include "vm/composition/session/session_state.h"

type_status vm_session_start(vm_session *machine);
type_status vm_session_reset(vm_session *machine);
type_status vm_session_finish_reset(vm_session *machine, type_status status);
C_VOID vm_session_stop(vm_session *machine);
type_status vm_session_resume(vm_session *machine);
type_status vm_session_request_pause(vm_session *machine);
typedef C_VOID (*vm_session_lifecycle_reporter)(C_VOID *context,
    vm_session_lifecycle lifecycle);
typedef C_VOID (*vm_session_display_reporter)(C_VOID *context,
    const core_machine_guest_display_frame *frame);
void vm_session_set_lifecycle_reporter(vm_session *machine,
    vm_session_lifecycle_reporter reporter, C_VOID *context);
void vm_session_set_display_reporter(vm_session *machine,
    vm_session_display_reporter reporter, C_VOID *context);
void vm_session_report_lifecycle(vm_session *machine,
    vm_session_lifecycle lifecycle);

type_status vm_session_initialize(vm_session *machine);
C_VOID vm_session_finalize(vm_session *machine);

type_status vm_session_bind_execution_provider(vm_session *machine);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
