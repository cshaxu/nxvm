#include "lib/types/test.h"
#include "lib/base/input_adapter_interface.h"
#include "lib/base/linux/input.h"

static lib_key_input received;
int main(void)
{
    base_keyboard_decoder state = { 0 };
    static const lib_key keys[] = {
        LIB_KEY_ENTER, LIB_KEY_BACKSPACE,
        LIB_KEY_F1, LIB_KEY_F2, LIB_KEY_F3, LIB_KEY_F4, LIB_KEY_F5, LIB_KEY_F6,
        LIB_KEY_F7, LIB_KEY_F8, LIB_KEY_F9, LIB_KEY_F10, LIB_KEY_F11, LIB_KEY_F12,
        LIB_KEY_UP, LIB_KEY_DOWN, LIB_KEY_LEFT, LIB_KEY_RIGHT,
        LIB_KEY_HOME, LIB_KEY_END, LIB_KEY_PAGE_UP, LIB_KEY_PAGE_DOWN,
        LIB_KEY_INSERT, LIB_KEY_DELETE
    };
    for (lib_u16 key = BASE_LINUX_KEY_ENTER; key <= BASE_LINUX_KEY_DELETE; ++key) {
        base_keyboard_record record = { .kind = BASE_KEYBOARD_TRANSITION,
            .key = key, .pressed = LIB_TRUE, .repeat_count = 3u,
            .modifiers = LIB_KEY_MODIFIER_SHIFT };
        lib_test_assert(base_keyboard_decode(&state, &record, &received) == BASE_KEYBOARD_ACCEPTED);
        lib_test_assert(received.key == (lib_u32)keys[key] && received.scan_code == 0u &&
            received.pressed && received.repeat_count == 3u &&
            received.modifiers == LIB_KEY_MODIFIER_SHIFT);
        record.pressed = LIB_FALSE;
        lib_test_assert(base_keyboard_decode(&state, &record, &received) == BASE_KEYBOARD_ACCEPTED);
        lib_test_assert(!received.pressed && received.repeat_count == 1u);
    }
    /* This terminal backend cannot invent physical identity for text. */
    received = (lib_key_input){ .key = LIB_KEY_ENTER, .scan_code = 1u };
    lib_test_assert(!base_keyboard_map_scalar('a', &received));
    lib_test_assert(received.key == 0u && received.scan_code == 0u);
    lib_test_assert(base_keyboard_decode(&(base_keyboard_decoder){ 0 }, &(base_keyboard_record){ BASE_KEYBOARD_TRANSITION,
            0u, BASE_LINUX_KEY_UP, 0u, 0u, 0u, 1, 1u }, &received));
    lib_test_assert(received.key == LIB_KEY_UP &&
        received.scan_code == 0u);
    lib_test_assert(base_keyboard_decode(&state,
        &(base_keyboard_record){ .kind=BASE_KEYBOARD_CHARACTER, .utf16='a', .repeat_count=1u }, &received));
    lib_test_assert(received.scalar == 'a');
    lib_test_assert(base_keyboard_decode(&state,
        &(base_keyboard_record){ .kind=BASE_KEYBOARD_CHARACTER, .utf16=0xd83du, .repeat_count=1u }, &received));
    lib_test_assert(base_keyboard_decode(&state,
        &(base_keyboard_record){ .kind=BASE_KEYBOARD_CHARACTER, .utf16=0xde00u, .repeat_count=1u }, &received));
    lib_test_assert(received.scalar == 0x1f600u);
    lib_test_assert(base_keyboard_decode(&(base_keyboard_decoder){ 0 }, &(base_keyboard_record){ BASE_KEYBOARD_TRANSITION,
            0u, 0xffffu, 0u, 0u, 0u, 1, 1u }, &received) == BASE_KEYBOARD_UNMAPPED);
    return 0;
}
