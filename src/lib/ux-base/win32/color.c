#include "lib/ux-base/win32/color.h"

#ifdef _WIN32
COLORREF ux_win32_colorref_from_rgb(uint32_t rgb)
{
    return RGB((rgb >> 16u) & 0xffu, (rgb >> 8u) & 0xffu, rgb & 0xffu);
}
#endif
