#ifndef LIB_TYPES_WIN32_TEXT_H
#define LIB_TYPES_WIN32_TEXT_H

#include "lib/types/win32/scalar.h"

#define lib_win32_multi_byte_to_wide_char MultiByteToWideChar
#define lib_win32_wide_char_to_multi_byte WideCharToMultiByte
#define LIB_WIN32_CP_UTF8 CP_UTF8
#define LIB_WIN32_MB_ERR_INVALID_CHARS MB_ERR_INVALID_CHARS
#define LIB_WIN32_WC_ERR_INVALID_CHARS WC_ERR_INVALID_CHARS

#endif
