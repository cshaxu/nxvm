#ifndef VM_APP_COMPOSITION_H
#define VM_APP_COMPOSITION_H
#include "lib/types/types_interface.h"


#include "common/session/session_interface.h"
#include "common/ui/ui_interface.h"
#include "app-nxvm/machine/machine_interface.h"
#include "app-nxvm/product/request_interface.h"
typedef struct vm_app vm_app;

lib_status vm_app_create(vm_app **out_app);
lib_status vm_app_destroy(vm_app *app);
common_session *vm_app_session(const vm_app *app);
vm_machine *vm_app_machine(const vm_app *app);
common_machine *vm_app_common_machine(const vm_app *app);
common_ui *vm_app_ui(const vm_app *app);
lib_status vm_app_compose_machine(vm_app *app,
    const vm_session_request *request);
lib_status vm_app_compose_control(vm_app *app,
    const common_session_options *options);
lib_status vm_app_compose_ui(vm_app *app, const common_ui_options *options);

#endif
