#include "lib/types/types_interface.h"

int main(void)
{
    char text[32] = "AB";
    char format[32] = {0};
    char *cursor;
    lib_size remaining;
    lib_u8 source[] = { 0x01u, 0x02u, 0x03u };
    lib_u8 destination[] = { 0u, 0u, 0u };
    char *allocated;

    if (lib_memory_copy(destination, source, sizeof(source)) != destination ||
        lib_memory_compare(destination, source, sizeof(source)) != 0) return 1;
    lib_memory_set(destination, 0, sizeof(destination));
    if (destination[0] != 0u || destination[1] != 0u || destination[2] != 0u)
        return 2;
    lib_text_ascii_lower(text);
    if (lib_text_compare(text, "ab") != 0 || lib_text_length(text) != 2u) return 3;
    if (lib_text_format(format, sizeof(format), "%s", "one") != 3) return 4;
    cursor = format + lib_text_length(format);
    remaining = sizeof(format) - lib_text_length(format);
    if (
        lib_text_format_append(&cursor, &remaining, "%s", " two") != 4 ||
        lib_text_compare(format, "one two") != 0) return 4;
    allocated = lib_allocate_zero(3u, 1u);
    if (allocated == LIB_NULL || allocated[0] != '\0' || allocated[2] != '\0') {
        lib_release(allocated);
        return 5;
    }
    lib_release(allocated);
    return 0;
}
