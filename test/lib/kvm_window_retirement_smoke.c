#include "lib/types/test.h"
#include "lib/types/win32/test.h"
#include "lib/types/file.h"
#include "lib/kvm-window/window.h"
#include "lib/types/win32/window.h"
#include "lib/types/win32/sync.h"

static lib_win32_handle waiting, proceed;
static lib_win32_hwnd created;
static lib_i32 scenario, retired, failures, ordinary, closes;
static void input_scenario(void);
static void paint_scenario(void);
static lib_u32 paint_ends;
static lib_i32 inject_output_failure;
static lib_u32 notification_attempts;
static lib_i32 reject_join;
static lib_win32_bool LIB_WIN32_WINAPI startup_signal(lib_win32_handle h)
{ return scenario==19 ? LIB_WIN32_FALSE : lib_win32_set_event(h); }
static lib_win32_bool LIB_WIN32_WINAPI set_title(lib_win32_hwnd w, lib_win32_lpcstr title)
{ return scenario==22 ? LIB_WIN32_FALSE : lib_win32_set_window_text_a(w,title); }
static lib_status select_notify(kvm_component_mailboxes *mailboxes,
    kvm_mailbox_notify_fn notify_fn, void *context)
{
    return scenario==23 ? LIB_STATUS_INVALID_STATE :
        kvm_component_mailboxes_select_notify(mailboxes,notify_fn,context);
}
static lib_win32_dword LIB_WIN32_WINAPI join_wait(lib_win32_handle h,lib_win32_dword timeout)
{
    if(reject_join && timeout==KVM_COMPONENT_DESTROY_TIMEOUT_MS) return LIB_WIN32_WAIT_TIMEOUT;
    return lib_win32_wait_for_single_object(h,timeout);
}
static lib_win32_uint_ptr LIB_WIN32_WINAPI start_timer(lib_win32_hwnd w,lib_win32_uint_ptr id,lib_win32_uint ms,lib_win32_timer_proc fn)
{ return scenario==15 || scenario==16 ? 0 : lib_win32_set_timer(w,id,ms,fn); }
static lib_win32_bool LIB_WIN32_WINAPI stop_timer(lib_win32_hwnd w,lib_win32_uint_ptr id)
{ return scenario==17 ? LIB_WIN32_FALSE : lib_win32_kill_timer(w,id); }
static lib_win32_bool LIB_WIN32_WINAPI notify(lib_win32_hwnd w, lib_win32_uint m, lib_win32_wparam a, lib_win32_lparam b)
{
    if(scenario==18) { ++notification_attempts; return LIB_WIN32_FALSE; }
    if (scenario == 14 && notification_attempts++ == 0) return LIB_WIN32_FALSE;
    return lib_win32_send_notify_message_w(w,m,a,b);
}
static lib_win32_bool LIB_WIN32_WINAPI invalidate(lib_win32_hwnd w,const lib_win32_rect *r,lib_win32_bool erase)
{ return inject_output_failure && scenario==9 ? LIB_WIN32_FALSE : lib_win32_invalidate_rect(w,r,erase); }
static lib_win32_bool LIB_WIN32_WINAPI blit(lib_win32_hdc d,lib_i32 x,lib_i32 y,lib_i32 w,lib_i32 h,lib_win32_hdc s,lib_i32 sx,lib_i32 sy,lib_i32 sw,lib_i32 sh,lib_win32_dword op)
{ return scenario==10 ? LIB_WIN32_FALSE : lib_win32_stretch_blt(d,x,y,w,h,s,sx,sy,sw,sh,op); }
static lib_win32_bool LIB_WIN32_WINAPI invert(lib_win32_hdc d,const lib_win32_rect *r)
{ return scenario==11 ? LIB_WIN32_FALSE : lib_win32_invert_rect(d,r); }
static lib_win32_hdc LIB_WIN32_WINAPI begin(lib_win32_hwnd w,lib_win32_paintstruct *p)
{ return inject_output_failure && scenario==12 ? LIB_NULL : lib_win32_begin_paint(w,p); }
static lib_win32_bool LIB_WIN32_WINAPI end(lib_win32_hwnd w,const lib_win32_paintstruct *p)
{ ++paint_ends; lib_win32_bool result=lib_win32_end_paint(w,p); return scenario==20 && inject_output_failure ? LIB_WIN32_FALSE : result; }
static lib_win32_bool LIB_WIN32_WINAPI dispose_cursor(lib_win32_hcursor cursor)
{ return scenario==21 ? LIB_WIN32_FALSE : lib_win32_destroy_cursor(cursor); }
static lib_win32_hcursor LIB_WIN32_WINAPI make_cursor(lib_win32_hinstance i,lib_i32 x,lib_i32 y,lib_i32 w,lib_i32 h,const void *a,const void *b)
{ return scenario==13 ? LIB_NULL : lib_win32_create_cursor(i,x,y,w,h,a,b); }
static lib_win32_hwnd LIB_WIN32_WINAPI create_window(lib_win32_dword ex, lib_win32_lpcwstr klass, lib_win32_lpcwstr title,
    lib_win32_dword style, lib_i32 x, lib_i32 y, lib_i32 width, lib_i32 height, lib_win32_hwnd parent, lib_win32_hmenu menu,
    lib_win32_hinstance instance, lib_win32_lpvoid param)
{
    created = lib_win32_create_window_ex_w(ex, klass, title, style, x, y, width, height,
        parent, menu, instance, param);
    lib_test_assert(created == LIB_NULL || lib_win32_is_window_unicode(created));
    return created;
}
static lib_win32_bool LIB_WIN32_WINAPI hide_window(lib_win32_hwnd window, lib_i32 command)
{ (void)command; return lib_win32_show_window(window, LIB_WIN32_SW_HIDE); }
static lib_win32_bool LIB_WIN32_WINAPI no_foreground(lib_win32_hwnd window) { (void)window; return LIB_WIN32_TRUE; }
static lib_win32_hwnd LIB_WIN32_WINAPI no_focus(lib_win32_hwnd window) { return window; }
static lib_win32_hdc LIB_WIN32_WINAPI create_dc(lib_win32_hdc dc)
{ return scenario == 2 ? LIB_NULL : lib_win32_create_compatible_dc(dc); }
static lib_win32_dword LIB_WIN32_WINAPI controlled_wait(lib_win32_dword count, const lib_win32_handle *handles,
    lib_win32_bool all, lib_win32_dword timeout, lib_win32_dword mask)
{
    (void)count; (void)handles; (void)all; (void)mask; (void)timeout;
    lib_win32_set_event(waiting);
    lib_test_assert(lib_win32_wait_for_single_object(proceed, LIB_WIN32_INFINITE) == LIB_WIN32_WAIT_OBJECT_0);
    inject_output_failure=1;
    if (scenario == 1 || scenario == 6) return LIB_WIN32_WAIT_FAILED;
    if (scenario == 3) lib_win32_post_quit_message(0);
    if (scenario == 4) lib_test_assert(lib_win32_destroy_window(created));
    if (scenario == 7 || scenario == 8) input_scenario();
    if ((scenario >= 10 && scenario <= 12) || scenario==20) paint_scenario();
    lib_win32_send_message_w(created, LIB_WIN32_WM_APP + 1, 0, 0);
    return LIB_WIN32_WAIT_OBJECT_0;
}
#undef lib_win32_create_window_ex_w
#undef lib_win32_show_window
#undef lib_win32_set_foreground_window
#undef lib_win32_set_focus
#undef lib_win32_create_compatible_dc
#define lib_win32_create_window_ex_w create_window
#define lib_win32_show_window hide_window
#define lib_win32_set_foreground_window no_foreground
#define lib_win32_set_focus no_focus
#define lib_win32_create_compatible_dc create_dc
#undef lib_win32_invalidate_rect
#undef lib_win32_stretch_blt
#undef lib_win32_invert_rect
#undef lib_win32_begin_paint
#undef lib_win32_end_paint
#undef lib_win32_create_cursor
#define lib_win32_invalidate_rect invalidate
#define lib_win32_stretch_blt blit
#define lib_win32_invert_rect invert
#define lib_win32_begin_paint begin
#define lib_win32_end_paint end
#define lib_win32_create_cursor make_cursor
#undef lib_win32_destroy_cursor
#define lib_win32_destroy_cursor dispose_cursor
#undef lib_win32_send_notify_message_w
#define lib_win32_send_notify_message_w notify
#undef lib_win32_set_timer
#undef lib_win32_kill_timer
#define lib_win32_set_timer start_timer
#define lib_win32_kill_timer stop_timer
#undef lib_win32_msg_wait_for_multiple_objects
#define lib_win32_msg_wait_for_multiple_objects controlled_wait
#undef lib_win32_wait_for_single_object
#define lib_win32_wait_for_single_object join_wait
#undef lib_win32_set_event
#define lib_win32_set_event startup_signal
#undef lib_win32_set_window_text_a
#define lib_win32_set_window_text_a set_title
#define kvm_component_mailboxes_select_notify select_notify
static void checked_fail(kvm_component *component, lib_status status)
{
    static kvm_window_frame rejected = { .valid = 1, .text.base = { .text_columns = 80, .text_rows = 25 } };
    kvm_component_fail(component, status);
    lib_test_assert(kvm_component_mailboxes_publish_frame(&component->mailboxes, &rejected,
        kvm_window_frame_size_bytes(&rejected)) ==
        LIB_STATUS_INVALID_STATE);
    kvm_component_control title = { .kind = KVM_WINDOW_CONTROL_SET_TITLE };
    lib_test_assert(kvm_component_mailboxes_enqueue_control(&component->mailboxes, &title) ==
        LIB_STATUS_INVALID_STATE);
}
#define kvm_component_fail checked_fail
#include "lib/kvm-window/win32/component.c"
#undef kvm_component_fail

static lib_i32 input(void *context, const kvm_input_event *event)
{
    (void)context;
    if (scenario == 7 && event->type == KVM_EVENT_KEY) { ++ordinary; return 0; }
    if (scenario == 8 && event->type == KVM_EVENT_WINDOW_CLOSE) { ++closes; return 1; }
    lib_test_assert(event->type == KVM_EVENT_SOURCE_RETIRED);
    ++retired;
    return scenario < 5 || scenario >= 7;
}
static void failure(void *context, lib_u64 identity, lib_status status)
{
    (void)context;
    lib_test_assert(identity != 0u && status == LIB_STATUS_IO_ERROR);
    ++failures;
}
static void input_scenario(void)
{
    kvm_win32_window_context *ctx = win32_window_context(created);
    kvm_input_event key = { .type = KVM_EVENT_KEY };
    key.data.key.pressed = 1; key.data.key.scan_code = 0x1d;
    key.data.key.key = scenario == 7 ? 'A' : KVM_KEY_CONTROL;
    key.data.key.modifiers = KVM_HOTKEY_MODIFIER_CONTROL;
    if (scenario == 7) {
        lib_test_assert(!win32_window_emit_normalized(ctx, &key));
        lib_test_assert(!win32_window_emit_normalized(ctx, &key));
    } else {
        lib_test_assert(win32_window_emit_normalized(ctx, &key));
        lib_win32_send_message_a(created, LIB_WIN32_WM_CLOSE, 0, 0);
        key.data.key.pressed = 0;
        lib_test_assert(win32_window_emit_normalized(ctx, &key));
        lib_test_assert(kvm_component_request_stop(&ctx->component->base) == LIB_STATUS_OK);
    }
}
static void paint_scenario(void)
{
    kvm_win32_window_context *ctx=win32_window_context(created);
    if (!ctx->surface_dc) return; /* First wake consumes the copied frame. */
    lib_win32_send_message_w(created,LIB_WIN32_WM_PAINT,0,0);
}
int main(void)
{
    static kvm_window_frame frame;
    for (scenario = 0; scenario != 24; ++scenario) {
        kvm_window_options options = { 0 };
        kvm_window *window = LIB_NULL;
        waiting = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
        proceed = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
        lib_test_assert(waiting && proceed);
        retired = failures = ordinary = closes = 0;
        paint_ends=0;
        inject_output_failure=0;
        notification_attempts=0;
        reject_join=0;
        options.component.input_sink = input;
        options.component.failure_sink = failure;
        options.initial_title = "retirement proof";
        options.initial_frozen = scenario == 8 || scenario == 16;
        lib_test_assert(kvm_hotkey_registry_register(&options.component.hotkeys, 'P',
            KVM_HOTKEY_MODIFIER_CONTROL | KVM_HOTKEY_MODIFIER_ALT, "toggle") == LIB_STATUS_OK);
        if (scenario==13 || scenario==15 || scenario==19 || scenario>=22) {
            lib_test_assert(kvm_window_create(&window,&options)==
                (scenario==23 ? LIB_STATUS_INVALID_STATE : LIB_STATUS_IO_ERROR) && !window);
            lib_test_assert(!lib_win32_is_window(created) && retired==(scenario==19));
            lib_win32_close_handle(waiting); lib_win32_close_handle(proceed);
            continue;
        }
        lib_test_assert(kvm_window_create(&window, &options) == LIB_STATUS_OK);
        lib_test_assert(lib_win32_wait_for_single_object(waiting, LIB_WIN32_INFINITE) == LIB_WIN32_WAIT_OBJECT_0);
        frame.valid = 1u; frame.text.base.text_columns = 80u; frame.text.base.text_rows = 25u;
        frame.text.base.cursor_visible=1; frame.text.base.font_height=16; frame.text.base.cursor_top=14; frame.text.base.cursor_bottom=15;
        lib_test_assert(kvm_window_publish_frame(window, &frame) == LIB_STATUS_OK);
        if(scenario==18) {
            void *retained=window->worker_state;
            reject_join=1;
            lib_test_assert(kvm_window_destroy(window)==LIB_STATUS_IO_ERROR);
            lib_test_assert(window->worker_state==retained);
            lib_test_assert(!retired);
            /* Native wake coalescing is private.  The contract is the
             * independently reported component fault, not its attempt count. */
            lib_test_assert(failures==1);
            reject_join=0;
        }
        if (scenario == 0 || scenario == 5 || scenario==21)
            lib_test_assert(kvm_component_request_stop(&window->base) == LIB_STATUS_OK);
        if (scenario==16) lib_test_assert(kvm_window_unfreeze(window)==LIB_STATUS_OK);
        if (scenario==17) lib_test_assert(kvm_window_freeze(window)==LIB_STATUS_OK);
        lib_win32_set_event(proceed);
        /* Wait for the actual worker before destroy queues an additional STOP. */
        kvm_window_win32_state *state = window->worker_state;
        lib_test_assert(lib_win32_wait_for_single_object(state->worker, 5000u) == LIB_WIN32_WAIT_OBJECT_0);
        lib_test_assert(retired == 1 && failures == (scenario != 0 && scenario != 8));
        lib_test_assert(!lib_win32_is_window(created));
        if (scenario==10 || scenario==11) lib_test_assert(paint_ends>0);
        lib_test_assert(ordinary == (scenario == 7) && closes == (scenario == 8));
        lib_test_assert(kvm_window_publish_frame(window, &frame) == LIB_STATUS_INVALID_STATE);
        lib_test_assert(kvm_window_destroy(window)==LIB_STATUS_OK);
        lib_test_assert(retired == 1);
        lib_win32_close_handle(waiting); lib_win32_close_handle(proceed);
    }
    return 0;
}
