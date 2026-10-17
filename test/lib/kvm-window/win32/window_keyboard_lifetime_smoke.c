#include "lib/types/test.h"
#include "lib/types/win32/test.h"
#include "lib/kvm-window/window.h"
#include "lib/types/win32/window.h"
#include "lib/kvm-window/win32/input.h"
/* Execute this message adapter without owning desktop focus. */
/* Modifiers are controlled, not read from the user's live keyboard. */
static lib_u8 fixture_modifiers(void) { return 0u; }
#define kvm_window_modifiers_from_key_state fixture_modifiers
static void *window_context;
static lib_u32 translations;
static lib_win32_bool LIB_WIN32_WINAPI translate_unmapped(const lib_win32_msg *message)
{ lib_test_assert(message->wParam == LIB_WIN32_KEY_PACKET); ++translations; return LIB_WIN32_TRUE; }
#undef lib_win32_translate_message
#define lib_win32_translate_message translate_unmapped
static lib_win32_long_ptr LIB_WIN32_WINAPI context_pointer(lib_win32_hwnd window, lib_i32 index)
{ (void)window; (void)index; return (lib_win32_long_ptr)window_context; }
#undef lib_win32_get_window_long_ptr_a
#define lib_win32_get_window_long_ptr_a context_pointer
#include "lib/kvm-window/win32/component.c"

#include "lib/kvm-base/input_interface.h"
/* Isolate the leaf's message lifetime from the host keyboard layout. */
static lib_bool fixture_scalar(lib_u32 scalar, lib_key_input *out)
{
    if (scalar!='a' && scalar!='A') return LIB_FALSE;
    *out=(lib_key_input){.key='A',.scan_code=0x1e,
        .modifiers=scalar=='A'?LIB_KEY_MODIFIER_SHIFT:0};
    return LIB_TRUE;
}
#define base_keyboard_map_scalar fixture_scalar
#include "lib/kvm-base/input.c"
#undef base_keyboard_map_scalar

typedef struct capture {
    kvm_input_event events[256];
    lib_u32 count, attempts, reject_at;
} capture;
static lib_bool capture_event(void *opaque, const kvm_input_event *event)
{
    capture *c = opaque;
    if (++c->attempts == c->reject_at) return LIB_FALSE;
    lib_test_assert(c->count < 256);
    if (event->type == KVM_EVENT_KEY) {
        lib_test_assert(event->data.key.key == 'A' || event->data.key.key == 'B');
        lib_test_assert(event->data.key.scan_code == (event->data.key.key == 'A' ? 0x1eu : 0x30u));
        lib_test_assert(event->data.key.flags == 0 && event->data.key.modifiers == 0);
    } else if (event->type == KVM_EVENT_TEXT) {
        lib_test_assert(event->data.text.scalar == 0x1f600 || event->data.text.scalar == 0x4e00);
    } else lib_test_assert(event->type == KVM_EVENT_INPUT_RESET);

    c->events[c->count] = *event;
    c->events[c->count].source = LIB_NULL;
    c->events[c->count++].source_identity = 0;
    return LIB_TRUE;
}
static void key_sequence(const capture *c, const char *keys, lib_u32 pressed)
{
    lib_test_assert(c->count == lib_text_length(keys));
    for (lib_u32 i = 0; i < c->count; ++i) {
        lib_test_assert(c->events[i].type == KVM_EVENT_KEY);
        lib_test_assert(c->events[i].data.key.key == (lib_u32)keys[i]);
        lib_test_assert(c->events[i].data.key.pressed == ((pressed >> i) & 1u));
    }
}

static void no_failure(void *opaque, lib_u64 source, lib_status status)
{ (void)opaque; (void)source; (void)status; }
static lib_status no_join(kvm_component *component, lib_u32 timeout_ms)
{ (void)component; (void)timeout_ms; return LIB_STATUS_OK; }
static void no_dispose(kvm_component *component)
{ kvm_component_mailboxes_destroy(&component->mailboxes); }
static void adapter_lifetime(lib_u32 scan)
{
    static kvm_window window;
    static kvm_win32_window_context context;
    lib_memory_set(&window, 0, sizeof(window));
    lib_memory_set(&context, 0, sizeof(context));
    capture w = {0};
    kvm_component_options options = { .input_sink = capture_event, .failure_sink = no_failure };
    options.input_context = &w;
    lib_test_assert(kvm_component_initialize(&window.base, &options, no_join, no_dispose,
        &window.pending_frame, sizeof(window.pending_frame)) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_select_notify(&window.base.mailboxes,
        LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    context.component = &window; window_context = &context;
    lib_win32_hwnd handle = (lib_win32_hwnd)1;
    lib_win32_lparam lp = (lib_win32_lparam)scan << 16;
    /* New source: the previous cooked reader consumed the make. */
    win32_window_proc(handle, LIB_WIN32_WM_KEYUP, 'A', lp);
    lib_test_assert(w.attempts == 0);
    /* Reset revokes the old make; its late break belongs to neither source. */
    win32_window_proc(handle, LIB_WIN32_WM_KEYDOWN, 'A', lp);
    lib_test_assert(kvm_component_reset_input(&window.base));
    lib_test_assert(w.count == 2);
    lib_test_assert(w.events[1].type == KVM_EVENT_INPUT_RESET);
    win32_window_proc(handle, LIB_WIN32_WM_KEYUP, 'A', lp);
    lib_test_assert(w.attempts == 2);
    w.count = w.attempts = 0;
    win32_window_proc(handle, LIB_WIN32_WM_KEYDOWN, 'A', lp);
    win32_window_proc(handle, LIB_WIN32_WM_KEYUP, 'A', lp);
    lib_test_assert(w.count == 2);
    key_sequence(&w, "AA", 1u);
    lib_test_assert(translations == 0);
    /* Physical keys with supplied PC scans emit no extra characters.
     * Scan-less RDP resolution is verified by Base.
     * Interleave releases to verify that no correlation credits can be lost. */
    win32_window_proc(handle, LIB_WIN32_WM_KEYDOWN, 'A', 0x1e0001);
    win32_window_proc(handle, LIB_WIN32_WM_KEYDOWN, 'B', 0x300001);
    win32_window_proc(handle, LIB_WIN32_WM_KEYUP, 'B', 0x300000);
    win32_window_proc(handle, LIB_WIN32_WM_KEYUP, 'A', 0x1e0000);
    key_sequence(&w, "AAABBA", 13u);
    lib_test_assert(w.count == 6 && translations == 0);
    win32_window_proc(handle, LIB_WIN32_WM_KEYDOWN, LIB_WIN32_KEY_PACKET, 0);
    lib_test_assert(translations == 1 && w.count == 6);
    translations = 0;
    w.count = w.attempts = 0;
    for (lib_u32 i = 0; i < 2; ++i) {
        lib_u32 unit = i ? 0xde00 : 0xd83d;
        win32_window_proc(handle, LIB_WIN32_WM_CHAR, unit, 0);
    }
    lib_test_assert(w.count == 1);
    lib_test_assert(w.events[0].type == KVM_EVENT_TEXT && w.events[0].data.text.scalar == 0x1f600);
    /* Adapters preserve counts; only the shared normalizer expands batches. */
    w.count = w.attempts = 0;
    win32_window_proc(handle, LIB_WIN32_WM_KEYDOWN, 'A', 0x1e0004);
    win32_window_proc(handle, LIB_WIN32_WM_KEYUP, 'A', 0x1e0001);
    lib_test_assert(w.count == 5);
    key_sequence(&w, "AAAAA", 15u);
    w.count = w.attempts = 0;
    win32_window_proc(handle, LIB_WIN32_WM_CHAR, 0x4e00, 3);
    lib_test_assert(w.count == 3);
    w.count = w.attempts = 0;
    win32_window_proc(handle, LIB_WIN32_WM_CHAR, 0x4e00, 1);
    /* Repetition and malformed-prefix recovery through the actual adapter.
     * Expected scalars use supplementary characters, independent of keyboard layout. */
    {
        static const struct {
            lib_u32 units[4], repeats[4], length, expected, scalar;
        } cases[] = {
            {{0xd83d,0xde00}, {2,2}, 2,2,0x1f600},
            {{0xd83d,0xde00,0xd83d,0xde00}, {1,1,1,1}, 4,2,0x1f600},
            {{0xd800,0xd83d,0xde00}, {1,3,3}, 3,3,0x1f600},
            {{0xdc00,0xd83d,0xde00}, {1,2,2}, 3,2,0x1f600},
            {{0xd83d,0xde00,0xd83d,0xde00}, {2,1,1,1}, 4,1,0x1f600},
            {{0xd83d,0xde00}, {0,0}, 2,1,0x1f600},
            {{0xd800,'a'}, {2,2}, 2,4,0} /* physical 'a' pairs after bad prefix */
        };
        for (lib_u32 n = 0; n < sizeof(cases)/sizeof(cases[0]); ++n) {
            w.count = w.attempts = 0;
            for (lib_u32 i = 0; i < cases[n].length; ++i) {
                win32_window_proc(handle, LIB_WIN32_WM_CHAR, cases[n].units[i], cases[n].repeats[i]);
            }
            lib_test_assert(w.count == cases[n].expected);
            for (lib_u32 i = 0; i < w.count; ++i) {
                if (cases[n].scalar)
                    lib_test_assert(w.events[i].type == KVM_EVENT_TEXT &&
                        w.events[i].data.text.scalar == cases[n].scalar);
                else
                    lib_test_assert(w.events[i].type == KVM_EVENT_KEY &&
                        w.events[i].data.key.key == 'A' &&
                        w.events[i].data.key.pressed == (i % 2 == 0));
            }
            lib_test_assert(context.keyboard_normalizer.pending_high_surrogate == 0);
        }
    }
    /* A physical record cancels an unfinished text prefix on this input. */
    w.count = w.attempts = 0;
    win32_window_proc(handle, LIB_WIN32_WM_CHAR, 0xd83d, 2);
    win32_window_proc(handle, LIB_WIN32_WM_KEYDOWN, 'A', 0x1e0001);
    win32_window_proc(handle, LIB_WIN32_WM_KEYUP, 'A', 0x1e0005);
    win32_window_proc(handle, LIB_WIN32_WM_CHAR, 0xde00, 2);
    lib_test_assert(w.count == 2);
    key_sequence(&w, "AA", 1u);
    w.count = w.attempts = 0;
    win32_window_proc(handle, LIB_WIN32_WM_CHAR, 0x4e00, 1);
    /* Malformed text does not poison the next complete pair. */
    win32_window_proc(handle, LIB_WIN32_WM_CHAR, 0xdc00, 0);
    lib_test_assert(w.count == 1);
    w.reject_at = 3;
    for (lib_u32 i = 0; i < 2; ++i) {
        lib_u32 unit = i ? 0xde00 : 0xd83d;
        win32_window_proc(handle, LIB_WIN32_WM_CHAR, unit, 4);
    }
    lib_test_assert(window.base.stopping);
    win32_window_proc(handle, LIB_WIN32_WM_KEYDOWN, 'B', 0x300001);
    lib_test_assert(w.attempts == 3 && w.count == 2);
    lib_test_assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);
}
static void repeat_delivery_failure(void)
{
    static kvm_window window;
    static kvm_win32_window_context context;
    lib_memory_set(&window, 0, sizeof(window));
    lib_memory_set(&context, 0, sizeof(context));
    capture c = { .reject_at = 3 };
    kvm_component_options options = { .input_context = &c,
        .input_sink = capture_event, .failure_sink = no_failure };
    lib_test_assert(kvm_component_initialize(&window.base, &options, no_join, no_dispose,
        &window.pending_frame, sizeof(window.pending_frame)) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_select_notify(&window.base.mailboxes,
        LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    context.component = &window; window_context = &context;
    win32_window_proc((lib_win32_hwnd)1, LIB_WIN32_WM_KEYDOWN, 'A', 0x1e0005);
    lib_test_assert(c.count == 2 && c.attempts == 3 && window.base.stopping);
    win32_window_proc((lib_win32_hwnd)1, LIB_WIN32_WM_KEYDOWN, 'B', 0x300001);
    lib_test_assert(c.attempts == 3);
    lib_test_assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);
}

int main(void)
{
    adapter_lifetime(0x1e);
    repeat_delivery_failure();
    return 0;
}
