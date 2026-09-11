#ifndef VM_SESSION_SESSION_H
#define VM_SESSION_SESSION_H

#include "type.h"

#include "lib/ui-base/event_interface.h"
#include "vm/events/presentation_plan.h"
#include "vm/session/catalog.h"
#include "vm/session/control.h"

typedef struct vm_session vm_session;

typedef enum vm_session_speed {
    VM_SESSION_SPEED_STANDARD,
    VM_SESSION_SPEED_TURBO
} vm_session_speed;

type_status vm_session_create(vm_session **out_session);
C_VOID vm_session_destroy(vm_session *session);
vm_session_control *vm_session_get_control(vm_session *session);
type_status vm_session_open_profile(vm_session *session,
    const vm_session_request *request);
type_status vm_session_set_presentation_target(vm_session *session,
    vm_presentation_surface target);
C_INT vm_session_is_running(const vm_session *session);
C_VOID vm_session_print_machine(const vm_session *session);
C_VOID vm_session_print_bios(const vm_session *session);
C_VOID vm_session_print_status(const vm_session *session);
type_status vm_session_get_speed(const vm_session *session, vm_session_speed *out_speed);
type_status vm_session_set_speed(vm_session *session, vm_session_speed speed);
type_status vm_session_debug(vm_session *session);
type_status vm_session_record_start(vm_session *session, const C_CHAR *path);
type_status vm_session_record_stop(vm_session *session);
C_INT vm_session_insert_fdd(vm_session *session, const C_CHAR *path);
C_INT vm_session_remove_fdd(vm_session *session, const C_CHAR *path);
type_status vm_session_start(vm_session *session,
    vm_presentation_plan *out_plan);
type_status vm_session_reset(vm_session *session);
type_status vm_session_stop(vm_session *session);
type_status vm_session_resume(vm_session *session,
    vm_presentation_plan *out_plan);
type_status vm_session_request_pause(vm_session *session);
type_status vm_session_submit_ui_input(vm_session *session,
    const ui_input_event *event);
type_status vm_session_submit_key(vm_session *session, type_unsigned_16 scan_code,
    type_unsigned_16 virtual_key, C_INT pressed);
type_status vm_session_reduce_fact(vm_session *session,
    const vm_session_fact *fact, const vm_machine_display_event *display,
    vm_presentation_plan *out_plan);

#endif
