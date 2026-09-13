#include "lib/types/types_interface.h"

int main(void)
{
    lib_u8 source[] = { 0x01u, 0x02u, 0x03u };
    lib_u8 destination[] = { 0u, 0u, 0u };
    char *allocated;

    if (lib_memory_copy(destination, source, sizeof(source)) != destination ||
        lib_memory_compare(destination, source, sizeof(source)) != 0) return 1;
    lib_memory_set(destination, 0, sizeof(destination));
    if (destination[0] != 0u || destination[1] != 0u || destination[2] != 0u)
        return 2;
    if (lib_memory_find(source, 0x02, sizeof(source)) != source + 1 ||
        lib_text_length("ab") != 2u) return 3;
    allocated = lib_allocate_zero(3u, 1u);
    if (allocated == LIB_NULL || allocated[0] != '\0' || allocated[2] != '\0') {
        lib_release(allocated);
        return 4;
    }
    lib_release(allocated);
    return 0;
}
