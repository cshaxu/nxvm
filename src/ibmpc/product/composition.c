#include "lib/types/types_interface.h"

#include "ibmpc/product/composition_interface.h"
#include "ibmpc/product/composition.h"

struct vm_app {
    vm_app_factory factory;
    void *machine;
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

lib_status vm_app_create(const vm_app_factory *factory, vm_app **out_app)
{
    vm_app *app;

    if (out_app == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_app = LIB_NULL;
    if (factory == LIB_NULL || factory->prepare == LIB_NULL ||
        factory->bind == LIB_NULL || factory->destroy == LIB_NULL ||
        factory->information == LIB_NULL || factory->get_speed == LIB_NULL ||
        factory->set_speed == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    app = lib_allocate_zero(1u, sizeof(*app));
    if (app == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    app->factory = *factory;
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
    shutdown_status = common_ui_destroy(app->ui);
    if (shutdown_status != LIB_STATUS_OK)
        return vm_app_status_from_lib(shutdown_status);
    app->ui = LIB_NULL;
    common_session_destroy(app->session);
    app->session = LIB_NULL;
    shutdown_status = common_machine_destroy(app->common_machine);
    if (shutdown_status != LIB_STATUS_OK)
        return vm_app_status_from_lib(shutdown_status);
    app->common_machine = LIB_NULL;
    (void)app->factory.bind(app->machine, LIB_NULL);
    app->factory.destroy(app->machine);
    lib_release(app);
    return LIB_STATUS_OK;
}

common_session *vm_app_session(const vm_app *app)
{ return app == LIB_NULL ? LIB_NULL : app->session; }

common_machine *vm_app_common_machine(const vm_app *app)
{ return app == LIB_NULL ? LIB_NULL : app->common_machine; }

common_ui *vm_app_ui(const vm_app *app)
{ return app == LIB_NULL ? LIB_NULL : app->ui; }

lib_status vm_app_compose_machine(vm_app *app, const vm_session_request *request)
{
    common_machine_driver driver;
    void *machine = LIB_NULL;
    common_machine *common_machine = LIB_NULL;
    lib_status status;

    if (app == LIB_NULL || request == LIB_NULL || app->machine != LIB_NULL)
        return LIB_STATUS_INVALID_STATE;
    status = app->factory.prepare(app->factory.context, request, &machine, &driver);
    if (status == LIB_STATUS_OK) {
        status = vm_app_status_from_lib(common_machine_create(&common_machine,
            &driver));
    }
    if (status == LIB_STATUS_OK) {
        status = app->factory.bind(machine, common_machine);
    }
    if (status != LIB_STATUS_OK) {
        (void)app->factory.bind(machine, LIB_NULL);
        lib_status cleanup_status = common_machine_destroy(common_machine);

        if (cleanup_status != LIB_STATUS_OK) {
            app->machine = machine;
            app->common_machine = common_machine;
            return vm_app_status_from_lib(cleanup_status);
        }
        app->factory.destroy(machine);
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
    if (status != LIB_STATUS_OK) return vm_app_status_from_lib(status);
    app->ui = ui;
    status = common_session_bind_ui(app->session, ui);
    if (status != LIB_STATUS_OK) {
        lib_status cleanup_status = common_ui_destroy(ui);

        if (cleanup_status != LIB_STATUS_OK)
            return vm_app_status_from_lib(cleanup_status);
        app->ui = LIB_NULL;
        return vm_app_status_from_lib(status);
    }
    return LIB_STATUS_OK;
}

lib_status vm_app_information_read(const vm_app *app, vm_app_information *out_info)
{
    return app == LIB_NULL || app->machine == LIB_NULL ? LIB_STATUS_INVALID_STATE :
        app->factory.information(app->factory.context, app->machine, out_info);
}

lib_status vm_app_speed_read(const vm_app *app, vm_app_speed *out_speed)
{
    return app == LIB_NULL || app->machine == LIB_NULL ? LIB_STATUS_INVALID_STATE :
        app->factory.get_speed(app->machine, out_speed);
}

lib_status vm_app_speed_write(vm_app *app, vm_app_speed speed)
{
    return app == LIB_NULL || app->machine == LIB_NULL ? LIB_STATUS_INVALID_STATE :
        app->factory.set_speed(app->machine, speed);
}
