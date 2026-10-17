#include "lib/types/test.h"
#include "lib/kvm-base/hotkey_interface.h"

static lib_bool allocation_failure;
static void *test_reallocate(void *memory, lib_size size)
{ return allocation_failure ? LIB_NULL : lib_reallocate(memory, size); }
#define lib_reallocate test_reallocate
#include "lib/kvm-base/hotkey.c"
#undef lib_reallocate

#include "lib/kvm-base/input_interface.h"
/* Deterministic layout: synthesis tests must not depend on the host session. */
static lib_bool map_test_scalar(lib_u32 scalar, base_key_input *out)
{
    if (scalar != 'a' && scalar != 'A') return LIB_FALSE;
    *out = (base_key_input){.key = 'A', .scan_code = 0x1e,
        .modifiers = scalar == 'A' ? BASE_KEY_MODIFIER_SHIFT : 0};
    return LIB_TRUE;
}
#define base_keyboard_map_scalar map_test_scalar
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
    c->events[c->count] = *event;
    c->events[c->count].source = LIB_NULL;
    c->events[c->count++].source_identity = 0;
    return LIB_TRUE;
}
static lib_bool submit(kvm_hotkey_matcher *m, capture *c, base_key key, lib_u32 scan,
    lib_u32 flags, lib_u32 mods, lib_bool down)
{
    kvm_input_event e = { .type = KVM_EVENT_KEY };
    e.data.key.key = key; e.data.key.scan_code = scan;
    e.data.key.flags = flags; e.data.key.modifiers = mods; e.data.key.pressed = down;
    return kvm_hotkey_matcher_submit(m, &e, capture_event, c, LIB_TRUE);
}
static void permutations(void)
{
    const lib_u32 orders[6][3] = {{0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}};
    const base_key triggers[3] = {'P','D','F'};
    for (lib_u32 t = 0; t < 3; ++t)
    for (lib_u32 side = 0; side < 4; ++side)
    for (lib_u32 a = 0; a < 6; ++a)
    for (lib_u32 b = 0; b < 6; ++b) {
        base_key keys[3] = {BASE_KEY_CONTROL, BASE_KEY_ALT, triggers[t]};
        lib_u32 scans[3] = {0x1d, 0x38, 0x19};
        lib_u32 flags[3] = {side & 1, (side >> 1) & 1, 0};
        lib_u32 mods = 0, hotkeys = 0, down[3] = {0};
        kvm_hotkey_registry registry = {0};
        kvm_hotkey_matcher matcher;
        capture c = {0};
        lib_test_assert(kvm_hotkey_registry_register(&registry, triggers[t], 3, "action") == LIB_STATUS_OK);
        kvm_hotkey_matcher_initialize(&matcher, &registry);
        for (lib_u32 n = 0; n < 3; ++n) {
            lib_u32 k = orders[a][n];
            if (k < 2) mods |= 1u << k;
            lib_test_assert(submit(&matcher, &c, keys[k], scans[k], flags[k], mods, 1));
        }
        /* Repeat after modifiers arrive must not consume an earlier ordinary make. */
        for (lib_u32 n = 0; n < 16; ++n)
            lib_test_assert(submit(&matcher, &c, keys[2], scans[2], 0, mods, 1));
        lib_test_assert(matcher.held_count == 3);
        for (lib_u32 n = 0; n < 3; ++n) {
            lib_u32 k = orders[b][n];
            if (k < 2) mods &= ~(1u << k);
            lib_test_assert(submit(&matcher, &c, keys[k], scans[k], flags[k], mods, 0));
        }
        for (lib_u32 n = 0; n < c.count; ++n) {
            kvm_input_event *e = &c.events[n];
            if (e->type == KVM_EVENT_HOTKEY) { ++hotkeys; continue; }
            lib_test_assert(e->type == KVM_EVENT_KEY);
            lib_u32 k = e->data.key.key == keys[0] ? 0 : e->data.key.key == keys[1] ? 1 : 2;
            if (e->data.key.pressed) down[k] = 1;
            else { lib_test_assert(down[k]); down[k] = 0; }
        }
        lib_test_assert(!down[0] && !down[1] && !down[2] && matcher.held_count == 0);
        lib_test_assert(hotkeys == (orders[a][2] == 2 ? 17u : 0u));
        kvm_hotkey_matcher_discard(&matcher);
    }
}
static void failure_paths(void)
{
    kvm_hotkey_registry registry = {0};
    kvm_hotkey_matcher matcher;
    capture c = {0};
    lib_test_assert(kvm_hotkey_registry_register(&registry, 'P', 3, "action") == LIB_STATUS_OK);
    kvm_hotkey_matcher_initialize(&matcher, &registry);
    allocation_failure = LIB_TRUE;
    lib_test_assert(!submit(&matcher, &c, 'A', 0x1e, 0, 0, 1));
    allocation_failure = LIB_FALSE;
    lib_test_assert(!submit(&matcher, &c, 'A', 0x1e, 0, 0, 1));
    lib_test_assert(c.count == 0);
    kvm_hotkey_matcher_discard(&matcher);
    lib_test_assert(submit(&matcher, &c, BASE_KEY_CONTROL, 0x1d, 0, 1, 1));
    lib_test_assert(submit(&matcher, &c, BASE_KEY_ALT, 0x38, 0, 3, 1));
    c.reject_at = 2;
    lib_test_assert(!submit(&matcher, &c, 'A', 0x1e, 0, 3, 1));
    lib_test_assert(!submit(&matcher, &c, 'B', 0x30, 0, 3, 1));
    lib_test_assert(c.count == 1 && c.attempts == 2);
    kvm_hotkey_matcher_discard(&matcher);
}
static void orphan_releases(void)
{
    kvm_hotkey_registry registry = {0};
    kvm_hotkey_matcher matcher;
    capture c = {0};
    lib_test_assert(kvm_hotkey_registry_register(&registry, 'P', 3, "action") == LIB_STATUS_OK);
    kvm_hotkey_matcher_initialize(&matcher, &registry);
    lib_test_assert(submit(&matcher, &c, BASE_KEY_ENTER, 0x1c, 0, 0, LIB_FALSE));
    lib_test_assert(c.attempts == 0 && matcher.held_count == 0);
    lib_test_assert(submit(&matcher, &c, BASE_KEY_CONTROL, 0x1d, 0, 1, LIB_TRUE));
    lib_test_assert(submit(&matcher, &c, BASE_KEY_ENTER, 0x1c, 0, 1, LIB_FALSE));
    lib_test_assert(c.attempts == 0 && matcher.held_count == 1);
    lib_test_assert(submit(&matcher, &c, BASE_KEY_CONTROL, 0x1d, 0, 0, LIB_FALSE));
    lib_test_assert(c.count == 2 && matcher.held_count == 0);
    lib_test_assert(submit(&matcher, &c, 'A', 0x1e, 0, 0, LIB_TRUE));
    kvm_hotkey_matcher_discard(&matcher);
    lib_u32 attempts = c.attempts;
    lib_test_assert(submit(&matcher, &c, 'A', 0x1e, 0, 0, LIB_FALSE));
    lib_test_assert(c.attempts == attempts && matcher.held_count == 0);
    kvm_hotkey_matcher_discard(&matcher);
}
static lib_bool match_normalized(void *opaque, const kvm_input_event *event)
{
    void **pair = opaque;
    return kvm_hotkey_matcher_submit(pair[0], event, capture_event, pair[1], LIB_TRUE);
}
static void synthesis_lifetimes(void)
{
    /* Reuse the one physical ledger for both sides of all modifiers and an
     * already-held trigger. Synthesis must release only keys it introduces. */
    for (lib_u32 mask = 0; mask < 8; ++mask)
    for (lib_u32 side = 0; side < 8; ++side)
    for (lib_u32 trigger = 0; trigger < 2; ++trigger) {
        kvm_hotkey_matcher m;
        base_keyboard_decoder n = {0};
        capture c = {0};
        void *pair[] = { &m, &c };
        const base_key keys[] = { BASE_KEY_CONTROL, BASE_KEY_ALT, BASE_KEY_SHIFT, 'A' };
        const lib_u32 scans[] = {0x1d, 0x38, side & 4 ? 0x36 : 0x2a, 0x1e};
        lib_u32 flags[] = {side & 1, (side >> 1) & 1, 0, 0};
        lib_u32 held = 0;
        kvm_hotkey_matcher_initialize(&m, LIB_NULL);
        for (lib_u32 i = 0; i < 4; ++i)
            if (i == 3 ? trigger : (mask & (1u << i))) {
                lib_test_assert(submit(&m, &c, keys[i], scans[i], flags[i], mask, 1));
                ++held;
            }
        lib_u32 first = c.count;
        base_keyboard_record text = { .kind = BASE_KEYBOARD_CHARACTER, .utf16 = 'A', .pressed = LIB_TRUE };
        for (lib_u32 repeat = 0; repeat < 3; ++repeat)
            lib_test_assert(kvm_keyboard_submit_record(&n, &m, pair, match_normalized, &text));
        lib_test_assert(m.held_count == held);
        for (lib_u32 j = first; j < c.count; ++j)
            if (c.events[j].type == KVM_EVENT_KEY && !c.events[j].data.key.pressed)
                for (lib_u32 i = 0; i < 4; ++i)
                    if (i == 3 ? trigger : (mask & (1u << i)))
                        lib_test_assert(c.events[j].data.key.key != keys[i]);
        for (lib_u32 i = 0; i < 4; ++i)
            if (i == 3 ? trigger : (mask & (1u << i)))
                lib_test_assert(submit(&m, &c, keys[i], scans[i], flags[i], 0, 0));
        lib_test_assert(m.held_count == 0);
        kvm_hotkey_matcher_discard(&m);
    }
    for (lib_u32 reject = 1; reject <= 4; ++reject) {
        kvm_hotkey_matcher m;
        base_keyboard_decoder n = {0};
        capture c = { .reject_at = reject };
        void *pair[] = { &m, &c };
        kvm_hotkey_matcher_initialize(&m, LIB_NULL);
        base_keyboard_record text = { .kind = BASE_KEYBOARD_CHARACTER, .utf16 = 'A', .pressed = LIB_TRUE };
        lib_test_assert(!kvm_keyboard_submit_record(&n, &m, pair, match_normalized, &text));
        lib_test_assert(m.failed && c.attempts == reject);
        lib_test_assert(!kvm_keyboard_submit_record(&n, &m, pair, match_normalized, &text));
        lib_test_assert(c.attempts == reject);
        kvm_hotkey_matcher_discard(&m);
    }
}

static void physical_identity(void)
{
    kvm_hotkey_matcher m;
    capture c = {0};
    kvm_hotkey_matcher_initialize(&m, LIB_NULL);
    lib_test_assert(submit(&m, &c, BASE_KEY_KEYPAD_7, 0x47, 0, 0, 1));
    lib_test_assert(submit(&m, &c, BASE_KEY_HOME, 0x47, 0, 0, 1));
    lib_test_assert(m.held_count == 1 && c.events[1].data.key.key == BASE_KEY_KEYPAD_7);
    lib_test_assert(submit(&m, &c, BASE_KEY_HOME, 0x47, 0, 0, 0));
    lib_test_assert(m.held_count == 0 && c.count == 3);
    lib_test_assert(c.events[2].data.key.key == BASE_KEY_KEYPAD_7 && !c.events[2].data.key.pressed);
    lib_test_assert(submit(&m, &c, BASE_KEY_HOME, 0x47, BASE_KEY_FLAG_EXTENDED, 0, 1));
    lib_test_assert(submit(&m, &c, BASE_KEY_KEYPAD_7, 0x47, 0, 0, 1));
    lib_test_assert(m.held_count == 2);
    lib_test_assert(submit(&m, &c, BASE_KEY_HOME, 0x47, 0, 0, 0));
    lib_test_assert(submit(&m, &c, BASE_KEY_HOME, 0x47, BASE_KEY_FLAG_EXTENDED, 0, 0));
    lib_test_assert(m.held_count == 0);
    lib_test_assert(submit(&m, &c, 'A', 0, 0, 0, 1));
    lib_test_assert(submit(&m, &c, 'B', 0, 0, 0, 1));
    lib_test_assert(submit(&m, &c, 'A', 0x1e, 0, 0, 1));
    lib_test_assert(m.held_count == 3);
    lib_test_assert(submit(&m, &c, 'A', 0, 0, 0, 0));
    lib_test_assert(submit(&m, &c, 'B', 0, 0, 0, 0));
    lib_test_assert(submit(&m, &c, 'A', 0x1e, 0, 0, 0));
    lib_test_assert(m.held_count == 0);
    kvm_hotkey_matcher_discard(&m);
}

int main(void)
{
    orphan_releases(); physical_identity(); permutations();
    failure_paths(); synthesis_lifetimes();
    return 0;
}
