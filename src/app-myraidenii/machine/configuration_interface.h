#ifndef MYRAIDENII_MACHINE_CONFIGURATION_INTERFACE_H
#define MYRAIDENII_MACHINE_CONFIGURATION_INTERFACE_H

#include "lib/types/types_interface.h"
#include "app-myraidenii/core/board_interface.h"

/* Ordered inputs consumed by the one Machine-owned Board materialization
 * transaction. Product configuration maps named INI keys onto these roles;
 * Core never sees paths or external-file identity. */
typedef enum myraidenii_machine_image {
    MYRAIDENII_MACHINE_IMAGE_PROGRAM_EVEN = 0,
    MYRAIDENII_MACHINE_IMAGE_PROGRAM_ODD,
    MYRAIDENII_MACHINE_IMAGE_SOUND_PROGRAM,
    MYRAIDENII_MACHINE_IMAGE_CHARACTERS,
    MYRAIDENII_MACHINE_IMAGE_BACKGROUND_1,
    MYRAIDENII_MACHINE_IMAGE_BACKGROUND_2,
    MYRAIDENII_MACHINE_IMAGE_SPRITES_1,
    MYRAIDENII_MACHINE_IMAGE_SPRITES_2,
    MYRAIDENII_MACHINE_IMAGE_SPRITES_3,
    MYRAIDENII_MACHINE_IMAGE_SPRITES_4,
    MYRAIDENII_MACHINE_IMAGE_SAMPLES_1,
    MYRAIDENII_MACHINE_IMAGE_SAMPLES_2,
    MYRAIDENII_MACHINE_IMAGE_COUNT
} myraidenii_machine_image;

typedef struct myraidenii_machine_images myraidenii_machine_images;

/* Paths are Product configuration input. The Machine resolves relative paths
 * against base_directory and owns every successfully read candidate byte span. */
lib_status myraidenii_machine_images_load(const char *base_directory,
    const char *const image_paths[MYRAIDENII_MACHINE_IMAGE_COUNT],
    myraidenii_machine_images **out_images);
void myraidenii_machine_images_destroy(myraidenii_machine_images *images);
lib_status myraidenii_machine_images_board_config(const myraidenii_machine_images *images,
    myraidenii_core_board_config *out_config);

#endif
