#include <assert.h>

#include "product/config.h"

int main(void)
{
    app_startup_config config;
    const lib_u8 valid[] = "; startup configuration\r\nrom = fixture.nes\r\ndisplay = console\r\n";
    const lib_u8 window[] = "rom = fixture.nes\ndisplay = window\n";
    const lib_u8 default_window[] = "rom = fixture.nes\n";
    const lib_u8 invalid_display[] = "display = raster\n";
    const lib_u8 retired_video[] = "video = window\n";
    const lib_u8 invalid_key[] = "set = anything\n";
    const lib_u8 quoted_with_comment[] =
        "ordinary heading\nrom = \"fixture game.nes\" ; local path\n"
        "display = console # presentation\n";

    assert(app_config_load_text(valid, sizeof(valid) - 1u, &config));
    assert(lib_c_strcmp((const char *)config.rom_path, "fixture.nes") == 0);
    assert(config.text_output);
    assert(app_config_load_text(window, sizeof(window) - 1u, &config));
    assert(lib_c_strcmp((const char *)config.rom_path, "fixture.nes") == 0 && !config.text_output);
    assert(app_config_load_text(default_window, sizeof(default_window) - 1u, &config));
    assert(lib_c_strcmp((const char *)config.rom_path, "fixture.nes") == 0 && !config.text_output);
    assert(!app_config_load_text(invalid_display, sizeof(invalid_display) - 1u, &config));
    assert(!app_config_load_text(retired_video, sizeof(retired_video) - 1u, &config));
    assert(lib_c_strcmp((const char *)config.rom_path, "fixture.nes") == 0 && !config.text_output);
    assert(!app_config_load_text(invalid_key, sizeof(invalid_key) - 1u, &config));
    assert(lib_c_strcmp((const char *)config.rom_path, "fixture.nes") == 0 && !config.text_output);
    assert(app_config_load_text(quoted_with_comment, sizeof(quoted_with_comment) - 1u,
        &config));
    assert(lib_c_strcmp((const char *)config.rom_path, "fixture game.nes") == 0 && config.text_output);
    return 0;
}
