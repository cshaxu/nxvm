#include "lib/base/text_interface.h"

lib_bool base_text_utf8_valid_bytes(const char *text, lib_size length)
{
    const lib_u8 *next = (const lib_u8 *)text;
    if (next == LIB_NULL) return length == 0u;
    while (length != 0u) {
        lib_u32 value = *next++;
        lib_u32 count, minimum;
        --length;
        if (value < 0x80u) continue;
        if (value >= 0xc2u && value <= 0xdfu) {
            value &= 0x1fu; count = 1u; minimum = 0x80u;
        } else if (value >= 0xe0u && value <= 0xefu) {
            value &= 0x0fu; count = 2u; minimum = 0x800u;
        } else if (value >= 0xf0u && value <= 0xf4u) {
            value &= 0x07u; count = 3u; minimum = 0x10000u;
        } else return LIB_FALSE;
        if (count > length) return LIB_FALSE;
        length -= count;
        while (count-- != 0u) {
            if ((*next & 0xc0u) != 0x80u) return LIB_FALSE;
            value = (value << 6) | (*next++ & 0x3fu);
        }
        if (value < minimum || value > 0x10ffffu ||
            (value >= 0xd800u && value <= 0xdfffu)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

lib_bool base_text_utf8_valid(const char *text)
{
    return text != LIB_NULL && base_text_utf8_valid_bytes(text, lib_text_length(text));
}
