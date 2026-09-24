#include "lib/types/test.h"
#include "lib/types/win32/test.h"
#include "lib/types/file.h"
#include "lib/kvm-window/window.h"
#include "lib/types/win32/window.h"
#include "lib/types/win32/sync.h"

static lib_win32_hwnd owner, focused;
static lib_win32_rect client = {0,0,640,480}, clipped;
static lib_win32_point origin = {100,200};
static lib_win32_raw_input raw_record;
static lib_win32_raw_input_device raw_binding;
static lib_u32 raw_reads, raw_registrations, raw_removals;
static lib_i32 raw_read_ok=1, raw_register_ok=1, raw_query_ok=1, raw_remove_ok=1;
static lib_win32_uint raw_size=sizeof(lib_win32_raw_input);
static lib_i32 desktop_width=65535, desktop_height=65535;
static lib_win32_uint LIB_WIN32_WINAPI query_raw(PRAWINPUTDEVICE d,PUINT count,lib_win32_uint size)
{
    lib_test_assert(size==sizeof(*d));
    if (!raw_query_ok) return (lib_win32_uint)-1;
    lib_win32_uint needed=raw_binding.usUsage ? 1u : 0u;
    if (!d) { *count=needed; return 0; }
    lib_test_assert(*count>=needed);
    if (needed) *d=raw_binding;
    return needed;
}
static lib_win32_bool LIB_WIN32_WINAPI register_raw(PCRAWINPUTDEVICE d,lib_win32_uint count,lib_win32_uint size)
{
    lib_test_assert(count==1 && size==sizeof(*d) && d->usUsagePage==1 && d->usUsage==2);
    if (d->dwFlags==LIB_WIN32_RIDEV_REMOVE) {
        lib_test_assert(!d->hwndTarget); ++raw_removals;
        if (!raw_remove_ok) return LIB_WIN32_FALSE;
        raw_binding=(lib_win32_raw_input_device){0};
    } else {
        lib_test_assert(d->dwFlags==0 && d->hwndTarget==(lib_win32_hwnd)1); ++raw_registrations;
        if (!raw_register_ok) return LIB_WIN32_FALSE;
        raw_binding=*d;
    }
    return LIB_WIN32_TRUE;
}
static lib_win32_uint LIB_WIN32_WINAPI read_raw(lib_win32_hraw_input h,lib_win32_uint command,lib_win32_lpvoid data,PUINT size,lib_win32_uint header)
{
    lib_test_assert(h==(lib_win32_hraw_input)1 && command==LIB_WIN32_RID_INPUT && *size==sizeof(lib_win32_raw_input));
    lib_test_assert(header==sizeof(lib_win32_raw_input_header)); ++raw_reads;
    if (!raw_read_ok) return (lib_win32_uint)-1;
    *(lib_win32_raw_input *)data=raw_record; return raw_size;
}
static lib_i32 LIB_WIN32_WINAPI metrics(lib_i32 index)
{
    lib_test_assert(index==LIB_WIN32_SM_CXSCREEN || index==LIB_WIN32_SM_CYSCREEN ||
        index==LIB_WIN32_SM_CXVIRTUALSCREEN || index==LIB_WIN32_SM_CYVIRTUALSCREEN);
    return index==LIB_WIN32_SM_CXSCREEN || index==LIB_WIN32_SM_CXVIRTUALSCREEN ? desktop_width : desktop_height;
}
static lib_win32_bool LIB_WIN32_WINAPI get_clip(lib_win32_rect *r) { *r=clipped; return LIB_WIN32_TRUE; }
static lib_u32 releases, clips, events;
static lib_i32 reject_input;
static lib_i32 release_ok=1;
static lib_u32 focus_requests, foreground_requests;
static lib_i32 clip_ok = 1, resize_ok = 1, title_ok = 1, client_ok = 1;
static lib_win32_dword ticks;
static lib_u32 timer_starts, timer_stops;
static lib_i32 timer_ok = 1;
static lib_win32_uint_ptr LIB_WIN32_WINAPI start_timer(lib_win32_hwnd w,lib_win32_uint_ptr id,lib_win32_uint ms,lib_win32_timer_proc fn)
{ (void)w; lib_test_assert(id==1 && ms==250 && !fn); ++timer_starts; return timer_ok ? id : 0; }
static lib_win32_bool LIB_WIN32_WINAPI stop_timer(lib_win32_hwnd w,lib_win32_uint_ptr id)
{ (void)w; lib_test_assert(id==1); ++timer_stops; return timer_ok; }
static lib_i32 selection_ok;
static lib_u32 selections, deleted_bitmaps, deleted_dcs;
static lib_u32 surface_bits[64];
static lib_win32_hdc LIB_WIN32_WINAPI surface_dc(lib_win32_hwnd w) { (void)w; return (lib_win32_hdc)1; }
static lib_win32_hdc LIB_WIN32_WINAPI compatible_dc(lib_win32_hdc d) { (void)d; return (lib_win32_hdc)2; }
static lib_win32_hbitmap LIB_WIN32_WINAPI bitmap(lib_win32_hdc d,const lib_win32_bitmapinfo *i,lib_win32_uint u,void **p,lib_win32_handle s,lib_win32_dword o)
{ (void)d;(void)i;(void)u;(void)s;(void)o;*p=surface_bits;return (lib_win32_hbitmap)3; }
static lib_i32 LIB_WIN32_WINAPI release_dc(lib_win32_hwnd w,lib_win32_hdc d) { (void)w;(void)d;return 1; }
static lib_win32_hgdiobj LIB_WIN32_WINAPI select_bitmap(lib_win32_hdc d,lib_win32_hgdiobj o)
{ (void)d;lib_test_assert(o==(lib_win32_hgdiobj)3 || o==(lib_win32_hgdiobj)4);++selections;return selection_ok ? (lib_win32_hgdiobj)4 : LIB_NULL; }
static lib_win32_bool LIB_WIN32_WINAPI delete_bitmap(lib_win32_hgdiobj o) { lib_test_assert(o==(lib_win32_hgdiobj)3);++deleted_bitmaps;return LIB_WIN32_TRUE; }
static lib_win32_bool LIB_WIN32_WINAPI delete_dc(lib_win32_hdc d) { lib_test_assert(d==(lib_win32_hdc)2);++deleted_dcs;return LIB_WIN32_TRUE; }
static lib_win32_dword LIB_WIN32_WINAPI clock_tick(void) { return ticks; }
static lib_u32 invalidations;
static lib_win32_rect invalidated, previous_invalidated;
static lib_win32_bool LIB_WIN32_WINAPI invalidate(lib_win32_hwnd w,const lib_win32_rect *r,lib_win32_bool erase)
{ (void)w;lib_test_assert(!erase);++invalidations;previous_invalidated=invalidated;if(r)invalidated=*r;return LIB_WIN32_TRUE; }
static void *context;
static void notify_loss(void);
static lib_i32 reenter_title;
static void title_notification(void);
static lib_win32_hwnd LIB_WIN32_WINAPI get_capture(void) { return owner; }
static lib_win32_hwnd LIB_WIN32_WINAPI set_capture(lib_win32_hwnd w) { lib_win32_hwnd old=owner; owner=w; return old; }
static lib_win32_bool LIB_WIN32_WINAPI release_capture(void)
{ ++releases; owner=LIB_NULL; notify_loss(); return release_ok; }
static lib_win32_hwnd LIB_WIN32_WINAPI set_focus(lib_win32_hwnd w) { ++focus_requests; focused=w; return w; }
static lib_win32_bool LIB_WIN32_WINAPI foreground(lib_win32_hwnd w) { (void)w; ++foreground_requests; return LIB_WIN32_TRUE; }
static lib_win32_hwnd LIB_WIN32_WINAPI get_focus(void) { return focused; }
static lib_win32_bool LIB_WIN32_WINAPI clip(const lib_win32_rect *r)
{ ++clips; if (r) clipped=*r; return r ? clip_ok : LIB_WIN32_TRUE; }
static lib_win32_bool LIB_WIN32_WINAPI get_client(lib_win32_hwnd w, lib_win32_rect *r)
{ (void)w; if (!client_ok) return LIB_WIN32_FALSE; *r=client; return LIB_WIN32_TRUE; }
static lib_win32_bool LIB_WIN32_WINAPI to_screen(lib_win32_hwnd w, lib_win32_point *p)
{ (void)w; p->x+=origin.x; p->y+=origin.y; return LIB_WIN32_TRUE; }
static lib_win32_hcursor LIB_WIN32_WINAPI cursor(lib_win32_hcursor c) { return c; }
static lib_win32_long_ptr LIB_WIN32_WINAPI get_context(lib_win32_hwnd w, lib_i32 index)
{ (void)w; return index==LIB_WIN32_GWLP_USERDATA ? (lib_win32_long_ptr)context : 0; }
static lib_win32_bool LIB_WIN32_WINAPI resize(lib_win32_hwnd w, lib_win32_hwnd after, lib_i32 x,lib_i32 y,lib_i32 cx,lib_i32 cy,lib_win32_uint f)
{ (void)w;(void)after;(void)x;(void)y;(void)cx;(void)cy;(void)f;return resize_ok; }
static lib_win32_bool LIB_WIN32_WINAPI title(lib_win32_hwnd w,lib_win32_lpcstr text)
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
#undef lib_win32_get_clip_cursor
#define lib_win32_get_clip_cursor get_clip
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
#undef lib_win32_register_raw_input_devices
#undef lib_win32_get_raw_input_data
#define lib_win32_register_raw_input_devices register_raw
#define lib_win32_get_raw_input_data read_raw
#undef lib_win32_get_registered_raw_input_devices
#define lib_win32_get_registered_raw_input_devices query_raw
#undef lib_win32_get_system_metrics
#define lib_win32_get_system_metrics metrics
#include "lib/kvm-window/win32/mouse.c"
#include "lib/kvm-window/win32/geometry.c"
#include "lib/kvm-window/win32/component.c"

static void notify_loss(void) { win32_window_proc((lib_win32_hwnd)1,LIB_WIN32_WM_CAPTURECHANGED,0,0); }
static lib_status immediate_notification(void *p)
{ (void)p; win32_window_proc((lib_win32_hwnd)1,WIN32_WINDOW_MAILBOX_READY,0,0); return 0; }
static void title_notification(void)
{
    kvm_win32_window_context *c=context;
    reenter_title=0;
    lib_test_assert(c->consuming && c->frozen);
    lib_test_assert(kvm_window_unfreeze(c->component)==0);
    lib_test_assert(c->frozen); /* Nested notification must not drain ahead of this control. */
}
static lib_i32 input(void *p,const kvm_input_event *e)
{ (void)p; lib_test_assert(e->type==KVM_EVENT_MOUSE); ++events; return !reject_input; }
static void failure(void *p,lib_u64 id,lib_status status)
{ (void)p;(void)id;(void)status; }
static lib_status join(kvm_component *p, lib_u32 timeout_ms)
{ (void)p; (void)timeout_ms; return LIB_STATUS_OK; }
static void dispose(kvm_component *p) { kvm_component_mailboxes_destroy(&p->mailboxes); }
static lib_u32 control_failures;
static void invalid_control_failure(void *p, lib_u64 id, lib_status status)
{ (void)p; lib_test_assert(id && status==LIB_STATUS_INVALID_ARGUMENT); ++control_failures; }
static void check_invalid_controls(void)
{
    static kvm_window window;
    static kvm_win32_window_context c;
    kvm_component_options options={.input_sink=input,.failure_sink=invalid_control_failure};
    for(lib_u32 i=0;i<3;++i) {
        kvm_component_control command={.kind=LIB_UINT32_MAX};
        lib_test_assert(kvm_component_initialize(&window.base, &options, join, dispose,
        &window.pending_frame, sizeof(window.pending_frame))==0);
        lib_test_assert(kvm_component_mailboxes_select_notify(&window.base.mailboxes,LIB_NULL,LIB_NULL)==0);
        lib_memory_set(&c,0,sizeof(c)); c.component=&window;
        if(i==1) { command.kind=KVM_WINDOW_CONTROL_SET_TITLE;
            lib_memory_set(command.payload,'x',sizeof(command.payload)); }
        if(i==2) { command.kind=KVM_WINDOW_CONTROL_SET_FROZEN; command.payload[0]=2; }
        lib_test_assert(kvm_component_enqueue_control(&window.base,&command)==0);
        lib_test_assert(!win32_window_consume_mailboxes((lib_win32_hwnd)1,&c));
        lib_test_assert(window.base.stopping && window.base.mailboxes.closed);
        lib_test_assert(control_failures==i+1);
        lib_test_assert(kvm_component_destroy(&window.base)==0);
    }
}
static void check_surface_damage(void)
{
    static kvm_window window;
    static kvm_win32_window_context c;
    static kvm_window_frame frame;
    kvm_component_options options={.input_sink=input,.failure_sink=failure};
    lib_test_assert(kvm_component_initialize(&window.base,&options,join,dispose,
        &window.pending_frame,sizeof(window.pending_frame))==0);
    lib_test_assert(kvm_component_mailboxes_select_notify(&window.base.mailboxes,LIB_NULL,LIB_NULL)==0);
    c.component=&window;
    c.surface_dc=(lib_win32_hdc)2; c.surface_pixels=surface_bits;
    c.surface_width=c.surface_height=c.client_surface_width=c.client_surface_height=8;
    c.client_width=c.client_height=8;
    frame.valid=frame.graphics=1;
    frame.image.width=frame.image.stride=frame.image.height=8;
    frame.image.palette[1]=0x123456;
    lib_test_assert(kvm_window_publish_frame(&window,&frame)==0);
    win32_window_consume_frame((lib_win32_hwnd)1,&c);
    lib_test_assert(c.surface_valid && invalidated.right==8 && invalidated.bottom==8);
    lib_u32 before=invalidations;
    /* Two updates before LIB_WIN32_WM_PAINT must each invalidate, not replace the first. */
    frame.image.pixels[0]=1;
    lib_test_assert(kvm_window_publish_frame(&window,&frame)==0);
    win32_window_consume_frame((lib_win32_hwnd)1,&c);
    lib_test_assert(invalidated.left==0 && invalidated.top==0 && invalidated.right==1 && invalidated.bottom==1);
    frame.image.pixels[63]=1;
    lib_test_assert(kvm_window_publish_frame(&window,&frame)==0);
    win32_window_consume_frame((lib_win32_hwnd)1,&c);
    lib_test_assert(invalidations==before+2 && invalidated.left==7 && invalidated.top==7);
    lib_test_assert(surface_bits[0]==0x123456 && surface_bits[63]==0x123456);
    lib_test_assert(kvm_window_publish_frame(&window,&frame)==0);
    win32_window_consume_frame((lib_win32_hwnd)1,&c);
    lib_test_assert(invalidations==before+2);
    frame=(kvm_window_frame){.valid=1};
    frame.text.base.text_columns=frame.text.base.text_rows=1;
    frame.text.base.font_height=8; /* Same pixel dimensions as graphics. */
    lib_test_assert(kvm_window_publish_frame(&window,&frame)==0);
    win32_window_consume_frame((lib_win32_hwnd)1,&c);
    lib_test_assert(c.surface_valid && invalidations==before+3);
    frame=(kvm_window_frame){.valid=1,.graphics=1};
    frame.image.width=frame.image.stride=frame.image.height=8;
    lib_test_assert(kvm_window_publish_frame(&window,&frame)==0);
    win32_window_consume_frame((lib_win32_hwnd)1,&c);
    lib_test_assert(c.surface_valid && invalidations==before+3);
    lib_test_assert(invalidated.left==0 && invalidated.top==0 && invalidated.right==8 && invalidated.bottom==8);
    /* Same black bitmap, but a new cursor overlay must still invalidate. */
    frame=(kvm_window_frame){.valid=1};
    frame.text.base.text_columns=1; frame.text.base.text_rows=2;
    frame.text.base.font_height=4;
    frame.text.base.cursor_visible=1;
    frame.text.base.cursor_top=frame.text.base.cursor_bottom=3;
    c.cursor_blink_visible=1;
    before=invalidations;
    lib_test_assert(kvm_window_publish_frame(&window,&frame)==0);
    win32_window_consume_frame((lib_win32_hwnd)1,&c);
    lib_test_assert(invalidations==before+1 && invalidated.top==3 && invalidated.bottom==4);
    frame.text.base.cursor_row=1;
    lib_test_assert(kvm_window_publish_frame(&window,&frame)==0);
    win32_window_consume_frame((lib_win32_hwnd)1,&c);
    lib_test_assert(invalidations==before+3 && previous_invalidated.top==3 && previous_invalidated.bottom==4);
    lib_test_assert(invalidated.top==7 && invalidated.bottom==8);
    lib_test_assert(kvm_window_publish_frame(&window,&frame)==0);
    win32_window_consume_frame((lib_win32_hwnd)1,&c);
    lib_test_assert(invalidations==before+3); /* Identical bitmap and cursor. */
    c.cursor_blink_due=ticks+250;
    ticks+=250;
    win32_window_advance_cursor_blink((lib_win32_hwnd)1,&c);
    lib_test_assert(!c.cursor_blink_visible && invalidations==before+4 && invalidated.top==7);
    ticks+=250;
    win32_window_advance_cursor_blink((lib_win32_hwnd)1,&c);
    lib_test_assert(c.cursor_blink_visible && invalidations==before+5);
    frame.text.base.cursor_visible=0;
    lib_test_assert(kvm_window_publish_frame(&window,&frame)==0);
    win32_window_consume_frame((lib_win32_hwnd)1,&c);
    lib_test_assert(invalidations==before+6 && invalidated.top==7);
    /* Font-only and palette-only changes go through the same pixel damage. */
    frame.text.base.cells[0].foreground=1;
    frame.text.base.text_palette[1]=0x123456;
    frame.text.font[0]=0x80;
    lib_test_assert(kvm_window_publish_frame(&window,&frame)==0);
    win32_window_consume_frame((lib_win32_hwnd)1,&c);
    lib_test_assert(invalidations==before+7 && invalidated.left==0 && invalidated.top==0 &&
        invalidated.right==1 && invalidated.bottom==1 && surface_bits[0]==0x123456);
    frame.text.base.text_palette[1]=0x654321;
    lib_test_assert(kvm_window_publish_frame(&window,&frame)==0);
    win32_window_consume_frame((lib_win32_hwnd)1,&c);
    lib_test_assert(invalidations==before+8 && surface_bits[0]==0x654321);
    frame.text.font[0]=0;
    lib_test_assert(kvm_window_publish_frame(&window,&frame)==0);
    win32_window_consume_frame((lib_win32_hwnd)1,&c);
    lib_test_assert(invalidations==before+9 && surface_bits[0]==0);
    /* Text A is skipped by latest-wins; B's full bitmap contains both edits. */
    frame.text.font[0]=0x80;
    lib_test_assert(kvm_window_publish_frame(&window,&frame)==0);
    frame.text.base.cells[KVM_TEXT_COLUMNS].foreground=1;
    lib_test_assert(kvm_window_publish_frame(&window,&frame)==0);
    win32_window_consume_frame((lib_win32_hwnd)1,&c);
    lib_test_assert(invalidations==before+10 && invalidated.left==0 && invalidated.top==0 &&
        invalidated.right==1 && invalidated.bottom==5);
    lib_test_assert(surface_bits[0]==0x654321 && surface_bits[32]==0x654321);
    c.surface_valid=0; /* Recreated surface must invalidate fully, even same lib_win32_rgb. */
    lib_test_assert(kvm_window_publish_frame(&window,&frame)==0);
    win32_window_consume_frame((lib_win32_hwnd)1,&c);
    lib_test_assert(invalidations==before+11 && invalidated.right==8 && invalidated.bottom==8);
    lib_test_assert(kvm_component_destroy(&window.base)==0);
}
int main(void)
{
    check_invalid_controls();
    static kvm_win32_window_context surface;
    lib_test_assert(!win32_window_ensure_surface((lib_win32_hwnd)1,&surface,8,8));
    lib_test_assert(!surface.surface_width && !surface.surface_height && !surface.surface_dc && !surface.surface_pixels);
    lib_test_assert(selections==1 && deleted_bitmaps==1 && deleted_dcs==1);
    selection_ok=1;
    lib_test_assert(win32_window_ensure_surface((lib_win32_hwnd)1,&surface,8,8));
    lib_test_assert(surface.surface_width==8 && selections==2);
    win32_window_destroy_surface(&surface);
    lib_test_assert(selections==3 && deleted_bitmaps==2 && deleted_dcs==2);
    static kvm_window window;
    static kvm_win32_window_context c;
    kvm_component_options options={.input_sink=input,.failure_sink=failure};
    lib_test_assert(kvm_component_initialize(&window.base, &options, join, dispose,
        &window.pending_frame, sizeof(window.pending_frame))==LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_select_notify(&window.base.mailboxes,
        LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    c.component=&window; context=&c;
    lib_test_assert(kvm_win32_mouse_refresh_bounds(&c.mouse) && clips==0);
    lib_test_assert(kvm_win32_mouse_capture(&c.mouse,(lib_win32_hwnd)1) == LIB_STATUS_OK);
    lib_test_assert(clipped.left==100 && clipped.top==200 && clipped.right==740 && clipped.bottom==680);
    origin.x=-300; origin.y=50;
    win32_window_proc((lib_win32_hwnd)1,LIB_WIN32_WM_MOVE,0,0);
    lib_test_assert(clipped.left==-300 && clipped.top==50);
    client.right=320; client.bottom=240;
    win32_window_proc((lib_win32_hwnd)1,LIB_WIN32_WM_SIZE,0,0);
    lib_test_assert(clipped.right==20 && clipped.bottom==290);
    c.left_button=1; c.mouse.motion.remainder_x=7;
    owner=(lib_win32_hwnd)2; notify_loss();
    lib_test_assert(!c.mouse.captured && !c.left_button && !c.mouse.motion.valid);
    lib_test_assert(!c.mouse.motion.remainder_x && releases==0 && events==1 && owner==(lib_win32_hwnd)2);
    notify_loss(); lib_test_assert(events==1 && releases==0);
    lib_test_assert(kvm_win32_mouse_capture(&c.mouse,(lib_win32_hwnd)1) == LIB_STATUS_OK);
    c.right_button=1; win32_window_release_mouse(&c);
    lib_test_assert(releases==1 && events==2 && !c.mouse.captured && !c.right_button);
    lib_test_assert(kvm_win32_mouse_capture(&c.mouse,(lib_win32_hwnd)1) == LIB_STATUS_OK);
    clip_ok=0; win32_window_proc((lib_win32_hwnd)1,LIB_WIN32_WM_MOVE,0,0);
    lib_test_assert(!c.mouse.captured && releases==2);
    lib_u32 previous=clips;
    win32_window_proc((lib_win32_hwnd)1,LIB_WIN32_WM_MOVE,0,0); lib_test_assert(clips==previous);
    resize_ok=0;
    win32_window_resize_client((lib_win32_hwnd)1,&c,640,480);
    lib_test_assert(c.client_surface_width==0 && window.base.stopping);
    lib_test_assert(kvm_component_destroy(&window.base)==LIB_STATUS_OK);
    lib_test_assert(kvm_component_initialize(&window.base, &options, join, dispose,
        &window.pending_frame, sizeof(window.pending_frame))==LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_select_notify(&window.base.mailboxes,
        LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    resize_ok=1;
    win32_window_resize_client((lib_win32_hwnd)1,&c,640,480);
    lib_test_assert(c.client_surface_width==640 && c.client_surface_height==480);
    focus_requests=foreground_requests=0;
    lib_test_assert(kvm_window_unfreeze(&window)==LIB_STATUS_OK);
    lib_test_assert(win32_window_consume_mailboxes((lib_win32_hwnd)1,&c));
    lib_test_assert(focus_requests==0 && foreground_requests==0);
    clip_ok=1;
    lib_test_assert(kvm_win32_mouse_capture(&c.mouse,(lib_win32_hwnd)1)==LIB_STATUS_OK);
    focus_requests=foreground_requests=0;
    lib_u32 releases_before_freeze=releases;
    lib_test_assert(kvm_window_freeze(&window)==LIB_STATUS_OK);
    lib_test_assert(c.mouse.captured && !c.frozen);
    lib_test_assert(win32_window_consume_mailboxes((lib_win32_hwnd)1,&c));
    lib_test_assert(c.frozen && !c.mouse.captured && releases==releases_before_freeze+1);
    lib_test_assert(focus_requests==0 && foreground_requests==0);
    lib_test_assert(kvm_window_unfreeze(&window)==LIB_STATUS_OK);
    lib_test_assert(win32_window_consume_mailboxes((lib_win32_hwnd)1,&c));
    lib_test_assert(focus_requests==1 && foreground_requests==1 && !c.mouse.captured);
    c.client_width=320; c.client_height=240; client_ok=0;
    win32_window_capture_client_size((lib_win32_hwnd)1,&c);
    lib_test_assert(c.client_width==320 && c.client_height==240 && window.base.stopping);
    lib_test_assert(kvm_component_destroy(&window.base)==LIB_STATUS_OK);
    lib_test_assert(kvm_component_initialize(&window.base, &options, join, dispose,
        &window.pending_frame, sizeof(window.pending_frame))==LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_select_notify(&window.base.mailboxes,
        LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    client_ok=1;
    c.frame.valid=1; c.frame.text.base.text_columns=80; c.frame.text.base.text_rows=25;
    c.frame.text.base.cursor_visible=1; c.frame.text.base.font_height=16;
    c.cursor_blink_due=250; c.cursor_blink_visible=1;
    for (ticks=0;ticks<250;++ticks) win32_window_proc((lib_win32_hwnd)1,LIB_WIN32_WM_TIMER,WIN32_WINDOW_CURSOR_TIMER,0);
    lib_test_assert(c.cursor_blink_visible);
    win32_window_proc((lib_win32_hwnd)1,LIB_WIN32_WM_TIMER,WIN32_WINDOW_CURSOR_TIMER,0);
    lib_test_assert(!c.cursor_blink_visible && c.cursor_blink_due==500);
    /* Delayed delivery preserves phase against the original 250ms grid. */
    c.cursor_blink_due=250; c.cursor_blink_visible=1;
    const lib_win32_dword delayed[]={260,500,750,1000};
    for (lib_u32 i=0;i<4;++i) {
        ticks=delayed[i];
        win32_window_proc((lib_win32_hwnd)1,LIB_WIN32_WM_TIMER,WIN32_WINDOW_CURSOR_TIMER,0);
        lib_test_assert(c.cursor_blink_visible == (i % 2 != 0));
        lib_test_assert(c.cursor_blink_due == (i+2)*250);
    }
    c.cursor_blink_visible=0; c.cursor_blink_due=500;
    /* A repeated unfreeze must not restart the native timer or reset phase. */
    lib_u32 starts_before=timer_starts, stops_before=timer_stops;
    for (ticks=300;ticks<500;ticks+=50) {
        lib_test_assert(kvm_window_unfreeze(&window)==LIB_STATUS_OK);
        lib_test_assert(win32_window_consume_mailboxes((lib_win32_hwnd)1,&c));
        lib_test_assert(!c.cursor_blink_visible && c.cursor_blink_due==500);
        lib_test_assert(timer_starts==starts_before && timer_stops==stops_before);
    }
    c.frozen=1; ticks=500; win32_window_proc((lib_win32_hwnd)1,LIB_WIN32_WM_TIMER,WIN32_WINDOW_CURSOR_TIMER,0);
    lib_test_assert(!c.cursor_blink_visible);
    c.frozen=0; c.cursor_blink_due=10; ticks=0xfffffff0u;
    win32_window_proc((lib_win32_hwnd)1,LIB_WIN32_WM_TIMER,WIN32_WINDOW_CURSOR_TIMER,0); lib_test_assert(!c.cursor_blink_visible);
    ticks=10; win32_window_proc((lib_win32_hwnd)1,LIB_WIN32_WM_TIMER,WIN32_WINDOW_CURSOR_TIMER,0); lib_test_assert(c.cursor_blink_visible);
    c.frame.text.base.cursor_visible=0; ticks=1000;
    win32_window_proc((lib_win32_hwnd)1,LIB_WIN32_WM_TIMER,WIN32_WINDOW_CURSOR_TIMER,0); lib_test_assert(c.cursor_blink_visible);
    lib_test_assert(kvm_window_unfreeze(&window)==LIB_STATUS_OK);
    lib_test_assert(win32_window_consume_mailboxes((lib_win32_hwnd)1,&c));
    lib_test_assert(focus_requests==1 && foreground_requests==1 && !c.mouse.captured);
    title_ok=0;
    kvm_component_control command={.kind=KVM_WINDOW_CONTROL_SET_TITLE};
    lib_test_assert(kvm_component_mailboxes_enqueue_control(&window.base.mailboxes,&command)==0);
    lib_test_assert(!win32_window_consume_mailboxes((lib_win32_hwnd)1,&c) && window.base.stopping);
    lib_test_assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_initialize(&window.base, &options, join, dispose,
        &window.pending_frame, sizeof(window.pending_frame))==0);
    c.component=&window; c.frozen=1; title_ok=1; reenter_title=1;
    lib_test_assert(kvm_component_mailboxes_select_notify(&window.base.mailboxes,
        immediate_notification,&c) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_select_notify(&window.base.mailboxes,
        immediate_notification,&c) == LIB_STATUS_INVALID_STATE);
    lib_test_assert(!window.base.mailboxes.wake);
    lib_test_assert(kvm_window_set_title(&window,"reentrant notification")==0);
    lib_test_assert(!c.frozen && !c.consuming && !window.base.mailboxes.control_count);
    lib_test_assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);
    /* A release callback faults mid-FIFO: later controls and frame stay untouched. */
    lib_test_assert(kvm_component_initialize(&window.base, &options, join, dispose,
        &window.pending_frame, sizeof(window.pending_frame))==0);
    lib_test_assert(kvm_component_mailboxes_select_notify(&window.base.mailboxes,
        LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    c.component=&window; c.frozen=0; c.left_button=1;
    reject_input=1; foreground_requests=0;
    lib_test_assert(kvm_window_release_mouse(&window)==0);
    lib_test_assert(kvm_window_freeze(&window)==0);
    lib_test_assert(kvm_window_unfreeze(&window)==0);
    lib_test_assert(kvm_window_publish_frame(&window,&c.frame)==0);
    win32_window_proc((lib_win32_hwnd)1,WIN32_WINDOW_MAILBOX_READY,0,0);
    lib_test_assert(window.base.stopping && !c.frozen && !foreground_requests);
    lib_test_assert(window.base.mailboxes.control_count==2);
    lib_test_assert(window.base.mailboxes.frame_pending);
    lib_test_assert(kvm_component_destroy(&window.base) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_initialize(&window.base, &options, join, dispose,
        &window.pending_frame, sizeof(window.pending_frame))==LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_select_notify(&window.base.mailboxes,
        LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    c.component=&window; c.left_button=c.right_button=0; reject_input=0;
    c.mouse.captured=LIB_TRUE; c.mouse.window=(lib_win32_hwnd)1; owner=(lib_win32_hwnd)1;
    release_ok=0;
    lib_test_assert(kvm_window_freeze(&window)==LIB_STATUS_OK);
    lib_test_assert(kvm_window_unfreeze(&window)==LIB_STATUS_OK);
    lib_test_assert(!win32_window_consume_mailboxes((lib_win32_hwnd)1,&c));
    lib_test_assert(c.frozen && window.base.mailboxes.control_count==1);
    lib_test_assert(window.base.failure==LIB_STATUS_IO_ERROR && window.base.stopping);
    lib_test_assert(!c.mouse.captured);
    lib_test_assert(kvm_component_destroy(&window.base)==LIB_STATUS_OK);
    for (lib_u32 edge=KVM_WINDOW_EDGE_LEFT;edge<=KVM_WINDOW_EDGE_BOTTOMRIGHT;++edge) {
        kvm_window_rect r={10,20,826,749};
        kvm_window_constrain_sizing(&r,(kvm_window_edge)edge,16,29,640,480);
        lib_i32 w=r.right-r.left-16,h=r.bottom-r.top-29;
        lib_test_assert((edge==KVM_WINDOW_EDGE_LEFT || edge==KVM_WINDOW_EDGE_RIGHT) ?
            (w==800 && h==600) : (w==934 && h==700));
        lib_i32 fitted_w,fitted_h;
        lib_test_assert(kvm_window_fit_aspect_size(w,h,640,480,&fitted_w,&fitted_h));
        lib_test_assert(fitted_w==w && fitted_h==h);
        lib_test_assert((edge==KVM_WINDOW_EDGE_LEFT || edge==KVM_WINDOW_EDGE_TOPLEFT || edge==KVM_WINDOW_EDGE_BOTTOMLEFT) ? r.right==826 : r.left==10);
        lib_test_assert((edge==KVM_WINDOW_EDGE_TOP || edge==KVM_WINDOW_EDGE_TOPLEFT || edge==KVM_WINDOW_EDGE_TOPRIGHT) ? r.bottom==749 : r.top==20);
    }
    /* Relative packets continue even at a clipped pointer edge, without warps. */
    release_ok=1; clip_ok=1;
    client.right=640; client.bottom=480; origin.x=-900; origin.y=80;
    lib_test_assert(kvm_component_initialize(&window.base, &options, join, dispose,
        &window.pending_frame, sizeof(window.pending_frame))==0);
    c.component=&window; c.frozen=0; c.left_button=c.right_button=0;
    c.client_width=c.surface_width=640; c.client_height=c.surface_height=480;
    raw_record.header.dwType=LIB_WIN32_RIM_TYPEMOUSE;
    raw_record.header.hDevice=(lib_win32_handle)1;
    lib_test_assert(kvm_win32_mouse_capture(&c.mouse,(lib_win32_hwnd)1)==0);
    lib_test_assert(raw_registrations>0 && raw_binding.hwndTarget==(lib_win32_hwnd)1);
    lib_i32 dx,dy, total=0;
    for(lib_u32 i=0;i<100;++i) {
        raw_record.data.mouse.lLastX=20; raw_record.data.mouse.lLastY=-10;
        lib_test_assert(kvm_win32_mouse_move(&c.mouse,1,640,480,640,480,&dx,&dy));
        lib_test_assert(dx==20 && dy==-10); total+=dx;
    }
    lib_test_assert(total==2000);
    raw_record.data.mouse.lLastX=1; raw_record.data.mouse.lLastY=0;
    for(lib_u32 i=0;i<2;++i) {
        lib_test_assert(kvm_win32_mouse_move(&c.mouse,1,1280,960,640,480,&dx,&dy));
        lib_test_assert(dx==(lib_i32)i && dy==0);
    }
    raw_record.data.mouse.lLastX=-1;
    lib_test_assert(kvm_win32_mouse_move(&c.mouse,1,1280,960,640,480,&dx,&dy) && dx==0);
    lib_test_assert(kvm_win32_mouse_move(&c.mouse,1,1280,960,640,480,&dx,&dy) && dx==-1);
    /* Absolute samples: first anchors, repeated positions are zero, left stays left. */
    raw_record.data.mouse.usFlags=LIB_WIN32_MOUSE_MOVE_ABSOLUTE | LIB_WIN32_MOUSE_VIRTUAL_DESKTOP;
    raw_record.data.mouse.lLastX=40209; raw_record.data.mouse.lLastY=30969;
    lib_test_assert(kvm_win32_mouse_move(&c.mouse,1,640,480,640,480,&dx,&dy) && dx==0 && dy==0);
    lib_test_assert(kvm_win32_mouse_move(&c.mouse,1,640,480,640,480,&dx,&dy) && dx==0 && dy==0);
    raw_record.data.mouse.lLastX=40072;
    lib_test_assert(kvm_win32_mouse_move(&c.mouse,1,640,480,640,480,&dx,&dy) && dx==-137 && dy==0);
    raw_record.data.mouse.lLastX=39936;
    lib_test_assert(kvm_win32_mouse_move(&c.mouse,1,640,480,640,480,&dx,&dy) && dx==-136 && dy==0);
    lib_test_assert(kvm_win32_mouse_move(&c.mouse,1,640,480,640,480,&dx,&dy) && dx==0 && dy==0);
    raw_record.data.mouse.lLastX+=10; raw_record.data.mouse.lLastY-=10;
    lib_test_assert(kvm_win32_mouse_move(&c.mouse,1,320,240,640,480,&dx,&dy) && dx==20 && dy==-20);
    /* Device, absolute coordinate space and desktop size changes cannot jump. */
    raw_record.header.hDevice=(lib_win32_handle)2; raw_record.data.mouse.lLastX=100;
    lib_test_assert(kvm_win32_mouse_move(&c.mouse,1,640,480,640,480,&dx,&dy) && dx==0 && dy==0);
    raw_record.data.mouse.usFlags=LIB_WIN32_MOUSE_MOVE_ABSOLUTE;
    raw_record.data.mouse.lLastX=500;
    lib_test_assert(kvm_win32_mouse_move(&c.mouse,1,640,480,640,480,&dx,&dy) && dx==0 && dy==0);
    desktop_width=1920; desktop_height=1080;
    raw_record.data.mouse.lLastX=32768; raw_record.data.mouse.lLastY=32768;
    lib_test_assert(kvm_win32_mouse_move(&c.mouse,1,640,480,640,480,&dx,&dy) && dx==0 && dy==0);
    raw_record.data.mouse.lLastX=65535; raw_record.data.mouse.lLastY=65535;
    lib_test_assert(kvm_win32_mouse_move(&c.mouse,1,640,480,640,480,&dx,&dy) && dx==960 && dy==540);
    raw_record.data.mouse.lLastX=65536;
    lib_test_assert(!kvm_win32_mouse_move(&c.mouse,1,640,480,640,480,&dx,&dy));
    raw_record.data.mouse.lLastX=100;
    client.right=320; origin.x=100;
    lib_test_assert(kvm_win32_mouse_refresh_bounds(&c.mouse));
    lib_test_assert(kvm_win32_mouse_move(&c.mouse,1,320,480,640,480,&dx,&dy) && dx==0 && dy==0);
    lib_test_assert(kvm_win32_mouse_release(&c.mouse)==0 && raw_binding.usUsage==0);
    lib_test_assert(kvm_win32_mouse_capture(&c.mouse,(lib_win32_hwnd)1)==0);
    raw_record.data.mouse.lLastX=30000;
    lib_test_assert(kvm_win32_mouse_move(&c.mouse,1,320,480,640,480,&dx,&dy) && dx==0 && dy==0);
    raw_record.data.mouse.usFlags=0; raw_record.data.mouse.lLastX=4; raw_record.data.mouse.lLastY=-2;
    lib_test_assert(kvm_win32_mouse_move(&c.mouse,1,640,480,640,480,&dx,&dy) && dx==4 && dy==-2);
    /* Legacy coordinates never deliver motion; LIB_WIN32_WM_INPUT is the one entry. */
    lib_u32 before=events, reads_before=raw_reads;
    win32_window_proc((lib_win32_hwnd)1,LIB_WIN32_WM_MOUSEMOVE,0,lib_win32_make_lparam(32767,32767));
    lib_test_assert(events==before && raw_reads==reads_before);
    win32_window_proc((lib_win32_hwnd)1,LIB_WIN32_WM_INPUT,0,1);
    win32_window_flush_mouse(&c);
    lib_test_assert(events==before+1 && raw_reads==reads_before+1);
    /* Buttons retain their legacy route and do not re-read/double motion. */
    reads_before=raw_reads; before=events;
    win32_window_proc((lib_win32_hwnd)1,LIB_WIN32_WM_LBUTTONDOWN,0,0);
    win32_window_proc((lib_win32_hwnd)1,LIB_WIN32_WM_LBUTTONUP,0,0);
    lib_test_assert(events==before+2 && raw_reads==reads_before);
    /* Clipping loss is detected on the next raw packet; no later content. */
    clipped.right+=100;
    win32_window_proc((lib_win32_hwnd)1,LIB_WIN32_WM_INPUT,0,1);
    lib_test_assert(!c.mouse.captured && raw_binding.usUsage==0);
    before=events;
    win32_window_proc((lib_win32_hwnd)1,LIB_WIN32_WM_INPUT,0,1);
    lib_test_assert(events==before);
    lib_test_assert(kvm_win32_mouse_capture(&c.mouse,(lib_win32_hwnd)1)==0);
    win32_window_proc((lib_win32_hwnd)1,LIB_WIN32_WM_ACTIVATEAPP,LIB_WIN32_FALSE,0);
    lib_test_assert(!c.mouse.captured);
    lib_test_assert(kvm_win32_mouse_capture(&c.mouse,(lib_win32_hwnd)1)==0);
    raw_read_ok=0;
    win32_window_proc((lib_win32_hwnd)1,LIB_WIN32_WM_INPUT,0,1);
    lib_test_assert(!c.mouse.captured && events==before);
    raw_read_ok=1;
    lib_test_assert(kvm_win32_mouse_capture(&c.mouse,(lib_win32_hwnd)1)==0);
    raw_size=sizeof(lib_win32_raw_input_header);
    lib_test_assert(!kvm_win32_mouse_move(&c.mouse,1,640,480,640,480,&dx,&dy));
    raw_size=sizeof(lib_win32_raw_input);
    lib_test_assert(kvm_win32_mouse_release(&c.mouse)==0);
    /* Registration failures/foreign consumers are never stolen or hidden. */
    raw_query_ok=0;
    lib_test_assert(kvm_win32_mouse_capture(&c.mouse,(lib_win32_hwnd)1)==LIB_STATUS_IO_ERROR && !c.mouse.captured);
    raw_query_ok=1;
    raw_binding=(lib_win32_raw_input_device){1,2,0,(lib_win32_hwnd)2};
    lib_u32 registrations_before=raw_registrations;
    lib_test_assert(kvm_win32_mouse_capture(&c.mouse,(lib_win32_hwnd)1)==LIB_STATUS_INVALID_STATE);
    lib_test_assert(raw_binding.hwndTarget==(lib_win32_hwnd)2 && raw_registrations==registrations_before);
    raw_binding=(lib_win32_raw_input_device){0};
    raw_register_ok=0;
    lib_test_assert(kvm_win32_mouse_capture(&c.mouse,(lib_win32_hwnd)1)==LIB_STATUS_IO_ERROR && !c.mouse.captured);
    raw_register_ok=1;
    lib_test_assert(kvm_win32_mouse_capture(&c.mouse,(lib_win32_hwnd)1)==0);
    raw_binding.hwndTarget=(lib_win32_hwnd)2;
    lib_u32 removals_before=raw_removals;
    lib_test_assert(kvm_win32_mouse_release(&c.mouse)==0);
    lib_test_assert(raw_binding.hwndTarget==(lib_win32_hwnd)2 && raw_removals==removals_before);
    raw_binding=(lib_win32_raw_input_device){0};
    lib_test_assert(kvm_win32_mouse_capture(&c.mouse,(lib_win32_hwnd)1)==0);
    raw_remove_ok=0;
    lib_test_assert(kvm_win32_mouse_release(&c.mouse)==LIB_STATUS_IO_ERROR && !c.mouse.captured);
    raw_remove_ok=1; raw_binding=(lib_win32_raw_input_device){0};
    lib_test_assert(kvm_component_destroy(&window.base)==0);
    check_surface_damage();
    return 0;
}
