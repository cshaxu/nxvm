#include "lib/types/test.h"
#include "lib/types/win32/test.h"
#include "lib/base/input_adapter_interface.h"

#ifdef _WIN32
#include "lib/types/win32/input.h"
static lib_win32_uint fixture_scan(lib_win32_uint key, lib_win32_uint kind)
{
    (void)kind;
    switch (key) {
    case 'A': return 0x1e; case 'B': return 0x30;
    case LIB_WIN32_KEY_RETURN: return 0x1c;
    case LIB_WIN32_KEY_CONTROL: return 0x1d;
    case LIB_WIN32_KEY_ALT: return 0x38;
    case LIB_WIN32_KEY_SHIFT: return 0x2a;
    default: return 0;
    }
}
static lib_win32_short fixture_scalar(lib_win32_wchar scalar)
{ return scalar == 'a' ? 'A' : scalar == 'A' ? 0x141 : -1; }
#undef lib_win32_map_virtual_key
#define lib_win32_map_virtual_key fixture_scan
#undef lib_win32_key_scan
#define lib_win32_key_scan fixture_scalar
#include "lib/base/win32/input.c"

int main(void)
{
    /* PC set-1 identities distinguish physical sides and keypad keys without
       requiring a consumer to interpret a native virtual-key value. */
    const struct {
        lib_u16 scan, native_key;
        lib_bool extended;
        base_key key;
    } physical[] = {
        { 0x2au, LIB_WIN32_KEY_LEFT_SHIFT, LIB_FALSE, BASE_KEY_SHIFT },
        { 0x36u, LIB_WIN32_KEY_RIGHT_SHIFT, LIB_FALSE, BASE_KEY_SHIFT },
        { 0x1du, LIB_WIN32_KEY_CONTROL, LIB_FALSE, BASE_KEY_CONTROL },
        { 0x1du, LIB_WIN32_KEY_CONTROL, LIB_TRUE, BASE_KEY_CONTROL },
        { 0x38u, LIB_WIN32_KEY_ALT, LIB_FALSE, BASE_KEY_ALT },
        { 0x38u, LIB_WIN32_KEY_ALT, LIB_TRUE, BASE_KEY_ALT },
        { 0x1cu, LIB_WIN32_KEY_RETURN, LIB_FALSE, BASE_KEY_ENTER },
        { 0x1cu, LIB_WIN32_KEY_RETURN, LIB_TRUE, BASE_KEY_ENTER },
        { 0x47u, LIB_WIN32_KEY_NUMPAD_0 + 7u, LIB_FALSE, BASE_KEY_KEYPAD_7 },
        { 0x47u, LIB_WIN32_KEY_HOME, LIB_TRUE, BASE_KEY_HOME },
        { 0x45u, LIB_WIN32_KEY_PAUSE, LIB_FALSE, BASE_KEY_PAUSE },
        { 0x37u, LIB_WIN32_KEY_SNAPSHOT, LIB_TRUE, BASE_KEY_PRINT_SCREEN }
    };
    for (lib_size i = 0; i < sizeof(physical) / sizeof(physical[0]); ++i) {
        base_keyboard_decoder decoder = { 0 };
        base_keyboard_record native = {
            .kind = BASE_KEYBOARD_COMBINED, .scan = physical[i].scan,
            .key = physical[i].native_key,
            .flags = physical[i].extended ? BASE_KEY_FLAG_EXTENDED : 0u,
            .pressed = LIB_TRUE, .repeat_count = 3u
        };
        base_key_input neutral;
        lib_test_assert(base_keyboard_decode(&decoder, &native, &neutral) ==
            BASE_KEYBOARD_ACCEPTED);
        lib_test_assert(neutral.key == physical[i].key && neutral.scalar == 0u &&
            neutral.scan_code == physical[i].scan &&
            neutral.extended == physical[i].extended && neutral.repeat_count == 3u);
        native.pressed = LIB_FALSE;
        lib_test_assert(base_keyboard_decode(&decoder, &native, &neutral) ==
            BASE_KEYBOARD_ACCEPTED);
        lib_test_assert(!neutral.pressed && neutral.repeat_count == 1u);
    }
    base_keyboard_decoder decoder = {0};
    base_key_input input;
    base_keyboard_record record = {.kind=BASE_KEYBOARD_TRANSITION,
        .key=LIB_WIN32_KEY_RETURN, .pressed=LIB_TRUE, .repeat_count=1};
    lib_test_assert(base_keyboard_decode(&decoder,&record,&input)==BASE_KEYBOARD_ACCEPTED);
    lib_test_assert(input.key==BASE_KEY_ENTER && input.scan_code==0x1c);
    record.pressed=LIB_FALSE;
    lib_test_assert(base_keyboard_decode(&decoder,&record,&input)==BASE_KEYBOARD_ACCEPTED && !input.pressed);
    record=(base_keyboard_record){.kind=BASE_KEYBOARD_TRANSITION,
        .scan=0xe04d,.key=LIB_WIN32_KEY_RIGHT,.flags=BASE_KEY_FLAG_EXTENDED,.pressed=LIB_TRUE};
    lib_test_assert(base_keyboard_decode(&decoder,&record,&input)==BASE_KEYBOARD_ACCEPTED);
    lib_test_assert(input.key==BASE_KEY_RIGHT && input.scan_code==0x4d && input.extended);
    record.scan=0x4d;
    lib_test_assert(base_keyboard_decode(&decoder,&record,&input)==BASE_KEYBOARD_ACCEPTED && input.scan_code==0x4d);
    record=(base_keyboard_record){.kind=BASE_KEYBOARD_TRANSITION,.key=LIB_WIN32_KEY_PACKET,.pressed=LIB_TRUE};
    lib_test_assert(base_keyboard_decode(&decoder,&record,&input)==BASE_KEYBOARD_UNMAPPED);
    record=(base_keyboard_record){.kind=BASE_KEYBOARD_CHARACTER,.utf16=0xd83d,.repeat_count=1};
    lib_test_assert(base_keyboard_decode(&decoder,&record,&input)==BASE_KEYBOARD_ACCEPTED && !input.scalar);
    record.utf16=0xde00;
    lib_test_assert(base_keyboard_decode(&decoder,&record,&input)==BASE_KEYBOARD_ACCEPTED && input.scalar==0x1f600);
    record.utf16=0xdc00;
    lib_test_assert(base_keyboard_decode(&decoder,&record,&input)==BASE_KEYBOARD_REJECTED);
    record.utf16=0xd800;
    for (lib_u32 i=0;i<3;++i) {
        lib_test_assert(base_keyboard_decode(&decoder,&record,&input)==BASE_KEYBOARD_ACCEPTED);
        lib_test_assert(decoder.pending_high_surrogate==0xd800 && !input.scalar);
    }
    record.utf16='a';
    lib_test_assert(base_keyboard_decode(&decoder,&record,&input)==BASE_KEYBOARD_ACCEPTED && input.scalar=='a');
    record.utf16=0x4e00;
    lib_test_assert(base_keyboard_decode(&decoder,&record,&input)==BASE_KEYBOARD_ACCEPTED && input.scalar==0x4e00);
    lib_test_assert(decoder.pending_high_surrogate==0);
    /* Repetition belongs to the complete scalar, not individual UTF-16 units. */
    static const struct {
        lib_u16 units[4], repeats[4];
        lib_u32 length, characters, repeats_total, scalar;
    } cases[] = {
        {{0xd83d,0xde00},{2,2},2,1,2,0x1f600},
        {{0xd83d,0xde00,0xd83d,0xde00},{1,1,1,1},4,2,2,0x1f600},
        {{0xd800,0xd83d,0xde00},{1,3,3},3,1,3,0x1f600},
        {{0xdc00,0xd83d,0xde00},{1,2,2},3,1,2,0x1f600},
        {{0xd83d,0xde00,0xd83d,0xde00},{2,1,1,1},4,1,1,0x1f600},
        {{0xd83d,0xde00},{0,0},2,1,1,0x1f600},
        {{0xd800,'a'},{2,2},2,1,2,'a'}
    };
    for(lib_size n=0;n<sizeof(cases)/sizeof(cases[0]);++n) {
        decoder=(base_keyboard_decoder){0};
        lib_u32 characters=0, repeats=0;
        for(lib_u32 i=0;i<cases[n].length;++i) {
            record=(base_keyboard_record){.kind=BASE_KEYBOARD_CHARACTER,
                .utf16=cases[n].units[i],.repeat_count=cases[n].repeats[i]};
            lib_i32 result=base_keyboard_decode(&decoder,&record,&input);
            if(result==BASE_KEYBOARD_ACCEPTED && input.scalar) {
                lib_test_assert(input.scalar==cases[n].scalar);
                ++characters; repeats+=input.repeat_count;
            }
        }
        lib_test_assert(characters==cases[n].characters && repeats==cases[n].repeats_total);
        lib_test_assert(decoder.pending_high_surrogate==0);
    }
    return 0;
}
#endif
