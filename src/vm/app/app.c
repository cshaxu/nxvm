#include "type.h"

#include "vm/app/app.h"
#include "vm/machine/runtime/machine_interface.h"
#include "vm/machine/request_factory.h"
#include "vm/machine/runtime/frame.h"

struct vm_app {
    vm_machine *machine;
    common_session *session;
};

static common_session_machine_state vm_app_machine_state(vm_machine_result_kind kind)
{
    switch (kind) {
    case VM_MACHINE_RESULT_RUNNING: return COMMON_SESSION_MACHINE_RUNNING;
    case VM_MACHINE_RESULT_PAUSED: return COMMON_SESSION_MACHINE_PAUSED;
    case VM_MACHINE_RESULT_RESET: return COMMON_SESSION_MACHINE_RESET;
    case VM_MACHINE_RESULT_STOPPED: return COMMON_SESSION_MACHINE_STOPPED;
    default: return COMMON_SESSION_MACHINE_FAULT;
    }
}

static C_VOID vm_app_machine_result(void *context, const vm_machine_result *result)
{
    vm_app *app = context;
    ui_frame frame;

    if (app == STD_NULL || result == STD_NULL) return;
    if (result->kind == VM_MACHINE_RESULT_DISPLAY) {
        if (vm_machine_frame_from_display(&result->value.display, &frame) ==
            TYPE_STATUS_OK) (C_VOID)common_session_publish_frame(app->session, &frame);
        return;
    }
    (C_VOID)common_session_publish_machine(app->session,
        vm_app_machine_state(result->kind), (lib_status)result->status);
}

type_status vm_app_create(vm_app **out_app)
{
    vm_app *app;

    if (out_app == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_app = STD_NULL;
    app = STD_CALLOC(1u, sizeof(*app));
    if (app == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    if (common_session_create(&app->session) != LIB_STATUS_OK) {
        STD_FREE(app);
        return TYPE_STATUS_NO_MEMORY;
    }
    *out_app = app;
    return TYPE_STATUS_OK;
}

C_VOID vm_app_destroy(vm_app *app)
{
    if (app == STD_NULL) return;
    vm_machine_destroy(app->machine);
    common_session_destroy(app->session);
    STD_FREE(app);
}

common_session *vm_app_session(vm_app *app)
{ return app == STD_NULL ? LIB_NULL : app->session; }

vm_machine *vm_app_machine(vm_app *app)
{ return app == STD_NULL ? STD_NULL : app->machine; }

type_status vm_app_compose_machine(vm_app *app, const vm_session_request *request)
{
    if (app == STD_NULL || request == STD_NULL || app->machine != STD_NULL)
        return TYPE_STATUS_INVALID_STATE;
    if (vm_machine_create_from_request(request, &app->machine) != TYPE_STATUS_OK)
        return TYPE_STATUS_INVALID_STATE;
    vm_machine_set_result_sink(app->machine, vm_app_machine_result, app);
    return TYPE_STATUS_OK;
}
