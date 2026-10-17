#include "lib/types/test.h"
#include "lib/base/text_interface.h"

int main(void)
{
    static const char *invalid[] = { "\x80", "\xc0\xaf", "\xc2", "\xe0\x80\x80",
        "\xed\xa0\x80", "\xf0\x80\x80\x80", "\xf4\x90\x80\x80", "\xf5\x80\x80\x80" };
    for (lib_size i = 0u; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
        lib_test_assert(!base_text_utf8_valid(invalid[i]));
    }
    lib_test_assert(base_text_utf8_valid("\x7f\xc2\x80\xdf\xbf\xe0\xa0\x80\xed\x9f\xbf\xee\x80\x80\xef\xbf\xbf\xf0\x90\x80\x80\xf4\x8f\xbf\xbf"));
    lib_test_assert(base_text_utf8_valid("") && !base_text_utf8_valid(LIB_NULL));
    const char span[] = { (char)0xf0, (char)0x90, (char)0x80, (char)0x80 };
    for (lib_size n=1u;n<sizeof(span);++n)
        lib_test_assert(!base_text_utf8_valid_bytes(span,n));
    lib_test_assert(base_text_utf8_valid_bytes(span,sizeof(span)));
    lib_test_assert(base_text_utf8_valid_bytes(LIB_NULL,0u));
    lib_test_assert(!base_text_utf8_valid_bytes(LIB_NULL,1u));
    lib_test_assert(base_text_utf8_valid_bytes("\0x",2u));
    return 0;
}
