#ifndef UX_BASE_WIN32_COLOR_H
#define UX_BASE_WIN32_COLOR_H

#ifdef _WIN32
#include <stdint.h>
#include <windows.h>

/* Shared native-boundary conversion for copied 0x00RRGGBB palette values. */
COLORREF ux_win32_colorref_from_rgb(uint32_t rgb);
#endif

#endif
