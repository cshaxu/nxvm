#ifndef MYRAIDENII_PRODUCT_CONFIG_INTERFACE_H
#define MYRAIDENII_PRODUCT_CONFIG_INTERFACE_H

#include "emulator/session/session_interface.h"
#include "lib/types/types_interface.h"

enum {
    MYRAIDENII_PRODUCT_PATH_CAPACITY = 1024u
};

typedef enum myraidenii_product_image {
    MYRAIDENII_PRODUCT_IMAGE_PROGRAM_EVEN = 0,
    MYRAIDENII_PRODUCT_IMAGE_PROGRAM_ODD,
    MYRAIDENII_PRODUCT_IMAGE_SOUND_PROGRAM,
    MYRAIDENII_PRODUCT_IMAGE_CHARACTERS,
    MYRAIDENII_PRODUCT_IMAGE_BACKGROUND_1,
    MYRAIDENII_PRODUCT_IMAGE_BACKGROUND_2,
    MYRAIDENII_PRODUCT_IMAGE_SPRITES_1,
    MYRAIDENII_PRODUCT_IMAGE_SPRITES_2,
    MYRAIDENII_PRODUCT_IMAGE_SPRITES_3,
    MYRAIDENII_PRODUCT_IMAGE_SPRITES_4,
    MYRAIDENII_PRODUCT_IMAGE_SAMPLES_1,
    MYRAIDENII_PRODUCT_IMAGE_SAMPLES_2,
    MYRAIDENII_PRODUCT_IMAGE_COUNT
} myraidenii_product_image;

typedef struct myraidenii_product_config {
    lib_u8 image_paths[MYRAIDENII_PRODUCT_IMAGE_COUNT][MYRAIDENII_PRODUCT_PATH_CAPACITY];
    emulator_session_display display;
    lib_bool console_control;
} myraidenii_product_config;

/* Parses MyRaidenII's fixed adjacent INI grammar. Every runtime image role is
 * required; file opening, relative-path resolution and content validation are
 * deliberately deferred to the composition transaction. */
lib_bool myraidenii_product_config_load_text(const lib_u8 *text, lib_size text_length,
    myraidenii_product_config *out_config);
lib_bool myraidenii_product_config_load(lib_i32 argc, char **argv,
    myraidenii_product_config *out_config);

#endif
