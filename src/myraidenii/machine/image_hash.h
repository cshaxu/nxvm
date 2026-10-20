#ifndef MYRAIDENII_MACHINE_IMAGE_HASH_H
#define MYRAIDENII_MACHINE_IMAGE_HASH_H

#include "lib/types/types_interface.h"

enum { MYRAIDENII_MACHINE_IMAGE_SHA256_BYTES = 32u };

void myraidenii_machine_image_sha256(const lib_u8 *bytes, lib_size byte_count,
    lib_u8 out_digest[MYRAIDENII_MACHINE_IMAGE_SHA256_BYTES]);

#endif
