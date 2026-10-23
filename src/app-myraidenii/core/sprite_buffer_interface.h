#ifndef MYRAIDENII_CORE_SPRITE_BUFFER_INTERFACE_H
#define MYRAIDENII_CORE_SPRITE_BUFFER_INTERFACE_H

#include "lib/types/types_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct myraidenii_core_sprite_buffer myraidenii_core_sprite_buffer;

enum {
    MYRAIDENII_CORE_SPRITE_BUFFER_BYTES = 0x1000u
};

/* Owns the selected board's live object RAM and its display-latched copy.
 * The Board performs bus routing; a later Video consumer reads only the
 * latched copy. */
lib_status myraidenii_core_sprite_buffer_create(myraidenii_core_sprite_buffer **out_buffer);
void myraidenii_core_sprite_buffer_destroy(myraidenii_core_sprite_buffer *buffer);
void myraidenii_core_sprite_buffer_reset(myraidenii_core_sprite_buffer *buffer);

lib_status myraidenii_core_sprite_buffer_read(const myraidenii_core_sprite_buffer *buffer,
    lib_size offset, void *out_data, lib_size bytes);
lib_status myraidenii_core_sprite_buffer_write(myraidenii_core_sprite_buffer *buffer,
    lib_size offset, const void *data, lib_size bytes);
lib_status myraidenii_core_sprite_buffer_latch(myraidenii_core_sprite_buffer *buffer);
lib_status myraidenii_core_sprite_buffer_latched_copy(
    const myraidenii_core_sprite_buffer *buffer, lib_size offset,
    void *out_data, lib_size bytes);

#ifdef __cplusplus
}
#endif

#endif
