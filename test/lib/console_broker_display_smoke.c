#include "lib/types/test.h"
#include "lib/types/win32/test.h"
#include "lib/types/file.h"
#include "lib/types/win32/console.h"
#include "lib/types/win32/sync.h"
#include "lib/base/sync_interface.h"

static lib_u32 mutex_creates, fail_mutex, live_mutexes;
static lib_status create_mutex(base_sync_mutex **out)
{
    if (++mutex_creates == fail_mutex) { *out = LIB_NULL; return LIB_STATUS_NO_MEMORY; }
    lib_status status = base_sync_mutex_create(out);
    if (status == LIB_STATUS_OK) ++live_mutexes;
    return status;
}
static void destroy_mutex(base_sync_mutex *mutex)
{
    if (mutex != LIB_NULL) { lib_test_assert(live_mutexes); --live_mutexes; }
    base_sync_mutex_destroy(mutex);
}

/* Native display I/O, deterministic reader/startup failures. The test owns a
 * hidden Console; it never changes the developer's Console or its input. */
static lib_i32 fail_allocate, fail_select, fail_reader, fail_query, fail_restore;
static lib_i32 fail_viewport, ignore_viewport;
static lib_win32_bool LIB_WIN32_WINAPI set_viewport(lib_win32_handle output, lib_win32_bool absolute, const lib_win32_small_rect *rect)
{
    if (fail_viewport) { fail_viewport = 0; return LIB_WIN32_FALSE; }
    if (ignore_viewport) { ignore_viewport = 0; return LIB_WIN32_TRUE; }
    return lib_win32_set_console_window_info(output, absolute, rect);
}
static lib_win32_bool LIB_WIN32_WINAPI query_display(lib_win32_handle output, lib_win32_console_screen_buffer_infoex *info)
{
    if (fail_query) { fail_query = 0; return LIB_WIN32_FALSE; }
    return lib_win32_get_console_screen_buffer_info_ex(output, info);
}
static lib_win32_bool LIB_WIN32_WINAPI restore_display(lib_win32_handle output, lib_win32_console_screen_buffer_infoex *info)
{
    if (fail_restore) { fail_restore = 0; return LIB_WIN32_FALSE; }
    return lib_win32_set_console_screen_buffer_info_ex(output, info);
}
static lib_win32_handle LIB_WIN32_WINAPI allocate_screen(lib_win32_dword access, lib_win32_dword share,
    const lib_win32_security_attributes *security, lib_win32_dword flags, lib_win32_lpvoid reserved)
{
    if (fail_allocate) { fail_allocate = 0; return LIB_WIN32_INVALID_HANDLE_VALUE; }
    return lib_win32_create_console_screen_buffer(access, share, security, flags, reserved);
}
static lib_win32_bool LIB_WIN32_WINAPI select_screen(lib_win32_handle output)
{
    if (fail_select) { fail_select = 0; return LIB_WIN32_FALSE; }
    return lib_win32_set_console_active_screen_buffer(output);
}
static lib_win32_handle LIB_WIN32_WINAPI start_reader(lib_win32_lpsecurity_attributes attributes, lib_win32_size_t size,
    lib_win32_thread_start_routine entry, lib_win32_lpvoid arg, lib_win32_dword flags, lib_win32_lpdword id)
{
    (void)attributes; (void)size; (void)entry; (void)arg; (void)flags; (void)id;
    if (fail_reader) { fail_reader = 0; return LIB_NULL; }
    return lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_TRUE, LIB_NULL);
}
static lib_win32_hwnd LIB_WIN32_WINAPI no_foreground(void) { return LIB_NULL; }
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
    lib_win32_console_screen_buffer_infoex info;
    lib_win32_console_cursor_info cursor;
    lib_win32_dword mode;
    lib_win32_char_info cells[120 * 30];
} display_snapshot;

static void snapshot(display_snapshot *s)
{
    lib_win32_handle output = lib_win32_create_file_a("CONOUT$", LIB_WIN32_GENERIC_READ | LIB_WIN32_GENERIC_WRITE,
        LIB_WIN32_FILE_SHARE_READ | LIB_WIN32_FILE_SHARE_WRITE, LIB_NULL, LIB_WIN32_OPEN_EXISTING, 0, LIB_NULL);
    lib_win32_coord size = {120, 30}, origin = {0, 0};
    lib_win32_small_rect region = {0, 0, 119, 29};
    lib_test_assert(output != LIB_WIN32_INVALID_HANDLE_VALUE);
    lib_memory_set(s, 0, sizeof(*s));
    s->info.cbSize = sizeof(s->info);
    lib_test_assert(lib_win32_get_console_screen_buffer_info_ex(output, &s->info));
    lib_test_assert(lib_win32_get_console_cursor_info(output, &s->cursor));
    lib_test_assert(lib_win32_get_console_mode(output, &s->mode));
    lib_test_assert(lib_win32_read_console_output_w(output, s->cells, size, origin, &region));
    lib_test_assert(region.Right >= 79 && region.Bottom >= 24);
    lib_test_assert(lib_win32_close_handle(output));
}

static void expect_display(const display_snapshot *expected)
{
    display_snapshot actual;
    static lib_u32 checkpoint;
    snapshot(&actual);
    ++checkpoint;
    if (lib_memory_compare(&actual, expected, sizeof(actual)) != 0) {
        lib_c_fprintf(lib_c_stderr, "display checkpoint %u: size %d,%d/%d,%d cursor %d,%d/%d,%d viewport %d,%d,%d,%d/%d,%d,%d,%d cells=%d palette=%d cursor-style=%d mode=%lu/%lu\n",
            checkpoint, actual.info.dwSize.X, actual.info.dwSize.Y,
            expected->info.dwSize.X, expected->info.dwSize.Y,
            actual.info.dwCursorPosition.X, actual.info.dwCursorPosition.Y,
            expected->info.dwCursorPosition.X, expected->info.dwCursorPosition.Y,
            actual.info.srWindow.Left, actual.info.srWindow.Top, actual.info.srWindow.Right, actual.info.srWindow.Bottom,
            expected->info.srWindow.Left, expected->info.srWindow.Top, expected->info.srWindow.Right, expected->info.srWindow.Bottom,
            lib_memory_compare(actual.cells, expected->cells, sizeof(actual.cells)),
            lib_memory_compare(actual.info.ColorTable, expected->info.ColorTable, sizeof(actual.info.ColorTable)),
            lib_memory_compare(&actual.cursor, &expected->cursor, sizeof(actual.cursor)), actual.mode, expected->mode);
    }
    lib_test_assert(lib_memory_compare(&actual, expected, sizeof(actual)) == 0);
}

static void check_frame_extent(lib_i16 columns, lib_i16 rows, lib_i32 scrolled)
{
    console_broker *broker = LIB_NULL;
    lib_console *cooked, *raw;
    lib_console_text_frame frame = {0};
    lib_win32_short write_rows;
    lib_win32_console_screen_buffer_info before, actual;
    lib_win32_coord extent = {columns, rows}, origin = {0, 0}, cells_size = {80, 25};
    lib_win32_small_rect viewport = {0, 0, 19, 9}, region;
    lib_win32_char_info cells[80 * 25];
    lib_test_assert(lib_console_create(&cooked) == 0);
    lib_test_assert(lib_console_create(&raw) == 0);
    lib_test_assert(console_broker_create(&broker, cooked, CONSOLE_BROKER_COOKED_LINES) == 0);
    lib_test_assert(lib_win32_set_console_cursor_position(broker->backend->output, origin));
    lib_test_assert(lib_win32_set_console_window_info(broker->backend->output, LIB_WIN32_TRUE, &viewport));
    lib_test_assert(lib_win32_set_console_screen_buffer_size(broker->backend->output, extent));
    viewport.Right = scrolled == 2 ? 19 : columns - 1;
    viewport.Bottom = scrolled == 2 ? 9 : rows < 30 ? rows - 1 : 29;
    if (scrolled == 1) {
        viewport.Top = 2; viewport.Bottom += 2;
    }
    lib_test_assert(lib_win32_set_console_window_info(broker->backend->output, LIB_WIN32_TRUE, &viewport));
    lib_test_assert(lib_win32_get_console_screen_buffer_info(broker->backend->output, &before));
    frame.columns = 80; frame.rows = 25; frame.font_height = 16;
    for (lib_u32 i=0; i<80u*25u; ++i) frame.text[i]='#';
    for (lib_i32 round = 0; round < 3; ++round) {
        lib_test_assert(console_broker_replace(broker, cooked, raw, CONSOLE_BROKER_RAW_EVENTS) == 0);
        lib_test_assert(lib_win32_set_console_window_info(broker->backend->output, LIB_WIN32_TRUE, &viewport));
        if (round == 0 && !scrolled && rows == 13) {
            fail_viewport = 1;
            lib_test_assert(!console_broker_ensure_text_surface(broker->backend, 25u, &write_rows));
            lib_test_assert(fail_viewport == 0);
            ignore_viewport = 1;
            lib_test_assert(!console_broker_ensure_text_surface(broker->backend, 25u, &write_rows));
            lib_test_assert(ignore_viewport == 0);
        }
        {
            lib_win32_console_screen_buffer_info raw_before;
            lib_i32 width = viewport.Right - viewport.Left + 1;
            lib_i32 height = viewport.Bottom - viewport.Top + 1;
            lib_test_assert(lib_win32_get_console_screen_buffer_info(broker->backend->output, &raw_before));
            lib_test_assert(console_broker_ensure_text_surface(broker->backend, 25u, &write_rows));
            lib_test_assert(lib_win32_get_console_screen_buffer_info(broker->backend->output, &actual));
            lib_test_assert(actual.srWindow.Left == 0 && actual.srWindow.Top == 0);
            lib_test_assert(actual.srWindow.Right + 1 == (width < 80 ? 80 : width));
            lib_test_assert(actual.srWindow.Bottom + 1 == (height < 25 ? 25 : height));
            lib_test_assert(actual.dwSize.X >= raw_before.dwSize.X);
            lib_test_assert(actual.dwSize.Y >= raw_before.dwSize.Y);
        }
        lib_test_assert(lib_console_write_text_frame(raw, &frame) == 0);
        lib_test_assert(lib_console_write_text_frame(raw, &frame) == 0);
        region = (lib_win32_small_rect){0, 0, 79, 24};
        lib_test_assert(lib_win32_read_console_output_w(broker->backend->output, cells, cells_size, origin, &region));
        lib_test_assert(region.Left == 0 && region.Top == 0 && region.Right == 79 && region.Bottom == 24);
        for (lib_u32 i = 0; i < 80 * 25; ++i) lib_test_assert(cells[i].Char.UnicodeChar == '#');
        lib_test_assert(console_broker_replace(broker, raw, cooked, CONSOLE_BROKER_COOKED_LINES) == 0);
        lib_test_assert(lib_win32_get_console_screen_buffer_info(broker->backend->output, &actual));
        if (actual.dwSize.X != before.dwSize.X || actual.dwSize.Y != before.dwSize.Y)
            lib_c_fprintf(lib_c_stderr, "extent %d,%d round %d: restored %d,%d expected %d,%d\n",
                columns, rows, round, actual.dwSize.X, actual.dwSize.Y, before.dwSize.X, before.dwSize.Y);
        lib_test_assert(actual.dwSize.X == before.dwSize.X && actual.dwSize.Y == before.dwSize.Y);
        if (lib_memory_compare(&actual.srWindow, &before.srWindow, sizeof(actual.srWindow)) != 0)
            lib_c_fprintf(lib_c_stderr, "viewport %d,%d round %d: %d,%d,%d,%d expected %d,%d,%d,%d\n",
                columns, rows, round, actual.srWindow.Left, actual.srWindow.Top,
                actual.srWindow.Right, actual.srWindow.Bottom, before.srWindow.Left,
                before.srWindow.Top, before.srWindow.Right, before.srWindow.Bottom);
        lib_test_assert(lib_memory_compare(&actual.srWindow, &before.srWindow, sizeof(actual.srWindow)) == 0);
    }
    lib_test_assert(console_broker_destroy(broker) == 0);
    lib_console_release(raw); lib_console_release(cooked);
}

int main(void)
{
    console_broker *broker;
    lib_console *cooked, *raw, *other;
    lib_console_text_frame frame = {0};
    display_snapshot before, after, raw_display;
    lib_win32_dword written;
    lib_win32_coord origin = {0, 0};
    lib_win32_console_cursor_info cursor = {25, LIB_WIN32_TRUE};
    /* FreeConsole only detaches this test process, never its parent. */
    (void)lib_win32_free_console(); /* An attached pseudoconsole need not have an lib_win32_hwnd. */
    lib_test_assert(lib_win32_alloc_console());
    lib_win32_show_window(lib_win32_get_console_window(), LIB_WIN32_SW_HIDE);
    for (fail_mutex = 1; fail_mutex <= 2; ++fail_mutex) {
        console_broker_backend *failed = LIB_NULL;
        lib_win32_dword handles_before, handles_after;
        mutex_creates = 0;
        lib_test_assert(lib_win32_get_process_handle_count(lib_win32_get_current_process(), &handles_before));
        lib_test_assert(console_broker_backend_create(&failed) == LIB_STATUS_NO_MEMORY);
        lib_test_assert(failed == LIB_NULL && live_mutexes == 0);
        lib_test_assert(lib_win32_get_process_handle_count(lib_win32_get_current_process(), &handles_after));
        lib_test_assert(handles_before == handles_after);
    }
    fail_mutex = 0;
    lib_test_assert(lib_console_create(&cooked) == 0);
    lib_test_assert(lib_console_create(&raw) == 0);
    lib_test_assert(lib_console_create(&other) == 0);
    lib_test_assert(console_broker_create(&broker, cooked, CONSOLE_BROKER_COOKED_LINES) == 0);
    /* AllocConsole inherits host defaults, including very narrow windows. */
    {
        lib_win32_small_rect viewport = {0, 0, 19, 9};
        lib_win32_coord size = {120, 60};
        lib_test_assert(lib_win32_set_console_window_info(broker->backend->output, LIB_WIN32_TRUE, &viewport));
        lib_test_assert(lib_win32_set_console_screen_buffer_size(broker->backend->output, size));
    }
    lib_test_assert(lib_win32_fill_console_output_character_w(broker->backend->output, L' ', 80 * 25, origin, &written));
    lib_test_assert(written == 80 * 25);
    lib_test_assert(lib_win32_set_console_cursor_position(broker->backend->output, origin));
    lib_test_assert(lib_win32_set_console_cursor_info(broker->backend->output, &cursor));
    {
        lib_win32_coord size = {120, 60}, marker = {100, 28};
        lib_win32_small_rect viewport = {0, 0, 119, 29};
        lib_test_assert(lib_win32_set_console_screen_buffer_size(broker->backend->output, size));
        lib_test_assert(lib_win32_set_console_window_info(broker->backend->output, LIB_WIN32_TRUE, &viewport));
        lib_test_assert(lib_win32_write_console_output_character_a(broker->backend->output, "wide history", 12, marker, &written));
        lib_test_assert(written == 12);
    }
    lib_test_assert(lib_console_write_text(cooked, "history\r\nMonitor> start\r\n", 25) == 0);
    snapshot(&before);

    /* Allocation failure happens in prepare, before any old reader/display changes. */
    fail_allocate = 1;
    lib_test_assert(console_broker_replace(broker, cooked, raw, CONSOLE_BROKER_RAW_EVENTS) == LIB_STATUS_IO_ERROR);
    expect_display(&before);
    lib_test_assert(broker->current == cooked && !broker->broken);
    /* Both switch failure and reader failure restore the old display. */
    for (lib_i32 failure = 0; failure < 3; ++failure) {
        fail_select = failure == 0;
        fail_reader = failure == 1;
        fail_query = failure == 2;
        lib_test_assert(console_broker_replace(broker, cooked, raw, CONSOLE_BROKER_RAW_EVENTS) != 0);
        expect_display(&before);
        lib_test_assert(broker->current == cooked && !broker->broken);
    }
    frame.columns = 80; frame.rows = 25; frame.font_height = 16;
    frame.cursor_row = 23; frame.cursor_column = 7;
    frame.cursor_bottom = 15; /* intentionally hidden */
    for (lib_u32 i=0; i<80u*25u; ++i) frame.text[i]='#';
    for (lib_u32 i = 0; i < 80 * 25; ++i) {
        frame.foreground[i] = 14;
        frame.background[i] = 1;
    }
    frame.palette[1] = 0x123456;
    for (lib_i32 round = 0; round < 3; ++round) {
        lib_test_assert(console_broker_replace(broker, cooked, raw, CONSOLE_BROKER_RAW_EVENTS) == 0);
        lib_test_assert(lib_console_write_text_frame(raw, &frame) == 0);
        snapshot(&raw_display);
        lib_test_assert(raw_display.cells[0].Char.UnicodeChar == '#');
        lib_test_assert(!raw_display.cursor.bVisible);
        if (round < 2) {
            fail_select = round == 0;
            fail_restore = round == 1;
            lib_test_assert(console_broker_replace(broker, raw, cooked, CONSOLE_BROKER_COOKED_LINES) == LIB_STATUS_IO_ERROR);
            lib_test_assert(broker->current == raw && !broker->broken);
            expect_display(&raw_display);
        }
        lib_test_assert(console_broker_replace(broker, raw, cooked, CONSOLE_BROKER_COOKED_LINES) == 0);
        expect_display(&before);
    }
    lib_test_assert(lib_console_write_text(cooked, "ok\r\n\r\nMonitor> ", 15) == 0);
    snapshot(&after);
    /* "Monitor> " is nine cells wide; the cursor is the next cell. */
    lib_test_assert(after.info.dwCursorPosition.X == 9);
    lib_test_assert(after.info.dwCursorPosition.Y == before.info.dwCursorPosition.Y + 2);
    for (lib_u32 x = 2; x < 80; ++x)
        lib_test_assert(after.cells[before.info.dwCursorPosition.Y * 120 + x].Char.UnicodeChar == ' ');
    /* Same-mode binding does not clear text, move the cursor or switch screens. */
    lib_test_assert(console_broker_replace(broker, cooked, other, CONSOLE_BROKER_COOKED_LINES) == 0);
    expect_display(&after);
    lib_test_assert(console_broker_replace(broker, other, raw, CONSOLE_BROKER_RAW_EVENTS) == 0);
    lib_test_assert(lib_console_write_text_frame(raw, &frame) == 0);
    lib_test_assert(console_broker_replace(broker, raw, other, CONSOLE_BROKER_RAW_EVENTS) == 0);
    snapshot(&raw_display);
    lib_test_assert(raw_display.cells[0].Char.UnicodeChar == '#');
    /* Closing while raw is active must restore the stream buffer as well. */
    lib_test_assert(console_broker_destroy(broker) == 0);
    expect_display(&after);
    /* Initial raw creation has no preceding prepare/replace call. Its failed
     * reader startup must restore the original screen and release ownership. */
    fail_reader = 1;
    broker = LIB_NULL;
    lib_test_assert(console_broker_create(&broker, raw, CONSOLE_BROKER_RAW_EVENTS) == LIB_STATUS_NO_MEMORY);
    lib_test_assert(broker == LIB_NULL);
    expect_display(&after);
    lib_test_assert(console_broker_create(&broker, raw, CONSOLE_BROKER_RAW_EVENTS) == 0);
    lib_test_assert(console_broker_destroy(broker) == 0);
    expect_display(&after);
    lib_console_release(other); lib_console_release(raw); lib_console_release(cooked);
    check_frame_extent(30, 30, 0);
    check_frame_extent(30, 30, 2);
    check_frame_extent(80, 12, 0);
    check_frame_extent(80, 13, 0);
    check_frame_extent(120, 13, 0);
    check_frame_extent(120, 60, 1);
    lib_test_assert(lib_win32_free_console());
    return 0;
}
