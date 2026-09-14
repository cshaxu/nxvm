#include "lib/kvm-base/hotkey_interface.h"
#include "lib/kvm-window/window.h"
#include "lib/kvm-console/console.h"
#include "lib/types/win32/window.h"
#include <assert.h>

static int allocation_failure;
static void *test_reallocate(void *memory, lib_size size)
{ return allocation_failure ? NULL : lib_reallocate(memory, size); }
#define lib_reallocate test_reallocate
#include "lib/kvm-base/hotkey.c"
#undef lib_reallocate

/* Execute both production message adapters without owning desktop focus. */
static void *window_context;
static unsigned translations;
static BOOL WINAPI translate_unmapped(const MSG *message)
{ assert(message->wParam == VK_PACKET); ++translations; return TRUE; }
#undef lib_win32_translate_message
#define lib_win32_translate_message translate_unmapped
static LONG_PTR WINAPI context_pointer(HWND window, int index)
{ (void)window; (void)index; return (LONG_PTR)window_context; }
#undef lib_win32_get_window_long_ptr_a
#define lib_win32_get_window_long_ptr_a context_pointer
#include "lib/kvm-window/win32/component.c"
#include "lib/kvm-console/win32/component.c"

typedef struct capture {
    kvm_input_event events[256];
    unsigned count, attempts, reject_at;
} capture;
static int capture_event(void *opaque, const kvm_input_event *event)
{
    capture *c = opaque;
    if (++c->attempts == c->reject_at) return 0;
    assert(c->count < 256);
    c->events[c->count] = *event;
    c->events[c->count].source = NULL;
    c->events[c->count++].source_identity = 0;
    return 1;
}
static int submit(kvm_hotkey_matcher *m, capture *c, kvm_key key, unsigned scan,
    unsigned flags, unsigned mods, int down)
{
    kvm_input_event e = { .type = KVM_EVENT_KEY };
    e.data.key.key = key; e.data.key.scan_code = scan;
    e.data.key.flags = flags; e.data.key.modifiers = mods; e.data.key.pressed = down;
    return kvm_hotkey_matcher_submit(m, &e, capture_event, c, LIB_TRUE);
}
static void permutations(void)
{
    const unsigned orders[6][3] = {{0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}};
    const kvm_key triggers[3] = {'P','D','F'};
    for (unsigned t = 0; t < 3; ++t)
    for (unsigned side = 0; side < 4; ++side)
    for (unsigned a = 0; a < 6; ++a)
    for (unsigned b = 0; b < 6; ++b) {
        kvm_key keys[3] = {KVM_KEY_CONTROL, KVM_KEY_ALT, triggers[t]};
        unsigned scans[3] = {0x1d, 0x38, 0x19};
        unsigned flags[3] = {side & 1, (side >> 1) & 1, 0};
        unsigned mods = 0, hotkeys = 0, down[3] = {0};
        kvm_hotkey_registry registry = {0};
        kvm_hotkey_matcher matcher;
        capture c = {0};
        assert(kvm_hotkey_registry_register(&registry, triggers[t], 3, "action") == LIB_STATUS_OK);
        kvm_hotkey_matcher_initialize(&matcher, &registry);
        for (unsigned n = 0; n < 3; ++n) {
            unsigned k = orders[a][n];
            if (k < 2) mods |= 1u << k;
            assert(submit(&matcher, &c, keys[k], scans[k], flags[k], mods, 1));
        }
        /* Repeat after modifiers arrive must not consume an earlier ordinary make. */
        for (unsigned n = 0; n < 16; ++n)
            assert(submit(&matcher, &c, keys[2], scans[2], 0, mods, 1));
        assert(matcher.held_count == 3);
        for (unsigned n = 0; n < 3; ++n) {
            unsigned k = orders[b][n];
            if (k < 2) mods &= ~(1u << k);
            assert(submit(&matcher, &c, keys[k], scans[k], flags[k], mods, 0));
        }
        for (unsigned n = 0; n < c.count; ++n) {
            kvm_input_event *e = &c.events[n];
            if (e->type == KVM_EVENT_HOTKEY) { ++hotkeys; continue; }
            assert(e->type == KVM_EVENT_KEY);
            unsigned k = e->data.key.key == keys[0] ? 0 : e->data.key.key == keys[1] ? 1 : 2;
            if (e->data.key.pressed) down[k] = 1;
            else { assert(down[k]); down[k] = 0; }
        }
        assert(!down[0] && !down[1] && !down[2] && matcher.held_count == 0);
        assert(hotkeys == (orders[a][2] == 2 ? 17u : 0u));
        kvm_hotkey_matcher_discard(&matcher);
    }
}
static void failure_paths(void)
{
    kvm_hotkey_registry registry = {0};
    kvm_hotkey_matcher matcher;
    capture c = {0};
    assert(kvm_hotkey_registry_register(&registry, 'P', 3, "action") == LIB_STATUS_OK);
    kvm_hotkey_matcher_initialize(&matcher, &registry);
    allocation_failure = 1;
    assert(!submit(&matcher, &c, 'A', 0x1e, 0, 0, 1));
    allocation_failure = 0;
    assert(!submit(&matcher, &c, 'A', 0x1e, 0, 0, 1));
    assert(c.count == 0);
    kvm_hotkey_matcher_discard(&matcher);
    assert(submit(&matcher, &c, KVM_KEY_CONTROL, 0x1d, 0, 1, 1));
    assert(submit(&matcher, &c, KVM_KEY_ALT, 0x38, 0, 3, 1));
    c.reject_at = 2;
    assert(!submit(&matcher, &c, 'A', 0x1e, 0, 3, 1));
    assert(!submit(&matcher, &c, 'B', 0x30, 0, 3, 1));
    assert(c.count == 1 && c.attempts == 2);
    kvm_hotkey_matcher_discard(&matcher);
}
static void no_failure(void *opaque, lib_u64 source, lib_status status)
{ (void)opaque; (void)source; (void)status; }
static lib_status no_join(kvm_component *component, lib_u32 timeout_ms)
{ (void)component; (void)timeout_ms; return LIB_STATUS_OK; }
static void no_dispose(kvm_component *component)
{ kvm_component_mailboxes_destroy(&component->mailboxes); }
static void console_record(kvm_console *console, unsigned key, unsigned scan,
    unsigned text, int down, unsigned repeat)
{
    lib_console_event e = { .kind = LIB_CONSOLE_EVENT_RAW_KEY };
    e.value.raw_key.key = key; e.value.raw_key.scan_code = scan;
    e.value.raw_key.unicode = text; e.value.raw_key.pressed = down;
    e.value.raw_key.repeat_count = repeat;
    kvm_console_receive_event(console, &e);
}
static void adapter_equivalence(unsigned scan)
{
    static kvm_window window;
    static kvm_console console;
    static kvm_win32_window_context context;
    lib_memory_set(&window, 0, sizeof(window));
    lib_memory_set(&console, 0, sizeof(console));
    lib_memory_set(&context, 0, sizeof(context));
    kvm_console_win32_state state = {0};
    capture w = {0}, c = {0};
    kvm_component_options options = { .input_sink = capture_event, .failure_sink = no_failure };
    options.input_context = &w;
    assert(kvm_component_initialize(&window.base, &options, no_join, no_dispose) == LIB_STATUS_OK);
    options.input_context = &c;
    assert(kvm_component_initialize(&console.base, &options, no_join, no_dispose) == LIB_STATUS_OK);
    context.component = &window; console.worker_state = &state; window_context = &context;
    HWND handle = (HWND)1;
    LPARAM lp = (LPARAM)scan << 16;
    win32_window_proc(handle, WM_KEYDOWN, 'A', lp);
    win32_window_proc(handle, WM_KEYUP, 'A', lp);
    console_record(&console, 'A', scan, 'a', 1, 0);
    console_record(&console, 'A', scan, 'a', 0, 0);
    assert(w.count == 2 && c.count == 2);
    assert(translations == 0);
    /* No characters are generated for physical keys, including scan-less RDP
     * packets interleaved with releases. No correlation credits can be lost. */
    win32_window_proc(handle, WM_KEYDOWN, 'A', 0);
    win32_window_proc(handle, WM_KEYDOWN, 'B', 0);
    win32_window_proc(handle, WM_KEYUP, 'B', 0x300000);
    win32_window_proc(handle, WM_KEYUP, 'A', 0x1e0000);
    console_record(&console, 'A', 0x1e, 'a', 1, 0);
    console_record(&console, 'B', 0x30, 'b', 1, 0);
    console_record(&console, 'B', 0x30, 'b', 0, 0);
    console_record(&console, 'A', 0x1e, 'a', 0, 0);
    assert(w.count == 6 && c.count == 6 && translations == 0);
    win32_window_proc(handle, WM_KEYDOWN, VK_PACKET, 0);
    assert(translations == 1 && w.count == 6);
    translations = 0;
    assert(lib_memory_compare(w.events, c.events, w.count * sizeof(w.events[0])) == 0);
    w.count = w.attempts = c.count = c.attempts = 0;
    for (unsigned i = 0; i < 2; ++i) {
        unsigned unit = i ? 0xde00 : 0xd83d;
        win32_window_proc(handle, WM_CHAR, unit, 0);
        console_record(&console, 0, 0, unit, 1, 0);
        console_record(&console, 0, 0, unit, 0, 0);
    }
    assert(w.count == 1 && c.count == 1);
    assert(w.events[0].type == KVM_EVENT_TEXT && w.events[0].data.text.scalar == 0x1f600);
    assert(lib_memory_compare(w.events, c.events, w.count * sizeof(w.events[0])) == 0);
    /* Adapters preserve counts; only the shared normalizer expands batches. */
    w.count = w.attempts = c.count = c.attempts = 0;
    win32_window_proc(handle, WM_KEYDOWN, 'A', 0x1e0004);
    win32_window_proc(handle, WM_KEYUP, 'A', 0x1e0001);
    console_record(&console, 'A', 0x1e, 'a', 1, 4);
    console_record(&console, 'A', 0x1e, 'a', 0, 0);
    assert(w.count == 5 && c.count == 5);
    assert(lib_memory_compare(w.events, c.events, w.count * sizeof(w.events[0])) == 0);
    w.count = w.attempts = c.count = c.attempts = 0;
    win32_window_proc(handle, WM_CHAR, 0x4e00, 3);
    console_record(&console, 0, 0, 0x4e00, 1, 3);
    assert(w.count == 3 && c.count == 3);
    assert(lib_memory_compare(w.events, c.events, w.count * sizeof(w.events[0])) == 0);
    w.count = w.attempts = c.count = c.attempts = 0;
    win32_window_proc(handle, WM_CHAR, 0x4e00, 1);
    console_record(&console, 0, 0, 0x4e00, 1, 0);
    /* Repetition and malformed-prefix recovery through BOTH actual adapters.
     * Expected scalars use supplementary characters, independent of keyboard layout. */
    {
        static const struct {
            unsigned units[4], repeats[4], length, expected, scalar;
        } cases[] = {
            {{0xd83d,0xde00}, {2,2}, 2,2,0x1f600},
            {{0xd83d,0xde00,0xd83d,0xde00}, {1,1,1,1}, 4,2,0x1f600},
            {{0xd800,0xd83d,0xde00}, {1,3,3}, 3,3,0x1f600},
            {{0xdc00,0xd83d,0xde00}, {1,2,2}, 3,2,0x1f600},
            {{0xd83d,0xde00,0xd83d,0xde00}, {2,1,1,1}, 4,1,0x1f600},
            {{0xd83d,0xde00}, {0,0}, 2,1,0x1f600},
            {{0xd800,'a'}, {2,2}, 2,4,0} /* physical 'a' pairs after bad prefix */
        };
        for (unsigned n = 0; n < sizeof(cases)/sizeof(cases[0]); ++n) {
            w.count = w.attempts = c.count = c.attempts = 0;
            for (unsigned i = 0; i < cases[n].length; ++i) {
                win32_window_proc(handle, WM_CHAR, cases[n].units[i], cases[n].repeats[i]);
                console_record(&console, 0, 0, cases[n].units[i], 1, cases[n].repeats[i]);
                console_record(&console, 0, 0, cases[n].units[i], 0, cases[n].repeats[i]);
            }
            assert(w.count == cases[n].expected && c.count == w.count);
            assert(lib_memory_compare(w.events, c.events, w.count * sizeof(w.events[0])) == 0);
            for (unsigned i = 0; i < w.count; ++i) {
                if (cases[n].scalar)
                    assert(w.events[i].type == KVM_EVENT_TEXT &&
                        w.events[i].data.text.scalar == cases[n].scalar);
                else
                    assert(w.events[i].type == KVM_EVENT_KEY &&
                        w.events[i].data.key.key == 'A' &&
                        w.events[i].data.key.pressed == (i % 2 == 0));
            }
            assert(context.keyboard_normalizer.pending_high_surrogate == 0 &&
                state.keyboard.pending_high_surrogate == 0);
        }
    }
    /* A physical record cancels an unfinished text prefix on both inputs. */
    w.count = w.attempts = c.count = c.attempts = 0;
    win32_window_proc(handle, WM_CHAR, 0xd83d, 2);
    console_record(&console, 0, 0, 0xd83d, 1, 2);
    win32_window_proc(handle, WM_KEYDOWN, 'A', 0x1e0001);
    console_record(&console, 'A', 0x1e, 'a', 1, 1);
    win32_window_proc(handle, WM_KEYUP, 'A', 0x1e0005);
    console_record(&console, 'A', 0x1e, 'a', 0, 5);
    win32_window_proc(handle, WM_CHAR, 0xde00, 2);
    console_record(&console, 0, 0, 0xde00, 1, 2);
    assert(w.count == 2 && c.count == 2);
    assert(lib_memory_compare(w.events, c.events, w.count * sizeof(w.events[0])) == 0);
    w.count = w.attempts = c.count = c.attempts = 0;
    win32_window_proc(handle, WM_CHAR, 0x4e00, 1);
    console_record(&console, 0, 0, 0x4e00, 1, 1);
    /* Malformed text does not poison the next complete pair. */
    win32_window_proc(handle, WM_CHAR, 0xdc00, 0);
    console_record(&console, 0, 0, 0xdc00, 1, 0);
    assert(w.count == 1 && c.count == 1);
    w.reject_at = c.reject_at = 3;
    for (unsigned i = 0; i < 2; ++i) {
        unsigned unit = i ? 0xde00 : 0xd83d;
        win32_window_proc(handle, WM_CHAR, unit, 4);
        console_record(&console, 0, 0, unit, 1, 4);
    }
    assert(window.base.stopping && console.base.stopping);
    win32_window_proc(handle, WM_KEYDOWN, 'B', 0);
    console_record(&console, 'B', 0, 'b', 1, 0);
    assert(w.attempts == 3 && c.attempts == 3 && w.count == 2 && c.count == 2);
    assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);
    assert(kvm_component_destroy(&console.base) == LIB_STATUS_OK);
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
    assert(kvm_component_initialize(&window.base, &options, no_join, no_dispose) == LIB_STATUS_OK);
    context.component = &window; window_context = &context;
    win32_window_proc((HWND)1, WM_KEYDOWN, 'A', 0x1e0005);
    assert(c.count == 2 && c.attempts == 3 && window.base.stopping);
    win32_window_proc((HWND)1, WM_KEYDOWN, 'B', 0x300001);
    assert(c.attempts == 3);
    assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);
}

static int match_normalized(void *opaque, const kvm_input_event *event)
{
    void **pair = opaque;
    return kvm_hotkey_matcher_submit(pair[0], event, capture_event, pair[1], LIB_TRUE);
}
static void synthesis_lifetimes(void)
{
    /* Reuse the one physical ledger for both sides of all modifiers and an
     * already-held trigger. Synthesis must release only keys it introduces. */
    for (unsigned mask = 0; mask < 8; ++mask)
    for (unsigned side = 0; side < 8; ++side)
    for (unsigned trigger = 0; trigger < 2; ++trigger) {
        kvm_hotkey_matcher m;
        kvm_keyboard_normalizer n = {0};
        capture c = {0};
        void *pair[] = { &m, &c };
        const kvm_key keys[] = { KVM_KEY_CONTROL, KVM_KEY_ALT, KVM_KEY_SHIFT, 'A' };
        const unsigned scans[] = {0x1d, 0x38, side & 4 ? 0x36 : 0x2a, 0x1e};
        unsigned flags[] = {side & 1, (side >> 1) & 1, 0, 0};
        unsigned held = 0;
        kvm_hotkey_matcher_initialize(&m, NULL);
        for (unsigned i = 0; i < 4; ++i)
            if (i == 3 ? trigger : (mask & (1u << i))) {
                assert(submit(&m, &c, keys[i], scans[i], flags[i], mask, 1));
                ++held;
            }
        unsigned first = c.count;
        kvm_keyboard_record text = { .kind = KVM_KEYBOARD_CHARACTER, .utf16 = 'A', .pressed = 1 };
        for (unsigned repeat = 0; repeat < 3; ++repeat)
            assert(kvm_keyboard_submit_record(&n, &m, pair, match_normalized, &text));
        assert(m.held_count == held);
        for (unsigned j = first; j < c.count; ++j)
            if (c.events[j].type == KVM_EVENT_KEY && !c.events[j].data.key.pressed)
                for (unsigned i = 0; i < 4; ++i)
                    if (i == 3 ? trigger : (mask & (1u << i)))
                        assert(c.events[j].data.key.key != keys[i]);
        for (unsigned i = 0; i < 4; ++i)
            if (i == 3 ? trigger : (mask & (1u << i)))
                assert(submit(&m, &c, keys[i], scans[i], flags[i], 0, 0));
        assert(m.held_count == 0);
        kvm_hotkey_matcher_discard(&m);
    }
    for (unsigned reject = 1; reject <= 4; ++reject) {
        kvm_hotkey_matcher m;
        kvm_keyboard_normalizer n = {0};
        capture c = { .reject_at = reject };
        void *pair[] = { &m, &c };
        kvm_hotkey_matcher_initialize(&m, NULL);
        kvm_keyboard_record text = { .kind = KVM_KEYBOARD_CHARACTER, .utf16 = 'A', .pressed = 1 };
        assert(!kvm_keyboard_submit_record(&n, &m, pair, match_normalized, &text));
        assert(m.failed && c.attempts == reject);
        assert(!kvm_keyboard_submit_record(&n, &m, pair, match_normalized, &text));
        assert(c.attempts == reject);
        kvm_hotkey_matcher_discard(&m);
    }
}

static void physical_identity(void)
{
    kvm_hotkey_matcher m;
    capture c = {0};
    kvm_hotkey_matcher_initialize(&m, NULL);
    assert(submit(&m, &c, KVM_KEY_KEYPAD_7, 0x47, 0, 0, 1));
    assert(submit(&m, &c, KVM_KEY_HOME, 0x47, 0, 0, 1));
    assert(m.held_count == 1 && c.events[1].data.key.key == KVM_KEY_KEYPAD_7);
    assert(submit(&m, &c, KVM_KEY_HOME, 0x47, 0, 0, 0));
    assert(m.held_count == 0 && c.count == 3);
    assert(c.events[2].data.key.key == KVM_KEY_KEYPAD_7 && !c.events[2].data.key.pressed);
    assert(submit(&m, &c, KVM_KEY_HOME, 0x47, KVM_KEY_FLAG_EXTENDED, 0, 1));
    assert(submit(&m, &c, KVM_KEY_KEYPAD_7, 0x47, 0, 0, 1));
    assert(m.held_count == 2);
    assert(submit(&m, &c, KVM_KEY_HOME, 0x47, 0, 0, 0));
    assert(submit(&m, &c, KVM_KEY_HOME, 0x47, KVM_KEY_FLAG_EXTENDED, 0, 0));
    assert(m.held_count == 0);
    assert(submit(&m, &c, 'A', 0, 0, 0, 1));
    assert(submit(&m, &c, 'B', 0, 0, 0, 1));
    assert(submit(&m, &c, 'A', 0x1e, 0, 0, 1));
    assert(m.held_count == 3);
    assert(submit(&m, &c, 'A', 0, 0, 0, 0));
    assert(submit(&m, &c, 'B', 0, 0, 0, 0));
    assert(submit(&m, &c, 'A', 0x1e, 0, 0, 0));
    assert(m.held_count == 0);
    kvm_hotkey_matcher_discard(&m);
}

int main(void)
{
    physical_identity();
    permutations();
    failure_paths();
    synthesis_lifetimes();
    repeat_delivery_failure();
    adapter_equivalence(0);
    adapter_equivalence(0x1e);
    return 0;
}
