#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "ibmpc/product/entry_interface.h"

struct vm_app { lib_bool live; };

static struct vm_app fixture;
static lib_u32 failure;
static lib_u32 created;
static lib_u32 destroyed;

static lib_status fixture_load_request(const char *name,
    vm_session_request *request)
{
    if (failure == 1u || failure == 2u ||
        lib_text_compare(name, "NXVM.ini") != 0)
        return LIB_STATUS_IO_ERROR;
    lib_memory_set(request, 0, sizeof(*request));
    lib_memory_copy(request->display, "console", 8u);
    return LIB_STATUS_OK;
}

lib_status vm_app_create(const vm_app_factory *factory, vm_app **out_app)
{
    (void)factory;
    ++created;
    if (failure == 3u) return LIB_STATUS_NO_MEMORY;
    fixture.live = LIB_TRUE;
    *out_app = &fixture;
    return LIB_STATUS_OK;
}

lib_status vm_app_destroy(vm_app *app)
{
    if (app == LIB_NULL) return LIB_STATUS_OK;
    ++destroyed;
    app->live = LIB_FALSE;
    return failure == 8u ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK;
}

common_session *vm_app_session(const vm_app *app)
{ return app == LIB_NULL ? LIB_NULL : (common_session *)app; }

common_machine *vm_app_common_machine(const vm_app *app)
{ return app == LIB_NULL ? LIB_NULL : (common_machine *)app; }

common_ui *vm_app_ui(const vm_app *app)
{ return app == LIB_NULL ? LIB_NULL : (common_ui *)app; }

lib_bool common_session_enqueue_ui_event(void *context,
    const common_ui_event *event)
{
    (void)context;
    (void)event;
    return LIB_TRUE;
}

lib_status vm_app_compose_machine(vm_app *app, const vm_session_request *request)
{
    (void)app;
    (void)request;
    return failure == 4u ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK;
}

lib_status app_command_provider_initialize(app_command_context *command,
    common_machine *machine, common_session_display display,
    const app_command_extensions *extensions,
    common_session_command_provider *provider)
{
    (void)command;
    (void)machine;
    (void)display;
    (void)extensions;
    (void)provider;
    return failure == 5u ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK;
}

lib_bool app_keyboard_hotkeys(kvm_hotkey_registry *registry)
{
    (void)registry;
    return failure == 6u ? LIB_FALSE : LIB_TRUE;
}

lib_status vm_app_compose_control(vm_app *app, const common_session_options *options)
{
    (void)app;
    (void)options;
    return failure == 7u ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK;
}

lib_status vm_app_compose_ui(vm_app *app, const common_ui_options *options)
{
    (void)app;
    (void)options;
    return LIB_STATUS_OK;
}

lib_bool common_session_run(common_session *session)
{
    (void)session;
    return LIB_TRUE;
}

void app_command_dispose(app_command_context *command)
{ (void)command; }

const char *app_command_hotkey_help(void)
{ return "hotkeys"; }

lib_i32 main(void)
{
    const vm_app_definition definition = {
        .name = "PC", .version = "test", .copyright = "fixture",
        .build_time = "fixed", .configuration_file = "NXVM.ini",
        .load_request = fixture_load_request
    };
    lib_u32 index;

    if (vm_app_run(LIB_NULL) != 1) return 1;
    for (index = 0u; index <= 8u; ++index) {
        failure = index;
        fixture.live = LIB_FALSE;
        created = destroyed = 0u;
        if (vm_app_run(&definition) != (index == 0u ? 0 : 1)) return 2;
        if (index != 0u && index < 3u && created != 0u) return 3;
        if (index != 0u && index <= 3u && destroyed != 0u) {
            lib_c_printf("unexpected cleanup %u/%u\n", index, destroyed);
            return 4;
        }
        if (index >= 4u && destroyed != 1u) {
            lib_c_printf("missing cleanup %u/%u\n", index, destroyed);
            return 4;
        }
        if (fixture.live) return 5;
    }
    return 0;
}
