#include "machine/driver.h"

#include "lib/storage/file_interface.h"

#define CORE_BATTERY_SAVE_HEADER_BYTES 24u
#define CORE_BATTERY_SAVE_VERSION 1u

static void core_battery_write_u32(lib_u8 *bytes, lib_u32 value)
{
    bytes[0] = (lib_u8)value;
    bytes[1] = (lib_u8)(value >> 8u);
    bytes[2] = (lib_u8)(value >> 16u);
    bytes[3] = (lib_u8)(value >> 24u);
}

static lib_u32 core_battery_read_u32(const lib_u8 *bytes)
{ return (lib_u32)bytes[0] | ((lib_u32)bytes[1] << 8u) |
    ((lib_u32)bytes[2] << 16u) | ((lib_u32)bytes[3] << 24u); }

static void core_battery_write_u64(lib_u8 *bytes, lib_u64 value)
{
    core_battery_write_u32(bytes, (lib_u32)value);
    core_battery_write_u32(bytes + 4u, (lib_u32)(value >> 32u));
}

static lib_u64 core_battery_read_u64(const lib_u8 *bytes)
{ return (lib_u64)core_battery_read_u32(bytes) |
    ((lib_u64)core_battery_read_u32(bytes + 4u) << 32u); }

lib_bool core_driver_set_media(void *context, const char *path,
    lib_storage_medium_mode mode)
{
    core_driver *driver = context;
    core_machine *candidate = LIB_NULL;
    void *bytes = LIB_NULL;
    lib_size byte_count = 0u;
    lib_status status;

    if (driver == LIB_NULL || mode != LIB_STORAGE_MEDIUM_READONLY) return LIB_FALSE;
    if (path == LIB_NULL) {
        core_machine_destroy(driver->machine);
        driver->machine = LIB_NULL;
        driver->published_frame_revision = 0u;
        return LIB_TRUE;
    }
    status = lib_storage_file_read_owned(path, core_machine_image_maximum_bytes(),
        &bytes, &byte_count);
    if (status == LIB_STATUS_OK && !core_machine_image_normalize_size(bytes, &byte_count))
        status = LIB_STATUS_INVALID_ARGUMENT;
    if (status == LIB_STATUS_OK) {
        core_machine_options options = { .initial_ram_byte = driver->options.initial_ram_byte };
        status = core_machine_create(&candidate, bytes, byte_count, &options);
    }
    lib_release(bytes);
    if (status != LIB_STATUS_OK) return LIB_FALSE;
    core_machine_destroy(driver->machine);
    driver->machine = candidate;
    driver->published_frame_revision = 0u;
    lib_atomic_i32_store_explicit(&driver->debug_stop_requested, 0,
        LIB_MEMORY_ORDER_SEQ_CST);
    return LIB_TRUE;
}

lib_status core_driver_save_battery_ram(core_driver *driver, const char *path)
{
    lib_u8 bytes[CORE_BATTERY_SAVE_HEADER_BYTES + CORE_MACHINE_BATTERY_BYTE_CAPACITY] = { 0 };
    core_machine_battery battery;
    lib_status status;

    if (driver == LIB_NULL || path == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (driver->machine == LIB_NULL) return LIB_STATUS_OK;
    status = core_machine_battery_capture(driver->machine, &battery);
    if (status != LIB_STATUS_OK) return status;
    if (battery.byte_count == 0u || battery.dirty == LIB_FALSE)
        return LIB_STATUS_OK;
    bytes[0] = 'M'; bytes[1] = 'N'; bytes[2] = 'S'; bytes[3] = 'R';
    bytes[4] = CORE_BATTERY_SAVE_VERSION;
    bytes[5] = battery.mapper;
    core_battery_write_u64(bytes + 8u, battery.identity);
    core_battery_write_u32(bytes + 16u, (lib_u32)battery.byte_count);
    lib_memory_copy(bytes + CORE_BATTERY_SAVE_HEADER_BYTES, battery.bytes,
        battery.byte_count);
    {
        lib_storage_file_writer *writer = LIB_NULL;
        status = lib_storage_file_writer_open(path,
            LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer);
        if (status == LIB_STATUS_OK)
            status = lib_storage_file_writer_write(writer, bytes,
                CORE_BATTERY_SAVE_HEADER_BYTES + battery.byte_count);
        if (writer != LIB_NULL && lib_storage_file_writer_close(writer) != LIB_STATUS_OK &&
            status == LIB_STATUS_OK) status = LIB_STATUS_IO_ERROR;
        if (status == LIB_STATUS_OK)
            status = core_machine_battery_mark_clean(driver->machine);
        return status;
    }
}

lib_status core_driver_load_battery_ram(core_driver *driver, const char *path)
{
    void *bytes = LIB_NULL;
    core_machine_battery battery;
    lib_size count;
    lib_status status;

    if (driver == LIB_NULL || path == LIB_NULL || driver->machine == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = core_machine_battery_capture(driver->machine, &battery);
    if (status != LIB_STATUS_OK) return status;
    if (battery.byte_count == 0u) return LIB_STATUS_INVALID_STATE;
    count = battery.byte_count;
    status = lib_storage_file_read_owned(path, CORE_BATTERY_SAVE_HEADER_BYTES + count,
        &bytes, &count);
    if (status == LIB_STATUS_OK && (count != CORE_BATTERY_SAVE_HEADER_BYTES +
            battery.byte_count ||
        ((const lib_u8 *)bytes)[0] != 'M' || ((const lib_u8 *)bytes)[1] != 'N' ||
        ((const lib_u8 *)bytes)[2] != 'S' || ((const lib_u8 *)bytes)[3] != 'R' ||
        ((const lib_u8 *)bytes)[4] != CORE_BATTERY_SAVE_VERSION ||
        ((const lib_u8 *)bytes)[5] != battery.mapper ||
        core_battery_read_u64((const lib_u8 *)bytes + 8u) !=
            battery.identity ||
        core_battery_read_u32((const lib_u8 *)bytes + 16u) !=
            battery.byte_count))
        status = LIB_STATUS_INVALID_ARGUMENT;
    if (status == LIB_STATUS_OK) {
        lib_memory_copy(battery.bytes,
            (const lib_u8 *)bytes + CORE_BATTERY_SAVE_HEADER_BYTES, battery.byte_count);
        status = core_machine_battery_restore(driver->machine, &battery);
    }
    lib_release(bytes);
    return status;
}
