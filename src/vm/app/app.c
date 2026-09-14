#include "type.h"

#include "vm/app/app.h"
#include "vm/machine/runtime/machine_interface.h"
#include "vm/app/request_factory.h"
#include "vm/machine/runtime/frame.h"

struct vm_app {
    vm_machine *machine;
    common_session *session;
    common_ui *ui;
};

static common_session_machine_state vm_app_machine_state(common_machine_state state)
{
    switch (state) {
    case COMMON_MACHINE_RUNNING: return COMMON_SESSION_MACHINE_RUNNING;
    case COMMON_MACHINE_PAUSED: return COMMON_SESSION_MACHINE_PAUSED;
    case COMMON_MACHINE_RESET_COMPLETED:
        return COMMON_SESSION_MACHINE_RESET_COMPLETED;
    case COMMON_MACHINE_ERROR: return COMMON_SESSION_MACHINE_ERROR;
    default: return COMMON_SESSION_MACHINE_STOPPED;
    }
}

static C_VOID vm_app_machine_state_completed(C_VOID *context,
    common_machine_state state, lib_u32 run_generation)
{
    (C_VOID)common_session_enqueue_runtime_completed((common_session *)context,
        vm_app_machine_state(state), run_generation);
}

static C_VOID vm_app_machine_frame_published(C_VOID *context, lib_u32 sequence,
    lib_bool graphics, lib_u32 run_generation)
{
    (C_VOID)common_session_enqueue_frame_completed((common_session *)context,
        sequence, graphics, run_generation);
}

type_status vm_app_create(vm_app **out_app)
{
    vm_app *app;

    if (out_app == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_app = STD_NULL;
    app = STD_CALLOC(1u, sizeof(*app));
    if (app == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    *out_app = app;
    return TYPE_STATUS_OK;
}

C_VOID vm_app_destroy(vm_app *app)
{
    if (app == STD_NULL) return;
    common_ui_destroy(app->ui);
    vm_machine_destroy(app->machine);
    common_session_destroy(app->session);
    STD_FREE(app);
}

common_session *vm_app_session(vm_app *app)
{ return app == STD_NULL ? LIB_NULL : app->session; }

vm_machine *vm_app_machine(vm_app *app)
{ return app == STD_NULL ? STD_NULL : app->machine; }

common_ui *vm_app_ui(vm_app *app)
{ return app == STD_NULL ? LIB_NULL : app->ui; }

type_status vm_app_compose_machine(vm_app *app, const vm_session_request *request)
{
    if (app == STD_NULL || request == STD_NULL || app->machine != STD_NULL)
        return TYPE_STATUS_INVALID_STATE;
    if (vm_machine_create_from_request(request, &app->machine) != TYPE_STATUS_OK)
        return TYPE_STATUS_INVALID_STATE;
    return TYPE_STATUS_OK;
}

type_status vm_app_compose_control(vm_app *app,
    const common_session_options *options)
{
    common_session_options resolved;

    if (app == STD_NULL || options == LIB_NULL || app->machine == STD_NULL ||
        app->session != LIB_NULL) return TYPE_STATUS_INVALID_STATE;
    resolved = *options;
    resolved.machine = vm_machine_common_machine(app->machine);
    if (common_session_create(&app->session, &resolved) != LIB_STATUS_OK)
        return TYPE_STATUS_INVALID_STATE;
    common_machine_set_state_sink(resolved.machine, vm_app_machine_state_completed,
        app->session);
    common_machine_set_frame_sink(resolved.machine, vm_app_machine_frame_published,
        app->session);
    return TYPE_STATUS_OK;
}

type_status vm_app_compose_ui(vm_app *app, const common_ui_options *options)
{
    if (app == STD_NULL || options == LIB_NULL || app->ui != LIB_NULL)
        return TYPE_STATUS_INVALID_STATE;
    if (common_ui_create(&app->ui, options) != LIB_STATUS_OK ||
        common_session_bind_ui(app->session, app->ui) != LIB_STATUS_OK) {
        common_ui_destroy(app->ui);
        app->ui = LIB_NULL;
        return TYPE_STATUS_INVALID_STATE;
    }
    return TYPE_STATUS_OK;
}
