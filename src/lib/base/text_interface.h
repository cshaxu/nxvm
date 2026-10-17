#ifndef LIB_BASE_TEXT_INTERFACE_H
#define LIB_BASE_TEXT_INTERFACE_H

#include "lib/types/types_interface.h"

/* True for a NUL-terminated UTF-8 scalar sequence; false for NULL, overlong
 * encodings, surrogate values, incomplete sequences or values above U+10FFFF. */
lib_bool base_text_utf8_valid(const char *text);
/* Complete UTF-8 scalar sequence in exactly length bytes, including U+0000.
 * NULL is valid only with zero length. No byte beyond the span is read. */
lib_bool base_text_utf8_valid_bytes(const char *text, lib_size length);

#endif
