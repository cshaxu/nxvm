#include "lib/types/test.h"
#include "lib/types/win32/test.h"
#include "lib/types/win32/console.h"
#include "lib/types/win32/sync.h"
#include "lib/console/binding_interface.h"

static const char *input;
static lib_u32 chunk = 7, reads, cancel_at;
static lib_win32_handle stop;
static lib_win32_bool LIB_WIN32_WINAPI read_chunk(lib_win32_handle h, lib_win32_lpvoid bytes, lib_win32_dword capacity, lib_win32_lpdword count, lib_win32_lpvoid p)
{
    lib_u32 n = 0;
    (void)h; (void)p;
    while (input[n] && n < capacity && n < chunk) {
        ((char *)bytes)[n] = input[n];
        if (input[n++] == '\n') break;
    }
    input += n; *count = n;
    if (++reads == cancel_at) lib_win32_set_event(stop);
    return LIB_WIN32_TRUE;
}
static lib_u32 palette_attempts, palette_sets, writes;
static lib_i32 partial_write;
static lib_i32 text_result=1;
static lib_win32_dword text_written=1;
static lib_win32_bool LIB_WIN32_WINAPI text_write(lib_win32_handle h,lib_win32_lpcvoid text,lib_win32_dword n,lib_win32_lpdword written,lib_win32_lpvoid r)
{ (void)h;(void)text;(void)r;*written=n==0 ? 0 : text_written;return text_result; }
static lib_i32 palette_query_ok, palette_set_ok, cursor_ok = 1;
static lib_win32_wchar first_cell;
static lib_win32_word first_attribute;
static lib_win32_char_info captured_cells[80u * 50u];
static lib_win32_small_rect captured_region;
static lib_win32_coord buffer_size={80,25};
static lib_win32_small_rect viewport={0,0,79,24};
static lib_bool reject_viewport;
static lib_win32_bool LIB_WIN32_WINAPI screen_info(lib_win32_handle h, lib_win32_console_screen_buffer_info *p)
{ (void)h; lib_memory_set(p, 0, sizeof(*p)); p->dwSize=buffer_size; p->srWindow=viewport; return LIB_WIN32_TRUE; }
static lib_win32_bool LIB_WIN32_WINAPI set_viewport(lib_win32_handle h,lib_win32_bool absolute,const lib_win32_small_rect *rect)
{ (void)h; lib_test_assert(absolute); if(reject_viewport) return LIB_WIN32_FALSE; viewport=*rect; return LIB_WIN32_TRUE; }
static lib_win32_bool LIB_WIN32_WINAPI resize_buffer(lib_win32_handle h,lib_win32_coord size)
{ (void)h; buffer_size=size; return LIB_WIN32_TRUE; }
static lib_win32_bool LIB_WIN32_WINAPI palette_get(lib_win32_handle h, lib_win32_console_screen_buffer_infoex *p)
{ (void)h; (void)p; ++palette_attempts; return palette_query_ok; }
static lib_win32_bool LIB_WIN32_WINAPI palette_set(lib_win32_handle h, lib_win32_console_screen_buffer_infoex *p)
{ (void)h; (void)p; ++palette_sets; if(palette_set_ok) buffer_size.Y=24; return palette_set_ok; }
static lib_win32_bool LIB_WIN32_WINAPI write_cells(lib_win32_handle h, const lib_win32_char_info *p, lib_win32_coord a, lib_win32_coord b, lib_win32_small_rect *r)
{
    (void)h; (void)a; (void)b; ++writes; first_cell=p[0].Char.UnicodeChar;
    first_attribute=p[0].Attributes;
    captured_region=*r;
    lib_memory_copy(captured_cells,p,(lib_size)a.X*a.Y*sizeof(*p));
    if (r->Bottom>=buffer_size.Y) r->Bottom=buffer_size.Y-1;
    if (partial_write==1) r->Right=39;
    if (partial_write==2) r->Bottom=11;
    if (partial_write==3) r->Left=1;
    if (partial_write==4) r->Top=1;
    return partial_write!=5;
}
static lib_win32_console_cursor_info last_cursor;
static lib_win32_bool LIB_WIN32_WINAPI cursor_info(lib_win32_handle h, const lib_win32_console_cursor_info *p)
{ (void)h; last_cursor=*p; return cursor_ok; }
static lib_win32_bool LIB_WIN32_WINAPI cursor_position(lib_win32_handle h, lib_win32_coord p)
{ (void)h; (void)p; return cursor_ok; }
static lib_u32 readers_started, mode_sets;
static lib_win32_bool LIB_WIN32_WINAPI set_mode(lib_win32_handle h, lib_win32_dword mode) { (void)h; (void)mode; ++mode_sets; return LIB_WIN32_TRUE; }
static lib_win32_bool LIB_WIN32_WINAPI flush_input(lib_win32_handle h) { (void)h; return LIB_WIN32_TRUE; }
static lib_win32_handle LIB_WIN32_WINAPI start_reader(lib_win32_lpsecurity_attributes a, lib_win32_size_t size,
    lib_win32_thread_start_routine entry, lib_win32_lpvoid arg, lib_win32_dword flags, lib_win32_lpdword id)
{
    (void)a; (void)size; (void)entry; (void)arg; (void)flags; (void)id;
    ++readers_started;
    return lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_TRUE, LIB_NULL); /* deterministic joined worker */
}
#undef lib_win32_create_thread
#define lib_win32_create_thread start_reader
#undef lib_win32_set_console_mode
#define lib_win32_set_console_mode set_mode
#undef lib_win32_flush_console_input_buffer
#define lib_win32_flush_console_input_buffer flush_input
#undef lib_win32_read_console_a
#define lib_win32_read_console_a read_chunk
#undef lib_win32_write_console_a
#define lib_win32_write_console_a text_write
#undef lib_win32_get_console_screen_buffer_info
#define lib_win32_get_console_screen_buffer_info screen_info
#undef lib_win32_set_console_screen_buffer_size
#define lib_win32_set_console_screen_buffer_size resize_buffer
#undef lib_win32_set_console_window_info
#define lib_win32_set_console_window_info set_viewport
#undef lib_win32_get_console_screen_buffer_info_ex
#define lib_win32_get_console_screen_buffer_info_ex palette_get
#undef lib_win32_set_console_screen_buffer_info_ex
#define lib_win32_set_console_screen_buffer_info_ex palette_set
#undef lib_win32_write_console_output_w
#define lib_win32_write_console_output_w write_cells
#undef lib_win32_set_console_cursor_info
#define lib_win32_set_console_cursor_info cursor_info
#undef lib_win32_set_console_cursor_position
#define lib_win32_set_console_cursor_position cursor_position
#include "lib/console-broker/win32/console.c"

static lib_u32 delivered;
static lib_console_event received;
static void receive(void *p, const lib_console_event *event)
{ (void)p; received=*event; ++delivered; }
static void cooked_restore(void)
{
    console_broker_backend b={0};
    lib_console *c;
    lib_bool pending = LIB_FALSE;
    lib_test_assert(lib_console_create(&c)==LIB_STATUS_OK);
    lib_test_assert(lib_console_bind_generation(c,1)==LIB_STATUS_OK);
    lib_test_assert(lib_console_set_event_sink(c,receive,LIB_NULL)==LIB_STATUS_OK);
    lib_test_assert(console_broker_backend_activate(&b,c,CONSOLE_BROKER_COOKED_LINES,1,0)==0);
    lib_test_assert(!b.reader && !b.cooked_line_pending && readers_started==0);
    lib_test_assert(console_broker_backend_request_cooked_line(&b)==0);
    lib_test_assert(b.reader && b.cooked_line_pending && readers_started==1);
    /* Cancellation preserves an unfinished request until join. */
    stop=b.stop_event; input="discarded\r\n"; reads=0; cancel_at=1;
    console_broker_reader(&b); cancel_at=0;
    lib_test_assert(b.cooked_line_pending);
    lib_test_assert(console_broker_backend_deactivate(&b,&pending)==0 && pending);
    lib_test_assert(!b.reader && !b.cooked_line_pending);
    lib_test_assert(console_broker_backend_activate(&b,c,CONSOLE_BROKER_COOKED_LINES,1,pending)==0);
    lib_test_assert(b.reader && b.cooked_line_pending && readers_started==2);
    input="complete\r\n";
    console_broker_reader(&b);
    lib_test_assert(!b.cooked_line_pending);
    lib_test_assert(console_broker_backend_deactivate(&b,&pending)==0 && !pending);
    lib_test_assert(console_broker_backend_activate(&b,c,CONSOLE_BROKER_COOKED_LINES,1,pending)==0);
    lib_test_assert(!b.reader && readers_started==2);
    lib_test_assert(console_broker_backend_deactivate(&b,&pending)==0 && !pending);
    lib_console_release(c);
}
int main(void)
{
    console_broker_backend b={0};
    char line[2200];
    lib_console_text_frame f={0};
    lib_test_assert(lib_console_create(&b.console)==LIB_STATUS_OK);
    lib_test_assert(lib_console_set_event_sink(b.console,receive,LIB_NULL)==LIB_STATUS_OK);
    lib_test_assert(lib_console_bind_generation(b.console,1)==LIB_STATUS_OK);
    b.generation=1; b.mode=CONSOLE_BROKER_COOKED_LINES;
    stop=b.stop_event=lib_win32_create_event_a(LIB_NULL,LIB_WIN32_TRUE,LIB_WIN32_FALSE,LIB_NULL); lib_test_assert(stop);
    for (lib_u32 n=1022;n<=1024;++n) {
        lib_memory_set(line,'x',n); lib_text_copy(line+n,"\r\nhelp\r\n"); input=line;
        for (chunk=1;chunk<=1023;chunk+=1022) {
            input=line; delivered=0;
            console_broker_reader(&b);
            lib_test_assert(delivered==1);
            lib_test_assert(received.kind==(n>1023 ? LIB_CONSOLE_EVENT_REJECTED_LINE : LIB_CONSOLE_EVENT_COOKED_LINE));
            lib_test_assert(received.value.line.length==(n>1023 ? 0 : n));
            console_broker_reader(&b);
            lib_test_assert(delivered==2 && received.kind==LIB_CONSOLE_EVENT_COOKED_LINE);
            lib_test_assert(lib_text_compare(received.value.line.text,"help")==0);
        }
    }
    lib_memory_set(line,'x',2048);lib_text_copy(line+2048,"\r\n");input=line;
    reads=0;cancel_at=2;chunk=7;delivered=0;
    console_broker_reader(&b);lib_test_assert(delivered==0);lib_win32_reset_event(stop);cancel_at=0;
    lib_test_assert(base_sync_mutex_create(&b.output_lock)==LIB_STATUS_OK);
    lib_test_assert(base_sync_mutex_create(&b.transaction_lock)==LIB_STATUS_OK);b.output=(lib_win32_handle)1;
    f.columns=80;f.rows=25;f.text[0]=0x2588;f.palette[0]=1;
    for(lib_i32 i=0;i<2;++i) lib_test_assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==LIB_STATUS_OK);
    lib_test_assert(first_cell==0x2588 && writes==1 && palette_attempts==2);
    palette_query_ok=1;
    lib_test_assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==0);
    lib_test_assert(b.previous_palette[0]==0 && palette_sets==1);
    palette_set_ok=1;
    lib_test_assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==0);
    lib_test_assert(b.previous_palette[0]==1 && palette_sets==2);
    lib_test_assert(buffer_size.Y==25); /* Palette must precede surface preparation. */
    lib_test_assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==0);
    lib_test_assert(palette_sets==2);
    /* Native approximation consumes the already normalized scanline range. */
    f.font_height=16; f.cursor_top=14; f.cursor_bottom=15;
    f.cursor_visible=f.cursor_phase=LIB_TRUE;
    lib_test_assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==0);
    lib_test_assert(last_cursor.dwSize==12 && last_cursor.bVisible);
    f.cursor_visible=LIB_FALSE;
    lib_test_assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==0);
    lib_test_assert(!last_cursor.bVisible);
    f.cursor_visible=LIB_TRUE; f.cursor_top=9; f.cursor_bottom=8;
    lib_test_assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==0);
    lib_test_assert(last_cursor.dwSize==100 && last_cursor.bVisible);
    for (lib_i32 axis=0;axis<2;++axis) {
        lib_u32 before=writes;
        if (axis==0) buffer_size.Y=24;
        else buffer_size.X=40;
        lib_test_assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==0);
        lib_test_assert(buffer_size.X>=80 && buffer_size.Y>=25 && writes==before+1);
        lib_test_assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==0);
        lib_test_assert(writes==before+1);
    }
    cursor_ok=0;
    lib_test_assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==LIB_STATUS_IO_ERROR);
    f.cursor_visible=f.cursor_phase=LIB_TRUE;
    lib_test_assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==LIB_STATUS_IO_ERROR);
    cursor_ok=1;
    lib_test_assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==LIB_STATUS_OK);
    lib_u32 previous_writes=writes;
    lib_test_assert(console_broker_backend_write_bound(&b,b.console,2,"x",1)==LIB_STATUS_OK);
    lib_test_assert(console_broker_backend_write_bound(&b,b.console,1,"",0)==LIB_STATUS_OK);
    lib_test_assert(b.previous_columns==80 && b.previous_rows==25);
    lib_test_assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==0 && writes==previous_writes);
    for (lib_i32 scenario=0;scenario<3;++scenario) {
        text_result=scenario!=2; text_written=scenario==1 ? 0 : 1;
        lib_test_assert(console_broker_backend_write_bound(&b,b.console,1,"x",1)==
            (scenario==0 ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR));
        lib_test_assert(!b.previous_columns && !b.previous_rows);
        lib_test_assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==0);
        lib_test_assert(writes==++previous_writes && b.previous_columns==80);
    }
    /* Failed B can partly overwrite A: retrying A must not hit the old cache. */
    for (lib_i32 failure=1;failure<=5;++failure) {
        partial_write=failure; f.text[0]='B';
        lib_test_assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==LIB_STATUS_IO_ERROR);
        lib_test_assert(!b.previous_columns && !b.previous_rows);
        lib_u32 attempted=writes;
        partial_write=0; f.text[0]=0x2588;
        lib_test_assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==LIB_STATUS_OK);
        lib_test_assert(writes==attempted+1 && b.previous_columns==80);
    }
    for (lib_u32 attribute=0;attribute<256;++attribute) {
        f.foreground[0]=attribute & 15u; f.background[0]=attribute >> 4;
        lib_test_assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==LIB_STATUS_OK);
        lib_test_assert(first_attribute==attribute);
        lib_u32 completed=writes;
        lib_test_assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==LIB_STATUS_OK);
        lib_test_assert(writes==completed);
    }
    /* Taller modes reach the last cell; shrinking clears old lower rows
     * while preserving the native viewport and normal 25-row startup. */
    {
        const lib_u16 rows[] = {22u,25u,43u,50u,25u,50u};
        for (lib_size i=0;i<sizeof(rows)/sizeof(rows[0]);++i) {
            f.rows=rows[i];
            lib_memory_set(f.text,0,sizeof(f.text));
            f.text[(lib_size)f.rows*80u-1u]='Z';
            lib_test_assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==LIB_STATUS_OK);
            lib_test_assert(captured_cells[(lib_size)f.rows*80u-1u].Char.UnicodeChar=='Z');
            lib_test_assert(captured_region.Bottom>=f.rows-1);
            for (lib_u32 row=f.rows;row<=(lib_u32)captured_region.Bottom;++row)
                for (lib_u32 col=0;col<80u;++col)
                    lib_test_assert(captured_cells[row*80u+col].Char.UnicodeChar==' ');
        }
    }
    viewport.Bottom=24; reject_viewport=1;
    lib_u32 completed=writes;
    lib_test_assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==LIB_STATUS_IO_ERROR);
    lib_test_assert(writes==completed);
    reject_viewport=0;
    lib_test_assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==LIB_STATUS_OK);
    lib_test_assert(viewport.Bottom==49);
    base_sync_mutex_destroy(b.transaction_lock);base_sync_mutex_destroy(b.output_lock);lib_win32_close_handle(stop);lib_console_release(b.console);
    cooked_restore();
    /* Disposal must not restore native mode a second time. */
    console_broker_backend *disposed=lib_allocate_zero(1,sizeof(*disposed));
    disposed->input=disposed->output=LIB_WIN32_INVALID_HANDLE_VALUE;
    lib_test_assert(base_sync_mutex_create(&disposed->output_lock)==LIB_STATUS_OK);
    lib_test_assert(base_sync_mutex_create(&disposed->transaction_lock)==LIB_STATUS_OK);
    lib_test_assert(console_broker_backend_deactivate(disposed,LIB_NULL)==LIB_STATUS_OK);
    lib_u32 restored=mode_sets;
    console_broker_backend_destroy(disposed);
    lib_test_assert(mode_sets==restored);
    return 0;
}
