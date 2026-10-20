#include "myraidenii/core/sprite_buffer_interface.h"

static lib_i32 sprite_buffer_latching(void)
{
    myraidenii_core_sprite_buffer *buffer = LIB_NULL;
    const lib_u8 first[] = {0x11u,0x22u,0x33u,0x44u};
    const lib_u8 second[] = {0xaau,0xbbu};
    lib_u8 actual[sizeof(first)] = {0};
    lib_i32 failed = 1;

    if (myraidenii_core_sprite_buffer_create(&buffer) != LIB_STATUS_OK || buffer == LIB_NULL ||
        myraidenii_core_sprite_buffer_write(buffer, 0x12u, first, sizeof(first)) !=
            LIB_STATUS_OK ||
        myraidenii_core_sprite_buffer_latch(buffer) != LIB_STATUS_OK ||
        myraidenii_core_sprite_buffer_write(buffer, 0x12u, second, sizeof(second)) !=
            LIB_STATUS_OK ||
        myraidenii_core_sprite_buffer_read(buffer, 0x12u, actual, sizeof(first)) !=
            LIB_STATUS_OK || actual[0] != second[0] || actual[1] != second[1] ||
        actual[2] != first[2] || actual[3] != first[3] ||
        myraidenii_core_sprite_buffer_latched_copy(buffer, 0x12u, actual,
            sizeof(actual)) != LIB_STATUS_OK ||
        lib_memory_compare(actual, first, sizeof(first)) != 0) {
        goto done;
    }
    myraidenii_core_sprite_buffer_reset(buffer);
    if (myraidenii_core_sprite_buffer_read(buffer, 0x12u, actual, sizeof(actual)) !=
            LIB_STATUS_OK || actual[0] != 0u || actual[1] != 0u || actual[2] != 0u ||
        actual[3] != 0u || myraidenii_core_sprite_buffer_latched_copy(buffer, 0x12u,
            actual, sizeof(actual)) != LIB_STATUS_OK || actual[0] != 0u ||
        actual[1] != 0u || actual[2] != 0u || actual[3] != 0u) {
        goto done;
    }
    failed = 0;
done:
    myraidenii_core_sprite_buffer_destroy(buffer);
    return failed;
}

static lib_i32 sprite_buffer_invalid_arguments(void)
{
    myraidenii_core_sprite_buffer *buffer = LIB_NULL;
    lib_u8 value = 0u;
    lib_i32 failed = 1;

    if (myraidenii_core_sprite_buffer_create(LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT ||
        myraidenii_core_sprite_buffer_create(&buffer) != LIB_STATUS_OK ||
        myraidenii_core_sprite_buffer_read(LIB_NULL, 0u, &value, 1u) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        myraidenii_core_sprite_buffer_read(buffer, MYRAIDENII_CORE_SPRITE_BUFFER_BYTES,
            &value, 1u) != LIB_STATUS_INVALID_ARGUMENT ||
        myraidenii_core_sprite_buffer_write(buffer, 0u, LIB_NULL, 1u) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        myraidenii_core_sprite_buffer_latched_copy(buffer, 0u, &value, 0u) !=
            LIB_STATUS_INVALID_ARGUMENT) {
        goto done;
    }
    failed = 0;
done:
    myraidenii_core_sprite_buffer_destroy(buffer);
    return failed;
}

lib_i32 main(void)
{
    return sprite_buffer_latching() || sprite_buffer_invalid_arguments();
}
