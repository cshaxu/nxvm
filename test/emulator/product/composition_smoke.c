#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "emulator/product/composition_interface.h"

struct fixture_machine { emulator_machine *bound; lib_bool live; };
struct emulator_machine { lib_bool live; };
struct emulator_session { emulator_ui *ui; lib_bool live; };
struct emulator_ui { lib_bool live; };

typedef enum composition_failure {
    COMPOSITION_FAILURE_NONE,
    COMPOSITION_FAILURE_EMULATOR_MACHINE_CREATE,
    COMPOSITION_FAILURE_MACHINE_BIND,
    COMPOSITION_FAILURE_MACHINE_UNBIND,
    COMPOSITION_FAILURE_SESSION_CREATE,
    COMPOSITION_FAILURE_UI_CREATE,
    COMPOSITION_FAILURE_UI_BIND
} composition_failure;

typedef struct composition_fixture {
    composition_failure failure;
    struct fixture_machine machine;
    struct emulator_machine emulator_machine;
    struct emulator_session session;
    struct emulator_ui ui;
    lib_u32 machine_destroy_count;
    lib_u32 emulator_machine_destroy_count;
    lib_u32 session_destroy_count;
    lib_u32 ui_destroy_count;
    lib_u32 teardown_sequence;
    lib_u32 unbind_attempt_count;
    lib_u32 unbind_sequence;
    lib_u32 emulator_machine_destroy_sequence;
    lib_status shutdown_status;
    lib_status ui_destroy_status;
    lib_status machine_destroy_status;
    lib_bool initial_state_enqueued;
    lib_bool initial_state_enqueue_succeeds;
} composition_fixture;

static composition_fixture fixture;

static void composition_fixture_reset(composition_failure failure)
{
    lib_memory_set(&fixture, 0, sizeof(fixture));
    fixture.failure = failure;
    fixture.shutdown_status = LIB_STATUS_OK;
    fixture.ui_destroy_status = LIB_STATUS_OK;
    fixture.machine_destroy_status = LIB_STATUS_OK;
    fixture.initial_state_enqueue_succeeds = LIB_TRUE;
    fixture.machine.live = LIB_TRUE;
}

static lib_i32 composition_fixture_clean(void)
{
    return !fixture.machine.live && !fixture.emulator_machine.live &&
        !fixture.session.live && !fixture.ui.live &&
        fixture.machine.bound == LIB_NULL && fixture.session.ui == LIB_NULL;
}

static lib_status destroy(void *opaque)
{
    struct fixture_machine *machine = opaque;
    if (machine == LIB_NULL) return LIB_STATUS_OK;
    ++fixture.machine_destroy_count;
    machine->bound = LIB_NULL;
    machine->live = LIB_FALSE;
    return LIB_STATUS_OK;
}

static lib_status bind(void *opaque, emulator_machine *emulator)
{
    struct fixture_machine *machine = opaque;
    if (machine == LIB_NULL || !machine->live) return LIB_STATUS_INVALID_STATE;
    if (emulator != LIB_NULL && fixture.failure == COMPOSITION_FAILURE_MACHINE_BIND)
        return LIB_STATUS_INVALID_STATE;
    if (emulator == LIB_NULL) {
        ++fixture.unbind_attempt_count;
        if (fixture.failure == COMPOSITION_FAILURE_MACHINE_UNBIND)
            return LIB_STATUS_IO_ERROR;
        fixture.unbind_sequence = ++fixture.teardown_sequence;
    }
    machine->bound = emulator;
    return LIB_STATUS_OK;
}

static const emulator_product_machine composition_machine = {
    .machine = &fixture.machine, .bind = bind, .destroy = destroy
};

lib_status emulator_machine_create(emulator_machine **out_machine,
    const emulator_machine_driver *driver)
{
    if (out_machine == LIB_NULL || driver == LIB_NULL || fixture.emulator_machine.live ||
        fixture.failure == COMPOSITION_FAILURE_EMULATOR_MACHINE_CREATE)
        return LIB_STATUS_INVALID_ARGUMENT;
    fixture.emulator_machine.live = LIB_TRUE;
    *out_machine = &fixture.emulator_machine;
    return LIB_STATUS_OK;
}

lib_status emulator_machine_shutdown(emulator_machine *machine)
{
    (void)machine;
    return fixture.shutdown_status;
}

lib_status emulator_machine_destroy(emulator_machine *machine)
{
    if (machine == LIB_NULL) return LIB_STATUS_OK;
    if (fixture.ui.live || fixture.session.live) return LIB_STATUS_INTERNAL_ERROR;
    fixture.emulator_machine_destroy_sequence = ++fixture.teardown_sequence;
    ++fixture.emulator_machine_destroy_count;
    if (fixture.machine_destroy_status != LIB_STATUS_OK)
        return fixture.machine_destroy_status;
    machine->live = LIB_FALSE;
    return LIB_STATUS_OK;
}

void emulator_machine_set_state_sink(emulator_machine *machine,
    emulator_machine_state_sink sink, void *context)
{ (void)machine; (void)sink; (void)context; }

void emulator_machine_set_frame_sink(emulator_machine *machine,
    emulator_machine_frame_sink sink, void *context)
{ (void)machine; (void)sink; (void)context; }

emulator_machine_state emulator_machine_state_get(const emulator_machine *machine)
{ (void)machine; return EMULATOR_MACHINE_STOPPED; }

lib_u32 emulator_machine_run_generation(const emulator_machine *machine)
{ (void)machine; return 0u; }

lib_status emulator_session_create(emulator_session **out_session,
    const emulator_session_options *options)
{
    if (out_session == LIB_NULL || options == LIB_NULL || fixture.session.live ||
        fixture.failure == COMPOSITION_FAILURE_SESSION_CREATE)
        return LIB_STATUS_INVALID_ARGUMENT;
    fixture.session.live = LIB_TRUE;
    *out_session = &fixture.session;
    return LIB_STATUS_OK;
}

lib_status emulator_session_bind_ui(emulator_session *session, emulator_ui *ui)
{
    if (session == LIB_NULL || ui == LIB_NULL || !session->live || !ui->live ||
        fixture.failure == COMPOSITION_FAILURE_UI_BIND)
        return LIB_STATUS_INVALID_ARGUMENT;
    session->ui = ui;
    return LIB_STATUS_OK;
}

lib_status emulator_session_destroy(emulator_session *session)
{
    if (session == LIB_NULL) return LIB_STATUS_OK;
    if (fixture.ui.live) return LIB_STATUS_INTERNAL_ERROR;
    ++fixture.session_destroy_count;
    session->ui = LIB_NULL;
    session->live = LIB_FALSE;
    return LIB_STATUS_OK;
}

lib_bool emulator_session_enqueue_runtime_completed(emulator_session *session,
    emulator_session_machine_state state, lib_u32 generation)
{
    (void)session; (void)state; (void)generation;
    fixture.initial_state_enqueued = LIB_TRUE;
    return fixture.initial_state_enqueue_succeeds;
}

lib_bool emulator_session_enqueue_frame_completed(emulator_session *session,
    lib_u32 sequence, lib_bool graphics, lib_u32 generation)
{ (void)session; (void)sequence; (void)graphics; (void)generation; return LIB_TRUE; }

lib_bool emulator_session_run(emulator_session *session)
{ (void)session; return LIB_TRUE; }

lib_bool emulator_session_enqueue_ui_event(void *context, const emulator_ui_event *event)
{ (void)context; (void)event; return LIB_TRUE; }

lib_status emulator_ui_create(emulator_ui **out_ui, const emulator_ui_options *options)
{
    if (out_ui == LIB_NULL || options == LIB_NULL || fixture.ui.live ||
        fixture.failure == COMPOSITION_FAILURE_UI_CREATE)
        return LIB_STATUS_INVALID_ARGUMENT;
    fixture.ui.live = LIB_TRUE;
    *out_ui = &fixture.ui;
    return LIB_STATUS_OK;
}

lib_status emulator_ui_destroy(emulator_ui *ui)
{
    if (ui == LIB_NULL) return LIB_STATUS_OK;
    ++fixture.ui_destroy_count;
    if (fixture.ui_destroy_status != LIB_STATUS_OK) return fixture.ui_destroy_status;
    ui->live = LIB_FALSE;
    return LIB_STATUS_OK;
}

static lib_i32 composition_machine_failure_recovers(composition_failure failure)
{
    emulator_product *app = LIB_NULL;

    composition_fixture_reset(failure);
    if (emulator_product_create(&composition_machine, &app) != LIB_STATUS_OK ||
        emulator_product_compose_machine(app) == LIB_STATUS_OK ||
        fixture.emulator_machine.live || !fixture.machine.live ||
        (failure == COMPOSITION_FAILURE_MACHINE_BIND && fixture.unbind_attempt_count != 0u))
        return 0;
    fixture.failure = COMPOSITION_FAILURE_NONE;
    if (emulator_product_compose_machine(app) != LIB_STATUS_OK ||
        !fixture.emulator_machine.live) return 0;
    return emulator_product_destroy(app) == LIB_STATUS_OK && composition_fixture_clean();
}

static lib_i32 composition_destroy_before_compose(void)
{
    emulator_product *app = LIB_NULL;

    composition_fixture_reset(COMPOSITION_FAILURE_NONE);
    if (emulator_product_create(&composition_machine, &app) != LIB_STATUS_OK ||
        emulator_product_destroy(app) != LIB_STATUS_OK || !composition_fixture_clean())
        return 0;
    return fixture.machine_destroy_count == 1u &&
        fixture.emulator_machine_destroy_count == 0u &&
        fixture.session_destroy_count == 0u && fixture.ui_destroy_count == 0u;
}

static lib_i32 composition_destroy_unbinds_before_wrapper(void)
{
    emulator_product *app = LIB_NULL;

    composition_fixture_reset(COMPOSITION_FAILURE_NONE);
    if (emulator_product_create(&composition_machine, &app) != LIB_STATUS_OK ||
        emulator_product_compose_machine(app) != LIB_STATUS_OK ||
        emulator_product_destroy(app) != LIB_STATUS_OK || !composition_fixture_clean())
        return 0;
    return fixture.unbind_sequence != 0u &&
        fixture.emulator_machine_destroy_sequence != 0u &&
        fixture.unbind_sequence < fixture.emulator_machine_destroy_sequence;
}

static lib_i32 composition_destroy_unbind_failure_recovers(void)
{
    emulator_product *app = LIB_NULL;

    composition_fixture_reset(COMPOSITION_FAILURE_NONE);
    if (emulator_product_create(&composition_machine, &app) != LIB_STATUS_OK ||
        emulator_product_compose_machine(app) != LIB_STATUS_OK) return 0;
    fixture.failure = COMPOSITION_FAILURE_MACHINE_UNBIND;
    if (emulator_product_destroy(app) != LIB_STATUS_IO_ERROR || !fixture.machine.live ||
        !fixture.emulator_machine.live ||
        fixture.machine.bound != &fixture.emulator_machine ||
        fixture.machine_destroy_count != 0u ||
        fixture.emulator_machine_destroy_count != 0u) return 0;
    fixture.failure = COMPOSITION_FAILURE_NONE;
    return emulator_product_destroy(app) == LIB_STATUS_OK && composition_fixture_clean() &&
        fixture.machine_destroy_count == 1u &&
        fixture.emulator_machine_destroy_count == 1u;
}

static lib_i32 composition_control_failure_recovers(void)
{
    emulator_product *app = LIB_NULL;
    emulator_session_options options = {0};

    composition_fixture_reset(COMPOSITION_FAILURE_SESSION_CREATE);
    if (emulator_product_create(&composition_machine, &app) != LIB_STATUS_OK ||
        emulator_product_compose_machine(app) != LIB_STATUS_OK ||
        emulator_product_compose_control(app, &options) != LIB_STATUS_INVALID_ARGUMENT ||
        !fixture.machine.live ||
        !fixture.emulator_machine.live || fixture.session.live) return 0;
    fixture.failure = COMPOSITION_FAILURE_NONE;
    if (emulator_product_compose_control(app, &options) != LIB_STATUS_OK ||
        !fixture.session.live) return 0;
    return emulator_product_destroy(app) == LIB_STATUS_OK && composition_fixture_clean();
}

static lib_i32 composition_ui_failure_recovers(composition_failure failure)
{
    emulator_product *app = LIB_NULL;
    emulator_session_options session_options = {0};
    emulator_ui_options ui_options = {0};

    composition_fixture_reset(failure);
    if (emulator_product_create(&composition_machine, &app) != LIB_STATUS_OK ||
        emulator_product_compose_machine(app) != LIB_STATUS_OK ||
        emulator_product_compose_control(app, &session_options) != LIB_STATUS_OK ||
        emulator_product_compose_ui(app, &ui_options) != LIB_STATUS_INVALID_ARGUMENT ||
        fixture.ui.live) return 0;
    fixture.failure = COMPOSITION_FAILURE_NONE;
    if (emulator_product_compose_ui(app, &ui_options) != LIB_STATUS_OK ||
        !fixture.ui.live) return 0;
    return emulator_product_destroy(app) == LIB_STATUS_OK && composition_fixture_clean();
}

static lib_i32 composition_ui_requires_control(void)
{
    emulator_product *app = LIB_NULL;
    emulator_ui_options ui_options = {0};

    composition_fixture_reset(COMPOSITION_FAILURE_NONE);
    if (emulator_product_create(&composition_machine, &app) != LIB_STATUS_OK ||
        emulator_product_compose_machine(app) != LIB_STATUS_OK ||
        emulator_product_compose_ui(app, &ui_options) != LIB_STATUS_INVALID_STATE ||
        fixture.ui.live || fixture.ui_destroy_count != 0u) return 0;
    return emulator_product_destroy(app) == LIB_STATUS_OK && composition_fixture_clean();
}

static lib_i32 composition_destroy_failure_recovers(void)
{
    emulator_product *app = LIB_NULL;
    emulator_session_options session_options = {0};
    emulator_ui_options ui_options = {0};

    composition_fixture_reset(COMPOSITION_FAILURE_NONE);
    if (emulator_product_create(&composition_machine, &app) != LIB_STATUS_OK ||
        emulator_product_compose_machine(app) != LIB_STATUS_OK ||
        emulator_product_compose_control(app, &session_options) != LIB_STATUS_OK ||
        emulator_product_compose_ui(app, &ui_options) != LIB_STATUS_OK) return 0;
    fixture.shutdown_status = LIB_STATUS_IO_ERROR;
    if (emulator_product_destroy(app) != LIB_STATUS_IO_ERROR || !fixture.machine.live ||
        !fixture.emulator_machine.live || !fixture.session.live || !fixture.ui.live ||
        fixture.machine_destroy_count != 0u || fixture.emulator_machine_destroy_count != 0u ||
        fixture.session_destroy_count != 0u || fixture.ui_destroy_count != 0u) return 0;
    fixture.shutdown_status = LIB_STATUS_OK;
    return emulator_product_destroy(app) == LIB_STATUS_OK && composition_fixture_clean();
}

static lib_i32 composition_initial_state_reports_enqueue_result(void)
{
    emulator_product *app = LIB_NULL;
    emulator_session_options options = {0};

    composition_fixture_reset(COMPOSITION_FAILURE_NONE);
    if (emulator_product_create(&composition_machine, &app) != LIB_STATUS_OK ||
        emulator_product_compose_machine(app) != LIB_STATUS_OK ||
        emulator_product_compose_control(app, &options) != LIB_STATUS_OK ||
        emulator_product_publish_initial_state(app) != LIB_STATUS_OK ||
        !fixture.initial_state_enqueued) return 0;
    fixture.initial_state_enqueued = LIB_FALSE;
    fixture.initial_state_enqueue_succeeds = LIB_FALSE;
    if (emulator_product_publish_initial_state(app) != LIB_STATUS_INTERNAL_ERROR ||
        !fixture.initial_state_enqueued) return 0;
    return emulator_product_destroy(app) == LIB_STATUS_OK && composition_fixture_clean();
}

static lib_i32 composition_machine_cleanup_failure_recovers(void)
{
    emulator_product *app = LIB_NULL;

    composition_fixture_reset(COMPOSITION_FAILURE_MACHINE_BIND);
    fixture.machine_destroy_status = LIB_STATUS_IO_ERROR;
    if (emulator_product_create(&composition_machine, &app) != LIB_STATUS_OK ||
        emulator_product_compose_machine(app) != LIB_STATUS_IO_ERROR ||
        !fixture.emulator_machine.live || !fixture.machine.live ||
        fixture.machine_destroy_count != 0u ||
        emulator_product_compose_machine(app) != LIB_STATUS_INVALID_STATE) return 0;
    fixture.machine_destroy_status = LIB_STATUS_OK;
    return emulator_product_destroy(app) == LIB_STATUS_OK && composition_fixture_clean() &&
        fixture.emulator_machine_destroy_count == 2u && fixture.machine_destroy_count == 1u;
}

static lib_i32 composition_ui_destroy_failure_recovers(lib_bool binding_failed)
{
    emulator_product *app = LIB_NULL;
    emulator_session_options session_options = {0};
    emulator_ui_options ui_options = {0};

    composition_fixture_reset(binding_failed ? COMPOSITION_FAILURE_UI_BIND :
        COMPOSITION_FAILURE_NONE);
    if (emulator_product_create(&composition_machine, &app) != LIB_STATUS_OK ||
        emulator_product_compose_machine(app) != LIB_STATUS_OK ||
        emulator_product_compose_control(app, &session_options) != LIB_STATUS_OK) return 0;
    fixture.ui_destroy_status = binding_failed ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
    if (emulator_product_compose_ui(app, &ui_options) !=
            (binding_failed ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK) ||
        !fixture.ui.live) return 0;
    fixture.ui_destroy_status = LIB_STATUS_IO_ERROR;
    if (emulator_product_destroy(app) != LIB_STATUS_IO_ERROR ||
        !fixture.ui.live ||
        !fixture.session.live || !fixture.emulator_machine.live ||
        !fixture.machine.live || fixture.session_destroy_count != 0u ||
        fixture.emulator_machine_destroy_count != 0u ||
        fixture.machine_destroy_count != 0u) return 0;
    fixture.ui_destroy_status = LIB_STATUS_OK;
    return emulator_product_destroy(app) == LIB_STATUS_OK && composition_fixture_clean() &&
        fixture.ui_destroy_count == (binding_failed ? 3u : 2u) &&
        fixture.session_destroy_count == 1u &&
        fixture.emulator_machine_destroy_count == 1u && fixture.machine_destroy_count == 1u;
}

lib_i32 main(void)
{
    if (!composition_machine_failure_recovers(
            COMPOSITION_FAILURE_EMULATOR_MACHINE_CREATE) ||
        !composition_machine_failure_recovers(COMPOSITION_FAILURE_MACHINE_BIND) ||
        !composition_destroy_before_compose() ||
        !composition_destroy_unbinds_before_wrapper() ||
        !composition_destroy_unbind_failure_recovers() ||
        !composition_control_failure_recovers() ||
        !composition_ui_requires_control() ||
        !composition_ui_failure_recovers(COMPOSITION_FAILURE_UI_CREATE) ||
        !composition_ui_failure_recovers(COMPOSITION_FAILURE_UI_BIND) ||
        !composition_destroy_failure_recovers() ||
        !composition_initial_state_reports_enqueue_result() ||
        !composition_machine_cleanup_failure_recovers() ||
        !composition_ui_destroy_failure_recovers(LIB_FALSE) ||
        !composition_ui_destroy_failure_recovers(LIB_TRUE)) return 1;
    lib_c_printf("APP-COMPOSITION-ATOMICITY:OK\n");
    return 0;
}
