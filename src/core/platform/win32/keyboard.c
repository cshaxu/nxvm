#include "type.h"

#include <windows.h>

#include "core/platform/win32/keyboard.h"

static type_unsigned_16 core_platform_win32_keyboard_scan(WORD scan)
{
    return (scan & 0xff00u) == 0xe000u ? (type_unsigned_16)(0x0100u |
        (scan & 0x00ffu)) : (type_unsigned_16)(scan & 0x00ffu);
}

static type_status core_platform_win32_keyboard_emit(C_VOID *context,
    core_platform_win32_keyboard_submit submit, type_unsigned_16 scan,
    type_unsigned_16 virtual_key, C_INT pressed)
{
    core_platform_input_event event;

    event.kind = CORE_PLATFORM_INPUT_KEY;
    event.data.key.scan_code = scan;
    event.data.key.virtual_key = virtual_key;
    event.data.key.pressed = pressed;
    return submit(context, &event);
}

type_unsigned_16 core_platform_win32_keyboard_resolve_scan(
    type_unsigned_16 virtual_key)
{
    HKL layout = GetKeyboardLayout(0u);

    return core_platform_win32_keyboard_scan((WORD)MapVirtualKeyExW(virtual_key,
        MAPVK_VK_TO_VSC_EX, layout));
}

C_INT core_platform_win32_keyboard_character_matches_virtual_key(
    type_unsigned_16 code_unit, type_unsigned_16 virtual_key)
{
    SHORT mapped;

    if (code_unit == 0u || (code_unit >= 0xd800u && code_unit <= 0xdfffu)) return 0;
    mapped = VkKeyScanExW((WCHAR)code_unit, GetKeyboardLayout(0u));
    return mapped != -1 && (type_unsigned_16)(mapped & 0xffu) == virtual_key;
}

type_status core_platform_win32_keyboard_submit_key(C_VOID *context,
    core_platform_win32_keyboard_submit submit, type_unsigned_16 scan,
    type_unsigned_16 virtual_key, C_INT pressed)
{
    if (submit == STD_NULL || virtual_key == 0u) return TYPE_STATUS_INVALID_ARGUMENT;
    if (scan == 0u) scan = core_platform_win32_keyboard_resolve_scan(virtual_key);
    return scan == 0u ? TYPE_STATUS_UNSUPPORTED : core_platform_win32_keyboard_emit(
        context, submit, scan, virtual_key, pressed);
}

type_status core_platform_win32_keyboard_submit_character(C_VOID *context,
    core_platform_win32_keyboard_submit submit, type_unsigned_32 scalar)
{
    HKL layout;
    SHORT mapped;
    type_unsigned_16 virtual_key;
    type_unsigned_16 scan;
    type_unsigned_8 modifiers;

    if (submit == STD_NULL || scalar == 0u || scalar > 0xffffu ||
        (scalar >= 0xd800u && scalar <= 0xdfffu)) return TYPE_STATUS_UNSUPPORTED;
    layout = GetKeyboardLayout(0u);
    mapped = VkKeyScanExW((WCHAR)scalar, layout);
    if (mapped == -1) return TYPE_STATUS_UNSUPPORTED;
    virtual_key = (type_unsigned_16)(mapped & 0xff);
    scan = core_platform_win32_keyboard_scan((WORD)MapVirtualKeyExW(virtual_key,
        MAPVK_VK_TO_VSC_EX, layout));
    if (scan == 0u) return TYPE_STATUS_UNSUPPORTED;
    modifiers = (type_unsigned_8)((mapped >> 8u) & 0xff);
    if ((modifiers & 2u) != 0u && core_platform_win32_keyboard_emit(context, submit,
            0x001du, VK_CONTROL, TYPE_TRUE) != TYPE_STATUS_OK) return TYPE_STATUS_FAULT;
    if ((modifiers & 4u) != 0u && core_platform_win32_keyboard_emit(context, submit,
            0x0038u, VK_MENU, TYPE_TRUE) != TYPE_STATUS_OK) return TYPE_STATUS_FAULT;
    if ((modifiers & 1u) != 0u && core_platform_win32_keyboard_emit(context, submit,
            0x002au, VK_SHIFT, TYPE_TRUE) != TYPE_STATUS_OK) return TYPE_STATUS_FAULT;
    if (core_platform_win32_keyboard_emit(context, submit, scan, virtual_key,
            TYPE_TRUE) != TYPE_STATUS_OK || core_platform_win32_keyboard_emit(context,
            submit, scan, virtual_key, TYPE_FALSE) != TYPE_STATUS_OK) return TYPE_STATUS_FAULT;
    if ((modifiers & 1u) != 0u) (C_VOID)core_platform_win32_keyboard_emit(context,
        submit, 0x002au, VK_SHIFT, TYPE_FALSE);
    if ((modifiers & 4u) != 0u) (C_VOID)core_platform_win32_keyboard_emit(context,
        submit, 0x0038u, VK_MENU, TYPE_FALSE);
    if ((modifiers & 2u) != 0u) (C_VOID)core_platform_win32_keyboard_emit(context,
        submit, 0x001du, VK_CONTROL, TYPE_FALSE);
    return TYPE_STATUS_OK;
}

type_status core_platform_win32_keyboard_submit_utf16(
    core_platform_win32_keyboard_utf16 *state, C_VOID *context,
    core_platform_win32_keyboard_submit submit, type_unsigned_16 code_unit)
{
    type_unsigned_32 scalar;

    if (state == STD_NULL || submit == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (code_unit >= 0xd800u && code_unit <= 0xdbffu) {
        if (state->pending_high_surrogate != 0u) return TYPE_STATUS_UNSUPPORTED;
        state->pending_high_surrogate = code_unit;
        return TYPE_STATUS_OK;
    }
    if (code_unit >= 0xdc00u && code_unit <= 0xdfffu) {
        if (state->pending_high_surrogate == 0u) return TYPE_STATUS_UNSUPPORTED;
        scalar = 0x10000u + (((type_unsigned_32)state->pending_high_surrogate -
            0xd800u) << 10u) + ((type_unsigned_32)code_unit - 0xdc00u);
        state->pending_high_surrogate = 0u;
        return core_platform_win32_keyboard_submit_character(context, submit, scalar);
    }
    if (state->pending_high_surrogate != 0u) {
        state->pending_high_surrogate = 0u;
        return TYPE_STATUS_UNSUPPORTED;
    }
    return core_platform_win32_keyboard_submit_character(context, submit, code_unit);
}
