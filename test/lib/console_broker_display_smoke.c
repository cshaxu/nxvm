#include "lib/types/win32/console.h"
#include "lib/types/win32/sync.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "lib/base/sync_interface.h"

static unsigned mutex_creates, fail_mutex, live_mutexes;
static lib_status create_mutex(base_sync_mutex **out)
{
    if (++mutex_creates == fail_mutex) { *out = NULL; return LIB_STATUS_NO_MEMORY; }
    lib_status status = base_sync_mutex_create(out);
    if (status == LIB_STATUS_OK) ++live_mutexes;
    return status;
}
static void destroy_mutex(base_sync_mutex *mutex)
{
    if (mutex != NULL) { assert(live_mutexes); --live_mutexes; }
    base_sync_mutex_destroy(mutex);
}

/* Native display I/O, deterministic reader/startup failures. The test owns a
 * hidden Console; it never changes the developer's Console or its input. */
static int fail_allocate, fail_select, fail_reader, fail_query, fail_restore;
static int fail_viewport, ignore_viewport;
static BOOL WINAPI set_viewport(HANDLE output, BOOL absolute, const SMALL_RECT *rect)
{
    if (fail_viewport) { fail_viewport = 0; return FALSE; }
    if (ignore_viewport) { ignore_viewport = 0; return TRUE; }
    return SetConsoleWindowInfo(output, absolute, rect);
}
static BOOL WINAPI query_display(HANDLE output, PCONSOLE_SCREEN_BUFFER_INFOEX info)
{
    if (fail_query) { fail_query = 0; return FALSE; }
    return GetConsoleScreenBufferInfoEx(output, info);
}
static BOOL WINAPI restore_display(HANDLE output, PCONSOLE_SCREEN_BUFFER_INFOEX info)
{
    if (fail_restore) { fail_restore = 0; return FALSE; }
    return SetConsoleScreenBufferInfoEx(output, info);
}
static HANDLE WINAPI allocate_screen(DWORD access, DWORD share,
    const SECURITY_ATTRIBUTES *security, DWORD flags, LPVOID reserved)
{
    if (fail_allocate) { fail_allocate = 0; return INVALID_HANDLE_VALUE; }
    return CreateConsoleScreenBuffer(access, share, security, flags, reserved);
}
static BOOL WINAPI select_screen(HANDLE output)
{
    if (fail_select) { fail_select = 0; return FALSE; }
    return SetConsoleActiveScreenBuffer(output);
}
static HANDLE WINAPI start_reader(LPSECURITY_ATTRIBUTES attributes, SIZE_T size,
    LPTHREAD_START_ROUTINE entry, LPVOID arg, DWORD flags, LPDWORD id)
{
    (void)attributes; (void)size; (void)entry; (void)arg; (void)flags; (void)id;
    if (fail_reader) { fail_reader = 0; return NULL; }
    return CreateEventA(NULL, TRUE, TRUE, NULL);
}
static HWND WINAPI no_foreground(void) { return NULL; }
#undef lib_win32_create_console_screen_buffer
#define lib_win32_create_console_screen_buffer allocate_screen
#undef lib_win32_set_console_active_screen_buffer
#define lib_win32_set_console_active_screen_buffer select_screen
#undef lib_win32_create_thread
#define lib_win32_create_thread start_reader
#undef lib_win32_get_console_window
#define lib_win32_get_console_window no_foreground
#undef lib_win32_get_console_screen_buffer_info_ex
#define lib_win32_get_console_screen_buffer_info_ex query_display
#undef lib_win32_set_console_screen_buffer_info_ex
#define lib_win32_set_console_screen_buffer_info_ex restore_display
#undef lib_win32_set_console_window_info
#define lib_win32_set_console_window_info set_viewport
#define base_sync_mutex_create create_mutex
#define base_sync_mutex_destroy destroy_mutex
#include "lib/console-broker/win32/console.c"
#undef base_sync_mutex_create
#undef base_sync_mutex_destroy
#include "lib/console-broker/console.c"

typedef struct display_snapshot {
    CONSOLE_SCREEN_BUFFER_INFOEX info;
    CONSOLE_CURSOR_INFO cursor;
    DWORD mode;
    CHAR_INFO cells[120 * 30];
} display_snapshot;

static void snapshot(display_snapshot *s)
{
    HANDLE output = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    COORD size = {120, 30}, origin = {0, 0};
    SMALL_RECT region = {0, 0, 119, 29};
    assert(output != INVALID_HANDLE_VALUE);
    memset(s, 0, sizeof(*s));
    s->info.cbSize = sizeof(s->info);
    assert(GetConsoleScreenBufferInfoEx(output, &s->info));
    assert(GetConsoleCursorInfo(output, &s->cursor));
    assert(GetConsoleMode(output, &s->mode));
    assert(ReadConsoleOutputW(output, s->cells, size, origin, &region));
    assert(region.Right >= 79 && region.Bottom >= 24);
    assert(CloseHandle(output));
}

static void expect_display(const display_snapshot *expected)
{
    display_snapshot actual;
    static unsigned checkpoint;
    snapshot(&actual);
    ++checkpoint;
    if (memcmp(&actual, expected, sizeof(actual)) != 0) {
        fprintf(stderr, "display checkpoint %u: size %d,%d/%d,%d cursor %d,%d/%d,%d viewport %d,%d,%d,%d/%d,%d,%d,%d cells=%d palette=%d cursor-style=%d mode=%lu/%lu\n",
            checkpoint, actual.info.dwSize.X, actual.info.dwSize.Y,
            expected->info.dwSize.X, expected->info.dwSize.Y,
            actual.info.dwCursorPosition.X, actual.info.dwCursorPosition.Y,
            expected->info.dwCursorPosition.X, expected->info.dwCursorPosition.Y,
            actual.info.srWindow.Left, actual.info.srWindow.Top, actual.info.srWindow.Right, actual.info.srWindow.Bottom,
            expected->info.srWindow.Left, expected->info.srWindow.Top, expected->info.srWindow.Right, expected->info.srWindow.Bottom,
            memcmp(actual.cells, expected->cells, sizeof(actual.cells)),
            memcmp(actual.info.ColorTable, expected->info.ColorTable, sizeof(actual.info.ColorTable)),
            memcmp(&actual.cursor, &expected->cursor, sizeof(actual.cursor)), actual.mode, expected->mode);
    }
    assert(memcmp(&actual, expected, sizeof(actual)) == 0);
}

static void check_frame_extent(short columns, short rows, int scrolled)
{
    console_broker *broker = NULL;
    lib_console *cooked, *raw;
    lib_console_text_frame frame = {0};
    CONSOLE_SCREEN_BUFFER_INFO before, actual;
    COORD extent = {columns, rows}, origin = {0, 0}, cells_size = {80, 25};
    SMALL_RECT viewport = {0, 0, 19, 9}, region;
    CHAR_INFO cells[80 * 25];
    assert(lib_console_create(&cooked) == 0);
    assert(lib_console_create(&raw) == 0);
    assert(console_broker_create(&broker, cooked, CONSOLE_BROKER_COOKED_LINES) == 0);
    assert(SetConsoleCursorPosition(broker->backend->output, origin));
    assert(SetConsoleWindowInfo(broker->backend->output, TRUE, &viewport));
    assert(SetConsoleScreenBufferSize(broker->backend->output, extent));
    viewport.Right = scrolled == 2 ? 19 : columns - 1;
    viewport.Bottom = scrolled == 2 ? 9 : rows < 30 ? rows - 1 : 29;
    if (scrolled == 1) {
        viewport.Top = 2; viewport.Bottom += 2;
    }
    assert(SetConsoleWindowInfo(broker->backend->output, TRUE, &viewport));
    assert(GetConsoleScreenBufferInfo(broker->backend->output, &before));
    frame.columns = 80; frame.rows = 25; frame.font_height = 16;
    for (unsigned i=0; i<80u*25u; ++i) frame.text[i]='#';
    for (int round = 0; round < 3; ++round) {
        assert(console_broker_replace(broker, cooked, raw, CONSOLE_BROKER_RAW_EVENTS) == 0);
        assert(SetConsoleWindowInfo(broker->backend->output, TRUE, &viewport));
        if (round == 0 && !scrolled && rows == 13) {
            fail_viewport = 1;
            assert(!console_broker_ensure_text_surface(broker->backend));
            assert(fail_viewport == 0);
            ignore_viewport = 1;
            assert(!console_broker_ensure_text_surface(broker->backend));
            assert(ignore_viewport == 0);
        }
        {
            CONSOLE_SCREEN_BUFFER_INFO raw_before;
            int width = viewport.Right - viewport.Left + 1;
            int height = viewport.Bottom - viewport.Top + 1;
            assert(GetConsoleScreenBufferInfo(broker->backend->output, &raw_before));
            assert(console_broker_ensure_text_surface(broker->backend));
            assert(GetConsoleScreenBufferInfo(broker->backend->output, &actual));
            assert(actual.srWindow.Left == 0 && actual.srWindow.Top == 0);
            assert(actual.srWindow.Right + 1 == (width < 80 ? 80 : width));
            assert(actual.srWindow.Bottom + 1 == (height < 25 ? 25 : height));
            assert(actual.dwSize.X >= raw_before.dwSize.X);
            assert(actual.dwSize.Y >= raw_before.dwSize.Y);
        }
        assert(lib_console_write_text_frame(raw, &frame) == 0);
        assert(lib_console_write_text_frame(raw, &frame) == 0);
        region = (SMALL_RECT){0, 0, 79, 24};
        assert(ReadConsoleOutputW(broker->backend->output, cells, cells_size, origin, &region));
        assert(region.Left == 0 && region.Top == 0 && region.Right == 79 && region.Bottom == 24);
        for (unsigned i = 0; i < 80 * 25; ++i) assert(cells[i].Char.UnicodeChar == '#');
        assert(console_broker_replace(broker, raw, cooked, CONSOLE_BROKER_COOKED_LINES) == 0);
        assert(GetConsoleScreenBufferInfo(broker->backend->output, &actual));
        if (actual.dwSize.X != before.dwSize.X || actual.dwSize.Y != before.dwSize.Y)
            fprintf(stderr, "extent %d,%d round %d: restored %d,%d expected %d,%d\n",
                columns, rows, round, actual.dwSize.X, actual.dwSize.Y, before.dwSize.X, before.dwSize.Y);
        assert(actual.dwSize.X == before.dwSize.X && actual.dwSize.Y == before.dwSize.Y);
        if (memcmp(&actual.srWindow, &before.srWindow, sizeof(actual.srWindow)) != 0)
            fprintf(stderr, "viewport %d,%d round %d: %d,%d,%d,%d expected %d,%d,%d,%d\n",
                columns, rows, round, actual.srWindow.Left, actual.srWindow.Top,
                actual.srWindow.Right, actual.srWindow.Bottom, before.srWindow.Left,
                before.srWindow.Top, before.srWindow.Right, before.srWindow.Bottom);
        assert(memcmp(&actual.srWindow, &before.srWindow, sizeof(actual.srWindow)) == 0);
    }
    assert(console_broker_destroy(broker) == 0);
    lib_console_release(raw); lib_console_release(cooked);
}

int main(void)
{
    console_broker *broker;
    lib_console *cooked, *raw, *other;
    lib_console_text_frame frame = {0};
    display_snapshot before, after, raw_display;
    DWORD written;
    COORD origin = {0, 0};
    CONSOLE_CURSOR_INFO cursor = {25, TRUE};
    /* FreeConsole only detaches this test process, never its parent. */
    (void)FreeConsole(); /* An attached pseudoconsole need not have an HWND. */
    assert(AllocConsole());
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    for (fail_mutex = 1; fail_mutex <= 2; ++fail_mutex) {
        console_broker_backend *failed = NULL;
        DWORD handles_before, handles_after;
        mutex_creates = 0;
        assert(GetProcessHandleCount(GetCurrentProcess(), &handles_before));
        assert(console_broker_backend_create(&failed) == LIB_STATUS_NO_MEMORY);
        assert(failed == NULL && live_mutexes == 0);
        assert(GetProcessHandleCount(GetCurrentProcess(), &handles_after));
        assert(handles_before == handles_after);
    }
    fail_mutex = 0;
    assert(lib_console_create(&cooked) == 0);
    assert(lib_console_create(&raw) == 0);
    assert(lib_console_create(&other) == 0);
    assert(console_broker_create(&broker, cooked, CONSOLE_BROKER_COOKED_LINES) == 0);
    /* AllocConsole inherits host defaults, including very narrow windows. */
    {
        SMALL_RECT viewport = {0, 0, 19, 9};
        COORD size = {120, 60};
        assert(SetConsoleWindowInfo(broker->backend->output, TRUE, &viewport));
        assert(SetConsoleScreenBufferSize(broker->backend->output, size));
    }
    assert(FillConsoleOutputCharacterW(broker->backend->output, L' ', 80 * 25, origin, &written));
    assert(written == 80 * 25);
    assert(SetConsoleCursorPosition(broker->backend->output, origin));
    assert(SetConsoleCursorInfo(broker->backend->output, &cursor));
    {
        COORD size = {120, 60}, marker = {100, 28};
        SMALL_RECT viewport = {0, 0, 119, 29};
        assert(SetConsoleScreenBufferSize(broker->backend->output, size));
        assert(SetConsoleWindowInfo(broker->backend->output, TRUE, &viewport));
        assert(WriteConsoleOutputCharacterA(broker->backend->output, "wide history", 12, marker, &written));
        assert(written == 12);
    }
    assert(lib_console_write_text(cooked, "history\r\nSoftPC> start\r\n", 24) == 0);
    snapshot(&before);

    /* Allocation failure happens in prepare, before any old reader/display changes. */
    fail_allocate = 1;
    assert(console_broker_replace(broker, cooked, raw, CONSOLE_BROKER_RAW_EVENTS) == LIB_STATUS_IO_ERROR);
    expect_display(&before);
    assert(broker->current == cooked && !broker->broken);
    /* Both switch failure and reader failure restore the old display. */
    for (int failure = 0; failure < 3; ++failure) {
        fail_select = failure == 0;
        fail_reader = failure == 1;
        fail_query = failure == 2;
        assert(console_broker_replace(broker, cooked, raw, CONSOLE_BROKER_RAW_EVENTS) != 0);
        expect_display(&before);
        assert(broker->current == cooked && !broker->broken);
    }
    frame.columns = 80; frame.rows = 25; frame.font_height = 16;
    frame.cursor_row = 23; frame.cursor_column = 7;
    frame.cursor_bottom = 15; /* intentionally hidden */
    for (unsigned i=0; i<80u*25u; ++i) frame.text[i]='#';
    for (unsigned i = 0; i < 80 * 25; ++i) {
        frame.foreground[i] = 14;
        frame.background[i] = 1;
    }
    frame.palette[1] = 0x123456;
    for (int round = 0; round < 3; ++round) {
        assert(console_broker_replace(broker, cooked, raw, CONSOLE_BROKER_RAW_EVENTS) == 0);
        assert(lib_console_write_text_frame(raw, &frame) == 0);
        snapshot(&raw_display);
        assert(raw_display.cells[0].Char.UnicodeChar == '#');
        assert(!raw_display.cursor.bVisible);
        if (round < 2) {
            fail_select = round == 0;
            fail_restore = round == 1;
            assert(console_broker_replace(broker, raw, cooked, CONSOLE_BROKER_COOKED_LINES) == LIB_STATUS_IO_ERROR);
            assert(broker->current == raw && !broker->broken);
            expect_display(&raw_display);
        }
        assert(console_broker_replace(broker, raw, cooked, CONSOLE_BROKER_COOKED_LINES) == 0);
        expect_display(&before);
    }
    assert(lib_console_write_text(cooked, "ok\r\n\r\nSoftPC> ", 14) == 0);
    snapshot(&after);
    assert(after.info.dwCursorPosition.X == 8);
    assert(after.info.dwCursorPosition.Y == before.info.dwCursorPosition.Y + 2);
    for (unsigned x = 2; x < 80; ++x)
        assert(after.cells[before.info.dwCursorPosition.Y * 120 + x].Char.UnicodeChar == ' ');
    /* Same-mode binding does not clear text, move the cursor or switch screens. */
    assert(console_broker_replace(broker, cooked, other, CONSOLE_BROKER_COOKED_LINES) == 0);
    expect_display(&after);
    assert(console_broker_replace(broker, other, raw, CONSOLE_BROKER_RAW_EVENTS) == 0);
    assert(lib_console_write_text_frame(raw, &frame) == 0);
    assert(console_broker_replace(broker, raw, other, CONSOLE_BROKER_RAW_EVENTS) == 0);
    snapshot(&raw_display);
    assert(raw_display.cells[0].Char.UnicodeChar == '#');
    /* Closing while raw is active must restore the stream buffer as well. */
    assert(console_broker_destroy(broker) == 0);
    expect_display(&after);
    /* Initial raw creation has no preceding prepare/replace call. Its failed
     * reader startup must restore the original screen and release ownership. */
    fail_reader = 1;
    broker = NULL;
    assert(console_broker_create(&broker, raw, CONSOLE_BROKER_RAW_EVENTS) == LIB_STATUS_NO_MEMORY);
    assert(broker == NULL);
    expect_display(&after);
    assert(console_broker_create(&broker, raw, CONSOLE_BROKER_RAW_EVENTS) == 0);
    assert(console_broker_destroy(broker) == 0);
    expect_display(&after);
    lib_console_release(other); lib_console_release(raw); lib_console_release(cooked);
    check_frame_extent(30, 30, 0);
    check_frame_extent(30, 30, 2);
    check_frame_extent(80, 12, 0);
    check_frame_extent(80, 13, 0);
    check_frame_extent(120, 13, 0);
    check_frame_extent(120, 60, 1);
    assert(FreeConsole());
    return 0;
}
