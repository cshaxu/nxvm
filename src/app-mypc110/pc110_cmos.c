#include "app-mypc110/pc110_cmos.h"
#include "lib/storage/file_interface.h"

lib_status mypc110_cmos_load(const char *path, lib_u8 *image, lib_bool *present)
{
    lib_storage_file_writer *writer = LIB_NULL;
    void *bytes = LIB_NULL;
    lib_size count = 0u;
    lib_status status;

    if (path == LIB_NULL || image == LIB_NULL || present == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *present = LIB_FALSE;
    /* Create an empty first-run file without truncating an existing image.
     * Open failures remain failures; they are not interpreted as absence. */
    status = lib_storage_file_writer_open(path, LIB_STORAGE_FILE_WRITER_APPEND, &writer);
    if (status == LIB_STATUS_OK) status = lib_storage_file_writer_close(writer);
    if (status == LIB_STATUS_OK)
        status = lib_storage_file_read_owned(path, X86_NXVM_MACHINE_EXTENDED_CMOS_SEED_BYTES,
            &bytes, &count);
    if (status == LIB_STATUS_OK && count != 0u &&
        count != X86_NXVM_MACHINE_EXTENDED_CMOS_SEED_BYTES)
        status = LIB_STATUS_INVALID_ARGUMENT;
    if (status == LIB_STATUS_OK && count != 0u) {
        lib_memory_copy(image, bytes, count);
        *present = LIB_TRUE;
    }
    lib_release(bytes);
    return status;
}

lib_status mypc110_cmos_save(const char *path, x86_nxvm_machine *machine)
{
    lib_u8 image[X86_NXVM_MACHINE_EXTENDED_CMOS_SEED_BYTES];
    lib_status status;

    if (path == LIB_NULL || machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_nxvm_machine_capture_cmos(machine, image, sizeof(image));
    if (status == LIB_STATUS_OK)
        status = x86_nxvm_machine_capture_rtc(machine, image,
            X86_NXVM_MACHINE_RTC_SEED_BYTES);
    return status == LIB_STATUS_OK ? mypc110_cmos_save_image(path, image) : status;
}

lib_status mypc110_cmos_save_image(const char *path, const lib_u8 *image)
{
    lib_storage_file_writer *writer = LIB_NULL;
    lib_status status;

    if (path == LIB_NULL || image == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = lib_storage_file_writer_open(path, LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer);
    if (status == LIB_STATUS_OK)
        status = lib_storage_file_writer_write(writer, image,
            X86_NXVM_MACHINE_EXTENDED_CMOS_SEED_BYTES);
    if (writer != LIB_NULL) {
        const lib_status close_status = lib_storage_file_writer_close(writer);
        if (status == LIB_STATUS_OK) status = close_status;
    }
    return status;
}
