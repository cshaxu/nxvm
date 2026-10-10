/* Copyright 2012-2026 Neko. */
#include "lib/types/file.h"

#include "x86/product/entry_interface.h"
#include "x86/product/command_provider_interface.h"
#include "emulator/product/composition_interface.h"
#include "emulator/product/monitor_interface.h"
#include "x86/product/keyboard_interface.h"

#define X86_PRODUCT_WINDOW_TEXT_CAPACITY 256u

typedef struct x86_product_run_context {
    const x86_product_definition *definition;
    app_composed_machine machine;
    x86_product_command_context command;
    x86_product_command_extensions extensions;
    kvm_hotkey_registry hotkeys;
    lib_u8 running_title[X86_PRODUCT_WINDOW_TEXT_CAPACITY];
    lib_u8 paused_title[X86_PRODUCT_WINDOW_TEXT_CAPACITY];
    lib_u8 graphics_status[EMULATOR_SESSION_TEXT_CAPACITY];
} x86_product_run_context;

static lib_status x86_product_configure_control(void *opaque,
    emulator_machine *machine, emulator_session_options *out_options)
{
    x86_product_run_context *context = opaque;
    lib_status status;

    if (context == LIB_NULL || machine == LIB_NULL || out_options == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (context->definition->configure_extensions != LIB_NULL) {
        status = context->definition->configure_extensions(&context->machine,
            &context->extensions);
        if (status != LIB_STATUS_OK) return status;
    }
    status = x86_product_command_provider_initialize(&context->command,
        machine, context->definition->ui.display,
        &context->extensions, &out_options->command);
    if (status != LIB_STATUS_OK) return status;
    if (!x86_product_keyboard_hotkeys(&context->hotkeys)) return LIB_STATUS_INTERNAL_ERROR;
    out_options->display = context->definition->ui.display;
    out_options->console_control = context->definition->ui.console_control != 0 ?
        LIB_TRUE : LIB_FALSE;
    return LIB_STATUS_OK;
}

static lib_status x86_product_configure_ui(void *opaque,
    emulator_ui_options *out_options)
{
    x86_product_run_context *context = opaque;
    if (context == LIB_NULL || out_options == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (emulator_product_monitor_format_window_titles(context->definition->name,
            (char *)context->running_title, sizeof(context->running_title),
            (char *)context->paused_title, sizeof(context->paused_title)) != LIB_STATUS_OK ||
        emulator_product_monitor_format_window_status(context->definition->name,
            x86_product_keyboard_hotkey_help(), (char *)context->graphics_status,
            sizeof(context->graphics_status)) != LIB_STATUS_OK) {
        return LIB_STATUS_LIMIT_EXCEEDED;
    }
    out_options->hotkeys = context->hotkeys;
    out_options->running_window_title = (const char *)context->running_title;
    out_options->paused_window_title = (const char *)context->paused_title;
    out_options->graphics_console_status_text = (const char *)context->graphics_status;
    return LIB_STATUS_OK;
}

lib_i32 x86_product_run(const x86_product_definition *definition)
{
    x86_product_run_context context = {0};
    lib_i32 result;

    if (definition == LIB_NULL || definition->name == LIB_NULL ||
        definition->banner == LIB_NULL ||
        (definition->ui.display != EMULATOR_SESSION_DISPLAY_CONSOLE &&
         definition->ui.display != EMULATOR_SESSION_DISPLAY_WINDOW)) return 1;
    context.definition = definition;
    context.machine = definition->machine;
    result = emulator_product_run(&(emulator_product_definition){
        .banner = definition->banner,
        .machine = definition->machine.composition,
        .context = &context,
        .configure_control = x86_product_configure_control,
        .configure_ui = x86_product_configure_ui});
    x86_product_command_dispose(&context.command);
    return result;
}
