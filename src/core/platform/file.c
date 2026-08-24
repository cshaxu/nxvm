#include "core/platform/file.h"

struct core_platform_file_reader { STD_FILE *file; };
struct core_platform_file_writer { STD_FILE *file; };

core_platform_file_write_result core_platform_file_write_exclusive(const C_CHAR *path, const C_VOID *bytes,
    STD_SIZE_T count)
{
    STD_FILE *file;
    C_INT failed;

    if (path == STD_NULL || (count != 0u && bytes == STD_NULL))
        return CORE_PLATFORM_FILE_WRITE_FAULT;
    if ((file = STD_FOPEN_EXCLUSIVE_WRITE(path)) == STD_NULL)
        return CORE_PLATFORM_FILE_WRITE_EXISTS;
    failed = (count != 0u && STD_FWRITE(bytes, 1u, count, file) != count) ||
        STD_FCLOSE(file) != 0;
    if (failed) (C_VOID)STD_REMOVE(path);
    return failed ? CORE_PLATFORM_FILE_WRITE_FAULT : CORE_PLATFORM_FILE_WRITE_OK;
}

C_INT core_platform_file_read_all(const C_CHAR *path, STD_SIZE_T maximum,
    C_VOID **out_bytes, STD_SIZE_T *out_count)
{
    STD_FILE *file;
    type_signed_64 length;
    C_VOID *bytes = STD_NULL;

    if (out_bytes == STD_NULL || out_count == STD_NULL) return TYPE_TRUE;
    *out_bytes = STD_NULL;
    *out_count = 0u;
    if (path == STD_NULL || (file = STD_FOPEN(path, "rb")) == STD_NULL) return TYPE_TRUE;
    if (STD_FSEEK_64(file, 0, STD_SEEK_END) != 0 ||
        (length = STD_FTELL_64(file)) < 0 || (type_unsigned_64)length > maximum ||
        STD_FSEEK_64(file, 0, STD_SEEK_SET) != 0 ||
        (bytes = STD_MALLOC((STD_SIZE_T)length == 0u ? 1u : (STD_SIZE_T)length)) == STD_NULL ||
        ((STD_SIZE_T)length != 0u && STD_FREAD(bytes, 1u, (STD_SIZE_T)length, file) !=
            (STD_SIZE_T)length)) {
        (C_VOID)STD_FCLOSE(file);
        if (bytes != STD_NULL) STD_FREE(bytes);
        return TYPE_TRUE;
    }
    if (STD_FCLOSE(file) != 0) {
        STD_FREE(bytes);
        return TYPE_TRUE;
    }
    *out_bytes = bytes;
    *out_count = (STD_SIZE_T)length;
    return TYPE_FALSE;
}

C_INT core_platform_file_exists(const C_CHAR *path)
{
    STD_FILE *file = path == STD_NULL ? STD_NULL : STD_FOPEN(path, "rb");

    if (file == STD_NULL) return TYPE_FALSE;
    return STD_FCLOSE(file) == 0 ? TYPE_TRUE : TYPE_FALSE;
}

C_INT core_platform_file_replace(const C_CHAR *source, const C_CHAR *destination)
{
    return source == STD_NULL || destination == STD_NULL ||
        STD_RENAME_REPLACE(source, destination) != 0 ? TYPE_TRUE : TYPE_FALSE;
}

C_INT core_platform_file_remove(const C_CHAR *path)
{
    return path == STD_NULL || STD_REMOVE(path) != 0 ? TYPE_TRUE : TYPE_FALSE;
}

type_status core_platform_file_reader_open(const C_CHAR *path,
    core_platform_file_reader **out_reader)
{
    core_platform_file_reader *reader;

    if (out_reader == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_reader = STD_NULL;
    if (path == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    reader = (core_platform_file_reader *)STD_MALLOC(sizeof(*reader));
    if (reader == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    reader->file = STD_FOPEN(path, "rb");
    if (reader->file == STD_NULL) { STD_FREE(reader); return TYPE_STATUS_FAULT; }
    *out_reader = reader;
    return TYPE_STATUS_OK;
}

C_INT core_platform_file_reader_next(core_platform_file_reader *reader,
    C_CHAR *line, STD_SIZE_T capacity)
{
    return reader != STD_NULL && line != STD_NULL && capacity != 0u &&
        capacity <= 0x7fffffffu && STD_FGETS(line, (C_INT)capacity, reader->file) != STD_NULL;
}

C_VOID core_platform_file_reader_close(core_platform_file_reader *reader)
{
    if (reader == STD_NULL) return;
    if (reader->file != STD_NULL) (C_VOID)STD_FCLOSE(reader->file);
    STD_FREE(reader);
}

type_status core_platform_file_writer_open(const C_CHAR *path,
    core_platform_file_writer **out_writer)
{
    core_platform_file_writer *writer;

    if (out_writer == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_writer = STD_NULL;
    if (path == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    writer = (core_platform_file_writer *)STD_MALLOC(sizeof(*writer));
    if (writer == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    writer->file = STD_FOPEN(path, "w");
    if (writer->file == STD_NULL) { STD_FREE(writer); return TYPE_STATUS_FAULT; }
    *out_writer = writer;
    return TYPE_STATUS_OK;
}

type_status core_platform_file_writer_write(core_platform_file_writer *writer,
    const C_CHAR *text)
{
    return writer == STD_NULL || text == STD_NULL || STD_FPUTS(text, writer->file) < 0 ?
        TYPE_STATUS_FAULT : TYPE_STATUS_OK;
}

type_status core_platform_file_writer_close(core_platform_file_writer *writer)
{
    C_INT result;

    if (writer == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    result = STD_FCLOSE(writer->file);
    STD_FREE(writer);
    return result == 0 ? TYPE_STATUS_OK : TYPE_STATUS_FAULT;
}
