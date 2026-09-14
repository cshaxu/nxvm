#include "lib/types/win32/console.h"
#include "lib/types/win32/sync.h"
#include "lib/console/binding_interface.h"
#include <assert.h>
#include <string.h>

static const char *input;
static unsigned chunk = 7, reads, cancel_at;
static HANDLE stop;
static BOOL WINAPI read_chunk(HANDLE h, LPVOID bytes, DWORD capacity, LPDWORD count, LPVOID p)
{
    unsigned n = 0;
    (void)h; (void)p;
    while (input[n] && n < capacity && n < chunk) {
        ((char *)bytes)[n] = input[n];
        if (input[n++] == '\n') break;
    }
    input += n; *count = n;
    if (++reads == cancel_at) SetEvent(stop);
    return TRUE;
}
static unsigned palette_attempts, palette_sets, writes;
static int partial_write;
static int text_result=1;
static DWORD text_written=1;
static BOOL WINAPI text_write(HANDLE h,LPCVOID text,DWORD n,LPDWORD written,LPVOID r)
{ (void)h;(void)text;(void)r;*written=n==0 ? 0 : text_written;return text_result; }
static int palette_query_ok, palette_set_ok, cursor_ok = 1;
static WCHAR first_cell;
static COORD buffer_size={80,25};
static BOOL WINAPI screen_info(HANDLE h, PCONSOLE_SCREEN_BUFFER_INFO p)
{ (void)h; memset(p, 0, sizeof(*p)); p->dwSize=buffer_size; return TRUE; }
static BOOL WINAPI resize_buffer(HANDLE h,COORD size)
{ (void)h; buffer_size=size; return TRUE; }
static BOOL WINAPI palette_get(HANDLE h, PCONSOLE_SCREEN_BUFFER_INFOEX p)
{ (void)h; (void)p; ++palette_attempts; return palette_query_ok; }
static BOOL WINAPI palette_set(HANDLE h, PCONSOLE_SCREEN_BUFFER_INFOEX p)
{ (void)h; (void)p; ++palette_sets; if(palette_set_ok) buffer_size.Y=24; return palette_set_ok; }
static BOOL WINAPI write_cells(HANDLE h, const CHAR_INFO *p, COORD a, COORD b, PSMALL_RECT r)
{
    (void)h; (void)a; (void)b; ++writes; first_cell=p[0].Char.UnicodeChar;
    if (r->Bottom>=buffer_size.Y) r->Bottom=buffer_size.Y-1;
    if (partial_write==1) r->Right=39;
    if (partial_write==2) r->Bottom=11;
    if (partial_write==3) r->Left=1;
    if (partial_write==4) r->Top=1;
    return partial_write!=5;
}
static BOOL WINAPI cursor_info(HANDLE h, const CONSOLE_CURSOR_INFO *p)
{ (void)h; (void)p; return cursor_ok; }
static BOOL WINAPI cursor_position(HANDLE h, COORD p)
{ (void)h; (void)p; return cursor_ok; }
static unsigned readers_started, mode_sets;
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
#include "lib/host/win32/console.c"

static unsigned delivered;
static lib_console_event received;
static void receive(void *p, const lib_console_event *event)
{ (void)p; received=*event; ++delivered; }
static void cooked_restore(void)
{
    host_console_backend b={0};
    lib_console *c;
    lib_bool pending;
    assert(lib_console_create(&c)==LIB_STATUS_OK);
    assert(lib_console_bind_generation(c,1)==LIB_STATUS_OK);
    assert(lib_console_set_event_sink(c,receive,NULL)==LIB_STATUS_OK);
    assert(host_console_backend_activate(&b,c,HOST_CONSOLE_COOKED_LINES,1,0)==0);
    assert(!b.reader && !b.cooked_line_pending && readers_started==0);
    assert(host_console_backend_request_cooked_line(&b)==0);
    assert(b.reader && b.cooked_line_pending && readers_started==1);
    /* Cancellation preserves an unfinished request until join. */
    stop=b.stop_event; input="discarded\r\n"; reads=0; cancel_at=1;
    host_console_reader(&b); cancel_at=0;
    assert(b.cooked_line_pending);
    assert(host_console_backend_deactivate(&b,&pending)==0 && pending);
    assert(!b.reader && !b.cooked_line_pending);
    assert(host_console_backend_activate(&b,c,HOST_CONSOLE_COOKED_LINES,1,pending)==0);
    assert(b.reader && b.cooked_line_pending && readers_started==2);
    input="complete\r\n";
    host_console_reader(&b);
    assert(!b.cooked_line_pending);
    assert(host_console_backend_deactivate(&b,&pending)==0 && !pending);
    assert(host_console_backend_activate(&b,c,HOST_CONSOLE_COOKED_LINES,1,pending)==0);
    assert(!b.reader && readers_started==2);
    assert(host_console_backend_deactivate(&b,&pending)==0 && !pending);
    lib_console_release(c);
}
int main(void)
{
    host_console_backend b={0};
    char line[2200];
    lib_console_text_frame f={0};
    assert(lib_console_create(&b.console)==LIB_STATUS_OK);
    assert(lib_console_set_event_sink(b.console,receive,NULL)==LIB_STATUS_OK);
    assert(lib_console_bind_generation(b.console,1)==LIB_STATUS_OK);
    b.generation=1; b.mode=HOST_CONSOLE_COOKED_LINES;
    stop=b.stop_event=CreateEventA(NULL,TRUE,FALSE,NULL); assert(stop);
    for (unsigned n=1022;n<=1024;++n) {
        memset(line,'x',n); strcpy(line+n,"\r\nhelp\r\n"); input=line;
        for (chunk=1;chunk<=1023;chunk+=1022) {
            input=line; delivered=0;
            host_console_reader(&b);
            assert(delivered==1);
            assert(received.kind==(n>1023 ? LIB_CONSOLE_EVENT_REJECTED_LINE : LIB_CONSOLE_EVENT_COOKED_LINE));
            assert(received.value.line.length==(n>1023 ? 0 : n));
            host_console_reader(&b);
            assert(delivered==2 && received.kind==LIB_CONSOLE_EVENT_COOKED_LINE);
            assert(strcmp(received.value.line.text,"help")==0);
        }
    }
    memset(line,'x',2048);strcpy(line+2048,"\r\n");input=line;
    reads=0;cancel_at=2;chunk=7;delivered=0;
    host_console_reader(&b);assert(delivered==0);ResetEvent(stop);cancel_at=0;
    assert(lib_console_pc_glyph(0)==' ' && lib_console_pc_glyph('A')=='A');
    assert(lib_console_pc_glyph(1)==0x263a && lib_console_pc_glyph(0x7f)==0x2302);
    assert(lib_console_pc_glyph(0xb3)==0x2502 && lib_console_pc_glyph(0xc4)==0x2500);
    assert(lib_console_pc_glyph(0xda)==0x250c && lib_console_pc_glyph(0xdb)==0x2588);
    assert(lib_console_pc_glyph(0x82)==0xe9 && lib_console_pc_glyph(0xff)==0xa0);
    InitializeCriticalSection(&b.output_lock);InitializeCriticalSection(&b.transaction_lock);b.output=(HANDLE)1;
    f.columns=80;f.rows=25;f.text[0]=0xdb;f.palette[0]=1;
    for(int i=0;i<2;++i) assert(host_console_backend_write_text_frame_bound(&b,b.console,1,&f)==LIB_STATUS_OK);
    assert(first_cell==0x2588 && writes==1 && palette_attempts==2);
    palette_query_ok=1;
    assert(host_console_backend_write_text_frame_bound(&b,b.console,1,&f)==0);
    assert(b.previous_palette[0]==0 && palette_sets==1);
    palette_set_ok=1;
    assert(host_console_backend_write_text_frame_bound(&b,b.console,1,&f)==0);
    assert(b.previous_palette[0]==1 && palette_sets==2);
    assert(buffer_size.Y==25); /* Palette must precede surface preparation. */
    assert(host_console_backend_write_text_frame_bound(&b,b.console,1,&f)==0);
    assert(palette_sets==2);
    for (int axis=0;axis<2;++axis) {
        unsigned before=writes;
        if (axis==0) buffer_size.Y=24;
        else buffer_size.X=40;
        assert(host_console_backend_write_text_frame_bound(&b,b.console,1,&f)==0);
        assert(buffer_size.X>=80 && buffer_size.Y>=25 && writes==before+1);
        assert(host_console_backend_write_text_frame_bound(&b,b.console,1,&f)==0);
        assert(writes==before+1);
    }
    cursor_ok=0;
    assert(host_console_backend_write_text_frame_bound(&b,b.console,1,&f)==LIB_STATUS_IO_ERROR);
    f.cursor_visible=f.cursor_phase=1;
    assert(host_console_backend_write_text_frame_bound(&b,b.console,1,&f)==LIB_STATUS_IO_ERROR);
    cursor_ok=1;
    assert(host_console_backend_write_text_frame_bound(&b,b.console,1,&f)==LIB_STATUS_OK);
    unsigned previous_writes=writes;
    assert(host_console_backend_write_bound(&b,b.console,2,"x",1)==LIB_STATUS_NOT_CURRENT);
    assert(host_console_backend_write_bound(&b,b.console,1,"",0)==LIB_STATUS_OK);
    assert(b.previous_columns==80 && b.previous_rows==25);
    assert(host_console_backend_write_text_frame_bound(&b,b.console,1,&f)==0 && writes==previous_writes);
    for (int scenario=0;scenario<3;++scenario) {
        text_result=scenario!=2; text_written=scenario==1 ? 0 : 1;
        assert(host_console_backend_write_bound(&b,b.console,1,"x",1)==
            (scenario==0 ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR));
        assert(!b.previous_columns && !b.previous_rows);
        assert(host_console_backend_write_text_frame_bound(&b,b.console,1,&f)==0);
        assert(writes==++previous_writes && b.previous_columns==80);
    }
    /* Failed B can partly overwrite A: retrying A must not hit the old cache. */
    for (int failure=1;failure<=5;++failure) {
        partial_write=failure; f.text[0]='B';
        assert(host_console_backend_write_text_frame_bound(&b,b.console,1,&f)==LIB_STATUS_IO_ERROR);
        assert(!b.previous_columns && !b.previous_rows);
        unsigned attempted=writes;
        partial_write=0; f.text[0]=0xdb;
        assert(host_console_backend_write_text_frame_bound(&b,b.console,1,&f)==LIB_STATUS_OK);
        assert(writes==attempted+1 && b.previous_columns==80);
    }
    DeleteCriticalSection(&b.transaction_lock);DeleteCriticalSection(&b.output_lock);CloseHandle(stop);lib_console_release(b.console);
    cooked_restore();
    /* Disposal must not restore native mode a second time. */
    host_console_backend *disposed=calloc(1,sizeof(*disposed));
    disposed->input=disposed->output=INVALID_HANDLE_VALUE;
    InitializeCriticalSection(&disposed->output_lock);
    InitializeCriticalSection(&disposed->transaction_lock);
    assert(host_console_backend_deactivate(disposed,NULL)==LIB_STATUS_OK);
    unsigned restored=mode_sets;
    host_console_backend_destroy(disposed);
    assert(mode_sets==restored);
    return 0;
}
