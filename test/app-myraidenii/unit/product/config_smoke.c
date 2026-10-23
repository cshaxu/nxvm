#include <assert.h>

#include "lib/base/process_interface.h"
#include "lib/storage/file_interface.h"
#include "app-myraidenii/product/config_interface.h"

static const char *myraidenii_test_directory;
static const char *myraidenii_test_expected_path;
static lib_status myraidenii_test_read_status = LIB_STATUS_OK;
static lib_bool myraidenii_test_invalid_text = LIB_FALSE;

lib_status base_process_executable_directory(char *output, lib_size capacity)
{
    lib_size length = lib_text_length(myraidenii_test_directory);

    assert(length < capacity);
    lib_memory_copy(output, myraidenii_test_directory, length + 1u);
    return LIB_STATUS_OK;
}

lib_status lib_storage_file_read_owned(const char *path, lib_size maximum,
    void **out_bytes, lib_size *out_byte_count)
{
    static const char text[] =
        "program_even=even.bin\nprogram_odd=odd.bin\nsound_program=sound.bin\n"
        "characters=chars.bin\nbackground_1=bg1.bin\nbackground_2=bg2.bin\n"
        "sprites_1=spr1.bin\nsprites_2=spr2.bin\nsprites_3=spr3.bin\n"
        "sprites_4=spr4.bin\nsamples_1=pcm1.bin\nsamples_2=pcm2.bin\n";

    assert(lib_text_compare(path, myraidenii_test_expected_path) == 0);
    if (myraidenii_test_read_status != LIB_STATUS_OK) return myraidenii_test_read_status;
    assert(maximum >= sizeof(text));
    *out_bytes = lib_allocate(sizeof(text));
    assert(*out_bytes != LIB_NULL);
    lib_memory_copy(*out_bytes, text, sizeof(text));
    *out_byte_count = myraidenii_test_invalid_text ? 1u : sizeof(text) - 1u;
    return LIB_STATUS_OK;
}

int main(void)
{
    static const lib_u8 valid[] =
        "; selected external inputs\r\nprogram_even = \"even.bin\"\r\n"
        "program_odd=odd.bin\r\nsound_program=sound.bin\r\ncharacters=chars.bin\r\n"
        "background_1=bg1.bin\r\nbackground_2=bg2.bin\r\nsprites_1=spr1.bin\r\n"
        "sprites_2=spr2.bin\r\nsprites_3=spr3.bin\r\nsprites_4=spr4.bin\r\n"
        "samples_1=pcm1.bin\r\nsamples_2=pcm2.bin\r\ndisplay=console\r\n"
        "console_control=0\r\n";
    static const lib_u8 missing_image[] =
        "program_even=even.bin\nprogram_odd=odd.bin\nsound_program=sound.bin\n"
        "characters=chars.bin\nbackground_1=bg1.bin\nbackground_2=bg2.bin\n"
        "sprites_1=spr1.bin\nsprites_2=spr2.bin\nsprites_3=spr3.bin\n"
        "sprites_4=spr4.bin\nsamples_1=pcm1.bin\n";
    static const lib_u8 duplicate_image[] =
        "program_even=even.bin\nprogram_even=again.bin\n";
    static const lib_u8 invalid_display[] = "display=raster\n";
    static const lib_u8 duplicate_display[] = "display=window\ndisplay=console\n";
    myraidenii_product_config config;
    char *argv[] = {"myraidenii"};
    static const char *const directories[] = {"C:\\", "/", "\\\\server\\share\\", "C:\\apps"};
    static const char *const expected[] = {"C:\\myraidenii.ini", "/myraidenii.ini",
        "\\\\server\\share\\myraidenii.ini", "C:\\apps\\myraidenii.ini"};

    assert(myraidenii_product_config_load_text(valid, sizeof(valid) - 1u, &config));
    assert(config.display == EMULATOR_SESSION_DISPLAY_CONSOLE &&
        config.console_control == LIB_FALSE);
    assert(lib_text_compare((const char *)config.image_paths[
        MYRAIDENII_MACHINE_IMAGE_SPRITES_4], "spr4.bin") == 0);
    assert(!myraidenii_product_config_load_text(missing_image, sizeof(missing_image) - 1u,
        &config));
    assert(!myraidenii_product_config_load_text(duplicate_image,
        sizeof(duplicate_image) - 1u, &config));
    assert(!myraidenii_product_config_load_text(invalid_display,
        sizeof(invalid_display) - 1u, &config));
    assert(!myraidenii_product_config_load_text(duplicate_display,
        sizeof(duplicate_display) - 1u, &config));
    for (lib_size index = 0u; index < sizeof(directories) / sizeof(directories[0]); ++index) {
        myraidenii_test_directory = directories[index];
        myraidenii_test_expected_path = expected[index];
        assert(myraidenii_product_config_load(1, argv, &config) == LIB_STATUS_OK);
        assert(lib_text_compare((const char *)config.ini_directory, directories[index]) == 0);
    }
    myraidenii_test_read_status = LIB_STATUS_IO_ERROR;
    assert(myraidenii_product_config_load(1, argv, &config) == LIB_STATUS_IO_ERROR);
    myraidenii_test_read_status = LIB_STATUS_OK;
    myraidenii_test_invalid_text = LIB_TRUE;
    assert(myraidenii_product_config_load(1, argv, &config) == LIB_STATUS_INVALID_ARGUMENT);
    assert(myraidenii_product_config_load(2, argv, &config) == LIB_STATUS_INVALID_ARGUMENT);
    return 0;
}
