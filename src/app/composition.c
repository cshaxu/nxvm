#include "type.h"

#include "app/composition.h"
#include "core/machine/machine_interface.h"
#include "app/config.h"
#include "core/machine/frame.h"

struct vm_app {
    vm_machine *machine;
    common_machine *common_machine;
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
    common_machine_shutdown(app->common_machine);
    common_ui_destroy(app->ui);
    common_session_destroy(app->session);
    common_machine_destroy(app->common_machine);
    (C_VOID)vm_machine_bind_common_machine(app->machine, LIB_NULL);
    vm_machine_destroy(app->machine);
    STD_FREE(app);
}

common_session *vm_app_session(vm_app *app)
{ return app == STD_NULL ? LIB_NULL : app->session; }

vm_machine *vm_app_machine(vm_app *app)
{ return app == STD_NULL ? STD_NULL : app->machine; }

common_machine *vm_app_common_machine(vm_app *app)
{ return app == STD_NULL ? LIB_NULL : app->common_machine; }

common_ui *vm_app_ui(vm_app *app)
{ return app == STD_NULL ? LIB_NULL : app->ui; }

type_status vm_app_compose_machine(vm_app *app, const vm_session_request *request)
{
    vm_machine_config config;
    common_machine_driver driver;
    vm_machine *machine = STD_NULL;
    common_machine *common_machine = LIB_NULL;

    if (app == STD_NULL || request == STD_NULL || app->machine != STD_NULL)
        return TYPE_STATUS_INVALID_STATE;
    if (vm_app_configure_machine(request, &config) != TYPE_STATUS_OK ||
        vm_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        vm_machine_describe_common_driver(machine, &driver) != TYPE_STATUS_OK ||
        common_machine_create(&common_machine, &driver) != LIB_STATUS_OK ||
        vm_machine_bind_common_machine(machine, common_machine) != TYPE_STATUS_OK) {
        (C_VOID)vm_machine_bind_common_machine(machine, LIB_NULL);
        common_machine_destroy(common_machine);
        vm_machine_destroy(machine);
        return TYPE_STATUS_INVALID_STATE;
    }
    app->machine = machine;
    app->common_machine = common_machine;
    return TYPE_STATUS_OK;
}

type_status vm_app_compose_control(vm_app *app,
    const common_session_options *options)
{
    common_session_options resolved;
    common_session *session = LIB_NULL;

    if (app == STD_NULL || options == LIB_NULL || app->machine == STD_NULL ||
        app->session != LIB_NULL) return TYPE_STATUS_INVALID_STATE;
    resolved = *options;
    resolved.machine = app->common_machine;
    if (common_session_create(&session, &resolved) != LIB_STATUS_OK)
        return TYPE_STATUS_INVALID_STATE;
    common_machine_set_state_sink(resolved.machine, vm_app_machine_state_completed,
        session);
    common_machine_set_frame_sink(resolved.machine, vm_app_machine_frame_published,
        session);
    app->session = session;
    return TYPE_STATUS_OK;
}

type_status vm_app_compose_ui(vm_app *app, const common_ui_options *options)
{
    common_ui *ui = LIB_NULL;

    if (app == STD_NULL || options == LIB_NULL || app->ui != LIB_NULL)
        return TYPE_STATUS_INVALID_STATE;
    if (common_ui_create(&ui, options) != LIB_STATUS_OK ||
        common_session_bind_ui(app->session, ui) != LIB_STATUS_OK) {
        common_ui_destroy(ui);
        return TYPE_STATUS_INVALID_STATE;
    }
    app->ui = ui;
    return TYPE_STATUS_OK;
}
