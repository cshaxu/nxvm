#include "lib/types/types_interface.h"

#include "app-nxvm/product/composition.h"
#include "app-nxvm/machine/machine_interface.h"
#include "app-nxvm/product/config.h"
#include "app-nxvm/machine/frame.h"

struct vm_app {
    vm_machine *machine;
    common_machine *common_machine;
    common_session *session;
    common_ui *ui;
};

static lib_status vm_app_status_from_lib(lib_status status)
{
    if (status == LIB_STATUS_OK) return LIB_STATUS_OK;
    if (status == LIB_STATUS_INVALID_ARGUMENT) return LIB_STATUS_INVALID_ARGUMENT;
    if (status == LIB_STATUS_INVALID_STATE) return LIB_STATUS_INVALID_STATE;
    if (status == LIB_STATUS_UNSUPPORTED) return LIB_STATUS_UNSUPPORTED;
    if (status == LIB_STATUS_NO_MEMORY) return LIB_STATUS_NO_MEMORY;
    return LIB_STATUS_INTERNAL_ERROR;
}

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

static void vm_app_machine_state_completed(void *context,
    common_machine_state state, lib_u32 run_generation)
{
    (void)common_session_enqueue_runtime_completed((common_session *)context,
        vm_app_machine_state(state), run_generation);
}

static void vm_app_machine_frame_published(void *context, lib_u32 sequence,
    lib_bool graphics, lib_u32 run_generation)
{
    (void)common_session_enqueue_frame_completed((common_session *)context,
        sequence, graphics, run_generation);
}

lib_status vm_app_create(vm_app **out_app)
{
    vm_app *app;

    if (out_app == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_app = LIB_NULL;
    app = lib_allocate_zero(1u, sizeof(*app));
    if (app == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    *out_app = app;
    return LIB_STATUS_OK;
}

lib_status vm_app_destroy(vm_app *app)
{
    lib_status shutdown_status;

    if (app == LIB_NULL) return LIB_STATUS_OK;
    shutdown_status = common_machine_shutdown(app->common_machine);
    if (shutdown_status != LIB_STATUS_OK)
        return vm_app_status_from_lib(shutdown_status);
    common_ui_destroy(app->ui);
    common_session_destroy(app->session);
    shutdown_status = common_machine_destroy(app->common_machine);
    if (shutdown_status != LIB_STATUS_OK)
        return vm_app_status_from_lib(shutdown_status);
    (void)vm_machine_bind_common_machine(app->machine, LIB_NULL);
    vm_machine_destroy(app->machine);
    lib_release(app);
    return LIB_STATUS_OK;
}

common_session *vm_app_session(const vm_app *app)
{ return app == LIB_NULL ? LIB_NULL : app->session; }

vm_machine *vm_app_machine(const vm_app *app)
{ return app == LIB_NULL ? LIB_NULL : app->machine; }

common_machine *vm_app_common_machine(const vm_app *app)
{ return app == LIB_NULL ? LIB_NULL : app->common_machine; }

common_ui *vm_app_ui(const vm_app *app)
{ return app == LIB_NULL ? LIB_NULL : app->ui; }

lib_status vm_app_compose_machine(vm_app *app, const vm_session_request *request)
{
    vm_machine_config config;
    common_machine_driver driver;
    vm_machine *machine = LIB_NULL;
    common_machine *common_machine = LIB_NULL;
    lib_status status;

    if (app == LIB_NULL || request == LIB_NULL || app->machine != LIB_NULL)
        return LIB_STATUS_INVALID_STATE;
    status = vm_app_configure_machine(request, &config);
    if (status == LIB_STATUS_OK) {
        status = vm_machine_create(&config, &machine);
    }
    if (status == LIB_STATUS_OK) {
        status = vm_machine_describe_common_driver(machine, &driver);
    }
    if (status == LIB_STATUS_OK) {
        status = vm_app_status_from_lib(common_machine_create(&common_machine,
            &driver));
    }
    if (status == LIB_STATUS_OK) {
        status = vm_machine_bind_common_machine(machine, common_machine);
    }
    if (status != LIB_STATUS_OK) {
        (void)vm_machine_bind_common_machine(machine, LIB_NULL);
        common_machine_destroy(common_machine);
        vm_machine_destroy(machine);
        return status;
    }
    app->machine = machine;
    app->common_machine = common_machine;
    return LIB_STATUS_OK;
}

lib_status vm_app_compose_control(vm_app *app,
    const common_session_options *options)
{
    common_session_options resolved;
    common_session *session = LIB_NULL;
    lib_status status;

    if (app == LIB_NULL || options == LIB_NULL || app->machine == LIB_NULL ||
        app->session != LIB_NULL) return LIB_STATUS_INVALID_STATE;
    resolved = *options;
    resolved.machine = app->common_machine;
    status = common_session_create(&session, &resolved);
    if (status != LIB_STATUS_OK) return vm_app_status_from_lib(status);
    common_machine_set_state_sink(resolved.machine, vm_app_machine_state_completed,
        session);
    common_machine_set_frame_sink(resolved.machine, vm_app_machine_frame_published,
        session);
    app->session = session;
    return LIB_STATUS_OK;
}

lib_status vm_app_compose_ui(vm_app *app, const common_ui_options *options)
{
    common_ui *ui = LIB_NULL;
    lib_status status;

    if (app == LIB_NULL || options == LIB_NULL || app->ui != LIB_NULL)
        return LIB_STATUS_INVALID_STATE;
    status = common_ui_create(&ui, options);
    if (status == LIB_STATUS_OK) status = common_session_bind_ui(app->session, ui);
    if (status != LIB_STATUS_OK) {
        common_ui_destroy(ui);
        return vm_app_status_from_lib(status);
    }
    app->ui = ui;
    return LIB_STATUS_OK;
}
