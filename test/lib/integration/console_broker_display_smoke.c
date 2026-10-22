#include "lib/types/test.h"
#include "lib/types/win32/test.h"
#include "lib/types/file.h"
#include "lib/types/win32/console.h"
#include "lib/types/win32/sync.h"
#include "lib/base/sync_interface.h"
#include "lib/kvm-console/console_interface.h"

static lib_win32_handle frame_drawn;
static lib_win32_bool LIB_WIN32_WINAPI set_frame_cursor(lib_win32_handle output,
    const lib_win32_console_cursor_info *cursor)
{
    lib_win32_bool result = lib_win32_set_console_cursor_info(output, cursor);
    if (result && frame_drawn != LIB_NULL) lib_win32_set_event(frame_drawn);
    return result;
}
#undef lib_win32_set_console_cursor_info
#define lib_win32_set_console_cursor_info set_frame_cursor

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
static lib_i32 fail_resize, ignore_resize;
static lib_win32_bool LIB_WIN32_WINAPI resize_buffer(lib_win32_handle output, lib_win32_coord size)
{
    if (fail_resize) { fail_resize = 0; return LIB_WIN32_FALSE; }
    if (ignore_resize) { ignore_resize = 0; return LIB_WIN32_TRUE; }
    return lib_win32_set_console_screen_buffer_size(output, size);
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
static void hide_owned_console(void)
{
    lib_win32_show_window(lib_win32_get_console_window(), LIB_WIN32_SW_HIDE);
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
#undef lib_win32_set_console_screen_buffer_size
#define lib_win32_set_console_screen_buffer_size resize_buffer
#define base_sync_mutex_create create_mutex
#define base_sync_mutex_destroy destroy_mutex
#include "lib/console-broker/win32/console.c"
#undef base_sync_mutex_create
#undef base_sync_mutex_destroy
#include "lib/console-broker/console.c"

/* A Console buffer can be wider or taller than the native window which its
 * current font and display can show. Keep the requested viewport shape where
 * possible, but make every test setup legal for the host that runs it. */
static void set_supported_viewport(lib_win32_handle output, lib_win32_small_rect *viewport)
{
    lib_win32_console_screen_buffer_info info;
    lib_i32 width = viewport->Right - viewport->Left + 1;
    lib_i32 height = viewport->Bottom - viewport->Top + 1;
    lib_i32 maximum_width;
    lib_i32 maximum_height;

    lib_test_assert(lib_win32_get_console_screen_buffer_info(output, &info));
    maximum_width = info.dwMaximumWindowSize.X;
    maximum_height = info.dwMaximumWindowSize.Y;
    lib_test_assert(maximum_width > 0 && maximum_height > 0);
    if (width > maximum_width) width = maximum_width;
    if (height > maximum_height) height = maximum_height;
    if (viewport->Left + width > info.dwSize.X) viewport->Left = info.dwSize.X - width;
    if (viewport->Top + height > info.dwSize.Y) viewport->Top = info.dwSize.Y - height;
    viewport->Right = (lib_win32_short)(viewport->Left + width - 1);
    viewport->Bottom = (lib_win32_short)(viewport->Top + height - 1);
    lib_test_assert(lib_win32_set_console_window_info(output, LIB_WIN32_TRUE, viewport));
}

static lib_win32_handle open_console_output(void)
{
    lib_win32_handle output = lib_win32_create_file_a("CONOUT$",
        LIB_WIN32_GENERIC_READ | LIB_WIN32_GENERIC_WRITE,
        LIB_WIN32_FILE_SHARE_READ | LIB_WIN32_FILE_SHARE_WRITE, LIB_NULL,
        LIB_WIN32_OPEN_EXISTING, 0, LIB_NULL);
    lib_test_assert(output != LIB_WIN32_INVALID_HANDLE_VALUE);
    return output;
}

static void prepare_output_surface(lib_win32_coord size,
    lib_win32_small_rect *viewport)
{
    lib_win32_handle output = open_console_output();
    lib_win32_small_rect initial = { 0, 0, 19, 9 };

    set_supported_viewport(output, &initial);
    lib_test_assert(lib_win32_set_console_screen_buffer_size(output, size));
    set_supported_viewport(output, viewport);
    lib_test_assert(lib_win32_close_handle(output));
}

typedef struct display_snapshot {
    lib_win32_console_screen_buffer_infoex info;
    lib_win32_console_cursor_info cursor;
    lib_win32_dword mode;
    lib_i16 columns;
    lib_i16 rows;
    lib_win32_char_info cells[120 * 30];
} display_snapshot;

static void snapshot(display_snapshot *s)
{
    lib_win32_handle output = open_console_output();
    lib_win32_coord size, origin = {0, 0};
    lib_win32_small_rect region;
    lib_memory_set(s, 0, sizeof(*s));
    s->info.cbSize = sizeof(s->info);
    lib_test_assert(lib_win32_get_console_screen_buffer_info_ex(output, &s->info));
    lib_test_assert(lib_win32_get_console_cursor_info(output, &s->cursor));
    lib_test_assert(lib_win32_get_console_mode(output, &s->mode));
    s->columns = s->info.srWindow.Right - s->info.srWindow.Left + 1;
    s->rows = s->info.srWindow.Bottom - s->info.srWindow.Top + 1;
    if (s->columns > 120) s->columns = 120;
    if (s->rows > 30) s->rows = 30;
    lib_test_assert(s->columns > 0 && s->rows > 0);
    size = (lib_win32_coord){s->columns, s->rows};
    region = s->info.srWindow;
    region.Right = (lib_win32_short)(region.Left + s->columns - 1);
    region.Bottom = (lib_win32_short)(region.Top + s->rows - 1);
    lib_test_assert(lib_win32_read_console_output_w(output, s->cells, size, origin, &region));
    lib_test_assert(lib_win32_close_handle(output));
}

static void expect_display(const display_snapshot *expected)
{
    display_snapshot actual;
    display_snapshot normalized;
    lib_size cells;
    static lib_u32 checkpoint;

    snapshot(&actual);
    normalized = *expected;
    /* A Console host may reduce its visible maximum while keeping the saved
     * backing buffer and display metadata intact.  Restore must preserve every
     * visible cell that the current native geometry can expose; it cannot make
     * an unavailable physical row visible. */
    normalized.info.dwMaximumWindowSize = actual.info.dwMaximumWindowSize;
    lib_test_assert(console_broker_fit_viewport(&normalized.info.srWindow,
        &expected->info.srWindow, normalized.info.dwSize,
        normalized.info.dwMaximumWindowSize));
    normalized.columns = normalized.info.srWindow.Right - normalized.info.srWindow.Left + 1;
    normalized.rows = normalized.info.srWindow.Bottom - normalized.info.srWindow.Top + 1;
    cells = (lib_size)normalized.columns * (lib_size)normalized.rows;
    ++checkpoint;
    if (lib_memory_compare(&actual.info, &normalized.info, sizeof(actual.info)) != 0 ||
        lib_memory_compare(&actual.cursor, &normalized.cursor, sizeof(actual.cursor)) != 0 ||
        actual.mode != normalized.mode || actual.columns != normalized.columns ||
        actual.rows != normalized.rows ||
        lib_memory_compare(actual.cells, normalized.cells,
            cells * sizeof(actual.cells[0])) != 0) {
        lib_c_fprintf(lib_c_stderr, "display checkpoint %u: size %d,%d/%d,%d cursor %d,%d/%d,%d viewport %d,%d,%d,%d/%d,%d,%d,%d cells=%d palette=%d cursor-style=%d mode=%lu/%lu\n",
            checkpoint, actual.info.dwSize.X, actual.info.dwSize.Y,
            normalized.info.dwSize.X, normalized.info.dwSize.Y,
            actual.info.dwCursorPosition.X, actual.info.dwCursorPosition.Y,
            normalized.info.dwCursorPosition.X, normalized.info.dwCursorPosition.Y,
            actual.info.srWindow.Left, actual.info.srWindow.Top, actual.info.srWindow.Right, actual.info.srWindow.Bottom,
            normalized.info.srWindow.Left, normalized.info.srWindow.Top, normalized.info.srWindow.Right, normalized.info.srWindow.Bottom,
            lib_memory_compare(actual.cells, normalized.cells, cells * sizeof(actual.cells[0])),
            lib_memory_compare(actual.info.ColorTable, normalized.info.ColorTable, sizeof(actual.info.ColorTable)),
            lib_memory_compare(&actual.cursor, &normalized.cursor, sizeof(actual.cursor)), actual.mode, normalized.mode);
    }
    lib_test_assert(lib_memory_compare(&actual.info, &normalized.info, sizeof(actual.info)) == 0);
    lib_test_assert(lib_memory_compare(&actual.cursor, &normalized.cursor, sizeof(actual.cursor)) == 0);
    lib_test_assert(actual.mode == normalized.mode);
    lib_test_assert(actual.columns == normalized.columns && actual.rows == normalized.rows);
    lib_test_assert(lib_memory_compare(actual.cells, normalized.cells,
        cells * sizeof(actual.cells[0])) == 0);
}

static void check_frame_extent(lib_i16 columns, lib_i16 rows, lib_i32 scrolled)
{
    console_broker *broker = LIB_NULL;
    lib_console *cooked, *raw;
    lib_u16 text[80u * 25u] = {0};
    lib_u8 foreground[80u * 25u] = {0}, background[80u * 25u] = {0},
        underline[80u * 25u] = {0};
    lib_console_text_frame frame = {.stride=80u, .capacity=80u*25u,
        .text=text, .foreground=foreground, .background=background, .underline=underline};
    lib_win32_console_screen_buffer_info before, actual;
    display_snapshot cooked_display;
    lib_win32_coord extent = {columns, rows}, origin = {0, 0}, cells_size = {80, 25};
    lib_win32_small_rect viewport = {0, 0, 19, 9}, region;
    lib_win32_char_info cells[80 * 25];
    lib_test_assert(lib_console_create(&cooked) == 0);
    lib_test_assert(lib_console_create(&raw) == 0);
    viewport.Right = scrolled == 2 ? 19 : columns - 1;
    viewport.Bottom = scrolled == 2 ? 9 : rows < 30 ? rows - 1 : 29;
    if (scrolled == 1) {
        viewport.Top = 2; viewport.Bottom += 2;
    }
    prepare_output_surface(extent, &viewport);
    lib_test_assert(console_broker_create(&broker, cooked, CONSOLE_BROKER_COOKED_LINES) == 0);
    lib_test_assert(lib_win32_set_console_cursor_position(broker->backend->output, origin));
    lib_test_assert(lib_win32_get_console_screen_buffer_info(broker->backend->output, &before));
    lib_test_assert(lib_console_write_text(cooked, "cooked", 6u) == LIB_STATUS_OK);
    snapshot(&cooked_display);
    frame.columns = 80; frame.rows = 25; frame.font_height = 16;
    frame.cursor_column = 7; frame.cursor_row = 8; frame.cursor_visible = LIB_TRUE; frame.cursor_phase = LIB_TRUE;
    frame.cursor_bottom = 15;
    for (lib_u32 i=0; i<80u*25u; ++i) {
        frame.text[i]=(lib_u32)('A'+i%26u);
        frame.foreground[i]=14u; frame.background[i]=1u;
        frame.underline[i]=(lib_u8)(i % 3u == 0u);
    }
    for (lib_i32 round = 0; round < 3; ++round) {
        lib_status status = console_broker_replace(broker, cooked, raw, CONSOLE_BROKER_RAW_EVENTS);
        lib_test_assert(status == LIB_STATUS_OK);
        if (round != 0 && before.srWindow.Right - before.srWindow.Left + 1 >= 80 &&
                before.srWindow.Bottom - before.srWindow.Top + 1 >= 25) {
            /* No ensure, viewport adjustment or rewrite may precede this read. */
            region = (lib_win32_small_rect){0,0,79,24};
            lib_test_assert(lib_win32_read_console_output_w(broker->backend->output, cells, cells_size, origin, &region));
            lib_test_assert(region.Right==79 && region.Bottom==24);
            for (lib_u32 i=0; i<80u*25u; ++i) {
                lib_test_assert(cells[i].Char.UnicodeChar==frame.text[i]);
                lib_test_assert(cells[i].Attributes==(0x1eu |
                    (frame.underline[i] ? LIB_WIN32_COMMON_LVB_UNDERSCORE : 0u)));
            }
            lib_test_assert(lib_win32_get_console_screen_buffer_info(broker->backend->output, &actual));
            lib_test_assert(actual.dwCursorPosition.X==7 && actual.dwCursorPosition.Y==8);
        }
        /* Deliberate test resize and subsequent frame repair are separate. */
        set_supported_viewport(broker->backend->output, &viewport);
        if (round == 0 && !scrolled && rows == 13) {
            fail_resize = 1;
            lib_test_assert(!console_broker_ensure_text_surface(broker->backend, 80u, 25u));
            lib_test_assert(fail_resize == 0);
            ignore_resize = 1;
            lib_test_assert(!console_broker_ensure_text_surface(broker->backend, 80u, 25u));
            lib_test_assert(ignore_resize == 0);
        }
        {
            lib_win32_console_screen_buffer_info raw_before;
            lib_test_assert(lib_win32_get_console_screen_buffer_info(broker->backend->output, &raw_before));
            lib_test_assert(console_broker_ensure_text_surface(broker->backend, 80u, 25u));
            lib_test_assert(lib_win32_get_console_screen_buffer_info(broker->backend->output, &actual));
            lib_test_assert(lib_memory_compare(&actual.srWindow, &raw_before.srWindow,
                sizeof(actual.srWindow)) == 0);
            lib_test_assert(actual.dwSize.X >= 80 && actual.dwSize.Y >= 25);
            lib_test_assert(actual.dwSize.X >= raw_before.dwSize.X);
            lib_test_assert(actual.dwSize.Y >= raw_before.dwSize.Y);
        }
        lib_test_assert(lib_console_write_text_frame(raw, &frame) == 0);
        lib_test_assert(lib_console_write_text_frame(raw, &frame) == 0);
        region = (lib_win32_small_rect){0, 0, 79, 24};
        lib_test_assert(lib_win32_read_console_output_w(broker->backend->output, cells, cells_size, origin, &region));
        lib_test_assert(region.Left == 0 && region.Top == 0 && region.Right == 79 && region.Bottom == 24);
        for (lib_u32 i = 0; i < 80 * 25; ++i) {
            lib_test_assert(cells[i].Char.UnicodeChar == frame.text[i]);
            lib_test_assert(cells[i].Attributes == (0x1eu |
                (frame.underline[i] ? LIB_WIN32_COMMON_LVB_UNDERSCORE : 0u)));
        }
        lib_test_assert(console_broker_replace(broker, raw, cooked, CONSOLE_BROKER_COOKED_LINES) == 0);
        expect_display(&cooked_display);
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

static lib_bool restore_input(void *context, const kvm_input_event *event)
{ (void)context; (void)event; return LIB_TRUE; }

static void restore_failure(void *context, lib_u64 source, lib_status status)
{ (void)context; (void)source; (void)status; lib_test_assert(0); }

static void check_retained_console_restore(void)
{
    lib_console *cooked;
    console_broker *broker;
    kvm_console *console;
    kvm_frame_exchange *frames;
    kvm_console_options options = {0};
    lib_win32_coord extent = {30,30}, origin = {0,0}, size = {80,25};
    lib_win32_small_rect viewport = {0,0,29,29}, region;
    lib_win32_char_info cells[80*25];
    display_snapshot cooked_display;
    prepare_output_surface(extent, &viewport);
    lib_test_assert(lib_console_create(&cooked) == LIB_STATUS_OK);
    lib_test_assert(console_broker_create(&broker, cooked, CONSOLE_BROKER_COOKED_LINES) == LIB_STATUS_OK);
    lib_test_assert(lib_win32_set_console_cursor_position(broker->backend->output, origin));
    lib_test_assert(lib_console_write_text(cooked, "monitor", 7u) == LIB_STATUS_OK);
    snapshot(&cooked_display);
    options.input_sink = restore_input; options.failure_sink = restore_failure;
    lib_test_assert(kvm_console_create(&console, &options) == LIB_STATUS_OK);
    lib_test_assert(kvm_frame_exchange_create(&frames, sizeof(kvm_console_text_frame), kvm_console_frame_retire) == LIB_STATUS_OK);
    kvm_console_text_frame *frame = kvm_frame_exchange_writer(frames);
    lib_test_assert(kvm_console_frame_prepare(frame, 80u, 25u, 80u) == LIB_STATUS_OK);
    frame->base.cursor_mode=KVM_CURSOR_HOST;
    frame->base.cursor_column=7; frame->base.cursor_row=8;
    frame->base.cursor[0] = (kvm_text_cursor_span){ 14, 2u };
    for (lib_u32 i=0;i<256u;++i) frame->characters->primary[i]=(lib_u16)i;
    for (lib_u32 i=0;i<2000u;++i) {
        frame->base.cells[i].glyph_index=(lib_u8)('A'+i%26u);
        frame->base.cells[i].foreground=14; frame->base.cells[i].background=1;
    }
    lib_test_assert(kvm_console_frame_commit(frames) == LIB_STATUS_OK);
    frame_drawn=lib_win32_create_event_a(LIB_NULL,LIB_WIN32_FALSE,LIB_WIN32_FALSE,LIB_NULL);
    lib_test_assert(frame_drawn != LIB_NULL);
    lib_console *raw=kvm_console_get_console(console);
    lib_test_assert(console_broker_replace(broker,cooked,raw,CONSOLE_BROKER_RAW_EVENTS)==LIB_STATUS_OK);
    lib_test_assert(kvm_console_publish_frame(console,frames)==LIB_STATUS_OK);
    for (lib_u32 round=0;round<3u;++round) {
        lib_test_assert(lib_win32_wait_for_single_object(frame_drawn,5000u)==LIB_WIN32_WAIT_OBJECT_0);
        console_broker_backend_lock_output(broker->backend);
        region=(lib_win32_small_rect){0,0,79,24};
        lib_test_assert(lib_win32_read_console_output_w(broker->backend->output,cells,size,origin,&region));
        lib_test_assert(region.Right==79 && region.Bottom==24);
        for (lib_u32 i=0;i<2000u;++i) {
            lib_test_assert(cells[i].Char.UnicodeChar==(lib_u16)('A'+i%26u));
            lib_test_assert(cells[i].Attributes==0x1eu);
        }
        lib_win32_console_screen_buffer_info actual;
        lib_test_assert(lib_win32_get_console_screen_buffer_info(broker->backend->output,&actual));
        lib_test_assert(actual.dwCursorPosition.X==7 && actual.dwCursorPosition.Y==8);
        console_broker_backend_unlock_output(broker->backend);
        lib_test_assert(console_broker_replace(broker,raw,cooked,CONSOLE_BROKER_COOKED_LINES)==LIB_STATUS_OK);
        expect_display(&cooked_display);
        if (round!=2u)
            lib_test_assert(console_broker_replace(broker,cooked,raw,CONSOLE_BROKER_RAW_EVENTS)==LIB_STATUS_OK);
        /* No producer commit or notification after the first publication. */
    }
    lib_test_assert(console_broker_destroy(broker)==LIB_STATUS_OK);
    lib_test_assert(kvm_console_destroy(console)==LIB_STATUS_OK);
    kvm_frame_exchange_destroy(frames);
    lib_win32_close_handle(frame_drawn); frame_drawn=LIB_NULL;
    lib_console_release(cooked);
}

int main(void)
{
    console_broker *broker;
    lib_console *cooked, *raw, *other;
    lib_u16 text[80u * 50u] = {0};
    lib_u8 foreground[80u * 50u] = {0}, background[80u * 50u] = {0},
        underline[80u * 50u] = {0};
    lib_console_text_frame frame = {.stride=80u, .capacity=80u*50u,
        .text=text, .foreground=foreground, .background=background, .underline=underline};
    display_snapshot before, after, raw_display;
    lib_win32_dword written;
    lib_win32_coord origin = {0, 0};
    lib_win32_console_cursor_info cursor = {25, LIB_WIN32_TRUE};
    /* FreeConsole only detaches this test process, never its parent. */
    (void)lib_win32_free_console(); /* An attached pseudoconsole need not have an lib_win32_hwnd. */
    lib_test_assert(lib_win32_alloc_console());
    hide_owned_console();
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
    {
        lib_win32_small_rect viewport = {0, 0, 119, 29};
        lib_win32_coord size = {120, 60};
        /* AllocConsole inherits host defaults, including very narrow windows. */
        prepare_output_surface(size, &viewport);
    }
    lib_test_assert(console_broker_create(&broker, cooked, CONSOLE_BROKER_COOKED_LINES) == 0);
    lib_test_assert(lib_win32_fill_console_output_character_w(broker->backend->output, L' ', 80 * 25, origin, &written));
    lib_test_assert(written == 80 * 25);
    lib_test_assert(lib_win32_set_console_cursor_position(broker->backend->output, origin));
    lib_test_assert(lib_win32_set_console_cursor_info(broker->backend->output, &cursor));
    {
        lib_win32_coord marker = {100, 28};
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
        /* Prove saved raw contents before rewriting them. The separate extent
         * tests intentionally resize host geometry and test frame repair. */
        if (round != 0) expect_display(&raw_display);
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
    {
        lib_i16 row = before.info.dwCursorPosition.Y - after.info.srWindow.Top;

        for (lib_i16 x = 2; x < 80; ++x)
            lib_test_assert(after.cells[row * after.columns + x].Char.UnicodeChar == ' ');
    }
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
    check_retained_console_restore();
    lib_test_assert(lib_win32_free_console());
    return 0;
}
