#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "product/surface/composition_interface.h"

struct fixture_machine { common_machine *bound; lib_bool live; };
struct common_machine { lib_bool live; };
struct common_session { common_ui *ui; lib_bool live; };
struct common_ui { lib_bool live; };

typedef enum composition_failure {
    COMPOSITION_FAILURE_NONE,
    COMPOSITION_FAILURE_COMMON_MACHINE_CREATE,
    COMPOSITION_FAILURE_MACHINE_BIND,
    COMPOSITION_FAILURE_SESSION_CREATE,
    COMPOSITION_FAILURE_UI_CREATE,
    COMPOSITION_FAILURE_UI_BIND
} composition_failure;

typedef struct composition_fixture {
    composition_failure failure;
    struct fixture_machine machine;
    struct common_machine common_machine;
    struct common_session session;
    struct common_ui ui;
    lib_u32 machine_destroy_count;
    lib_u32 common_machine_destroy_count;
    lib_u32 session_destroy_count;
    lib_u32 ui_destroy_count;
    lib_status shutdown_status;
    lib_status ui_destroy_status;
    lib_status machine_destroy_status;
} composition_fixture;

static composition_fixture fixture;

static void composition_fixture_reset(composition_failure failure)
{
    lib_memory_set(&fixture, 0, sizeof(fixture));
    fixture.failure = failure;
    fixture.shutdown_status = LIB_STATUS_OK;
    fixture.ui_destroy_status = LIB_STATUS_OK;
    fixture.machine_destroy_status = LIB_STATUS_OK;
    fixture.machine.live = LIB_TRUE;
}

static lib_i32 composition_fixture_clean(void)
{
    return !fixture.machine.live && !fixture.common_machine.live &&
        !fixture.session.live && !fixture.ui.live &&
        fixture.machine.bound == LIB_NULL && fixture.session.ui == LIB_NULL;
}

static void destroy(void *opaque)
{
    struct fixture_machine *machine = opaque;
    if (machine == LIB_NULL) return;
    ++fixture.machine_destroy_count;
    machine->bound = LIB_NULL;
    machine->live = LIB_FALSE;
}

static lib_status bind(void *opaque, common_machine *common)
{
    struct fixture_machine *machine = opaque;
    if (machine == LIB_NULL || !machine->live) return LIB_STATUS_INVALID_STATE;
    if (common != LIB_NULL && fixture.failure == COMPOSITION_FAILURE_MACHINE_BIND)
        return LIB_STATUS_INVALID_STATE;
    machine->bound = common;
    return LIB_STATUS_OK;
}

static const app_composed_machine composition_machine = {
    .machine = &fixture.machine, .bind = bind, .destroy = destroy
};

lib_status common_machine_create(common_machine **out_machine,
    const common_machine_driver *driver)
{
    if (out_machine == LIB_NULL || driver == LIB_NULL || fixture.common_machine.live ||
        fixture.failure == COMPOSITION_FAILURE_COMMON_MACHINE_CREATE)
        return LIB_STATUS_INVALID_ARGUMENT;
    fixture.common_machine.live = LIB_TRUE;
    *out_machine = &fixture.common_machine;
    return LIB_STATUS_OK;
}

lib_status common_machine_shutdown(common_machine *machine)
{
    (void)machine;
    return fixture.shutdown_status;
}

lib_status common_machine_destroy(common_machine *machine)
{
    if (machine == LIB_NULL) return LIB_STATUS_OK;
    if (fixture.ui.live || fixture.session.live) return LIB_STATUS_INTERNAL_ERROR;
    ++fixture.common_machine_destroy_count;
    if (fixture.machine_destroy_status != LIB_STATUS_OK)
        return fixture.machine_destroy_status;
    machine->live = LIB_FALSE;
    return LIB_STATUS_OK;
}

void common_machine_set_state_sink(common_machine *machine,
    common_machine_state_sink sink, void *context)
{ (void)machine; (void)sink; (void)context; }

void common_machine_set_frame_sink(common_machine *machine,
    common_machine_frame_sink sink, void *context)
{ (void)machine; (void)sink; (void)context; }

lib_status common_session_create(common_session **out_session,
    const common_session_options *options)
{
    if (out_session == LIB_NULL || options == LIB_NULL || fixture.session.live ||
        fixture.failure == COMPOSITION_FAILURE_SESSION_CREATE)
        return LIB_STATUS_INVALID_ARGUMENT;
    fixture.session.live = LIB_TRUE;
    *out_session = &fixture.session;
    return LIB_STATUS_OK;
}

lib_status common_session_bind_ui(common_session *session, common_ui *ui)
{
    if (session == LIB_NULL || ui == LIB_NULL || !session->live || !ui->live ||
        fixture.failure == COMPOSITION_FAILURE_UI_BIND)
        return LIB_STATUS_INVALID_ARGUMENT;
    session->ui = ui;
    return LIB_STATUS_OK;
}

lib_status common_session_destroy(common_session *session)
{
    if (session == LIB_NULL) return LIB_STATUS_OK;
    if (fixture.ui.live) return LIB_STATUS_INTERNAL_ERROR;
    ++fixture.session_destroy_count;
    session->ui = LIB_NULL;
    session->live = LIB_FALSE;
    return LIB_STATUS_OK;
}

lib_bool common_session_enqueue_runtime_completed(common_session *session,
    common_session_machine_state state, lib_u32 generation)
{ (void)session; (void)state; (void)generation; return LIB_TRUE; }

lib_bool common_session_enqueue_frame_completed(common_session *session,
    lib_u32 sequence, lib_bool graphics, lib_u32 generation)
{ (void)session; (void)sequence; (void)graphics; (void)generation; return LIB_TRUE; }

lib_status common_ui_create(common_ui **out_ui, const common_ui_options *options)
{
    if (out_ui == LIB_NULL || options == LIB_NULL || fixture.ui.live ||
        fixture.failure == COMPOSITION_FAILURE_UI_CREATE)
        return LIB_STATUS_INVALID_ARGUMENT;
    fixture.ui.live = LIB_TRUE;
    *out_ui = &fixture.ui;
    return LIB_STATUS_OK;
}

lib_status common_ui_destroy(common_ui *ui)
{
    if (ui == LIB_NULL) return LIB_STATUS_OK;
    ++fixture.ui_destroy_count;
    if (fixture.ui_destroy_status != LIB_STATUS_OK) return fixture.ui_destroy_status;
    ui->live = LIB_FALSE;
    return LIB_STATUS_OK;
}

static lib_i32 composition_machine_failure_recovers(composition_failure failure)
{
    product_surface *app = LIB_NULL;

    composition_fixture_reset(failure);
    if (product_surface_create(&composition_machine, &app) != LIB_STATUS_OK ||
        product_surface_compose_machine(app) == LIB_STATUS_OK ||
        product_surface_common_machine(app) != LIB_NULL || !fixture.machine.live) return 0;
    fixture.failure = COMPOSITION_FAILURE_NONE;
    if (product_surface_compose_machine(app) != LIB_STATUS_OK ||
        product_surface_common_machine(app) == LIB_NULL) return 0;
    return product_surface_destroy(app) == LIB_STATUS_OK && composition_fixture_clean();
}

static lib_i32 composition_control_failure_recovers(void)
{
    product_surface *app = LIB_NULL;
    common_session_options options = {0};

    composition_fixture_reset(COMPOSITION_FAILURE_SESSION_CREATE);
    if (product_surface_create(&composition_machine, &app) != LIB_STATUS_OK ||
        product_surface_compose_machine(app) != LIB_STATUS_OK ||
        product_surface_compose_control(app, &options) != LIB_STATUS_INVALID_ARGUMENT ||
        product_surface_session(app) != LIB_NULL || !fixture.machine.live ||
        !fixture.common_machine.live || fixture.session.live) return 0;
    fixture.failure = COMPOSITION_FAILURE_NONE;
    if (product_surface_compose_control(app, &options) != LIB_STATUS_OK ||
        product_surface_session(app) == LIB_NULL) return 0;
    return product_surface_destroy(app) == LIB_STATUS_OK && composition_fixture_clean();
}

static lib_i32 composition_ui_failure_recovers(composition_failure failure)
{
    product_surface *app = LIB_NULL;
    common_session_options session_options = {0};
    common_ui_options ui_options = {0};

    composition_fixture_reset(failure);
    if (product_surface_create(&composition_machine, &app) != LIB_STATUS_OK ||
        product_surface_compose_machine(app) != LIB_STATUS_OK ||
        product_surface_compose_control(app, &session_options) != LIB_STATUS_OK ||
        product_surface_compose_ui(app, &ui_options) != LIB_STATUS_INVALID_ARGUMENT ||
        product_surface_ui(app) != LIB_NULL) return 0;
    fixture.failure = COMPOSITION_FAILURE_NONE;
    if (product_surface_compose_ui(app, &ui_options) != LIB_STATUS_OK ||
        product_surface_ui(app) == LIB_NULL) return 0;
    return product_surface_destroy(app) == LIB_STATUS_OK && composition_fixture_clean();
}

static lib_i32 composition_destroy_failure_recovers(void)
{
    product_surface *app = LIB_NULL;
    common_session_options session_options = {0};
    common_ui_options ui_options = {0};

    composition_fixture_reset(COMPOSITION_FAILURE_NONE);
    if (product_surface_create(&composition_machine, &app) != LIB_STATUS_OK ||
        product_surface_compose_machine(app) != LIB_STATUS_OK ||
        product_surface_compose_control(app, &session_options) != LIB_STATUS_OK ||
        product_surface_compose_ui(app, &ui_options) != LIB_STATUS_OK) return 0;
    fixture.shutdown_status = LIB_STATUS_IO_ERROR;
    if (product_surface_destroy(app) != LIB_STATUS_INTERNAL_ERROR || !fixture.machine.live ||
        !fixture.common_machine.live || !fixture.session.live || !fixture.ui.live ||
        fixture.machine_destroy_count != 0u || fixture.common_machine_destroy_count != 0u ||
        fixture.session_destroy_count != 0u || fixture.ui_destroy_count != 0u) return 0;
    fixture.shutdown_status = LIB_STATUS_OK;
    return product_surface_destroy(app) == LIB_STATUS_OK && composition_fixture_clean();
}

static lib_i32 composition_machine_cleanup_failure_recovers(void)
{
    product_surface *app = LIB_NULL;

    composition_fixture_reset(COMPOSITION_FAILURE_MACHINE_BIND);
    fixture.machine_destroy_status = LIB_STATUS_IO_ERROR;
    if (product_surface_create(&composition_machine, &app) != LIB_STATUS_OK ||
        product_surface_compose_machine(app) != LIB_STATUS_INTERNAL_ERROR ||
        product_surface_common_machine(app) != &fixture.common_machine ||
        !fixture.common_machine.live || !fixture.machine.live ||
        fixture.machine_destroy_count != 0u ||
        product_surface_compose_machine(app) != LIB_STATUS_INVALID_STATE) return 0;
    fixture.machine_destroy_status = LIB_STATUS_OK;
    return product_surface_destroy(app) == LIB_STATUS_OK && composition_fixture_clean() &&
        fixture.common_machine_destroy_count == 2u && fixture.machine_destroy_count == 1u;
}

static lib_i32 composition_ui_destroy_failure_recovers(lib_bool binding_failed)
{
    product_surface *app = LIB_NULL;
    common_session_options session_options = {0};
    common_ui_options ui_options = {0};

    composition_fixture_reset(binding_failed ? COMPOSITION_FAILURE_UI_BIND :
        COMPOSITION_FAILURE_NONE);
    if (product_surface_create(&composition_machine, &app) != LIB_STATUS_OK ||
        product_surface_compose_machine(app) != LIB_STATUS_OK ||
        product_surface_compose_control(app, &session_options) != LIB_STATUS_OK) return 0;
    fixture.ui_destroy_status = binding_failed ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
    if (product_surface_compose_ui(app, &ui_options) !=
            (binding_failed ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK) ||
        product_surface_ui(app) != &fixture.ui || !fixture.ui.live) return 0;
    fixture.ui_destroy_status = LIB_STATUS_IO_ERROR;
    if (product_surface_destroy(app) != LIB_STATUS_INTERNAL_ERROR ||
        product_surface_ui(app) != &fixture.ui || !fixture.ui.live ||
        !fixture.session.live || !fixture.common_machine.live ||
        !fixture.machine.live || fixture.session_destroy_count != 0u ||
        fixture.common_machine_destroy_count != 0u ||
        fixture.machine_destroy_count != 0u) return 0;
    fixture.ui_destroy_status = LIB_STATUS_OK;
    return product_surface_destroy(app) == LIB_STATUS_OK && composition_fixture_clean() &&
        fixture.ui_destroy_count == (binding_failed ? 3u : 2u) &&
        fixture.session_destroy_count == 1u &&
        fixture.common_machine_destroy_count == 1u && fixture.machine_destroy_count == 1u;
}

lib_i32 main(void)
{
    if (!composition_machine_failure_recovers(
            COMPOSITION_FAILURE_COMMON_MACHINE_CREATE) ||
        !composition_machine_failure_recovers(COMPOSITION_FAILURE_MACHINE_BIND) ||
        !composition_control_failure_recovers() ||
        !composition_ui_failure_recovers(COMPOSITION_FAILURE_UI_CREATE) ||
        !composition_ui_failure_recovers(COMPOSITION_FAILURE_UI_BIND) ||
        !composition_destroy_failure_recovers() ||
        !composition_machine_cleanup_failure_recovers() ||
        !composition_ui_destroy_failure_recovers(LIB_FALSE) ||
        !composition_ui_destroy_failure_recovers(LIB_TRUE)) return 1;
    lib_c_printf("APP-COMPOSITION-ATOMICITY:OK\n");
    return 0;
}
