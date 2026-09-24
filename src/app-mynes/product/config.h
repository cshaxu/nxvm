#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "lib/types/types_interface.h"

#define APP_CONFIG_PATH_CAPACITY 1024u

typedef struct app_startup_config {
    lib_u8 rom_path[APP_CONFIG_PATH_CAPACITY];
    lib_bool text_output;
} app_startup_config;

/* Parses the contents of MyNes's fixed startup file.  This is public for
 * focused App tests; normal startup obtains the bytes from mynes.ini. */
lib_bool app_config_load_text(const lib_u8 *text, lib_size text_length,
    app_startup_config *out_config);
lib_bool app_config_parse(lib_i32 argc, char **argv, app_startup_config *out_config);

#endif
