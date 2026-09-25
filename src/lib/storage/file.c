#include "lib/types/types_interface.h"

#include "lib/storage/file.h"
#include "lib/storage/file_interface.h"

lib_status lib_storage_file_close(lib_storage_file *file)
{
    lib_i32 result;
    if (file == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (file->stream == LIB_NULL) return LIB_STATUS_OK;
    result = lib_c_fclose(file->stream);
    file->stream = LIB_NULL;
    return result == 0 ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}

struct lib_storage_file_writer { lib_storage_file file; };
struct lib_storage_file_reader { lib_storage_file file; };

lib_status lib_storage_file_read_exact(const lib_storage_file *file, void *bytes,
    lib_size byte_count)
{
    lib_size transferred;

    if (file == LIB_NULL || (bytes == LIB_NULL && byte_count != 0u))
        return LIB_STATUS_INVALID_ARGUMENT;
    transferred = lib_c_fread(bytes, 1u, byte_count, file->stream);
    return transferred == byte_count && !lib_c_ferror(file->stream) ?
        LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}

lib_status lib_storage_file_write_exact(lib_storage_file *file,
    const void *bytes, lib_size byte_count)
{
    lib_size transferred;

    if (file == LIB_NULL || (bytes == LIB_NULL && byte_count != 0u))
        return LIB_STATUS_INVALID_ARGUMENT;
    transferred = lib_c_fwrite(bytes, 1u, byte_count, file->stream);
    return transferred == byte_count && !lib_c_ferror(file->stream) ?
        LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}

lib_status lib_storage_file_flush(lib_storage_file *file)
{ return file == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
    lib_c_fflush(file->stream) == 0 ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR; }

lib_status lib_storage_file_seek_absolute(const lib_storage_file *file,
    lib_i64 offset)
{ return file == LIB_NULL || offset < 0 ? LIB_STATUS_INVALID_ARGUMENT :
    storage_file_platform_seek(file, offset, LIB_SEEK_SET); }

lib_status lib_storage_file_byte_count(lib_storage_file *file,
    lib_i64 *out_byte_count)
{
    lib_i64 offset, length;
    if (file == LIB_NULL || out_byte_count == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    offset = storage_file_platform_tell(file);
    if (offset < 0 || storage_file_platform_seek(file, 0, LIB_SEEK_END) != LIB_STATUS_OK)
        return LIB_STATUS_IO_ERROR;
    length = storage_file_platform_tell(file);
    if (length < 0 || storage_file_platform_seek(file, offset, LIB_SEEK_SET) != LIB_STATUS_OK)
        return LIB_STATUS_IO_ERROR;
    *out_byte_count = length;
    return LIB_STATUS_OK;
}

lib_status lib_storage_file_read_owned(const char *path, lib_size maximum,
    void **out_bytes, lib_size *out_byte_count)
{
    lib_storage_file file = { 0 };
    lib_i64 length;
    void *bytes = LIB_NULL;
    lib_status status, close_status;

    if (out_bytes == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_bytes = LIB_NULL;
    if (out_byte_count == LIB_NULL || path == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_byte_count = 0u;
    status = storage_file_platform_open(path, LIB_FALSE, &file);
    if (status == LIB_STATUS_OK) status = lib_storage_file_byte_count(&file, &length);
    if (status == LIB_STATUS_OK && (length < 0 || (lib_u64)length > maximum))
        status = LIB_STATUS_LIMIT_EXCEEDED;
    if (status == LIB_STATUS_OK) {
        bytes = lib_allocate((lib_size)length == 0u ? 1u : (lib_size)length);
        status = bytes == LIB_NULL ? LIB_STATUS_NO_MEMORY :
            lib_storage_file_read_exact(&file, bytes, (lib_size)length);
    }
    close_status = lib_storage_file_close(&file);
    if (status == LIB_STATUS_OK) status = close_status;
    if (status != LIB_STATUS_OK) {
        lib_release(bytes);
        return status;
    }
    *out_bytes = bytes;
    *out_byte_count = (lib_size)length;
    return LIB_STATUS_OK;
}

lib_status lib_storage_file_reader_open(const char *path,
    lib_storage_file_reader **out_reader)
{
    lib_storage_file_reader *reader;

    if (out_reader == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_reader = LIB_NULL;
    if (path == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    reader = lib_allocate(sizeof(*reader));
    if (reader == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    if (storage_file_platform_open(path, LIB_FALSE, &reader->file) !=
        LIB_STATUS_OK) {
        lib_release(reader);
        return LIB_STATUS_IO_ERROR;
    }
    *out_reader = reader;
    return LIB_STATUS_OK;
}

lib_status lib_storage_file_reader_read(lib_storage_file_reader *reader,
    void *bytes, lib_size byte_count)
{
    if (reader == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return lib_storage_file_read_exact(&reader->file, bytes, byte_count);
}

lib_status lib_storage_file_reader_close(lib_storage_file_reader *reader)
{
    lib_status status;
    if (reader == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = lib_storage_file_close(&reader->file);
    lib_release(reader);
    return status;
}

lib_status lib_storage_file_writer_open(const char *path,
    lib_storage_file_writer_mode mode,
    lib_storage_file_writer **out_writer)
{
    lib_storage_file_writer *writer;

    if (out_writer == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_writer = LIB_NULL;
    if (path == LIB_NULL ||
        mode < LIB_STORAGE_FILE_WRITER_TRUNCATE ||
        mode > LIB_STORAGE_FILE_WRITER_APPEND) return LIB_STATUS_INVALID_ARGUMENT;
    writer = lib_allocate(sizeof(*writer));
    if (writer == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    writer->file.stream = lib_c_fopen(path,
        mode == LIB_STORAGE_FILE_WRITER_TRUNCATE ? "wb" : "ab");
    if (writer->file.stream == LIB_NULL) {
        lib_release(writer);
        return LIB_STATUS_IO_ERROR;
    }
    *out_writer = writer;
    return LIB_STATUS_OK;
}

lib_status lib_storage_file_writer_write(lib_storage_file_writer *writer,
    const void *bytes, lib_size byte_count)
{
    if (writer == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return lib_storage_file_write_exact(&writer->file, bytes, byte_count);
}

lib_status lib_storage_file_writer_close(lib_storage_file_writer *writer)
{
    lib_status status;
    if (writer == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = lib_storage_file_close(&writer->file);
    lib_release(writer);
    return status;
}
