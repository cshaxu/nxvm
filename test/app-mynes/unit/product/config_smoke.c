#include <assert.h>

#include "product/config.h"
#include "lib/base/process_interface.h"
#include "lib/storage/file_interface.h"

static const char *config_directory;
static const char *expected_config_path;

lib_status base_process_executable_directory(char *output, lib_size capacity)
{
    lib_size length = lib_text_length(config_directory);
    assert(length < capacity);
    lib_memory_copy(output, config_directory, length + 1u);
    return LIB_STATUS_OK;
}

lib_status lib_storage_file_read_owned(const char *path, lib_size limit,
    void **bytes, lib_size *count)
{
    static const char config_text[] = "rom=fixture.nes\n";
    assert(lib_text_compare(path, expected_config_path) == 0);
    assert(limit >= sizeof(config_text));
    *count = sizeof(config_text) - 1u;
    *bytes = lib_allocate(sizeof(config_text));
    assert(*bytes != LIB_NULL);
    lib_memory_copy(*bytes, config_text, sizeof(config_text));
    return LIB_STATUS_OK;
}

int main(void)
{
    app_startup_config config;
    const lib_u8 valid[] = "; startup configuration\r\nrom = fixture.nes\r\ndisplay = console\r\nconsole_control = 0\r\n";
    const lib_u8 window[] = "rom = fixture.nes\ndisplay = window\nconsole_control = 1\n";
    const lib_u8 default_window[] = "rom = fixture.nes\n";
    const lib_u8 invalid_display[] = "display = raster\n";
    const lib_u8 retired_video[] = "video = window\n";
    const lib_u8 invalid_key[] = "set = anything\n";
    const lib_u8 invalid_console_control[] = "console_control = yes\n";
    const lib_u8 quoted_with_comment[] =
        "ordinary heading\nrom = \"fixture game.nes\" ; local path\n"
        "display = console # presentation\n";

    assert(app_config_load_text(valid, sizeof(valid) - 1u, &config));
    assert(lib_text_compare((const char *)config.rom_path, "fixture.nes") == 0);
    assert(config.display == EMULATOR_SESSION_DISPLAY_CONSOLE && !config.console_control);
    assert(app_config_load_text(window, sizeof(window) - 1u, &config));
    assert(lib_text_compare((const char *)config.rom_path, "fixture.nes") == 0 &&
        config.display == EMULATOR_SESSION_DISPLAY_WINDOW && config.console_control);
    assert(app_config_load_text(default_window, sizeof(default_window) - 1u, &config));
    assert(lib_text_compare((const char *)config.rom_path, "fixture.nes") == 0 &&
        config.display == EMULATOR_SESSION_DISPLAY_WINDOW && config.console_control);
    assert(!app_config_load_text(invalid_display, sizeof(invalid_display) - 1u, &config));
    assert(!app_config_load_text(retired_video, sizeof(retired_video) - 1u, &config));
    assert(lib_text_compare((const char *)config.rom_path, "fixture.nes") == 0 &&
        config.display == EMULATOR_SESSION_DISPLAY_WINDOW);
    assert(!app_config_load_text(invalid_key, sizeof(invalid_key) - 1u, &config));
    assert(lib_text_compare((const char *)config.rom_path, "fixture.nes") == 0 &&
        config.display == EMULATOR_SESSION_DISPLAY_WINDOW);
    assert(!app_config_load_text(invalid_console_control, sizeof(invalid_console_control) - 1u,
        &config));
    assert(app_config_load_text(quoted_with_comment, sizeof(quoted_with_comment) - 1u,
        &config));
    assert(lib_text_compare((const char *)config.rom_path, "fixture game.nes") == 0 &&
        config.display == EMULATOR_SESSION_DISPLAY_CONSOLE);
    {
        static const char *const directories[] = {"C:\\", "/", "\\\\server\\share\\", "C:\\apps"};
        static const char *const expected[] = {"C:\\mynes.ini", "/mynes.ini",
            "\\\\server\\share\\mynes.ini", "C:\\apps\\mynes.ini"};
        char *argv[] = {"mynes"};
        for (lib_size index = 0u; index < sizeof(directories) / sizeof(directories[0]); ++index) {
            config_directory = directories[index];
            expected_config_path = expected[index];
            assert(app_config_parse(1, argv, &config));
        }
    }
    return 0;
}
