#include "myraidenii/core/sprite_buffer_interface.h"

struct myraidenii_core_sprite_buffer {
    lib_u8 live[MYRAIDENII_CORE_SPRITE_BUFFER_BYTES];
    lib_u8 latched[MYRAIDENII_CORE_SPRITE_BUFFER_BYTES];
};

static lib_bool myraidenii_core_sprite_buffer_range_is_valid(lib_size offset,
    lib_size bytes)
{
    return bytes != 0u && offset <= MYRAIDENII_CORE_SPRITE_BUFFER_BYTES &&
        bytes <= MYRAIDENII_CORE_SPRITE_BUFFER_BYTES - offset;
}

lib_status myraidenii_core_sprite_buffer_create(myraidenii_core_sprite_buffer **out_buffer)
{
    myraidenii_core_sprite_buffer *buffer;

    if (out_buffer == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_buffer = LIB_NULL;
    buffer = lib_allocate_zero(1u, sizeof(*buffer));
    if (buffer == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    *out_buffer = buffer;
    return LIB_STATUS_OK;
}

void myraidenii_core_sprite_buffer_destroy(myraidenii_core_sprite_buffer *buffer)
{
    lib_release(buffer);
}

void myraidenii_core_sprite_buffer_reset(myraidenii_core_sprite_buffer *buffer)
{
    if (buffer == LIB_NULL) return;
    lib_memory_set(buffer->live, 0, sizeof(buffer->live));
    lib_memory_set(buffer->latched, 0, sizeof(buffer->latched));
}

lib_status myraidenii_core_sprite_buffer_read(const myraidenii_core_sprite_buffer *buffer,
    lib_size offset, void *out_data, lib_size bytes)
{
    if (buffer == LIB_NULL || out_data == LIB_NULL ||
        !myraidenii_core_sprite_buffer_range_is_valid(offset, bytes)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    lib_memory_copy(out_data, buffer->live + offset, bytes);
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_sprite_buffer_write(myraidenii_core_sprite_buffer *buffer,
    lib_size offset, const void *data, lib_size bytes)
{
    if (buffer == LIB_NULL || data == LIB_NULL ||
        !myraidenii_core_sprite_buffer_range_is_valid(offset, bytes)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    lib_memory_copy(buffer->live + offset, data, bytes);
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_sprite_buffer_latch(myraidenii_core_sprite_buffer *buffer)
{
    if (buffer == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_copy(buffer->latched, buffer->live, sizeof(buffer->latched));
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_sprite_buffer_latched_copy(
    const myraidenii_core_sprite_buffer *buffer, lib_size offset,
    void *out_data, lib_size bytes)
{
    if (buffer == LIB_NULL || out_data == LIB_NULL ||
        !myraidenii_core_sprite_buffer_range_is_valid(offset, bytes)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    lib_memory_copy(out_data, buffer->latched + offset, bytes);
    return LIB_STATUS_OK;
}
