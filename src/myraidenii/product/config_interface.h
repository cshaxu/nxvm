#ifndef MYRAIDENII_PRODUCT_CONFIG_INTERFACE_H
#define MYRAIDENII_PRODUCT_CONFIG_INTERFACE_H

#include "emulator/session/session_interface.h"
#include "lib/types/types_interface.h"
#include "myraidenii/machine/configuration_interface.h"

enum {
    MYRAIDENII_PRODUCT_PATH_CAPACITY = 1024u
};

typedef struct myraidenii_product_config {
    /* Runtime image paths are interpreted only against this one copied INI
     * directory.  It is established by the adjacent-file loader, not guessed
     * from the executable name or process working directory. */
    lib_u8 ini_directory[MYRAIDENII_PRODUCT_PATH_CAPACITY];
    lib_u8 image_paths[MYRAIDENII_MACHINE_IMAGE_COUNT][MYRAIDENII_PRODUCT_PATH_CAPACITY];
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
