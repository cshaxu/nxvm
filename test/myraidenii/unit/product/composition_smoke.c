#include <assert.h>

#include "emulator/product/composition_interface.h"
#include "emulator/product/monitor_interface.h"
#include "lib/audio/pcm/stream_interface.h"
#include "lib/types/file.h"
#include "myraidenii/core/audio_interface.h"
#include "myraidenii/machine/configuration_interface.h"
#include "myraidenii/machine/driver_interface.h"
#include "myraidenii/product/composition_interface.h"
#include "myraidenii/product/version_interface.h"

static lib_status image_status = LIB_STATUS_OK;
static lib_status board_status = LIB_STATUS_OK;
static lib_status stream_status = LIB_STATUS_OK;
static lib_status machine_status = LIB_STATUS_OK;
static lib_status driver_status = LIB_STATUS_OK;
static lib_i32 image_destroy_count;
static lib_i32 board_destroy_count;
static lib_i32 stream_destroy_count;
static lib_i32 machine_destroy_count;
static lib_i32 bind_count;
static lib_i32 hotkey_count;

lib_status myraidenii_machine_images_load(const char *directory,
    const char *const paths[MYRAIDENII_MACHINE_IMAGE_COUNT],
    myraidenii_machine_images **out_images)
{
    if (out_images == LIB_NULL || directory == LIB_NULL || paths == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    assert(lib_text_compare(directory, "C:\\raiden") == 0);
    for (lib_size index = 0u; index < MYRAIDENII_MACHINE_IMAGE_COUNT; ++index)
        assert(paths[index] != LIB_NULL && paths[index][0] != '\0');
    *out_images = image_status == LIB_STATUS_OK ?
        (myraidenii_machine_images *)(lib_uptr)1u : LIB_NULL;
    return image_status;
}

void myraidenii_machine_images_destroy(myraidenii_machine_images *images)
{
    if (images != LIB_NULL) ++image_destroy_count;
}

lib_status myraidenii_machine_images_board_config(const myraidenii_machine_images *images,
    myraidenii_core_board_config *out_config)
{
    if (images == LIB_NULL || out_config == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_config = (myraidenii_core_board_config){0};
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_board_create(const myraidenii_core_board_config *config,
    myraidenii_core_board **out_board)
{
    if (config == LIB_NULL || out_board == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_board = board_status == LIB_STATUS_OK ?
        (myraidenii_core_board *)(lib_uptr)2u : LIB_NULL;
    return board_status;
}

void myraidenii_core_board_destroy(myraidenii_core_board *board)
{
    if (board != LIB_NULL) ++board_destroy_count;
}

lib_status lib_audio_pcm_stream_create(const lib_audio_pcm_stream_options *options,
    lib_audio_pcm_stream **out_stream)
{
    if (options == LIB_NULL || out_stream == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    assert(options->sample_rate == MYRAIDENII_CORE_AUDIO_PCM_SAMPLE_RATE);
    assert(options->channel_count == MYRAIDENII_CORE_AUDIO_PCM_CHANNELS);
    *out_stream = stream_status == LIB_STATUS_OK ?
        (lib_audio_pcm_stream *)(lib_uptr)3u : LIB_NULL;
    return stream_status;
}

lib_status lib_audio_pcm_stream_destroy(lib_audio_pcm_stream **stream)
{
    if (stream == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (*stream != LIB_NULL) {
        ++stream_destroy_count;
        *stream = LIB_NULL;
    }
    return LIB_STATUS_OK;
}

lib_status myraidenii_machine_create(myraidenii_core_board *board, lib_audio_pcm_stream *stream,
    myraidenii_machine **out_machine)
{
    (void)stream;
    if (board == LIB_NULL || out_machine == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_machine = machine_status == LIB_STATUS_OK ?
        (myraidenii_machine *)(lib_uptr)4u : LIB_NULL;
    return machine_status;
}

lib_status myraidenii_machine_destroy(myraidenii_machine **machine)
{
    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (*machine != LIB_NULL) {
        ++machine_destroy_count;
        *machine = LIB_NULL;
    }
    return LIB_STATUS_OK;
}

lib_status myraidenii_machine_driver_get(myraidenii_machine *machine,
    emulator_machine_driver *out_driver)
{
    if (machine == LIB_NULL || out_driver == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (driver_status != LIB_STATUS_OK) return driver_status;
    *out_driver = (emulator_machine_driver){.context = machine};
    return LIB_STATUS_OK;
}

lib_status emulator_product_monitor_format_window_titles(const char *name,
    char *running, lib_size running_capacity, char *paused, lib_size paused_capacity)
{
    if (name == LIB_NULL || running == LIB_NULL || paused == LIB_NULL ||
        running_capacity == 0u || paused_capacity == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    assert(lib_text_compare(name, MYRAIDENII_PRODUCT_NAME) == 0);
    running[0] = '\0';
    paused[0] = '\0';
    return LIB_STATUS_OK;
}

lib_status emulator_product_monitor_format_window_status(const char *name,
    emulator_product_help_map hotkeys, char *text, lib_size capacity)
{
    if (name == LIB_NULL || text == LIB_NULL || capacity == 0u) return LIB_STATUS_INVALID_ARGUMENT;
    assert(hotkeys.count == 7u && hotkeys.rows != LIB_NULL);
    text[0] = '\0';
    return LIB_STATUS_OK;
}

void emulator_product_monitor_provider_open(void *context,
    emulator_session_command_result *out_result)
{
    (void)context;
    if (out_result != LIB_NULL) *out_result = (emulator_session_command_result){0};
}

void emulator_product_monitor_provider_reject_line(void *context,
    emulator_session_command_result *out_result)
{
    (void)context;
    if (out_result != LIB_NULL) *out_result = (emulator_session_command_result){0};
}

void emulator_product_monitor_provider_submit_line(void *context,
    emulator_session_machine_state state, const char *line,
    emulator_session_command_result *out_result)
{
    (void)context;
    (void)state;
    (void)line;
    if (out_result != LIB_NULL) *out_result = (emulator_session_command_result){0};
}

void emulator_product_monitor_provider_note_runtime(void *context,
    emulator_session_machine_state prior, emulator_session_machine_state completed,
    emulator_session_command_result *out_result)
{
    (void)context;
    (void)prior;
    (void)completed;
    if (out_result != LIB_NULL) *out_result = (emulator_session_command_result){0};
}

void kvm_hotkey_registry_initialize(kvm_hotkey_registry *registry)
{
    assert(registry != LIB_NULL);
    *registry = (kvm_hotkey_registry){0};
}

lib_status kvm_hotkey_registry_register(kvm_hotkey_registry *registry,
    base_key key, lib_u8 modifiers, const char *identifier)
{
    if (registry == LIB_NULL || identifier == LIB_NULL || key != BASE_KEY_ESCAPE ||
        modifiers != 0u || lib_text_compare(identifier, "pause-toggle") != 0)
        return LIB_STATUS_INVALID_ARGUMENT;
    registry->count = 1u;
    return LIB_STATUS_OK;
}

lib_bool emulator_session_request_pause_toggle(emulator_session_machine_state state,
    emulator_session_request *out_request)
{
    if (out_request == LIB_NULL) return LIB_FALSE;
    *out_request = state == EMULATOR_SESSION_MACHINE_PAUSED ?
        EMULATOR_SESSION_REQUEST_RESUME : EMULATOR_SESSION_REQUEST_PAUSE;
    return state == EMULATOR_SESSION_MACHINE_RUNNING || state == EMULATOR_SESSION_MACHINE_PAUSED;
}

lib_bool emulator_product_monitor_request_lifecycle(
    const emulator_product_monitor_provider *provider,
    emulator_product_monitor_command command, emulator_session_machine_state state,
    emulator_session_command_result *out_result)
{
    if (provider == LIB_NULL || out_result == LIB_NULL) return LIB_FALSE;
    assert((state == EMULATOR_SESSION_MACHINE_RUNNING &&
            command == EMULATOR_PRODUCT_MONITOR_COMMAND_PAUSE) ||
        (state == EMULATOR_SESSION_MACHINE_PAUSED &&
            command == EMULATOR_PRODUCT_MONITOR_COMMAND_RESUME));
    ++hotkey_count;
    out_result->request = command == EMULATOR_PRODUCT_MONITOR_COMMAND_PAUSE ?
        EMULATOR_SESSION_REQUEST_PAUSE : EMULATOR_SESSION_REQUEST_RESUME;
    return LIB_TRUE;
}

lib_i32 emulator_product_run(const emulator_product_definition *definition)
{
    emulator_session_options control = {0};
    emulator_ui_options ui = {0};

    hotkey_count = 0;
    assert(definition != LIB_NULL);
    assert(lib_text_compare(definition->banner, MYRAIDENII_PRODUCT_BANNER) == 0);
    assert(definition->machine.machine != LIB_NULL && definition->machine.bind != LIB_NULL &&
        definition->machine.destroy != LIB_NULL);
    assert(definition->machine.bind(definition->machine.machine,
        (emulator_machine *)(lib_uptr)5u) == LIB_STATUS_OK);
    ++bind_count;
    if (definition->configure_control(definition->context,
            (emulator_machine *)(lib_uptr)5u, &control) != LIB_STATUS_OK) {
        assert(definition->machine.bind(definition->machine.machine, LIB_NULL) == LIB_STATUS_OK);
        ++bind_count;
        assert(definition->machine.destroy(definition->machine.machine) == LIB_STATUS_OK);
        return 1;
    }
    assert(control.display == EMULATOR_SESSION_DISPLAY_WINDOW &&
        control.command.open == emulator_product_monitor_provider_open &&
        control.command.submit_line == emulator_product_monitor_provider_submit_line &&
        control.command.handle_hotkey != LIB_NULL);
    assert(control.command.handle_hotkey(control.command.context,
        EMULATOR_SESSION_MACHINE_RUNNING, (const lib_u8 *)"pause-toggle",
        &(emulator_session_command_result){0}) == LIB_TRUE && hotkey_count == 1);
    assert(definition->configure_ui(definition->context, &ui) == LIB_STATUS_OK &&
        ui.hotkeys.count == 1u);
    assert(definition->machine.bind(definition->machine.machine, LIB_NULL) == LIB_STATUS_OK);
    ++bind_count;
    assert(definition->machine.destroy(definition->machine.machine) == LIB_STATUS_OK);
    return 0;
}

static myraidenii_product_config composition_config(void)
{
    myraidenii_product_config config = {0};

    lib_memory_copy(config.ini_directory, "C:\\raiden", sizeof("C:\\raiden"));
    for (lib_size index = 0u; index < MYRAIDENII_MACHINE_IMAGE_COUNT; ++index)
        lib_c_snprintf((char *)config.image_paths[index],
            sizeof(config.image_paths[index]), "image%u.bin", (unsigned int)index);
    config.display = EMULATOR_SESSION_DISPLAY_WINDOW;
    config.console_control = LIB_TRUE;
    return config;
}

int main(void)
{
    myraidenii_product_config config = composition_config();

    assert(myraidenii_product_run(&config) == 0);
    assert(image_destroy_count == 1 && machine_destroy_count == 1 &&
        board_destroy_count == 1 && bind_count == 2);
    image_status = LIB_STATUS_IO_ERROR;
    assert(myraidenii_product_run(&config) == 1);
    assert(image_destroy_count == 1 && machine_destroy_count == 1 && board_destroy_count == 1);
    image_status = LIB_STATUS_OK;
    machine_status = LIB_STATUS_NO_MEMORY;
    assert(myraidenii_product_run(&config) == 1);
    assert(image_destroy_count == 2 && stream_destroy_count == 1 && board_destroy_count == 2);
    machine_status = LIB_STATUS_OK;
    stream_status = LIB_STATUS_IO_ERROR;
    assert(myraidenii_product_run(&config) == 0);
    assert(image_destroy_count == 3 && machine_destroy_count == 2 && board_destroy_count == 3);
    stream_status = LIB_STATUS_UNSUPPORTED;
    assert(myraidenii_product_run(&config) == 0);
    assert(image_destroy_count == 4 && machine_destroy_count == 3 && board_destroy_count == 4);
    stream_status = LIB_STATUS_NO_MEMORY;
    assert(myraidenii_product_run(&config) == 1);
    assert(image_destroy_count == 5 && machine_destroy_count == 3 && board_destroy_count == 5);
    stream_status = LIB_STATUS_OK;
    driver_status = LIB_STATUS_INTERNAL_ERROR;
    assert(myraidenii_product_run(&config) == 1);
    assert(image_destroy_count == 6 && machine_destroy_count == 4 && board_destroy_count == 6);
    driver_status = LIB_STATUS_OK;
    config.display = EMULATOR_SESSION_DISPLAY_CONSOLE;
    assert(myraidenii_product_run(&config) == 1);
    assert(image_destroy_count == 7 && machine_destroy_count == 5 && board_destroy_count == 7);
    return 0;
}
