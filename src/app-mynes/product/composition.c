#include "product/composition.h"

#include "product/command.h"
#include "product/version_interface.h"
#include "emulator/product/composition_interface.h"
#include "emulator/product/monitor_interface.h"
#include "emulator/machine/machine_interface.h"
#include "emulator/session/session_interface.h"
#include "emulator/ui/ui_interface.h"
#include "core/driver_interface.h"
#include "lib/kvm-base/hotkey_interface.h"
#include "lib/storage/medium_interface.h"
#include "lib/types/file.h"

#define APP_COMPOSITION_WINDOW_TITLE_CAPACITY 256u

typedef struct app_composition
{
    core_driver *driver;
    app_command_context command;
    lib_u8 startup_rom_path[APP_CONFIG_PATH_CAPACITY];
    lib_u8 battery_path[APP_CONFIG_PATH_CAPACITY];
    lib_u8 running_window_title[APP_COMPOSITION_WINDOW_TITLE_CAPACITY];
    lib_u8 paused_window_title[APP_COMPOSITION_WINDOW_TITLE_CAPACITY];
    lib_u8 graphics_console_status[EMULATOR_SESSION_TEXT_CAPACITY];
    lib_bool console_control;
} app_composition;

static lib_bool app_battery_path(const lib_u8 *rom_path, lib_u8 *out_path,
                                 lib_size capacity)
{
    const lib_u8 *cursor;
    const lib_u8 *extension = LIB_NULL;
    lib_size length;

    if (rom_path == LIB_NULL || out_path == LIB_NULL || capacity < 5u)
        return LIB_FALSE;
    cursor = rom_path;
    while (*cursor != '\0')
    {
        if (*cursor == '/' || *cursor == '\\')
            extension = LIB_NULL;
        else if (*cursor == '.')
            extension = cursor;
        ++cursor;
    }
    length = extension == LIB_NULL ? (lib_size)(cursor - rom_path) : (lib_size)(extension - rom_path);
    if (length == 0u || length + sizeof(".sav") > capacity)
        return LIB_FALSE;
    lib_memory_copy(out_path, rom_path, length);
    lib_memory_copy(out_path + length, ".sav", sizeof(".sav"));
    return LIB_TRUE;
}

static lib_status app_composition_save_battery(app_composition *composition)
{
    lib_status status = LIB_STATUS_OK;

    if (composition->battery_path[0] != '\0')
        status = core_driver_save_battery_ram(composition->driver,
                                              (const char *)composition->battery_path);
    if (status != LIB_STATUS_OK)
        lib_c_fprintf(lib_c_stderr, "Cannot save battery RAM: %s\n",
                      (const char *)composition->battery_path);
    return status;
}

static lib_bool app_composition_set_media(void *opaque, const char *path)
{
    app_composition *composition = opaque;
    lib_u8 battery_path[APP_CONFIG_PATH_CAPACITY] = {0};
    emulator_machine *machine = composition == LIB_NULL ? LIB_NULL : composition->command.machine;
    emulator_machine_state state;

    if (machine == LIB_NULL)
        return LIB_FALSE;
    state = emulator_machine_state_get(machine);
    if (state != EMULATOR_MACHINE_STOPPED && state != EMULATOR_MACHINE_PAUSED)
        return LIB_FALSE;
    if (path != LIB_NULL && !app_battery_path((const lib_u8 *)path, battery_path,
                                              sizeof(battery_path)))
        return LIB_FALSE;
    if (app_composition_save_battery(composition) != LIB_STATUS_OK)
        return LIB_FALSE;
    if (!emulator_machine_set_removable_media(machine, path,
                                              LIB_STORAGE_MEDIUM_READONLY))
        return LIB_FALSE;
    lib_memory_copy(composition->battery_path, battery_path, sizeof(battery_path));
    if (path != LIB_NULL)
        (void)core_driver_load_battery_ram(composition->driver,
                                           (const char *)composition->battery_path);
    return LIB_TRUE;
}

static emulator_session_machine_state app_composition_map_state(void *context,
                                                                emulator_machine_state state)
{
    app_composition *composition = context;
    emulator_session_machine_state session_state = EMULATOR_SESSION_MACHINE_ERROR;

    if (composition == LIB_NULL)
        return session_state;

    switch (state)
    {
    case EMULATOR_MACHINE_STOPPED:
        session_state = EMULATOR_SESSION_MACHINE_STOPPED;
        break;
    case EMULATOR_MACHINE_RUNNING:
        session_state = EMULATOR_SESSION_MACHINE_RUNNING;
        break;
    case EMULATOR_MACHINE_PAUSED:
        session_state = EMULATOR_SESSION_MACHINE_PAUSED;
        break;
    case EMULATOR_MACHINE_RESET_COMPLETED:
        session_state = EMULATOR_SESSION_MACHINE_RESET_COMPLETED;
        break;
    case EMULATOR_MACHINE_STARTING:
        session_state = EMULATOR_SESSION_MACHINE_INIT;
        break;
    case EMULATOR_MACHINE_ERROR:
        break;
    }
    if (state == EMULATOR_MACHINE_PAUSED || state == EMULATOR_MACHINE_STOPPED ||
        state == EMULATOR_MACHINE_ERROR)
        core_driver_request_input_reset(composition->driver);
    return session_state;
}

static lib_status app_composition_destroy_machine(void *opaque)
{
    app_composition *composition = opaque;
    lib_status status;

    if (composition == LIB_NULL || composition->driver == LIB_NULL)
        return LIB_STATUS_INVALID_STATE;
    status = app_composition_save_battery(composition);
    if (status != LIB_STATUS_OK)
        return status;
    status = core_driver_destroy(composition->driver);
    if (status == LIB_STATUS_OK)
        composition->driver = LIB_NULL;
    return status;
}

static lib_status app_composition_bind_machine(void *opaque,
                                               emulator_machine *machine)
{
    app_composition *composition = opaque;

    if (composition == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    /* NULL is the ordered teardown revoke of this non-owning link. */
    composition->command.machine = machine;
    return LIB_STATUS_OK;
}

static lib_status app_composition_configure_control(void *opaque,
                                                    emulator_machine *machine, emulator_session_options *out_options)
{
    app_composition *composition = opaque;

    if (composition == LIB_NULL || machine == LIB_NULL || out_options == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (composition->command.cartridge_present &&
        !app_composition_set_media(composition,
                                   (const char *)composition->startup_rom_path))
        return LIB_STATUS_IO_ERROR;
    app_command_initialize(&composition->command, machine,
                           composition->command.cartridge_present, composition->command.display);
    composition->command.media_context = composition;
    composition->command.set_media = app_composition_set_media;
    *out_options = (emulator_session_options){
        .display = composition->command.display,
        .console_control = composition->console_control,
        .command = {
            .context = &composition->command,
            .open = app_command_open,
            .reject_line = app_command_reject_line,
            .submit_line = app_command_submit_line,
            .handle_hotkey = app_command_handle_hotkey,
            .note_runtime = app_command_note_runtime}};
    return LIB_STATUS_OK;
}

static lib_status app_composition_configure_ui(void *opaque,
                                               emulator_ui_options *out_options)
{
    app_composition *composition = opaque;
    kvm_hotkey_registry hotkeys;

    if (composition == LIB_NULL || out_options == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    kvm_hotkey_registry_initialize(&hotkeys);
    if (kvm_hotkey_registry_register(&hotkeys, KVM_KEY_ESCAPE, 0u,
                                     "pause-toggle") != LIB_STATUS_OK)
        return LIB_STATUS_INTERNAL_ERROR;
    if (emulator_product_monitor_format_window_titles("MyNES",
            (char *)composition->running_window_title,
            sizeof(composition->running_window_title),
            (char *)composition->paused_window_title,
            sizeof(composition->paused_window_title)) != LIB_STATUS_OK ||
        emulator_product_monitor_format_window_status("MyNES", app_command_hotkey_help(),
            (char *)composition->graphics_console_status,
            sizeof(composition->graphics_console_status)) != LIB_STATUS_OK)
        return LIB_STATUS_LIMIT_EXCEEDED;
    *out_options = (emulator_ui_options){
        .hotkeys = hotkeys,
        .running_window_title = (const char *)composition->running_window_title,
        .paused_window_title = (const char *)composition->paused_window_title,
        .graphics_console_status_text = (const char *)composition->graphics_console_status};
    return LIB_STATUS_OK;
}

lib_i32 app_composition_run(const app_startup_config *config)
{
    app_composition *composition;
    emulator_machine_driver emulator_driver;
    lib_i32 result = 1;

    if (config == LIB_NULL)
        return 1;
    composition = lib_allocate_zero(1u, sizeof(*composition));
    if (composition == LIB_NULL)
        return 1;
    if (core_driver_create(&composition->driver, &(core_driver_options){
                                                     .text_output = config->display ==
                                                                    EMULATOR_SESSION_DISPLAY_CONSOLE,
                                                     .audio_enabled = LIB_TRUE}) != LIB_STATUS_OK)
        goto cleanup;
    if (core_driver_make_driver(composition->driver, &emulator_driver) != LIB_STATUS_OK)
        goto cleanup;
    composition->command.cartridge_present = config->rom_path[0] != '\0';
    composition->console_control = config->console_control;
    composition->command.display = config->display;
    if (composition->command.cartridge_present)
        lib_memory_copy(composition->startup_rom_path, config->rom_path,
                        lib_text_length((const char *)config->rom_path) + 1u);
    result = emulator_product_run(&(emulator_product_definition){
        .banner = APP_PRODUCT_BANNER,
        .machine = {
            .machine = composition,
            .driver = emulator_driver,
            .bind = app_composition_bind_machine,
            .destroy = app_composition_destroy_machine,
            .state_context = composition,
            .map_state = app_composition_map_state},
        .context = composition,
        .configure_control = app_composition_configure_control,
        .configure_ui = app_composition_configure_ui});
cleanup:
    if (composition->driver != LIB_NULL &&
        app_composition_destroy_machine(composition) != LIB_STATUS_OK)
        return 1;
    lib_release(composition);
    return result;
}
