#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "product/surface/entry_interface.h"

struct product_surface { lib_bool live; };

static struct product_surface fixture;
static lib_u32 failure;
static lib_u32 created;
static lib_u32 destroyed;

static lib_status fixture_bind(void *machine, emulator_machine *emulator)
{
    (void)machine;
    (void)emulator;
    return LIB_STATUS_OK;
}

static void fixture_destroy(void *machine)
{
    (void)machine;
    ++destroyed;
}

lib_status product_surface_create(const app_composed_machine *machine, product_surface **out_app)
{
    (void)machine;
    ++created;
    if (failure == 1u) return LIB_STATUS_NO_MEMORY;
    fixture.live = LIB_TRUE;
    *out_app = &fixture;
    return LIB_STATUS_OK;
}

lib_status product_surface_destroy(product_surface *app)
{
    if (app == LIB_NULL) return LIB_STATUS_OK;
    app->live = LIB_FALSE;
    return failure == 6u ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK;
}

emulator_session *product_surface_session(const product_surface *app)
{ return app == LIB_NULL ? LIB_NULL : (emulator_session *)app; }

emulator_machine *product_surface_emulator_machine(const product_surface *app)
{ return app == LIB_NULL ? LIB_NULL : (emulator_machine *)app; }

emulator_ui *product_surface_ui(const product_surface *app)
{ return app == LIB_NULL ? LIB_NULL : (emulator_ui *)app; }

lib_bool emulator_session_enqueue_ui_event(void *context,
    const emulator_ui_event *event)
{
    (void)context;
    (void)event;
    return LIB_TRUE;
}

lib_status product_surface_compose_machine(product_surface *app)
{
    (void)app;
    return failure == 2u ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK;
}

lib_status product_surface_command_provider_initialize(product_surface_command_context *command,
    emulator_machine *machine, emulator_session_display display,
    const product_surface_command_extensions *extensions,
    emulator_session_command_provider *provider)
{
    (void)command;
    (void)machine;
    (void)display;
    (void)extensions;
    (void)provider;
    return failure == 3u ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK;
}

lib_bool product_surface_keyboard_hotkeys(kvm_hotkey_registry *registry)
{
    (void)registry;
    return failure == 4u ? LIB_FALSE : LIB_TRUE;
}

lib_status product_surface_compose_control(product_surface *app, const emulator_session_options *options)
{
    (void)app;
    (void)options;
    return failure == 5u ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK;
}

lib_status product_surface_compose_ui(product_surface *app, const emulator_ui_options *options)
{
    (void)app;
    (void)options;
    return LIB_STATUS_OK;
}

lib_bool emulator_session_run(emulator_session *session)
{
    (void)session;
    return LIB_TRUE;
}

void product_surface_command_dispose(product_surface_command_context *command)
{ (void)command; }

const char *product_surface_command_hotkey_help(void)
{ return "hotkeys"; }

lib_i32 main(void)
{
    const product_surface_definition definition = {
        .name = "PC",
        .machine = {.machine = &fixture, .bind = fixture_bind,
            .destroy = fixture_destroy},
        .ui = {.display = EMULATOR_SESSION_DISPLAY_CONSOLE}
    };
    product_surface_definition invalid_ui = definition;
    lib_u32 index;

    invalid_ui.ui.display = (emulator_session_display)99;
    if (product_surface_run(LIB_NULL) != 1 || product_surface_run(&invalid_ui) != 1) return 1;
    for (index = 0u; index <= 6u; ++index) {
        failure = index;
        fixture.live = LIB_FALSE;
        created = destroyed = 0u;
        if (product_surface_run(&definition) != (index == 0u ? 0 : 1)) return 2;
        if (index == 1u && (created != 1u || destroyed != 1u)) return 3;
        if (index != 1u && (created != 1u || destroyed != 0u)) return 4;
        if (fixture.live) return 5;
    }
    return 0;
}
