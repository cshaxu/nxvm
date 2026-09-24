#include "lib/kvm-window/window.h"
#include "lib/types/win32/window.h"
#include "lib/types/win32/sync.h"
#include <assert.h>

static HANDLE waiting, proceed;
static HWND created;
static lib_i32 scenario, retired, failures, ordinary, closes;
static void input_scenario(void);
static void paint_scenario(void);
static lib_u32 paint_ends;
static lib_i32 inject_output_failure;
static lib_u32 notification_attempts;
static lib_i32 reject_join;
static BOOL WINAPI startup_signal(HANDLE h)
{ return scenario==19 ? FALSE : SetEvent(h); }
static BOOL WINAPI set_title(HWND w, LPCSTR title)
{ return scenario==22 ? FALSE : SetWindowTextA(w,title); }
static lib_status select_notify(kvm_component_mailboxes *mailboxes,
    kvm_mailbox_notify_fn notify_fn, void *context)
{
    return scenario==23 ? LIB_STATUS_INVALID_STATE :
        kvm_component_mailboxes_select_notify(mailboxes,notify_fn,context);
}
static DWORD WINAPI join_wait(HANDLE h,DWORD timeout)
{
    if(reject_join && timeout==KVM_COMPONENT_DESTROY_TIMEOUT_MS) return WAIT_TIMEOUT;
    return WaitForSingleObject(h,timeout);
}
static UINT_PTR WINAPI start_timer(HWND w,UINT_PTR id,UINT ms,TIMERPROC fn)
{ return scenario==15 || scenario==16 ? 0 : SetTimer(w,id,ms,fn); }
static BOOL WINAPI stop_timer(HWND w,UINT_PTR id)
{ return scenario==17 ? FALSE : KillTimer(w,id); }
static BOOL WINAPI notify(HWND w, UINT m, WPARAM a, LPARAM b)
{
    if(scenario==18) { ++notification_attempts; return FALSE; }
    if (scenario == 14 && notification_attempts++ == 0) return FALSE;
    return SendNotifyMessageW(w,m,a,b);
}
static BOOL WINAPI invalidate(HWND w,const RECT *r,BOOL erase)
{ return inject_output_failure && scenario==9 ? FALSE : InvalidateRect(w,r,erase); }
static BOOL WINAPI blit(HDC d,lib_i32 x,lib_i32 y,lib_i32 w,lib_i32 h,HDC s,lib_i32 sx,lib_i32 sy,lib_i32 sw,lib_i32 sh,DWORD op)
{ return scenario==10 ? FALSE : StretchBlt(d,x,y,w,h,s,sx,sy,sw,sh,op); }
static BOOL WINAPI invert(HDC d,const RECT *r)
{ return scenario==11 ? FALSE : InvertRect(d,r); }
static HDC WINAPI begin(HWND w,PAINTSTRUCT *p)
{ return inject_output_failure && scenario==12 ? NULL : BeginPaint(w,p); }
static BOOL WINAPI end(HWND w,const PAINTSTRUCT *p)
{ ++paint_ends; BOOL result=EndPaint(w,p); return scenario==20 && inject_output_failure ? FALSE : result; }
static BOOL WINAPI dispose_cursor(HCURSOR cursor)
{ return scenario==21 ? FALSE : DestroyCursor(cursor); }
static HCURSOR WINAPI make_cursor(HINSTANCE i,lib_i32 x,lib_i32 y,lib_i32 w,lib_i32 h,const void *a,const void *b)
{ return scenario==13 ? NULL : CreateCursor(i,x,y,w,h,a,b); }
static HWND WINAPI create_window(DWORD ex, LPCWSTR klass, LPCWSTR title,
    DWORD style, lib_i32 x, lib_i32 y, lib_i32 width, lib_i32 height, HWND parent, HMENU menu,
    HINSTANCE instance, LPVOID param)
{
    created = CreateWindowExW(ex, klass, title, style, x, y, width, height,
        parent, menu, instance, param);
    assert(created == NULL || IsWindowUnicode(created));
    return created;
}
static BOOL WINAPI hide_window(HWND window, lib_i32 command)
{ (void)command; return ShowWindow(window, SW_HIDE); }
static BOOL WINAPI no_foreground(HWND window) { (void)window; return TRUE; }
static HWND WINAPI no_focus(HWND window) { return window; }
static HDC WINAPI create_dc(HDC dc)
{ return scenario == 2 ? NULL : CreateCompatibleDC(dc); }
static DWORD WINAPI controlled_wait(DWORD count, const HANDLE *handles,
    BOOL all, DWORD timeout, DWORD mask)
{
    (void)count; (void)handles; (void)all; (void)mask; (void)timeout;
    SetEvent(waiting);
    assert(WaitForSingleObject(proceed, INFINITE) == WAIT_OBJECT_0);
    inject_output_failure=1;
    if (scenario == 1 || scenario == 6) return WAIT_FAILED;
    if (scenario == 3) PostQuitMessage(0);
    if (scenario == 4) assert(DestroyWindow(created));
    if (scenario == 7 || scenario == 8) input_scenario();
    if ((scenario >= 10 && scenario <= 12) || scenario==20) paint_scenario();
    SendMessageW(created, WM_APP + 1, 0, 0);
    return WAIT_OBJECT_0;
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
    assert(kvm_component_mailboxes_publish_frame(&component->mailboxes, &rejected,
        kvm_window_frame_size_bytes(&rejected)) ==
        LIB_STATUS_INVALID_STATE);
    kvm_component_control title = { .kind = KVM_WINDOW_CONTROL_SET_TITLE };
    assert(kvm_component_mailboxes_enqueue_control(&component->mailboxes, &title) ==
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
    assert(event->type == KVM_EVENT_SOURCE_RETIRED);
    ++retired;
    return scenario < 5 || scenario >= 7;
}
static void failure(void *context, lib_u64 identity, lib_status status)
{
    (void)context;
    assert(identity != 0u && status == LIB_STATUS_IO_ERROR);
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
        assert(!win32_window_emit_normalized(ctx, &key));
        assert(!win32_window_emit_normalized(ctx, &key));
    } else {
        assert(win32_window_emit_normalized(ctx, &key));
        SendMessageA(created, WM_CLOSE, 0, 0);
        key.data.key.pressed = 0;
        assert(win32_window_emit_normalized(ctx, &key));
        assert(kvm_component_request_stop(&ctx->component->base) == LIB_STATUS_OK);
    }
}
static void paint_scenario(void)
{
    kvm_win32_window_context *ctx=win32_window_context(created);
    if (!ctx->surface_dc) return; /* First wake consumes the copied frame. */
    SendMessageW(created,WM_PAINT,0,0);
}
int main(void)
{
    static kvm_window_frame frame;
    for (scenario = 0; scenario != 24; ++scenario) {
        kvm_window_options options = { 0 };
        kvm_window *window = NULL;
        waiting = CreateEventA(NULL, TRUE, FALSE, NULL);
        proceed = CreateEventA(NULL, TRUE, FALSE, NULL);
        assert(waiting && proceed);
        retired = failures = ordinary = closes = 0;
        paint_ends=0;
        inject_output_failure=0;
        notification_attempts=0;
        reject_join=0;
        options.component.input_sink = input;
        options.component.failure_sink = failure;
        options.initial_title = "retirement proof";
        options.initial_frozen = scenario == 8 || scenario == 16;
        assert(kvm_hotkey_registry_register(&options.component.hotkeys, 'P',
            KVM_HOTKEY_MODIFIER_CONTROL | KVM_HOTKEY_MODIFIER_ALT, "toggle") == LIB_STATUS_OK);
        if (scenario==13 || scenario==15 || scenario==19 || scenario>=22) {
            assert(kvm_window_create(&window,&options)==
                (scenario==23 ? LIB_STATUS_INVALID_STATE : LIB_STATUS_IO_ERROR) && !window);
            assert(!IsWindow(created) && retired==(scenario==19));
            CloseHandle(waiting); CloseHandle(proceed);
            continue;
        }
        assert(kvm_window_create(&window, &options) == LIB_STATUS_OK);
        assert(WaitForSingleObject(waiting, INFINITE) == WAIT_OBJECT_0);
        frame.valid = 1u; frame.text.base.text_columns = 80u; frame.text.base.text_rows = 25u;
        frame.text.base.cursor_visible=1; frame.text.base.font_height=16; frame.text.base.cursor_top=14; frame.text.base.cursor_bottom=15;
        assert(kvm_window_publish_frame(window, &frame) == LIB_STATUS_OK);
        if(scenario==18) {
            void *retained=window->worker_state;
            reject_join=1;
            assert(kvm_window_destroy(window)==LIB_STATUS_IO_ERROR);
            assert(window->worker_state==retained);
            assert(!retired);
            /* Native wake coalescing is private.  The contract is the
             * independently reported component fault, not its attempt count. */
            assert(failures==1);
            reject_join=0;
        }
        if (scenario == 0 || scenario == 5 || scenario==21)
            assert(kvm_component_request_stop(&window->base) == LIB_STATUS_OK);
        if (scenario==16) assert(kvm_window_unfreeze(window)==LIB_STATUS_OK);
        if (scenario==17) assert(kvm_window_freeze(window)==LIB_STATUS_OK);
        SetEvent(proceed);
        /* Wait for the actual worker before destroy queues an additional STOP. */
        kvm_window_win32_state *state = window->worker_state;
        assert(WaitForSingleObject(state->worker, 5000u) == WAIT_OBJECT_0);
        assert(retired == 1 && failures == (scenario != 0 && scenario != 8));
        assert(!IsWindow(created));
        if (scenario==10 || scenario==11) assert(paint_ends>0);
        assert(ordinary == (scenario == 7) && closes == (scenario == 8));
        assert(kvm_window_publish_frame(window, &frame) == LIB_STATUS_INVALID_STATE);
        assert(kvm_window_destroy(window)==LIB_STATUS_OK);
        assert(retired == 1);
        CloseHandle(waiting); CloseHandle(proceed);
    }
    return 0;
}
