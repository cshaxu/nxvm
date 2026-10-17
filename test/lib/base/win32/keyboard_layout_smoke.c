#include "lib/types/test.h"
#include "lib/types/win32/test.h"
/* Compile the real component bodies against controlled external queries.
 * No KVM interaction, sleeping, or timing-dependent assertion is needed. */
#include "lib/types/win32/input.h"
#include "lib/types/types_interface.h"

static lib_win32_short layout_result;
static lib_win32_short fake_key_scan(lib_win32_wchar scalar)
{ (void)scalar; return layout_result; }
static lib_win32_uint fake_scan(lib_win32_uint key, lib_win32_uint kind)
{ (void)kind; return key == 'A' ? 0x1eu : 0u; }

#undef lib_win32_key_scan
#define lib_win32_key_scan fake_key_scan
#undef lib_win32_map_virtual_key
#define lib_win32_map_virtual_key fake_scan
#include "lib/base/win32/input.c"
#include "lib/base/input.c"

int main(void)
{
    for (lib_u32 raw=0;raw<8;++raw) {
        lib_u16 key;
        lib_u8 modifiers, expected=0;
        if(raw&1u) expected|=BASE_KEY_MODIFIER_SHIFT;
        if(raw&2u) expected|=BASE_KEY_MODIFIER_CONTROL;
        if(raw&4u) expected|=BASE_KEY_MODIFIER_ALT;
        layout_result=(lib_win32_short)((raw<<8u)|'A');
        lib_test_assert(base_keyboard_platform_map_scalar('a',&key,&modifiers));
        lib_test_assert(key=='A' && modifiers==expected);
        base_key_input mapped;
        lib_test_assert(base_keyboard_map_scalar('a',&mapped));
        lib_test_assert(mapped.key=='A' && mapped.scan_code==0x1e && mapped.modifiers==expected);
    }
    layout_result=-1;
    lib_u16 key=99; lib_u8 modifiers=99;
    lib_test_assert(!base_keyboard_platform_map_scalar('a',&key,&modifiers));
    lib_test_assert(key==99 && modifiers==99);
    layout_result=LIB_WIN32_KEY_OEM_102;
    base_key_input mapped;
    lib_test_assert(!base_keyboard_map_scalar('<',&mapped));
    return 0;
}
