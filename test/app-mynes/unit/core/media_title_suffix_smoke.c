#include <assert.h>
#include <string.h>

#include "core/driver.h"
#include "core/machine.h"
#include "lib/storage/file_interface.h"

#define TITLE_IMAGE_PAYLOAD (16u + 131072u + 131072u)
#define TITLE_IMAGE_BYTES (TITLE_IMAGE_PAYLOAD + 128u)

static lib_u8 image[TITLE_IMAGE_BYTES];
static void write_image(void)
{
    lib_storage_file_writer *writer = LIB_NULL;

    assert(lib_storage_file_writer_open("title.nes", LIB_STORAGE_FILE_WRITER_TRUNCATE,
        &writer) == LIB_STATUS_OK);
    assert(lib_storage_file_writer_write(writer, image, sizeof(image)) == LIB_STATUS_OK);
    assert(lib_storage_file_writer_close(writer) == LIB_STATUS_OK);
}

static void write_truncated_image(void)
{
    lib_storage_file_writer *writer = LIB_NULL;

    assert(lib_storage_file_writer_open("title.nes", LIB_STORAGE_FILE_WRITER_TRUNCATE,
        &writer) == LIB_STATUS_OK);
    assert(lib_storage_file_writer_write(writer, image, TITLE_IMAGE_PAYLOAD + 1u) ==
        LIB_STATUS_OK);
    assert(lib_storage_file_writer_close(writer) == LIB_STATUS_OK);
}

static void make_image(void)
{
    memset(image, 0, sizeof(image));
    image[0] = 'N'; image[1] = 'E'; image[2] = 'S'; image[3] = 0x1au;
    image[4] = 8u; image[5] = 16u; image[6] = 0x41u;
    image[16u + 131072u - 4u] = 0u;
    image[16u + 131072u - 3u] = 0x80u;
    lib_memory_copy(image + TITLE_IMAGE_PAYLOAD, "Super Mario Bros II\r\n\x1a", 22u);
}

int main(void)
{
    core_driver *driver = LIB_NULL;
    core_machine *accepted;

    make_image();
    write_image();
    assert(core_driver_create(&driver, &(core_driver_options) { 0 }) == LIB_STATUS_OK);
    assert(core_driver_set_media(driver, "title.nes", LIB_STORAGE_MEDIUM_READONLY));
    accepted = driver->machine;
    assert(accepted != LIB_NULL && accepted->cartridge->mapper == 4u);
    write_truncated_image();
    assert(!core_driver_set_media(driver, "title.nes", LIB_STORAGE_MEDIUM_READONLY));
    assert(driver->machine == accepted);
    assert(core_driver_destroy(driver) == LIB_STATUS_OK);
    return 0;
}
