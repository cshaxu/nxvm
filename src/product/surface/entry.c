/* Copyright 2012-2026 Neko. */
#include "lib/types/file.h"

#include "product/surface/entry_interface.h"
#include "product/surface/command_provider_interface.h"
#include "product/surface/composition.h"
#include "product/surface/keyboard_interface.h"

#define PRODUCT_SURFACE_WINDOW_TEXT_CAPACITY 256u

lib_i32 product_surface_run(const product_surface_definition *definition)
{
    product_surface *app = LIB_NULL;
    product_surface_command_context commands = {0};
    product_surface_command_extensions extensions = {0};
    emulator_session_options session_options = {0};
    emulator_ui_options ui_options = {0};
    kvm_hotkey_registry hotkeys;
    lib_u8 running_title[PRODUCT_SURFACE_WINDOW_TEXT_CAPACITY];
    lib_u8 paused_title[PRODUCT_SURFACE_WINDOW_TEXT_CAPACITY];
    lib_u8 graphics_status[EMULATOR_SESSION_TEXT_CAPACITY];
    lib_status status;
    lib_status destroy_status;

    if (definition == LIB_NULL || definition->name == LIB_NULL ||
        definition->machine.machine == LIB_NULL ||
        definition->machine.bind == LIB_NULL || definition->machine.destroy == LIB_NULL)
        return 1;
    /* The App supplies only its product identity.  Product owns the stable
     * monitor preamble and its spacing before it opens the shared command
     * provider. */
    lib_c_printf("%s\n\nBuilt on %s %s\n\n", definition->name,
        __DATE__, __TIME__);
    if (definition->ui.display != EMULATOR_SESSION_DISPLAY_CONSOLE &&
        definition->ui.display != EMULATOR_SESSION_DISPLAY_WINDOW) {
        definition->machine.destroy(definition->machine.machine);
        return 1;
    }
    if (product_surface_create(&definition->machine, &app) != LIB_STATUS_OK) {
        definition->machine.destroy(definition->machine.machine);
        return 1;
    }
    status = product_surface_compose_machine(app);
    if (status == LIB_STATUS_OK && definition->configure_extensions != LIB_NULL)
        status = definition->configure_extensions(app, &extensions);
    if (status == LIB_STATUS_OK)
        status = product_surface_command_provider_initialize(&commands,
            product_surface_emulator_machine(app), definition->ui.display, &extensions,
            &session_options.command);
    if (status == LIB_STATUS_OK && !product_surface_keyboard_hotkeys(&hotkeys))
        status = LIB_STATUS_INTERNAL_ERROR;
    if (status == LIB_STATUS_OK) {
        session_options.display = definition->ui.display;
        session_options.console_control = definition->ui.console_control != 0 ?
            LIB_TRUE : LIB_FALSE;
        status = product_surface_compose_control(app, &session_options);
    }
    if (status == LIB_STATUS_OK &&
        (lib_c_snprintf((char *)running_title, sizeof(running_title),
            "%s (Running)", definition->name) < 0 ||
        lib_c_snprintf((char *)paused_title, sizeof(paused_title),
            "%s (Paused)", definition->name) < 0 ||
        lib_c_snprintf((char *)graphics_status, sizeof(graphics_status),
            "%s is running in the Window.\r\n\r\n%s\r\n",
            definition->name, product_surface_command_hotkey_help()) < 0))
        status = LIB_STATUS_LIMIT_EXCEEDED;
    if (status == LIB_STATUS_OK) {
        ui_options.event_context = product_surface_session(app);
        ui_options.event_sink = emulator_session_enqueue_ui_event;
        ui_options.hotkeys = hotkeys;
        ui_options.running_window_title = (const char *)running_title;
        ui_options.paused_window_title = (const char *)paused_title;
        ui_options.graphics_console_status_text = (const char *)graphics_status;
        status = product_surface_compose_ui(app, &ui_options);
    }
    if (status == LIB_STATUS_OK && !emulator_session_run(product_surface_session(app)))
        status = LIB_STATUS_INTERNAL_ERROR;
    product_surface_command_dispose(&commands);
    destroy_status = product_surface_destroy(app);
    return status == LIB_STATUS_OK && destroy_status == LIB_STATUS_OK ? 0 : 1;
}
