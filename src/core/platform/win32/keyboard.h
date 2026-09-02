#ifndef CORE_PLATFORM_WIN32_KEYBOARD_H
#define CORE_PLATFORM_WIN32_KEYBOARD_H

#include "type.h"
#include "core/platform/input_interface.h"

typedef type_status (*core_platform_win32_keyboard_submit)(C_VOID *context,
    const core_platform_input_event *event);

typedef struct core_platform_win32_keyboard_normalizer {
    type_unsigned_16 pending_high_surrogate;
    type_unsigned_16 recovered_virtual_key;
} core_platform_win32_keyboard_normalizer;

/* Resolves a Windows virtual key to the matching PC scan code for the active
 * host layout.  A zero result means the key has no physical representation. */
type_unsigned_16 core_platform_win32_keyboard_resolve_scan(
    type_unsigned_16 virtual_key);

/* Identifies the text result of a recovered virtual key without emitting it. */
C_INT core_platform_win32_keyboard_character_matches_virtual_key(
    type_unsigned_16 code_unit, type_unsigned_16 virtual_key);

/* Publishes a native/recovered physical transition through the caller's sink. */
type_status core_platform_win32_keyboard_submit_key(C_VOID *context,
    core_platform_win32_keyboard_submit submit, type_unsigned_16 scan,
    type_unsigned_16 virtual_key, C_INT pressed);

/* Correlates a Window zero-scan virtual key with its later text message. */
C_VOID core_platform_win32_keyboard_note_recovered_key(
    core_platform_win32_keyboard_normalizer *state, type_unsigned_16 virtual_key);
C_VOID core_platform_win32_keyboard_release_recovered_key(
    core_platform_win32_keyboard_normalizer *state, type_unsigned_16 virtual_key);
C_INT core_platform_win32_keyboard_consume_duplicate_character(
    core_platform_win32_keyboard_normalizer *state, type_unsigned_16 code_unit);

/* Converts one Windows Unicode scalar to the physical key sequence selected by
 * the active host layout.  It never owns guest state or text input. */
type_status core_platform_win32_keyboard_submit_character(C_VOID *context,
    core_platform_win32_keyboard_submit submit, type_unsigned_32 scalar);

/* Accepts one UTF-16 code unit without truncating surrogate pairs.  Valid
 * scalars that lack a physical host-layout sequence return UNSUPPORTED. */
type_status core_platform_win32_keyboard_submit_utf16(
    core_platform_win32_keyboard_normalizer *state, C_VOID *context,
    core_platform_win32_keyboard_submit submit, type_unsigned_16 code_unit);

#endif
