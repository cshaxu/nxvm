#include "lib/kvm-window/window.h"
#include "lib/types/win32/window.h"
#include "lib/types/win32/sync.h"
#include <assert.h>

static HWND owner, focused;
static RECT client = {0,0,640,480}, clipped;
static POINT origin = {100,200};
static unsigned releases, clips, events;
static int reject_input;
static int release_ok=1;
static unsigned focus_requests, foreground_requests;
static int clip_ok = 1, resize_ok = 1, title_ok = 1, client_ok = 1;
static DWORD ticks;
static unsigned timer_starts, timer_stops;
static int timer_ok = 1;
static UINT_PTR WINAPI start_timer(HWND w,UINT_PTR id,UINT ms,TIMERPROC fn)
{ (void)w; assert(id==1 && ms==250 && !fn); ++timer_starts; return timer_ok ? id : 0; }
static BOOL WINAPI stop_timer(HWND w,UINT_PTR id)
{ (void)w; assert(id==1); ++timer_stops; return timer_ok; }
static int selection_ok;
static unsigned selections, deleted_bitmaps, deleted_dcs;
static lib_u32 surface_bits[64];
static HDC WINAPI surface_dc(HWND w) { (void)w; return (HDC)1; }
static HDC WINAPI compatible_dc(HDC d) { (void)d; return (HDC)2; }
static HBITMAP WINAPI bitmap(HDC d,const BITMAPINFO *i,UINT u,void **p,HANDLE s,DWORD o)
{ (void)d;(void)i;(void)u;(void)s;(void)o;*p=surface_bits;return (HBITMAP)3; }
static int WINAPI release_dc(HWND w,HDC d) { (void)w;(void)d;return 1; }
static HGDIOBJ WINAPI select_bitmap(HDC d,HGDIOBJ o)
{ (void)d;assert(o==(HGDIOBJ)3 || o==(HGDIOBJ)4);++selections;return selection_ok ? (HGDIOBJ)4 : NULL; }
static BOOL WINAPI delete_bitmap(HGDIOBJ o) { assert(o==(HGDIOBJ)3);++deleted_bitmaps;return TRUE; }
static BOOL WINAPI delete_dc(HDC d) { assert(d==(HDC)2);++deleted_dcs;return TRUE; }
static DWORD WINAPI clock_tick(void) { return ticks; }
static BOOL WINAPI invalidate(HWND w,const RECT *r,BOOL erase)
{ (void)w;(void)r;(void)erase;return TRUE; }
static void *context;
static void notify_loss(void);
static int reenter_title;
static void title_notification(void);
static HWND WINAPI get_capture(void) { return owner; }
static HWND WINAPI set_capture(HWND w) { HWND old=owner; owner=w; return old; }
static BOOL WINAPI release_capture(void)
{ ++releases; owner=NULL; notify_loss(); return release_ok; }
static HWND WINAPI set_focus(HWND w) { ++focus_requests; focused=w; return w; }
static BOOL WINAPI foreground(HWND w) { (void)w; ++foreground_requests; return TRUE; }
static HWND WINAPI get_focus(void) { return focused; }
static BOOL WINAPI clip(const RECT *r)
{ ++clips; if (r) clipped=*r; return r ? clip_ok : TRUE; }
static BOOL WINAPI get_client(HWND w, RECT *r)
{ (void)w; if (!client_ok) return FALSE; *r=client; return TRUE; }
static BOOL WINAPI to_screen(HWND w, POINT *p)
{ (void)w; p->x+=origin.x; p->y+=origin.y; return TRUE; }
static HCURSOR WINAPI cursor(HCURSOR c) { return c; }
static LONG_PTR WINAPI get_context(HWND w, int index)
{ (void)w; return index==GWLP_USERDATA ? (LONG_PTR)context : 0; }
static BOOL WINAPI resize(HWND w, HWND after, int x,int y,int cx,int cy,UINT f)
{ (void)w;(void)after;(void)x;(void)y;(void)cx;(void)cy;(void)f;return resize_ok; }
static BOOL WINAPI title(HWND w,LPCSTR text)
{ (void)w;(void)text; if(reenter_title) title_notification(); return title_ok; }
#undef lib_win32_get_capture
#undef lib_win32_set_capture
#undef lib_win32_release_capture
#undef lib_win32_set_focus
#undef lib_win32_set_foreground_window
#undef lib_win32_get_focus
#undef lib_win32_clip_cursor
#undef lib_win32_get_client_rect
#undef lib_win32_client_to_screen
#undef lib_win32_set_cursor
#undef lib_win32_get_window_long_ptr_a
#undef lib_win32_set_window_pos
#undef lib_win32_set_window_text_a
#define lib_win32_get_capture get_capture
#define lib_win32_set_capture set_capture
#define lib_win32_release_capture release_capture
#define lib_win32_set_focus set_focus
#define lib_win32_set_foreground_window foreground
#define lib_win32_get_focus get_focus
#define lib_win32_clip_cursor clip
#define lib_win32_get_client_rect get_client
#define lib_win32_client_to_screen to_screen
#define lib_win32_set_cursor cursor
#define lib_win32_get_window_long_ptr_a get_context
#define lib_win32_set_window_pos resize
#define lib_win32_set_window_text_a title
#undef lib_win32_get_tick_count
#define lib_win32_get_tick_count clock_tick
#undef lib_win32_set_timer
#undef lib_win32_kill_timer
#define lib_win32_set_timer start_timer
#define lib_win32_kill_timer stop_timer
#undef lib_win32_invalidate_rect
#define lib_win32_invalidate_rect invalidate
#undef lib_win32_get_dc
#undef lib_win32_create_compatible_dc
#undef lib_win32_create_dibsection
#undef lib_win32_release_dc
#undef lib_win32_select_object
#undef lib_win32_delete_object
#undef lib_win32_delete_dc
#define lib_win32_get_dc surface_dc
#define lib_win32_create_compatible_dc compatible_dc
#define lib_win32_create_dibsection bitmap
#define lib_win32_release_dc release_dc
#define lib_win32_select_object select_bitmap
#define lib_win32_delete_object delete_bitmap
#define lib_win32_delete_dc delete_dc
#include "lib/kvm-window/win32/mouse.c"
#include "lib/kvm-window/win32/geometry.c"
#include "lib/kvm-window/win32/component.c"

static void notify_loss(void) { win32_window_proc((HWND)1,WM_CAPTURECHANGED,0,0); }
static lib_status immediate_notification(void *p)
{ (void)p; win32_window_proc((HWND)1,WIN32_WINDOW_MAILBOX_READY,0,0); return 0; }
static void title_notification(void)
{
    kvm_win32_window_context *c=context;
    reenter_title=0;
    assert(c->consuming && c->frozen);
    assert(kvm_window_unfreeze(c->component)==0);
    assert(c->frozen); /* Nested notification must not drain ahead of this control. */
}
static int input(void *p,const kvm_input_event *e)
{ (void)p; assert(e->type==KVM_EVENT_MOUSE); ++events; return !reject_input; }
static void failure(void *p,lib_u64 id,lib_status status)
{ (void)p;(void)id;(void)status; }
static lib_status join(kvm_component *p, lib_u32 timeout_ms)
{ (void)p; (void)timeout_ms; return LIB_STATUS_OK; }
static void dispose(kvm_component *p) { kvm_component_mailboxes_destroy(&p->mailboxes); }
int main(void)
{
    static kvm_win32_window_context surface;
    assert(!win32_window_ensure_surface((HWND)1,&surface,8,8));
    assert(!surface.surface_width && !surface.surface_height && !surface.surface_dc && !surface.surface_pixels);
    assert(selections==1 && deleted_bitmaps==1 && deleted_dcs==1);
    selection_ok=1;
    assert(win32_window_ensure_surface((HWND)1,&surface,8,8));
    assert(surface.surface_width==8 && selections==2);
    win32_window_destroy_surface(&surface);
    assert(selections==3 && deleted_bitmaps==2 && deleted_dcs==2);
    static kvm_window window;
    static kvm_win32_window_context c;
    kvm_component_options options={.input_sink=input,.failure_sink=failure};
    assert(kvm_component_initialize(&window.base,&options,join,dispose)==LIB_STATUS_OK);
    assert(kvm_component_mailboxes_select_notify(&window.base.mailboxes,
        LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    c.component=&window; context=&c;
    assert(kvm_win32_mouse_refresh_bounds(&c.mouse) && clips==0);
    assert(kvm_win32_mouse_capture(&c.mouse,(HWND)1,0) == LIB_STATUS_OK);
    assert(clipped.left==100 && clipped.top==200 && clipped.right==740 && clipped.bottom==680);
    origin.x=-300; origin.y=50;
    win32_window_proc((HWND)1,WM_MOVE,0,0);
    assert(clipped.left==-300 && clipped.top==50);
    client.right=320; client.bottom=240;
    win32_window_proc((HWND)1,WM_SIZE,0,0);
    assert(clipped.right==20 && clipped.bottom==290);
    c.left_button=1; c.mouse.motion.remainder_x=7;
    owner=(HWND)2; notify_loss();
    assert(!c.mouse.captured && !c.left_button && !c.mouse.motion.valid);
    assert(!c.mouse.motion.remainder_x && releases==0 && events==1 && owner==(HWND)2);
    notify_loss(); assert(events==1 && releases==0);
    assert(kvm_win32_mouse_capture(&c.mouse,(HWND)1,0) == LIB_STATUS_OK);
    c.right_button=1; win32_window_release_mouse(&c);
    assert(releases==1 && events==2 && !c.mouse.captured && !c.right_button);
    assert(kvm_win32_mouse_capture(&c.mouse,(HWND)1,0) == LIB_STATUS_OK);
    clip_ok=0; win32_window_proc((HWND)1,WM_MOVE,0,0);
    assert(!c.mouse.captured && releases==2);
    unsigned previous=clips;
    win32_window_proc((HWND)1,WM_MOVE,0,0); assert(clips==previous);
    resize_ok=0;
    win32_window_resize_client((HWND)1,&c,640,480);
    assert(c.client_surface_width==0 && window.base.stopping);
    assert(kvm_component_destroy(&window.base)==LIB_STATUS_OK);
    assert(kvm_component_initialize(&window.base,&options,join,dispose)==LIB_STATUS_OK);
    assert(kvm_component_mailboxes_select_notify(&window.base.mailboxes,
        LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    resize_ok=1;
    win32_window_resize_client((HWND)1,&c,640,480);
    assert(c.client_surface_width==640 && c.client_surface_height==480);
    focus_requests=foreground_requests=0;
    assert(kvm_window_unfreeze(&window)==LIB_STATUS_OK);
    assert(win32_window_consume_mailboxes((HWND)1,&c));
    assert(focus_requests==0 && foreground_requests==0);
    clip_ok=1;
    assert(kvm_win32_mouse_capture(&c.mouse,(HWND)1,0)==LIB_STATUS_OK);
    focus_requests=foreground_requests=0;
    unsigned releases_before_freeze=releases;
    assert(kvm_window_freeze(&window)==LIB_STATUS_OK);
    assert(c.mouse.captured && !c.frozen);
    assert(win32_window_consume_mailboxes((HWND)1,&c));
    assert(c.frozen && !c.mouse.captured && releases==releases_before_freeze+1);
    assert(focus_requests==0 && foreground_requests==0);
    assert(kvm_window_unfreeze(&window)==LIB_STATUS_OK);
    assert(win32_window_consume_mailboxes((HWND)1,&c));
    assert(focus_requests==1 && foreground_requests==1 && !c.mouse.captured);
    c.client_width=320; c.client_height=240; client_ok=0;
    win32_window_capture_client_size((HWND)1,&c);
    assert(c.client_width==320 && c.client_height==240 && window.base.stopping);
    assert(kvm_component_destroy(&window.base)==LIB_STATUS_OK);
    assert(kvm_component_initialize(&window.base,&options,join,dispose)==LIB_STATUS_OK);
    assert(kvm_component_mailboxes_select_notify(&window.base.mailboxes,
        LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    client_ok=1;
    c.frame.valid=1; c.frame.text_columns=80; c.frame.text_rows=25;
    c.frame.cursor_visible=1; c.frame.font_height=16;
    c.cursor_blink_due=250; c.cursor_blink_visible=1;
    for (ticks=0;ticks<250;++ticks) win32_window_proc((HWND)1,WM_TIMER,WIN32_WINDOW_CURSOR_TIMER,0);
    assert(c.cursor_blink_visible);
    win32_window_proc((HWND)1,WM_TIMER,WIN32_WINDOW_CURSOR_TIMER,0);
    assert(!c.cursor_blink_visible && c.cursor_blink_due==500);
    /* Delayed delivery preserves phase against the original 250ms grid. */
    c.cursor_blink_due=250; c.cursor_blink_visible=1;
    const DWORD delayed[]={260,500,750,1000};
    for (unsigned i=0;i<4;++i) {
        ticks=delayed[i];
        win32_window_proc((HWND)1,WM_TIMER,WIN32_WINDOW_CURSOR_TIMER,0);
        assert(c.cursor_blink_visible == (i % 2 != 0));
        assert(c.cursor_blink_due == (i+2)*250);
    }
    c.cursor_blink_visible=0; c.cursor_blink_due=500;
    /* A repeated unfreeze must not restart the native timer or reset phase. */
    unsigned starts_before=timer_starts, stops_before=timer_stops;
    for (ticks=300;ticks<500;ticks+=50) {
        assert(kvm_window_unfreeze(&window)==LIB_STATUS_OK);
        assert(win32_window_consume_mailboxes((HWND)1,&c));
        assert(!c.cursor_blink_visible && c.cursor_blink_due==500);
        assert(timer_starts==starts_before && timer_stops==stops_before);
    }
    c.frozen=1; ticks=500; win32_window_proc((HWND)1,WM_TIMER,WIN32_WINDOW_CURSOR_TIMER,0);
    assert(!c.cursor_blink_visible);
    c.frozen=0; c.cursor_blink_due=10; ticks=0xfffffff0u;
    win32_window_proc((HWND)1,WM_TIMER,WIN32_WINDOW_CURSOR_TIMER,0); assert(!c.cursor_blink_visible);
    ticks=10; win32_window_proc((HWND)1,WM_TIMER,WIN32_WINDOW_CURSOR_TIMER,0); assert(c.cursor_blink_visible);
    c.frame.cursor_visible=0; ticks=1000;
    win32_window_proc((HWND)1,WM_TIMER,WIN32_WINDOW_CURSOR_TIMER,0); assert(c.cursor_blink_visible);
    assert(kvm_window_unfreeze(&window)==LIB_STATUS_OK);
    assert(win32_window_consume_mailboxes((HWND)1,&c));
    assert(focus_requests==1 && foreground_requests==1 && !c.mouse.captured);
    title_ok=0;
    kvm_component_control command={.kind=KVM_COMPONENT_CONTROL_SET_WINDOW_TITLE};
    assert(kvm_component_mailboxes_enqueue_control(&window.base.mailboxes,&command)==0);
    assert(!win32_window_consume_mailboxes((HWND)1,&c) && window.base.stopping);
    assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);
    assert(kvm_component_initialize(&window.base,&options,join,dispose)==0);
    c.component=&window; c.frozen=1; title_ok=1; reenter_title=1;
    assert(kvm_component_mailboxes_select_notify(&window.base.mailboxes,
        immediate_notification,&c) == LIB_STATUS_OK);
    assert(kvm_component_mailboxes_select_notify(&window.base.mailboxes,
        immediate_notification,&c) == LIB_STATUS_INVALID_STATE);
    assert(!kvm_component_mailboxes_wake(&window.base.mailboxes));
    assert(kvm_window_set_title(&window,"reentrant notification")==0);
    assert(!c.frozen && !c.consuming && !window.base.mailboxes.control_count);
    assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);
    /* A release callback faults mid-FIFO: later controls and frame stay untouched. */
    assert(kvm_component_initialize(&window.base,&options,join,dispose)==0);
    assert(kvm_component_mailboxes_select_notify(&window.base.mailboxes,
        LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    c.component=&window; c.frozen=0; c.left_button=1;
    reject_input=1; foreground_requests=0;
    assert(kvm_window_release_mouse(&window)==0);
    assert(kvm_window_freeze(&window)==0);
    assert(kvm_window_unfreeze(&window)==0);
    assert(kvm_window_publish_frame(&window,&c.frame)==0);
    win32_window_proc((HWND)1,WIN32_WINDOW_MAILBOX_READY,0,0);
    assert(window.base.stopping && !c.frozen && !foreground_requests);
    assert(window.base.mailboxes.control_count==2);
    assert(window.base.mailboxes.frame_pending);
    assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);
    assert(kvm_component_initialize(&window.base,&options,join,dispose)==LIB_STATUS_OK);
    assert(kvm_component_mailboxes_select_notify(&window.base.mailboxes,
        LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    c.component=&window; c.left_button=c.right_button=0; reject_input=0;
    c.mouse.captured=LIB_TRUE; c.mouse.window=(HWND)1; owner=(HWND)1;
    release_ok=0;
    assert(kvm_window_freeze(&window)==LIB_STATUS_OK);
    assert(kvm_window_unfreeze(&window)==LIB_STATUS_OK);
    assert(!win32_window_consume_mailboxes((HWND)1,&c));
    assert(c.frozen && window.base.mailboxes.control_count==1);
    assert(window.base.failure==LIB_STATUS_IO_ERROR && window.base.stopping);
    assert(!c.mouse.captured);
    assert(kvm_component_destroy(&window.base)==LIB_STATUS_OK);
    for (unsigned edge=KVM_WINDOW_EDGE_LEFT;edge<=KVM_WINDOW_EDGE_BOTTOMRIGHT;++edge) {
        kvm_window_rect r={10,20,826,749};
        kvm_window_constrain_sizing(&r,(kvm_window_edge)edge,16,29,640,480);
        int w=r.right-r.left-16,h=r.bottom-r.top-29;
        assert((edge==KVM_WINDOW_EDGE_LEFT || edge==KVM_WINDOW_EDGE_RIGHT) ?
            (w==800 && h==600) : (w==933 && h==700));
        assert((edge==KVM_WINDOW_EDGE_LEFT || edge==KVM_WINDOW_EDGE_TOPLEFT || edge==KVM_WINDOW_EDGE_BOTTOMLEFT) ? r.right==826 : r.left==10);
        assert((edge==KVM_WINDOW_EDGE_TOP || edge==KVM_WINDOW_EDGE_TOPLEFT || edge==KVM_WINDOW_EDGE_TOPRIGHT) ? r.bottom==749 : r.top==20);
    }
    return 0;
}
