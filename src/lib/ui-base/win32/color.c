#include "lib/ui-base/win32/color.h"

#ifdef _WIN32
COLORREF ui_win32_colorref_from_rgb(lib_u32 rgb)
{
    return RGB((rgb >> 16u) & 0xffu, (rgb >> 8u) & 0xffu, rgb & 0xffu);
}
#endif
