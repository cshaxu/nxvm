#include "lib/types/types_interface.h"

#include "product/surface/composition_interface.h"

struct product_surface {
    app_composed_machine machine;
    common_machine *common_machine;
    common_session *session;
    common_ui *ui;
};

static lib_status product_surface_status_from_lib(lib_status status)
{
    if (status == LIB_STATUS_OK) return LIB_STATUS_OK;
    if (status == LIB_STATUS_INVALID_ARGUMENT) return LIB_STATUS_INVALID_ARGUMENT;
    if (status == LIB_STATUS_INVALID_STATE) return LIB_STATUS_INVALID_STATE;
    if (status == LIB_STATUS_UNSUPPORTED) return LIB_STATUS_UNSUPPORTED;
    if (status == LIB_STATUS_NO_MEMORY) return LIB_STATUS_NO_MEMORY;
    return LIB_STATUS_INTERNAL_ERROR;
}

static common_session_machine_state product_surface_machine_state(common_machine_state state)
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

static void product_surface_machine_state_completed(void *context,
    common_machine_state state, lib_u32 run_generation)
{
    (void)common_session_enqueue_runtime_completed((common_session *)context,
        product_surface_machine_state(state), run_generation);
}

static void product_surface_machine_frame_published(void *context, lib_u32 sequence,
    lib_bool graphics, lib_u32 run_generation)
{
    (void)common_session_enqueue_frame_completed((common_session *)context,
        sequence, graphics, run_generation);
}

lib_status product_surface_create(const app_composed_machine *machine,
    product_surface **out_app)
{
    product_surface *app;

    if (out_app == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_app = LIB_NULL;
    if (machine == LIB_NULL || machine->machine == LIB_NULL ||
        machine->bind == LIB_NULL || machine->destroy == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    app = lib_allocate_zero(1u, sizeof(*app));
    if (app == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    app->machine = *machine;
    *out_app = app;
    return LIB_STATUS_OK;
}

lib_status product_surface_destroy(product_surface *app)
{
    lib_status shutdown_status;

    if (app == LIB_NULL) return LIB_STATUS_OK;
    shutdown_status = common_machine_shutdown(app->common_machine);
    if (shutdown_status != LIB_STATUS_OK)
        return product_surface_status_from_lib(shutdown_status);
    shutdown_status = common_ui_destroy(app->ui);
    if (shutdown_status != LIB_STATUS_OK)
        return product_surface_status_from_lib(shutdown_status);
    app->ui = LIB_NULL;
    common_session_destroy(app->session);
    app->session = LIB_NULL;
    shutdown_status = common_machine_destroy(app->common_machine);
    if (shutdown_status != LIB_STATUS_OK)
        return product_surface_status_from_lib(shutdown_status);
    app->common_machine = LIB_NULL;
    (void)app->machine.bind(app->machine.machine, LIB_NULL);
    app->machine.destroy(app->machine.machine);
    lib_release(app);
    return LIB_STATUS_OK;
}

common_session *product_surface_session(const product_surface *app)
{ return app == LIB_NULL ? LIB_NULL : app->session; }

common_machine *product_surface_common_machine(const product_surface *app)
{ return app == LIB_NULL ? LIB_NULL : app->common_machine; }

common_ui *product_surface_ui(const product_surface *app)
{ return app == LIB_NULL ? LIB_NULL : app->ui; }

lib_status product_surface_compose_machine(product_surface *app)
{
    common_machine_driver driver;
    void *machine = LIB_NULL;
    common_machine *common_machine = LIB_NULL;
    lib_status status;

    if (app == LIB_NULL || app->common_machine != LIB_NULL)
        return LIB_STATUS_INVALID_STATE;
    machine = app->machine.machine;
    driver = app->machine.driver;
    status = product_surface_status_from_lib(common_machine_create(&common_machine,
        &driver));
    if (status == LIB_STATUS_OK) {
        status = app->machine.bind(machine, common_machine);
    }
    if (status != LIB_STATUS_OK) {
        (void)app->machine.bind(machine, LIB_NULL);
        lib_status cleanup_status = common_machine_destroy(common_machine);

        if (cleanup_status != LIB_STATUS_OK) {
            app->common_machine = common_machine;
            return product_surface_status_from_lib(cleanup_status);
        }
        return status;
    }
    app->common_machine = common_machine;
    return LIB_STATUS_OK;
}

lib_status product_surface_compose_control(product_surface *app,
    const common_session_options *options)
{
    common_session_options resolved;
    common_session *session = LIB_NULL;
    lib_status status;

    if (app == LIB_NULL || options == LIB_NULL ||
        app->common_machine == LIB_NULL ||
        app->session != LIB_NULL) return LIB_STATUS_INVALID_STATE;
    resolved = *options;
    resolved.machine = app->common_machine;
    status = common_session_create(&session, &resolved);
    if (status != LIB_STATUS_OK) return product_surface_status_from_lib(status);
    common_machine_set_state_sink(resolved.machine, product_surface_machine_state_completed,
        session);
    common_machine_set_frame_sink(resolved.machine, product_surface_machine_frame_published,
        session);
    app->session = session;
    return LIB_STATUS_OK;
}

lib_status product_surface_compose_ui(product_surface *app, const common_ui_options *options)
{
    common_ui *ui = LIB_NULL;
    lib_status status;

    if (app == LIB_NULL || options == LIB_NULL || app->ui != LIB_NULL)
        return LIB_STATUS_INVALID_STATE;
    status = common_ui_create(&ui, options);
    if (status != LIB_STATUS_OK) return product_surface_status_from_lib(status);
    app->ui = ui;
    status = common_session_bind_ui(app->session, ui);
    if (status != LIB_STATUS_OK) {
        lib_status cleanup_status = common_ui_destroy(ui);

        if (cleanup_status != LIB_STATUS_OK)
            return product_surface_status_from_lib(cleanup_status);
        app->ui = LIB_NULL;
        return product_surface_status_from_lib(status);
    }
    return LIB_STATUS_OK;
}

lib_status product_surface_information_read(const product_surface *app, product_surface_information *out_info)
{
    return app == LIB_NULL || app->machine.machine == LIB_NULL ? LIB_STATUS_INVALID_STATE :
        app->machine.information == LIB_NULL ? LIB_STATUS_UNSUPPORTED :
        app->machine.information(app->machine.context, app->machine.machine, out_info);
}

lib_status product_surface_speed_read(const product_surface *app, product_surface_speed *out_speed)
{
    return app == LIB_NULL || app->machine.machine == LIB_NULL ? LIB_STATUS_INVALID_STATE :
        app->machine.get_speed == LIB_NULL ? LIB_STATUS_UNSUPPORTED :
        app->machine.get_speed(app->machine.machine, out_speed);
}

lib_status product_surface_speed_write(product_surface *app, product_surface_speed speed)
{
    return app == LIB_NULL || app->machine.machine == LIB_NULL ? LIB_STATUS_INVALID_STATE :
        app->machine.set_speed == LIB_NULL ? LIB_STATUS_UNSUPPORTED :
        app->machine.set_speed(app->machine.machine, speed);
}
