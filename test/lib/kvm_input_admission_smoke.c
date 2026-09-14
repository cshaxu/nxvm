#include "lib/kvm-window/window.h"
#include "lib/types/win32/window.h"
/* This matcher fixture has no native Window; timer lifecycle is verified by
 * the capture/retirement/modal fixtures, not by creating thread timers here. */
static UINT_PTR WINAPI test_set_timer(HWND window, UINT_PTR id, UINT interval,
    TIMERPROC callback)
{ (void)window; (void)interval; (void)callback; return id; }
static BOOL WINAPI test_kill_timer(HWND window, UINT_PTR id)
{ (void)window; (void)id; return TRUE; }
#undef lib_win32_set_timer
#undef lib_win32_kill_timer
#define lib_win32_set_timer test_set_timer
#define lib_win32_kill_timer test_kill_timer
#include "lib/kvm-window/win32/component.c"
#include <assert.h>

static kvm_window window;
static kvm_win32_window_context context;
static kvm_frame frame;
static kvm_input_event delivered[32];
static unsigned count, attempts, reject_at, failures;
static int sink(void *opaque, const kvm_input_event *event)
{
    (void)opaque;
    if (++attempts == reject_at) return 0;
    assert(count < 32);
    delivered[count++] = *event;
    return 1;
}
static void failure(void *opaque, lib_u64 identity, lib_status status)
{ (void)opaque; assert(identity && status == LIB_STATUS_IO_ERROR); ++failures; }
static lib_status join(kvm_component *component, lib_u32 timeout_ms)
{ (void)component; (void)timeout_ms; return LIB_STATUS_OK; }
static void dispose(kvm_component *component)
{ kvm_component_mailboxes_destroy(&component->mailboxes); }
static void initialize(void)
{
    kvm_component_options options = { 0 };
    options.input_sink = sink; options.failure_sink = failure;
    assert(kvm_hotkey_registry_register(&options.hotkeys, 'P',
        KVM_HOTKEY_MODIFIER_CONTROL | KVM_HOTKEY_MODIFIER_ALT, "toggle") == LIB_STATUS_OK);
    assert(kvm_component_initialize(&window.base, &options, join, dispose) == LIB_STATUS_OK);
    lib_memory_set(&context, 0, sizeof(context));
    context.component = &window;
    count = attempts = reject_at = failures = 0;
    frame.valid = 1; frame.text_columns = 80; frame.text_rows = 25;
}
static int key(kvm_key key, lib_u16 scan, int down, lib_u8 modifiers)
{
    kvm_input_event event = { 0 };
    event.type = KVM_EVENT_KEY;
    event.data.key.key = key; event.data.key.scan_code = scan;
    event.data.key.pressed = down; event.data.key.modifiers = modifiers;
    return win32_window_emit_normalized(&context, &event);
}
static void set_frozen(lib_bool frozen)
{
    kvm_component_control control = { .kind = KVM_COMPONENT_CONTROL_SET_WINDOW_FROZEN };
    control.value.window_frozen = frozen;
    assert(kvm_component_enqueue_controls(&window.base, &control, 1) == LIB_STATUS_OK);
    assert(win32_window_consume_mailboxes(NULL, &context));
    assert(context.frozen == frozen);
}
static void frozen_prefix_replay(void)
{
    /* All ordinary flush causes share the same permission captured at make.
     * Repeats after unfreeze must not upgrade a frozen-origin prefix. */
    for (unsigned frozen = 0; frozen < 2; ++frozen)
    for (unsigned cause = 0; cause < 3; ++cause) {
        kvm_input_event text = { .type = KVM_EVENT_TEXT };
        text.data.text.scalar = 0x1f600;
        initialize();
        set_frozen(frozen);
        assert(key(KVM_KEY_CONTROL, 0x1d, 1, 1));
        set_frozen(!frozen);
        set_frozen(LIB_FALSE);
        assert(key(KVM_KEY_CONTROL, 0x1d, 1, 1));
        assert(window.base.hotkey_matcher.held_count == 1 && count == 0);
        if (cause == 0) assert(key('A', 0x1e, 1, 1));
        else if (cause == 1) assert(key(KVM_KEY_CONTROL, 0x1d, 0, 0));
        else assert(win32_window_emit_normalized(&context, &text));
        assert(count == (frozen ? 1u : 2u));
        if (!frozen)
            assert(delivered[0].type == KVM_EVENT_KEY &&
                delivered[0].data.key.key == KVM_KEY_CONTROL &&
                delivered[0].data.key.pressed);
        if (cause == 0) assert(delivered[count - 1].data.key.key == 'A');
        if (cause == 1) assert(!delivered[count - 1].data.key.pressed);
        if (cause == 2) assert(delivered[count - 1].type == KVM_EVENT_TEXT);
        assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);
    }
    /* Neither freezing direction may disable an otherwise matched hotkey. */
    for (unsigned frozen = 0; frozen < 2; ++frozen) {
        initialize();
        set_frozen(frozen);
        assert(key(KVM_KEY_CONTROL, 0x1d, 1, 1));
        set_frozen(!frozen);
        assert(key(KVM_KEY_ALT, 0x38, 1, 3));
        assert(key('P', 0x19, 1, 3));
        assert(count == 1 && delivered[0].type == KVM_EVENT_HOTKEY);
        assert(key('P', 0x19, 0, 3));
        assert(key(KVM_KEY_ALT, 0x38, 0, 1));
        assert(key(KVM_KEY_CONTROL, 0x1d, 0, 0));
        assert(count == 1);
        assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);
    }
    /* Owner explicitly retains per-event filtering, not make/break balancing. */
    for (unsigned frozen = 0; frozen < 2; ++frozen) {
        initialize();
        set_frozen(frozen);
        assert(key('A', 0x1e, 1, 0));
        set_frozen(!frozen);
        assert(key('A', 0x1e, 0, 0));
        assert(count == 1 && delivered[0].data.key.key == 'A' &&
            delivered[0].data.key.pressed == !frozen);
        assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);
    }
    initialize();
    set_frozen(LIB_TRUE);
    assert(key(KVM_KEY_CONTROL, 0x1d, 1, 1));
    set_frozen(LIB_FALSE);
    reject_at = 1;
    assert(!key('A', 0x1e, 1, 1));
    assert(count == 0 && attempts == 1 && window.base.stopping);
    assert(!key('B', 0x30, 1, 0) && attempts == 1);
    assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);
}
static HANDLE start_race;
static lib_status publish_status;
static DWORD WINAPI publish(void *unused)
{
    (void)unused;
    assert(WaitForSingleObject(start_race, 5000) == WAIT_OBJECT_0);
    publish_status = kvm_window_publish_frame(&window, &frame);
    return 0;
}
static DWORD WINAPI enqueue_title(void *unused)
{
    (void)unused;
    assert(kvm_window_set_title(&window, "independent") == LIB_STATUS_OK);
    return 0;
}
int main(void)
{
    kvm_input_event event = { .type = KVM_EVENT_WINDOW_CLOSE };
    kvm_component_control taken;
    frozen_prefix_replay();
    initialize();
    context.frozen = LIB_TRUE;
    assert(key(KVM_KEY_CONTROL, 0x1d, 1, 1));
    assert(win32_window_emit_normalized(&context, &event));
    assert(count == 1 && delivered[0].type == KVM_EVENT_WINDOW_CLOSE);
    assert(key(KVM_KEY_CONTROL, 0x1d, 0, 0));
    assert(count == 1); /* mismatch replay also uses the frozen filter */
    win32_window_emit_mouse(&context, 1, 1, 0);
    assert(count == 1);
    assert(key(KVM_KEY_CONTROL, 0x1d, 1, 1));
    assert(key(KVM_KEY_ALT, 0x38, 1, 3));
    assert(key('P', 0x19, 1, 3));
    assert(count == 2 && delivered[1].type == KVM_EVENT_HOTKEY);
    assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);

    initialize();
    assert(key(KVM_KEY_CONTROL, 0x1d, 1, 1));
    win32_window_emit_mouse(&context, 1, 1, 0);
    assert(count == 1 && delivered[0].type == KVM_EVENT_MOUSE);
    assert(key(KVM_KEY_ALT, 0x38, 1, 3));
    assert(key('P', 0x19, 1, 3));
    assert(key('P', 0x19, 0, 3));
    assert(key(KVM_KEY_ALT, 0x38, 0, 1));
    assert(key(KVM_KEY_CONTROL, 0x1d, 0, 0));
    assert(count == 2 && delivered[1].type == KVM_EVENT_HOTKEY);
    assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);

    initialize();
    assert(key(KVM_KEY_CONTROL, 0x1d, 1, 1));
    assert(key(KVM_KEY_ALT, 0x38, 1, 3));
    reject_at = 2;
    assert(!key('A', 0x1e, 1, 3));
    assert(count == 1 && delivered[0].data.key.key == KVM_KEY_CONTROL);
    assert(window.base.hotkey_matcher.held_count == 0);
    assert(!key('B', 0x30, 1, 0));
    assert(attempts == 2 && count == 1);
    assert(kvm_window_publish_frame(&window, &frame) == LIB_STATUS_INVALID_STATE);
    kvm_component_retire(&window.base, LIB_STATUS_OK);
    assert(failures == 1 && count == 2 && delivered[1].type == KVM_EVENT_SOURCE_RETIRED);
    assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);

    initialize();
    assert(kvm_window_set_title(&window, "first") == LIB_STATUS_OK);
    assert(kvm_window_publish_frame(&window, &frame) == LIB_STATUS_OK);
    assert(kvm_component_request_stop(&window.base) == LIB_STATUS_OK);
    assert(kvm_component_request_stop(&window.base) == LIB_STATUS_OK);
    assert(kvm_window_publish_frame(&window, &frame) == LIB_STATUS_INVALID_STATE);
    /* Call mailbox API so ordinary rejected control does not add a failure report. */
    kvm_component_control title = { .kind = KVM_COMPONENT_CONTROL_SET_WINDOW_TITLE };
    assert(kvm_component_mailboxes_enqueue_controls(&window.base.mailboxes, &title, 1u) == LIB_STATUS_INVALID_STATE);
    assert(kvm_component_mailboxes_take_control(&window.base.mailboxes, &taken));
    assert(taken.kind == KVM_COMPONENT_CONTROL_SET_WINDOW_TITLE);
    assert(kvm_component_mailboxes_take_control(&window.base.mailboxes, &taken));
    assert(taken.kind == KVM_COMPONENT_CONTROL_STOP);
    assert(!kvm_component_mailboxes_take_control(&window.base.mailboxes, &taken));
    assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);
    /* A held frame-copy lock must not block control producer/consumer. */
    initialize();
    assert(!lib_atomic_flag_test_and_set_explicit(&window.base.mailboxes.frame_lock,
        LIB_MEMORY_ORDER_ACQUIRE));
    HANDLE control_thread = CreateThread(NULL, 0, enqueue_title, NULL, 0, NULL);
    assert(control_thread);
    assert(WaitForSingleObject(control_thread, 5000) == WAIT_OBJECT_0);
    assert(kvm_component_mailboxes_take_control(&window.base.mailboxes, &taken));
    assert(taken.kind == KVM_COMPONENT_CONTROL_SET_WINDOW_TITLE);
    lib_atomic_flag_clear_explicit(&window.base.mailboxes.frame_lock, LIB_MEMORY_ORDER_RELEASE);
    CloseHandle(control_thread);
    assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);
    /* Either producer may win the admission lock, but no publish may commit
     * after STOP's boundary. Repeat the public concurrent paths without Sleep. */
    for (unsigned i = 0; i != 32; ++i) {
        initialize();
        start_race = CreateEventA(NULL, TRUE, FALSE, NULL);
        HANDLE thread = CreateThread(NULL, 0, publish, NULL, 0, NULL);
        assert(thread && start_race);
        SetEvent(start_race);
        assert(kvm_component_request_stop(&window.base) == LIB_STATUS_OK);
        lib_u32 stopped_sequence = window.base.mailboxes.frame_generation;
        assert(WaitForSingleObject(thread, 5000) == WAIT_OBJECT_0);
        assert(publish_status == LIB_STATUS_OK || publish_status == LIB_STATUS_INVALID_STATE);
        assert(window.base.mailboxes.frame_generation == stopped_sequence);
        assert(kvm_window_publish_frame(&window, &frame) == LIB_STATUS_INVALID_STATE);
        assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);
        CloseHandle(thread); CloseHandle(start_race);
    }
    return 0;
}
