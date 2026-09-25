#include "product/composition.h"

#include "product/command.h"
#include "common/machine/machine_interface.h"
#include "common/session/session_interface.h"
#include "common/ui/ui_interface.h"
#include "core/driver_interface.h"
#include "lib/kvm-base/hotkey_interface.h"
#include "lib/storage/medium_interface.h"
#include "lib/types/file.h"

typedef struct app_composition
{
    core_driver *driver;
    common_machine *machine;
    common_session *session;
    common_ui *ui;
    app_command_context command;
    lib_u8 battery_path[APP_CONFIG_PATH_CAPACITY];
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
    lib_u8 battery_path[APP_CONFIG_PATH_CAPACITY] = { 0 };
    common_machine_state state;

    if (composition == LIB_NULL)
        return LIB_FALSE;
    state = common_machine_state_get(composition->machine);
    if (state != COMMON_MACHINE_STOPPED && state != COMMON_MACHINE_PAUSED)
        return LIB_FALSE;
    if (path != LIB_NULL && !app_battery_path((const lib_u8 *)path, battery_path,
            sizeof(battery_path))) return LIB_FALSE;
    if (app_composition_save_battery(composition) != LIB_STATUS_OK)
        return LIB_FALSE;
    if (!common_machine_set_removable_media(composition->machine, path,
                                            LIB_STORAGE_MEDIUM_READONLY))
        return LIB_FALSE;
    lib_memory_copy(composition->battery_path, battery_path, sizeof(battery_path));
    if (path != LIB_NULL)
        (void)core_driver_load_battery_ram(composition->driver,
                                           (const char *)composition->battery_path);
    return LIB_TRUE;
}

static void app_machine_state_sink(void *context, common_machine_state state,
                                   lib_u32 run_generation)
{
    app_composition *composition = context;
    common_session_machine_state session_state = COMMON_SESSION_MACHINE_ERROR;

    if (composition == LIB_NULL || composition->session == LIB_NULL)
        return;

    switch (state)
    {
    case COMMON_MACHINE_STOPPED:
        session_state = COMMON_SESSION_MACHINE_STOPPED;
        break;
    case COMMON_MACHINE_RUNNING:
        session_state = COMMON_SESSION_MACHINE_RUNNING;
        break;
    case COMMON_MACHINE_PAUSED:
        session_state = COMMON_SESSION_MACHINE_PAUSED;
        break;
    case COMMON_MACHINE_RESET_COMPLETED:
        if (composition->command.suppress_window_after_reset)
        {
            composition->command.suppress_window_after_reset = LIB_FALSE;
            composition->command.report_suppressed_reset = LIB_TRUE;
            session_state = COMMON_SESSION_MACHINE_PAUSED;
        }
        else
            session_state = COMMON_SESSION_MACHINE_RESET_COMPLETED;
        break;
    case COMMON_MACHINE_STARTING:
        session_state = COMMON_SESSION_MACHINE_INIT;
        break;
    case COMMON_MACHINE_ERROR:
        break;
    }
    if (state == COMMON_MACHINE_PAUSED || state == COMMON_MACHINE_STOPPED ||
        state == COMMON_MACHINE_ERROR)
        core_driver_request_input_reset(composition->driver);
    (void)common_session_enqueue_runtime_completed(composition->session, session_state,
                                                   run_generation);
}

static void app_machine_frame_sink(void *context, lib_u32 sequence,
                                   lib_bool graphics, lib_u32 run_generation)
{
    app_composition *composition = context;

    if (composition == LIB_NULL || composition->session == LIB_NULL)
        return;
    (void)common_session_enqueue_frame_completed(composition->session, sequence, graphics,
                                                 run_generation);
}

lib_i32 app_composition_run(const app_startup_config *config)
{
    app_composition *composition;
    common_machine_driver common_driver;
    common_session_options session_options;
    common_ui_options ui_options;
    kvm_hotkey_registry hotkeys;
    lib_status status;
    lib_i32 result = 1;

    if (config == LIB_NULL)
        return 1;
    composition = lib_allocate_zero(1u, sizeof(*composition));
    if (composition == LIB_NULL)
        return 1;
    if (core_driver_create(&composition->driver, &(core_driver_options){
                                                     .text_output = config->text_output}) != LIB_STATUS_OK)
        goto cleanup;
    if (core_driver_make_driver(composition->driver, &common_driver) != LIB_STATUS_OK)
        goto cleanup;
    if (common_machine_create(&composition->machine, &common_driver) != LIB_STATUS_OK)
        goto cleanup;
    if (config->rom_path[0] != '\0' &&
        !app_composition_set_media(composition, (const char *)config->rom_path))
        goto cleanup;

    app_command_initialize(&composition->command, composition->machine,
                           config->rom_path[0] != '\0',
                           config->text_output ? COMMON_SESSION_DISPLAY_CONSOLE : COMMON_SESSION_DISPLAY_WINDOW);
    composition->command.media_context = composition;
    composition->command.set_media = app_composition_set_media;
    session_options = (common_session_options){
        .display = config->text_output ? COMMON_SESSION_DISPLAY_CONSOLE : COMMON_SESSION_DISPLAY_WINDOW,
        .console_control = LIB_TRUE,
        .machine = composition->machine,
        .command = {
            .context = &composition->command,
            .open = app_command_open,
            .reject_line = app_command_reject_line,
            .submit_line = app_command_submit_line,
            .begin_external = app_command_begin_external,
            .handle_hotkey = app_command_handle_hotkey,
            .note_runtime = app_command_note_runtime,
            .note_broker = app_command_note_broker,
            .note_monitor_current = app_command_note_monitor_current}};
    if (common_session_create(&composition->session, &session_options) != LIB_STATUS_OK)
        goto cleanup;
    common_machine_set_state_sink(composition->machine, app_machine_state_sink, composition);
    common_machine_set_frame_sink(composition->machine, app_machine_frame_sink, composition);
    if (common_machine_state_get(composition->machine) != COMMON_MACHINE_STOPPED)
        goto cleanup;
    /* Binding an observer does not replay Common's initial state. Cartridge
     * attachment does not start execution; publish STOPPED for either case. */
    app_machine_state_sink(composition, COMMON_MACHINE_STOPPED,
                           common_machine_run_generation(composition->machine));

    kvm_hotkey_registry_initialize(&hotkeys);
    if (kvm_hotkey_registry_register(&hotkeys, KVM_KEY_ESCAPE, 0u,
                                     "pause-toggle") != LIB_STATUS_OK)
        goto cleanup;
    ui_options = (common_ui_options){
        .event_context = composition->session,
        .event_sink = common_session_enqueue_ui_event,
        .hotkeys = hotkeys,
        .running_window_title = "MyNes (Running)",
        .paused_window_title = "MyNes (Paused)",
        .graphics_console_status_text = "NES video requires a window."};
    if (common_ui_create(&composition->ui, &ui_options) != LIB_STATUS_OK)
        goto cleanup;
    if (common_session_bind_ui(composition->session, composition->ui) != LIB_STATUS_OK)
        goto cleanup;
    result = common_session_run(composition->session) == 1 ? 0 : 1;

cleanup:
    if (composition->machine != LIB_NULL)
    {
        status = common_machine_shutdown(composition->machine);
        if (status != LIB_STATUS_OK)
            return 1;
    }
    if (app_composition_save_battery(composition) != LIB_STATUS_OK)
        result = 1;
    if (composition->ui != LIB_NULL && common_ui_destroy(composition->ui) != LIB_STATUS_OK)
        return 1;
    composition->ui = LIB_NULL;
    if (composition->session != LIB_NULL &&
        common_session_destroy(composition->session) != LIB_STATUS_OK)
        return 1;
    composition->session = LIB_NULL;
    if (composition->machine != LIB_NULL &&
        common_machine_destroy(composition->machine) != LIB_STATUS_OK)
        return 1;
    composition->machine = LIB_NULL;
    if (core_driver_destroy(composition->driver) != LIB_STATUS_OK)
        return 1;
    composition->driver = LIB_NULL;
    lib_release(composition);
    return result;
}
