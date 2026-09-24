#include "lib/types/win32/console.h"
#include "lib/types/win32/sync.h"
#include "lib/console/binding_interface.h"
#include <assert.h>
#include <string.h>

static const char *input;
static lib_u32 chunk = 7, reads, cancel_at;
static HANDLE stop;
static BOOL WINAPI read_chunk(HANDLE h, LPVOID bytes, DWORD capacity, LPDWORD count, LPVOID p)
{
    lib_u32 n = 0;
    (void)h; (void)p;
    while (input[n] && n < capacity && n < chunk) {
        ((char *)bytes)[n] = input[n];
        if (input[n++] == '\n') break;
    }
    input += n; *count = n;
    if (++reads == cancel_at) SetEvent(stop);
    return TRUE;
}
static lib_u32 palette_attempts, palette_sets, writes;
static lib_i32 partial_write;
static lib_i32 text_result=1;
static DWORD text_written=1;
static BOOL WINAPI text_write(HANDLE h,LPCVOID text,DWORD n,LPDWORD written,LPVOID r)
{ (void)h;(void)text;(void)r;*written=n==0 ? 0 : text_written;return text_result; }
static lib_i32 palette_query_ok, palette_set_ok, cursor_ok = 1;
static WCHAR first_cell;
static WORD first_attribute;
static COORD buffer_size={80,25};
static SMALL_RECT viewport={0,0,79,24};
static BOOL WINAPI screen_info(HANDLE h, PCONSOLE_SCREEN_BUFFER_INFO p)
{ (void)h; memset(p, 0, sizeof(*p)); p->dwSize=buffer_size; p->srWindow=viewport; return TRUE; }
static BOOL WINAPI set_viewport(HANDLE h,BOOL absolute,const SMALL_RECT *rect)
{ (void)h; assert(absolute); viewport=*rect; return TRUE; }
static BOOL WINAPI resize_buffer(HANDLE h,COORD size)
{ (void)h; buffer_size=size; return TRUE; }
static BOOL WINAPI palette_get(HANDLE h, PCONSOLE_SCREEN_BUFFER_INFOEX p)
{ (void)h; (void)p; ++palette_attempts; return palette_query_ok; }
static BOOL WINAPI palette_set(HANDLE h, PCONSOLE_SCREEN_BUFFER_INFOEX p)
{ (void)h; (void)p; ++palette_sets; if(palette_set_ok) buffer_size.Y=24; return palette_set_ok; }
static BOOL WINAPI write_cells(HANDLE h, const CHAR_INFO *p, COORD a, COORD b, PSMALL_RECT r)
{
    (void)h; (void)a; (void)b; ++writes; first_cell=p[0].Char.UnicodeChar;
    first_attribute=p[0].Attributes;
    if (r->Bottom>=buffer_size.Y) r->Bottom=buffer_size.Y-1;
    if (partial_write==1) r->Right=39;
    if (partial_write==2) r->Bottom=11;
    if (partial_write==3) r->Left=1;
    if (partial_write==4) r->Top=1;
    return partial_write!=5;
}
static CONSOLE_CURSOR_INFO last_cursor;
static BOOL WINAPI cursor_info(HANDLE h, const CONSOLE_CURSOR_INFO *p)
{ (void)h; last_cursor=*p; return cursor_ok; }
static BOOL WINAPI cursor_position(HANDLE h, COORD p)
{ (void)h; (void)p; return cursor_ok; }
static lib_u32 readers_started, mode_sets;
static BOOL WINAPI set_mode(HANDLE h, DWORD mode) { (void)h; (void)mode; ++mode_sets; return TRUE; }
static BOOL WINAPI flush_input(HANDLE h) { (void)h; return TRUE; }
static HANDLE WINAPI start_reader(LPSECURITY_ATTRIBUTES a, SIZE_T size,
    LPTHREAD_START_ROUTINE entry, LPVOID arg, DWORD flags, LPDWORD id)
{
    (void)a; (void)size; (void)entry; (void)arg; (void)flags; (void)id;
    ++readers_started;
    return CreateEventA(NULL, TRUE, TRUE, NULL); /* deterministic joined worker */
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
    assert(lib_console_create(&c)==LIB_STATUS_OK);
    assert(lib_console_bind_generation(c,1)==LIB_STATUS_OK);
    assert(lib_console_set_event_sink(c,receive,NULL)==LIB_STATUS_OK);
    assert(console_broker_backend_activate(&b,c,CONSOLE_BROKER_COOKED_LINES,1,0)==0);
    assert(!b.reader && !b.cooked_line_pending && readers_started==0);
    assert(console_broker_backend_request_cooked_line(&b)==0);
    assert(b.reader && b.cooked_line_pending && readers_started==1);
    /* Cancellation preserves an unfinished request until join. */
    stop=b.stop_event; input="discarded\r\n"; reads=0; cancel_at=1;
    console_broker_reader(&b); cancel_at=0;
    assert(b.cooked_line_pending);
    assert(console_broker_backend_deactivate(&b,&pending)==0 && pending);
    assert(!b.reader && !b.cooked_line_pending);
    assert(console_broker_backend_activate(&b,c,CONSOLE_BROKER_COOKED_LINES,1,pending)==0);
    assert(b.reader && b.cooked_line_pending && readers_started==2);
    input="complete\r\n";
    console_broker_reader(&b);
    assert(!b.cooked_line_pending);
    assert(console_broker_backend_deactivate(&b,&pending)==0 && !pending);
    assert(console_broker_backend_activate(&b,c,CONSOLE_BROKER_COOKED_LINES,1,pending)==0);
    assert(!b.reader && readers_started==2);
    assert(console_broker_backend_deactivate(&b,&pending)==0 && !pending);
    lib_console_release(c);
}
int main(void)
{
    console_broker_backend b={0};
    char line[2200];
    lib_console_text_frame f={0};
    assert(lib_console_create(&b.console)==LIB_STATUS_OK);
    assert(lib_console_set_event_sink(b.console,receive,NULL)==LIB_STATUS_OK);
    assert(lib_console_bind_generation(b.console,1)==LIB_STATUS_OK);
    b.generation=1; b.mode=CONSOLE_BROKER_COOKED_LINES;
    stop=b.stop_event=CreateEventA(NULL,TRUE,FALSE,NULL); assert(stop);
    for (lib_u32 n=1022;n<=1024;++n) {
        memset(line,'x',n); strcpy(line+n,"\r\nhelp\r\n"); input=line;
        for (chunk=1;chunk<=1023;chunk+=1022) {
            input=line; delivered=0;
            console_broker_reader(&b);
            assert(delivered==1);
            assert(received.kind==(n>1023 ? LIB_CONSOLE_EVENT_REJECTED_LINE : LIB_CONSOLE_EVENT_COOKED_LINE));
            assert(received.value.line.length==(n>1023 ? 0 : n));
            console_broker_reader(&b);
            assert(delivered==2 && received.kind==LIB_CONSOLE_EVENT_COOKED_LINE);
            assert(strcmp(received.value.line.text,"help")==0);
        }
    }
    memset(line,'x',2048);strcpy(line+2048,"\r\n");input=line;
    reads=0;cancel_at=2;chunk=7;delivered=0;
    console_broker_reader(&b);assert(delivered==0);ResetEvent(stop);cancel_at=0;
    assert(base_sync_mutex_create(&b.output_lock)==LIB_STATUS_OK);
    assert(base_sync_mutex_create(&b.transaction_lock)==LIB_STATUS_OK);b.output=(HANDLE)1;
    f.columns=80;f.rows=25;f.text[0]=0x2588;f.palette[0]=1;
    for(lib_i32 i=0;i<2;++i) assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==LIB_STATUS_OK);
    assert(first_cell==0x2588 && writes==1 && palette_attempts==2);
    palette_query_ok=1;
    assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==0);
    assert(b.previous_palette[0]==0 && palette_sets==1);
    palette_set_ok=1;
    assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==0);
    assert(b.previous_palette[0]==1 && palette_sets==2);
    assert(buffer_size.Y==25); /* Palette must precede surface preparation. */
    assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==0);
    assert(palette_sets==2);
    /* Native approximation consumes the already normalized scanline range. */
    f.font_height=16; f.cursor_top=14; f.cursor_bottom=15;
    f.cursor_visible=f.cursor_phase=1;
    assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==0);
    assert(last_cursor.dwSize==12 && last_cursor.bVisible);
    f.cursor_visible=0;
    assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==0);
    assert(!last_cursor.bVisible);
    f.cursor_visible=1; f.cursor_top=9; f.cursor_bottom=8;
    assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==0);
    assert(last_cursor.dwSize==100 && last_cursor.bVisible);
    for (lib_i32 axis=0;axis<2;++axis) {
        lib_u32 before=writes;
        if (axis==0) buffer_size.Y=24;
        else buffer_size.X=40;
        assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==0);
        assert(buffer_size.X>=80 && buffer_size.Y>=25 && writes==before+1);
        assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==0);
        assert(writes==before+1);
    }
    cursor_ok=0;
    assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==LIB_STATUS_IO_ERROR);
    f.cursor_visible=f.cursor_phase=1;
    assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==LIB_STATUS_IO_ERROR);
    cursor_ok=1;
    assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==LIB_STATUS_OK);
    lib_u32 previous_writes=writes;
    assert(console_broker_backend_write_bound(&b,b.console,2,"x",1)==LIB_STATUS_OK);
    assert(console_broker_backend_write_bound(&b,b.console,1,"",0)==LIB_STATUS_OK);
    assert(b.previous_columns==80 && b.previous_rows==25);
    assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==0 && writes==previous_writes);
    for (lib_i32 scenario=0;scenario<3;++scenario) {
        text_result=scenario!=2; text_written=scenario==1 ? 0 : 1;
        assert(console_broker_backend_write_bound(&b,b.console,1,"x",1)==
            (scenario==0 ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR));
        assert(!b.previous_columns && !b.previous_rows);
        assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==0);
        assert(writes==++previous_writes && b.previous_columns==80);
    }
    /* Failed B can partly overwrite A: retrying A must not hit the old cache. */
    for (lib_i32 failure=1;failure<=5;++failure) {
        partial_write=failure; f.text[0]='B';
        assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==LIB_STATUS_IO_ERROR);
        assert(!b.previous_columns && !b.previous_rows);
        lib_u32 attempted=writes;
        partial_write=0; f.text[0]=0x2588;
        assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==LIB_STATUS_OK);
        assert(writes==attempted+1 && b.previous_columns==80);
    }
    for (lib_u32 attribute=0;attribute<256;++attribute) {
        f.foreground[0]=attribute & 15u; f.background[0]=attribute >> 4;
        assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==LIB_STATUS_OK);
        assert(first_attribute==attribute);
        lib_u32 completed=writes;
        assert(console_broker_backend_write_text_frame_bound(&b,b.console,1,&f)==LIB_STATUS_OK);
        assert(writes==completed);
    }
    base_sync_mutex_destroy(b.transaction_lock);base_sync_mutex_destroy(b.output_lock);CloseHandle(stop);lib_console_release(b.console);
    cooked_restore();
    /* Disposal must not restore native mode a second time. */
    console_broker_backend *disposed=calloc(1,sizeof(*disposed));
    disposed->input=disposed->output=INVALID_HANDLE_VALUE;
    assert(base_sync_mutex_create(&disposed->output_lock)==LIB_STATUS_OK);
    assert(base_sync_mutex_create(&disposed->transaction_lock)==LIB_STATUS_OK);
    assert(console_broker_backend_deactivate(disposed,NULL)==LIB_STATUS_OK);
    lib_u32 restored=mode_sets;
    console_broker_backend_destroy(disposed);
    assert(mode_sets==restored);
    return 0;
}
