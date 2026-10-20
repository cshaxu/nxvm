#include "myraidenii/product/config_interface.h"

#include "lib/base/process_interface.h"
#include "lib/storage/file_interface.h"

typedef struct myraidenii_product_image_key {
    const char *name;
    myraidenii_machine_image image;
} myraidenii_product_image_key;

static const myraidenii_product_image_key myraidenii_product_image_keys[] = {
    {"program_even", MYRAIDENII_MACHINE_IMAGE_PROGRAM_EVEN},
    {"program_odd", MYRAIDENII_MACHINE_IMAGE_PROGRAM_ODD},
    {"sound_program", MYRAIDENII_MACHINE_IMAGE_SOUND_PROGRAM},
    {"characters", MYRAIDENII_MACHINE_IMAGE_CHARACTERS},
    {"background_1", MYRAIDENII_MACHINE_IMAGE_BACKGROUND_1},
    {"background_2", MYRAIDENII_MACHINE_IMAGE_BACKGROUND_2},
    {"sprites_1", MYRAIDENII_MACHINE_IMAGE_SPRITES_1},
    {"sprites_2", MYRAIDENII_MACHINE_IMAGE_SPRITES_2},
    {"sprites_3", MYRAIDENII_MACHINE_IMAGE_SPRITES_3},
    {"sprites_4", MYRAIDENII_MACHINE_IMAGE_SPRITES_4},
    {"samples_1", MYRAIDENII_MACHINE_IMAGE_SAMPLES_1},
    {"samples_2", MYRAIDENII_MACHINE_IMAGE_SAMPLES_2}
};

static lib_bool myraidenii_product_config_space(lib_u8 value)
{
    return value == ' ' || value == '\t' || value == '\r';
}

static const lib_u8 *myraidenii_product_config_trim_left(const lib_u8 *start,
    const lib_u8 *end)
{
    while (start != end && myraidenii_product_config_space(*start)) ++start;
    return start;
}

static const lib_u8 *myraidenii_product_config_trim_right(const lib_u8 *start,
    const lib_u8 *end)
{
    while (end != start && myraidenii_product_config_space(end[-1])) --end;
    return end;
}

static lib_bool myraidenii_product_config_equals(const lib_u8 *start, const lib_u8 *end,
    const char *literal)
{
    while (start != end && *literal != '\0') {
        if (*start++ != (lib_u8)*literal++) return LIB_FALSE;
    }
    return start == end && *literal == '\0';
}

static lib_bool myraidenii_product_config_copy(lib_u8 *target, lib_size capacity,
    const lib_u8 *start, const lib_u8 *end)
{
    lib_size length = (lib_size)(end - start);

    if (length == 0u || length >= capacity) return LIB_FALSE;
    lib_memory_copy(target, start, length);
    target[length] = '\0';
    return LIB_TRUE;
}

static lib_bool myraidenii_product_config_image(const lib_u8 *start, const lib_u8 *end,
    myraidenii_machine_image *out_image)
{
    for (lib_size index = 0u; index < sizeof(myraidenii_product_image_keys) /
            sizeof(myraidenii_product_image_keys[0]); ++index) {
        if (myraidenii_product_config_equals(start, end,
                myraidenii_product_image_keys[index].name)) {
            *out_image = myraidenii_product_image_keys[index].image;
            return LIB_TRUE;
        }
    }
    return LIB_FALSE;
}

static lib_bool myraidenii_product_config_complete(const myraidenii_product_config *config)
{
    for (lib_size image = 0u; image < MYRAIDENII_MACHINE_IMAGE_COUNT; ++image) {
        if (config->image_paths[image][0] == '\0') return LIB_FALSE;
    }
    return LIB_TRUE;
}

lib_bool myraidenii_product_config_load_text(const lib_u8 *text, lib_size text_length,
    myraidenii_product_config *out_config)
{
    const lib_u8 *cursor;
    const lib_u8 *limit;
    lib_bool display_seen = LIB_FALSE;
    lib_bool console_control_seen = LIB_FALSE;
    myraidenii_product_config candidate = {
        .display = EMULATOR_SESSION_DISPLAY_WINDOW,
        .console_control = LIB_TRUE};

    if (text == LIB_NULL || out_config == LIB_NULL) return LIB_FALSE;
    cursor = text;
    limit = text + text_length;
    while (cursor != limit) {
        const lib_u8 *line_end = cursor;
        const lib_u8 *comment;
        const lib_u8 *key_start;
        const lib_u8 *key_end;
        const lib_u8 *value_start;
        const lib_u8 *value_end;
        const lib_u8 *equals;
        const lib_u8 *next_line;
        myraidenii_machine_image image;

        while (line_end != limit && *line_end != '\n') ++line_end;
        next_line = line_end;
        comment = cursor;
        while (comment != line_end && *comment != ';' && *comment != '#') ++comment;
        line_end = comment;
        key_start = myraidenii_product_config_trim_left(cursor, line_end);
        key_end = myraidenii_product_config_trim_right(key_start, line_end);
        if (key_start != key_end) {
            equals = key_start;
            while (equals != key_end && *equals != '=') ++equals;
            if (equals == key_end) return LIB_FALSE;
            value_start = myraidenii_product_config_trim_left(equals + 1u, key_end);
            value_end = myraidenii_product_config_trim_right(value_start, key_end);
            key_end = myraidenii_product_config_trim_right(key_start, equals);
            if (value_end > value_start + 1u && *value_start == '"' &&
                value_end[-1] == '"') {
                ++value_start;
                --value_end;
            }
            if (myraidenii_product_config_image(key_start, key_end, &image)) {
                if (candidate.image_paths[image][0] != '\0' ||
                    !myraidenii_product_config_copy(candidate.image_paths[image],
                        sizeof(candidate.image_paths[image]), value_start, value_end))
                    return LIB_FALSE;
            } else if (myraidenii_product_config_equals(key_start, key_end, "display")) {
                if (display_seen) return LIB_FALSE;
                if (myraidenii_product_config_equals(value_start, value_end, "window"))
                    candidate.display = EMULATOR_SESSION_DISPLAY_WINDOW;
                else if (myraidenii_product_config_equals(value_start, value_end, "console"))
                    candidate.display = EMULATOR_SESSION_DISPLAY_CONSOLE;
                else return LIB_FALSE;
                display_seen = LIB_TRUE;
            } else if (myraidenii_product_config_equals(key_start, key_end,
                    "console_control")) {
                if (console_control_seen) return LIB_FALSE;
                if (value_end == value_start + 1u && value_start[0] == '0')
                    candidate.console_control = LIB_FALSE;
                else if (value_end == value_start + 1u && value_start[0] == '1')
                    candidate.console_control = LIB_TRUE;
                else return LIB_FALSE;
                console_control_seen = LIB_TRUE;
            } else return LIB_FALSE;
        }
        cursor = next_line == limit ? limit : next_line + 1u;
    }
    if (!myraidenii_product_config_complete(&candidate)) return LIB_FALSE;
    *out_config = candidate;
    return LIB_TRUE;
}

static lib_bool myraidenii_product_config_path(lib_u8 *path)
{
    const char *name = "myraidenii.ini";
    lib_size length = 0u;

    if (base_process_executable_directory((char *)path,
            MYRAIDENII_PRODUCT_PATH_CAPACITY) != LIB_STATUS_OK)
        return LIB_FALSE;
    while (path[length] != '\0') ++length;
    if (length != 0u && path[length - 1u] != '/' && path[length - 1u] != '\\') {
        if (length + 1u + lib_text_length(name) + 1u > MYRAIDENII_PRODUCT_PATH_CAPACITY)
            return LIB_FALSE;
        path[length++] = '\\';
    }
    if (length + lib_text_length(name) + 1u > MYRAIDENII_PRODUCT_PATH_CAPACITY)
        return LIB_FALSE;
    lib_memory_copy(path + length, name, lib_text_length(name) + 1u);
    return LIB_TRUE;
}

lib_bool myraidenii_product_config_load(lib_i32 argc, char **argv,
    myraidenii_product_config *out_config)
{
    lib_u8 path[MYRAIDENII_PRODUCT_PATH_CAPACITY];
    void *bytes = LIB_NULL;
    lib_size byte_count;
    lib_status status;
    lib_bool result;

    if (out_config == LIB_NULL || argc != 1 || argv == LIB_NULL ||
        !myraidenii_product_config_path(path)) return LIB_FALSE;
    status = lib_storage_file_read_owned((const char *)path, 24u * 1024u, &bytes,
        &byte_count);
    if (status != LIB_STATUS_OK) return LIB_FALSE;
    result = myraidenii_product_config_load_text((const lib_u8 *)bytes, byte_count,
        out_config);
    lib_release(bytes);
    return result;
}
