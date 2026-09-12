#ifndef VM_APP_H
#define VM_APP_H

#include "type.h"

#include "common/session/session_interface.h"
#include "common/ui/ui_interface.h"
#include "vm/machine/runtime/machine_interface.h"
#include "vm/machine/request_interface.h"
typedef struct vm_app vm_app;

type_status vm_app_create(vm_app **out_app);
C_VOID vm_app_destroy(vm_app *app);
common_session *vm_app_session(vm_app *app);
vm_machine *vm_app_machine(vm_app *app);
common_ui *vm_app_ui(vm_app *app);
type_status vm_app_compose_machine(vm_app *app,
    const vm_session_request *request);
type_status vm_app_compose_ui(vm_app *app, const common_ui_options *options);

#endif
