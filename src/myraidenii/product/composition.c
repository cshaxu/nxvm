#include "myraidenii/product/composition_interface.h"

#include "emulator/product/composition_interface.h"
#include "emulator/product/monitor_interface.h"
#include "lib/kvm-base/hotkey_interface.h"
#include "lib/audio/pcm/stream_interface.h"
#include "myraidenii/core/audio_interface.h"
#include "myraidenii/core/board_interface.h"
#include "myraidenii/machine/configuration_interface.h"
#include "myraidenii/machine/driver_interface.h"
#include "myraidenii/product/version_interface.h"
#include "lib/types/file.h"

enum {
    MYRAIDENII_PRODUCT_WINDOW_TITLE_CAPACITY = 256u
};

typedef struct myraidenii_product_composition {
    const myraidenii_product_config *config;
    myraidenii_core_board *board;
    myraidenii_machine *machine;
    emulator_machine *emulator;
    emulator_machine_driver driver;
    emulator_product_monitor_provider monitor;
    char running_window_title[MYRAIDENII_PRODUCT_WINDOW_TITLE_CAPACITY];
    char paused_window_title[MYRAIDENII_PRODUCT_WINDOW_TITLE_CAPACITY];
    char graphics_console_status[EMULATOR_SESSION_TEXT_CAPACITY];
} myraidenii_product_composition;

/* These host bindings are Product policy (L1).  Machine alone translates the
 * resulting ordinary KVM events into active-low board controls. */
static const emulator_product_help_row myraidenii_product_hotkeys[] = {
    {"Arrow keys", "Player 1 move"},
    {"Z / X", "Player 1 fire / bomb"},
    {"W / A / S / D", "Player 2 move"},
    {"J / K", "Player 2 fire / bomb"},
    {"1 / 2", "Player 1 / 2 start"},
    {"5 / 6", "Player 1 / 2 coin"},
    {"F2", "service"},
    {"Esc", "pause or resume"}
};

static lib_bool myraidenii_product_handle_hotkey(void *opaque,
    emulator_session_machine_state state, const lib_u8 *identifier,
    emulator_session_command_result *out_result)
{
    emulator_product_monitor_provider *monitor = opaque;
    emulator_session_request request;

    if (monitor == LIB_NULL || identifier == LIB_NULL || out_result == LIB_NULL ||
        lib_text_compare((const char *)identifier, "pause-toggle") != 0)
        return LIB_FALSE;
    if (!emulator_session_request_pause_toggle(state, &request))
        request = EMULATOR_SESSION_REQUEST_PAUSE;
    return emulator_product_monitor_request_lifecycle(monitor,
        request == EMULATOR_SESSION_REQUEST_RESUME ?
            EMULATOR_PRODUCT_MONITOR_COMMAND_RESUME :
            EMULATOR_PRODUCT_MONITOR_COMMAND_PAUSE,
        state, out_result);
}

static void myraidenii_product_note_runtime(void *opaque,
    emulator_session_machine_state prior, emulator_session_machine_state completed,
    emulator_session_command_result *out_result)
{
    emulator_product_monitor_provider *monitor = opaque;
    myraidenii_product_composition *composition = monitor == LIB_NULL ? LIB_NULL : monitor->context;

    emulator_product_monitor_provider_note_runtime(opaque, prior, completed, out_result);
    if (composition != LIB_NULL && out_result != LIB_NULL &&
        prior == EMULATOR_SESSION_MACHINE_PAUSED && completed == EMULATOR_SESSION_MACHINE_RUNNING &&
        !myraidenii_machine_release_inputs(composition->emulator)) {
        lib_memory_copy(out_result->text, "Input reset failed; stopping machine.\r\n",
            sizeof("Input reset failed; stopping machine.\r\n"));
        out_result->request = EMULATOR_SESSION_REQUEST_STOP;
    }
}

static lib_status myraidenii_product_destroy_machine(void *opaque)
{
    myraidenii_product_composition *composition = opaque;
    lib_status status;

    if (composition == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = myraidenii_machine_destroy(&composition->machine);
    if (status != LIB_STATUS_OK) return status;
    myraidenii_core_board_destroy(composition->board);
    composition->board = LIB_NULL;
    return LIB_STATUS_OK;
}

static lib_status myraidenii_product_bind_machine(void *opaque,
    emulator_machine *emulator)
{
    myraidenii_product_composition *composition = opaque;

    if (composition == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    composition->emulator = emulator;
    return LIB_STATUS_OK;
}

static lib_status myraidenii_product_configure_control(void *opaque,
    emulator_machine *emulator, emulator_session_options *out_options)
{
    myraidenii_product_composition *composition = opaque;

    if (composition == LIB_NULL || emulator == LIB_NULL || out_options == LIB_NULL ||
        composition->config == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (composition->config->display != EMULATOR_SESSION_DISPLAY_WINDOW)
        return LIB_STATUS_UNSUPPORTED;
    composition->monitor = (emulator_product_monitor_provider){
        .context = composition,
        .hotkeys = {myraidenii_product_hotkeys,
            sizeof(myraidenii_product_hotkeys) / sizeof(myraidenii_product_hotkeys[0])}};
    *out_options = (emulator_session_options){
        .display = composition->config->display,
        .console_control = composition->config->console_control,
        .command = {
            .context = &composition->monitor,
            .open = emulator_product_monitor_provider_open,
            .reject_line = emulator_product_monitor_provider_reject_line,
            .submit_line = emulator_product_monitor_provider_submit_line,
            .handle_hotkey = myraidenii_product_handle_hotkey,
            .note_runtime = myraidenii_product_note_runtime}};
    return LIB_STATUS_OK;
}

static lib_status myraidenii_product_configure_ui(void *opaque,
    emulator_ui_options *out_options)
{
    myraidenii_product_composition *composition = opaque;
    kvm_hotkey_registry hotkeys;
    lib_status status;

    if (composition == LIB_NULL || out_options == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = emulator_product_monitor_format_window_titles(MYRAIDENII_PRODUCT_NAME,
        composition->running_window_title, sizeof(composition->running_window_title),
        composition->paused_window_title, sizeof(composition->paused_window_title));
    if (status != LIB_STATUS_OK) return status;
    status = emulator_product_monitor_format_window_status(MYRAIDENII_PRODUCT_NAME,
        (emulator_product_help_map){myraidenii_product_hotkeys,
            sizeof(myraidenii_product_hotkeys) / sizeof(myraidenii_product_hotkeys[0])},
        composition->graphics_console_status,
        sizeof(composition->graphics_console_status));
    if (status != LIB_STATUS_OK) return status;
    kvm_hotkey_registry_initialize(&hotkeys);
    if (kvm_hotkey_registry_register(&hotkeys, BASE_KEY_ESCAPE, 0u,
            "pause-toggle") != LIB_STATUS_OK)
        return LIB_STATUS_INTERNAL_ERROR;
    *out_options = (emulator_ui_options){
        .hotkeys = hotkeys,
        .running_window_title = composition->running_window_title,
        .paused_window_title = composition->paused_window_title,
        .graphics_console_status_text = composition->graphics_console_status};
    return LIB_STATUS_OK;
}

static lib_status myraidenii_product_construct(myraidenii_product_composition *composition)
{
    const char *paths[MYRAIDENII_MACHINE_IMAGE_COUNT];
    myraidenii_machine_images *images = LIB_NULL;
    myraidenii_core_board_config board_config;
    lib_audio_pcm_stream *stream = LIB_NULL;
    lib_status status;

    if (composition == LIB_NULL || composition->config == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    for (lib_size index = 0u; index < MYRAIDENII_MACHINE_IMAGE_COUNT; ++index)
        paths[index] = (const char *)composition->config->image_paths[index];
    status = myraidenii_machine_images_load((const char *)composition->config->ini_directory,
        paths, &images);
    if (status == LIB_STATUS_OK)
        status = myraidenii_machine_images_board_config(images, &board_config);
    if (status == LIB_STATUS_OK)
        status = myraidenii_core_board_create(&board_config, &composition->board);
    myraidenii_machine_images_destroy(images);
    if (status != LIB_STATUS_OK) return status;
    status = lib_audio_pcm_stream_create(&(lib_audio_pcm_stream_options){
        .sample_rate = MYRAIDENII_CORE_AUDIO_PCM_SAMPLE_RATE,
        .channel_count = MYRAIDENII_CORE_AUDIO_PCM_CHANNELS}, &stream);
    if (status == LIB_STATUS_IO_ERROR || status == LIB_STATUS_UNSUPPORTED)
        status = LIB_STATUS_OK;
    if (status == LIB_STATUS_OK)
        status = myraidenii_machine_create(composition->board, stream, &composition->machine);
    if (status != LIB_STATUS_OK) {
        (void)lib_audio_pcm_stream_destroy(&stream);
        myraidenii_core_board_destroy(composition->board);
        composition->board = LIB_NULL;
        return status;
    }
    status = myraidenii_machine_driver_get(composition->machine, &composition->driver);
    if (status != LIB_STATUS_OK) {
        lib_status cleanup_status = myraidenii_product_destroy_machine(composition);

        return cleanup_status == LIB_STATUS_OK ? status : cleanup_status;
    }
    return LIB_STATUS_OK;
}

lib_i32 myraidenii_product_run(const myraidenii_product_config *config)
{
    myraidenii_product_composition *composition;
    lib_i32 result = 1;
    lib_status status;

    if (config == LIB_NULL) return 1;
    if (config->display != EMULATOR_SESSION_DISPLAY_WINDOW) {
        lib_c_fprintf(lib_c_stderr, "Console display is not supported; set display=window in myraidenii.ini.\n");
        return 1;
    }
    composition = lib_allocate_zero(1u, sizeof(*composition));
    if (composition == LIB_NULL) {
        lib_c_fprintf(lib_c_stderr, "Cannot allocate machine composition.\n");
        return 1;
    }
    composition->config = config;
    status = myraidenii_product_construct(composition);
    if (status == LIB_STATUS_OK) {
        result = emulator_product_run(&(emulator_product_definition){
            .banner = MYRAIDENII_PRODUCT_BANNER,
            .machine = {
                .machine = composition,
                .driver = composition->driver,
                .bind = myraidenii_product_bind_machine,
                .destroy = myraidenii_product_destroy_machine},
            .context = composition,
            .configure_control = myraidenii_product_configure_control,
            .configure_ui = myraidenii_product_configure_ui});
    } else lib_c_fprintf(lib_c_stderr,
        "Cannot initialize machine or ROM images (status %d); check ROM paths and identities.\n",
        (int)status);
    /* A failed shared shutdown retains the binding and may retain a worker.
     * Keep its callback context and private machine alive until process exit. */
    if (composition->emulator != LIB_NULL) return 1;
    if ((composition->machine != LIB_NULL || composition->board != LIB_NULL) &&
        myraidenii_product_destroy_machine(composition) != LIB_STATUS_OK)
        return 1;
    lib_release(composition);
    return result;
}
