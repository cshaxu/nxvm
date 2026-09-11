#ifndef VM_APP_H
#define VM_APP_H

#include "type.h"

#include "common/session/session_interface.h"
typedef struct vm_app vm_app;
typedef struct vm_session_request vm_session_request;

typedef enum vm_app_speed {
    VM_APP_SPEED_STANDARD,
    VM_APP_SPEED_TURBO
} vm_app_speed;

type_status vm_app_create(vm_app **out_app);
C_VOID vm_app_destroy(vm_app *app);
common_session *vm_app_session(vm_app *app);
type_status vm_app_open_profile(vm_app *app, const vm_session_request *request);
type_status vm_app_set_presentation_target(vm_app *app,
    common_session_target target);
C_INT vm_app_is_running(const vm_app *app);
C_VOID vm_app_print_machine(const vm_app *app);
C_VOID vm_app_print_bios(const vm_app *app);
C_VOID vm_app_print_status(const vm_app *app);
type_status vm_app_get_speed(const vm_app *app, vm_app_speed *out_speed);
type_status vm_app_set_speed(vm_app *app, vm_app_speed speed);
type_status vm_app_debug(vm_app *app);
type_status vm_app_record_start(vm_app *app, const C_CHAR *path);
type_status vm_app_record_stop(vm_app *app);
C_INT vm_app_insert_fdd(vm_app *app, const C_CHAR *path);
C_INT vm_app_remove_fdd(vm_app *app, const C_CHAR *path);
type_status vm_app_start(vm_app *app, common_session_plan *out_plan);
type_status vm_app_reset(vm_app *app);
type_status vm_app_stop(vm_app *app);
type_status vm_app_resume(vm_app *app, common_session_plan *out_plan);
type_status vm_app_request_pause(vm_app *app);
type_status vm_app_submit_ui_input(vm_app *app, const ui_input_event *event);
type_status vm_app_submit_key(vm_app *app, type_unsigned_16 scan_code,
    type_unsigned_16 virtual_key, C_INT pressed);
type_status vm_app_reduce_input(vm_app *app, const ui_input_event *event,
    common_session_plan *out_plan);

#endif
