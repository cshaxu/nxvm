#include "type.h"

#include <assert.h>

#include "app/composition.h"
#include "app/config.h"

struct vm_machine {
    common_machine *bound;
    lib_bool live;
};

struct common_machine {
    lib_bool live;
};

struct common_session {
    common_ui *ui;
    lib_bool live;
};

struct common_ui {
    lib_bool live;
};

typedef enum composition_failure {
    COMPOSITION_FAILURE_NONE,
    COMPOSITION_FAILURE_CONFIGURE,
    COMPOSITION_FAILURE_MACHINE_CREATE,
    COMPOSITION_FAILURE_DRIVER_DESCRIBE,
    COMPOSITION_FAILURE_COMMON_MACHINE_CREATE,
    COMPOSITION_FAILURE_MACHINE_BIND,
    COMPOSITION_FAILURE_SESSION_CREATE,
    COMPOSITION_FAILURE_UI_CREATE,
    COMPOSITION_FAILURE_UI_BIND
} composition_failure;

typedef struct composition_fixture {
    composition_failure failure;
    struct vm_machine machine;
    struct common_machine common_machine;
    struct common_session session;
    struct common_ui ui;
    C_UINT machine_destroy_count;
    C_UINT common_machine_destroy_count;
    C_UINT session_destroy_count;
    C_UINT ui_destroy_count;
    lib_status shutdown_status;
} composition_fixture;

static composition_fixture fixture;

static C_VOID composition_fixture_reset(composition_failure failure)
{
    STD_MEMSET(&fixture, 0, sizeof(fixture));
    fixture.failure = failure;
    fixture.shutdown_status = LIB_STATUS_OK;
}

static C_INT composition_fixture_clean(C_VOID)
{
    return !fixture.machine.live && !fixture.common_machine.live &&
        !fixture.session.live && !fixture.ui.live &&
        fixture.machine.bound == LIB_NULL && fixture.session.ui == LIB_NULL;
}

type_status vm_app_configure_machine(const vm_session_request *request,
    vm_machine_config *out_config)
{
    if (request == STD_NULL || out_config == STD_NULL ||
        fixture.failure == COMPOSITION_FAILURE_CONFIGURE) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_config = (vm_machine_config){0};
    return TYPE_STATUS_OK;
}

type_status vm_machine_create(const vm_machine_config *config,
    vm_machine **out_machine)
{
    if (config == STD_NULL || out_machine == STD_NULL || fixture.machine.live)
        return TYPE_STATUS_INVALID_STATE;
    if (fixture.failure == COMPOSITION_FAILURE_MACHINE_CREATE)
        return TYPE_STATUS_NO_MEMORY;
    fixture.machine.live = LIB_TRUE;
    *out_machine = &fixture.machine;
    return TYPE_STATUS_OK;
}

C_VOID vm_machine_destroy(vm_machine *machine)
{
    if (machine == STD_NULL) return;
    ++fixture.machine_destroy_count;
    machine->bound = LIB_NULL;
    machine->live = LIB_FALSE;
}

type_status vm_machine_describe_common_driver(vm_machine *machine,
    common_machine_driver *out_driver)
{
    if (machine == STD_NULL || out_driver == LIB_NULL || !machine->live ||
        fixture.failure == COMPOSITION_FAILURE_DRIVER_DESCRIBE) return TYPE_STATUS_INVALID_STATE;
    *out_driver = (common_machine_driver){0};
    return TYPE_STATUS_OK;
}

type_status vm_machine_bind_common_machine(vm_machine *machine,
    common_machine *common_machine)
{
    if (machine == STD_NULL || !machine->live) return TYPE_STATUS_INVALID_STATE;
    if (common_machine != LIB_NULL && fixture.failure == COMPOSITION_FAILURE_MACHINE_BIND)
        return TYPE_STATUS_INVALID_STATE;
    machine->bound = common_machine;
    return TYPE_STATUS_OK;
}

lib_status common_machine_create(common_machine **out_machine,
    const common_machine_driver *driver)
{
    if (out_machine == LIB_NULL || driver == LIB_NULL || fixture.common_machine.live ||
        fixture.failure == COMPOSITION_FAILURE_COMMON_MACHINE_CREATE) return LIB_STATUS_INVALID_ARGUMENT;
    fixture.common_machine.live = LIB_TRUE;
    *out_machine = &fixture.common_machine;
    return LIB_STATUS_OK;
}

lib_status common_machine_shutdown(common_machine *machine)
{
    (C_VOID)machine;
    return fixture.shutdown_status;
}

lib_status common_machine_destroy(common_machine *machine)
{
    if (machine == LIB_NULL) return LIB_STATUS_OK;
    ++fixture.common_machine_destroy_count;
    machine->live = LIB_FALSE;
    return LIB_STATUS_OK;
}

C_VOID common_machine_set_state_sink(common_machine *machine,
    common_machine_state_sink sink, C_VOID *context)
{
    (C_VOID)machine;
    (C_VOID)sink;
    (C_VOID)context;
}

C_VOID common_machine_set_frame_sink(common_machine *machine,
    common_machine_frame_sink sink, C_VOID *context)
{
    (C_VOID)machine;
    (C_VOID)sink;
    (C_VOID)context;
}

lib_status common_session_create(common_session **out_session,
    const common_session_options *options)
{
    if (out_session == LIB_NULL || options == LIB_NULL || fixture.session.live ||
        fixture.failure == COMPOSITION_FAILURE_SESSION_CREATE) return LIB_STATUS_INVALID_ARGUMENT;
    fixture.session.live = LIB_TRUE;
    *out_session = &fixture.session;
    return LIB_STATUS_OK;
}

lib_status common_session_bind_ui(common_session *session, common_ui *ui)
{
    if (session == LIB_NULL || ui == LIB_NULL || !session->live || !ui->live ||
        fixture.failure == COMPOSITION_FAILURE_UI_BIND) return LIB_STATUS_INVALID_ARGUMENT;
    session->ui = ui;
    return LIB_STATUS_OK;
}

lib_status common_session_destroy(common_session *session)
{
    if (session == LIB_NULL) return LIB_STATUS_OK;
    ++fixture.session_destroy_count;
    session->ui = LIB_NULL;
    session->live = LIB_FALSE;
    return LIB_STATUS_OK;
}

C_INT common_session_enqueue_runtime_completed(common_session *session,
    common_session_machine_state state, lib_u32 generation)
{
    (C_VOID)session;
    (C_VOID)state;
    (C_VOID)generation;
    return TYPE_TRUE;
}

C_INT common_session_enqueue_frame_completed(common_session *session,
    lib_u32 sequence, lib_bool graphics, lib_u32 generation)
{
    (C_VOID)session;
    (C_VOID)sequence;
    (C_VOID)graphics;
    (C_VOID)generation;
    return TYPE_TRUE;
}

lib_status common_ui_create(common_ui **out_ui, const common_ui_options *options)
{
    if (out_ui == LIB_NULL || options == LIB_NULL || fixture.ui.live ||
        fixture.failure == COMPOSITION_FAILURE_UI_CREATE) return LIB_STATUS_INVALID_ARGUMENT;
    fixture.ui.live = LIB_TRUE;
    *out_ui = &fixture.ui;
    return LIB_STATUS_OK;
}

lib_status common_ui_destroy(common_ui *ui)
{
    if (ui == LIB_NULL) return LIB_STATUS_OK;
    ++fixture.ui_destroy_count;
    ui->live = LIB_FALSE;
    return LIB_STATUS_OK;
}

static C_INT composition_machine_failure_recovers(composition_failure failure,
    type_status expected)
{
    vm_app *app = STD_NULL;
    vm_session_request request = {0};

    composition_fixture_reset(failure);
    if (vm_app_create(&app) != TYPE_STATUS_OK ||
        vm_app_compose_machine(app, &request) != expected ||
        vm_app_machine(app) != STD_NULL || vm_app_common_machine(app) != LIB_NULL ||
        !composition_fixture_clean()) return 0;
    fixture.failure = COMPOSITION_FAILURE_NONE;
    if (vm_app_compose_machine(app, &request) != TYPE_STATUS_OK ||
        vm_app_machine(app) == STD_NULL || vm_app_common_machine(app) == LIB_NULL) return 0;
    vm_app_destroy(app);
    return composition_fixture_clean();
}

static C_INT composition_control_failure_recovers(C_VOID)
{
    vm_app *app = STD_NULL;
    vm_session_request request = {0};
    common_session_options options = {0};

    composition_fixture_reset(COMPOSITION_FAILURE_SESSION_CREATE);
    if (vm_app_create(&app) != TYPE_STATUS_OK ||
        vm_app_compose_machine(app, &request) != TYPE_STATUS_OK ||
        vm_app_compose_control(app, &options) != TYPE_STATUS_INVALID_ARGUMENT ||
        vm_app_session(app) != LIB_NULL || !fixture.machine.live ||
        !fixture.common_machine.live || fixture.session.live) return 0;
    fixture.failure = COMPOSITION_FAILURE_NONE;
    if (vm_app_compose_control(app, &options) != TYPE_STATUS_OK ||
        vm_app_session(app) == LIB_NULL) return 0;
    vm_app_destroy(app);
    return composition_fixture_clean();
}

static C_INT composition_ui_failure_recovers(composition_failure failure)
{
    vm_app *app = STD_NULL;
    vm_session_request request = {0};
    common_session_options session_options = {0};
    common_ui_options ui_options = {0};

    composition_fixture_reset(failure);
    if (vm_app_create(&app) != TYPE_STATUS_OK ||
        vm_app_compose_machine(app, &request) != TYPE_STATUS_OK ||
        vm_app_compose_control(app, &session_options) != TYPE_STATUS_OK ||
        vm_app_compose_ui(app, &ui_options) != TYPE_STATUS_INVALID_ARGUMENT ||
        vm_app_ui(app) != LIB_NULL || !fixture.machine.live ||
        !fixture.common_machine.live || !fixture.session.live || fixture.ui.live ||
        fixture.session.ui != LIB_NULL) return 0;
    fixture.failure = COMPOSITION_FAILURE_NONE;
    if (vm_app_compose_ui(app, &ui_options) != TYPE_STATUS_OK ||
        vm_app_ui(app) == LIB_NULL || fixture.session.ui != vm_app_ui(app)) return 0;
    vm_app_destroy(app);
    return composition_fixture_clean();
}

static C_INT composition_destroy_failure_recovers(C_VOID)
{
    vm_app *app = STD_NULL;
    vm_session_request request = {0};
    common_session_options session_options = {0};
    common_ui_options ui_options = {0};

    composition_fixture_reset(COMPOSITION_FAILURE_NONE);
    if (vm_app_create(&app) != TYPE_STATUS_OK ||
        vm_app_compose_machine(app, &request) != TYPE_STATUS_OK ||
        vm_app_compose_control(app, &session_options) != TYPE_STATUS_OK ||
        vm_app_compose_ui(app, &ui_options) != TYPE_STATUS_OK) return 0;
    fixture.shutdown_status = LIB_STATUS_IO_ERROR;
    if (vm_app_destroy(app) != TYPE_STATUS_FAULT || !fixture.machine.live ||
        !fixture.common_machine.live || !fixture.session.live || !fixture.ui.live ||
        fixture.machine_destroy_count != 0u || fixture.common_machine_destroy_count != 0u ||
        fixture.session_destroy_count != 0u || fixture.ui_destroy_count != 0u) return 0;
    fixture.shutdown_status = LIB_STATUS_OK;
    return vm_app_destroy(app) == TYPE_STATUS_OK && composition_fixture_clean();
}

C_INT main(C_VOID)
{
    static const struct {
        composition_failure failure;
        type_status expected;
    } machine_failures[] = {
        { COMPOSITION_FAILURE_CONFIGURE, TYPE_STATUS_INVALID_ARGUMENT },
        { COMPOSITION_FAILURE_MACHINE_CREATE, TYPE_STATUS_NO_MEMORY },
        { COMPOSITION_FAILURE_DRIVER_DESCRIBE, TYPE_STATUS_INVALID_STATE },
        { COMPOSITION_FAILURE_COMMON_MACHINE_CREATE, TYPE_STATUS_INVALID_ARGUMENT },
        { COMPOSITION_FAILURE_MACHINE_BIND, TYPE_STATUS_INVALID_STATE }
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(machine_failures) / sizeof(machine_failures[0u]);
        ++index) {
        if (!composition_machine_failure_recovers(machine_failures[index].failure,
                machine_failures[index].expected)) return 1;
    }
    if (!composition_control_failure_recovers() ||
        !composition_ui_failure_recovers(COMPOSITION_FAILURE_UI_CREATE) ||
        !composition_ui_failure_recovers(COMPOSITION_FAILURE_UI_BIND) ||
        !composition_destroy_failure_recovers()) return 1;
    STD_PRINTF("M5:T534:S8:APP-COMPOSITION-ATOMICITY:OK\n");
    return 0;
}
