#include "lib/types/test.h"
#include "lib/base/input_adapter_interface.h"
#include "lib/base/linux/input.h"

static base_key_input received;
int main(void)
{
    base_keyboard_decoder state = { 0 };
    static const base_key keys[] = {
        BASE_KEY_ENTER, BASE_KEY_BACKSPACE,
        BASE_KEY_F1, BASE_KEY_F2, BASE_KEY_F3, BASE_KEY_F4, BASE_KEY_F5, BASE_KEY_F6,
        BASE_KEY_F7, BASE_KEY_F8, BASE_KEY_F9, BASE_KEY_F10, BASE_KEY_F11, BASE_KEY_F12,
        BASE_KEY_UP, BASE_KEY_DOWN, BASE_KEY_LEFT, BASE_KEY_RIGHT,
        BASE_KEY_HOME, BASE_KEY_END, BASE_KEY_PAGE_UP, BASE_KEY_PAGE_DOWN,
        BASE_KEY_INSERT, BASE_KEY_DELETE
    };
    for (lib_u16 key = BASE_LINUX_KEY_ENTER; key <= BASE_LINUX_KEY_DELETE; ++key) {
        base_keyboard_record record = { .kind = BASE_KEYBOARD_TRANSITION,
            .key = key, .pressed = LIB_TRUE, .repeat_count = 3u,
            .modifiers = BASE_KEY_MODIFIER_SHIFT };
        lib_test_assert(base_keyboard_decode(&state, &record, &received) == BASE_KEYBOARD_ACCEPTED);
        lib_test_assert(received.key == (lib_u32)keys[key] && received.scan_code == 0u &&
            received.pressed && received.repeat_count == 3u &&
            received.modifiers == BASE_KEY_MODIFIER_SHIFT);
        record.pressed = LIB_FALSE;
        lib_test_assert(base_keyboard_decode(&state, &record, &received) == BASE_KEYBOARD_ACCEPTED);
        lib_test_assert(!received.pressed && received.repeat_count == 1u);
    }
    /* This terminal backend cannot invent physical identity for text. */
    received = (base_key_input){ .key = BASE_KEY_ENTER, .scan_code = 1u };
    lib_test_assert(!base_keyboard_map_scalar('a', &received));
    lib_test_assert(received.key == 0u && received.scan_code == 0u);
    lib_test_assert(base_keyboard_decode(&(base_keyboard_decoder){ 0 }, &(base_keyboard_record){ BASE_KEYBOARD_TRANSITION,
            0u, BASE_LINUX_KEY_UP, 0u, 0u, 0u, 1, 1u }, &received));
    lib_test_assert(received.key == BASE_KEY_UP &&
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
