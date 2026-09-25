#include "lib/types/test.h"
#include "lib/types/win32/test.h"
#include "lib/kvm-window/window.h"
#include "lib/types/win32/window.h"
/* This matcher fixture has no native Window; timer lifecycle is verified by
 * the capture/retirement/modal fixtures, not by creating thread timers here. */
static lib_win32_uint_ptr LIB_WIN32_WINAPI test_set_timer(lib_win32_hwnd window, lib_win32_uint_ptr id, lib_win32_uint interval,
    lib_win32_timer_proc callback)
{ (void)window; (void)interval; (void)callback; return id; }
static lib_win32_bool LIB_WIN32_WINAPI test_kill_timer(lib_win32_hwnd window, lib_win32_uint_ptr id)
{ (void)window; (void)id; return LIB_WIN32_TRUE; }
#undef lib_win32_set_timer
#undef lib_win32_kill_timer
#define lib_win32_set_timer test_set_timer
#define lib_win32_kill_timer test_kill_timer
#include "lib/kvm-window/win32/component.c"

static kvm_window window;
static kvm_win32_window_context context;
static kvm_window_frame frame;
static kvm_input_event delivered[32];
static lib_u32 count, attempts, reject_at, failures;
static lib_bool sink(void *opaque, const kvm_input_event *event)
{
    (void)opaque;
    if (++attempts == reject_at) return LIB_FALSE;
    lib_test_assert(count < 32);
    delivered[count++] = *event;
    return LIB_TRUE;
}
static void failure(void *opaque, lib_u64 identity, lib_status status)
{ (void)opaque; lib_test_assert(identity && status == LIB_STATUS_IO_ERROR); ++failures; }
static lib_status join(kvm_component *component, lib_u32 timeout_ms)
{ (void)component; (void)timeout_ms; return LIB_STATUS_OK; }
static void dispose(kvm_component *component)
{ kvm_component_mailboxes_destroy(&component->mailboxes); }
static void initialize(void)
{
    kvm_component_options options = { 0 };
    options.input_sink = sink; options.failure_sink = failure;
    lib_test_assert(kvm_hotkey_registry_register(&options.hotkeys, 'P',
        KVM_HOTKEY_MODIFIER_CONTROL | KVM_HOTKEY_MODIFIER_ALT, "toggle") == LIB_STATUS_OK);
    lib_test_assert(kvm_component_initialize(&window.base, &options, join, dispose,
        &window.pending_frame, sizeof(window.pending_frame)) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_select_notify(&window.base.mailboxes,
        LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    lib_memory_set(&context, 0, sizeof(context));
    context.component = &window;
    count = attempts = reject_at = failures = 0;
    frame.valid = LIB_TRUE; frame.text.base.text_columns = 80; frame.text.base.text_rows = 25;
}
static lib_bool key(kvm_key key, lib_u16 scan, lib_bool down, lib_u8 modifiers)
{
    kvm_input_event event = { 0 };
    event.type = KVM_EVENT_KEY;
    event.data.key.key = key; event.data.key.scan_code = scan;
    event.data.key.pressed = down; event.data.key.modifiers = modifiers;
    return win32_window_emit_normalized(&context, &event);
}
static void set_frozen(lib_bool frozen)
{
    kvm_component_control control = { .kind = KVM_WINDOW_CONTROL_SET_FROZEN };
    control.payload[0] = frozen;
    lib_test_assert(kvm_component_enqueue_control(&window.base, &control) == LIB_STATUS_OK);
    lib_test_assert(win32_window_consume_mailboxes(LIB_NULL, &context));
    lib_test_assert(context.frozen == frozen);
}
static void frozen_prefix_replay(void)
{
    /* All ordinary flush causes share the same permission captured at make.
     * Repeats after unfreeze must not upgrade a frozen-origin prefix. */
    for (lib_u32 frozen = 0u; frozen < 2; ++frozen)
    for (lib_u32 cause = 0; cause < 3; ++cause) {
        kvm_input_event text = { .type = KVM_EVENT_TEXT };
        text.data.text.scalar = 0x1f600;
        initialize();
        set_frozen(frozen);
        lib_test_assert(key(KVM_KEY_CONTROL, 0x1d, 1, 1));
        set_frozen(!frozen);
        set_frozen(LIB_FALSE);
        lib_test_assert(key(KVM_KEY_CONTROL, 0x1d, 1, 1));
        lib_test_assert(window.base.hotkey_matcher.held_count == 1 && count == 0);
        if (cause == 0) lib_test_assert(key('A', 0x1e, 1, 1));
        else if (cause == 1) lib_test_assert(key(KVM_KEY_CONTROL, 0x1d, 0, 0));
        else lib_test_assert(win32_window_emit_normalized(&context, &text));
        lib_test_assert(count == (frozen ? 1u : 2u));
        if (!frozen)
            lib_test_assert(delivered[0].type == KVM_EVENT_KEY &&
                delivered[0].data.key.key == KVM_KEY_CONTROL &&
                delivered[0].data.key.pressed);
        if (cause == 0) lib_test_assert(delivered[count - 1].data.key.key == 'A');
        if (cause == 1) lib_test_assert(!delivered[count - 1].data.key.pressed);
        if (cause == 2) lib_test_assert(delivered[count - 1].type == KVM_EVENT_TEXT);
        lib_test_assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);
    }
    /* Neither freezing direction may disable an otherwise matched hotkey. */
    for (lib_u32 frozen = 0u; frozen < 2; ++frozen) {
        initialize();
        set_frozen(frozen);
        lib_test_assert(key(KVM_KEY_CONTROL, 0x1d, 1, 1));
        set_frozen(!frozen);
        lib_test_assert(key(KVM_KEY_ALT, 0x38, 1, 3));
        lib_test_assert(key('P', 0x19, 1, 3));
        lib_test_assert(count == 1 && delivered[0].type == KVM_EVENT_HOTKEY);
        lib_test_assert(key('P', 0x19, 0, 3));
        lib_test_assert(key(KVM_KEY_ALT, 0x38, 0, 1));
        lib_test_assert(key(KVM_KEY_CONTROL, 0x1d, 0, 0));
        lib_test_assert(count == 1);
        lib_test_assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);
    }
    /* Owner explicitly retains per-event filtering, not make/break balancing. */
    for (lib_u32 frozen = 0u; frozen < 2; ++frozen) {
        initialize();
        set_frozen(frozen);
        lib_test_assert(key('A', 0x1e, 1, 0));
        set_frozen(!frozen);
        lib_test_assert(key('A', 0x1e, 0, 0));
        lib_test_assert(count == 1 && delivered[0].data.key.key == 'A' &&
            delivered[0].data.key.pressed == !frozen);
        lib_test_assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);
    }
    initialize();
    set_frozen(LIB_TRUE);
    lib_test_assert(key(KVM_KEY_CONTROL, 0x1d, 1, 1));
    set_frozen(LIB_FALSE);
    reject_at = 1;
    lib_test_assert(!key('A', 0x1e, 1, 1));
    lib_test_assert(count == 0 && attempts == 1 && window.base.stopping);
    lib_test_assert(!key('B', 0x30, 1, 0) && attempts == 1);
    lib_test_assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);
}
static lib_win32_handle start_race;
static lib_status publish_status;
static lib_win32_dword LIB_WIN32_WINAPI publish(void *unused)
{
    (void)unused;
    lib_test_assert(lib_win32_wait_for_single_object(start_race, 5000) == LIB_WIN32_WAIT_OBJECT_0);
    publish_status = kvm_window_publish_frame(&window, &frame);
    return 0;
}
static lib_win32_dword LIB_WIN32_WINAPI enqueue_title(void *unused)
{
    (void)unused;
    lib_test_assert(kvm_window_set_title(&window, "independent") == LIB_STATUS_OK);
    return 0;
}
int main(void)
{
    kvm_input_event event = { .type = KVM_EVENT_WINDOW_CLOSE };
    kvm_component_control taken;
    frozen_prefix_replay();
    initialize();
    context.frozen = LIB_TRUE;
    lib_test_assert(key(KVM_KEY_CONTROL, 0x1d, 1, 1));
    lib_test_assert(win32_window_emit_normalized(&context, &event));
    lib_test_assert(count == 1 && delivered[0].type == KVM_EVENT_WINDOW_CLOSE);
    lib_test_assert(key(KVM_KEY_CONTROL, 0x1d, 0, 0));
    lib_test_assert(count == 1); /* mismatch replay also uses the frozen filter */
    win32_window_emit_mouse(&context, 1, 1, 0);
    lib_test_assert(count == 1);
    lib_test_assert(key(KVM_KEY_CONTROL, 0x1d, 1, 1));
    lib_test_assert(key(KVM_KEY_ALT, 0x38, 1, 3));
    lib_test_assert(key('P', 0x19, 1, 3));
    lib_test_assert(count == 2 && delivered[1].type == KVM_EVENT_HOTKEY);
    lib_test_assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);

    initialize();
    lib_test_assert(key(KVM_KEY_CONTROL, 0x1d, 1, 1));
    win32_window_emit_mouse(&context, 1, 1, 0);
    lib_test_assert(count == 1 && delivered[0].type == KVM_EVENT_MOUSE);
    lib_test_assert(key(KVM_KEY_ALT, 0x38, 1, 3));
    lib_test_assert(key('P', 0x19, 1, 3));
    lib_test_assert(key('P', 0x19, 0, 3));
    lib_test_assert(key(KVM_KEY_ALT, 0x38, 0, 1));
    lib_test_assert(key(KVM_KEY_CONTROL, 0x1d, 0, 0));
    lib_test_assert(count == 2 && delivered[1].type == KVM_EVENT_HOTKEY);
    lib_test_assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);

    initialize();
    lib_test_assert(key(KVM_KEY_CONTROL, 0x1d, 1, 1));
    lib_test_assert(key(KVM_KEY_ALT, 0x38, 1, 3));
    reject_at = 2;
    lib_test_assert(!key('A', 0x1e, 1, 3));
    lib_test_assert(count == 1 && delivered[0].data.key.key == KVM_KEY_CONTROL);
    lib_test_assert(window.base.hotkey_matcher.held_count == 0);
    lib_test_assert(!key('B', 0x30, 1, 0));
    lib_test_assert(attempts == 2 && count == 1);
    lib_test_assert(kvm_window_publish_frame(&window, &frame) == LIB_STATUS_INVALID_STATE);
    kvm_component_retire(&window.base, LIB_STATUS_OK);
    lib_test_assert(failures == 1 && count == 2 && delivered[1].type == KVM_EVENT_SOURCE_RETIRED);
    lib_test_assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);

    initialize();
    lib_test_assert(kvm_window_set_title(&window, "first") == LIB_STATUS_OK);
    lib_test_assert(kvm_window_publish_frame(&window, &frame) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_request_stop(&window.base) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_request_stop(&window.base) == LIB_STATUS_OK);
    lib_test_assert(kvm_window_publish_frame(&window, &frame) == LIB_STATUS_INVALID_STATE);
    /* Call mailbox API so ordinary rejected control does not add a failure report. */
    kvm_component_control title = { .kind = KVM_WINDOW_CONTROL_SET_TITLE };
    lib_test_assert(kvm_component_mailboxes_enqueue_control(&window.base.mailboxes, &title) == LIB_STATUS_INVALID_STATE);
    lib_test_assert(kvm_component_mailboxes_take_control(&window.base.mailboxes, &taken));
    lib_test_assert(taken.kind == KVM_WINDOW_CONTROL_SET_TITLE);
    lib_test_assert(kvm_component_mailboxes_take_control(&window.base.mailboxes, &taken));
    lib_test_assert(taken.kind == KVM_COMPONENT_CONTROL_STOP);
    lib_test_assert(!kvm_component_mailboxes_take_control(&window.base.mailboxes, &taken));
    lib_test_assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);
    /* A held frame-copy lock must not block control producer/consumer. */
    initialize();
    base_sync_mutex_lock(window.base.mailboxes.frame_lock);
    lib_win32_handle control_thread = lib_win32_create_thread(LIB_NULL, 0, enqueue_title, LIB_NULL, 0, LIB_NULL);
    lib_test_assert(control_thread);
    lib_test_assert(lib_win32_wait_for_single_object(control_thread, 5000) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(kvm_component_mailboxes_take_control(&window.base.mailboxes, &taken));
    lib_test_assert(taken.kind == KVM_WINDOW_CONTROL_SET_TITLE);
    base_sync_mutex_unlock(window.base.mailboxes.frame_lock);
    lib_win32_close_handle(control_thread);
    lib_test_assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);
    /* Either producer may win the admission lock, but no publish may commit
     * after STOP's boundary. Repeat the public concurrent paths without lib_win32_sleep. */
    for (lib_u32 i = 0; i != 32; ++i) {
        initialize();
        start_race = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
        lib_win32_handle thread = lib_win32_create_thread(LIB_NULL, 0, publish, LIB_NULL, 0, LIB_NULL);
        lib_test_assert(thread && start_race);
        lib_win32_set_event(start_race);
        lib_test_assert(kvm_component_request_stop(&window.base) == LIB_STATUS_OK);
        lib_u32 stopped_sequence = window.base.mailboxes.frame_generation;
        lib_test_assert(lib_win32_wait_for_single_object(thread, 5000) == LIB_WIN32_WAIT_OBJECT_0);
        lib_test_assert(publish_status == LIB_STATUS_OK || publish_status == LIB_STATUS_INVALID_STATE);
        lib_test_assert(window.base.mailboxes.frame_generation == stopped_sequence);
        lib_test_assert(kvm_window_publish_frame(&window, &frame) == LIB_STATUS_INVALID_STATE);
        lib_test_assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);
        lib_win32_close_handle(thread); lib_win32_close_handle(start_race);
    }
    return 0;
}
