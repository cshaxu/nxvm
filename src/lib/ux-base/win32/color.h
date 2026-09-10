#ifndef UX_BASE_WIN32_COLOR_H
#define UX_BASE_WIN32_COLOR_H

#include "lib/base/base_interface.h"

#ifdef _WIN32
#include <windows.h>

/* Shared native-boundary conversion for copied 0x00RRGGBB palette values. */
COLORREF ux_win32_colorref_from_rgb(lib_u32 rgb);
#endif

#endif
