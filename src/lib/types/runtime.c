#include "lib/types/types_interface.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *lib_memory_set(void *destination, int value, lib_size byte_count)
{
    return memset(destination, value, byte_count);
}

void *lib_memory_copy(void *destination, const void *source, lib_size byte_count)
{
    return memcpy(destination, source, byte_count);
}

void *lib_memory_move(void *destination, const void *source, lib_size byte_count)
{
    return memmove(destination, source, byte_count);
}

int lib_memory_compare(const void *left, const void *right, lib_size byte_count)
{
    return memcmp(left, right, byte_count);
}

lib_size lib_text_length(const char *text)
{
    return strlen(text);
}

int lib_text_compare(const char *left, const char *right)
{
    return strcmp(left, right);
}

char *lib_text_tokenize(char *text, const char *delimiters)
{
    return strtok(text, delimiters);
}

void lib_text_ascii_lower(char *text)
{
    if (text == LIB_NULL) return;
    while (*text != '\0') {
        if (*text >= 'A' && *text <= 'Z') *text = (char)(*text + ('a' - 'A'));
        ++text;
    }
}

int lib_text_format(char *destination, lib_size destination_capacity,
    const char *format, ...)
{
    int written;
    va_list arguments;

    va_start(arguments, format);
    written = lib_text_format_v(destination, destination_capacity, format, arguments);
    va_end(arguments);
    return written;
}

int lib_text_format_v(char *destination, lib_size destination_capacity,
    const char *format, lib_format_arguments arguments)
{
    return vsnprintf(destination, destination_capacity, format, arguments);
}

int lib_text_format_append(char **cursor, lib_size *remaining,
    const char *format, ...)
{
    int written;
    va_list arguments;

    if (cursor == LIB_NULL || *cursor == LIB_NULL || remaining == LIB_NULL ||
        *remaining == 0u || format == LIB_NULL) return -1;
    va_start(arguments, format);
    written = lib_text_format_append_v(cursor, remaining, format, arguments);
    va_end(arguments);
    return written;
}

int lib_text_format_append_v(char **cursor, lib_size *remaining,
    const char *format, lib_format_arguments arguments)
{
    int written;

    if (cursor == LIB_NULL || *cursor == LIB_NULL || remaining == LIB_NULL ||
        *remaining == 0u || format == LIB_NULL) return -1;
    written = vsnprintf(*cursor, *remaining, format, arguments);
    if (written < 0 || (lib_size)written >= *remaining) return -1;
    *cursor += written;
    *remaining -= (lib_size)written;
    return written;
}

void *lib_allocate(lib_size byte_count)
{
    return malloc(byte_count);
}

void *lib_allocate_zero(lib_size count, lib_size byte_count)
{
    return calloc(count, byte_count);
}

void lib_release(void *memory)
{
    free(memory);
}
