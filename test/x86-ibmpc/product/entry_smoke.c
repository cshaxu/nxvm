#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "lib/kvm-base/hotkey_interface.h"
#include "emulator/product/composition_interface.h"
#include "x86-ibmpc/product/entry_interface.h"

struct emulator_product { lib_bool live; };

static struct emulator_product fixture;
static lib_u32 failure;
static lib_u32 created;
static lib_u32 destroyed;
static lib_u32 disposed, title_calls, status_calls;
static const char *received_banner;

static lib_status fixture_bind(void *machine, emulator_machine *emulator)
{
    (void)machine;
    (void)emulator;
    return LIB_STATUS_OK;
}

static lib_status fixture_destroy(void *machine)
{
    (void)machine;
    ++destroyed;
    return LIB_STATUS_OK;
}

lib_bool emulator_session_enqueue_ui_event(void *context,
    const emulator_ui_event *event)
{
    (void)context;
    (void)event;
    return LIB_TRUE;
}

lib_status x86_ibmpc_product_command_provider_initialize(x86_ibmpc_product_command_context *command,
    emulator_machine *machine, emulator_session_display display,
    const x86_ibmpc_product_command_extensions *extensions,
    emulator_session_command_provider *provider)
{
    (void)command;
    (void)machine;
    (void)display;
    (void)extensions;
    (void)provider;
    return failure == 3u ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK;
}

lib_bool x86_ibmpc_product_keyboard_hotkeys(kvm_hotkey_registry *registry)
{
    (void)registry;
    return failure == 4u ? LIB_FALSE : LIB_TRUE;
}

lib_bool emulator_session_run(emulator_session *session)
{
    (void)session;
    return LIB_TRUE;
}

lib_i32 emulator_product_run(const emulator_product_definition *definition)
{
    emulator_product product = {0};
    emulator_session_options session_options = {0};
    emulator_ui_options ui_options = {0};

    if (definition == LIB_NULL || definition->configure_control == LIB_NULL ||
        definition->configure_ui == LIB_NULL) return 1;
    received_banner = definition->banner;
    ++created;
    if (failure == 1u) {
        ++destroyed;
        return 1;
    }
    if (failure == 2u || failure == 5u) return 1;
    fixture.live = LIB_TRUE;
    if (definition->configure_control(definition->context, (emulator_machine *)&product,
            &session_options) != LIB_STATUS_OK ||
        definition->configure_ui(definition->context, &ui_options) != LIB_STATUS_OK) {
        fixture.live = LIB_FALSE;
        return 1;
    }
    fixture.live = LIB_FALSE;
    return failure == 6u ? 1 : 0;
}

void x86_ibmpc_product_command_dispose(x86_ibmpc_product_command_context *command)
{ (void)command; ++disposed; }

emulator_product_help_map x86_ibmpc_product_keyboard_hotkey_help(void)
{ return (emulator_product_help_map){LIB_NULL, 0u}; }

lib_i32 main(void)
{
    const x86_ibmpc_product_definition definition = {
        .name = "PC",
        .banner = "PC",
        .machine = {.machine = &fixture, .bind = fixture_bind,
            .destroy = fixture_destroy},
        .ui = {.display = EMULATOR_SESSION_DISPLAY_CONSOLE}
    };
    x86_ibmpc_product_definition invalid_ui = definition;
    lib_u32 index;

    invalid_ui.ui.display = (emulator_session_display)99;
    if (x86_ibmpc_product_run(LIB_NULL) != 1 || x86_ibmpc_product_run(&invalid_ui) != 1) return 1;
    for (index = 0u; index <= 8u; ++index) {
        failure = index;
        fixture.live = LIB_FALSE;
        created = destroyed = disposed = title_calls = status_calls = 0u;
        received_banner = LIB_NULL;
        if (x86_ibmpc_product_run(&definition) != (index == 0u ? 0 : 1)) return 2;
        if (index == 1u && (created != 1u || destroyed != 1u)) return 3;
        if (index != 1u && (created != 1u || destroyed != 0u)) return 4;
        if (fixture.live) return 5;
        if (received_banner != definition.banner) return 6;
        if (disposed != 1u) return 7;
        if (index == 7u && (title_calls != 1u || status_calls != 0u)) return 8;
        if (index == 8u && (title_calls != 1u || status_calls != 1u)) return 9;
    }
    return 0;
}
lib_status emulator_product_monitor_format_window_titles(const char *name,
    char *out_running, lib_size running_capacity, char *out_paused,
    lib_size paused_capacity)
{
    ++title_calls;
    if (lib_text_compare(name, "PC") != 0 || out_running == LIB_NULL ||
        out_paused == LIB_NULL || running_capacity == 0u || paused_capacity == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (failure == 7u) return LIB_STATUS_LIMIT_EXCEEDED;
    out_running[0] = out_paused[0] = '\0';
    return LIB_STATUS_OK;
}

lib_status emulator_product_monitor_format_window_status(const char *name,
    emulator_product_help_map hotkeys, char *out_text, lib_size capacity)
{
    ++status_calls;
    if (lib_text_compare(name, "PC") != 0 || out_text == LIB_NULL ||
        capacity == 0u || hotkeys.rows != LIB_NULL || hotkeys.count != 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (failure == 8u) return LIB_STATUS_LIMIT_EXCEEDED;
    out_text[0] = '\0';
    return LIB_STATUS_OK;
}
