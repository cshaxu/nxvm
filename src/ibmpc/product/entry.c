/* Copyright 2012-2026 Neko. */
#include "lib/types/file.h"

#include "ibmpc/product/entry_interface.h"
#include "ibmpc/product/command_provider_interface.h"
#include "ibmpc/product/composition.h"
#include "ibmpc/product/keyboard_interface.h"

#define VM_APP_WINDOW_TEXT_CAPACITY 256u

lib_i32 vm_app_run(const vm_app_definition *definition)
{
    vm_app *app = LIB_NULL;
    vm_session_request request;
    app_command_context commands = {0};
    app_command_extensions extensions = {0};
    common_session_options session_options = {0};
    common_ui_options ui_options = {0};
    kvm_hotkey_registry hotkeys;
    lib_u8 running_title[VM_APP_WINDOW_TEXT_CAPACITY];
    lib_u8 paused_title[VM_APP_WINDOW_TEXT_CAPACITY];
    lib_u8 graphics_status[COMMON_SESSION_TEXT_CAPACITY];
    common_session_display display;
    lib_status status;
    lib_status destroy_status;

    if (definition == LIB_NULL || definition->name == LIB_NULL ||
        definition->version == LIB_NULL || definition->copyright == LIB_NULL ||
        definition->build_time == LIB_NULL ||
        definition->configuration_file == LIB_NULL ||
        definition->load_request == LIB_NULL)
        return 1;
    lib_c_printf("%s [%s]\n%s\n\nBuilt on %s\n\n", definition->name,
        definition->version, definition->copyright, definition->build_time);
    if (definition->load_request(definition->configuration_file, &request) !=
        LIB_STATUS_OK) {
        lib_c_printf("Unable to load %s.\n", definition->configuration_file);
        return 1;
    }
    if (vm_app_create(&definition->factory, &app) != LIB_STATUS_OK) return 1;
    status = vm_app_compose_machine(app, &request);
    display = lib_text_compare((const char *)request.display, "window") == 0 ?
        COMMON_SESSION_DISPLAY_WINDOW : COMMON_SESSION_DISPLAY_CONSOLE;
    if (status == LIB_STATUS_OK && definition->configure_extensions != LIB_NULL)
        status = definition->configure_extensions(app, &extensions);
    if (status == LIB_STATUS_OK)
        status = app_command_provider_initialize(&commands,
            vm_app_common_machine(app), display, &extensions,
            &session_options.command);
    if (status == LIB_STATUS_OK && !app_keyboard_hotkeys(&hotkeys))
        status = LIB_STATUS_INTERNAL_ERROR;
    if (status == LIB_STATUS_OK) {
        session_options.display = display;
        session_options.console_control = request.console_control != 0 ?
            LIB_TRUE : LIB_FALSE;
        status = vm_app_compose_control(app, &session_options);
    }
    if (status == LIB_STATUS_OK &&
        (lib_c_snprintf((char *)running_title, sizeof(running_title),
            "%s (Running)", definition->name) < 0 ||
        lib_c_snprintf((char *)paused_title, sizeof(paused_title),
            "%s (Paused)", definition->name) < 0 ||
        lib_c_snprintf((char *)graphics_status, sizeof(graphics_status),
            "%s is running in the Window.\r\n\r\n%s\r\n",
            definition->name, app_command_hotkey_help()) < 0))
        status = LIB_STATUS_LIMIT_EXCEEDED;
    if (status == LIB_STATUS_OK) {
        ui_options.event_context = vm_app_session(app);
        ui_options.event_sink = common_session_enqueue_ui_event;
        ui_options.hotkeys = hotkeys;
        ui_options.running_window_title = (const char *)running_title;
        ui_options.paused_window_title = (const char *)paused_title;
        ui_options.graphics_console_status_text = (const char *)graphics_status;
        status = vm_app_compose_ui(app, &ui_options);
    }
    if (status == LIB_STATUS_OK && !common_session_run(vm_app_session(app)))
        status = LIB_STATUS_INTERNAL_ERROR;
    app_command_dispose(&commands);
    destroy_status = vm_app_destroy(app);
    return status == LIB_STATUS_OK && destroy_status == LIB_STATUS_OK ? 0 : 1;
}
