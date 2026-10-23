#include <assert.h>

#include "lib/storage/file_interface.h"
#include "app-myraidenii/machine/configuration_interface.h"

static lib_size myraidenii_test_bytes;
static lib_u8 myraidenii_test_value;

lib_status lib_storage_file_read_owned(const char *path, lib_size maximum,
    void **out_bytes, lib_size *out_byte_count)
{
    assert(lib_text_compare(path, "C:\\images\\first.bin") == 0);
    assert(maximum == 524288u);
    *out_bytes = lib_allocate(myraidenii_test_bytes == 0u ? 1u : myraidenii_test_bytes);
    assert(*out_bytes != LIB_NULL);
    lib_memory_set(*out_bytes, myraidenii_test_value, myraidenii_test_bytes);
    *out_byte_count = myraidenii_test_bytes;
    return LIB_STATUS_OK;
}

int main(void)
{
    const char *paths[MYRAIDENII_MACHINE_IMAGE_COUNT] = {"first.bin"};
    myraidenii_machine_images *images = (myraidenii_machine_images *)(lib_uptr)1u;

    myraidenii_test_bytes = 1u;
    assert(myraidenii_machine_images_load("C:\\images", paths, &images) ==
        LIB_STATUS_INVALID_ARGUMENT);
    assert(images == LIB_NULL);
    images = (myraidenii_machine_images *)(lib_uptr)1u;
    myraidenii_test_bytes = 524288u;
    myraidenii_test_value = 0u;
    assert(myraidenii_machine_images_load("C:\\images", paths, &images) ==
        LIB_STATUS_INVALID_ARGUMENT);
    assert(images == LIB_NULL);
    assert(myraidenii_machine_images_load(LIB_NULL, paths, &images) ==
        LIB_STATUS_INVALID_ARGUMENT);
    return 0;
}
